// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "Sortly/Gui/Theme/ThemeManager.hpp"
#include "Sortly/Gui/Theme/UiTheme.hpp"

#include <imgui.h>
#include <implot.h>

#include <cctype>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>

namespace Sortly::ui {

// Constants for style limits
constexpr float kMaxSize = 64.0f;
constexpr float kMaxBorder = 8.0f;
constexpr float kMaxWindow = 1000.0f;

// Converts camelCase or PascalCase names to kebab-case for theme keys. "FrameBgHovered" --> "frame-bg-hovered"
std::string toKebab(std::string_view name)
{
    std::string out;
    for (std::size_t i = 0; i < name.size(); ++i)
    {
        const auto c = static_cast<unsigned char>(name[i]);
        if (i > 0 && std::isupper(c))
            out += '-';
        out += static_cast<char>(std::tolower(c));
    }
    return out;
}

ThemeManager::Style number(float* value, float min, float max, bool needsScaling = true)
{
    return { value, min, max, needsScaling };
}

ThemeManager::Style pair(ImVec2* value, float min, float max, bool needsScaling = true)
{
    return { value, min, max, needsScaling };
}

// --- ImGui -------------------------------------------------------------------

void registerImGui()
{
    ThemeManager::ColorMap colorMap;
    for (int i = 0; i < ImGuiCol_COUNT; ++i)
        colorMap[toKebab(ImGui::GetStyleColorName(i))] = static_cast<std::uint32_t>(i);

    ThemeManager::addThemeHandler("imgui", colorMap,
        [](std::uint32_t id) { return ImGui::GetStyle().Colors[id]; },
        [](std::uint32_t id, ImVec4 color) { ImGui::GetStyle().Colors[id] = color; });

    ImGuiStyle& s = ImGui::GetStyle();
    ThemeManager::addStyleHandler("imgui", {
        { "alpha", number(&s.Alpha, 0.1f, 1.0f, false) },
        { "disabled-alpha", number(&s.DisabledAlpha, 0.0f, 1.0f, false) },
        { "window-padding", pair(&s.WindowPadding, 0.0f, kMaxSize) },
        { "window-rounding", number(&s.WindowRounding, 0.0f, kMaxSize) },
        { "window-border-size", number(&s.WindowBorderSize, 0.0f, kMaxBorder) },
        { "window-min-size", pair(&s.WindowMinSize, 0.0f, kMaxWindow) },
        { "window-title-align", pair(&s.WindowTitleAlign, 0.0f, 1.0f, false) },
        { "child-rounding", number(&s.ChildRounding, 0.0f, kMaxSize) },
        { "child-border-size", number(&s.ChildBorderSize, 0.0f, kMaxBorder) },
        { "popup-rounding", number(&s.PopupRounding, 0.0f, kMaxSize) },
        { "popup-border-size", number(&s.PopupBorderSize, 0.0f, kMaxBorder) },
        { "frame-padding", pair(&s.FramePadding, 0.0f, kMaxSize) },
        { "frame-rounding", number(&s.FrameRounding, 0.0f, kMaxSize) },
        { "frame-border-size", number(&s.FrameBorderSize, 0.0f, kMaxBorder) },
        { "item-spacing", pair(&s.ItemSpacing, 0.0f, kMaxSize) },
        { "item-inner-spacing", pair(&s.ItemInnerSpacing, 0.0f, kMaxSize) },
        { "cell-padding", pair(&s.CellPadding, 0.0f, kMaxSize) },
        { "touch-extra-padding", pair(&s.TouchExtraPadding, 0.0f, 10.0f) },
        { "indent-spacing", number(&s.IndentSpacing, 0.0f, kMaxSize) },
        { "columns-min-spacing", number(&s.ColumnsMinSpacing, 0.0f, kMaxSize) },
        { "scrollbar-size", number(&s.ScrollbarSize, 0.0f, kMaxSize) },
        { "scrollbar-rounding", number(&s.ScrollbarRounding, 0.0f, kMaxSize) },
        { "grab-min-size", number(&s.GrabMinSize, 0.0f, kMaxSize) },
        { "grab-rounding", number(&s.GrabRounding, 0.0f, kMaxSize) },
        { "log-slider-deadzone", number(&s.LogSliderDeadzone, 0.0f, kMaxSize) },
        { "tab-rounding", number(&s.TabRounding, 0.0f, kMaxSize) },
        { "tab-border-size", number(&s.TabBorderSize, 0.0f, kMaxBorder) },
        { "separator-text-border-size", number(&s.SeparatorTextBorderSize, 0.0f, kMaxBorder) },
        { "separator-text-align", pair(&s.SeparatorTextAlign, 0.0f, 1.0f, false) },
        { "separator-text-padding", pair(&s.SeparatorTextPadding, 0.0f, kMaxSize) },
        { "display-window-padding", pair(&s.DisplayWindowPadding, 0.0f, kMaxSize) },
        { "display-safe-area-padding", pair(&s.DisplaySafeAreaPadding, 0.0f, kMaxSize) },
        { "mouse-cursor-scale", number(&s.MouseCursorScale, 0.1f, 10.0f) },
    });
}

// --- ImPlot ------------------------------------------------------------------
// ImPlot 1.0 moved line weight, marker size, fill alpha... to ImPlotSpec: they are per item,
// not part of the style, so they are not themed here.

void registerImPlot()
{
    ThemeManager::ColorMap colorMap;
    for (int i = 0; i < ImPlotCol_COUNT; ++i)
        colorMap[toKebab(ImPlot::GetStyleColorName(i))] = static_cast<std::uint32_t>(i);

    ThemeManager::addThemeHandler("implot", colorMap,
        [](std::uint32_t id) { return ImPlot::GetStyle().Colors[id]; },
        [](std::uint32_t id, ImVec4 color) { ImPlot::GetStyle().Colors[id] = color; });

    ImPlotStyle& s = ImPlot::GetStyle();
    ThemeManager::addStyleHandler("implot", {
        { "plot-border-size", number(&s.PlotBorderSize, 0.0f, kMaxBorder) },
        { "minor-alpha", number(&s.MinorAlpha, 0.0f, 1.0f, false) },
        { "major-tick-len", pair(&s.MajorTickLen, 0.0f, kMaxSize) },
        { "minor-tick-len", pair(&s.MinorTickLen, 0.0f, kMaxSize) },
        { "major-tick-size", pair(&s.MajorTickSize, 0.0f, kMaxBorder) },
        { "minor-tick-size", pair(&s.MinorTickSize, 0.0f, kMaxBorder) },
        { "major-grid-size", pair(&s.MajorGridSize, 0.0f, kMaxBorder) },
        { "minor-grid-size", pair(&s.MinorGridSize, 0.0f, kMaxBorder) },
        { "plot-padding", pair(&s.PlotPadding, 0.0f, kMaxSize) },
        { "label-padding", pair(&s.LabelPadding, 0.0f, kMaxSize) },
        { "legend-padding", pair(&s.LegendPadding, 0.0f, kMaxSize) },
        { "legend-inner-padding", pair(&s.LegendInnerPadding, 0.0f, kMaxSize) },
        { "legend-spacing", pair(&s.LegendSpacing, 0.0f, kMaxSize) },
        { "mouse-pos-padding", pair(&s.MousePosPadding, 0.0f, kMaxSize) },
        { "annotation-padding", pair(&s.AnnotationPadding, 0.0f, kMaxSize) },
        { "fit-padding", pair(&s.FitPadding, 0.0f, 0.2f, false) },
        { "plot-default-size", pair(&s.PlotDefaultSize, 0.0f, kMaxWindow) },
        { "plot-min-size", pair(&s.PlotMinSize, 0.0f, kMaxWindow) },
    });
}

// --- Sortly tokens -----------------------------------------------------------
// Adding a token = one line here + the same key in the theme JSON files.

struct Token
{
    const char* key;
    ImVec4* color;
};

std::span<const Token> sortlyTokens()
{
    static const Token tokens[] = {
        { "bar-idle", &colors::BarIdle },
        { "accent", &colors::Accent },
        { "warning", &colors::Warning },
        { "danger", &colors::Danger },
        { "info", &colors::Info },
        { "violet", &colors::Violet },
        { "success", &colors::Success },
        { "muted", &colors::Muted },
        { "bar", &colors::Bar },
        { "surface", &colors::Surface },
        { "surface-hi", &colors::SurfaceHi },
        { "clear", &colors::Clear },
        { "text-dim", &colors::TextDim },
        { "border", &colors::Border },
        { "card-border", &colors::CardBorder },
        { "track", &colors::Track },
        { "text-bright", &colors::TextBright },
        { "on-accent", &colors::OnAccent },
        { "slot-0", &colors::Slot0 },
        { "slot-1", &colors::Slot1 },
        { "slot-2", &colors::Slot2 },
        { "slot-3", &colors::Slot3 },
    };
    return tokens;
}

void registerSortly()
{
    const std::span<const Token> tokens = sortlyTokens();

    ThemeManager::ColorMap colorMap;
    for (std::size_t i = 0; i < tokens.size(); ++i)
        colorMap[tokens[i].key] = static_cast<std::uint32_t>(i);

    ThemeManager::addThemeHandler("sortly", colorMap,
        [](std::uint32_t id) { return *sortlyTokens()[id].color; },
        [](std::uint32_t id, ImVec4 color) { *sortlyTokens()[id].color = color; });
}

} // namespace

void registerThemeHandlers()
{
    ThemeManager::clearHandlers();
    registerImGui();
    registerImPlot();
    registerSortly();
}

} // namespace Sortly::ui
