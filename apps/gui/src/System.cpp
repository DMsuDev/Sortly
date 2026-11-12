// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "Sortly/Gui/System.hpp"

#include <foundry/platform/paths.h>
#include <vigil/logging/log_system.h>

#include <cerrno>
#include <cstring>

#if defined(OS_WINDOWS)
    #include <Windows.h>

    extern "C" LONG WINAPI RtlGetVersion(OSVERSIONINFOW* version);
#elif defined(OS_LINUX) || defined(OS_MACOS)
    #include <sys/utsname.h>
#endif

namespace Sortly::System
{
    namespace impl
    {

        static float s_scaleMultiplier = 1.0f;
        void setScaleMultiplier(float scale) {
            s_scaleMultiplier = scale;
        }

        static float s_globalScale = 1.0f;
        void setGlobalScale(float scale) {
            s_globalScale = scale;
        }

        static std::string s_gpuVendor;
        void setGPUVendor(const std::string &vendor) {
            s_gpuVendor = vendor;
        }

        static std::string s_glRenderer;
        void setGLRenderer(const std::string &renderer) {
            s_glRenderer = renderer;
        }

        // Meyers' singleton for OpenGL version.
        static SemanticVersion& glVersion() {
            static SemanticVersion s_openGLVersion{0, 0, 0};
            return s_openGLVersion;
        }
        void setGLVersion(SemanticVersion version) {
            glVersion() = std::move(version);
        }

    } // namespace impl

    float getGlobalScale() {
        return impl::s_globalScale * impl::s_scaleMultiplier;
    }

    const SemanticVersion& getSortlyVersion() {
        static const SemanticVersion version{
            SORTLY_VERSION_MAJOR,
            SORTLY_VERSION_MINOR,
            SORTLY_VERSION_PATCH,
            SORTLY_VERSION_PRERELEASE
        };
        return version;
    }

    std::string_view getOSName() {
        #if defined(OS_WINDOWS)
            return "Windows";
        #elif defined(OS_MACOS)
            return "macOS";
        #elif defined(OS_LINUX)
            return "Linux";
        #else
            return "Unknown";
        #endif
    }

    std::string getOSVersion() {
        #if defined(OS_WINDOWS)
            OSVERSIONINFOW info{};
            info.dwOSVersionInfoSize = sizeof(info);
            const LONG status = RtlGetVersion(&info);
            if (status != 0) {
                VIGIL_LOG_WARN("Could not query Windows version (NTSTATUS: {})", status);
                return "Unknown";
            }

            return std::to_string(info.dwMajorVersion) + "." +
                   std::to_string(info.dwMinorVersion) + "." +
                   std::to_string(info.dwBuildNumber);
        #elif defined(OS_LINUX) || defined(OS_MACOS)
            struct utsname details{};
            if (uname(&details) != 0) {
                VIGIL_LOG_WARN("Could not query OS version: {}", std::strerror(errno));
                return "Unknown";
            }

            return std::string(details.release) + " " + details.version;
        #else
            return "Unknown";
        #endif
    }

    std::string getArchitecture() {
        #if defined(OS_WINDOWS)
            SYSTEM_INFO info;
            ::GetNativeSystemInfo(&info);

            switch (info.wProcessorArchitecture) {
                case PROCESSOR_ARCHITECTURE_AMD64:
                    return "x86_64";
                case PROCESSOR_ARCHITECTURE_ARM:
                    return "ARM";
                case PROCESSOR_ARCHITECTURE_ARM64:
                    return "ARM64";
                case PROCESSOR_ARCHITECTURE_IA64:
                    return "IA64";
                case PROCESSOR_ARCHITECTURE_INTEL:
                    return "x86";
                default:
                    return "Unknown";
            }
        #elif defined(OS_LINUX) || defined(OS_MACOS)
            struct utsname details = { };

            if (uname(&details) != 0) {
                return "Unknown";
            }

            return { details.machine };
        #else
            return "Unknown";
        #endif
    }

    const std::string &getGPUVendor() { return impl::s_gpuVendor; }
    const std::string &getGLRenderer() { return impl::s_glRenderer; }
    const SemanticVersion &getGLVersion() { return impl::glVersion(); }

    bool isDebugBuild() {
        #if defined(SORTLY_DEBUG)
            return true;
        #else
            return false;
        #endif
    }

    std::optional<std::filesystem::path> findAsset(std::string_view relative)
    {
        namespace fs = std::filesystem;

        std::error_code ec;
        const fs::path assets = fs::path("assets") / fs::path(std::string(relative));
        const fs::path candidates[] = {
            foundry::platform::executable_directory(ec) / assets,
            assets,
        };

        for (const fs::path &candidate : candidates)
            if (fs::exists(candidate, ec))
                return candidate;

        return std::nullopt;
    }

} // namespace Sortly::System
