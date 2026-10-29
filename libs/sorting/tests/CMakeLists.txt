# ==============================================================================
#  SORTLY - SORTING TESTS
# ==============================================================================
#  Description: Unit tests for the sorting domain (algorithms, data, operations,
#               playback, sessions).
#
#  Copyright (c) 2026 @DMsuDev. Licensed under the MIT License.
#  See LICENSE file in the project root for full license text.
# ==============================================================================

sortly_add_tests(sortly_sorting_tests
  LIBS
    Sortly::Sorting
    Sortly::Vigil
)

# White-box tests: some of them need the library's private headers (src/).
target_include_directories(sortly_sorting_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../src")
