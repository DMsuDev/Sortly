# ==============================================================================
#  SORTLY - TEST HELPERS
# ==============================================================================
#  Description: Helper to build and register GoogleTest executables with the
#               project's compiler and warning policy.
#
#  Copyright (c) 2026 @DMsuDev. Licensed under the MIT License.
#  See LICENSE file in the project root for full license text.
# ==============================================================================

include_guard()

# ------------------------------------------------------------------------------
# sortly_add_tests(<target> LIBS <libs...>)
#
# Builds every *.cpp under the calling directory into one GoogleTest executable
# and registers its tests with CTest.
#   target  -> name of the test executable
#   LIBS    -> libraries under test (linked PRIVATE)
# ------------------------------------------------------------------------------

function(sortly_add_tests target)
  cmake_parse_arguments(ARG "" "" "LIBS" ${ARGN})

  if(NOT TARGET GTest::gtest)
    message(FATAL_ERROR "sortly_add_tests: GoogleTest is not loaded. "
                        "Include cmake/dependencies/gtest.cmake first.")
  endif()

  file(GLOB_RECURSE _sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

  if(NOT _sources)
    message(WARNING "[Sortly] No test sources found in ${CMAKE_CURRENT_SOURCE_DIR}")
    return()
  endif()

  add_executable(${target} ${_sources})

  target_include_directories(${target} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

  target_link_libraries(${target}
    PRIVATE
      ${ARG_LIBS}
      GTest::gtest
      GTest::gtest_main
  )

  sortly_configure_compiler(${target})
  sortly_configure_warnings(${target})

  set_target_properties(${target} PROPERTIES FOLDER "Sortly/Tests")

  gtest_discover_tests(${target}
    PROPERTIES        LABELS "sortly;unit"
    DISCOVERY_TIMEOUT 30
    DISCOVERY_MODE    PRE_TEST
  )
endfunction()
