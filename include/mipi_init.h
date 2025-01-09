
/**
 * 
 * 
 * Author(s): Lane W Surface
 * Created:   2025-01-08
 * License:   MIT
 * 
 * Copyright Surface EP, LLC 2025.
 */

#ifndef __MIPI_INIT__
#define __MIPI_INIT__

#include "mipi.h"
#include "mipi_dcs.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * ST7735
 * 
 * width: 128, height: 160 
 */
static const u8 MIPI_DEV_ST7735[]=
{
  SWRST, 
  MIPI_DELAY, 150,
  SLPOUT, 
  MIPI_DELAY, 255,
  // FPS=Fosc/((RTNA*2+40)*(LINE+FPA+BPA))
  // Fosc=625 KHz
  // FRMCTL1 [1,44,45];
  FRMCTRL1, 3, 0x01, 0x2C, 0x2D, // 59 FPS
  FRMCTRL2, 3, 0x01, 0x2C, 0x2D,
  FRMCTRL3, 6, 0x01, 0x2C, 0x2D, 0x01, 0x2C, 0x2D,
  INVCTRL, 1, 0x07,
  PWCTRL1, 3, 0xA2, 0x02, 0x84,
  PWCTRL2, 1, 0xC5,
  PWCTRL3, 2, 0x0A, 0x00,
  PWCTRL3, 2, 0x8A, 0x2A,
  PWCTRL4, 2, 0x8A, 0xEE,
  PWCTRL5, 1, 0x0E,
  INVOFF, 0, 

  MADCTL, 1, MX | SWAP_XY | BGR, 
  // COLMOD <mipi_color_fmt::RGB_565>;
  COLMOD, 1, IFPF_16_BIT, 
  
  // CASET <0,xi,0,xf>;
  CASET, 4, 
    0x00, 0x00, 
    0x00, 0x9f, 
 
  // RASET <0,yi,0,yf>;
  RASET, 4, 
    0x00, 0x00, 
    0x00, 0x7f, 
  
  0xE0, 16, // Positive Gamma
    0x02, 0x1C, 0x07, 0x12, 
    0x37, 0x32, 0x29, 0x2D, 
    0x29, 0x25, 0x2B, 0x39, 
    0x00, 0x01, 0x03, 0x10,
  0xE1, 16, // Negative Gamma
    0x03, 0x1D, 0x07, 0x06, 
    0x2E, 0x2C, 0x29, 0x2D, 
    0x2E, 0x2E, 0x37, 0x3F, 
    0x00, 0x00, 0x02, 0x10,
  
  NORON, 
  MIPI_DELAY, 10,
  DISPON, 
  MIPI_DELAY, 100,
  
  END_DCS_SEQ // sentinel
};

/**
 * ST7789
 */

/**
 * ILI9341
 */


#ifdef __cplusplus
}
#endif

#endif