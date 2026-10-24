// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#pragma once

#include "Sortly/Sorting/Operations/SortOperation.hpp"

#include <imgui.h>

#include <span>

namespace Sortly::ui {

/// @brief What the plot should emphasize on top of the plain bars.
struct ArrayHighlight
{
    const SortOperation* operation  = nullptr; // Next operation to be applied; its indices are tinted.
    int                  pivotIndex = -1;      // Bar kept marked as the pivot while its partition runs (-1: none).
    bool                 sorted     = false;   // Whole run finished: every bar is painted as done.
};

struct ArrayPlotStyle
{
    ImVec4 color;        // Bar color while the run is not finished.
    bool   axes = false; // Index/value axes; small cards look better without them.
};

/// @param id Unique within the current ImGui ID stack (several plots are on screen at once).
void drawArrayPlot(const char* id, std::span<const int> data, const ArrayHighlight& highlight,
                   const ArrayPlotStyle& style, const ImVec2& size);

} // namespace Sortly::ui
