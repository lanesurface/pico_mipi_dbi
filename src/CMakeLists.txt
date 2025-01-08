
cmake_minimum_required( VERSION 3.13 )

add_library( pico_mipi_dbi )
target_sources( pico_mipi_dbi PRIVATE
  mipi_dbi.c
  mipi_spi_connector.c )

target_include_directories( pico_mipi_dbi INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}../
  ${CMAKE_CURRENT_LIST_DIR}../include )

target_link_libraries( pico_mipi_dbi INTERFACE 
  pico_stdlib 
  hardware_spi 
  hardware_dma )