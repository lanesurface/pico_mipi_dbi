
/**
 * 
 * Copyright Surface EP, LLC 2025.
 */

#include "hardware/spi.h"
#include "mipi.h"
#include "mipi_dcs.h"
#include "mipi_dbi_spi.h"

/**
 * At the moment, starting and ending a transaction simply requires driving the
 * chip-select pin hi-lo, but, in the future, it may be necessary to obtain a 
 * lock on the peripheral if multiple displays are to be connected to a single
 * bus.
 */
#define SPI_BEGIN_TRANSACTION(spi_conn) \
  gpio_put ( \
    spi_conn->cs, \
    0 );

#define SPI_END_TRANACTION(spi_conn) \
  gpio_put ( \
    spi_conn->cs, \
    1 );

const struct mipi_io_ctr MIPI_SPI_CTR_FUNCS=
(struct mipi_io_ctr) {
  .send_cmd=mipi_spi_send_cmd,
  .recv_params=mipi_spi_recv_params,
  .flush_fmbf=mipi_spi_flush_fmbf
};

struct mipi_spi_ctr 
mipi_spi_ctr ( 
  spi_inst_t * spi,
  uint sck,
  uint mosi,
  uint miso,
  uint cs,
  uint dcx )
{
  return (struct mipi_spi_ctr){
    .io=MIPI_SPI_CTR_FUNCS,
    .sck=sck,
    .mosi=mosi,
    .miso=miso,
    .cs=cs,
    .dcx=dcx
  };
}

void
mipi_dbi_spi_connector_init (struct mipi_spi_ctr * self)
{
  spi_init ( 
    self->spi, 
    MIPI_SPI_DEFAULT_BAUD 
  );

  gpio_set_function (self->miso, GPIO_FUNC_SPI );
  gpio_set_function (self->mosi, GPIO_FUNC_SPI);
  gpio_set_function (self->sck,  GPIO_FUNC_SPI);
  gpio_set_function (self->cs,   GPIO_FUNC_SIO);
  gpio_set_function (self->dcx,  GPIO_FUNC_SIO);

  gpio_init (self->cs);
  gpio_set_dir (self->cs, GPIO_OUT);
  gpio_put (self->cs, 1);

  gpio_init (self->dcx);
  gpio_set_dir (self->dcx, GPIO_OUT);
  gpio_put (self->dcx, 1);
}

void
mipi_spi_send_cmd (
  struct mipi_io_ctr * self,
  uint cmd,
  _IN_ uint8_t * params, 
  size_t len )
{
  struct mipi_spi_ctr * spi_conn;

  if (params==NULL) {
    __mipi_dbg (
      self->DBG_TAG, 
      "command buffer empty, aborting transaction\n"
    );
    spi_conn->errno|=EINVAL;
    return;
  }

  spi_conn = (struct mipi_spi_ctr *) self;
  if (spi_conn) {
    SPI_BEGIN_TRANSACTION (spi_conn);

    gpio_put (spi_conn->dcx, 0);
    spi_write_blocking ( 
      spi_conn->spi, 
      &cmd, 
      1 
    );

    gpio_put (spi_conn->dcx, 1);
    spi_write_blocking ( 
      spi_conn->spi, 
      params, 
      len 
    );

    SPI_END_TRANACTION (spi_conn);
  } else {
    __mipi_dbg (
      self->DBG_TAG, 
      "SPI connector not initialized\n"
    );
    spi_conn->errno|=ENODEV;
    return; 
  }
}

size_t 
mipi_spi_recieve_params ( 
  struct mipi_io_ctr * self,
  uint cmd,
  _OUT_ uint8_t * params,
  size_t len )
{
  /*
   * TODO  
   */
  self->errno|=ENOTSUP;
}

void 
mipi_spi_flush_fmbf ( 
  struct mipi_io_ctr * self,
  _IN_ uint8_t * pix_buff,
  const struct mipi_area bounds,
  size_t len )
{
  struct mipi_spi_ctr * spi_conn;

  if (pix_buff==NULL) {
    __mipi_dbg (
      self->DBG_TAG,
      "pixel data buffer empty, aborting transaction\n"
    );
    spi_conn->errno|=EINVAL;
    return;
  } else {
    spi_conn=(struct mipi_spi_ctr *) self;
    if (spi_conn) {
      SPI_BEGIN_TRANSACTION (spi_conn);

      // u8 ca_params[];
      // spi_conn->io->write_cmd(
      //   self,
      //   CASET,
      //   ca_params,
      //   4
      // );

      // spi_conn->io->write_cmd(
      //   self, 
      //   RAMWR, 
      //   NULL, 
      //   0
      // );

      gpio_put (spi_conn->dcx, 1);
      spi_write_blocking ( 
        spi_conn->spi, 
        pix_buff, 
        len 
      );

      SPI_END_TRANACTION (spi_conn);
    } else {
      __mipi_dbg (
        self->DBG_TAG, 
        "SPI connector not initialized\n"
      );
      spi_conn->errno|=ENODEV;
      return;
    }
  }
}