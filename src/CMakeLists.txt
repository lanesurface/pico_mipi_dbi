
cmake_minimum_required (VERSION 3.13)

# Source files required for all configurations.
list (
  MIPI_DBI_CORE_SRCS
  mipi_dbi.c 
  mipi_spi_ctr.c 
  pico_runtime_osal.c # Implementation of OSAL for `pico_runtime`.
  ll.c )
if (MIPI_MGL_EN)
  list (
    APPEND MIPI_DBI_CORE_SRCS
    mgl.c 
    mgl_draw_gfx.c 
    mgl_tx_fmbf.c )
endif ()

add_library (pico_mipi_dbi STATIC)
target_sources (
  pico_mipi_dbi PRIVATE 
  MIPI_DBI_CORE_SRCS )
# Add the current and build directory to the include path.
set (CMAKE_INCLUDE_CURRENT_DIR on)
target_include_directories (
  pico_mipi_dbi PUBLIC
  ${MIPI_ROOT_LIB_DIR}
  ${MIPI_ROOT_LIB_DIR}/include )

target_link_libraries (
  pico_mipi_dbi PRIVATE 
  pico_stdlib 
  pico_async_context
  hardware_spi 
  hardware_dma )

if (MIPI_DBG_EN) 
  target_compile_options (
    pico_mipi_dbi PRIVATE 
    "-fstack-usage"
    "-Wframe-larger-than=2048" 
    "-Wunreachable-code"
    "-Wnull-dereference"
    "-Wstrict-prototypes" )
endif ()