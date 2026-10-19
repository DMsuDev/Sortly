// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#pragma once

#include "Sortly/Gui/Application.hpp"
#include "Sortly/Gui/GlfwWindow.hpp"
#include "Sortly/Gui/ImGuiLayer.hpp"
#include "Sortly/Gui/Panels/IPanel.hpp"
#include "Sortly/Gui/RunController.hpp"
#include "Sortly/Gui/RunSettings.hpp"

#include <memory>
#include <vector>

namespace Sortly {

class GuiApplication final : public Application
{
public:
    GuiApplication();

protected:
    [[nodiscard]] bool onInit() override;
    [[nodiscard]] int  onRun() override;
    void               onShutdown() override;

private:
    void teardown() noexcept;
    void renderFrame();
    void drawUi();
    void layoutPanels(UiContext& ctx);

    // Members are destroyed in reverse order: panels, then the UI layer, then the window.
    GlfwWindow    m_Window;
    ImGuiLayer    m_Gui;
    RunSettings   m_Settings;
    RunController m_Run;
    std::vector<std::unique_ptr<IPanel>> m_Panels;
};

} // namespace Sortly
