#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/spi.h"
// #include "hardware/dma.h"
// #include "hardware/uart.h"

#define MIPI_DBG_EN

#include "mipi.h"
#include "mipi_dbi_spi.h"

#define SPI_PORT spi0
#define PIN_MISO 16 // Potentially Unused
#define PIN_CS   17
#define PIN_SCK  18
#define PIN_MOSI 19

#define PIN_DCX  20


int
main()
{
    stdio_init_all();

    // spi_init(SPI_PORT, 32*1000*1000 /* 32 MHz */);
    // gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    // gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    // gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    // /**
    //  * In theory, the SPI peripheral should be able to drive the chip select
    //  * pin high and low as needed. However, there is a bug in the
    //  * implementation which causes the state of CS to toggle after each
    //  * successive byte is sent. This is not the desired behaviour, so we will
    //  * need to toggle the CS pin manually.
    //  */
    // gpio_set_function(PIN_CS,   GPIO_FUNC_SIO);

    // // CS is active low.
    // gpio_set_dir(PIN_CS, GPIO_OUT);
    // gpio_put(PIN_CS, 1);

    // // Get a free channel, panic() if there are none
    // int chan = dma_claim_unused_channel(true);

    // // 8 bit transfers. Both read and write address increment after each
    // // transfer (each pointing to a location in src or dst respectively).
    // // No DREQ is selected, so the DMA transfers as fast as it can.

    // dma_channel_config c = dma_channel_get_default_config(chan);
    // channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
    // channel_config_set_read_increment(&c, true);
    // channel_config_set_write_increment(&c, true);

    // dma_channel_configure(
    //     chan,          // Channel to be configured
    //     &c,            // The configuration we just created
    //     dst,           // The initial write address
    //     src,           // The initial read address
    //     count_of(src), // Number of transfers; in this case each is 1 byte.
    //     true           // Start immediately.
    // );

    // // We could choose to go and do something else whilst the DMA is doing its
    // // thing. In this case the processor has nothing else to do, so we just
    // // wait for the DMA to finish.
    // dma_channel_wait_for_finish_blocking(chan);

    // // The DMA has now copied our text from the transmit buffer (src) to the
    // // receive buffer (dst), so we can print it out from there.
    // puts(dst);

    // // Set up our UART
    // uart_init(UART_ID, BAUD_RATE);
    // // Set the TX and RX pins by using the function select on the GPIO
    // // Set datasheet for more information on function select
    // gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    // gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    // // Send out a string, with CR/LF conversions
    // uart_puts(UART_ID, " Hello, UART!\n");

  struct mipi_spi_ctr ctr;
  struct mipi_dbi_dev * dev;

  ctr=mipi_spi_ctr (/*...*/);
  mipi_spi_connector_init (ctr);

  dev=mipi_dbi_panel_device (  // mipi_dbi_dev (  );
    128,
		160,
    16,
    1
  );
  mipi_dbi_dev_init (dev, MIPI_IO_CTR_PTR (ctr));

release_dev:
  if (dev) {
    mipi_dbi_dev_free (dev);
  } else {
    _mipi_dbg (MIPI_DBG_TAG, "no device to release\n");
  }
release_conn:
  mipi_spi_connector_free (ctr); // mipi_spi_connector_deinit (ctr);
  // gpio_put(PIN_DCX, 0);
  // gpio_put(PIN_CS, 0);
  // spi_write_blocking(SPI_PORT, &write_cmd, 1);

  // gpio_put(PIN_DCX, 1);
  // for (size_t i=0; i<128*(180/2); i++) {
  //   spi_write_blocking(SPI_PORT, clr_bytes, 2);
  // }
  // gpio_put(PIN_CS, 1);

  for (;;) {
    tight_loop_contents();
  }
}

