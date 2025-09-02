# This file defines the 'xvcfiopt' library.
add_compiler_rt_component(xvcfiopt)

# if(OS_NAME MATCHES "Linux" OR OS_NAME MATCHES "FreeBSD" OR OS_NAME MATCHES "NetBSD")

  # List the source files for this library.
  set(CFI_XDSO_CACHE_SOURCES
    cfi_xdso_cache.cpp
    )

  # Include headers from parent directories (e.g., sanitizer_common)
  include_directories(..)

  # Set the special compile flag for your source file.
  set_source_files_properties(cfi_xdso_cache.cpp
    PROPERTIES
    COMPILE_FLAGS "-march=native"
    )

  # Loop over all supported architectures to create a library for each.
  foreach(arch ${CFI_SUPPORTED_ARCH})
    add_compiler_rt_runtime(clang_rt.xvcfiopt
      STATIC
      ARCHS ${arch}
      # Use the correct variable for your sources.
      SOURCES ${CFI_XDSO_CACHE_SOURCES}
      # Use the standard sanitizer common flags.
      CFLAGS ${SANITIZER_COMMON_CFLAGS}
      PARENT_TARGET xvcfiopt)
  endforeach()

# endif()