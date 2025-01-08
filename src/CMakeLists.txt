
cmake_minimum_required(VERSION 3.13)

add_library(mipi_dbi)
target_sources(mipi_dbi PRIVATE
  mipi_dbi.c
  mipi_spi_connector.c)

target_include_directories(mipi_dbi INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}../
  ${CMAKE_CURRENT_LIST_DIR}../include
)