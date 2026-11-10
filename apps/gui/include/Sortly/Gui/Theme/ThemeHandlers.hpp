// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#pragma once

namespace Sortly::ui {

/// @brief Registers the ImGui, ImPlot and Sortly handlers with the ThemeManager.
/// The ImGui and ImPlot contexts must exist, and no theme may have been applied yet
/// (their current values become the "native" ones).
void registerThemeHandlers();

} // namespace Sortly::ui
