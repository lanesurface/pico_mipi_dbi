/**
 * ========================
 *         osal.h
 * ========================
 *
 * Operating System Abstraction Layer
 *
 * Refer to the documentation for information on the types necessary to
 * define and the behavior of ported functions. This OSAL is intended to
 * abstract the HAL and runtime infrustructure of an embedded system. Typically
 * an implementation will target an RTOS and/or platform-specific SDKs. It may
 * be possible to implement limited support for a kernel such as Linux, but
 * that is neither the intention of this abstraction nor a reasonable target.
 *
 * Author(s): Lane W Surface
 * Created: 	01/23/2025
 * License: 	MIT
 *
 * Copyright Surface EP, LLC 2025.
 */

#include <stdbool.h>
#include _PF_TYPE_DEFNS // Defined in the build system per platform target.

/**
 * <<WARNING>>
 * The function definitions need to be provided for all uses of a given
 * feature; and, as such, are sorted into dependency categories. For each of
 * these categories, a function may rely on the implementation of any of the
 * others. No link-time errors will be emitted because the declarations
 * provided are weakly defined; however, runtime errors will occur when a call
 * is made to a function for which no implementation has been provided. This
 * may lead to seemingly arbitrary errors which are difficult to debug, so
 * please be cognisant of this requirement.
 */
#ifdef __GNUC__ || __clang__ // Clang also defines __GNUC__, left for visibility
# define _WEAK_DEF __attribute__((weak))
#else
# ifdef __IAR__
#  define _WEAK_DEF __weak
# else
#  error Unsupported Compiler/Toolchain
# endif
#endif


// typedef mutex_t _osal_mtx_T;
// typedef spi_inst_t _osal_spi_dev_T;
// typedef const uint8_t _osal_gpio_pin_T;

// typedef spi_inst_t * _osal_base_spi_dev_T;

/**
 * On some platforms, DMA is integrated as a part of peripheral interfaces (ie,
 * there is no user-configurable DMA controller availble). As such, all memory
 * interacting with a controller should be aligned on the bounds of a `uint32_t`
 * and, if configurable, the transfers should always be set to this size.
 */
#define _DMA_MEM_ATTR __attribute__((aligned(4)))

struct dma_mem {
	/**
	 * In C, `_Alignas` may not be used on the declaration of an aggregate type,
	 * so use GCC attribute for this which does.
	 */
	union _DMA_MEM_ATTR {
		uint32_t dma_mem_al;
		/**
		 * It's possible that `dma_mem_al` has a stricter alignment than its type
		 * dictates; inherit this requirement. Note that use of `_Alignof` with an
		 * expression is supported by some compilers but ultimately a non-standard
		 * feature.
		 */
		_Alignas (_Alignof (typeid(dma_mem_al)))
		uint8_t dma_buff[sizeof(dma_mem_al)];
	};
};


#ifdef __STDC_HAS_THREADS__
typedef mtx_t    mipi_osal_mtx_T;
typedef thrd_t * mipi_osal_thrd_handle_T;
#endif

/**
 * The standard library functions are guaranteed thread-safe by their
 * implementation in the Pico SDK.
 */
#define mipi_osal_alloc   malloc
#define mipi_osal_calloc  calloc
#define mipi_osal_realloc realloc
#define mipi_osal_free    free

/**
 * dma_mem_T _dma_buff[<const_expr>];
 * for (size_t i=0; i<countof(_dma_buff); i++)
 *  for (size_t j=0; j<DMA_ELEM_SZ; j++)
 *   _dma_buff[(void)i,j]=<r_val>;
 */

/**
 * `_PACKED_AG` should be applied to all library structures whose memory
 * footprint can be significantly reduced by packing its members.
 */
#define _PACKED_AG __attribute__(( \
		__packed__,                    \
		designated_init                \
	))

#ifndef __force_inline
# define __force_inline inline __attribute__((always_inline))
#endif

#ifndef __isr
# define __isr __attribute__((interrupt_handler))
#endif


/**
 * Please, where necessary, prefer the use of fixed-width integer types so
 * that the interface for this library remains as platform agnostic as
 * possible, and to ease the porting of it to other MCU and ISA.
 */

struct _osal_stm_bus {
	size_t
	(*write_block_ms)(
		struct _osal_io_dev * dev,
		/*_IN_*/ uint8_t in_buff[],
		size_t in_buff_sz,
		uint32_t ms
	);

	size_t
	(*read_block_ms)(
		struct _osal_io_dev * dev,
		/*_OUT*/ uint8_t out_buff[],
		size_t out_buff_sz,
		uint32_t ms
	);
};

typedef struct _osal_stm_bus osal_stm_bus_T; // static osal_stm_bus_T * _usb_bus[];

struct _hid_ptc_dsc;
typedef struct _hid_ptc_dsc hid_ptc_dsc_T;
/*
 * How difficult would it be to support the display over USB?
 */

_WEAK_DEF extern void
_osal_init_gpio_pin (
	const _osal_gpio_pin_T pin,
	int _pf_caps /* The function of this parameter depends
	                on the implementation logic. */
);

_WEAK_DEF extern void
_osal_set_gpio_pin_state (
	_osal_gpio_pin_T pin,
	const _Bool pin_val
);

/**
 * <<SPI>>
 */

_WEAK_DEF extern void
_osal_init_spi_dev ();

_WEAK_DEF extern _Bool
_osal_spi_read_block_ms (
	struct _osal_spi_dev * spi_dev,
	/*_OUT*/ uint8_t byte_arr[],
	size_t num_bytes,
	uint32_t ms
);

_WEAK_DEF extern _Bool
_osal_spi_write_block_ms (
	struct _osal_spi_dev * spi_dev,
	/*_IN_*/ const uint8_t byte_arr[],
	size_t num_bytes,
	uint32_t ms
);

/**
 * <<MGL>>
 */

_WEAK_DEF extern mipi_osal_mtx_T
_osal_create_mutex (void);

_WEAK_DEF extern _Bool
_osal_try_lock_mtx (mipi_osal_mtx_T * osal_mtx);

_WEAK_DEF extern _Bool
_osal_lock_mtx_block_ms (
	mipi_osal_mtx_T * osal_mtx,
	uint32_t ms
);

_WEAK_DEF extern uint32_t
_osal_get_time_ms (void);
