# ==============================================================================
#  SORTLY - TEST DEPENDENCIES
# ==============================================================================
#  Description: GoogleTest, fetched on demand. Only loaded when a test target
#               is configured (SORTLY_ENABLE_UNIT_TESTS).
#
#  Copyright (c) 2026 @DMsuDev. Licensed under the MIT License.
#  See LICENSE file in the project root for full license text.
# ==============================================================================

include_guard()

include(FetchContent)

# ------------------------------------------------------------------------------
#  GoogleTest
# ------------------------------------------------------------------------------

# Match MSVC runtime settings to avoid mismatches on Windows.
set(gtest_force_shared_crt ON  CACHE BOOL "Use shared CRT for gtest"        FORCE)

# Skip gmock and installation rules to reduce build footprint.
set(BUILD_GMOCK            OFF CACHE BOOL "Disable gmock framework"         FORCE)
set(INSTALL_GTEST          OFF CACHE BOOL "Disable gtest installation rules" FORCE)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.17.0
  GIT_SHALLOW    TRUE
  SYSTEM
)
FetchContent_MakeAvailable(googletest)

# Provides gtest_discover_tests().
include(GoogleTest)
