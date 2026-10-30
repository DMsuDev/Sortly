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
  PRIVATE
    nlohmann_json
)

# The lib holds all the real code, so the compiler policy must apply to it as well.
sortly_configure_compiler(SortlyGuiLib)
sortly_configure_warnings(SortlyGuiLib)
sortly_strip_dead_code(SortlyGuiLib)

set_target_properties(SortlyGuiLib PROPERTIES FOLDER "Sortly/Apps")

# ---------------------------------------------------------------------------
#  Executable
# ---------------------------------------------------------------------------
add_executable(SortlyGui src/main.cpp)

set_target_properties(SortlyGui PROPERTIES
  WIN32_EXECUTABLE $<NOT:$<CONFIG:Debug>>
)
target_link_libraries(SortlyGui PRIVATE Sortly::GuiLib)

# Copy the whole assets folder (fonts + licenses) next to the executable.
add_custom_command(TARGET SortlyGui POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          "${SORTLY_BASE_FOLDER}/assets"
          "$<TARGET_FILE_DIR:SortlyGui>/assets"
)

sortly_configure_compiler(SortlyGui)
sortly_configure_warnings(SortlyGui)
sortly_strip_dead_code(SortlyGui)

set_target_properties(SortlyGui PROPERTIES
  FOLDER      "Sortly/Apps"
  OUTPUT_NAME sortly
)
