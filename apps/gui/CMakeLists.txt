# ==============================================================================
#  SORTLY - GUI APPLICATION
# ==============================================================================
#  Description: Graphical frontend (Dear ImGui + GLFW + OpenGL).
#
#  Copyright (c) 2026 @DMsuDev. Licensed under the MIT License.
#  See LICENSE file in the project root for full license text.
# ==============================================================================

include("${SORTLY_BASE_FOLDER}/cmake/dependencies/gui.cmake")

add_executable(SortlyGui main.cpp GuiApplication.cpp)

target_include_directories(SortlyGui PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(SortlyGui
  PRIVATE
    Sortly::App
    Sortly::ImGui
    Sortly::ImPlot
    glfw::glfw
    nlohmann_json
)

sortly_configure_compiler(SortlyGui)
sortly_configure_warnings(SortlyGui)
sortly_strip_dead_code(SortlyGui)

set_target_properties(SortlyGui PROPERTIES
  OUTPUT_NAME sortly
  FOLDER      "Sortly/Apps"
)

