/**
 * Implementations of the MGL primitive draw operations.
 *
 * Copyright Surface EP, LLC 2025.
 */

#include "mipi.h"
#include "mgl.h"
#include "osal.h"

struct mipi_gfx_obj_attrs;

static void
_mgl_draw_line (
	struct mipi_shared_fmbf * gfx_fmbf,
	uint x0,
	uint y0,
	uint x1,
	uint y1
);

static void
_mgl_get_obj_intercept (
	struct mgl_gfx_obj * obj,
	const struct mipi_area * fmbf_bd,
	_OUT_ struct _mgl_pt pt_icept_arr[]
);

void
_mgl_render_gfx_objs (struct mgl_gfx_ctx * gfx_ctx);
