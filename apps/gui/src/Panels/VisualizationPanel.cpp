// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "VisualizationPanel.hpp"
#include "Sortly/Gui/Icons.hpp"
#include "Sortly/Gui/UiTheme.hpp"
#include "Sortly/Gui/Widgets/ArrayPlot.hpp"
#include "Sortly/Gui/Widgets/MetricsTable.hpp"
#include "Sortly/Gui/Widgets/PlaybackBar.hpp"
#include "Sortly/Gui/Widgets/UiKit.hpp"

#include <imgui.h>

#include <algorithm>
#include <cstdio>
#include <string>

namespace Sortly {

namespace {

std::string describeOperation(const SortOperation& op)
{
    char buffer[64];
    if (op.indexB >= 0)
        std::snprintf(buffer, sizeof(buffer), "%s  %d \xC2\xB7 %d", toString(op.type).data(), op.indexA, op.indexB);
    else if (op.indexA >= 0)
        std::snprintf(buffer, sizeof(buffer), "%s  %d", toString(op.type).data(), op.indexA);
    else
        std::snprintf(buffer, sizeof(buffer), "%s", toString(op.type).data());
    return buffer;
}

void drawHeader(const SortSession* session)
{
    ImGui::PushFont(ui::fonts::Heading);
    ImGui::TextUnformatted(ICON_CHART "  Sorting visualization");
    ImGui::PopFont();

    const char* tag = session ? session->getAlgorithmName().data() : "NO ACTIVE RUN";
    ImGui::SameLine(ImGui::GetContentRegionMax().x - ui::pillWidth(tag));
    ui::pill(tag, session ? ui::colors::Accent : ui::colors::Muted);

    ImGui::PushFont(ui::fonts::Label);
    ImGui::TextDisabled("Watch how values move as the algorithm processes the dataset.");
    ImGui::PopFont();
}

} // namespace

void VisualizationPanel::draw(UiContext& ctx)
{
    SortSession* session = ctx.run.session();

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ui::dp(22.0f, 18.0f));
    ImGui::BeginChild("Visualization", ImVec2(0.0f, 0.0f),
                      ImGuiChildFlags_Borders | ImGuiChildFlags_AlwaysUseWindowPadding);
    ImGui::PopStyleVar();

    drawHeader(session);
    ImGui::Spacing();

    ui::drawMetricsTable(session ? &session->getStatistics() : nullptr);

    ImGui::Spacing();
    ui::sectionLabel(ICON_CHART, "ARRAY");

    const bool hasPlayback = session && !ctx.run.computing() && session->getPlayback().isLoaded();

    ui::ArrayHighlight highlight;
    if (hasPlayback)
    {
        const auto& playback = session->getPlayback();
        highlight.sorted    = playback.isFinished();
        highlight.operation = playback.getCurrentOperation();
    }

    // Name of the operation about to be applied, right-aligned on the ARRAY caption line.
    if (highlight.operation && !highlight.sorted)
    {
        const std::string text = describeOperation(*highlight.operation);
        ImGui::SameLine(ImGui::GetContentRegionMax().x - ui::pillWidth(text.c_str()));
        ui::pill(text.c_str(), ui::operationColor(highlight.operation->type));
    }

    const float plotHeight = std::max(ui::dp(120.0f), ImGui::GetContentRegionAvail().y - ui::dp(150.0f));
    ui::drawArrayPlot(session ? session->getVisualData() : std::span<const int>{}, highlight, plotHeight);

    ImGui::Spacing();
    if (hasPlayback)
        ui::drawPlaybackBar(session->getPlayback(), ctx.settings.stepDelayMs);
    else if (!ctx.run.computing())
        ui::centeredText(ICON_INFO "  Generate a run to enable playback controls.", ui::colors::TextDim);

    ImGui::EndChild();
}

} // namespace Sortly
