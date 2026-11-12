// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#pragma once

#include "Sortly/Gui/System.hpp"

#include <imgui.h>

namespace Sortly::ui {

/// @brief Converts a design-time pixel size to the current display scale.
[[nodiscard]] inline float scaled(float pixels)        { return pixels * System::getGlobalScale(); }
[[nodiscard]] inline ImVec2 scaled(float x, float y)   { return { scaled(x), scaled(y) }; }
[[nodiscard]] inline ImVec2 scaled(const ImVec2& size) { return scaled(size.x, size.y); }

} // namespace Sortly::ui

namespace Sortly {

// 10_scaled, 1.5_scaled, ... Visible from every namespace inside Sortly.
[[nodiscard]] inline float operator""_scaled(long double pixels) {
    return ui::scaled(static_cast<float>(pixels));
}
[[nodiscard]] inline float operator""_scaled(unsigned long long pixels) {
    return ui::scaled(static_cast<float>(pixels));
}

} // namespace Sortly
