/**
 * ========================
 *         asio.h
 * ========================
 * Wrapper for asynchronous IO processing. The composite IO types are blocking
 * by their nature. ASIO provides a common interface to delegate these IO
 * operations to be run in the background.
 *
 * Author(s): Lane W Surface
 * Created:   2025-02-06
 * License:   MIT
 *
 * Copyright Surface EP, LLC 2025.
 */

#ifndef __MIPI_ASIO__
#define __MIPI_ASIO__

#include "mipi.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * ASYNC IO
 *
 * Asynchronous wrapper over blocking IO operations.
 *
 * As a core guarantee provided by this asynchronous IO interface, user-provided
 * event handlers must not be called concurrently, as this would expose the
 * them to synchronization issues within their own code base. Therefore, the
 * threads or tasks used by this async IO mechnism must only execute within the
 * bounds of the IO processing and return control back to the main thread
 * upon reception of an IO result.
 *
 * Ultimately this means that any mutable data accepted by these async
 * operations must ensure NOT to modify the underlying buffer and must make a
 * copy so that the client code does not need to concern iteself with control
 * flow in the ASYNC context.
 */

struct async_io_ctx {};
struct async_awaitable_result {};
typedef void
(*async_io_read_cb)(
	mipi_err_T errno,
	byte_buffer_T out_buff
);

typedef void
(*async_io_write_cb)(
	mipi_err_T errno,
	size_t bytes_written
);

/**
 * Given the IO context and the connector, writes at most N bytes across the
 * connector from the provided buffer, where N is the size of this buffer. The
 * write may block to lock the `io_ctx` but should return immediately after
 * enqueuing the operation.
 */
extern mipi_err_T
async_io_write_some (
	struct async_io_ctx * io_ctx,
	struct mipi_io_ctr * io_ctr,
	_COPY_FROM_USER byte_buffer_view_T in_buff
);

extern mipi_err_T
async_io_read_some (
	struct async_io_ctx * io_ctx,
	struct mipi_io_ctr * io_ctr,
	_COPY_TO_USER byte_buffer_T out_buff
);

/**
 * async_io_write (_spi_ctr, MIPI_ASYNC_IO_CTX, _spi_write_hdlr, byte_buffer (
 *	ptl_fmbf,
 *	ptl_fmbf_sz
 * ));
 */

 /**
  * To make it easy as possible to both implement support for, and to cater
	* toward, generic targets of this library, a small abstraction is provided
	* which requires configuration and implementations of primitves usually
	* provided in the HAL of your platform or the RTOS.
	*
	* Forefront in the consideration of the design of such a mechanism, I wanted
	* to avoid making assumptions about the hardware's capabilities. Especially
	* because many MCU devices are not multi-processor systems, and I didn't
	* favor making this API dependent on a particular RTOS, the user must do a
	* bit of timekeeping on their part.
  */

extern mipi_err_T
async_write ();

extern mipi_err_T
async_read ();

/**
 * Read all bytes coming across the connector until the delimeter is reached.
 * Note that the callback will not be dispatched until this delimeter is
 * received, even if that requires multiple read operations.
 */
extern mipi_err_T
async_io_read_until ();

extern mipi_err_T
async_io_await_result (
	struct async_io_ctx * io_ctx,
	struct mipi_io_ctr * io_ctr,
	_IN byte_buffer_T read_params_buff,
	_OUT struct async_awaitable_result * result
);

/**
 * In the case that the platform has no system-provided scheduling mechanism,
 * the processing of IO operations requires explicit polling in client code.
 */
extern void
async_io_poll (struct mipi_io_ctr * io_ctr);


#ifdef __cplusplus
}
#endif

#endif // __MIPI_ASIO__
