
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
#include "pico/stdlib.h"
#include "ll.h"

/**
 * Maximum time a task will block attempting to take ownership of a 
 * lock.
 */
#define MIPI_MAX_TM 500 // << ms
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

#ifdef __cplusplus
extern "C" {
#endif


/******************** 
 *     Macros
 *******************/

#ifdef MIPI_DBG_EN
#define LOG_BUFF_SZ 128 // << bytes
/**
 * A note on debugging: by default, this macro will output all logs related 
 * to the MIPI DBI driver to stdout, which means that, in an application  
 * which needs to analyze the output, a necessary call to the appropriate
 * `pico_enable_stdio_*` must be performed in the Makefile.
 */
#define _mipi_dbg(tag, fmt, ...)         \
  {                                       \
    char log_buff[LOG_BUFF_SZ];           \
    snprintf(                             \
      log_buff,                           \
      LOG_BUFF_SZ,                        \
      fmt,                                \
      ##__VA_ARGS__                       \
    );                                    \
    printf("[%s]: %s \n", tag, log_buff); \
  }

#define MIPI_DBG_HDR \
  struct mipi_dbg_info_hdr hdr;
#define GET_DBG_TAG (_x_ptr) \
  (_x_ptr)->hdr->DBG_TAG

#else 
#define _mipi_dbg(tag, fmt, ...) 
#endif

#define MIPI_IO_CTR(x) (struct mipi_io_ctr *)(x)
#define MIPI_IO_CTR_PTR(x) (MIPI_IO_CTR(&x))

#define MIPI_CLR(r, g, b) \
  (struct mipi_color){    \
    r,                    \
    g,                    \
    b                     \
  }

// struct mipi_dbi_dev * dev=mipi_dbi_dev_create (
//   "panel/0",
//   width,
//   height,
//   io_ctx,
//   MIPI_DEV_ILI9341
// );
// MIPI_CHK_NOT_NULL_OR_EXIT (dev, de_init_io);
// MIPI_CHK_FLAGS_OR_EXIT (
//  dev,
//  de_init_dev
// );

#define MIPI_PRINT_ERR(dev, errno) \
  _mipi_dbg (GET_DBG_TAG (dev), err_to_str (errno));

#define MIPI_CHK_FLAGS_OR_EXIT(mipi_obj, set_jump) \
  {                                                \
    mipi_err_t errno=mipi_obj->hdr->errno;         \
    if (errno) {                                   \
      MIPI_PRINT_ERR (                             \
        dev,                                       \
        errno                                      \
      );                                           \
      goto set_jump;                               \
    }                                              \
  } 

#define _OUT_
#define _IN_


/******************** 
 * Static Prototypes
 *******************/

static size_t 
_mipi_cvt_clr_rgb565 (
  struct mipi_panel_fmt * fmt,
  _IN_ struct mipi_color clr[],
  _OUT_ u8 * clr_buff[],
  size_t buff_sz
);

/******************** 
 *      Types
 *******************/

typedef uint8_t u8;
typedef uint32_t u32;
typedef u8 mipi_dcs_cmd_t;
typedef int mipi_err_t;

struct mipi_dbg_info_hdr {
  const char * DBG_TAG;
  mipi_err_t errno;
};

/**
 * Internally, colors are represented as a 24-bit RGB tuple.
 */
struct mipi_color {
  u8 r, g, b;
};

struct mipi_area {
  uint x, y, w, h;
};

enum mipi_color_fmt {
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
 * in some source color space, is to be converted into a byte stream of an
 * appropriate format for storage in the panel frame memory.
 */
struct mipi_panel_fmt {
  MIPI_DBG_HDR;

  enum mipi_color_fmt fmt;
  const uint bpp; // << bytes per pixel, stride = WIDTH*bpp

  size_t 
  (*fmt_color)( 
    struct mipi_panel_fmt * self,
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
 * your device and to the documentation of the respective implementation for
 * information about pin declaration and assignment for the connector.
 */
struct mipi_io_ctr {
  MIPI_DBG_HDR;
  uint ctr_caps;

  /**
   * Transmits a command to the display panel. If the command has no parameters,
   * then `params` should be `NULL` and the `len` parameter should be `0`.
   */
  void 
  (*send_cmd)(
    struct mipi_io_ctr * self, 
    mipi_dcs_cmd_t cmd,
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
  size_t 
  (*recv_params)(
    struct mipi_io_ctr * self, 
    mipi_dcs_cmd_t cmd,
    _OUT_ u8 * params,
    size_t len 
  );

  /**
   * Transmits pixel data from the buffer `fmbf` to an absolute position on the
   * panel, specified by `bounds`, clipping this buffer to those bounds and the 
   * bounds of the screen, if necessary.
   */
  void 
  (*flush_fmbf)(
    struct mipi_io_ctr * self, 
    _IN_ u8 * fmbf, 
    const struct mipi_area * bounds,
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
 * and the panel initialization sequence.
 */
struct mipi_dbi_dev {
  MIPI_DBG_HDR;
  uint width, height;

  /**
   * The output format determines the binary representation of the color data 
   * sent to the panel.
   */
  struct mipi_panel_fmt out_fmt;
  struct mipi_io_ctr * io;

  /**
   * The initialization sequence for the display. Must be provided by the 
   * display manufacturer or otherwise obtained if no existing sequence is 
   * available. Sometimes certain parameters in the display initialization may 
   * differ from the default values provided here; and, if this is the case, 
   * these commands may be sent through the IO connector after the 
   * initialization has completed. (e.g. gamma correction, etc.)
   */
  const u8 * __init_seq;
};


/******************** 
 * Global Variables
 *******************/

static const char MIPI_DGB_TAG[]="mipi_dbi_spi";

static const enum mipi_color_fmt MIPI_SRC_FMT=RGB_888;
extern const struct mipi_panel_fmt MIPI_PANEL_FMT[];


/******************** 
 * Global Functions
 *******************/

extern struct mipi_dbi_dev 
mipi_create_dbi_dev (
  const char * panel_name, 
  uint width,
  uint height,
  _IN_ const u8 mipi_init_seq[]
);

extern void
mipi_init_dbi_dev ( 
  struct mipi_dbi_dev * dev,
  struct mipi_io_ctr * ctr
);

extern void 
mipi_free_dbi_dev (struct mipi_dbi_dev * dev);

extern _Bool
mipi_lock_dev_blocking (
  struct mipi_dbi_dev * dev, 
  u32 ms
);

extern _Bool
mipi_try_lock_dev (struct mipi_dbi_dev * dev);

extern struct mipi_panel_fmt *
mipi_panel_get_fmt (void);

extern _Bool
mipi_check_panel_fmt_supported (
  struct mipi_dbi_dev * dev,
  struct mipi_panel_fmt * fmt
);

/**
 * When the IFPF is changed, the entire frame buffer needs to be marked
 * invalid, as the binary represention of colors in the destination
 * pixel format is different from that which is stored in the internal
 * FMBF memory.
 */
extern void
mipi_set_panel_output_fmt (enum mipi_color_fmt fmt);


/******************** 
 * Inline Functions
 *******************/

static inline size_t 
_mipi_cvt_clr_rgb565 (
  struct mipi_panel_fmt * self,
  _IN_ struct mipi_color clr[],
  _OUT_ u8 * clr_buff[],
  size_t buff_sz )
{
  *clr_buff=calloc(buff_sz, self->bpp);
  for (size_t i=0; i<buff_sz; i++) {
    struct mipi_color c=clr[i];
    (*clr_buff)[i]  =(c.b & 0xf8)^(c.g>>5);
    (*clr_buff)[i+1]=((c.g&0x1c)<<3)^(c.r>>3);
  }
  return (self->bpp)*buff_sz;
}

static inline const char *
_mipi_err_to_str (mipi_err_t e)
{
  switch (e) {
  case ENODEV:
  default:
    return (u8 *)(&e);
  }
}

#ifdef __cplusplus
}
#endif

#endif // __MIPI_H__
