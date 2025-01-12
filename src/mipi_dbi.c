
/**
 * 
 * Copyright Surface EP, LLC 2025.
 */

#include "mipi.h"


static const struct mipi_panel_fmt MIPI_FMT_RGB565=
  (struct mipi_panel_fmt) {
    .DBG_TAG="mipi_fmt_rgb565",
    .fmt=RGB_565,
    .bpp=2,
    .fmt_color=__mipi_color_conv_rgb565
  };

static const struct mipi_panel_fmt MIPI_FMT_RGB888=
  (struct mipi_panel_fmt) {
    .DBG_TAG="mipi_fmt_rgb888",
    .fmt=RGB_888,
    .bpp=3,
    .fmt_color=NULL
  };

const struct mipi_panel_fmt MIPI_PANEL_FMT[]=
{
  NULL,
  MIPI_FMT_RGB565,
  MIPI_FMT_RGB888,
};


struct mipi_dbi_dev 
mipi_dbi_dev_create ( 
  const char * panel_name,
  uint width,
  uint height,
  struct mipi_color_fmt clr_fmt,
  _IN_ const u8 mipi_init_seq[] )
{
  return (struct mipi_dbi_dev) {
    .DBG_TAG=panel_name,
    .width=width,
    .height=height,
    .out_fmt=clr_fmt,
    .__init_seq=mipi_init_seq
  };
}

void 
mipi_dbi_dev_init (
  struct mipi_dbi_dev * dev, 
  struct mipi_io_ctr * ctr )
{
  if (dev==NULL) {
    __mipi_dbg (
      MIPI_DBG_TAG, 
      "no DBI device provided, aborting.."
    );
    
    return;
  }

  if (ctr==NULL) {
    __mipi_dbg (
      dev->DBG_TAG,
      "invalid IO connector supplied during init, aborting.."
    );
    dev->errno|=EINVAL;
    
    return;
  }

  dev->io=ctr;
  /**
   * Set output format, initialize frame buffer.
   */

  if (dev->__init_seq) {
    __mipi_dcs_write_seq (
      dev->io, 
      dev->__init_seq
    );
  } else {
    __mipi_dbg (
      dev->DBG_TAG,
      "panel initialization sequence required, init failed"
    );
    dev->errno|=EINVAL;
    return;
  }
}