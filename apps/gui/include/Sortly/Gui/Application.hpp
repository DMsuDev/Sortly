// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#pragma once

#include "Sortly/Gui/SessionManager.hpp"

#include <string>

namespace Sortly {

struct ApplicationConfig
{
    std::string name           = "Sortly";
    bool        verboseLogging = false;
    // The GUI build has no console, so a log file is the only place diagnostics can go.
    std::string logDir         = "logs";
    std::string logFile        = "sortly.log";
};

/// @brief Base class for the desktop application.
///
/// Run() performs the shared logging and crash-handler bootstrap, then drives
/// the lifecycle: onInit() -> onRun() -> onShutdown()
///
/// @note onShutdown() is only called if onInit() returns true.
class Application
{
public:
    explicit Application(ApplicationConfig config = {});
    virtual ~Application() = default;

    Application(const Application&)            = delete;
    Application& operator=(const Application&) = delete;
    Application(Application&&)                 = delete;
    Application& operator=(Application&&)      = delete;

    /// @return Process exit code.
    [[nodiscard]] int Run();

protected:
    /// @brief Frontend-specific setup. Returning false aborts before onRun().
    [[nodiscard]] virtual bool onInit() { return true; }

    /// @brief Main body of the frontend. Its return value is the exit code.
    [[nodiscard]] virtual int onRun() = 0;

    /// @brief Frontend-specific cleanup. Only called if onInit() succeeded.
    virtual void onShutdown() {}

    [[nodiscard]] SessionManager&          sessions()       noexcept { return m_Sessions; }
    [[nodiscard]] const ApplicationConfig& config()   const noexcept { return m_Config; }

private:
    ApplicationConfig m_Config;
    SessionManager    m_Sessions;
};

} // namespace Sortly
