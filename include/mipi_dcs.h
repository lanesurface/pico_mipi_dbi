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
// # <<GFX Memory Buffer Write>>
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
// # <<Memory Access Ctl>>
// MADCTL <MX,MY, // mirror x, mirror y
//   MV,ML, // xchange rows/columns, scan direction
//   RGB, // 0=RGB, 1=BGR
//   0,0>; 
#define MADCTL 0x36
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

struct mipi_dcs_cmd 
{
  uint code;
  size_t len;
  u8 * params;
};


#define __DEFINE_MIPI_DCS_CMD(name, code, n_params) \
  

/**
 * const struct mipi_dcs_cmd cmd=mipi_dcs_cmd(
 *   RAMWR, 
 *   0, 
 *   NULL);
 */

#ifdef __cplusplus
}
#endif

#endif // __MIPI_DCS__