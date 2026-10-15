// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#pragma once

#include "Sortly/Sorting/Operations/SortStatistics.hpp"

namespace Sortly::ui {

/// @param stats May be nullptr (no active run).
void drawMetricsTable(const SortStatistics* stats);

} // namespace Sortly::ui
