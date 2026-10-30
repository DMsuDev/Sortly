// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "VisualizationPanel.hpp"
#include "Sortly/Gui/Icons.hpp"
#include "Sortly/Gui/UiTheme.hpp"
#include "Sortly/Gui/Widgets/RunCard.hpp"
#include "Sortly/Gui/Widgets/UiKit.hpp"

#include <imgui.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <string>
#include <vector>

namespace Sortly {

namespace {

void drawHeader(const RunController& run)
{
    const auto runs = run.runs();
    const bool compare = runs.size() > 1;

    ImGui::PushFont(ui::fonts::Heading);
    const std::string title = compare
        ? std::string(ICON_COMPARE "  Comparing ") + std::to_string(runs.size()) + " algorithms"
        : std::string(ICON_CHART "  Sorting visualization");
    ImGui::TextUnformatted(title.c_str());
    ImGui::PopFont();

    std::size_t values = 0;
    for (const Run& r : runs)
    {
        if (r.session)
        {
            values = r.session->getVisualData().size();
            break;
        }
    }

    ImGui::PushFont(ui::fonts::Label);
    if (compare && values > 0)
        ImGui::TextDisabled("Every algorithm sorts the same %zu values.", values);
    else
        ImGui::TextDisabled("Watch how values move as the algorithm processes the dataset.");
    ImGui::PopFont();
}

void drawLegend()
{
    struct Item
    {
        const char* label;
        ImVec4      color;
    };
    const std::array<Item, 4> items = {{
        { "Comparing", ui::operationColor(SortOperation::Type::Compare) },
        { "Swapping",  ui::operationColor(SortOperation::Type::Swap)    },
        { "Pivot",     ui::operationColor(SortOperation::Type::Pivot)   },
        { "Sorted",    ui::colors::Success                              },
    }};

    const float dot     = ui::dp(5.0f);
    const float gapText = ui::dp(8.0f);
    const float gapItem = ui::dp(22.0f);

    ImGui::PushFont(ui::fonts::Label);
    float total = 0.0f;
    for (const Item& item : items)
        total += dot * 2.0f + gapText + ImGui::CalcTextSize(item.label).x + gapItem;
    total -= gapItem;

    const float offset = (ImGui::GetContentRegionAvail().x - total) * 0.5f;
    if (offset > 0.0f)
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);

    ImDrawList* dl = ImGui::GetWindowDrawList();
    for (std::size_t i = 0; i < items.size(); ++i)
    {
        const Item& item = items[i];
        if (i > 0)
            ImGui::SameLine(0.0f, gapItem);

        const ImVec2 pos = ImGui::GetCursorScreenPos();
        const float  h   = ImGui::GetTextLineHeight();
        dl->AddCircleFilled(ImVec2(pos.x + dot, pos.y + h * 0.5f), dot, ImGui::ColorConvertFloat4ToU32(item.color));
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + dot * 2.0f + gapText);
        ImGui::TextColored(ui::colors::Muted, "%s", item.label);
    }
    ImGui::PopFont();
}

// Cards per row. Each count has a few candidate arrangements; the one whose plots come out
// closest to a landscape rectangle for the available area wins.
using RowLayout = std::vector<std::size_t>;

const std::vector<RowLayout>& candidateLayouts(std::size_t count)
{
    static const std::vector<RowLayout> one   = { { 1 } };
    static const std::vector<RowLayout> two   = { { 1, 1 }, { 2 } };
    static const std::vector<RowLayout> three = { { 2, 1 }, { 3 } };
    static const std::vector<RowLayout> four  = { { 2, 2 }, { 1, 1, 1, 1 } };
    switch (count)
    {
        case 1:  return one;
        case 2:  return two;
        case 3:  return three;
        default: return four;
    }
}

constexpr float kPlotAspect = 1.7f;

float layoutScore(const RowLayout& rows, const ImVec2& area, float gapX, float gapY)
{
    const float cardH = (area.y - gapY * static_cast<float>(rows.size() - 1)) / static_cast<float>(rows.size());
    float worst = 0.0f;
    const ImVec2 chrome = ui::cardSideChrome();
    for (const std::size_t columns : rows)
    {
        const float cardW = (area.x - gapX * static_cast<float>(columns - 1)) / static_cast<float>(columns);
        const float plotW = cardW - chrome.x;
        const float plotH = cardH - chrome.y;
        if (plotW < ui::dp(100.0f) || plotH < ui::dp(70.0f))
            return 1e9f;
        worst = std::max(worst, std::abs(std::log(plotW / plotH / kPlotAspect)));
    }
    return worst;
}

void drawGrid(const RunController& run, const ImVec2& area)
{
    const auto runs = run.runs();

    const float gapX = ui::dp(12.0f);
    const float gapY = ImGui::GetStyle().ItemSpacing.y;

    const RowLayout* best = nullptr;
    float bestScore = 0.0f;
    for (const RowLayout& layout : candidateLayouts(runs.size()))
    {
        const float score = layoutScore(layout, area, gapX, gapY);
        if (!best || score < bestScore)
        {
            best      = &layout;
            bestScore = score;
        }
    }

    const std::size_t rowCount = best->size();
    const float cellH = (area.y - gapY * static_cast<float>(rowCount - 1)) / static_cast<float>(rowCount);

    std::size_t slot = 0;
    for (const std::size_t columns : *best)
    {
        const float cardW = (area.x - gapX * static_cast<float>(columns - 1)) / static_cast<float>(columns);
        // Never taller than a landscape plot needs, even when the area has room to spare.
        const float cardH = std::min(cellH, ui::maxUsefulCardHeight(cardW));

        for (std::size_t column = 0; column < columns && slot < runs.size(); ++column, ++slot)
        {
            if (column > 0)
                ImGui::SameLine(0.0f, gapX);
            ui::drawRunCard(slot, runs[slot], ImVec2(cardW, cardH));
        }
    }
}

} // namespace

void VisualizationPanel::draw(UiContext& ctx)
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ui::dp(22.0f, 18.0f));
    ImGui::BeginChild("Visualization", ImVec2(ctx.width, 0.0f),
                      ImGuiChildFlags_Borders | ImGuiChildFlags_AlwaysUseWindowPadding,
                      ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::PopStyleVar();

    drawHeader(ctx.run);
    ImGui::Spacing();

    if (ctx.run.runs().empty())
    {
        ui::centeredText(ICON_INFO "  Choose the algorithms to compare in the side panel.", ui::colors::TextDim);
    }
    else
    {
        const float legendH = ui::dp(34.0f);
        const ImVec2 avail  = ImGui::GetContentRegionAvail();
        drawGrid(ctx.run, ImVec2(avail.x, std::max(ui::dp(120.0f), avail.y - legendH)));

        ImGui::Spacing();
        drawLegend();
    }

    ImGui::EndChild();
}

} // namespace Sortly
