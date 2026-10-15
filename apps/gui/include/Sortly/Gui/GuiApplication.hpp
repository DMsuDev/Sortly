// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#pragma once

#include "Sortly/Gui/GlfwWindow.hpp"
#include "Sortly/Gui/Panels/IPanel.hpp"
#include "Sortly/Gui/RunController.hpp"
#include "Sortly/Gui/RunSettings.hpp"

#include <Sortly/Gui/Application.hpp>

#include <memory>
#include <vector>

namespace Sortly {

class GuiApplication final : public Application
{
public:
    GuiApplication();

protected:
    bool onInit() override;
    int  onRun() override;
    void onShutdown() override;

private:
    void drawUi();

    GlfwWindow     m_Window;
    RunSettings    m_Settings;
    RunController  m_Run;
    std::vector<std::unique_ptr<IPanel>> m_Panels;
};

} // namespace Sortly
