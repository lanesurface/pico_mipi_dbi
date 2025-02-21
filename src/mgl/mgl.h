
/**
 * ========================
 *       mipi_gfx.h
 * ========================
 *
 * The MIPI Graphics Library (MGL) is a simple and bare-bones 2D graphics layer
 * providing primitive draw operations (line, rect, arc, etc) for MIPI displays
 * driven by this library. Its primary design goal is memory and thread safety,
 * as a trade off for speed. Most dynamically allocated objects are managed by
 * the graphics context, such that the user must only make a call to destroy
 * their respective context to release these resources.
 *
 * Refer to the documentation for information about the restrictions on creating
 * a 2D context, registering event callbacks, and where and how to acquire the
 * necessary locks.
 *
 * Author(s): Lane W Surface
 * Created:   2025-01-10
 * License:   MIT
 *
 * Copyright Surface EP, LLC 2025.
 */

#ifndef __MIPI_GFX__
#define __MIPI_GFX__

#include <pico/mutex.h>
#include "mipi.h"


#define MGL_EVT_TICK_PER_SEC 60
#define MGL_FMBF_SZ          2048 // << bytes
#define MGL_GFX_STACK_SZ     256  // (8+8*N)*M


#ifdef __cplusplus
extern "C" {
#endif


/********************
 *      Types
 *******************/

typedef const ssize_t mgl_obj_handle_T;
typedef uint32_t mgl_delta_tm_T;

/**
 * Instead of calcuating the points making up the line segments of the
 * shape and storing these points, a more memory-efficient representation
 * can be acheived by storing shape-specific properties and calculating
 * the line-segments at runtime. This may be fairly slow.
 */
enum mgl_obj_type {
  MGL_PT,
  MGL_LINE,
  MGL_POLY_LINE,
  MGL_ARC,
  MGL_CIRCLE,
  MGL_TRIANGLE,
  MGL_TRAPEZOID,
  MGL_BEZIER_CURVE, // Curve of order N, _mgl_pt pt_arr[N]
  MGL_GEN_POLYGON
};

struct _mgl_pt {
  uint x, y;
};

struct mgl_gfx_obj {
  const enum mgl_obj_type obj_type;
  _Bool fill_obj;
  const size_t n_pts;
  struct _mgl_pt pt_arr[];
};

struct mipi_shared_fmbf {
  const size_t fmbf_sz;
  mutex_t clr_buff_mtx; // struct rw_lock buff_lk;
  volatile struct mipi_color clr_buff[];
};
// THREAD 1:
// volatile int f,x; // volatile ensures that compiler optimizations do not
// // reorder accesses of volatile qualified types wrt themselves and their
// // value is not cached by the cpu
// while (f==0)
//  tight_loop_contents ();
// __dmb ();
// printf ("%d\n", x);
//
// THREAD 2:
// x=42; // << sequence point
// __asm volatile ("dbm"); // prevent the CPU from reordering modifications
// // of `f` and `x` wrt each other.
// f=1;

struct _mgl_evt_tk_ll_node;
/**
 * For each graphics context, client code may register a callback which is
 * called once per tick to update graphics state and handle any other
 * necessary operations (see `mgl_set_evt_tick_cb`).
 */
typedef void
(*mgl_evt_tick_cb)(
  struct mgl_gfx_ctx * ctx,
  const mgl_delta_tm_T tk_delta
);

typedef uint32_t
(*mgl_bkgd_task_cb)(void * tsk_prm);

struct _mgl_obj_ll_node;

struct mgl_gfx_ctx {
  struct mipi_area fmbf_bounds;
  struct mipi_dbi_dev * panel_dev;
  /**
   * Each entry in the object stack is an object node, which consists of a
   * linked list of `_mgl_pt` objects to be joined in an anti-clockwise order
   * by line segments to form the final shape.
   *
   * It should be noted that the context is responsible for maintaining the
   * order of these objects, such that the last entry in the stack has the
   * highest z-index, so as to give the appearance of proper depth when
   * rendered from first to last.
   */
  struct _mgl_obj_ll_node * gfx_nodes[MGL_GFX_STACK_SZ];
  struct mipi_shared_fmbf gfx_fmbf;
};


/********************
 * Global Functions
 *******************/

extern struct mgl_gfx_ctx
mgl_create_gfx_ctx (
  struct mipi_dbi_dev * dev,
  size_t rdr_buff_sz,
  size_t stack_sz
);
/**
 * static mipi_dev_handle_T _panel;
 * ...
 * _panel=mipi_create_panel (...);
 * mipi_init_dbi_dev (_panel, mipi_create_spi_ctr (
 *   SPI_PORT,
 *   miso,
 *   mosi,
 *   sck,
 *   cs
 * ));
 * // NOTHING to do with the panel device.
 * // Need some config structure `mipi_dbi_panel_cfg_T`, which
 * // has device properties.
 *
 * **** NEED AN ISR FOR CALLBACK WHEN TX NEEDS TO OCCUR, SHARED FMBF ****
 *
 * mgl_draw_rect (_ctx, 0, 0, w, h);
 */

extern void
mgl_start_evt_tick_loop (void);

extern _Bool
mgl_suspend_evt_tick (void);

extern void
mgl_set_evt_tick_cb (
	struct mgl_gfx_ctx * ctx,
	mgl_evt_tick_cb evt_tick_cb
);

/**
 * Because the MGL holds control over core 1, client code which needs to run
 * asynchronously must call this function to register such a task so that
 * it does not compete with the rendering task for control of the core. There
 * are no guarantees about when the task is scheduled to run or the order
 * that background tasks will execute.
 *
 * This function may block in order to aquire a lock on the async context; if
 * the async context is unlocked, it shall return immediately.
 */
extern void
mgl_exec_task_in_bkgd (mgl_bkgd_task_cb bkgd_cb);

extern void
mgl_ctx_set_render_buffer (
  struct mgl_gfx_ctx * self,
  _IN struct mipi_shared_fmbf * out_buff,
  size_t n
);

/**
 * Modifies the extents of the render buffer, notifying consumers of the
 * affected context. The output of the graphics context is first scaled by the
 * scale factor and then rasterized into the frame buffer assuming a screen of
 * these dimensions. Usually a call to this function, providing the dimensions
 * of the panel, should occur before any objects are added to the context to
 * avoid needless and expensive draw operations.
 */
extern void
mgl_set_screen_bounds (
  struct mgl_gfx_ctx * self,
  uint width,
  uint height
);

extern void
mgl_set_scale (
  struct mgl_gfx_ctx * self,
  float scale
);

extern mgl_obj_handle_T
mgl_create_pt ();

extern mgl_obj_handle_T
mgl_create_line ();

extern mgl_obj_handle_T
mgl_create_rect ();

extern mgl_obj_handle_T
mgl_create_arc ();

extern mgl_obj_handle_T
mgl_create_trangle ();

/**
 * Remove the graphics object with handle `hdl_` from this context.
 *
 * NOTE: If this function is called outside of an event tick callback,
 * the caller MUST hold the object's lock.
 */
extern void
mgl_destroy_gfx_obj (
  struct mgl_gfx_ctx * ctx_,
  mgl_obj_handle_T hdl_
);

extern _Bool
mgl_try_lock_gfx_obj (
  struct mgl_gfx_ctx * self,
  mgl_obj_handle_T hdl,
  _OUT struct mgl_gfx_obj * out_obj
);

extern _Bool
mgl_lock_gfx_obj_timeout_ms (
  struct mgl_gfx_ctx * self,
  mgl_obj_handle_T hdl,
  _OUT struct mgl_gfx_obj * out_obj,
  const uint32_t tm
);

extern void
mgl_unlock_gfx_obj (
  struct mgl_gfx_ctx * self,
  _IN struct mgl_gfx_obj * owned_obj
);

extern void
mgl_clear_screen (struct mgl_gfx_ctx * self);

// extern void
// mipi_gfx_sub_event (
//   struct mipi_gfx_ctx * self,
//   enum mipi_gfx_event_type ev_type,
//   mgl_evt_tick_cb cb
// );

extern struct mipi_shared_fmbf *
mgl_create_shared_fmbf (
  size_t n,
  struct mipi_ifpf p_fmt /* << DESTINATION IFPF */
);

extern _Bool
mgl_try_lock_fmbf ();

extern void
mgl_lock_fmbf_timeout_ms ();

extern void
mgl_unlock_shared_fmbf ();


#ifdef __cplusplus
}
#endif

#endif // __MIPI_GFX__
