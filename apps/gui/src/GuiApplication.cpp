// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "Sortly/Gui/GuiApplication.hpp"
#include "Sortly/Gui/Panels/ControlsPanel.hpp"
#include "Sortly/Gui/Panels/TopBarPanel.hpp"
#include "Sortly/Gui/Panels/VisualizationPanel.hpp"

#include <imgui.h>

#include <chrono>

namespace Sortly {

GuiApplication::GuiApplication()
    : Application({ .name = "Sortly", .verboseLogging = true })
    , m_Run([this](SortAlgorithmType type, const DataSetConfig& config) {
          // Only one run is shown at a time; release the previous one.
          sessions().clear();
          return sessions().createSession(type, config);
      })
{}

bool GuiApplication::onInit()
{
    if (!m_Window.create(1440, 900, "Sortly - Sorting Lab", 1100, 700))
        return false;

    // Adding a screen section = one new IPanel + one line here.
    m_Panels.push_back(std::make_unique<TopBarPanel>());
    m_Panels.push_back(std::make_unique<ControlsPanel>());
    m_Panels.push_back(std::make_unique<VisualizationPanel>());
    return true;
}

int GuiApplication::onRun()
{
    auto previous = std::chrono::steady_clock::now();

    while (!m_Window.shouldClose())
    {
        m_Window.pollEvents();

        const auto now = std::chrono::steady_clock::now();
        const float deltaTime = std::chrono::duration<float>(now - previous).count();
        previous = now;

        m_Run.update(deltaTime);

        m_Window.beginFrame();
        drawUi();
        m_Window.endFrame();
    }

    return 0;
}

void GuiApplication::onShutdown()
{
    m_Panels.clear();
    m_Window.destroy();
}

void GuiApplication::drawUi()
{
    const auto& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(io.DisplaySize, ImGuiCond_Always);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::Begin("Sortly Workspace", nullptr,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus);

    UiContext ctx{ m_Settings, m_Run };
    for (auto& panel : m_Panels)
        panel->draw(ctx);

    ImGui::End();
    ImGui::PopStyleVar(2);
}

} // namespace Sortly
