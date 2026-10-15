// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#pragma once

#include "Sortly/Sorting/Operations/SortOperation.hpp"

#include <imgui.h>

namespace Sortly::ui {

namespace colors {
inline const ImVec4 Accent    {0.31f, 0.86f, 0.75f, 1.0f};
inline const ImVec4 Warning   {1.00f, 0.72f, 0.31f, 1.0f};
inline const ImVec4 Danger    {1.00f, 0.42f, 0.46f, 1.0f};
inline const ImVec4 Info      {0.40f, 0.65f, 1.00f, 1.0f};
inline const ImVec4 Violet    {0.68f, 0.55f, 1.00f, 1.0f};
inline const ImVec4 Success   {0.40f, 0.88f, 0.52f, 1.0f};
inline const ImVec4 Muted     {0.67f, 0.77f, 0.84f, 1.0f};
inline const ImVec4 Bar       {0.20f, 0.75f, 0.66f, 0.88f};
inline const ImVec4 Surface   {0.075f, 0.090f, 0.118f, 1.0f};
inline const ImVec4 SurfaceHi {0.105f, 0.125f, 0.160f, 1.0f};
inline const ImVec4 Clear     {0.047f, 0.059f, 0.082f, 1.0f};
inline const ImVec4 TextDim   {0.48f, 0.54f, 0.62f, 1.0f};
inline const ImVec4 Border    {0.17f, 0.20f, 0.25f, 0.80f};
inline const ImVec4 CardBorder{0.20f, 0.24f, 0.30f, 0.70f};
inline const ImVec4 Track     {0.13f, 0.16f, 0.21f, 1.0f};
inline const ImVec4 TextBright{0.94f, 0.97f, 1.00f, 1.0f};
inline const ImVec4 OnAccent  {0.03f, 0.10f, 0.10f, 1.0f};

inline ImVec4 withAlpha(const ImVec4& c, float a) { return ImVec4(c.x, c.y, c.z, a); }
} // namespace colors

// Fonts are baked once in loadFonts(); every one of them also contains the Lucide icons.
namespace fonts {
inline ImFont* Body    = nullptr; // regular text
inline ImFont* Label   = nullptr; // small semibold, used for CAPS captions
inline ImFont* Heading = nullptr; // semibold section / panel titles
inline ImFont* Title   = nullptr; // app title
inline ImFont* Value   = nullptr; // big statistic numbers
} // namespace fonts

/// @brief Display scale (1.0 = 96 dpi). Set once by loadFonts().
inline float g_Scale = 1.0f;

/// @brief Converts a design-time pixel size to the current display scale.
[[nodiscard]] inline float dp(float pixels) { return pixels * g_Scale; }
[[nodiscard]] inline ImVec2 dp(float x, float y) { return ImVec2(x * g_Scale, y * g_Scale); }

/// @brief Tint used for an operation type (plot highlight, operation pill).
[[nodiscard]] ImVec4 operationColor(SortOperation::Type type);

/// @brief Builds the font atlas (system UI font + Lucide icons) for the given display scale.
/// Call before the backend init.
void loadFonts(float scale = 1.0f);

/// @brief Applies colors and metrics; sizes are scaled by the value given to loadFonts().
void applyTheme();

} // namespace Sortly::ui
