// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "ControlsPanel.hpp"
#include "Sortly/Gui/Icons.hpp"
#include "Sortly/Gui/UiTheme.hpp"
#include "Sortly/Gui/Widgets/PlaybackBar.hpp"
#include "Sortly/Gui/Widgets/UiKit.hpp"

#include <Sortly/Sorting/Algorithms/SortAlgorithmFactory.hpp>

#include <imgui.h>

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <string>

namespace Sortly {

namespace {

void sectionSeparator()
{
    ImGui::Dummy(ImVec2(0.0f, ui::dp(4.0f)));
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0.0f, ui::dp(4.0f)));
}

// Right-aligned small text on the line of the section caption just drawn.
void rightCaption(const std::string& text, const ImVec4& color)
{
    ImGui::PushFont(ui::fonts::Label);
    ImGui::SameLine(ImGui::GetContentRegionMax().x - ImGui::CalcTextSize(text.c_str()).x);
    ImGui::TextColored(color, "%s", text.c_str());
    ImGui::PopFont();
}

/// @return True if the selection changed.
bool drawAlgorithmList(RunSettings& settings)
{
    ui::sectionLabel(ICON_LIST, "ALGORITHMS");
    rightCaption(std::to_string(settings.algorithms.size()) + " / " + std::to_string(settings.capacity()) + " selected",
                 settings.complete() ? ui::colors::Muted : ui::colors::Warning);
    ImGui::Spacing();

    bool changed = false;
    for (const auto& info : SortAlgorithmFactory::all())
    {
        const auto found = std::find(settings.algorithms.begin(), settings.algorithms.end(), info.type);
        const bool selected = found != settings.algorithms.end();
        // With several slots, a full selection must be released before another algorithm can join.
        const bool locked = !selected && settings.algorithms.size() >= settings.capacity() && settings.capacity() > 1;

        int colorPushes = 0;
        if (selected)
        {
            const ImVec4 color = ui::colors::slot(static_cast<std::size_t>(found - settings.algorithms.begin()));
            ImGui::PushStyleColor(ImGuiCol_CheckMark, color);
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ui::colors::withAlpha(color, 0.22f));
            ImGui::PushStyleColor(ImGuiCol_Border, ui::colors::withAlpha(color, 0.7f));
            colorPushes = 3;
        }

        ImGui::BeginDisabled(locked);
        bool checked = selected;
        if (ImGui::Checkbox(info.name.data(), &checked))
            changed |= settings.toggle(info.type);
        ImGui::EndDisabled();

        if (colorPushes > 0)
            ImGui::PopStyleColor(colorPushes);
    }

    if (!settings.complete())
    {
        const std::size_t missing = settings.capacity() - settings.algorithms.size();
        ImGui::PushFont(ui::fonts::Label);
        ImGui::TextColored(ui::colors::Warning, "Select %zu more to start.", missing);
        ImGui::PopFont();
    }
    return changed;
}

/// @return True if the dataset changed.
bool drawArraySettings(RunSettings& settings)
{
    ui::sectionLabel(ICON_SLIDERS, "ARRAY SETTINGS");
    ImGui::Spacing();

    bool changed = false;

    ImGui::TextUnformatted("Array size");
    ImGui::SetNextItemWidth(-1.0f);
    ImGui::SliderInt("##Elements", &settings.elementCount,
                     RunSettings::kMinElements, RunSettings::kMaxElements, "%d values");
    // Rebuilding while the slider is dragged would restart every algorithm on each frame.
    if (ImGui::IsItemDeactivatedAfterEdit())
        changed = true;

    ImGui::Spacing();
    if (ui::iconButton(ICON_SHUFFLE "  New array", ImVec2(-1.0f, ui::dp(32.0f)), ui::ButtonKind::Secondary, ui::fonts::Body))
    {
        settings.reshuffle();
        changed = true;
    }
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Every algorithm sorts the same data, and keeps it\nuntil you ask for a new array.");

    return changed;
}

void drawSpeed(RunSettings& settings)
{
    ui::sectionLabel(ICON_GAUGE, "ANIMATION SPEED");
    ImGui::Spacing();

    ImGui::SetNextItemWidth(-1.0f);
    ImGui::SliderFloat("##Speed", &settings.speed, RunSettings::kMinSpeed, RunSettings::kMaxSpeed, "%.2gx",
                       ImGuiSliderFlags_Logarithmic);

    ImGui::PushFont(ui::fonts::Label);
    ImGui::TextDisabled("Slow");
    ImGui::SameLine(ImGui::GetContentRegionMax().x - ImGui::CalcTextSize("Fast").x);
    ImGui::TextDisabled("Fast");
    ImGui::PopFont();
}

/// @return True if the view changed.
bool drawViewSelector(RunSettings& settings)
{
    ui::sectionLabel(ICON_GRID, "VIEW");
    ImGui::Spacing();

    struct Option
    {
        const char* label;
        const char* tooltip;
        ViewMode    mode;
    };
    static constexpr Option options[] = {
        { ICON_SINGLE    "  Single",    "One algorithm, full size",     ViewMode::Single   },
        { ICON_COLUMNS_2 "  Compare 2", "Two algorithms side by side",  ViewMode::Compare2 },
        { ICON_COLUMNS_3 "  Compare 3", "Three algorithms at once",     ViewMode::Compare3 },
        { ICON_GRID      "  Compare 4", "Four algorithms in a grid",    ViewMode::Compare4 },
    };

    const float gap   = ImGui::GetStyle().ItemSpacing.x;
    const float width = (ImGui::GetContentRegionAvail().x - gap) * 0.5f;

    bool changed = false;
    for (std::size_t i = 0; i < std::size(options); ++i)
    {
        if (i % 2 != 0)
            ImGui::SameLine();

        const Option& option = options[i];
        const bool active    = settings.view == option.mode;
        const bool available = RunSettings::isViewAvailable(option.mode);

        ImGui::BeginDisabled(!available);
        if (ui::iconButton(option.label, ImVec2(width, ui::dp(32.0f)),
                           active ? ui::ButtonKind::Primary : ui::ButtonKind::Secondary, ui::fonts::Body) && !active)
        {
            settings.setView(option.mode);
            changed = true;
        }
        ImGui::EndDisabled();

        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
        {
            if (available)
                ImGui::SetTooltip("%s", option.tooltip);
            else
                ImGui::SetTooltip("Needs %zu algorithms; only %zu are implemented so far.",
                                  slotCount(option.mode), SortAlgorithmFactory::all().size());
        }
    }
    return changed;
}

} // namespace

float ControlsPanel::width() const { return ui::dp(252.0f); }

void ControlsPanel::draw(UiContext& ctx)
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ui::dp(16.0f, 14.0f));
    ImGui::BeginChild("Controls", ImVec2(ctx.width, 0.0f),
                      ImGuiChildFlags_Borders | ImGuiChildFlags_AlwaysUseWindowPadding,
                      ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::PopStyleVar();

    // Compact metrics for everything in the sidebar: checkboxes, sliders and buttons.
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ui::dp(8.0f, 4.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ui::dp(8.0f, 6.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, ui::dp(10.0f));

    RunSettings& settings = ctx.settings;
    bool changed = false;

    // The settings scroll; the playback controls stay pinned below them. The footer height is the
    // one measured on the previous frame, so the scrolling area never overlaps it.
    ImGui::BeginChild("##Settings", ImVec2(0.0f, -m_FooterHeight), ImGuiChildFlags_None);
    changed |= drawViewSelector(settings);
    sectionSeparator();
    changed |= drawAlgorithmList(settings);
    sectionSeparator();
    changed |= drawArraySettings(settings);
    sectionSeparator();
    drawSpeed(settings);
    ImGui::EndChild();

    const float footerTop = ImGui::GetCursorPosY();
    ImGui::Separator();
    ImGui::Spacing();
    ui::sectionLabel(ICON_PLAY, "CONTROLS");
    ImGui::Spacing();
    ui::drawPlaybackControls(ctx.run, settings);
    m_FooterHeight = ImGui::GetCursorPosY() - footerTop;

    // The workspace always shows what is selected: changing it rebuilds the runs, ready to play.
    if (changed && settings.complete())
        ctx.run.requestStart(settings);

    ImGui::PopStyleVar(3);
    ImGui::EndChild();
}

} // namespace Sortly
