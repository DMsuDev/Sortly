// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#pragma once

#include <Sortly/App/Application.hpp>

struct GLFWwindow;

namespace Sortly {

class GuiApplication final : public Application
{
public:
    GuiApplication() : Application({ .name = "Sortly", .verboseLogging = true }) {}

protected:
    bool onInit() override;
    int  onRun(int argc, char** argv) override;
    void onShutdown() override;

private:
    void drawUi();

    GLFWwindow* m_Window = nullptr;
};

} // namespace Sortly
