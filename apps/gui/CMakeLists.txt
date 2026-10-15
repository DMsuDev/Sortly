# ==============================================================================
#  SORTLY - GUI APPLICATION
# ==============================================================================
#  Description: Graphical frontend (Dear ImGui + GLFW + OpenGL).
#
#  Copyright (c) 2026 @DMsuDev. Licensed under the MIT License.
#  See LICENSE file in the project root for full license text.
# ==============================================================================

include("${SORTLY_BASE_FOLDER}/cmake/dependencies/gui.cmake")

file(GLOB_RECURSE SORTLY_GUI_SRC CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
list(REMOVE_ITEM SORTLY_GUI_SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")

# ---------------------------------------------------------------------------
#  Lib (everything except main.cpp)
# ---------------------------------------------------------------------------
add_library(SortlyGuiLib STATIC ${SORTLY_GUI_SRC})
add_library(Sortly::GuiLib ALIAS SortlyGuiLib)

target_include_directories(SortlyGuiLib PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_link_libraries(SortlyGuiLib
  PUBLIC
    Sortly::Sorting
    Sortly::Version
    Sortly::ImGui
    Sortly::ImPlot
    Sortly::Vigil
    glfw::glfw
    nlohmann_json
)

# Lucide icon font: found next to the executable (copied below) or in the source tree.
target_compile_definitions(SortlyGuiLib PRIVATE
  SORTLY_ASSETS_DIR="${SORTLY_THIRD_PARTY_DIR}/imgui"
)

set_target_properties(SortlyGuiLib PROPERTIES FOLDER "apps/gui")

# ---------------------------------------------------------------------------
#  Executable
# ---------------------------------------------------------------------------
add_executable(SortlyGui WIN32 src/main.cpp)
target_link_libraries(SortlyGui PRIVATE Sortly::GuiLib)

add_custom_command(TARGET SortlyGui POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:SortlyGui>/assets/fonts"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          "${SORTLY_THIRD_PARTY_DIR}/imgui/lucide.ttf"
          "$<TARGET_FILE_DIR:SortlyGui>/assets/fonts/lucide.ttf"
)

sortly_configure_compiler(SortlyGui)
sortly_configure_warnings(SortlyGui)
sortly_strip_dead_code(SortlyGui)

set_target_properties(SortlyGui PROPERTIES
  FOLDER      "apps/gui"
  OUTPUT_NAME "Sortly"
)
