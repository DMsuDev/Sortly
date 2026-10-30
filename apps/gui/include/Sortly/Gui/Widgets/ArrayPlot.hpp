// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#pragma once

#include "Sortly/Sorting/Operations/SortOperation.hpp"

#include <imgui.h>

#include <span>

namespace Sortly { class BarMotion; class SortPlayback; } // namespace Sortly

namespace Sortly::ui {

/// @brief What the plot should emphasize on top of the plain bars.
struct ArrayHighlight
{
    const SortOperation* operation  = nullptr; // Next operation to be applied; its bars are tinted.
    bool                 sorted     = false;   // Whole run finished: every bar is painted as done.
    int                  pivotIndex = -1;      // Slot of the active pivot, or -1.
    float                sweep      = 1.0f;    // Only used when `sorted`: fraction of bars (left to right) already painted as sorted.
    const SortPlayback*  playback   = nullptr; // Tells which slots already hold their final value.
    const BarMotion*     motion     = nullptr; // Animated bar positions; null = each bar sits on its slot.
};

struct ArrayPlotStyle
{
    ImVec4 color; // Bar color of the bars already in place (until the run is finished).
};

/// @param id Unique within the current ImGui ID stack (several plots are on screen at once).
void drawArrayPlot(const char* id, std::span<const int> data, const ArrayHighlight& highlight,
                   const ArrayPlotStyle& style, const ImVec2& size);

} // namespace Sortly::ui
