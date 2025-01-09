#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/spi.h"
// #include "hardware/dma.h"
// #include "hardware/uart.h"

#define __MIPI_DBG_ENABLE__

#include "mipi.h"
#include "mipi_dbi_spi.h"

#define SPI_PORT spi0
#define PIN_MISO 16 // Potentially Unused
#define PIN_CS   17
#define PIN_SCK  18
#define PIN_MOSI 19

#define PIN_DCX  20

#define __MIPI_DELAY 0x80

const char MIPI_DBG_TAG[]="MIPI DBI Panel Dvr";
const uint8_t PROG_MEM[]=
{
  21,
  0x01, // Software Reset
  __MIPI_DELAY, 150,
  0x11, // Sleep Out
  __MIPI_DELAY, 255,
  0xB1, 3, 0x01, 0x2C, 0x2D,
  0xB2, 3, 0x01, 0x2C, 0x2D,
  0xB3, 6, 0x01, 0x2C, 0x2D, 0x01, 0x2C, 0x2D,
  0xB4, 1, 0x07,
  0xC0, 3, 0xA2, 0x02, 0x84,
  0xC1, 1, 0xC5,
  0xC2, 2, 0x0A, 0x00,
  0xC3, 2, 0x8A, 0x2A,
  0xC4, 2, 0x8A, 0xEE,
  0xC5, 1, 0x0E,
  0x20, 0, // Display Inversion Off

  // MADCTL <MY,MX,
  //   MV,ML,
  //   BGR,
  //   MC,FH,FV>;
  0x36, 1, 0x68, // MADCTL <0,1,1,0,
  // 1,
  // 0,0,0>;

  // COLMOD <IFPF[2:0]>; # <0,1,1>, <1,0,1>, <1,1,0>
  // COLMOD <1,0,1>;
  0x3A, 1, 0x05, // 16-bit color 
  
  // CASET <
  //   0,xi,
  //   0,xf>;
  0x2A, 4, 
    0x00, 0x00, 
    0x00, 0x9f, 
 
  // RASET <
  //   0x00,yi,
  //   0x00,yf>;
  0x2B, 4, 
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
  0x13, // Normal Display Mode
  __MIPI_DELAY, 10,
  0x29, // Display On
  __MIPI_DELAY, 100
};

struct mipi_panel_cfg 
{
  uint8_t * init_panel;
  size_t cmd_sz;
  void (*read)(uint8_t * buf, size_t len);
};

static void
init_panel(const uint8_t cmds[])
{
  if (cmds) {
    size_t i;
    uint8_t cmd, len, delay;
    uint8_t * ptr = (uint8_t *)(cmds);

    i = *ptr++;
    while (i-->0) {
      cmd=*ptr++, len=*ptr++;
      delay=len & __MIPI_DELAY;
      len=len & ~__MIPI_DELAY;

      printf("cmd: %02X, len: %d\n", cmd, len);

      gpio_put(PIN_DCX, 0);
      gpio_put(PIN_CS, 0);
      spi_write_blocking(SPI_PORT, &cmd, 1);
      gpio_put(PIN_DCX, 1);
      spi_write_blocking(SPI_PORT, ptr, len);
      gpio_put(PIN_CS, 1);
      ptr+=len;

      if (delay) {
        delay=*ptr++;
        if (delay>254) {
          sleep_ms(500);
        } else {
          sleep_ms(delay);
        }
      }
    }
  } else {
    __mipi_dbg(MIPI_DBG_TAG, "no panel init provided, failed..\n");
  }

  gpio_put(PIN_DCX, 1);
}

void 
get_color_bytes(uint8_t r, uint8_t g, uint8_t b, _OUT_ uint8_t bytes[2])
{
  bytes[0] =  (b & 0xf8)^(g>>5);
  bytes[1] =  ((g&0x1c)<<3)^(r>>3);
}

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
  
  spi_init(SPI_PORT, 32*1000*1000 /* 32 MHz */);
  gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
  gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
  gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
  gpio_set_function(PIN_CS,   GPIO_FUNC_SIO);
  gpio_set_function(PIN_DCX,  GPIO_FUNC_SIO);

  gpio_init(PIN_CS);
  gpio_set_dir(PIN_CS, GPIO_OUT);
  gpio_put(PIN_CS, 1);

  gpio_init(PIN_DCX);
  gpio_set_dir(PIN_DCX, GPIO_OUT);
  gpio_put(PIN_DCX, 1);

  init_panel(PROG_MEM);
  printf("device initialized on SPI 0\n");

  uint8_t write_cmd=0x2C;
  uint8_t clr_bytes[2];
  get_color_bytes(0x43, 0xf1, 0xab, clr_bytes);
  // get_color_bytes(255,255,255,clr_bytes);
  
  gpio_put(PIN_DCX, 0);
  gpio_put(PIN_CS, 0);
  spi_write_blocking(SPI_PORT, &write_cmd, 1);

  gpio_put(PIN_DCX, 1);
  int r,g,b;
  r=0, g=150, b=0;
  for (size_t i=0; i<128; i++) {
    r++;

    for (size_t j=0; j<160; j++) {
      get_color_bytes(r,g,j,clr_bytes);
      spi_write_blocking(SPI_PORT, clr_bytes, 2);
    }
  }
  gpio_put(PIN_CS, 1);

  struct mipi_spi_ctr ctr;
  struct mipi_dbi_dev * dev;

  ctr=MIPI_SPI_DBI_CTR ( 
    SPI_PORT,  
    PIN_SCK,
    PIN_MOSI,
    PIN_MISO,
    PIN_CS,
    PIN_DCX  
  );
  mipi_spi_connector_init (ctr);
  if (ctr.errno) {
    goto release_conn;
  }

  dev=mipi_dbi_panel_device (  // mipi_dbi_dev (  );
    "ILI9163C",
    128, 160,
    16,
    1
  );
  mipi_panel_dev_init (dev, IO_CTR_PTR (ctr));

release_dev:
  if (dev) {
    mipi_panel_dev_free (dev);
  } else {
    __mipi_dbg (MIPI_DBG_TAG, "no device to release\n");
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

