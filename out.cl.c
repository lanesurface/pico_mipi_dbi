
#include <stdatomic.h>

#include "pico/async_context_poll.h"
#include "pico/multicore.h"

#include "mgl.h"

#include "mipi.h"

/**
 *
 */
static mgl_delta_tm_T
get_time_ms (void);

static void
_mgl_init_fmbf_tx (struct mgl_gfx_ctx * ctx);

static void
_mgl_render_gfx_objs (struct mgl_gfx_ctx * ctx);

/* clang-format off */
/**
 * Number of MS per each tick.
 */
static const mgl_delta_tm_T
_MGL_TICK_TIME_MS = (1000 / MGL_EVT_TICK_PER_SEC);
/* clang-format on */

static uint32_t _ticks;

#ifndef __STDC_NO_ATOMICS__
static volatile /* _OSAL_ATOMIC_BOOL */ atomic_bool _evt_tk_running;
#else
/**
 * The mutex itself should enforce memory order consistency, but defined as
 * a volatile-qualified type anyway.
 */
static volatile _Bool _evt_tk_running;
static _osal_mutex_T _loop_stat_mtx;
#endif

static mutex_t _evt_tk_mtx, _tk_cbs_mtx;
static struct _mipi_evt_tk_ll_node * _evt_tk_cbs;

enum _mgl_async_task_type {
	MGL_REDRAW_DIRTY_FMBF_TASK,
	MGL_INIT_FMBF_TX_TASK
};

/**
 * The context for all asynchronous operations that need to be run on the
 * second core. There are a few types of work items:
 * o The user event tick loop, which is called the designated amount of
 *   times per second (see MIPI_EVT_TICK_PER_SEC) to update the state of
 *   the graphics objects.
 * o The conversion of frame buffer contents to the destination color space
 *   and subsequent transmission of frame data to the panel.
 * o The rendering of the graphics objects into the internal frame buffer
 *   after the context is marked dirty.
 * o Any user provided functions which need to be executed asynchronously
 *   and which have been registered by calling `mipi_exec_task_in_bg`.
 *
 * It is important that these tasks do not block waiting for acquisition
 * of a system resource and attempt to execute in the least time possible.
 * A blocking task will delay other tasks which need processor time; and,
 * because there is no RTOS, there is also no notion of time sharing or
 * preemption. All tasks are given equal priority in the context, and there
 * is no guarantee about the order in which they are chosen to run.
 *
 * The one exception to this the user-provided tick callbacks will always
 * run after all other tasks have been completed, assuming any are pending,
 * as they should be capable of updating state based on the time delta,
 * such that the speed of the event tick loop does not affect their result.
 */
static async_context_t * _async_ctx;
static async_when_pending_worker_t
_evt_tick_wkr[]=
{
	[MGL_REDRAW_DIRTY_FMBF_TASK]=
	{
		.do_work=_mgl_render_gfx_objs,
	},
	[MGL_INIT_FMBF_TX_TASK]=
	{
		.do_work=_mgl_init_fmbf_tx,
	},
	{
		/* SENTINEL */
	}
};

struct _mgl_evt_tk_ll_node {
	struct _mipi_evt_tk_ll_node * next;
	struct mgl_gfx_ctx * gfx_ctx;
	const mgl_evt_tick_cb evt_tk_cb;
};


static __force_inline _Bool
_is_evt_tick_running (
	void
)
{
#ifdef __STDC_NO_ATOMICS__
	_Bool
		b_lock = __osal_mutex_acquire_block_ms (&_loop_stat_mtx, MIPI_MAX_TM);
	if (!b_lock) {
		_mipi_dbg (MIPI_DBG_TAG, "synchronization error");
		return false;
	} else {
		_Bool r = _evt_tk_running;
		__osal_mutex_release (&_loop_stat_mtx);
		return r;
	}
#else
	return _evt_tk_running;
#endif
}

// OSAL_FETCH_ATOMIC_V (_is_loop_running);

/**
 * The event tick loop handles all tasks on core 1, which includes calling
 * the user-provided tick callbacks, rasterizing the frame buffer, and
 * transmitting pixel data after a frame update.
 */
static void
_mgl_evt_tick_loop (
	void
)
{
	mgl_delta_tm_T last_tm, now, delta_tm;
	struct _mgl_evt_tk_ll_node * nd;
	int64_t slp_tm; // << SIGNED int.
	_Bool b_lock;

	delta_tm = 0, last_tm = get_time_ms();
	while (_is_evt_tick_running()) {
		async_context_poll (&_async_ctx);

		now = get_time_ms();
		b_lock = mutex_try_enter (&_tk_cbs_mtx, NULL);
		if (b_lock) {
			delta_tm = (last_tm - now);
			nd = _evt_tk_cbs;
			while (nd) {
				nd->evt_tk_cb (
					nd->gfx_ctx, // << mgl_gfx_ctx_T * gfx_ctx_
					delta_tm
				);
				nd = (nd->next);
			}
			mutex_exit (&_tk_cbs_mtx);
		} else {
			/**
			 * Spin a bit until the lock is released, performing any pending
			 * tasks in the meantime.
			 */
			continue;
		}

		slp_tm = ((int64_t)(_MGL_TICK_TIME_MS)-delta_tm);
		if (slp_tm > 0) {
			sleep_ms ((uint32_t)slp_tm);
		} else {
			_mipi_dbg (
				MIPI_DBG_TAG,
				"warning: event tick loop slower than requested"
			);
		}

		last_tm = now, _ticks++;
	}
}

_Bool
mgl_suspend_evt_tick (
	void
)
{
#ifdef __STDC_NO_ATOMICS__
	_Bool
		b_lock = __osal_mutex_acquire_block_ms (&_loop_stat_mtx, MIPI_MAX_TM);
	if (b_lock) {
		_evt_tk_running = false;
		__osal_mutex_release (&_loop_stat_mtx);

		return true;
	} else {
		_mipi_dbg (MIPI_DGB_TAG, "failed to lock evt_tk core, aborting");
	}

	return false;
#else
	atomic_compare_exchange_strong_explicit (
		&_evt_tk_running,
		true,
		false,
		memory_order_release
	);
#endif
}

static void
_mgl_register_gfx_ctx (struct mgl_gfx_ctx * gfx_ctx);

static struct _mipi_evt_ll_node *
_mipi_gfx_evt_make_node();

static void
_mipi_gfx_evt_destroy_node();

/**
 * The tick callback is a client-provided function which is called to make
 * updates to the screen once per tick. Because it is called by the
 * graphics subsystem, no lock need be held on the context or frame buffer,
 * so that such updates may take place without blocking.
 *
 * It is important for users to note that the provided function is called
 * concurrently, so any state which could possibly be modified outside of
 * its scope should be carefully guarded, and is better avoided altogether.
 *
 * IF THE PANEL DEVICE NEEDS TO BE ACCESSED IN THIS CALLBACK, THE CLIENT
 * CODE MUST ENSURE IT HOLDS THIS LOCK. ACQUIRING THIS LOCK SHOULD BE A
 * NON-BLOCKING OPERATION. CALL `mipi_try_lock_dev` AND RETURN PREEMPTIVELY
 * IF THE OPERATION FAILS.
 */
void
mgl_set_evt_tick_cb (
	struct mgl_gfx_ctx * ctx,
	mgl_evt_tick_cb evt_tick_cb
)
{
	struct _mgl_evt_tk_ll_node *nd, tmp = {
		.next = _evt_tk_cbs,
		.gfx_ctx = ctx,
		.evt_tk_cb = evt_tick_cb
	};
	nd = malloc (sizeof (*nd)); // _osal_sync_malloc (...);
	if (nd) {
		memcpy (nd, &tmp, sizeof (*nd));
		/* clang-format off */
		_Bool b_lock = mutex_enter_timeout_ms (
			&_tk_cbs_mtx,
			MIPI_MAX_TM
		);
		/* clang-format on */

		if (b_lock) {
			_evt_tk_cbs = nd;
			mutex_exit (&_tk_cbs_mtx);
		} else {
			free (nd);
			_mipi_dbg (MIPI_DBG_TAG, "stalled acquiring lock for `_tk_cbs_mtx`");
		}
	} else {
		_mipi_dbg (
			MIPI_DGB_TAG,
			"failed to allocate resources for tick callback"
		);
	}
}

void
mgl_mark_fmbf_dirty (
	struct mgl_gfx_ctx * gfx_ctx
)
{
	async_context_set_work_pending (
		&_async_ctx,
		&_evt_tick_wkr[MGL_REDRAW_DIRTY_FMBF_TASK]
	);
}

void
mgl_exec_task_in_bkgd (
	mgl_bkgd_task_cb bkgd_tsk
)
{
	async_context_execute_sync (&_async_ctx, bkgd_tsk,
	                            _ticks); // <<<<
}

struct mgl_gfx_ctx
mgl_create_gfx_ctx (
	struct mipi_dbi_dev * dev,
	size_t rdr_buff_sz,
	size_t stack_sz
)
{
	struct mipi_shared_fmbf * fmbf;
	struct mgl_gfx_ctx ctx_ = {
		.gfx_nodes = NULL,
		.fmbf_bounds = { 0, 0, dev->width, dev->height },
		.gfx_fmbf = fmbf,
		.panel_dev = dev,
	};

	return ctx_;
}

void
mgl_destroy_gfx_ctx (struct mgl_gfx_ctx * self);

void
mgl_start_evt_tick_loop (
	void
)
{
	// ARGS: async_context_poll_t _async_ctx
	async_context_poll_init_with_defaults (&_async_ctx);
	for (size_t i = 0; _evt_tick_wkr[i].do_work; i++) {
		async_context_add_when_pending_worker (&_async_ctx, &_evt_tick_wkr[i]);
	}
	_evt_tk_running = ATOMIC_VAR_INIT (true);
	multicore_launch_core1 (_mgl_evt_tick_loop);
}

/**
 * Releases all allocated resources for MGL, setting the state back to
 * their inital values.
 */
void
mgl_reset_evt_tick_subsys (
	void
)
{
	_Bool b_did_end = mgl_suspend_evt_tick();
	if (!b_did_end) {
		_mipi_dbg (
			MIPI_DGB_TAG,
			"failed to terminate evt_tk loop, no reset performed"
		);

		return;
	}
	/**
	 * From this point on, there is no reason to acquire the
	 * `_evt_tk_mtx`because all code running on core 1 has stopped. It is
	 * thus safe to alter this state without locking.
	 */
	struct _mgl_evt_tk_ll_node *nd, *tmp;
	nd = _evt_tk_cbs;

	while (nd) {
		tmp = (nd->next);
		free (nd);
		nd = tmp;
	}
	_ticks = 0;

	async_context_deinit (&_async_ctx);
}

/**
 * There is no particular reason this should be its own function, besides
 * to aid in porting this library to an RTOS or another platform.
 */
static inline mgl_delta_tm_T
get_time_ms()
{
	return (mgl_delta_tm_T)(to_ms_since_boot (get_absolute_time()));
}
