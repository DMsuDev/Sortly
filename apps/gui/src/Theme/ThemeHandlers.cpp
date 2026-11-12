// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "Sortly/Gui/Theme/ThemeManager.hpp"
#include "Sortly/Gui/Theme/Palette.hpp"

#include <imgui.h>
#include <implot.h>

#include <cctype>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
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

namespace {

palette::Color paletteColor(std::uint32_t id)
{
    if (id >= static_cast<std::uint32_t>(palette::Color::Count))
        throw std::out_of_range("Invalid Sortly palette color ID");

    return static_cast<palette::Color>(id);
}

} // namespace

// --- ImGui -------------------------------------------------------------------

void registerColorHandlers()
{
    ThemeManager::ColorMap imguiColorMap;
    for (int i = 0; i < ImGuiCol_COUNT; ++i)
        imguiColorMap[toKebab(ImGui::GetStyleColorName(i))] = static_cast<std::uint32_t>(i);

    ThemeManager::addThemeHandler("imgui", imguiColorMap,
        [](std::uint32_t id) -> ImColor {
            return ImGui::GetStyle().Colors[id];
        },
        [](std::uint32_t id, ImVec4 color) {
            ImGui::GetStyle().Colors[id] = color;
        }
    );

    // --- ImPlot colors -------------------------------------------------------
    ThemeManager::ColorMap implotColorMap;
    for (int i = 0; i < ImPlotCol_COUNT; ++i)
        implotColorMap[toKebab(ImPlot::GetStyleColorName(i))] = static_cast<std::uint32_t>(i);

    ThemeManager::addThemeHandler("implot", implotColorMap,
        [](std::uint32_t id) -> ImColor {
            return ImPlot::GetStyle().Colors[id];
        },
        [](std::uint32_t id, ImVec4 color) {
            ImPlot::GetStyle().Colors[id] = color;
        }
    );

    // --- Sortly colors -------------------------------------------------------
    ThemeManager::ColorMap sortlyColorMap;
    for (const auto& color : palette::getColors())
        sortlyColorMap.emplace(color.name, static_cast<std::uint32_t>(color.id));

    ThemeManager::addThemeHandler("sortly", sortlyColorMap,
        [](std::uint32_t id) -> ImColor {
            return palette::getColor(paletteColor(id));
        },
        [](std::uint32_t id, ImVec4 color) {
            palette::setColor(paletteColor(id), color);
        }
    );
}

void registerStyleHandlers()
{
    {
        ImGuiStyle& style = ImGui::GetStyle();
        ThemeManager::StyleMap imguiStyleMap = {
            { "alpha",                      { .value=&style.Alpha,                      .min=0.1F,    .max=1.0F,          .needsScaling=false } },
            { "disabled-alpha",             { .value=&style.DisabledAlpha,              .min=0.0F,    .max=1.0F,          .needsScaling=false } },
            { "window-padding",             { .value=&style.WindowPadding,              .min=0.0F,    .max=kMaxSize,      .needsScaling=true  } },
            { "window-rounding",            { .value=&style.WindowRounding,             .min=0.0F,    .max=kMaxSize,      .needsScaling=true  } },
            { "window-border-size",         { .value=&style.WindowBorderSize,           .min=0.0F,    .max=kMaxBorder,    .needsScaling=true  } },
            { "window-min-size",            { .value=&style.WindowMinSize,              .min=0.0F,    .max=kMaxWindow,    .needsScaling=true  } },
            { "window-title-align",         { .value=&style.WindowTitleAlign,           .min=0.0F,    .max=1.0F,          .needsScaling=false } },
            { "child-rounding",             { .value=&style.ChildRounding,              .min=0.0F,    .max=kMaxSize,      .needsScaling=true  } },
            { "child-border-size",          { .value=&style.ChildBorderSize,            .min=0.0F,    .max=kMaxBorder,    .needsScaling=true  } },
            { "popup-rounding",             { .value=&style.PopupRounding,              .min=0.0F,    .max=kMaxSize,      .needsScaling=true  } },
            { "popup-border-size",          { .value=&style.PopupBorderSize,            .min=0.0F,    .max=kMaxBorder,    .needsScaling=true  } },
            { "frame-padding",              { .value=&style.FramePadding,               .min=0.0F,    .max=kMaxSize,      .needsScaling=true  } },
            { "frame-rounding",             { .value=&style.FrameRounding,              .min=0.0F,    .max=kMaxSize,      .needsScaling=true  } },
            { "frame-border-size",          { .value=&style.FrameBorderSize,            .min=0.0F,    .max=kMaxBorder,    .needsScaling=true  } },
            { "item-spacing",               { .value=&style.ItemSpacing,                .min=0.0F,    .max=kMaxSize,      .needsScaling=true  } },
            { "item-inner-spacing",         { .value=&style.ItemInnerSpacing,           .min=0.0F,    .max=kMaxSize,      .needsScaling=true  } },
            { "cell-padding",               { .value=&style.CellPadding,                .min=0.0F,    .max=kMaxSize,      .needsScaling=true  } },
            { "touch-extra-padding",        { .value=&style.TouchExtraPadding,          .min=0.0F,    .max=10.0F,         .needsScaling=true  } },
            { "indent-spacing",             { .value=&style.IndentSpacing,              .min=0.0F,    .max=kMaxSize,      .needsScaling=true  } },
            { "columns-min-spacing",        { .value=&style.ColumnsMinSpacing,          .min=0.0F,    .max=kMaxSize,      .needsScaling=true  } },
            { "scrollbar-size",             { .value=&style.ScrollbarSize,              .min=0.0F,    .max=kMaxSize,      .needsScaling=true  } },
            { "scrollbar-rounding",         { .value=&style.ScrollbarRounding,          .min=0.0F,    .max=kMaxSize,      .needsScaling=true  } },
            { "grab-min-size",              { .value=&style.GrabMinSize,                .min=0.0F,    .max=kMaxSize,      .needsScaling=true  } },
            { "grab-rounding",              { .value=&style.GrabRounding,               .min=0.0F,    .max=kMaxSize,      .needsScaling=true  } },
            { "log-slider-deadzone",        { .value=&style.LogSliderDeadzone,          .min=0.0F,    .max=kMaxSize,      .needsScaling=true  } },
            { "tab-rounding",               { .value=&style.TabRounding,                .min=0.0F,    .max=kMaxSize,      .needsScaling=true  } },
            { "tab-border-size",            { .value=&style.TabBorderSize,              .min=0.0F,    .max=kMaxBorder,    .needsScaling=true  } },
            { "separator-text-border-size", { .value=&style.SeparatorTextBorderSize,    .min=0.0F,    .max=kMaxBorder,    .needsScaling=true  } },
            { "separator-text-align",       { .value=&style.SeparatorTextAlign,         .min=0.0F,    .max=1.0F,          .needsScaling=false } },
            { "separator-text-padding",     { .value=&style.SeparatorTextPadding,       .min=0.0F,    .max=kMaxSize,      .needsScaling=true  } },
            { "display-window-padding",     { .value=&style.DisplayWindowPadding,       .min=0.0F,    .max=kMaxSize,      .needsScaling=true  } },
            { "display-safe-area-padding",  { .value=&style.DisplaySafeAreaPadding,     .min=0.0F,    .max=kMaxSize,      .needsScaling=true  } },
            { "mouse-cursor-scale",         { .value=&style.MouseCursorScale,           .min=0.1F,    .max=10.0F,         .needsScaling=true  } },
        };

        ThemeManager::addStyleHandler("imgui", imguiStyleMap);
    }

    // --- ImPlot styles -------------------------------------------------------
    // ImPlot 1.0 moved line weight, marker size, fill alpha... to ImPlotSpec: they are per item,
    // not part of the style, so they are not themed here.
    {
        ImPlotStyle& style = ImPlot::GetStyle();
        ThemeManager::StyleMap implotStyleMap = {
            { "plot-border-size",     { .value=&style.PlotBorderSize,        .min=0.0F,    .max=kMaxBorder,    .needsScaling=true  } },
            { "minor-alpha",          { .value=&style.MinorAlpha,            .min=0.0F,    .max=1.0F,          .needsScaling=false } },
            { "major-tick-len",       { .value=&style.MajorTickLen,          .min=0.0F,    .max=kMaxSize,      .needsScaling=true  } },
            { "minor-tick-len",       { .value=&style.MinorTickLen,          .min=0.0F,    .max=kMaxSize,      .needsScaling=true  } },
            { "major-tick-size",      { .value=&style.MajorTickSize,         .min=0.0F,    .max=kMaxBorder,    .needsScaling=true  } },
            { "minor-tick-size",      { .value=&style.MinorTickSize,         .min=0.0F,    .max=kMaxBorder,    .needsScaling=true  } },
            { "major-grid-size",      { .value=&style.MajorGridSize,         .min=0.0F,    .max=kMaxBorder,    .needsScaling=true  } },
            { "minor-grid-size",      { .value=&style.MinorGridSize,         .min=0.0F,    .max=kMaxBorder,    .needsScaling=true  } },
            { "plot-padding",         { .value=&style.PlotPadding,           .min=0.0F,    .max=kMaxSize,      .needsScaling=true  } },
            { "label-padding",        { .value=&style.LabelPadding,          .min=0.0F,    .max=kMaxSize,      .needsScaling=true  } },
            { "legend-padding",       { .value=&style.LegendPadding,         .min=0.0F,    .max=kMaxSize,      .needsScaling=true  } },
            { "legend-inner-padding", { .value=&style.LegendInnerPadding,    .min=0.0F,    .max=kMaxSize,      .needsScaling=true  } },
            { "legend-spacing",       { .value=&style.LegendSpacing,         .min=0.0F,    .max=kMaxSize,      .needsScaling=true  } },
            { "mouse-pos-padding",    { .value=&style.MousePosPadding,       .min=0.0F,    .max=kMaxSize,      .needsScaling=true  } },
            { "annotation-padding",   { .value=&style.AnnotationPadding,     .min=0.0F,    .max=kMaxSize,      .needsScaling=true  } },
            { "fit-padding",          { .value=&style.FitPadding,            .min=0.0F,    .max=0.2F,          .needsScaling=false } },
            { "plot-default-size",    { .value=&style.PlotDefaultSize,       .min=0.0F,    .max=kMaxWindow,    .needsScaling=true  } },
            { "plot-min-size",        { .value=&style.PlotMinSize,           .min=0.0F,    .max=kMaxWindow,    .needsScaling=true  } },
        };

        ThemeManager::addStyleHandler("implot", implotStyleMap);
    }
}

void registerThemeHandlers()
{
    ThemeManager::clearHandlers();
    registerColorHandlers();
    registerStyleHandlers();
}

} // namespace Sortly::ui
