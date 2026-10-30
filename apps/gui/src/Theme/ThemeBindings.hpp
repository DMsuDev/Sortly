// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#pragma once

#include <nlohmann/json.hpp>

namespace Sortly::ui::theme {

/// @brief Resets ImGui, ImPlot and the Sortly tokens to the theme's base, then applies its
/// "colors" and "styles". Must run outside a frame (pending PopStyleVar would undo it).
void applyBindings(const nlohmann::json& data);

} // namespace Sortly::ui::theme
