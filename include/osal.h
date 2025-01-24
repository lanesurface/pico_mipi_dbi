/**
 * ========================
 *         osal.h
 * ========================
 *
 * Operating System Abstraction Layer
 *
 * Refer to the documentation for information on the types necessary to
 * define and the behavior of ported functions. 
 *
 * Author(s): Lane W Surface
 * Created: 	01/23/2025
 * License: 	MIT
 *
 * Copyright Surface EP, LLC 2025.
 */

#include <stdbool.h>
#include "pico/mutex.h"
#include "hardware/spi.h"


#define MIPI_WEAK_DEF __weak

/**
 * typedef <mtx_type> _osal_mtx_T;
 * typedef <spi_dev_type> _osal_spi_dev_T;
 * typedef <gpio_pin_type> _osal_gpio_pin_T;
 * 
 *
 */
 
 /**
  * <<IMPLEMENTATION NOTE>>
	* 
  * The `_osal_*` functions below are organized by category. For 
	* each category is a corresponding MIPI feature (eg, SPI based 
	* IO connector). To use that feature in client code, there must
	* be a corresponding implemntation of all functions in that 
	* category. The base set of OSAL functions and types required 
	* for all configuration immediately follows this text. 
  */

typedef mutex_t _osal_mtx_T;
typedef spi_inst_t _osal_spi_dev_T; 
typedef const uint8_t _osal_gpio_pin_T;


MIPI_WEAK_DEF extern void 
_osal_init_gpio_pin ();

MIPI_WEAK_DEF extern void 
_osal_set_gpio_pin_state (
	_osal_gpio_pin_T pin,
	const _Bool pin_val
);

/**
 * <<SPI>>
 */

MIPI_WEAK_DEF extern void
_osal_init_spi_dev ();

/**
 * <<MGL>>
 */

MIPI_WEAK_DEF extern _osal_mtx_T
_osal_create_mutex (void);

MIPI_WEAK_DEF extern _Bool
_osal_try_lock_mtx (_osal_mtx_T * osal_mtx);

MIPI_WEAK_DEF extern _Bool
_osal_lock_mtx_timeout_ms (
	_osal_mtx_T * osal_mtx,
	uint32_t ms
);

MIPI_WEAK_DEF extern uint32_t 
_osal_get_time_ms (void);