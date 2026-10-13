# ==============================================================================
#  SORTLY - APP LIBRARY
# ==============================================================================
#  Description: Desktop application lifecycle and session management. This
#               library stays independent from ImGui and GLFW.
#
#  Copyright (c) 2026 @DMsuDev. Licensed under the MIT License.
#  See LICENSE file in the project root for full license text.
# ==============================================================================

add_library(SortlyApp STATIC
  src/Application.cpp
  src/SessionManager.cpp
)

add_library(Sortly::App ALIAS SortlyApp)

target_include_directories(SortlyApp
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(SortlyApp
  PUBLIC
    Sortly::Sorting
    Sortly::Version
)

sortly_configure_compiler(SortlyApp)
sortly_configure_warnings(SortlyApp)
sortly_strip_dead_code(SortlyApp)

set_target_properties(SortlyApp PROPERTIES FOLDER "Sortly/Libs")

if(SORTLY_ENABLE_UNIT_TESTS)
  add_subdirectory(tests)
endif()
