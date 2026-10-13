# ==============================================================================
#  SORTLY - CORE DEPENDENCIES
# ==============================================================================
#  Description: Third-party libraries required by the sorting and application
#               libraries. Desktop graphics dependencies live in gui.cmake.
#
#  Copyright (c) 2026 @DMsuDev. Licensed under the MIT License.
#  See LICENSE file in the project root for full license text.
# ==============================================================================

include_guard()

# ------------------------------------------------------------------------------
#  nlohmann/json (header-only)
# ------------------------------------------------------------------------------
add_library(nlohmann_json INTERFACE IMPORTED GLOBAL)

target_include_directories(nlohmann_json SYSTEM INTERFACE
  "${SORTLY_THIRD_PARTY_DIR}/nlohmann"
)

# ------------------------------------------------------------------------------
#  vigil
# ------------------------------------------------------------------------------
set(VIGIL_INSTALL          OFF CACHE BOOL "" FORCE)
set(VIGIL_BUILD_TESTS      OFF CACHE BOOL "" FORCE)
set(VIGIL_BUILD_EXAMPLES   OFF CACHE BOOL "" FORCE)
set(VIGIL_BUILD_WARNINGS   OFF CACHE BOOL "" FORCE)
set(VIGIL_USE_SYSTEM_FMT   OFF CACHE BOOL "" FORCE)

add_subdirectory("${SORTLY_THIRD_PARTY_DIR}/vigil" "${PROJECT_BINARY_DIR}/third_party/vigil")
sortly_disable_third_party_warnings(vigil)

if(NOT TARGET Sortly::Vigil)
  add_library(Sortly::Vigil ALIAS vigil)
endif()

# ------------------------------------------------------------------------------
#  foundry
# ------------------------------------------------------------------------------
add_subdirectory("${SORTLY_THIRD_PARTY_DIR}/foundry" "${PROJECT_BINARY_DIR}/third_party/foundry")
sortly_disable_third_party_warnings(Foundry)

if(NOT TARGET Sortly::Foundry)
  add_library(Sortly::Foundry ALIAS Foundry)
endif()
