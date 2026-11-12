// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#pragma once

#include <sortrace/core/operation.hpp>

#include <imgui.h>

namespace Sortly::ui {

// Fonts are baked once in loadFonts(); every one of them also contains the Lucide icons.
namespace fonts
{
    inline ImFont* Body    = nullptr; // regular text
    inline ImFont* Label   = nullptr; // small semibold, used for CAPS captions
    inline ImFont* Heading = nullptr; // semibold section / panel titles
    inline ImFont* Title   = nullptr; // app title
    inline ImFont* Value   = nullptr; // big statistic numbers
}

/// @brief Tint used for an operation type (plot highlight, operation pill).
[[nodiscard]] ImVec4 operationColor(sortrace::OperationType type);

/// @brief Applies colors and metrics; sizes are scaled by the value given to loadFonts().
void applyTheme();

} // namespace Sortly::ui
