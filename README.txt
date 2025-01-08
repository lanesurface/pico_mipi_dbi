Pico MCU MIPI DBI and DSI support
===

The aim of this library is to provide compatibility with the MIPI
standards, codified in [1], [2], and [3] by the Mobile Industry
Processor Alliance. These standards govern the vast majority of any
matrix display that you might wish to connect with a microprocessor.

Though there are three MIPI Phy (hardware-interconnect) specifications,
the two which are supported are those with built-in framebuffers, 
which are generally connected by a small parallel bus or serial interface
(SPI, in the case of DBI devices).

These screens share much in common with one another, including a unified
command set, called the display command set (DCS). Therefore, it is the
intention to provide also a unified way of connecting and communicating 
with active-matrix screens which adhere to these standards.

It should be noted that there are some limitations in driving a display
with a microcontroller; namely, the nature of these small, embedded SOC
limits both the amount of static storage, such as flash, which can be 
used to hold images, fonts, etc. and the RAM which is required to maintain
the framebuffers. However, with either sufficiently small resolutions, and/or
through the use of color compresion and other techniches, these problems can 
be mitigated to an extent (or, at least, as much as can be feasibly required 
of a MCU-driven device).

There are a great many platform-agnostic graphics libraries which can be 
built atop this HAL, including eg: FreeType, libJPEG, and LVGL. 
Integration with these components is not difficult, as they need only
direction where to render their graphics, which is provided by the 
framebuffers in this library. The rest is handled transparently, as these
buffers are consumed by the library and drive updates to the panel.