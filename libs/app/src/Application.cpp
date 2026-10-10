// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "Sortly/App/Application.hpp"
#include "Sortly/version.hpp"

#include <vigil/vigil.h>

#include <cstdio>
#include <exception>
#include <utility>

namespace Sortly {

Application::Application(ApplicationConfig config)
    : m_Config(std::move(config))
{
}

int Application::Run(int argc, char** argv)
{
    if (!vigil::EnableUTF8Console())
    {
        std::fprintf(stderr,
            "[Vigil] Failed to enable UTF-8 console support. "
            "Unicode output may not display correctly.\n");
    }

    vigil::LogSystem::Init({
        .Name         = m_Config.name.c_str(),
        .ConsoleLevel = m_Config.verboseLogging ? vigil::LogLevel::Trace
                                                : vigil::LogLevel::Info
    });

    vigil::CrashHandler::Install();

    VIGIL_LOG_INFO("{} version {}", m_Config.name, version::current.to_string());

    try
    {
        if (!onInit())
            return 1;

        const int code = onRun(argc, argv);
        onShutdown();
        return code;
    }
    catch (const std::exception& e)
    {
        VIGIL_LOG_ERROR("Unhandled exception: {}", e.what());
        return 1;
    }
}

} // namespace Sortly
