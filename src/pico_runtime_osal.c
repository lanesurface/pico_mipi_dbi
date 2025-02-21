/**
 *
 * Implemenation of the OSAL (Operating System Abstraction Layer) for
 * the `pico_runtime`. This provides compatibility with Pico SDK.
 *
 * Author(s): Lane W Surface
 * Created:   2025-01-24
 * License:   MIT
 *
 * Copyright Surface EP, LLC 2025.
 */

#include "pico/mutex.h"
#include "hardware/spi.h"
#include "osal.h"

//      Board Pin Number   GPIO Pin
#define PICO_W_BOARD_PIN_1 0
#define PICO_W_BOARD_PIN_2 1
#define PICO_W_BOARD_PIN_4 2
#define PICO_W_BOARD_PIN_5 3

#define NUM_PICO_SPI_DEV 2

typedef uint8_t _osal_gpio_pin_T;
typedef mutex_t _osal_mtx_T;
typedef size_t _osal_mtx_handle_T;
typedef spi_inst_t _osal_base_spi_dev_T;

#define MISO_PIN 0
#define MOSI_PIN 1
#define SCK_PIN  2

struct _osal_pin_mux_map;
/**
 * For each of the 2 SPI peripherals, this table defines the mapping
 * of GPIO pins which can be MUX for each. These are used to determine
 * which of the devices will be selected when a client requests SPI
 * initialization.
 */
static const struct _osal_pin_mux_map
_SPI_PIN_MAP[NUM_PICO_SPI_DEV];

static struct _osal_spi_dev *
_SPI_DEV[NUM_PICO_SPI_DEV];

extern int
_osal_set_gpio_pin_func ();

/**
 * typedef <mtx_type>      _osal_mtx_T;
 * typedef <spi_dev_type>  _osal_spi_dev_T;
 * typedef <gpio_pin_type> _osal_gpio_pin_T;
 *
 * extern _osal_mtx_T
 * _osal_create_mutex (void);
 */

 extern _osal_mtx_handle_T
 _osal_create_mutex (void);

 extern _Bool
 _osal_try_lock_mtx (_osal_mtx_T * osal_mtx);
