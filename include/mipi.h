
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
#include "pico/stdlib.h"

#ifdef __cplusplus
extern "C" {
#endif


/******************** 
 *     Macros
 *******************/

#ifdef __MIPI_DBG_ENABLE__
#define LOG_BUFF_SZ 128
/**
 * A note on debugging: by default, this macro will output all logs related 
 * to the MIPI DBI driver to stdout, which means that, in an application  
 * which needs to analyze the output, a necessary call to the appropriate
 * `pico_enable_stdio_*` must be performed in the Makefile.
 */
#define __mipi_dbg(tag, fmt, ...) \
  { \
    char log_buff[LOG_BUFF_SZ]; \
    snprintf( \
      log_buff, \
      LOG_BUFF_SZ, \
      fmt, \
      ##__VA_ARGS__ \
    ); \
    printf("[%s]: %s \n", tag, log_buff); \
  }
#else 
#define __mipi_dbg(tag, fmt, ...) 
#endif

#define IO_CTR( x ) (struct mipi_panel_io_connector *)(x)
#define IO_CTR_PTR( x ) (IO_CTR(&x))
#define MIPI_CLR( r, g, b ) \
  (struct mipi_color){ \
    r, \
    g, \
    b \
  }

#define _OUT_
#define _IN_


/******************** 
 * Global Constants
 *******************/

extern const char MIPI_DBG_TAG[];

/**
 * ========================
 *   MIPI Dvr Error Codes
 * ========================
 */
#define EINVAL  1<<0
#define ENOMEM  1<<1
#define ENOTSUP 1<<2
#define EIO     1<<3
#define EINTR   1<<4
#define EAGAIN  1<<5
#define EWOULDBLOCK 1<<6
#define ENODEV  1<<7


/******************** 
 * Type Definitions
 *******************/

typedef uint8_t u8;
typedef uint32_t u32;


/******************** 
 *  Data Structures
 *******************/

/**
 * Internally, colors are represented as a 24-bit RGB tuple.
 */
struct mipi_color {
  u8 r, g, b;
};

struct mipi_area {
  uint x, y, w, h;
};

enum color_format {
  RGB_111, // Monochrome
  RGB_565, // 16-bit color
  /**
   * It would be pretty useless to implement support for 18-bit color, as these 
   * displays expect that each of the 6-bit color components are aligned on the
   * MSB of a single byte, and the lower two bits are "don't care" values. In 
   * this case, 24-bit color can be sent as is, as clamping would have resulted
   * in identical output on the panel.
   */
  /* RGB_666, */
  RGB_888,
  YCbCr_422,
};

static const enum color_format MIPI_SRC_FMT=RGB_888;

/**
 * ========================
 *    MIPI Panel Format
 * ========================
 * 
 * The destination pixel format, which dictates the order and stride of each
 * line in the buffer that the panel expects to receive during frame 
 * transmission over the IO connector. Any number of these formats may be 
 * supported; however, only one format is active at a time, and the panel must 
 * receive the `COLMOD` command before changing to any other.
 * 
 * The destination format shall determine how the color data is to be 
 * interpreted; that is to say, the method by which an (R,G,B) color tuple,
 * in some source color space, is to be converted into a byte stream capable of 
 * being displayed on the panel.
 */
struct mipi_dbi_panel_format {
  const char * name;
  uint bpp, fmt, bytes_req;

  size_t (*fmt_color)( 
    struct mipi_dbi_panel_format * self,
    _IN_ struct mipi_color clr[], 
    _OUT_ u8 * clr_buffer, 
    size_t buff_sz
  );
};

/**
 * ========================
 *    Panel IO Interface
 * ========================
 * 
 * An instance of this interface represents the physical hardware connection
 * between the Pico and the display panel. There are varous protocols that a
 * particular display may use, such as SPI, I2C, or an 8080 parallel bus. 
 * Refer to the display datasheet for the specific protocol(s) supported by
 * the display. 
 */
struct mipi_panel_io_connector {
  /**
   * Transmits a command to the display panel. If the command has no parameters,
   * then `params` should be `NULL` and the `len` parameter should be `0`.
   */
  void (*send_cmd)(
    struct mipi_panel_io_connector * self, 
    uint cmd,
    _IN_ u8 * params, 
    size_t len 
  );
  
  /**
   * Receives parameters from the panel. The `cmd` parameter is one of the
   * various `RD*` commands that the panel may support. The `params` buffer
   * should be pre-allocated by the caller, and the `len` parameter should
   * be the size of this buffer. The function returns the number of bytes
   * received from the panel.
   * 
   * If the `params` buffer is not large enough to hold the received data,
   * then the function returns the trunkated data and sets the `errno` value
   * to `ENOMEM`.
   * 
   * Do note that not all displays support reading data from the panel, and
   * if this is the case, then the IO connector should return `0` and set the
   * `errno` value to `ENOTSUP`.
   */
  size_t (*recv_params)(
    struct mipi_panel_io_connector * self, 
    uint cmd,
    _OUT_ u8 * params,
    size_t len 
  );

  /**
   * Transmits pixel data from the buffer `fmbf` to an absolute position on the
   * panel, specified by `bounds`, clipping this buffer to those bounds and the 
   * bounds of the screen, if necessary.
   */
  void (*flush_fmbf)(
    struct mipi_panel_io_connector * self, 
    _IN_ u8 * fmbf, 
    const struct mipi_area bounds,
    size_t len 
  );
};

/**
 * ========================
 *  MIPI DBI Panel Device 
 * ========================
 * 
 * An instance of this structure represents a display panel that is compatible
 * with the MIPI Display Bus Interface (DBI) standard. The structure contains
 * information about the panel, such as its name, resolution, color formats,
 * and the initialization sequence required to bring the panel into a usable
 * state.
 */
struct mipi_dbi_panel_device {
  const char * name;
  size_t nfmts, init_seq_sz;
  uint width, height;
  /**
   * Color formats the panel is capable of displaying.
   */
  struct mipi_dbi_panel_format out_fmt, * fmt_list;
  struct mipi_panel_io_connector * io;
  // struct mipi_fmbf * fmbf;
  /**
   * Set when the panel is in an invalid state due to incompatibility with a 
   * request made or system error. 
   */
  int errno;
  /**
   * The initialization sequence for the display. Must be provided by the
   * display manufacturer, or otherwise obtained if no existing sequence
   * is available. Sometimes certain parameters in the display initialization
   * may differ from the default values provided here; and, if this is the 
   * case, these commands may be sent through the IO connector after the 
   * initialization has completed. (e.g. gamma correction, etc.)
   */
  u8 __init_seq[];
};


/******************** 
 * Global Functions
 *******************/

extern void
mipi_panel_dev_init( 
  struct mipi_dbi_panel_device * dev,
  struct mipi_panel_io_connector * ctr
);

extern void 
mipi_panel_dev_free( struct mipi_dbi_panel_device * dev );

extern struct mipi_dbi_panel_format *
mipi_panel_get_fmt( void );

extern void
mipi_panel_set_output_fmt( struct mipi_dbi_panel_format * fmt );


/******************** 
 * Inline Functions
 *******************/


#ifdef __cplusplus
}
#endif

#endif // __MIPI_H__
