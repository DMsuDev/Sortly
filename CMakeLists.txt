# ==============================================================================
#  SORTLY - ROOT CMAKE CONFIGURATION
# ==============================================================================
#  Description: Main project orchestrator. Manages configuration options,
#               global build settings, internal dependencies, and subdirectories.
#
#  Copyright (c) 2026 @DMsuDev. Licensed under the MIT License.
#  See LICENSE file in the project root for full license text.
# ==============================================================================

cmake_minimum_required(VERSION 3.28)

# ------------------------------------------------------------------------------
#  Project Options
# ------------------------------------------------------------------------------
option(SORTLY_ENABLE_LTO          "Enables Link Time Optimizations if possible"              OFF)
option(SORTLY_ENABLE_CXX_MODULES  "Enable C++20 Module compilation. Testing only!"           OFF)
option(SORTLY_ENABLE_UNIT_TESTS   "Enable building unit tests"                               ON )
option(SORTLY_STRICT_WARNINGS     "Enable most available warnings and treat them as errors"  OFF)

# ------------------------------------------------------------------------------
#  Paths, Modules & Helpers
# ------------------------------------------------------------------------------
set(SORTLY_BASE_FOLDER      "${CMAKE_CURRENT_SOURCE_DIR}")
set(SORTLY_THIRD_PARTY_DIR  "${SORTLY_BASE_FOLDER}/third_party")

list(APPEND CMAKE_MODULE_PATH "${SORTLY_BASE_FOLDER}/cmake/modules")

# ------------------------------------------------------------------------------
#  Project Configuration
# ------------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_SCAN_FOR_MODULES ${SORTLY_ENABLE_CXX_MODULES})
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include("${SORTLY_BASE_FOLDER}/cmake/helpers/build_helpers.cmake")

# ------------------------------------------------------------------------------
#  Project Version
# ------------------------------------------------------------------------------
sortly_load_version(SORTLY_VERSION SORTLY_VERSION_CORE SORTLY_VERSION_PRERELEASE)

project(Sortly
  LANGUAGES   CXX
  VERSION     ${SORTLY_VERSION_CORE}
  DESCRIPTION "Desktop sorting algorithm visualizer"
)
message(STATUS "${PROJECT_NAME} version: ${SORTLY_VERSION}")

# ------------------------------------------------------------------------------
#  Configure Project
# ------------------------------------------------------------------------------
sortly_configure_cmake()
sortly_configure_project()
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# ------------------------------------------------------------------------------
#  Output Directories & Installation Paths
# ------------------------------------------------------------------------------

# Standard install directories (bin, lib, include, etc.)
include(GNUInstallDirs)

# Local build output directories (binaries, static/dynamic libs)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}")

# ------------------------------------------------------------------------------
#  Core Dependencies
# ------------------------------------------------------------------------------
include("${SORTLY_BASE_FOLDER}/cmake/dependencies/core.cmake")

# ------------------------------------------------------------------------------
#  Generated Version Header
# ------------------------------------------------------------------------------
configure_file(
  "${SORTLY_BASE_FOLDER}/cmake/templates/version.hpp.in"
  "${PROJECT_BINARY_DIR}/generated/include/Sortly/version.hpp"
  @ONLY
)

add_library(SortlyVersion INTERFACE)
add_library(Sortly::Version ALIAS SortlyVersion)

target_include_directories(SortlyVersion INTERFACE
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/generated/include>
)
target_link_libraries(SortlyVersion INTERFACE Sortly::Foundry)

# ------------------------------------------------------------------------------
#  Test Infrastructure
# ------------------------------------------------------------------------------
include("${SORTLY_BASE_FOLDER}/cmake/helpers/test_helpers.cmake")

if(SORTLY_ENABLE_UNIT_TESTS)
  message(STATUS "Sortly: Tests suite execution is enabled.")
  enable_testing()
  include("${SORTLY_BASE_FOLDER}/cmake/dependencies/gtest.cmake")
endif()

# ------------------------------------------------------------------------------
#  Libraries
# ------------------------------------------------------------------------------

add_subdirectory(libs/sorting)

# ------------------------------------------------------------------------------
#  Desktop Application
# ------------------------------------------------------------------------------
add_subdirectory(apps/gui)
