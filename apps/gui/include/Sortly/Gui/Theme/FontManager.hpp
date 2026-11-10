// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#pragma once

#include <imgui.h>

#include <cstdint>
#include <span>
#include <string>

namespace Sortly::ui {

/// @brief Bakes the font atlas from the registered fonts. Sizes are design pixels:
/// load() applies the global scale. UI thread only.
class FontManager
{
public:
    struct Font
    {
        std::string file;          // Under assets/, e.g. "fonts/Inter-SemiBold.ttf".
        std::string fallback;      // Used if file is missing. Empty = ImGui's built-in bitmap font.
        float size = 16.0f;        // Design pixels.
        float iconScale = 1.0f;    // Size of the merged icons relative to the text.
        ImFont** target = nullptr; // Receives the baked font (e.g. &fonts::Body). Optional.
    };

    /// @brief Icon font merged into every font, restricted to @p codepoints.
    static void setIcons(const std::string& file, std::span<const std::uint16_t> codepoints);

    /// @brief Registers a font. The one named "body" becomes ImGui's default.
    static void addFont(const std::string& name, const Font& font);

    /// @brief Bakes every registered font at the current global scale, replacing the old atlas.
    /// Call it outside a frame: before the backends at startup, or again after the scale changed.
    /// @return False if nothing was baked.
    static bool load();

    [[nodiscard]] static ImFont* get(const std::string& name);

    /// @brief Forgets every registration and baked font (they die with the ImGui context).
    static void clear();

private:
    FontManager() = default;
};

/// @brief Registers Sortly's fonts (see FontRegistry.cpp). Call before FontManager::load().
void registerDefaultFonts();

} // namespace Sortly::ui
