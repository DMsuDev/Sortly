// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#pragma once

#include <foundry/types/semantic_version.h>
#include <foundry/platform/paths.h>

#include <string_view>
#include <optional>
#include <string>

using namespace foundry::types;

namespace Sortly::System
{
    namespace impl
    {
        void setGlobalScale(float scale);
        void setScaleMultiplier(float scale);

        void setGPUVendor(const std::string &vendor);
        void setGLRenderer(const std::string &renderer);
        void setGLVersion(SemanticVersion version);

    } // namespace impl

    /// @brief Gets the current global display scale (1.0 = 96 dpi). UI thread only.
    [[nodiscard]] float getGlobalScale();

    /// @brief Gets the current Sortly version, including any prerelease suffix.
    [[nodiscard]] const SemanticVersion& getSortlyVersion();

    /// @brief Gets the current operating system name.
    [[nodiscard]] std::string_view getOSName();

    /// @brief Gets the OS version, or "Unknown" if it cannot be determined.
    /// On Linux and macOS, this is the kernel release and version.
    [[nodiscard]] std::string getOSVersion();

    /// @brief Returns the architecture of the system (e.g., x86_64, arm64).
    [[nodiscard]] std::string getArchitecture();

    /// @brief Gets the GPU vendor name.
    [[nodiscard]] const std::string& getGPUVendor();

    /// @brief Gets the GL renderer string.
    [[nodiscard]] const std::string& getGLRenderer();

    /// @brief Gets the GL version as a semantic version.
    [[nodiscard]] const SemanticVersion& getGLVersion();

    /// @brief Checks whether the CMake build configuration is Debug.
    [[nodiscard]] bool isDebugBuild();

    /// Looks for assets/<relative> next to the executable, then in the working directory.
    [[nodiscard]] std::optional<std::filesystem::path> findAsset(std::string_view relative);

} // namespace Sortly::System
