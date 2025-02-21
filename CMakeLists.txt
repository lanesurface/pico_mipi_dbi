
cmake_minimum_required(VERSION 3.24)
set(CMAKE_EXPORT_COMPILE_COMMANDS on)


# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.0)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================

set (PICO_SDK_FETCH_FROM_GIT on)
set (PICO_BOARD pico_w CACHE STRING "Board Type")

list (APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include (cmake/pico_sdk_import.cmake)
project (test_tft DESCRIPTION "" LANGUAGES C
  CXX
  ASM)

# Initialize the Raspberry Pi Pico SDK
pico_sdk_init ()

set (MIPI_ROOT_LIB_DIR ${test_tft_SOURCE_DIR} CACHE PATH "")
add_subdirectory (src)
add_executable (test_tft_main main.c)

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart (test_tft_main 0)
pico_enable_stdio_usb (test_tft_main 1)

set (LIB_PATH "${CMAKE_CURRENT_LIST_DIR}/lib")
# add_subdirectory (${LIB_PATH}/freetype)

# find_library (free_type_lib NAMES freetype freetype2 PATHS
#   "${LIB_PATH}/freetype/build"
#   REQUIRED)

# target_link_options (test_tft_main PRIVATE
#   "/SECTION:          \
#   .__run_in_ram_func, \
#   wrx,                \
#   ALIGN: 2")

target_link_libraries (
  test_tft_main
  PRIVATE
    pico_stdlib
    mipi_dbi
    freetype
    mipi_gfx_lib)

pico_add_extra_outputs (test_tft_main) # UF2, map, etc.
