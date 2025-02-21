# At the moment, the build system requires GCC or Clang for compilation of C
# sources with GNU C language extentions.
cmake_minimum_required (VERSION 3.24)
include(CheckSymbolExists)

project (
  mipi_dbi_dvr
  VERSION 0.1.0.0
  DESCRIPTION
    "MIPI DBI display interface for embedded systems"
  HOMEPAGE_URL https://github.com/lanesurface/pico_mipi_dbi
  LANGUAGES
    C ASM)
add_subdirectory (mgl)

# cmake -S .. -B . -DMIPI_MGL_EN=ON -D_PF_STACK_FRAME_SZ=2048, make all
# set (MIPI_MGL_EN ON)
# target_link_libraries (
#  my_exe
#  PRIVATE
#   pico_stdlib
#   pico_cyw42_driver
#   pico_mipi_dbi
#   )

# Source files required for all configurations.
set (
  MIPI_DBI_CORE_SRCS
  CACHE INTERNAL
    mipi_dbi.c
    mipi_i80_parallel_ctr.c
    mipi_spi_ctr.c
    ll.c)

# set (
#   MIPI_COMPILE_DEFINITIONS
#   $<TARGET_PROPERTY:pico_mipi_dbi,COMPILE_DEFINITIONS>
# )
# set (
#   _PF_HAS_ATOMICS
#   $<NOT:$<BOOL:$<IN_LIST:__STDC_NO_ATOMICS__,${MIPI_COMPILE_DEFINITIONS}>>>
# )
check_symbol_exists (__STDC_NO_ATOMICS__ "stdatomic.h" _PF_MISSING_ATOMICS)
add_library (
  pico_mipi_dbi
  STATIC
    mipi_dbi.c
    mipi_i80_parallel_ctr.c
    mipi_spi_ctr.c
#    $<IF:${_PF_HAS_ATOMICS},atomic_native.c,atomic_lock_impl.c>
    )

target_include_directories (
  pico_mipi_dbi
  PRIVATE ${MIPI_ROOT_LIB_DIR}/src # Private Headers
  PUBLIC  ${MIPI_ROOT_LIB_DIR}/include)
target_compile_features (
  pico_mipi_dbi
  PUBLIC
    c_std_11)

# Interface target for building against the Pico SDK. Also adds some
# configuration addressing the limitations of `pico_runtime`.
# set (BUILD_PF pico_mipi_dbi_runtime)
# add_library (
#   ${BUILD_PF}
#   STATIC
#     pico_runtime_osal.c
# )
# target_link_libraries (
#   ${BUILD_PF}
#   PUBLIC
#     pico_stdlib
#     pico_atomic
#   PRIVATE
#     pico_async_context
#     hardware_spi
#     hardware_dma
# )
# target_compile_definitions (
#   ${BUILD_PF}
#   PUBLIC _PF_NATIVE_TYPES=pico_types.h
# )

add_library (compiler_flags INTERFACE)
set_property (TARGET compiler_flags PROPERTY warn_base "-Wall -Wextra")
set_property (
  TARGET compiler_flags
  PROPERTY warn_on_deprecated_features
    "-Wold-style-definition"
    "-Wdeprecated-non-prototype"
    )
set_property (
  TARGET compiler_flags
  PROPERTY disable_warn_unused_global_const
    "-Wno-unused-const-variables"
    )
set_property (
  TARGET compiler_flags
  PROPERTY disable_warn_empty_func_def
    "-Wno-empty-body"
    )
set_property (
  TARGET compiler_flags
  PROPERTY warn_align
    "-Wcast-align=strict"
    "-Werror=pointer-arith" # Illegal in C11+.
  )
set_property (
  TARGET compiler_flags
  PROPERTY warn_narrowing_cnv
    "-Wconversion" # Implicit conversions which may alter value
    )
# Much easier to catch missing includes before these turn into seemingly
# random link-time errors!
set_property (
  TARGET compiler_flags
  PROPERTY warn_missing_include_dirs
    "-Werror=missing-include-dirs"
    )

# add_library (c_compiler_options INTERFACE)
# define_property (TARGET PROPERTY warn_base INHERITED)

target_compile_options (
  pico_mipi_dbi
  PRIVATE
    $<TARGET_PROPERTY:compiler_flags,warn_base>)

set (DBG_CFG $<BOOL:$<CONFIG:Debug>>)
set (
  GCC_COMPILE_FLAGS
  CACHE LIST
    -Wall
    -Wextra
    -Werror=nonnull
    -Werror=pointer-arith
    -Wcast-qual
    # Warn when a pointer is cast to a type which has stricter alignment,
    # regardless of whether it's possible to access the type on unaligned
    # addresses of that target.
    -Wcast-align=strict
    -Wconversion
    -Wformat-signedness
    -Wold-style-definition
    -Wdeprecated-non-prototype
    -Wno-empty-body
    -Wno-missing-field-initializers
    -Wno-main
    -Wno-unused-const-variable
    -fmessage-length=80
    #-fno-diagnostics-show-line-numbers
    -fdiagnostics-generate-patch
    -fdiagnostics-path-format=inline-events
)
if (${DBG_CFG})
  # get_property (
  #   _PF_STACK_FRAME_SZ
  #   TARGET   ${BUILD_PF}
  #   PROPERTY _PF_STACK_SZ
  # )
  list (
    APPEND
    GCC_COMPILE_FLAGS
      # static analysis
      -fanalyzer
      -fipa-cp
      -ftree-vrp
      -Warray-bounds
      # -Winline
      # It's likely the target does not implement HW accelerated 64-bit float
      # types.
      -Wdouble-promotion
      -Wfloat-equal
      -Wredundant-decls
      -Wmissing-include-dirs
      # All variables and functions with external linkage (which is the default
      # unless otherwise declared `static`) should provide a declaration in a
      # header file or their linkage should be changed so that the global name-
      # space remains as uncluttered as possible.
      -Wmissing-prototypes
      -Wmissing-variable-declarations
      -Wswitch-default
      # `FALLTHRU` or `FALLTHROUGH` disable this warning when added as a comment
      # on the last line of a case and should be used to indicate that this is
      # the intended behavior.
      -Wimplicit-fallthrough=4
      # Warn about using types declared as `packed` when they appear as a
      # member in a struct not declared as a packed type, which geherally is
      # a mistake. The member will be misaligned with little benefit.
      # eg:
      # struct __attribute__((packed)) s {
      #  int x, y, z;
      # };
      # struct a {
      #  uint8_t int_;
      #  struct s str_; // discards 3 bytes of padding that would otherwise be
      #                 // used to align `x`.
      # };
      -Wpacked
      -fstack-usage
      # Property `_PF_STACK_SZ` on the OSAL target linked against this library
      # should be set appropriately so that it reflects the size of the
      # maximum stack frame, whether user-allocated or otherwise. Do note that
      # it's assumed you aren't making recursive calls (please don't). This
      # calculation does not include space for variable-length arrays or
      # memory allocated with `alloca`, so please be aware of this limitation
      # and ensure that the stack space required in these cases does not
      # exceed what your platform mandates.
      -Werror=frame-larger-than=${_PF_STACK_FRAME_SZ}
  )
endif ()
target_compile_options (
  pico_mipi_dbi
  PRIVATE
    ${GCC_COMPILE_FLAGS})

# target_compile_options (
#   mipi_dbi_pico_runtime
#   INTERFACE
#     # ...
# )

# Native config linking SDL to test MGL on development device.
# add_library (
#   mipi_dbi_native
#   STATIC
#     native_pf_osal.c
# )
# target_link_libraries (
#   mipi_dbi_native
#   PRIVATE SDL2
#   PUBLIC  mipi_gfx_lib
# )

target_compile_definitions (
  pico_mipi_dbi
  PRIVATE
    $<${DBG_CFG}:MIPI_DBG_EN>)
