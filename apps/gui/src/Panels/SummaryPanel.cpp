// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "SummaryPanel.hpp"
#include "Sortly/Gui/Icons.hpp"
#include "Sortly/Gui/UiTheme.hpp"
#include "Sortly/Gui/Widgets/RunMetrics.hpp"
#include "Sortly/Gui/Widgets/UiKit.hpp"

#include <imgui.h>

#include <algorithm>
#include <cstddef>
#include <span>
#include <string>
#include <utility>
#include <vector>

namespace Sortly {

namespace {

std::string fullName(const Run& run) { return std::string(SortAlgorithmFactory::toString(run.type)); }

// "Bubble Sort" -> "Bubble": column headers and chart labels are too narrow for the suffix.
std::string shortName(const Run& run)
{
    std::string name = fullName(run);
    constexpr const char suffix[] = " Sort";
    if (name.size() > sizeof(suffix) - 1 && name.compare(name.size() - (sizeof(suffix) - 1), sizeof(suffix) - 1, suffix) == 0)
        name.resize(name.size() - (sizeof(suffix) - 1));
    return name;
}

void rightAlignedText(const std::string& text, const ImVec4& color)
{
    const float offset = ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(text.c_str()).x;
    if (offset > 0.0f)
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);
    ImGui::TextColored(color, "%s", text.c_str());
}

// Lowest value of a metric among the finished runs, used to mark the best cell of each row.
double bestValue(std::span<const Run> runs, ui::Metric metric)
{
    double best = 0.0;
    bool   found = false;
    for (const Run& run : runs)
    {
        if (!run.ready())
            continue;
        const double value = ui::metricValue(run.session->getStatistics(), metric);
        if (!found || value < best)
        {
            best  = value;
            found = true;
        }
    }
    return best;
}

void drawSummaryTable(std::span<const Run> runs)
{
    const int columns = static_cast<int>(runs.size()) + 1;
    const bool highlightBest = runs.size() > 1;

    const ui::ScopedFont font(ui::fonts::Label);
    if (ImGui::BeginTable("##Summary", columns,
                          ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_RowBg))
    {
        ImGui::TableSetupColumn("Metric", ImGuiTableColumnFlags_WidthStretch, 1.5f);
        for (std::size_t i = 0; i < runs.size(); ++i)
            ImGui::TableSetupColumn(shortName(runs[i]).c_str(), ImGuiTableColumnFlags_WidthStretch, 1.0f);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextColored(ui::colors::TextDim, "Metric");
        for (std::size_t i = 0; i < runs.size(); ++i)
        {
            ImGui::TableNextColumn();
            rightAlignedText(shortName(runs[i]), ui::colors::slot(i));
        }

        for (const ui::Metric metric : ui::kMetrics)
        {
            const double best = bestValue(runs, metric);

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::TextColored(ui::colors::Muted, "%s", metric == ui::Metric::ComputeTime ? "Time (ms)" : ui::metricLabel(metric));

            for (const Run& run : runs)
            {
                ImGui::TableNextColumn();
                if (!run.ready())
                {
                    rightAlignedText("-", ui::colors::TextDim);
                    continue;
                }
                const SortStatistics& stats = run.session->getStatistics();
                const bool isBest = highlightBest && ui::metricValue(stats, metric) <= best;
                rightAlignedText(ui::formatMetric(stats, metric, false),
                                 isBest ? ui::colors::Success : ui::colors::TextBright);
            }
        }

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextColored(ui::colors::Muted, "Complexity");
        for (const Run& run : runs)
        {
            ImGui::TableNextColumn();
            rightAlignedText(run.session ? std::string(run.session->getAlgorithmComplexity()) : std::string("-"),
                             ui::colors::TextDim);
        }

        ImGui::EndTable();
    }
}

void drawMetricCombo(int& selected)
{
    ImGui::SetNextItemWidth(-1.0f);
    if (!ImGui::BeginCombo("##ChartMetric", ui::metricLabel(ui::kMetrics[static_cast<std::size_t>(selected)])))
        return;

    for (std::size_t i = 0; i < ui::kMetrics.size(); ++i)
    {
        const bool isSelected = static_cast<int>(i) == selected;
        if (ImGui::Selectable(ui::metricLabel(ui::kMetrics[i]), isSelected))
            selected = static_cast<int>(i);
        if (isSelected)
            ImGui::SetItemDefaultFocus();
    }
    ImGui::EndCombo();
}

// One column per algorithm, scaled to the largest value; value on top, name below.
void drawBarChart(std::span<const Run> runs, ui::Metric metric, float height)
{
    const ImVec2 pos   = ImGui::GetCursorScreenPos();
    const float  width = ImGui::GetContentRegionAvail().x;
    ImDrawList*  dl    = ImGui::GetWindowDrawList();

    dl->AddRectFilled(pos, ImVec2(pos.x + width, pos.y + height), ui::toU32(ui::colors::withAlpha(ui::colors::Clear, 0.55f)),
                      ui::dp(8.0f));

    const ui::ScopedFont font(ui::fonts::Label);
    const float labelH = ImGui::GetTextLineHeight();
    const float padX   = ui::dp(10.0f);
    const float top    = pos.y + labelH + ui::dp(8.0f);
    const float bottom = pos.y + height - labelH - ui::dp(8.0f);
    const float areaH  = std::max(1.0f, bottom - top);

    for (int line = 0; line <= 3; ++line)
    {
        const float y = bottom - areaH * static_cast<float>(line) / 3.0f;
        dl->AddLine(ImVec2(pos.x + padX, y), ImVec2(pos.x + width - padX, y), ui::toU32(ImVec4(1, 1, 1, 0.06f)));
    }

    double maxValue = 0.0;
    for (const Run& run : runs)
        if (run.ready())
            maxValue = std::max(maxValue, ui::metricValue(run.session->getStatistics(), metric));

    const float slotW = (width - padX * 2.0f) / static_cast<float>(runs.size());
    const float barW  = std::min(slotW * 0.62f, ui::dp(46.0f));

    for (std::size_t i = 0; i < runs.size(); ++i)
    {
        const Run&  run     = runs[i];
        const float centerX = pos.x + padX + slotW * (static_cast<float>(i) + 0.5f);

        const std::string name = shortName(run);
        const float nameW = ImGui::CalcTextSize(name.c_str()).x;
        dl->AddText(ImVec2(centerX - nameW * 0.5f, bottom + ui::dp(5.0f)), ui::toU32(ui::colors::Muted), name.c_str());

        if (!run.ready())
            continue;

        const SortStatistics& stats = run.session->getStatistics();
        const double value = ui::metricValue(stats, metric);
        const float  barH  = maxValue > 0.0 ? std::max(ui::dp(2.0f), areaH * static_cast<float>(value / maxValue)) : ui::dp(2.0f);
        const ImVec2 barMin(centerX - barW * 0.5f, bottom - barH);

        dl->AddRectFilled(barMin, ImVec2(barMin.x + barW, bottom), ui::toU32(ui::colors::slot(i)), ui::dp(4.0f),
                          ImDrawFlags_RoundCornersTop);

        const std::string text = ui::formatMetric(stats, metric, false);
        const float textW = ImGui::CalcTextSize(text.c_str()).x;
        dl->AddText(ImVec2(centerX - textW * 0.5f, barMin.y - labelH - ui::dp(2.0f)), ui::toU32(ui::colors::TextBright),
                    text.c_str());
    }
    ImGui::Dummy(ImVec2(width, height));
}

struct Winner
{
    std::size_t              index = 0;
    std::vector<ui::Metric>  wins;
    bool                     tie = false;
};

// The algorithm that is best in the most metrics. Equal values count for everyone who reached them.
Winner findWinner(std::span<const Run> runs)
{
    std::vector<std::vector<ui::Metric>> wins(runs.size());
    for (const ui::Metric metric : ui::kMetrics)
    {
        const double best = bestValue(runs, metric);
        for (std::size_t i = 0; i < runs.size(); ++i)
            if (ui::metricValue(runs[i].session->getStatistics(), metric) <= best)
                wins[i].push_back(metric);
    }

    Winner winner;
    std::size_t topCount = 0;
    for (std::size_t i = 0; i < runs.size(); ++i)
    {
        if (wins[i].size() > topCount)
        {
            topCount      = wins[i].size();
            winner.index  = i;
            winner.tie    = false;
        }
        else if (wins[i].size() == topCount)
        {
            winner.tie = true;
        }
    }
    winner.wins = std::move(wins[winner.index]);
    return winner;
}

void drawWinner(std::span<const Run> runs)
{
    const bool comparable = runs.size() > 1;
    const bool done = std::all_of(runs.begin(), runs.end(), [](const Run& run) { return run.ready(); });

    ImGui::PushStyleColor(ImGuiCol_ChildBg, ui::colors::SurfaceHi);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ui::dp(14.0f, 12.0f));
    ImGui::BeginChild("Winner", ImVec2(0.0f, 0.0f),
                      ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_AlwaysUseWindowPadding);
    ImGui::PopStyleVar();

    ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x);
    if (!comparable)
    {
        ImGui::TextColored(ui::colors::TextDim, "Pick the Compare 2, 3 or 4 view to see which algorithm is the most efficient.");
    }
    else if (!done)
    {
        ImGui::TextColored(ui::colors::TextDim, "Waiting for every algorithm to finish building...");
    }
    else
    {
        const Winner winner = findWinner(runs);

        {
            const ui::ScopedFont font(ui::fonts::Heading);
            ImGui::TextColored(ui::colors::Warning, "%s", ICON_TROPHY);
            ImGui::SameLine(0.0f, ui::dp(8.0f));
            if (winner.tie)
                ImGui::TextUnformatted("Tie for first place");
            else
                ImGui::TextColored(ui::colors::slot(winner.index), "%s", fullName(runs[winner.index]).c_str());
        }

        if (!winner.tie) ImGui::TextColored(ui::colors::Muted, "is the most efficient");

        {
            const ui::ScopedFont font(ui::fonts::Label);
            ImGui::TextColored(ui::colors::TextDim, "Best in %zu of %zu metrics", winner.wins.size(), ui::kMetrics.size());
        }

        ImGui::Spacing();
        {
            const ui::ScopedFont font(ui::fonts::Label);
            for (const ui::Metric metric : winner.wins)
            {
                ImGui::TextColored(ui::colors::Success, "%s", ICON_CHECK);
                ImGui::SameLine(0.0f, ui::dp(8.0f));
                ImGui::TextColored(ui::colors::Muted, "%s", ui::metricWinLabel(metric));
            }
        }
    }
    ImGui::PopTextWrapPos();

    ImGui::EndChild();
    ImGui::PopStyleColor();
}

} // namespace

float SummaryPanel::width() const { return ui::dp(330.0f); }

void SummaryPanel::draw(UiContext& ctx)
{
    const auto runs = ctx.run.runs();

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ui::dp(16.0f, 18.0f));
    ImGui::BeginChild("Summary", ImVec2(ctx.width, 0.0f),
                      ImGuiChildFlags_Borders | ImGuiChildFlags_AlwaysUseWindowPadding);
    ImGui::PopStyleVar();

    ui::sectionLabel(ICON_LIST, "STATISTICS SUMMARY");
    ImGui::Spacing();
    if (runs.empty())
        ImGui::TextDisabled("No active run.");
    else
        drawSummaryTable(runs);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ui::sectionLabel(ICON_CHART, "COMPARISON CHART");
    ImGui::Spacing();
    if (!runs.empty())
    {
        drawMetricCombo(m_ChartMetric);
        ImGui::Spacing();
        drawBarChart(runs, ui::kMetrics[static_cast<std::size_t>(m_ChartMetric)], ui::dp(140.0f));
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ui::sectionLabel(ICON_TROPHY, "WINNER");
    ImGui::Spacing();
    drawWinner(runs);

    ImGui::EndChild();
}

} // namespace Sortly
