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
};

/// @brief Base class for the desktop application. Run() performs the shared
/// logging and crash-handler bootstrap, then drives the lifecycle:
/// onInit() -> onRun() -> onShutdown().
class Application
{
public:
    explicit Application(ApplicationConfig config = {});
    virtual ~Application() = default;

    Application(const Application&)            = delete;
    Application& operator=(const Application&) = delete;

    /// @return Process exit code.
    [[nodiscard]] int Run();

protected:
    /// @brief Frontend-specific setup. Returning false aborts before onRun().
    virtual bool onInit() { return true; }

    /// @brief Main body of the frontend. Its return value is the exit code.
    virtual int onRun() = 0;

    /// @brief Cleanup; only called if onInit() succeeded.
    virtual void onShutdown() {}

    [[nodiscard]] SessionManager&          sessions()       { return m_Sessions; }
    [[nodiscard]] const ApplicationConfig& config()   const { return m_Config; }

private:
    ApplicationConfig m_Config;
    SessionManager    m_Sessions;
};

} // namespace Sortly
