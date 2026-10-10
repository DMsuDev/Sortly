# ==============================================================================
#  SORTLY - CLI APPLICATION
# ==============================================================================
#  Description: Command-line frontend (headless, no graphics dependencies).
#
#  Copyright (c) 2026 @DMsuDev. Licensed under the MIT License.
#  See LICENSE file in the project root for full license text.
# ==============================================================================

# ------------------------------------------------------------------------------
#  CLI11 (header-only)
# ------------------------------------------------------------------------------
add_library(CLI11::CLI11 INTERFACE IMPORTED GLOBAL)

target_include_directories(CLI11::CLI11 SYSTEM INTERFACE
  "${SORTLY_THIRD_PARTY_DIR}/CLI11/include"
)

add_executable(SortlyCli main.cpp CliApplication.cpp)

target_include_directories(SortlyCli PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(SortlyCli
  PRIVATE
    Sortly::App
    CLI11::CLI11
)

sortly_configure_compiler(SortlyCli)
sortly_configure_warnings(SortlyCli)
sortly_strip_dead_code(SortlyCli)

# ------------------------------------------------------------------------------
#  Black-box tests (CTest)
# ------------------------------------------------------------------------------
if(SORTLY_ENABLE_UNIT_TESTS)
  add_test(NAME cli.list          COMMAND SortlyCli list)
  add_test(NAME cli.run           COMMAND SortlyCli run -a cocktail -n 64 -s 42)
  add_test(NAME cli.invalid_count COMMAND SortlyCli run -n 1)

  set_tests_properties(cli.list cli.run cli.invalid_count
    PROPERTIES LABELS "sortly;cli"
  )
  set_tests_properties(cli.invalid_count PROPERTIES WILL_FAIL TRUE)
endif()

set_target_properties(SortlyCli PROPERTIES
  OUTPUT_NAME sortly-cli
  FOLDER      "Sortly/Apps"
)
