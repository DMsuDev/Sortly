// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "ControlsPanel.hpp"
#include "Sortly/Gui/Icons.hpp"
#include "Sortly/Gui/UiTheme.hpp"
#include "Sortly/Gui/Widgets/UiKit.hpp"

#include <Sortly/Sorting/Algorithms/SortAlgorithmFactory.hpp>

#include <imgui.h>

#include <string>

namespace Sortly {

namespace {

void drawAlgorithmCombo(SortAlgorithmType& selected)
{
    ImGui::SetNextItemWidth(-1.0f);
    const auto current = SortAlgorithmFactory::toString(selected);
    if (!ImGui::BeginCombo("##Algorithm", current.data()))
        return;

    for (const auto& info : SortAlgorithmFactory::all())
    {
        const bool isSelected = info.type == selected;
        if (ImGui::Selectable(info.name.data(), isSelected))
            selected = info.type;
        if (isSelected)
            ImGui::SetItemDefaultFocus();
    }
    ImGui::EndCombo();
}

// Rounded status box: big centered icon plus a centered message.
void drawStatusCard(const RunController& run)
{
    const bool computing = run.computing();
    const bool ready     = run.session() && !computing;
    const ImVec4 color   = computing ? ui::colors::Warning : (ready ? ui::colors::Success : ui::colors::Muted);
    const char*  icon    = computing ? ICON_LOADER : (ready ? ICON_CHECK : ICON_INFO);

    const float width = ImGui::GetContentRegionAvail().x;
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ui::colors::SurfaceHi);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ui::dp(14.0f, 12.0f));
    ImGui::BeginChild("StatusCard", ImVec2(width, 0.0f),
                      ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_AlwaysUseWindowPadding);
    ImGui::PopStyleVar();

    ImGui::Dummy(ImVec2(0.0f, ui::dp(2.0f)));
    ui::centeredText(icon, color, ui::fonts::Title);

    ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x);
    ui::centeredText(run.status().c_str(), computing ? ui::colors::Warning : ui::colors::Muted);
    ImGui::PopTextWrapPos();

    if (ready)
    {
        const std::string text = ui::formatCount(run.session()->getStatistics().operationCount) + " recorded operations";
        ui::centeredText(text.c_str(), ui::colors::TextDim);
    }
    ImGui::Dummy(ImVec2(0.0f, ui::dp(2.0f)));

    ImGui::EndChild();
    ImGui::PopStyleColor();
}

} // namespace

void ControlsPanel::draw(UiContext& ctx)
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ui::dp(20.0f, 18.0f));
    ImGui::BeginChild("Controls", ImVec2(ui::dp(300.0f), 0.0f),
                      ImGuiChildFlags_Borders | ImGuiChildFlags_AlwaysUseWindowPadding);
    ImGui::PopStyleVar();

    ui::sectionLabel(ICON_SLIDERS, "RUN SETUP");
    ImGui::Spacing();

    ImGui::TextUnformatted("Algorithm");
    drawAlgorithmCombo(ctx.settings.algorithm);

    ImGui::Spacing();
    ImGui::TextUnformatted("Dataset size");
    ImGui::SetNextItemWidth(-1.0f);
    ImGui::SliderInt("##Elements", &ctx.settings.elementCount,
                     RunSettings::kMinElements, RunSettings::kMaxElements, "%d values");

    ImGui::PushFont(ui::fonts::Label);
    ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x);
    ImGui::TextDisabled("Smaller datasets make each operation easier to follow.");
    ImGui::PopTextWrapPos();
    ImGui::PopFont();

    ImGui::Spacing();
    if (ui::iconButton(ICON_SHUFFLE "  New random data", ImVec2(-1.0f, ui::dp(36.0f))))
        ctx.settings.reshuffle();
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Runs reuse the same data until you ask for new data,\nso algorithms can be compared on equal input.");

    ImGui::Spacing();
    if (ui::iconButton(ICON_SPARKLES "  Generate visualization", ImVec2(-1.0f, ui::dp(46.0f)), ui::ButtonKind::Primary))
        ctx.run.requestStart(ctx.settings);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ui::sectionLabel(ICON_ACTIVITY, "CURRENT RUN");
    ImGui::Spacing();
    drawStatusCard(ctx.run);

    ImGui::EndChild();
}

} // namespace Sortly
