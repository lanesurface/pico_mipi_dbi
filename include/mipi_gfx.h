
/**
 * ========================
 *       mipi_gfx.h
 * ========================
 * 
 * Simple and bare-bones graphics primitives (rect, line, arc, etc.). There is
 * really no use to these besides for debugging and testing displays, as there
 * are much better graphics libraries that can be linked against and used 
 * instead. However, if you do not need the capabilities of a full-featured 
 * library like LVGL, this does provide some simple software-accelerated 
 * rendering capabilities which should be sufficient for 2D games and the like.
 * 
 * Author(s): Lane W Surface
 * Created:   2025-01-10
 * License:   MIT
 * 
 * Copyright Surface EP, LLC 2025.
 */

#ifndef __MIPI_GFX__
#define __MIPI_GFX__

#include "pico/mutex.h"
#include "mipi.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MIPI_GFX_OBJ_BUFF_SZ 256
#define NUM_MIPI_GFX_EVENTS  3


/******************** 
 *      Types
 *******************/

typedef const ssize_t gfx_obj_handle_t;

enum mipi_gfx_obj_type {
  MIPI_TRANGLE,
  MIPI_LINE,
  MIPI_RECT,
  MIPI_ARC,
  MIPI_PT,
  MIPI_POLY_LINE,
  MIPI_POLYGON
};

struct mipi_normal_pt {
  double x, y;
};

struct mipi_gfx_obj {
  const enum mipi_gfx_obj_type obj_type;
  _Bool fill_obj;
  const size_t n_pts;
  struct mipi_normal_pt pt_array[];
};

struct mipi_shared_fmbf {
  const size_t fmbf_sz;
  mutex_t buff_mtx; // struct rw_lock buff_lk;
  volatile struct mipi_color clr_buff[];
};

enum mipi_gfx_event_type {
  MIPI_GFX_NOTIFY_UPDATE,
  MIPI_GFX_NOTIFY_SCALE_CHANGED,
  MIPI_GFX_NOTIFY_NEW_EXTENTS
};

/**
 * A graphics context has various events which it may emit, and which can be 
 * subscribed to by a consumer of these events, by calling `mipi_gfx_event_sub`
 * with a callback having a signature matching this type.
 */
typedef void (*mipi_gfx_event_cb)(
  enum mipi_gfx_event_type ev_type, 
  void * params, 
  size_t num_params
);

/**
 * ========================
 *     MIPI Gfx Context    
 * ========================
 * 
 * A graphics context is responsible for directing the rasterization of the
 * various graphics primitives to an internal frame buffer; and, subsequently,
 * copying these contents to a shared buffer, which is used to write to the 
 * frame memory of an associated panel device. Generally, this shared buffer,
 * the output buffer of the graphics context, is used to supply the DMA 
 * controller with data to write across a bus connected to the panel, though
 * there is no reason that a second-stage in the rendering pipline could not
 * use it as the input to another mechanism which affects the character of the
 * data thereof.
 * 
 * A context has no explicitly bound panel device; however, the most sensible 
 * way of obtaining a context is by requesting that a panel provide one which
 * is guaranteed to be compatible. It should be noted, though, that objects 
 * added to this context have no notion of device-specific parameters, such as 
 * the destination color format or the dimensions of the screen. Objects are 
 * rendered in the source color space and their positions specified in a 
 * normalized coordinate system, so that a graphics application written for one
 * device may be equally well represented on any other which this library is
 * capable of driving.
 * 
 * It should be noted that, in order to simplify the asynchronous rendering
 * mechanism, a graphics object added to the context cannot be modified; these
 * objects are passed by value when pushed onto the object stack, and this
 * context then maintains a seperate copy of that object so that the context
 * is always aware when the frame buffer need be updated. When an object is
 * pushed onto the stack, a handle is returned from the context, which can be
 * used in subsequent calls to graphics functions which modify or otherwise
 * change the component in this context.
 */
struct mipi_gfx_ctx {
  struct mipi_bounds * fmbf_ext;
  size_t num_gfx_objs;

  /**
   * The graphics object buffer holds the metadata required for rasterization,
   * with each object specifying its device-independent location on screen, the
   * stroke or fill color or gradiant, and an array of `mipi_pt` objects which
   * describe line segments connected by these points in an anti-clockwise
   * manner. Further, this buffer shall be responsible for maintaining the 
   * z-order of its components, such that the last component in the buffer has 
   * the highest z index, and these components are rasterized last, giving the 
   * appearance of proper depth. For simplicity's sake, there is no culling of
   * objects which would otherwise be covered by another, so care must be
   * taken to clear the screen if the objects in the context are no longer 
   * required.
   */
  const struct mipi_gfx_obj gfx_objs[MIPI_GFX_OBJ_BUFF_SZ];

  /**
   * Each event type makes an entry in this table, with the event ID being its
   * position in this array; and, for each ID, a linked list holds the 
   * callbacks which requested registration to receive events of that type from 
   * this context.
   */
  struct linked_list * event_cb_table[NUM_MIPI_GFX_EVENTS];

  const size_t fmbf_sz;
  struct mipi_shared_fmbf * fmbf;
  struct mipi_color * render_buff[];
};

extern void 
mipi_gfx_set_render_buffer (
  struct mipi_gfx_ctx * self,
  _IN_ struct mipi_shared_fmbf * out_buff,
  size_t n
);

/**
 * Modifies the extents of the render buffer, notifying consumers of the 
 * affected context. The output of the graphics context is first scaled by the
 * scale factor and then rasterized into the frame buffer assuming a screen of
 * these dimensions. Usually a call to this function, providing the dimensions 
 * of the panel, should occur before any objects are added to the context to 
 * avoid needless and expensive draw operations.
 */
extern void 
mipi_gfx_set_screen_bounds (
  struct mipi_gfx_ctx * self, 
  uint width,
  uint height
);

extern void 
mipi_gfx_set_scale (
  struct mipi_gfx_ctx * self,
  double scale
);

extern gfx_obj_handle_t
mipi_gfx_push_obj (
  struct mipi_gfx_ctx * self,
  struct mipi_gfx_obj gfx_obj,
  int z_idx
);

extern void
mipi_gfx_pop_obj (
  struct mipi_gfx_ctx * self, 
  gfx_obj_handle_t hdl
);

extern void 
mipi_gfx_xchange_obj (
  struct mipi_gfx_ctx * self,
  gfx_obj_handle_t hdl,
  struct mipi_gfx_obj obj
);

extern void 
mipi_gfx_clear_screen (struct mipi_gfx_ctx * self);

extern void 
mipi_gfx_sub_event (
  struct mipi_gfx_ctx * self,
  enum mipi_gfx_event_type ev_type,
  mipi_gfx_event_cb cb
);

extern struct mipi_gfx_obj 
mipi_gfx_obj ();

extern struct mipi_shared_fmbf * 
mipi_shared_fmbf ();

/**
 * // Screen coordinates should be normalized between [0..1] so as to be
 * // agnostic of actual pixel dimensions.
 * struct mipi_pt pts[]={
 *  {0,0},
 *  {0,1},
 *  {1,1},
 *  {0,0}
 * };
 * struct mipi_gfx_obj * t_obj=mipi_gfx_object (
 *  MIPI_TRANGLE, 
 *  pts,
 *  4
 * );
 * ssize_t obj_handle=mipi_gfx_push_object (t_obj, -1);
 * typedef const ssize_t gfx_obj_handle_t;
 */

#ifdef __cplusplus
}
#endif

#endif // __MIPI_GFX__