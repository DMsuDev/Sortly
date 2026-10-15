// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#pragma once

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

/// @brief Builds the font atlas (system UI font + Lucide icons). Call before the backend init.
void loadFonts(float dpiScale = 1.0f);

void applyTheme();

} // namespace Sortly::ui
