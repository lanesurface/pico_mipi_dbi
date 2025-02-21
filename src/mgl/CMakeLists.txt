
cmake_minimum_required (VERSION 3.24)

add_library (
  mipi_gfx_lib
  STATIC
   mgl.c
   mgl_draw_gfx.c)

target_include_directories (
  mipi_gfx_lib
  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR})

if (${mipi_gfx_lib_IS_TOP_LEVEL})
  find_library (SDL_LIB NAMES sdl sdl2 HINTS
    "${CMAKE_CURRENT_LIST_DIR}/../lib"
    REQUIRED)
  target_link_libraries (mipi_gfx_lib PRIVATE ${SDL_LIB})
endif()

