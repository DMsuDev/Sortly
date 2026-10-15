// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#pragma once

#include "Sortly/Sorting/Operations/SortOperation.hpp"

#include <vector>

namespace Sortly::ui {

/// @brief What the plot should emphasize on top of the plain bars.
struct ArrayHighlight
{
    const SortOperation* operation = nullptr; // Next operation to be applied; its indices are tinted.
    bool                 sorted    = false;   // Whole run finished: every bar is painted as done.
};

void drawArrayPlot(const std::vector<int>& data, const ArrayHighlight& highlight, float height);

} // namespace Sortly::ui
