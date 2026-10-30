// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#pragma once

#include "Sortly/Gui/RunController.hpp"
#include "Sortly/Gui/RunSettings.hpp"

namespace Sortly::ui {

/// @brief Width the panel needs so the three playback buttons of a row fit their labels.
[[nodiscard]] float minPlaybackWidth();

/// @brief Start / Pause / Stop, step buttons and "Reset all". Every button acts on all the
/// runs at once.
/// @param settings Used to rebuild the runs on "Reset all" and to know whether Start is allowed.
void drawPlaybackControls(RunController& run, const RunSettings& settings);

} // namespace Sortly::ui
