MIPI DBI Display Interface
===
The intention of this library is to supply generic support for MIPI DBI capable
displays. Generally these displays are low-resolution, low-throughput active
matrix panels capable of being driven by a MCU.

Although many platforms implement support for these display drivers, they
(1) provide a very thin abstraction over the DBI protocol and/or (2) do not
implement this in a platform-agnostic manner. It is then up to the client to
determine which feature-set is supported by their particular driver and
provide initialization sequences which may differ greatly between one driver
and another.

Furthermore, many libraries require fixed color spaces, such that they limit
the output to 16, 18, or 24-bit color, which is a great hinderance if a
panel provides an extended color space which could be used instead.

It also is worth mentioning that these thin abstractions cannot be used in
multithreaded environments without much boilerplate and concern for the
implementation details by the client. So, also, this library strives to
reduce memory foorprint and promote thread-safety in a manner which is
resistent to errors on the part of a user.

It is all of these goals which culminated in the implementation of this library
and drive the motivation for its architecture and design. It should be neither
difficult nor pain-staking to use such a panel, which has a well-defined and
widely implemented standard, and which could benefit from greater support on
new and old hardware.

The documentation here attempts to be as thorough and navigable as possible,
but there is also a great effort to provide context and details in the public
headers as well. It is thus worth referring to both when looking for
information regarding a certain behavior or feature thereof.

Contributing
----
As with any free and open-source project, the usefulness of this library
greatly depends on the work of people who dedicate their free time to
maintaining, implementing, and documenting the code. It also would not be a
very valient effort without projects which use it in their code bases.

There are guidelines which dictate contributing and use of this libray,
however. Namely, at this time, the work is published under the MIT license,
allowing for derivative works and use in comercial projects. Thus contributers
to this project are also subject to the terms of this license and implicitly
accep these terms when their code is included in this (primary) work. Of
course, it is up to the discretion whether their additions are added to this
project or they are published under a derivative work under their own terms,
possibly at cost.

Callaborators provide the most useful contributions whenever their code and
documentation is consistent with what already exists. Therefore, it is asked
that you attempt to match the style in the context of your contributions. It is
subjective whether it's determined that this is the case. Pull requests may be
rejected for these reasons and others at the discretion of the maintainer.

Table of Contents
----
There is no tutorial for using this library. Instead extensive reference is
provided, with the expectation that a user can navigate and interpret source
code written in C. There are instructions for interation using the OSAL
(Operating System Abstraction Layer) and CMake build system. You should be
somewhat familiar with CMake and configuring a build system. Such documentation
is better provided by the CMake project itself. There are a few
[examples](../examples) which can provide a base for your project and serve to
test your initial configuration.

1. OSAL .. #
2. Build System Config .. #

