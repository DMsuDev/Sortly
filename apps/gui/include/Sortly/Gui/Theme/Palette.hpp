// Palette.hpp
// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#pragma once

#include <imgui.h>

#include <cstdint>
#include <optional>
#include <span>
#include <string_view>

namespace Sortly::ui::palette {

/// To add a color: add an enumerator here and a matching entry in Palette.cpp (same order).
enum class Color : std::uint8_t
{
    BarIdle, Accent, Warning, Danger, Info, Violet, Success, Muted, Bar,
    Surface, SurfaceHi, Clear, TextDim, Border, CardBorder, Track, TextBright,
    OnAccent, Slot0, Slot1, Slot2, Slot3,

    Count // Keep this as the last enumerator to track the number of colors.
};

struct ColorInfo
{
    Color            id;
    std::string_view name;         ///< Stable key used in theme JSON.
    ImVec4           defaultValue; ///< Built-in value used by resetColor() and resetAllColors().
};

// ---- Metadata ---------------------------------------------------------------------------
[[nodiscard]] std::span<const ColorInfo> getColors();
[[nodiscard]] std::string_view           getColorName(Color color);
[[nodiscard]] std::optional<Color>       findColor(std::string_view name);
[[nodiscard]] const ImVec4&              getDefaultColor(Color color);

// ---- Current palette (UI thread only) ---------------------------------------------------
/// The reference stays valid when the color is updated.
[[nodiscard]] const ImVec4& getColor(Color color);
void setColor(Color color, const ImVec4& value);
void resetColor(Color color);
void resetAllColors();

// ---- Helpers ----------------------------------------------------------------------------
/// Identity color of a comparison slot. Wraps around the four slot colors.
[[nodiscard]] const ImVec4& getSlotColor(std::size_t index);

/// Packed color with its alpha multiplied by @p alphaMultiplier.
[[nodiscard]] ImU32 getColorU32(Color color, float alphaMultiplier = 1.0f);

/// Copy of @p color with its alpha replaced by @p alpha.
[[nodiscard]] inline ImVec4 withAlpha(const ImVec4& color, float alpha)
{
    return { color.x, color.y, color.z, alpha };
}

} // namespace Sortly::ui::palette
