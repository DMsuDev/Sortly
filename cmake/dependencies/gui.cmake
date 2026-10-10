# ==============================================================================
#  SORTLY - GUI DEPENDENCIES
# ==============================================================================
#  Description: Third-party libraries required only by the graphical frontend
#               (apps/gui): OpenGL, GLFW, Dear ImGui and ImPlot.
#
#  Copyright (c) 2026 @DMsuDev. Licensed under the MIT License.
#  See LICENSE file in the project root for full license text.
# ==============================================================================

include_guard()

# ------------------------------------------------------------------------------
#  OpenGL
# ------------------------------------------------------------------------------
find_package(OpenGL REQUIRED)

# ------------------------------------------------------------------------------
#  GLFW
# ------------------------------------------------------------------------------
set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL        OFF CACHE BOOL "" FORCE)

add_subdirectory("${SORTLY_THIRD_PARTY_DIR}/glfw" "${PROJECT_BINARY_DIR}/third_party/glfw")
find_package(GLFW REQUIRED)
sortly_disable_third_party_warnings(glfw)

# ------------------------------------------------------------------------------
#  Dear ImGui  +  GLFW / OpenGL3 backends
# ------------------------------------------------------------------------------
add_library(SortlyImGui STATIC
  "${SORTLY_THIRD_PARTY_DIR}/imgui/imgui.cpp"
  "${SORTLY_THIRD_PARTY_DIR}/imgui/imgui_draw.cpp"
  "${SORTLY_THIRD_PARTY_DIR}/imgui/imgui_tables.cpp"
  "${SORTLY_THIRD_PARTY_DIR}/imgui/imgui_widgets.cpp"

  "${SORTLY_THIRD_PARTY_DIR}/imgui/backends/imgui_impl_glfw.cpp"
  "${SORTLY_THIRD_PARTY_DIR}/imgui/backends/imgui_impl_opengl3.cpp"
)

add_library(Sortly::ImGui ALIAS SortlyImGui)

target_include_directories(SortlyImGui SYSTEM PUBLIC
  "${SORTLY_THIRD_PARTY_DIR}/imgui"
  "${SORTLY_THIRD_PARTY_DIR}/imgui/backends"
)

target_link_libraries(SortlyImGui PUBLIC
  glfw::glfw
  OpenGL::GL
)

set_target_properties(SortlyImGui PROPERTIES FOLDER "third_party")
sortly_disable_third_party_warnings(SortlyImGui)

# ------------------------------------------------------------------------------
#  ImPlot
# ------------------------------------------------------------------------------
add_library(SortlyImPlot STATIC
  "${SORTLY_THIRD_PARTY_DIR}/implot/implot.cpp"
  "${SORTLY_THIRD_PARTY_DIR}/implot/implot_items.cpp"
)

add_library(Sortly::ImPlot ALIAS SortlyImPlot)

target_include_directories(SortlyImPlot SYSTEM PUBLIC
  "${SORTLY_THIRD_PARTY_DIR}/implot"
)

target_link_libraries(SortlyImPlot PUBLIC Sortly::ImGui)

set_target_properties(SortlyImPlot PROPERTIES FOLDER "third_party")
sortly_disable_third_party_warnings(SortlyImPlot)
