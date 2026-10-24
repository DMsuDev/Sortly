// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "Sortly/Gui/Widgets/PlaybackBar.hpp"
#include "Sortly/Gui/Widgets/UiKit.hpp"
#include "Sortly/Gui/Icons.hpp"
#include "Sortly/Gui/UiTheme.hpp"

#include <imgui.h>

namespace Sortly::ui {

namespace {

bool toolButton(const char* label, const char* tooltip, float width, bool enabled,
                ButtonKind kind = ButtonKind::Secondary)
{
    ImGui::BeginDisabled(!enabled);
    const bool pressed = iconButton(label, ImVec2(width, compactButtonHeight()), kind, fonts::Body);
    ImGui::EndDisabled();
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
        ImGui::SetTooltip("%s", tooltip);
    return pressed;
}

} // namespace

void drawPlaybackControls(RunController& run, const RunSettings& settings)
{
    const float gap   = ImGui::GetStyle().ItemSpacing.x;
    const float avail = ImGui::GetContentRegionAvail().x;
    const float third = (avail - gap * 2.0f) / 3.0f;
    const float half  = (avail - gap) / 2.0f;

    const bool hasRuns = !run.runs().empty() && settings.complete() && run.matches(settings);

    if (toolButton(ICON_PLAY " Start", "Play every algorithm (restarts if they all finished)", third, hasRuns,
                   ButtonKind::Primary))
        run.playAll();

    ImGui::SameLine();
    if (toolButton(ICON_PAUSE " Pause", "Pause every algorithm", third, run.anyPlaying()))
        run.pauseAll();

    ImGui::SameLine();
    if (toolButton(ICON_STOP " Stop", "Stop and go back to the unsorted data", third, hasRuns))
        run.stopAll();

    if (toolButton(ICON_SKIP_BACK " Back", "Undo one operation in every algorithm", half, hasRuns))
        run.stepAll(false);

    ImGui::SameLine();
    if (toolButton("Next " ICON_SKIP_FORWARD, "Apply one operation in every algorithm", half, hasRuns))
        run.stepAll(true);

    if (toolButton(ICON_RESET "  Reset all", "Rebuild every algorithm from the current settings", avail,
                   settings.complete()))
        run.requestStart(settings);
}

} // namespace Sortly::ui
