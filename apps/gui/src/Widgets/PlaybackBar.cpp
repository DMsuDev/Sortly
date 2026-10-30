// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "Sortly/Gui/Widgets/PlaybackBar.hpp"
#include "Sortly/Gui/Widgets/UiKit.hpp"
#include "Sortly/Gui/Icons.hpp"
#include "Sortly/Gui/UiTheme.hpp"

#include <algorithm>
#include <imgui.h>

namespace Sortly::ui {

namespace {

constexpr const char* kStartLabel = ICON_PLAY " Start";
constexpr const char* kPauseLabel = ICON_PAUSE " Pause";
constexpr const char* kStopLabel  = ICON_STOP " Stop";
constexpr const char* kBackLabel  = ICON_SKIP_BACK " Back";
constexpr const char* kNextLabel  = "Next " ICON_SKIP_FORWARD;
constexpr const char* kResetLabel = ICON_RESET "  Reset all";

bool toolButton(const char* label, const char* tooltip, float width, bool enabled,
                ButtonKind kind = ButtonKind::Secondary)
{
    ImGui::BeginDisabled(!enabled);
    const bool pressed = iconButton(label, ImVec2(width, compactButtonHeight()), kind, fonts::Body);
    ImGui::EndDisabled();
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
        ui::setTooltip("%s", tooltip);
    return pressed;
}

} // namespace

float minPlaybackWidth()
{
    float widest = 0.0f;
    {
        const ScopedFont font(fonts::Body); // The font the buttons draw with.
        for (const char* label : { kStartLabel, kPauseLabel, kStopLabel })
            widest = std::max(widest, ImGui::CalcTextSize(label).x);
    }
    return 3.0f * (widest + dp(16.0f)) + 2.0f * dp(8.0f); // frame padding + item spacing
}

void drawPlaybackControls(RunController& run, const RunSettings& settings)
{
    const float gap   = ImGui::GetStyle().ItemSpacing.x;
    const float avail = ImGui::GetContentRegionAvail().x;
    const float third = (avail - gap * 2.0f) / 3.0f;
    const float half  = (avail - gap) / 2.0f;

    const bool hasRuns = !run.runs().empty() && settings.complete() && run.matches(settings);

    if (toolButton(kStartLabel, "Play every algorithm (restarts if they all finished)", third, hasRuns,
                   ButtonKind::Primary))
        run.playAll();

    ImGui::SameLine();
    if (toolButton(kPauseLabel, "Pause every algorithm", third, run.anyPlaying()))
        run.pauseAll();

    ImGui::SameLine();
    if (toolButton(kStopLabel, "Stop and go back to the unsorted data", third, hasRuns))
        run.stopAll();

    if (toolButton(kBackLabel, "Undo one operation in every algorithm", half, hasRuns))
        run.stepAll(false);

    ImGui::SameLine();
    if (toolButton(kNextLabel, "Apply one operation in every algorithm", half, hasRuns))
        run.stepAll(true);

    if (toolButton(kResetLabel, "Rebuild every algorithm from the current settings", avail,
                   settings.complete()))
        run.requestStart(settings);
}

} // namespace Sortly::ui
