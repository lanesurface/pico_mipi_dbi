# ====================
#  FindFreeType.cmake
# ====================
# In general, CMake provides an implementation of the FreeType module.
# However, the installation of FreeType on a development system is
# unlikely to be configured appropriately for linking into the embedded
# system. For that reason, a simple find module is provided here to reduce
# any future headaches.
#
# Author(s): Lane W Surface
# Created:   2025-02-20
# License:   MIT
#
# Copyright Surface EP, LLC 2025.

add_subdirectory (lib/freetype)
target_link_libraries (my_target .. freetype)

# find_library (free_type_lib NAMES freetype freetype2 PATHS
# 	${CMAKE_CURRENT_SOURCE_DIR}/lib
# 	SUBDIRECTORIES
# 		freetype
# 		freetype2)
