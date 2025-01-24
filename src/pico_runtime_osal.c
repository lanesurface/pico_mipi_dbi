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

 #include "osal.h"


/**
 * typedef <mtx_type> _osal_mtx_T;
 * typedef <spi_dev_type> _osal_spi_dev_T;
 * typedef <gpio_pin_type> _osal_gpio_pin_T;
 * 
 * extern _osal_mtx_T
 * _osal_create_mutex (void);
 */

 extern _osal_mtx_T
 _osal_create_mutex (void);

 extern _Bool
 _osal_try_lock_mtx (_osal_mtx_T * osal_mtx);