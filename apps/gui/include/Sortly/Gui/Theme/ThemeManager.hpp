// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#pragma once

#include <cstddef>
#include <filesystem>
#include <string_view>

namespace Sortly::ui::theme {

/// @brief Reads every *.json in @p dir (file-name order). Returns how many themes were loaded.
std::size_t loadDirectory(const std::filesystem::path& dir);

/// @brief Applies a theme by name right now. Only call it outside a frame.
/// @return False (nothing changes) if no theme has that name.
bool apply(std::string_view name);

/// @brief Asks for the next theme (wrapping around). Safe to call from inside the UI.
void requestNext();

/// @brief Applies a pending requestNext(). Call it once per frame, before ImGuiLayer::beginFrame().
void update();

[[nodiscard]] std::size_t count();

} // namespace Sortly::ui::theme
