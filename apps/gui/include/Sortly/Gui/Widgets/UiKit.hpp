// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#pragma once

#include <imgui.h>

#include <cstdint>
#include <string>

namespace Sortly::ui {

enum class ButtonKind { Primary, Secondary };

/// @brief Thousands-separated integer, e.g. 1234567 -> "1,234,567".
[[nodiscard]] std::string formatCount(std::uint64_t value);

/// @brief Draws @p text horizontally centered in the remaining width of the current line.
void centeredText(const char* text, const ImVec4& color, ImFont* font = nullptr);

/// @brief Small caps caption with an accent icon: [icon] LABEL.
void sectionLabel(const char* icon, const char* label);

/// @brief Filled rounded button. Primary uses the accent color, Secondary a neutral surface.
/// @param font Defaults to the heading font; pass a smaller one for compact layouts.
bool iconButton(const char* label, const ImVec2& size, ButtonKind kind = ButtonKind::Secondary,
                ImFont* font = nullptr);

/// @brief Width a pill with @p text will occupy (for right alignment).
[[nodiscard]] float pillWidth(const char* text);

/// @brief Rounded tag, e.g. the algorithm name.
void pill(const char* text, const ImVec4& color);

/// @brief Statistic tile: tinted icon badge, caption and a big value (with optional unit).
void statCard(const char* icon, const char* label, const std::string& value,
              const char* unit, const ImVec4& color, const ImVec2& size);

/// @brief Slim rounded progress bar; @p color defaults to the accent.
void progressBar(float fraction, float height = 6.0f, const ImVec4* color = nullptr);

} // namespace Sortly::ui
