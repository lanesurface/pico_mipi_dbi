
/**
 * ========================
 *     mipi_dbi_spi.h
 * ========================
 * 
 * MIPI DBI type C device, 4-wire SPI interface.
 * 
 * Author(s): Lane W. Surface
 * Created:   2025-01-06
 * License:   MIT
 * 
 * Copyright Surface EP, LLC 2025.
 */

#ifndef __MIPI_DBI_SPI__
#define __MIPI_DBI_SPI__

#include "pico/mutex.h"
#include "hardware/spi.h"
#include "mipi.h"

#ifdef __cplusplus
extern "C" {
#endif


/******************** 
 *     Macros
 *******************/

#define MIPI_SPI_DBI_CTR( spi_prt, sck_pin, mosi_pin, miso_pin, cs_pin, dcx_pin ) \
  (struct mipi_spi_ctr){ \
    .io=_MIPI_SPI_CTR_FUNCS, \
    .spi=spi_prt, \
    .sck=sck_pin, \
    .mosi=mosi_pin, \
    .miso=miso_pin, \
    .cs=cs_pin, \
    .dcx=dcx_pin \
  }

#define _SPI_ACTIVE_STATE ((_Bool)0)

#define _SPI_BEGIN_TX(_spi_dev)              \
  mipi_lock_spi_dev (_spi_dev, MIPI_MAX_TM); \
  gpio_put (                                 \
    _spi_dev->cs,                            \
    _SPI_ACTIVE_STATE                        \
  );

#define _SPI_END_TX(_spi_dev)                  \
  mipi_unlock_spi_dev (_spi_dev, MIPI_MAX_TM); \
  gpio_put (                                   \
    _spi_dev->cs,                              \
    ~(_SPI_ACTIVE_STATE)                       \
  );


/******************** 
 * Global Constants
 *******************/

#define MIPI_SPI_DEFAULT_BAUD 32*1000*1000 /* 32 MHz */
#define MIPI_SPI_DEFAULT_PORT (spi_inst_t *)spi0
#define MIPI_SPI_DEFAULT_MOSI_PIN 19
#define MIPI_SPI_DEFAULT_MISO_PIN 16
#define MIPI_SPI_DEFAULT_SCK_PIN  18
#define MIPI_SPI_DEFAULT_CS_PIN   17
#define MIPI_SPI_DEFAULT_DCX_PIN  20

extern const struct mipi_io_ctr _MIPI_SPI_CTR_FUNCS;


/******************** 
 *      Types
 *******************/

struct _mipi_spi_dev {
  struct mipi_dbg_info_hdr * hdr;
  const spi_inst_t * spi;
  uint cs, sck, mosi, miso;
  mutex_t spi_mtx;
  const size_t buff_sz;
  uint8_t * tx_buff, * rx_buff;
};

struct mipi_spi_ctr {
  struct mipi_io_ctr io; /* BASE */
  struct _mipi_spi_dev * spi_dev;
  /**
   * In the case that a transaction fails, this flag is set to the relevant
   * error code(s). It is the responsibility of the caller of these interface
   * functions to check this flag and handle the error condition accordingly.
   */
  int errno;
};


/******************** 
 * Global Functions
 *******************/

extern struct mipi_spi_ctr 
mipi_create_spi_ctr (
  spi_inst_t * spi,
  uint sck,
  uint mosi,
  uint miso,
  uint cs,
  uint dcx
);

extern void
mipi_init_spi_ctr (struct mipi_spi_ctr * self);

extern void
mipi_free_spi_ctr (struct mipi_spi_ctr * self);

extern void
mipi_spi_send_cmd ( 
  struct mipi_io_ctr * self,
  mipi_dcs_cmd_t cmd,
  _IN_ uint8_t * buf, 
  size_t len 
);

extern size_t
mipi_spi_recv_params ( 
  struct mipi_io_ctr * self,
  mipi_dcs_cmd_t cmd,
  _OUT_ uint8_t * params,
  size_t len 
);

extern void 
mipi_spi_flush_fmbf ( 
  struct mipi_io_ctr * self,
  _IN_ mipi_dcs_cmd_t * buf,
  const struct mipi_area bounds,
  size_t len 
);

extern _Bool
mipi_lock_spi_dev_timeout_ms (
  struct _mipi_spi_dev * dev,
  uint32_t ms
);

extern _Bool
mipi_try_lock_spi_dev (struct _mipi_spi_dev * dev);

extern void 
mipi_unlock_spi_dev (struct _mipi_spi_dev * dev);


#ifdef __cplusplus
}
#endif 

#endif // __MIPI_DBI_SPI__
