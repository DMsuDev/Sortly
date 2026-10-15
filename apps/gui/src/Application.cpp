// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "Sortly/Gui/Application.hpp"
#include "Sortly/version.hpp"

#include <vigil/vigil.h>

#include <exception>
#include <utility>

namespace Sortly {

Application::Application(ApplicationConfig config)
    : m_Config(std::move(config))
{
}

int Application::Run()
{
    vigil::LogSystem::Init({
        .Name         = m_Config.name.c_str(),
        .LogDir       = m_Config.logDir,
        .LogFile      = m_Config.logFile,
        .ConsoleLevel = m_Config.verboseLogging ? vigil::LogLevel::Trace
                                                : vigil::LogLevel::Info
    });

    vigil::CrashHandler::Install();

    VIGIL_LOG_INFO("{} version {}", m_Config.name, version::current.to_string());

    try
    {
        if (!onInit())
            return 1;

        const int code = onRun();
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
