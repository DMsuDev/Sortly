// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "Sortly/Gui/GuiApplication.hpp"
#include "Sortly/Gui/UiTheme.hpp"

#include "Panels/ControlsPanel.hpp"
#include "Panels/SummaryPanel.hpp"
#include "Panels/TopBarPanel.hpp"
#include "Panels/VisualizationPanel.hpp"

#include <imgui.h>

#include <foundry/memory/scope_guard.h>
#include <vigil/logging/log_system.h>

#include <algorithm>
#include <chrono>

namespace Sortly {

namespace {

constexpr int         kWindowWidth   = 1440;
constexpr int         kWindowHeight  = 900;
constexpr int         kMinWidth      = 1180;
constexpr int         kMinHeight     = 720;
constexpr const char* kWindowTitle   = "Sortly - Sorting Lab";
constexpr const char* kWorkspaceName = "Sortly Workspace";

// Dragging or resizing the window blocks the loop on some platforms. Without a cap,
// playback would apply thousands of steps at once when it resumes.
constexpr float kMaxFrameSeconds = 0.1f;

// Gap between panels, in device-independent pixels.
constexpr float kPanelGap = 14.0f;

// Inset from the window edge to detect resizing.
constexpr float kEdgeInset = 8.0f;

/// @brief Seconds elapsed between consecutive calls to tick(), capped at kMaxFrameSeconds.
class FrameClock
{
public:
    [[nodiscard]] float tick()
    {
        const auto  now     = std::chrono::steady_clock::now();
        const float elapsed = std::chrono::duration<float>(now - m_Previous).count();
        m_Previous = now;
        return std::min(elapsed, kMaxFrameSeconds);
    }

    /// @brief Forgets the time spent paused, so the next tick() starts from zero.
    void reset() { m_Previous = std::chrono::steady_clock::now(); }

private:
    std::chrono::steady_clock::time_point m_Previous = std::chrono::steady_clock::now();
};

// The window has no OS frame, so the resize cursor has to be set by hand.
void applyEdgeCursor(Window::Edges edges)
{
    if (edges == Window::kEdgeNone)
        return;

    const bool horizontal = (edges & (Window::kEdgeLeft | Window::kEdgeRight)) != 0;
    const bool vertical   = (edges & (Window::kEdgeTop | Window::kEdgeBottom)) != 0;
    // Top-left and bottom-right share a diagonal; the other two corners share the other one.
    const bool nwse = ((edges & Window::kEdgeLeft) != 0) == ((edges & Window::kEdgeTop) != 0);

    ImGui::SetMouseCursor(horizontal && vertical ? (nwse ? ImGuiMouseCursor_ResizeNWSE : ImGuiMouseCursor_ResizeNESW)
                          : horizontal           ? ImGuiMouseCursor_ResizeEW
                                                 : ImGuiMouseCursor_ResizeNS);
}

} // namespace

GuiApplication::GuiApplication()
    : Application({ .name = "Sortly", .verboseLogging = true })
    , m_Run([this](SortAlgorithmType type, const RunSettings& settings) {
          // Every algorithm in a run gets the same seed, so they all sort the same data.
          sessions().setSeedProvider(SeedProvider::globalSeed(settings.seed));

          DataSetConfig config;
          config.count = settings.elementCount;
          return sessions().createSession(type, config);
      })
{}

bool GuiApplication::onInit()
{
    // Any failure below (return false or exception) releases what was built.
    FOUNDRY_SCOPE_EXIT_NAMED(rollback) { teardown(); };

    try
    {
        // The UI layer needs the window's GL context current, so the order is fixed.
        if (!m_Window.create(kWindowWidth, kWindowHeight, kWindowTitle, kMinWidth, kMinHeight) ||
            !m_Gui.init(m_Window.handle(), m_Window.uiScale()))
            return false;

        // Adding a screen section = one new IPanel (with its dock) + one line here.
        m_Panels.push_back(std::make_unique<TopBarPanel>(m_Window));
        m_Panels.push_back(std::make_unique<ControlsPanel>());
        m_Panels.push_back(std::make_unique<VisualizationPanel>());
        m_Panels.push_back(std::make_unique<SummaryPanel>());

        // Build the default selection up front so the workspace is never empty.
        m_Run.requestStart(m_Settings);

        rollback.dismiss();
        return true;
    }
    catch (...)
    {
        VIGIL_LOG_ERROR("Initialization of the GUI application failed");
        return false;
    }
}

int GuiApplication::onRun()
{
    FrameClock clock;

    while (!m_Window.isCloseRequested())
    {
        m_Window.pollEvents();

        // A minimized window has a 0x0 framebuffer and may stop waiting for vsync.
        // Sleep until something happens instead of spinning, and pause playback.
        if (m_Window.isMinimized())
        {
            m_Window.waitEvents();
            clock.reset();
            continue;
        }

        m_Run.setDelayMs(m_Settings.stepDelayMs());
        m_Run.setSmooth(m_Settings.smoothMotion);
        m_Run.update(clock.tick());

        renderFrame();
    }

    return 0;
}

void GuiApplication::onShutdown()
{
    teardown();
}

void GuiApplication::teardown() noexcept
{
    // Newest first. Every step is safe to repeat, so this can run from
    // a failed onInit() and again from onShutdown().
    m_Panels.clear();
    m_Gui.shutdown();
    m_Window.destroy();
}

void GuiApplication::renderFrame()
{
    const ImVec4& clear = ui::colors::Clear;
    m_Window.clear(clear.x, clear.y, clear.z, clear.w);

    m_Gui.beginFrame();
    drawUi();
    m_Gui.endFrame();

    m_Window.swapBuffers();
}

void GuiApplication::drawUi()
{
    const auto& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(io.DisplaySize, ImGuiCond_Always);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ui::dp(kEdgeInset, kEdgeInset));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::Begin(kWorkspaceName, nullptr,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus);

    applyEdgeCursor(m_Window.activeEdges());
    if (ImGui::IsKeyPressed(ImGuiKey_F11, false))
        m_Window.toggleFullscreen();

    UiContext ctx{ m_Settings, m_Run };
    layoutPanels(ctx);

    ImGui::End();
    ImGui::PopStyleVar(2);
}

void GuiApplication::layoutPanels(UiContext& ctx)
{
    // Top panels take a full-width row. Other panels sit side by side on the
    // row below. Fill panels take whatever the fixed-width Right panels leave over.
    const float gap = ui::dp(kPanelGap);

    float reservedRight = 0.0f;
    for (const auto& panel : m_Panels)
        if (panel->dock() == PanelDock::Right)
            reservedRight += gap + panel->width();

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

        ctx.width = panel->dock() == PanelDock::Fill ? -reservedRight : panel->width();

        if (rowStarted)
            ImGui::SameLine(0.0f, gap);

        panel->draw(ctx);
        rowStarted = true;
    }
}

} // namespace Sortly
