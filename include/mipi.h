
/**
 * ========================
 *         mipi.h
 * ========================
 *
 * A MIPI-compatible display is one which implements one of the three [1], [2],
 * and/or [3] specifications for display interfacing developed by the MIPI
 * Alliance. Of these, many options are available, which have different
 * communication requirements.
 *
 * This library concerns itself with the MIPI type C, DBI interface, which
 * is often used on small-resolution matrix displays capable of being driven
 * by a microcontroller. A DBI display itself may have many options for its IO
 * link, such as over SPI, I2C, or an 8080 parallel bus, and so a flexible
 * mechanism for communication with the panel is also required.
 *
 * Many of the most common display controllers ("drivers") have implementations
 * here, which may serve as a starting point; however, certain parameters are
 * manufacturer-specific, and thus need to be provided after initialization of
 * the  device, and before data transmission begins.
 *
 * Author(s): Lane W Surface
 * Created:   2025-01-06
 * License:   MIT
 *
 * Copyright Surface EP, LLC 2025.
 */

#ifndef __MIPI_H__
#define __MIPI_H__

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <pico/stdlib.h>
#include "_pf_osal/osal.h"

#include "bbuff.h"

/**
 * Maximum time a task will block attempting to take ownership of a
 * lock if no timeout is specified.
 */
#define MIPI_MAX_TM      500 // << ms
#define MIPI_CMD_BUFF_SZ 32

/**
 * ========================
 *   MIPI Dvr Error Codes
 * ========================
 */
#define MIPI_ERR_INV         (mipi_err_T)(1<<0)
#define MIPI_ERR_NO_MEM      (mipi_err_T)(1<<1)
#define MIPI_ERR_OP_NOT_IMPL (mipi_err_T)(1<<2)
#define MIPI_ERR_IO          (mipi_err_T)(1<<3)
#define MIPI_ERR_INTERRUPT   (mipi_err_T)(1<<4)
#define MIPI_ERR_RES_LOCKED  (mipi_err_T)(1<<5)

#ifdef __cplusplus
extern "C" {
#endif


/********************
 *     Macros
 *******************/

#ifdef MIPI_DBG_EN
#define LOG_BUFF_SZ 512 // << bytes
/**
 * A note on debugging: by default, this macro will output all logs related
 * to the MIPI DBI driver to stdout, which means that, in an application
 * which needs to analyze the output, a necessary call to the appropriate
 * `pico_enable_stdio_*` must be performed in the Makefile.
 */
#define _mipi_dbg(_tag, _fmt, ...)         \
  {                                        \
    char log_buff[LOG_BUFF_SZ];            \
    snprintf (                             \
      log_buff,                            \
      LOG_BUFF_SZ,                         \
      _fmt,                                \
      ##__VA_ARGS__                        \
    );                                     \
    printf (                               \
      "[%s] in %s, line no. <%d>: %s \n",  \
      _tag, __FILE__, __LINE__, log_buff   \
    );                                     \
  }
#else
#define _mipi_dbg(tag, fmt, ...)
#endif

#define SET_FLAGS(_arg, ...) \
	_arg | __VA_OPT__(|) __VA_ARGS__


// struct mipi_screen_geom screen_geom={
//   ...
// };
// mipi_dev_handle_T dev=mipi_dbi_dev_create (
//   "panel/0",
//   screen_geom,
//   io_ctx,
//   MIPI_DEV_ILI9341
// );
// MIPI_CHK_NOT_NULL_OR_EXIT (dev, de_init_io);
// MIPI_CHK_FLAGS_OR_EXIT (
//  dev,
//  de_init_dev
// );

#define MIPI_CHK_FLAGS_OR_EXIT(_mipi_obj, _set_jmp) \
  {                                                 \
    if (mipi_err_code) {                            \
      _mipi_dbg (                                   \
        MIPI_DBG_TAG,                               \
        _MIPI_ERR_STRING[mipi_err_code]             \
      );                                            \
      goto _set_jmp;                                \
    }                                               \
  }

/**
 * For object pointers which are returned by a function and require no explicit
 * allocation/deallocation, they should be marked with tag `_OUT` to specify
 * that the memory thereof is managed outside the caller's scope. For many of
 * the functions which provide this guarantee, they take an explicit context in
 * which these objects are managed and tracked, and the client code must ensure
 * that it destroys that context to release any resources which may have been
 * consumed by it.
 */
#define _OUT
/**
 * All objects which are passed into a library function and which are allocated
 * in the caller's scope, and for which there is no provided guarantee about the
 * behavior of the requested operation if this data should be modified or freed
 * before the operation completes should use this tag to specify that these
 * constraints must be abided by.
 */
#define _IN
/**
 * Objects used in contexts where the underlying data may be modified in the
 * process of the requested operation should mark parameters with this tag to
 * indicate that it safe to do so, as the function will make a copy of the
 * object or a part of the object that is used in the callee scope.
 */
#define _COPY_FROM_USER
/**
 * Objects allocated by a user and which library functions copy data to, and
 * there is the potential for user-code to modify, and which the context has
 * need to guarantee the consistency of the data should be marked with this tag
 * to guarantee this constraint. The use of this tag has no usual relevance to
 * calling code, so it should only serve as a reminder for the implementer of
 * the operation.
 */
#define _COPY_TO_USER


/********************
 * Static Prototypes
 *******************/

static size_t
_mipi_cvt_clr_rgb565 (
  struct mipi_ifpf * fmt,
  _IN struct mipi_color clr[],
  _OUT uint8_t * clr_buff[],
  size_t buff_sz
);

/********************
 *      Types
 *******************/

 #ifndef ssize_t
 #define ssize_t int64_t // [0..2^(64-1)]
 #endif

typedef uint8_t u8;
typedef uint32_t u32;
typedef uint8_t mipi_dcs_cmd_T;
typedef ssize_t mipi_err_T;
typedef int8_t mipi_dev_handle_T;

/**
 * Internally, colors are represented as a 24-bit RGB tuple.
 */
struct mipi_color {
	union {
		struct {
			uint8_t r, g, b;
		};
		struct {
			uint8_t h, s, l; // h: [0..360], which exceeds the capacity of a byte.
		} hsl;
	};
};

struct mipi_clr_rgb_565 {
	uint16_t
	/*BITFIELD*/
	r : 5,
	g : 6,
	b : 5;
};

// x*(1-a)+y*a

static __force_inline uint8_t
rgb_blend_over_alpha (); // TODO

struct mipi_area {
  uint16_t x, y, w, h;
};

enum mipi_color_fmt {
  MIPI_CLR_FMT_MONO,
  MIPI_CLR_FMT_RGB_565, // 16-bit color
  /**
   * It would be pretty useless to implement support for 18-bit color, as these
   * displays expect that each of the 6-bit color components are aligned on the
   * MSB of a single byte, the lower two bits of which are "don't care" values.
	 * In these cases, 24-bit color can be sent as is, as clamping would result
	 * in identical output.
   */
  MIPI_CLR_FMT_RGB_666,
  MIPI_CLR_FMT_RGB_888,
  MIPI_CLR_FMT_YCBCR_422,
	MIPI_CLR_FMT_HSV_32
};

/**
 * ========================
 *   Interface Pixel Fmt
 * ========================
 *
 * The destination pixel format, which dictates the order and stride of
 * each line in the buffer that the panel expects to receive during frame
 * transmission over the IO connector. Any number of these formats may be
 * supported; however, only one format may be active at a given time. See
 * `mipi_set_panel_output_ifpf` for details about specifying these parameters.
 *
 * The destination format shall determine how the color data is to be
 * interpreted; that is to say, the method by which a color tuple, in some
 * source color space, is converted into a byte stream in an appropriate format
 * for storage in panel frame memory.
 */
struct mipi_ifpf {
  enum mipi_color_fmt in_clr_fmt;
  const uint8_t bytes_per_px, stride; // Padding may cause stride to differ.

  size_t
  (*cvt_to_ifpf)(
    struct mipi_ifpf * self,
    _IN struct mipi_color clr_arr[],
    _OUT uint8_t out_clr_buff[],
    size_t num_clr_elems
  );
};


typedef uint8_t mipi_evt_class_T;
struct _mipi_evt {
	void
	(*mipi_evt_cb)(
		mipi_evt_class_T mipi_evt_cls,
		void * params[],
		size_t num_params
	);
};

typedef struct _mipi_evt mipi_evt_T;
/**
 * In order to prevent reliance on any one particular OS, the client should
 * provide a mechanism for notifying the MIPI context about a system tick
 * interrupt. This will be used to synchronize different threads of execution.
 */
typedef void
mipi_tick_cb (uint32_t tick); // TODO

/**
 * ========================
 *    Panel IO Interface
 * ========================
 *
 * An instance of this interface represents the physical hardware connection
 * between the MCU and the display panel. There are varous protocols that a
 * particular display may use, such as SPI, I2C, or an 8080 parallel bus.
 * Refer to the display datasheet for the specific protocol(s) supported by
 * your device and to the documentation of the respective implementation for
 * information about valid pin assignments for the connector.
 */
struct mipi_io_ctr {
	uint8_t
	/*BITFIELD*/
	can_rd     : 1,
	can_wt     : 1,
	rd_in_prog : 1,
	wt_in_prog : 1;

	/**
	 * Writes the given register to the command buffer. If the command has no
	 * parameters, then `params` should be `NULL` and the `len` parameter should
	 * be `0`.
	 */
  void
  (*write_panel_reg)(
    struct mipi_io_ctr * self,
    mipi_dcs_cmd_T reg,
    _IN const uint8_t params[],
    size_t num_params
  );

  /**
   * Reads the given panel register into the `params` buffer, assuming one of
	 * the various `RD*` commands, as defined in the DCS.
   *
   * If the `params` buffer is not large enough to hold the received data,
   * then the function returns the trunkated data and sets `MIPI_ERR_NO_MEM`.
   *
   * If the display is write-only, this function returns `-1` and
	 * `MIPI_ERR_NOT_SUP` is set appropriately.
   */
  ssize_t
  (*read_panel_reg)(
    struct mipi_io_ctr * self,
    mipi_dcs_cmd_T reg,
    _OUT uint8_t params[],
    size_t num_params
  );

  /**
   * Transmits pixel data from the buffer `ptl_fmfb_data` to an absolute
	 * position on the panel specified by `fmbf_dst_bounds`, clipping this
	 * buffer to those bounds and the bounds of the screen, if necessary.
   */
  void
  (*flush_fmbf)(
    struct mipi_io_ctr * self,
    _IN uint8_t ptl_fmbf_data[],
    const struct mipi_area fmbf_dest_bds,
    size_t fmbf_sz
  );
};

/**
 * ========================
 *  MIPI DBI Panel Device
 * ========================
 *
 * A device conformant with the MIPI DBI standard.
 */
struct mipi_dbi_dev {
	// TODO: Allow for device-independent positioning using a unit system
	// (precision yet to be specified) based on the physical dimensions of the
	// display and the resolution thereof.
  uint width, height, ppi;

  /**
   * The output interface pixel format (IFPF) determining the form of the
	 * binary color data the panel requires for display of the frame buffer.
   */
  struct mipi_ifpf dst_ifpf;
  struct mipi_io_ctr * io;

  /**
   * The initialization sequence for the display. Must be provided by the
   * display manufacturer or otherwise obtained if no existing sequence is
   * available. Sometimes certain parameters in the display initialization may
   * differ from the default values provided here; and, if this is the case,
   * these commands may be sent through the IO connector after the
   * initialization has completed. (e.g. for gamma correction, etc.)
   */
  const uint8_t * panel_init_seq;
};


/********************
 * Global Variables
 *******************/

static const enum mipi_color_fmt MIPI_SRC_FMT=(MIPI_CLR_FMT_RGB_888);
extern const struct mipi_ifpf MIPI_PANEL_FMT[];
/**
 * Provides similar function to the POSIX `errno` global. When any operation
 * fails, a client may catch the reason by interpreting this code.
 */
extern const char * _MIPI_ERR_STRING[];
volatile MIPI_OSAL_ATOMIC_INT mipi_err_code;


/********************
 * Global Functions
 *******************/

extern mipi_dev_handle_T
mipi_create_dbi_dev (
  const char * panel_name,
  uint width,
  uint height,
  _IN const uint8_t mipi_init_seq[]
);

extern mipi_err_T
mipi_init_dbi_dev (
  struct mipi_dev_handle_T dev,
  struct mipi_io_ctr * ctr
);

extern mipi_err_T
mipi_free_dbi_dev (struct mipi_dev_handle_T dev);

extern _Bool
mipi_lock_dev_blocking (
	mipi_dev_handle_T dev_panel_hdl,
  _OUT struct mipi_dbi_dev ** dbi_dev,
  uint32_t ms
);

extern _Bool
mipi_try_lock_dev (
	mipi_dev_handle_T dev_panel_hdl,
	_OUT struct mipi_dbi_dev ** dbi_dev
);

extern struct mipi_ifpf
mipi_panel_get_ifpf (mipi_dev_handle_T mipi_panel_hdl);

/**
 * When the IFPF is changed, the entire frame buffer needs to be marked
 * invalid, as the binary represention of colors in the destination
 * pixel format is different from that which is stored in the internal
 * frame buffer.
 */
extern mipi_err_T
mipi_set_panel_output_ifpf (
	mipi_dev_handle_T mipi_panel_hdl,
	enum mipi_color_fmt fmt
);


/********************
 * Inline Functions
 *******************/

static __force_inline size_t
_ifpf_cvt_rgb565 (
  struct mipi_ifpf * self,
  _IN struct mipi_color clr[],
  _OUT u8 * clr_buff[],
  size_t buff_sz )
{
	(*clr_buff)=calloc (
		buff_sz*(self->bytes_per_px),
		1
	);
  for (size_t i=0; i<buff_sz; i++) {
    struct mipi_color c=clr[i];
    (*clr_buff)[i]  =(c.b&0xf8)^(c.g>>5);
    (*clr_buff)[i+1]=((c.g&0x1c)<<3)^(c.r>>3);
  }
  return sizeof(*clr_buff);
}

#ifdef __cplusplus
}
#endif

#endif // __MIPI_H__
