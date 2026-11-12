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

    FontManager::addFont("body",    { .file=kRegular,  .fallback="",        .size=17.0f, .iconScale=1.00f, .target=&fonts::Body    });
    FontManager::addFont("label",   { .file=kSemibold, .fallback=kRegular,  .size=12.5f, .iconScale=1.00f, .target=&fonts::Label   });
    FontManager::addFont("heading", { .file=kSemibold, .fallback=kRegular,  .size=18.0f, .iconScale=1.00f, .target=&fonts::Heading });
    FontManager::addFont("title",   { .file=kSemibold, .fallback=kRegular,  .size=24.0f, .iconScale=0.95f, .target=&fonts::Title   });
    FontManager::addFont("value",   { .file=kMono,     .fallback=kSemibold, .size=22.0f, .iconScale=0.90f, .target=&fonts::Value   });
}

} // namespace Sortly::ui
