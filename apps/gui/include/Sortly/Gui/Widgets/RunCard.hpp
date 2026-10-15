// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#pragma once

#include "Sortly/Gui/RunController.hpp"

#include <imgui.h>

#include <cstddef>

namespace Sortly::ui {

/// @brief One algorithm's panel: title and state, array plot, statistics and progress.
/// Adapts to its size: statistics sit beside the plot when there is room, below it otherwise.
/// @param slot Position in the comparison; picks the identity color and the "N." prefix.
void drawRunCard(std::size_t slot, const Run& run, const ImVec2& size);

} // namespace Sortly::ui
