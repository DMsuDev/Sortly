// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

// The library logs through Vigil, which asserts if it is used before Init(). The application
// initializes it at startup; the tests do it once here, for every test in the executable.

#include <vigil/vigil.h>

#include <gtest/gtest.h>

#include <filesystem>
#include <string>

namespace {

class VigilEnvironment final : public ::testing::Environment
{
public:
    void SetUp() override
    {
        const auto dir = std::filesystem::temp_directory_path() / "sortly_tests";
        vigil::LogSystem::Init({
            .Name         = "SortlyTests",
            .LogDir       = dir.string(),
            .LogFile      = "sorting_tests.log",
            .ConsoleLevel = vigil::LogLevel::Off, // keep the test output readable; errors still go to the file
        });
    }

    void TearDown() override { vigil::LogSystem::Shutdown(); }
};

// gtest takes ownership of the environment.
[[maybe_unused]] const auto* const kEnvironment = ::testing::AddGlobalTestEnvironment(new VigilEnvironment);

} // namespace
