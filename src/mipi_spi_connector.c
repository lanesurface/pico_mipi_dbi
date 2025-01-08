
/**
 * 
 * Copyright Surface EP, LLC 2025.
 */

#include "hardware/spi.h"
#include "mipi.h"
#include "mipi_dcs.h"
#include "mipi_dbi_spi.h"

#define MIPI_SPI_DEFAULT_BAUD 32*1000*1000 /* 32 MHz */
#define MIPI_SPI_DEFAULT_MOSI_PIN 19
#define MIPI_SPI_DEFAULT_MISO_PIN 16
#define MIPI_SPI_DEFAULT_SCK_PIN  18
#define MIPI_SPI_DEFAULT_CS_PIN   17
#define MIPI_SPI_DEFAULT_DCX_PIN  20

#define MIPI_DBI_SPI_DEFAULT_CONFIG() \
  { \
    .io = { \
      .name="MIPI SPI I/O", \
      .type=MIPI_DBI_SPI, \
    }, \
    .spi = spi0, \
    .tx_buf = NULL, \
    .rx_buf = NULL, \
    .tx_len = 0, \
    .rx_len = 0, \
    .cs = MIPI_SPI_DEFAULT_CS_PIN, \
    .dcx = MIPI_SPI_DEFAULT_DCX_PIN, \
    .sck = MIPI_SPI_DEFAULT_SCK_PIN, \
    .mosi = MIPI_SPI_DEFAULT_MOSI_PIN, \
    .miso = MIPI_SPI_DEFAULT_MISO_PIN, \
  }

#define SPI_BEGIN_TRANSACTION(spi_conn) \
  gpio_put( \
    spi_conn->cs, \
    0 );

#define SPI_END_TRANACTION(spi_conn) \
  gpio_put( \
    spi_conn->cs, \
    1 );

void
mipi_dbi_spi_connector_init( struct mipi_panel_spi_connector * self )
{
  spi_init( 
    self->spi, 
    MIPI_SPI_DEFAULT_BAUD 
  );

  gpio_set_function( self->miso, GPIO_FUNC_SPI );
  gpio_set_function( self->sck,  GPIO_FUNC_SPI );
  gpio_set_function( self->mosi, GPIO_FUNC_SPI );
  gpio_set_function( self->cs,   GPIO_FUNC_SIO );
  gpio_set_function( self->dcx,  GPIO_FUNC_SIO );

  gpio_init( self->cs );
  gpio_set_dir( self->cs, GPIO_OUT );
  gpio_put( self->cs, 1 );

  gpio_init( self->dcx );
  gpio_set_dir( self->dcx, GPIO_OUT );
  gpio_put( self->dcx, 1 );
}

void
mipi_spi_send_cmd( 
  struct mipi_panel_io_connector * self,
  uint cmd,
  _IN_ uint8_t * params, 
  size_t len )
{
  struct mipi_panel_spi_connector * spi_conn;

  if (params==NULL) {
    __mipi_dbg("command buffer empty, aborting transaction\n");
    spi_conn->errno|=EINVAL;
    return;
  }

  spi_conn = (struct mipi_panel_spi_connector *) self;
  if (spi_conn) {
    SPI_BEGIN_TRANSACTION( spi_conn );

    gpio_put( spi_conn->dcx, 0 );
    spi_write_blocking( 
      spi_conn->spi, 
      &cmd, 
      1 
    );

    gpio_put( spi_conn->dcx, 1 );
    spi_write_blocking( 
      spi_conn->spi, 
      params, 
      len 
    );

    SPI_END_TRANACTION( spi_conn );
  } else {
    __mipi_dbg("SPI connector not initialized\n");
    spi_conn->errno|=ENODEV;
    return; 
  }
}

size_t 
mipi_spi_recieve_params( 
  struct mipi_panel_io_connector * self,
  uint cmd,
  _OUT_ uint8_t * params,
  size_t len )
{

}

void 
mipi_spi_flush_fmbf( 
  struct mipi_panel_io_connector * self,
  _IN_ uint8_t * pix_buff,
  size_t len )
{
  struct mipi_panel_spi_connector * spi_conn;

  if (pix_buff==NULL) {
    __mipi_dbg("pixel data buffer empty, aborting transaction\n");
    spi_conn->errno|=EINVAL;
    return;
  } else {
    spi_conn = (struct mipi_panel_spi_connector *) self;
    if (spi_conn) {
      SPI_BEGIN_TRANSACTION( spi_conn );

      u8 ram_wr=RAMWR;
      gpio_put( spi_conn->dcx, 0 );
      spi_write_blocking( 
        spi_conn->spi, 
        &ram_wr, 
        1 
      );

      gpio_put( spi_conn->dcx, 1 );
      spi_write_blocking( 
        spi_conn->spi, 
        pix_buff, 
        len 
      );

      SPI_END_TRANACTION( spi_conn );
    } else {
      __mipi_dbg("SPI connector not initialized\n");
      spi_conn->errno|=ENODEV;
      return;
    }
  }
}