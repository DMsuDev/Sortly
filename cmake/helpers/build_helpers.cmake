# ===========================================================================
#  SORTLY - CMAKE HELPERS
# ===========================================================================
#  Description: Reusable CMake functions for build environment configuration,
#               compiler options, warnings, and per-target optimizations.
#
#  Copyright (c) 2026 @DMsuDev. Licensed under the MIT License.
#  See LICENSE file in the project root for full license text.
# ===========================================================================

include_guard()

# ---------------------------------------------------------------------------
# Internal: resolves the correct scope keyword for a given target.
# INTERFACE targets only accept INTERFACE; all others use PRIVATE.
# ---------------------------------------------------------------------------

function(_sortly_target_scope target out_scope)
  get_target_property(_type "${target}" TYPE)
  if(_type STREQUAL "INTERFACE_LIBRARY")
    set(${out_scope} INTERFACE PARENT_SCOPE)
  else()
    set(${out_scope} PRIVATE PARENT_SCOPE)
  endif()
endfunction()

# ------------------------------------------------------------------------------
# sortly_load_version(out_version out_core_version out_prerelease)
#
# Reads ${CMAKE_CURRENT_SOURCE_DIR}/VERSION and sets:
#   out_version       -> full raw version string, e.g. "1.2.3-WIP"
#   out_core_version  -> numeric-only string for project(), e.g. "1.2.3"
#   out_prerelease    -> prerelease identifier, e.g. "WIP"
# ------------------------------------------------------------------------------

function(sortly_load_version out_version out_core_version out_prerelease)
  set(_version_file "${CMAKE_CURRENT_SOURCE_DIR}/VERSION")

  if(NOT EXISTS "${_version_file}")
    message(FATAL_ERROR "[Sortly] VERSION file not found: ${_version_file}")
  endif()

  set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${_version_file}")

  file(READ "${_version_file}" _version)
  string(STRIP "${_version}" _version)

  # Validate the version format before stripping the prerelease suffix.
  if(NOT _version MATCHES "^([0-9]+)\\.([0-9]+)\\.([0-9]+)(-WIP)?$")
    message(FATAL_ERROR
      "[Sortly] Invalid version '${_version}'. "
      "Expected format 'X.Y.Z' or 'X.Y.Z-WIP'."
    )
  endif()

  set(_core_version "${CMAKE_MATCH_1}.${CMAKE_MATCH_2}.${CMAKE_MATCH_3}")

  if(_version MATCHES "-WIP$")
    set(_prerelease "WIP")
  else()
    set(_prerelease "")
  endif()

  set(${out_version}      "${_version}"      PARENT_SCOPE)
  set(${out_core_version} "${_core_version}" PARENT_SCOPE)
  set(${out_prerelease}   "${_prerelease}"   PARENT_SCOPE)
endfunction()

# ---------------------------------------------------------------------------
# Project configuration
# ---------------------------------------------------------------------------

function(sortly_configure_cmake)
  if (DEFINED CMAKE_TOOLCHAIN_FILE)
    message(STATUS "Using toolchain file: \"${CMAKE_TOOLCHAIN_FILE}\"")
  endif()

  if(BUILD_SHARED_LIBS)
    if(APPLE)
      set(CMAKE_INSTALL_RPATH "@loader_path" PARENT_SCOPE)
      set(CMAKE_BUILD_RPATH   "@loader_path" PARENT_SCOPE)
    elseif(UNIX)
      set(CMAKE_INSTALL_RPATH "\$ORIGIN" PARENT_SCOPE)
      set(CMAKE_BUILD_RPATH   "\$ORIGIN" PARENT_SCOPE)
    endif()

    set(CMAKE_POSITION_INDEPENDENT_CODE ON CACHE BOOL "Enable position independent code for all targets" FORCE)
  endif()

  if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "RelWithDebInfo"
      CACHE STRING "Using RelWithDebInfo build type as it was left unset" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
      "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
  endif()
endfunction()

# ---------------------------------------------------------------------------
# Global project configuration
# ---------------------------------------------------------------------------

function(sortly_configure_project)
  message(STATUS "Configuring ${PROJECT_NAME} v${PROJECT_VERSION}")

  if(SORTLY_ENABLE_LTO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT _ipo_supported OUTPUT _ipo_error)

    if(_ipo_supported)
      set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE        ON PARENT_SCOPE)
      set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELWITHDEBINFO ON PARENT_SCOPE)
      set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_MINSIZEREL     ON PARENT_SCOPE)
      message(STATUS "LTO enabled for Release, RelWithDebInfo, MinSizeRel")
    else()
      message(WARNING "LTO/IPO is not supported: ${_ipo_error}")
    endif()
  endif()
endfunction()

# ------------------------------------------------------------------------------
# sortly_strip_dead_code(target)
#
# Enables per-target dead-code stripping for the Release configuration.
# Each toolchain uses its own mechanism:
#   - GCC/Clang (Linux):  -ffunction-sections/-fdata-sections + --gc-sections
#   - AppleClang (macOS): -dead_strip
#   - MSVC (Windows):     /Gy + /OPT:REF
# ------------------------------------------------------------------------------

function(sortly_strip_dead_code target)
  if(NOT TARGET "${target}")
    message(FATAL_ERROR "sortly_strip_dead_code: target '${target}' does not exist.")
  endif()

  _sortly_target_scope("${target}" _scope)

  target_compile_options("${target}" ${_scope}
    $<$<AND:$<CONFIG:Release>,$<CXX_COMPILER_ID:GNU,Clang,AppleClang>>:
      -ffunction-sections
      -fdata-sections
    >
    $<$<AND:$<CONFIG:Release>,$<CXX_COMPILER_ID:MSVC>>:
      /Gy
    >
  )

  target_link_options("${target}" ${_scope}
    $<$<AND:$<CONFIG:Release>,$<PLATFORM_ID:Linux>,$<CXX_COMPILER_ID:GNU,Clang>>:
      LINKER:--gc-sections
    >
    $<$<AND:$<CONFIG:Release>,$<PLATFORM_ID:Darwin>,$<CXX_COMPILER_ID:Clang,AppleClang>>:
      LINKER:-dead_strip
    >
    $<$<AND:$<CONFIG:Release>,$<CXX_COMPILER_ID:MSVC>>:
      /OPT:REF
    >
  )
endfunction()

# ---------------------------------------------------------------------------
# sortly_configure_compiler(target)
#
# Configures compiler options and definitions for the specified target according
# to Sortly's compiler policy. Supports MSVC.
# ---------------------------------------------------------------------------

function(sortly_configure_compiler target)
  if(NOT TARGET "${target}")
    message(FATAL_ERROR "sortly_configure_compiler: target '${target}' does not exist.")
  endif()

  _sortly_target_scope("${target}" _scope)

  set(_options)
  set(_definitions)

  if(MSVC)
    list(APPEND _options
      /utf-8              # Source and execution charset: UTF-8
      /permissive-        # Strict standards conformance (disables MSVC extensions)
      /Zc:__cplusplus     # Report the actual C++ standard in __cplusplus
      /Zc:preprocessor    # Use the conformant preprocessor (replaces legacy one)
      /MP                 # Multi-processor compilation
      /EHsc               # Standard C++ exception handling only
    )

    list(APPEND _definitions
      _CRT_SECURE_NO_WARNINGS   # Suppress MSVC's "use _s variants" noise
      NOMINMAX                  # Prevent <windows.h> from defining min/max macros
      WIN32_LEAN_AND_MEAN       # Exclude rarely-used Windows headers
      VC_EXTRALEAN              # Exclude even more rarely-used Windows headers
    )
  endif()

  target_compile_options(    "${target}" ${_scope} ${_options})
  target_compile_definitions("${target}" ${_scope} ${_definitions})
endfunction()

# ---------------------------------------------------------------------------
# sortly_configure_warnings(target)
#
# Configures compiler warnings for the specified target according to Sortly's
# warning policy. Supports GCC, Clang, AppleClang, and MSVC.
# ---------------------------------------------------------------------------

function(sortly_configure_warnings target)
  if(NOT TARGET "${target}")
    message(FATAL_ERROR "sortly_configure_warnings: target '${target}' does not exist.")
  endif()

  _sortly_target_scope("${target}" _scope)

  set(_warnings)

  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    list(APPEND _warnings
      -Wall
      -Wextra
      -Wpedantic
      -Wshadow              # Local variable shadows an outer one
      -Wnon-virtual-dtor    # Non-virtual destructor in a polymorphic class
      -Wold-style-cast      # C-style casts
      -Wcast-align          # Casts that increase alignment requirements
      -Woverloaded-virtual  # Derived class hides a base virtual method
      -Wconversion          # Implicit conversions that may lose data
      -Wsign-conversion     # Signed/unsigned implicit conversions
      -Wnull-dereference    # Potential null pointer dereferences
      -Wdouble-promotion    # float implicitly promoted to double
      -Wformat=2            # Extra format string checks
    )
  endif()

  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    list(APPEND _warnings
      -Wmisleading-indentation  # Indentation doesn't match control flow
      -Wduplicated-cond         # Duplicated conditions in if/else chains
      -Wduplicated-branches     # if/else branches with identical bodies
      -Wlogical-op              # Suspicious uses of logical operators
      -Wuseless-cast            # Casts to the same type
    )
  endif()

  if(MSVC)
    list(APPEND _warnings
      /W4
      /w14242   # Conversion: possible loss of data (int to char)
      /w14254   # Conversion: possible loss of data (larger to smaller bit field)
      /w14263   # Member function hides base class virtual function
      /w14265   # Class has virtual functions but destructor is not virtual
      /w14287   # Unsigned/negative constant mismatch
      /w14296   # Expression is always false
      /w14311   # Pointer truncation from larger to smaller type
      /w14546   # Function call before comma missing argument list
      /w14555   # Expression has no effect
      /w14826   # Conversion from one type to another is sign-extended
      /w14928   # Illegal copy-initialization
      # ---- Disable ----
      /wd4127   # Conditional expression is constant (noisy with if constexpr)
    )
  endif()

  target_compile_options("${target}" ${_scope} ${_warnings})

  set_target_properties("${target}" PROPERTIES COMPILE_WARNING_AS_ERROR ${SORTLY_STRICT_WARNINGS})
endfunction()

# ------------------------------------------------------------------------------
# sortly_disable_third_party_warnings(target)
#
# Silences all warnings for a third-party target so it is compiled
# independently from Sortly's warning policy.
# ------------------------------------------------------------------------------

function(sortly_disable_third_party_warnings target)
  if(NOT TARGET "${target}")
    message(FATAL_ERROR "sortly_disable_third_party_warnings: target '${target}' does not exist.")
  endif()

  _sortly_target_scope("${target}" _scope)

  set_target_properties("${target}" PROPERTIES COMPILE_WARNING_AS_ERROR OFF)

  target_compile_options("${target}" ${_scope}
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-w>
    $<$<CXX_COMPILER_ID:MSVC>:/W0>
  )
endfunction()
