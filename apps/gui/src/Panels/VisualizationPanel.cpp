// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "Sortly/Gui/Panels/VisualizationPanel.hpp"
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

constexpr float kPlaybackHeight = 140.0f;

ImVec4 operationTint(SortOperation::Type type)
{
    switch (type)
    {
        case SortOperation::Type::Compare:   return ui::colors::Warning;
        case SortOperation::Type::Swap:
        case SortOperation::Type::Overwrite: return ui::colors::Danger;
        case SortOperation::Type::Pivot:     return ui::colors::Violet;
        case SortOperation::Type::Sorted:    return ui::colors::Success;
    }
    return ui::colors::Accent;
}

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

    ImGui::SameLine(0.0f, 14.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(22.0f, 18.0f));
    ImGui::BeginChild("Visualization", ImVec2(0.0f, 0.0f),
                      ImGuiChildFlags_Borders | ImGuiChildFlags_AlwaysUseWindowPadding);
    ImGui::PopStyleVar();

    drawHeader(session);
    ImGui::Spacing();

    ui::drawMetricsTable(session ? &session->getStatistics() : nullptr);

    ImGui::Spacing();
    ui::sectionLabel(ICON_CHART, "ARRAY");

    static const std::vector<int> kEmpty;
    const std::vector<int>* data = &kEmpty;
    ui::ArrayHighlight highlight;
    const bool hasPlayback = session && !ctx.run.computing() && !session->getPlayback().getOperations().empty();

    if (session)
    {
        const auto& playback = session->getPlayback();
        data = playback.getOperations().empty()
            ? &session->getDataSet().mutableValues()
            : &playback.getVisualData();

        if (hasPlayback)
        {
            highlight.sorted    = playback.isFinished();
            highlight.operation = playback.getCurrentOperation();
        }
    }

    // Name of the operation about to be applied, right-aligned on the ARRAY caption line.
    if (highlight.operation && !highlight.sorted)
    {
        const std::string text = describeOperation(*highlight.operation);
        ImGui::SameLine(ImGui::GetContentRegionMax().x - ui::pillWidth(text.c_str()));
        ui::pill(text.c_str(), operationTint(highlight.operation->type));
    }

    const float plotHeight = std::max(160.0f, ImGui::GetContentRegionAvail().y - kPlaybackHeight);
    ui::drawArrayPlot(*data, highlight, plotHeight);

    ImGui::Spacing();
    if (hasPlayback)
        ui::drawPlaybackBar(session->getPlayback(), ctx.settings.stepDelayMs);
    else if (!ctx.run.computing())
        ui::centeredText(ICON_INFO "  Generate a run to enable playback controls.",
                         ImVec4(0.48f, 0.54f, 0.62f, 1.0f));

    ImGui::EndChild();
}

} // namespace Sortly
