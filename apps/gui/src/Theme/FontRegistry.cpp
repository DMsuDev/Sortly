// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "Sortly/Gui/Icons.hpp"
#include "Sortly/Gui/Theme/FontManager.hpp"
#include "Sortly/Gui/Theme/UiTheme.hpp"

namespace Sortly::ui {

namespace {

// Static TTF files only: variable fonts are not supported.
constexpr const char* kRegular  = "fonts/Inter-Regular.ttf";
constexpr const char* kSemibold = "fonts/Inter-SemiBold.ttf";
constexpr const char* kMono     = "fonts/JetBrainsMono-Medium.ttf";

} // namespace

// One line per role. A new font = one more line (+ a pointer in UiTheme.hpp's `fonts`).
void registerDefaultFonts()
{
    FontManager::setIcons("fonts/lucide.ttf", kIconCodepoints);

    FontManager::addFont("body",    { kRegular,  "",        17.0f, 1.00f, &fonts::Body    });
    FontManager::addFont("label",   { kSemibold, kRegular,  12.5f, 1.00f, &fonts::Label   });
    FontManager::addFont("heading", { kSemibold, kRegular,  18.0f, 1.00f, &fonts::Heading });
    FontManager::addFont("title",   { kSemibold, kRegular,  24.0f, 0.95f, &fonts::Title   });
    FontManager::addFont("value",   { kMono,     kSemibold, 22.0f, 0.90f, &fonts::Value   });
}

} // namespace Sortly::ui
