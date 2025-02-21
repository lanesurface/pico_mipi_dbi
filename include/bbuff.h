
/**
 * <<DESC>>
 *
 * Author(s): Lane W Surface
 * Created:   12-02-2025
 * License:   MIT
 *
 * Copyright Surface EP, LLC 2025.
 */

#ifndef __MIPI_BYTE_BUFFER__
#define __MIPI_BYTE_BUFFER__

#include "sep_osal_emb/sysdefs.h"

#ifdef __cplusplus
extern "C" {
#endif

struct byte_buffer {
	uint8_t * buff;
	size_t buff_sz;
};

typedef struct byte_buffer byte_buffer_T;
typedef const byte_buffer_T byte_buffer_view_T;

static __force_inline byte_buffer_T
byte_buffer (
	uint8_t * buff,
	size_t buff_sz )
{
	struct byte_buffer bb=
	{
		buff,
		buff_sz
	};
	return bb;
}

static __force_inline byte_buffer_T
byte_buffer_make_copy (
	byte_buffer_T byte_buff
)
{
	const size_t l=byte_buff.size;
	byte_buffer * r=__osal_thdr_safe_malloc ( l );
	__osal_thrd_safe_memcpy ( &byte_buff, l );
	return r;
}

static __force_inline byte_buffer_view_T
buffer_create_view_from (
	byte_buffer_T in_buff,
	size_t buff_offset,
	size_t len )
{
	struct byte_buffer bb=
	{
		buff+buff_offset,
		len
	};
	return bb;
}

#ifdef __cplusplus
}
#endif

#endif
