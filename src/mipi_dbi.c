
/**
 *
 * Copyright Surface EP, LLC 2025.
 */

#include "mipi.h"

const struct mipi_ifpf MIPI_PANEL_FMT[]=
{
  [RGB_565]=
  {
    .fmt=RGB_565,
    .bpp=2,
    .fmt_color=_ifpf_cvt_rgb565
  },
	/**
	 * Both `RGB_666` and `RGB_888` can be transmitted to the panel as-is due to
	 * the alignment requirements of the color components in the destination
	 * format.
	 */
  [RGB_888]=
  {
    .fmt=RGB_888,
    .bpp=3,
    .fmt_color=NULL
  }
};


struct mipi_dbi_dev
mipi_dbi_dev_create (
  const char * panel_name,
  uint width,
  uint height,
  struct mipi_color_fmt clr_fmt,
  _IN_ const u8 mipi_init_seq[] )
{
  return (struct mipi_dbi_dev)
	{
    .width=width,
    .height=height,
    .out_fmt=clr_fmt,
    .mipi_init_seq=mipi_init_seq
  };
}

void
mipi_dbi_dev_init (
  struct mipi_dbi_dev * dev,
  struct mipi_io_ctr * ctr )
{
  MIPI_CHK_NOT_NULL_OR_EXIT (dev, init_failed);
  MIPI_CHK_NOT_NULL_OR_EXIT (ctr, init_failed);

  dev->io=ctr;
  /**
   * Set output format, initialize frame buffer.
   */

  if (dev->mipi_init_seq) {
    _mipi_dcs_write_seq (
      dev->io,
      dev->mipi_init_seq
    );

    return;
  } else {
    _mipi_dbg (
      dev->DBG_TAG,
      "panel initialization sequence required, init failed"
    );
    goto init_failed;
  }

init_failed:
  dev->errno=EINVAL;
  return;
}

