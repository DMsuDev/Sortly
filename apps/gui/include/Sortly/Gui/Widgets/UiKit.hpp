// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#pragma once

#include <imgui.h>

#include <cstdint>
#include <string>

namespace Sortly::ui {

enum class ButtonKind { Primary, Secondary };

[[nodiscard]] inline ImU32 toU32(const ImVec4& color) { return ImGui::ColorConvertFloat4ToU32(color); }

/// @brief Pushes a font for its lifetime. A null font pushes nothing.
class ScopedFont
{
public:
    explicit ScopedFont(ImFont* font) : m_Pushed(font != nullptr)
    {
        if (m_Pushed)
            ImGui::PushFont(font);
    }
    ~ScopedFont()
    {
        if (m_Pushed)
            ImGui::PopFont();
    }

    ScopedFont(const ScopedFont&)            = delete;
    ScopedFont& operator=(const ScopedFont&) = delete;

private:
    bool m_Pushed;
};

/// @brief Pushes a style color for its lifetime; @p enabled = false makes it a no-op,
/// so conditional colors need no matching `if` around the pop.
class ScopedColor
{
public:
    ScopedColor(ImGuiCol slot, const ImVec4& color, bool enabled = true) : m_Enabled(enabled)
    {
        if (m_Enabled)
            ImGui::PushStyleColor(slot, color);
    }
    ~ScopedColor()
    {
        if (m_Enabled)
            ImGui::PopStyleColor();
    }

    ScopedColor(const ScopedColor&)            = delete;
    ScopedColor& operator=(const ScopedColor&) = delete;

private:
    bool m_Enabled;
};

/// @brief Thousands-separated integer, e.g. 1234567 -> "1,234,567".
[[nodiscard]] std::string formatCount(std::uint64_t value);

/// @brief Draws @p text horizontally centered in the remaining width of the current line.
void centeredText(const char* text, const ImVec4& color, ImFont* font = nullptr);

/// @brief Draws @p text flush with the right edge of the remaining width of the current line.
void rightAlignedText(const char* text, const ImVec4& color);

/// @brief Small caps caption with an accent icon: [icon] LABEL.
void sectionLabel(const char* icon, const char* label);

/// @brief Calculates the button size for a button with @p label and optional @p font.
[[nodiscard]] ImVec2 calculateButtonSize(const char* label, ImFont* font = nullptr);

/// @brief Height of a compact toolbar button: one line of the body font plus padding, so it
/// follows the font instead of a hard-coded pixel count.
[[nodiscard]] float compactButtonHeight();

/// @brief Filled rounded button. Primary uses the accent color, Secondary a neutral surface.
/// @param font Defaults to the heading font; pass a smaller one for compact layouts.
bool iconButton(const char* label, const ImVec2& size, ButtonKind kind = ButtonKind::Secondary,
                ImFont* font = nullptr);

/// @brief Width a pill with @p label needs; use it to size and align the button.
[[nodiscard]] float calculatePillWidth(const char* label);

/// @brief Tinted pill button: [Icon] label, content centered. Reads as a labeled action,
/// unlike the flat, icon-only buttons.
/// @param id Unique within the ID stack, e.g. "##Fullscreen".
bool pillButton(const char* id, const char* label, const char* icon, const ImVec4& tint, const ImVec2& size);

/// @brief Thin vertical separator that occupies @p height on the current line (draws 60% of it).
void verticalDivider(float height);

/// @brief Width a pill with @p text will occupy (for right alignment).
[[nodiscard]] float pillWidth(const char* text);

/// @brief Rounded tag, e.g. the algorithm name.
void pill(const char* text, const ImVec4& color);

/// @brief Statistic tile: tinted icon badge, caption and a big value (with optional unit).
void statCard(const char* icon, const char* label, const std::string& value,
              const char* unit, const ImVec4& color, const ImVec2& size);

/// @brief Slim rounded progress bar; @p color defaults to the accent.
void progressBar(float fraction, float height = 6.0f, const ImVec4* color = nullptr);

/// @brief Radial glow in the top-left corner of the current window: a quarter disc that fades
/// to transparent, clipped to the rounded corner. Draw it before the window's content.
/// @param min       Top-left corner of the card, in screen space.
/// @param radius    Distance at which the glow has fully faded.
/// @param rounding  Corner radius of the card.
/// @param peakAlpha Opacity at the corner itself.
void cornerGlow(const ImVec2& min, float radius, float rounding, const ImVec4& color, float peakAlpha);

/// @brief Styled replacements for ImGui::BeginTooltip / SetTooltip / BeginPopup: compact padding,
/// rounded corners and an accent border. End them with ImGui::EndTooltip() / ImGui::EndPopup().
[[nodiscard]] bool beginTooltip();
[[nodiscard]] bool beginPopup(const char* id);
void setTooltip(const char* fmt, ...) IM_FMTARGS(1);

} // namespace Sortly::ui
