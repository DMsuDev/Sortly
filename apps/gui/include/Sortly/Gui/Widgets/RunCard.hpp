// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#pragma once

#include "Sortly/Gui/RunController.hpp"

#include <imgui.h>

#include <cstddef>

namespace Sortly::ui {

/// @brief One algorithm's panel: title and state, array plot, statistics and progress footer.
/// @param slot Position in the comparison; picks the identity color and the "N." prefix.
void drawRunCard(std::size_t slot, const Run& run, const ImVec2& size);

/// @brief Tallest height a card of this width can use before its plot would grow taller than wide.
/// VisualizationPanel uses it to cap the card instead of leaving empty space below it.
[[nodiscard]] float maxUsefulCardHeight(float cardWidth);

/// @brief Space a card spends outside its plot when the statistics sit beside it (scaled pixels).
[[nodiscard]] ImVec2 cardSideChrome();

} // namespace Sortly::ui
