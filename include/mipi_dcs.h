/**
 * 
 * Copyright Surface EP, LLC 2025.
 */

#ifndef __MIPI_DCS__
#define __MIPI_DCS__

#include "mipi.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * ========================
 *  MIPI DCS Instructions
 * ========================
 * 
 * Refer to the MIPI Display Command Set [1] specifications for additional
 * information about the function of these commands and their parameters. 
 * Additionally, some parameters are hardware-specific, and thus must be
 * obtained from the display manufacturer or otherwise.
 */
#define NOP 0x00
#define SWRST 0x01
#define RDDID 0x04
#define RDDST 0x09
#define RDDPM 0x0A
#define RDDMADCTL 0x0B
#define RDDCOLMOD 0x0C
#define RDDIM 0x0D
#define RDDSM 0x0E
#define RDDSDR 0x0F
#define SLPIN 0x10
#define SLPOUT 0x11
#define PTLON 0x12
#define NORON 0x13
#define INVOFF 0x20
#define INVON 0x21
#define GAMSET 0x26
#define DISPOFF 0x28
#define DISPON 0x29
// <<GFX Memory Buffer Write>>
// RAMWR data[xi][yi], 
//   data[xi+1][yi],
//   ..., 
//   data[xf-1][yf-1];
#define RAMWR 0x2C
#define RGBSET 0x2D
#define RAMRD 0x2E
#define PTLAR 0x30
#define VSCRDEF 0x33
#define TEOFF 0x34
#define TEON 0x35
// <<Memory Access Ctl>>
// MADCTL <MX,MY, // mirror x, mirror y
//   MV,ML, // xchange rows/columns, scan direction
//   RGB, // 0=RGB, 1=BGR
//   0,0>; 
#define MADCTL 0x36
#define CASET 0x2A
#define RASET 0x2B
#define VSCRSADD 0x37
#define IDMOFF 0x38
#define IDMON 0x39
#define COLMOD 0x3A
#define RAMWRC 0x3C
#define RAMRDC 0x3E
#define TESCAN 0x44
#define DESSEL 0x45
#define GSCAN 0x46
#define DGPDR 0x4A
#define GAMCTRP 0x4D
#define GAMCTRN 0x4E
#define FRMCTRL1 0xB1
#define FRMCTRL2 0xB2
#define FRMCTRL3 0xB3
#define INVCTRL 0xB4
#define PWCTRL1 0xC0
#define PWCTRL2 0xC1
#define PWCTRL3 0xC2
#define PWCTRL4 0xC3
#define PWCTRL5 0xC4
#define VMCTRL1 0xC5
#define VMOFCTRL 0xC7
#define WRID2 0xD1
#define WRID3 0xD2
#define NVFCTRL1 0xD9
#define NVFCTRL2 0xDE
#define NVFCTRL3 0xDF

#define MIPI_DELAY (u8) 1<<7
#define END_DCS_SEQ NOP

/**
 * ========================
 *  Interface Pixel Format 
 * ========================
 */
#define IFPF_16_BIT 0x05 /* RGB_565 */
#define IFPF_18_BIT 0x06 /* RGB_888 */
#define IFPF_24_BIT      /* RGB_888 */

/**
 * ========================
 *   Memory Address Ctrl
 * ========================
 */
#define MX      1<<7
#define MY      1<<6
#define SWAP_XY 1<<5
#define BGR     1<<5
#define RGB     0<<5

struct mipi_dcs_cmd {
  uint code_pt;
  ssize_t nargs;
  u8 * params;
};

static size_t 
__mipi_dcs_get_seq_len (_IN_ u8 mipi_dcs_seq[]);

/**
 * Writes the given initialization commands in the format specified above to a
 * panel over the given `mipi_io_ctr`.
 */
static ssize_t
__mipi_dcs_write_seq (struct mipi_io_ctr * io_ctr, u8 init_seq[])
{
  if (io_ctr==NULL) {
    _mipi_dbg (
      MIPI_DBG_TAG,
      "IO connector uninitialized, failed to write init sequence"
    );

    return -(EINVAL);
  }

  if (init_seq==NULL) {
    _mipi_dbg (
      MIPI_DBG_TAG,
      "no initialization sequence provided, aborting transaction.."
    );
    
    return -(EINVAL);
  }

  while (true) {
    size_t i=0;

    if (init_seq[i]==END_DCS_SEQ) {
      return i;
    } else {
      // Get number of parameters to write.
      u8 num_params=init_seq[++i];
      u8 * params=init_seq+i;

      io_ctr->send_cmd (
        io_ctr, 
        init_seq[i], 
        params,
        num_params
      );
      i+=num_params;
    }
  }
}

#define __MIPI_DEFINE_DCS_CMD(name, code, n_params) \
  static const struct mipi_dcs_cmd MIPI_DCS_CMD ## name= \
  (struct mipi_dcs_cmd) { \
    .code_pt=code, \
    .nargs=n_params, \
  }; \
  \ 
  static const size_t              \
  name (                           \
    _OUT_ u8 dcs_seq_buff[],       \
    _IN_ u8 * args[],              \
    size_t n )                     \
  {                                \
    return (struct mipi_dcs_cmd) { \
      .code_pt=code,               \
      .nargs=n_params,             \
    };                             \
  } 

/**
 * const struct mipi_dcs_cmd cmd=mipi_dcs_cmd(
 *   RAMWR, 
 *   0, 
 *   NULL);
 * 
 * __DEFINE_MIPI_DCS_CMD (
 *   RAMWR,
 *   0x2C,
 *   -1
 * );
 * RAMWR (arg_buff, sz);
 */

#ifdef __cplusplus
}
#endif

#endif // __MIPI_DCS__