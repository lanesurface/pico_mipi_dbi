The MIPI Graphics Library
---
The MIPI Graphics Library (or MGL, as it is referred to in the source
code) is a 2D graphics layer which implements basic software-based
rasterization of 2D graphics primitives like lines, rectangles, arcs,
and other polygons which can be described by these shapes.

It is not the aim of this library to provide efficient implementations
of such mechanisms, nor was it designed with any particular goal in
mind; rather, its primary purpose is to render simple shapes which aid
in the debugging of MIPI devices capable of being driven by this 
library.

With that in mind, it is generic enough that a client may use it for
designing simple 2D games, or even extend it in such a way that it 
might be useful for more complex projects which require these capabilities.
The main goal in its implementation is memory and thread safety. To that
end, there must be a trade off between memory consumption, ease of use,
and protecting a client from themselves. 

This document describes the system design and some of the principles of
use of MGL, and attempts to explain the rationale behind certain decisions
made, as well as how to go about configuring the library to work for
your particular use case.

The Graphics Context
----
Each graphics context carries with it configuration parameters specified
by the client, such as the size of the internal frame buffer, the panel
which the context is associated with, and the stack that holds the 
graphics objects. The mapping between a graphics context and a panel 
device is and must be one-to-one, which is to say that each context is
associated with a single panel, and each panel must only initialize
one context to handle its rendering. 

You might think that it would be useful to have a single global context
which is responsible for *ALL* rendering, but this cannot be the case.
The reason is two-fold. For one, in designing this library, I wanted 
to decouple the panel dimensions from the rendering mechanism. This means
that objects are specified in a normalized coordinate system without
regard to the resolution of the rendering device. The second reason is
because it simplified the creation of background tasks so as to not 
interfere with the runtime of client code. 

So, then, how does a context get created at runtime? The first thing 
which is needed is a description of the context parameters, which is
done by creating a struct of the type `mgl_gfx_ctx_cfg`.

```
struct mgl_gfx_ctx_cfg {
  size_t fmbf_sz,gfx_stack_sz;
  struct mipi_dbi_dev * dev_;
};
``` 

So you can see there are three values which need to be provided, and these
can be summarized as such:
* `fmbf_sz`: The size of the internal frame buffer which is used to render
the graphics objects onto. There is only one, as there is only one thread
which handles all of the rendering and frame update operations.
* `gfx_stack_sz`: The number of graphics objects which can appear on the
screen at once. Higher values may use significantly more memory, so by
default, this should be set to a relatively low value, eg: 256.
* `dev_`: The panel which should be associated with this context.

The graphics context handles all frame update operations in the background
when the contents of the frame buffer change. What this means is that 
client threads need to be aware when the context has control of the panel,
and so should be sure to lock the panel device before sending any commands
and/or data themselves. Ideally, all panel configuration that needs to be
done before use of the panel should occur before the context is initiarted.
Attempting to update the panel driver's registers after the context has
began to render has the potential to block and/or be relatively slow.
