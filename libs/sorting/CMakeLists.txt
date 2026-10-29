# ==============================================================================
#  SORTLY - SORTING LIBRARY
# ==============================================================================
#  Description: Pure sorting domain (algorithms, data, operations, playback,
#               sessions). Must never depend on any UI or CLI library.
#
#  Copyright (c) 2026 @DMsuDev. Licensed under the MIT License.
#  See LICENSE file in the project root for full license text.
# ==============================================================================

# Every .cpp under src/ is part of the library: adding an algorithm needs no CMake edit
# (CONFIGURE_DEPENDS re-runs the glob on the next build).
file(GLOB_RECURSE SORTLY_SORTING_SRC CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_library(SortlySorting STATIC ${SORTLY_SORTING_SRC})
add_library(Sortly::Sorting ALIAS SortlySorting)

target_include_directories(SortlySorting
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_link_libraries(SortlySorting PRIVATE Sortly::Vigil)

sortly_configure_compiler(SortlySorting)
sortly_configure_warnings(SortlySorting)
sortly_strip_dead_code(SortlySorting)

set_target_properties(SortlySorting PROPERTIES FOLDER "Sortly/Libs")

if(SORTLY_ENABLE_UNIT_TESTS)
  add_subdirectory(tests)
endif()
