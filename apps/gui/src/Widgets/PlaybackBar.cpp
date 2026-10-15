// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "Sortly/Gui/Widgets/PlaybackBar.hpp"
#include "Sortly/Gui/Widgets/UiKit.hpp"
#include "Sortly/Gui/Icons.hpp"
#include "Sortly/Gui/UiTheme.hpp"

#include <cstdio>

namespace Sortly::ui {

namespace {

constexpr float kButtonHeight = 44.0f;

// Step buttons need a paused, started playback so the cursor semantics stay valid.
void prepareManualStep(SortPlayback& playback)
{
    if (playback.isPlaying())
        playback.pause();

    if (playback.isStopped())
    {
        playback.play();
        playback.pause();
    }
}

bool toolButton(const char* icon, const char* tooltip, float width, ButtonKind kind)
{
    const bool pressed = iconButton(icon, ImVec2(width, kButtonHeight), kind);
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", tooltip);
    return pressed;
}

const char* stateName(const SortPlayback& playback)
{
    if (playback.isPlaying())  return "PLAYING";
    if (playback.isPaused())   return "PAUSED";
    if (playback.isFinished()) return "FINISHED";
    return "READY";
}

ImVec4 stateColor(const SortPlayback& playback)
{
    if (playback.isPlaying())  return colors::Accent;
    if (playback.isPaused())   return colors::Warning;
    if (playback.isFinished()) return colors::Success;
    return colors::Muted;
}

} // namespace

void drawPlaybackBar(SortPlayback& playback, float& delayMs)
{
    constexpr float sliderWidth = 220.0f;

    if (toolButton(ICON_SKIP_BACK, "Step back", 52.0f, ButtonKind::Secondary))
    {
        prepareManualStep(playback);
        playback.stepBackward();
    }

    ImGui::SameLine();
    const bool playing = playback.isPlaying();
    if (toolButton(playing ? ICON_PAUSE " Pause" : ICON_PLAY " Play",
                   playing ? "Pause playback" : "Start / resume playback", 128.0f, ButtonKind::Primary))
    {
        if (playing) playback.pause();
        else         playback.play();
    }

    ImGui::SameLine();
    if (toolButton(ICON_SKIP_FORWARD, "Step forward", 52.0f, ButtonKind::Secondary))
    {
        prepareManualStep(playback);
        playback.stepForward();
    }

    ImGui::SameLine();
    if (toolButton(ICON_RESET, "Reset to the initial state", 52.0f, ButtonKind::Secondary))
        playback.stop();

    ImGui::SameLine(ImGui::GetContentRegionMax().x - sliderWidth);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (kButtonHeight - ImGui::GetFrameHeight()) * 0.5f);
    ImGui::SetNextItemWidth(sliderWidth);
    if (ImGui::SliderFloat("##Delay", &delayMs, 1.0f, 120.0f, ICON_GAUGE "  %.0f ms / step"))
        playback.setDelayMs(delayMs);

    ImGui::Spacing();
    progressBar(playback.getProgress(), 6.0f);

    char steps[64];
    std::snprintf(steps, sizeof(steps), "Step %s / %s",
                  formatCount(playback.getCurrentOperationIndex()).c_str(),
                  formatCount(playback.getTotalOperations()).c_str());

    ImGui::PushFont(fonts::Label);
    ImGui::TextColored(colors::Muted, "%s", steps);
    ImGui::PopFont();

    const char* state = stateName(playback);
    ImGui::SameLine(ImGui::GetContentRegionMax().x - pillWidth(state));
    pill(state, stateColor(playback));
}

} // namespace Sortly::ui
