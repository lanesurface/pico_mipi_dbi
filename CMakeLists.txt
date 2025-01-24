
cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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
set(PICO_BOARD pico_w CACHE STRING "Board Type")

include (pico_sdk_import.cmake)
project (test_tft C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init ()

set (MIPI_ROOT_LIB_DIR ${CMAKE_CURRENT_LIST_DIR})

add_subdirectory (src)
add_executable (test_tft_main main.c)

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart (test_tft 0)
pico_enable_stdio_usb (test_tft 1)

target_compile_definitions (
    tft_tft_main PUBLIC 
    MIPI_MGL_EN )

target_link_libraries (
  test_tft_main PUBLIC
  pico_stdlib 
  pico_mipi_dbi 
  # ... 
)

pico_add_extra_outputs (test_tft) # UF2, map, etc.
