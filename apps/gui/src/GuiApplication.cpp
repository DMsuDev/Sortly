// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "Sortly/Gui/GuiApplication.hpp"
#include "Sortly/Gui/UiTheme.hpp"
#include "Panels/ControlsPanel.hpp"
#include "Panels/SummaryPanel.hpp"
#include "Panels/TopBarPanel.hpp"
#include "Panels/VisualizationPanel.hpp"

#include <imgui.h>

#include <algorithm>
#include <chrono>

namespace Sortly {

namespace {

// Dragging or resizing the window blocks the loop on some platforms; without a cap the
// playback would apply thousands of steps at once when it resumes.
constexpr float kMaxFrameSeconds = 0.1f;

} // namespace

GuiApplication::GuiApplication()
    : Application({ .name = "Sortly", .verboseLogging = true })
    , m_Run([this](SortAlgorithmType type, const RunSettings& settings) {
          // Every algorithm of a run gets the same seed, hence the same data to sort.
          sessions().setSeedProvider(SeedProvider::globalSeed(settings.seed));

          DataSetConfig config;
          config.count = settings.elementCount;
          return sessions().createSession(type, config);
      })
{}

bool GuiApplication::onInit()
{
    if (!m_Window.create(1440, 900, "Sortly - Sorting Lab", 1180, 720))
        return false;

    // Adding a screen section = one new IPanel (with its dock) + one line here.
    m_Panels.push_back(std::make_unique<TopBarPanel>());
    m_Panels.push_back(std::make_unique<ControlsPanel>());
    m_Panels.push_back(std::make_unique<VisualizationPanel>());
    m_Panels.push_back(std::make_unique<SummaryPanel>());

    // Start with the default selection already built so the workspace is never empty.
    m_Run.requestStart(m_Settings);
    return true;
}

int GuiApplication::onRun()
{
    auto previous = std::chrono::steady_clock::now();

    while (!m_Window.shouldClose())
    {
        m_Window.pollEvents();

        const auto now = std::chrono::steady_clock::now();
        const float deltaTime = std::min(std::chrono::duration<float>(now - previous).count(), kMaxFrameSeconds);
        previous = now;

        m_Run.setDelayMs(m_Settings.stepDelayMs());
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

    // Layout lives here: Top panels get a full-width row, every other panel is placed
    // next to the previous one on the row below. Fill panels take what the fixed-width
    // Left/Right panels leave over.
    const float gap = ui::dp(14.0f);
    float reserved  = 0.0f;
    for (const auto& panel : m_Panels)
        if (panel->dock() == PanelDock::Right)
            reserved += gap + panel->width();

    UiContext ctx{ m_Settings, m_Run };
    bool rowStarted = false;
    for (auto& panel : m_Panels)
    {
        if (panel->dock() == PanelDock::Top)
        {
            ctx.width = 0.0f;
            panel->draw(ctx);
            rowStarted = false;
            continue;
        }

        ctx.width = panel->dock() == PanelDock::Fill ? -reserved : panel->width();

        if (rowStarted)
            ImGui::SameLine(0.0f, gap);
        panel->draw(ctx);
        rowStarted = true;
    }

    ImGui::End();
    ImGui::PopStyleVar(2);
}

} // namespace Sortly
