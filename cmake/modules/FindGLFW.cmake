# ===========================================================================
#  GLFW is consumed as a git submodule compiled via add_subdirectory.
#  This module normalizes the raw subproject target (glfw) under the
#  glfw:: namespace expected by the rest of the build.
#
#  As a fallback, a system or vcpkg config install is also accepted.
#
#  Imported targets:
#    glfw::glfw
#
#  Result variables:
#    GLFW_FOUND
#
#  Copyright (c) 2026 @DMsuDev. Licensed under the MIT License.
#  See LICENSE file in the project root for full license text.
# ===========================================================================

include_guard()

include(FindPackageHandleStandardArgs)

# ---------------------------------------------------------------------------
# If GLFW was already added as a subproject, skip the search entirely.
# ---------------------------------------------------------------------------

if(NOT TARGET glfw)
  find_package(glfw3 CONFIG QUIET)
endif()

if(TARGET glfw)
  if(NOT TARGET glfw::glfw)
    add_library(glfw::glfw INTERFACE IMPORTED)
    set_target_properties(glfw::glfw PROPERTIES INTERFACE_LINK_LIBRARIES glfw)
  endif()

  get_target_property(_glfw_include_dir glfw INTERFACE_INCLUDE_DIRECTORIES)

  find_package_handle_standard_args(GLFW DEFAULT_MSG _glfw_include_dir)
  return()
endif()

# ---------------------------------------------------------------------------
# Fallback: glfw3 config package exposed a namespaced target directly
# (vcpkg and most distros ship glfw3::glfw3 or glfw::glfw already).
# ---------------------------------------------------------------------------

if(TARGET glfw3::glfw3 AND NOT TARGET glfw::glfw)
  add_library(glfw::glfw INTERFACE IMPORTED)
  set_target_properties(glfw::glfw PROPERTIES INTERFACE_LINK_LIBRARIES glfw3::glfw3)

  get_target_property(_glfw_include_dir glfw3::glfw3 INTERFACE_INCLUDE_DIRECTORIES)

  find_package_handle_standard_args(GLFW DEFAULT_MSG _glfw_include_dir)
  return()
endif()

# ---------------------------------------------------------------------------
# Fallback: GLFW not found by any method.
# ---------------------------------------------------------------------------

find_package_handle_standard_args(GLFW
  REQUIRED_VARS GLFW_FOUND
  REASON_FAILURE_MESSAGE
    "GLFW not found. Initialize the git submodule: git submodule update --init third_party/glfw"
)
