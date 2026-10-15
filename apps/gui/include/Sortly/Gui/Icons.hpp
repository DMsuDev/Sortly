// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#pragma once

// Lucide icon font (third_party/imgui/lucide.ttf). Codepoints are in the Private Use Area;
// each constant is the UTF-8 encoding of one glyph so it can be concatenated into labels:
//     ImGui::Button(ICON_PLAY " Play");
// Add a glyph here AND to kIconCodepoints so the font atlas bakes it.

#define ICON_PLAY          "\xEE\x84\xBC"  // U+E13C
#define ICON_PAUSE         "\xEE\x84\xAE"  // U+E12E
#define ICON_SKIP_BACK     "\xEE\x85\x9F"  // U+E15F
#define ICON_SKIP_FORWARD  "\xEE\x85\xA0"  // U+E160
#define ICON_RESET         "\xEE\x85\x88"  // U+E148 (rotate-ccw)
#define ICON_SPARKLES      "\xEE\x90\x92"  // U+E412
#define ICON_SWAP          "\xEE\x89\x8A"  // U+E24A (arrow-left-right)
#define ICON_COMPARE       "\xEE\x8D\x99"  // U+E359 (git-compare)
#define ICON_ACTIVITY      "\xEE\x80\xB8"  // U+E038
#define ICON_TIMER         "\xEE\x87\xA0"  // U+E1E0
#define ICON_LIST          "\xEE\x87\x91"  // U+E1D1 (list-ordered)
#define ICON_CHART         "\xEE\x8A\xA3"  // U+E2A3 (bar-chart-3)
#define ICON_SLIDERS       "\xEE\x8A\x9A"  // U+E29A (sliders-horizontal)
#define ICON_SHUFFLE       "\xEE\x85\x9E"  // U+E15E
#define ICON_GAUGE         "\xEE\x86\xBF"  // U+E1BF
#define ICON_CPU           "\xEE\x82\xA9"  // U+E0A9
#define ICON_INFO          "\xEE\x83\xB9"  // U+E0F9
#define ICON_HASH          "\xEE\x83\xAF"  // U+E0EF
#define ICON_ZAP           "\xEE\x86\xB4"  // U+E1B4
#define ICON_CHECK         "\xEE\x81\xAC"  // U+E06C
#define ICON_LOADER        "\xEE\x84\x89"  // U+E109
#define ICON_STOP          "\xEE\x85\xA7"  // U+E167 (square)
#define ICON_TROPHY        "\xEE\x8D\xB3"  // U+E373
#define ICON_GRID          "\xEE\x83\xBF"  // U+E0FF (layout-grid)
#define ICON_COLUMNS_2     "\xEE\x82\x98"  // U+E098
#define ICON_COLUMNS_3     "\xEE\x82\x99"  // U+E099
#define ICON_SINGLE        "\xEE\x8D\xB6"  // U+E376 (rectangle-horizontal)

namespace Sortly::ui {

inline constexpr unsigned short kIconCodepoints[] = {
    0xE13C, 0xE12E, 0xE15F, 0xE160, 0xE148, 0xE412, 0xE24A, 0xE359, 0xE038, 0xE1E0,
    0xE1D1, 0xE2A3, 0xE29A, 0xE15E, 0xE1BF, 0xE0A9, 0xE0F9, 0xE0EF, 0xE1B4, 0xE06C,
    0xE109, 0xE167, 0xE373, 0xE0FF, 0xE098, 0xE099, 0xE376,
};

} // namespace Sortly::ui
