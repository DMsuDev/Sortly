// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#pragma once

#include "Sortly/Sorting/Operations/SortPlayback.hpp"

namespace Sortly::ui {

/// @brief Step/Play/Reset buttons, delay slider and progress bar.
/// @param delayMs Edited in place; applied to @p playback when changed.
void drawPlaybackBar(SortPlayback& playback, float& delayMs);

} // namespace Sortly::ui
