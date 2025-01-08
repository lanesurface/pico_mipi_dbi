
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

#include "mipi.h"
#include "hardware/spi.h"

#ifdef __cplusplus
extern "C" {
#endif

struct mipi_panel_spi_connector {
  struct mipi_panel_io_connector * io; /* BASE */
  spi_inst_t * spi;
  uint8_t * tx_buf, * rx_buf;
  size_t tx_len, rx_len;
  uint sck, mosi, miso, cs, dcx;
  /**
   * In the case that a transaction fails, this flag is set to the relevant
   * error code(s). It is the responsibility of the caller of these interface
   * functions to check this flag and handle the error condition accordingly.
   */
  int errno;
};

extern struct mipi_panel_spi_connector *
mipi_dbi_spi_connector(
  spi_inst_t * spi,
  uint sck,
  uint mosi,
  uint miso,
  uint cs,
  uint dcx
);

extern void
mipi_dbi_spi_connector_init( struct mipi_panel_spi_connector * self );

extern void
mipi_dbi_spi_connector_free( struct mipi_panel_spi_connector * self );

extern void
mipi_spi_send_cmd( 
  struct mipi_panel_io_connector * self,
  uint cmd,
  _IN_ uint8_t * buf, 
  size_t len 
);

extern size_t
mipi_spi_recv_params( 
  struct mipi_panel_io_connector * self,
  uint cmd,
  _OUT_ uint8_t * params,
  size_t len 
);

extern void 
mipi_spi_flush_fmbf( 
  struct mipi_panel_io_connector * self,
  _IN_ uint8_t * buf,
  const struct mipi_area bounds,
  size_t len 
);

#ifdef __cplusplus
}
#endif 

#endif // __MIPI_DBI_SPI__