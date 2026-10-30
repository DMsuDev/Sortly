// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "ThemeBindings.hpp"
#include "Sortly/Gui/UiTheme.hpp"

#include <imgui.h>
#include <implot.h>
#include <vigil/vigil.h>

#include <algorithm>
#include <cctype>
#include <charconv>
#include <span>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

namespace Sortly::ui::theme {

namespace {

using Json = nlohmann::json;

struct ColorEntry
{
    std::string key;
    ImVec4*     target;
};

struct StyleEntry
{
    const char* key;
    float*      scalar = nullptr;
    ImVec2*     vector = nullptr;
};

// ---------------------------------------------------------------------------
// Parsing
// ---------------------------------------------------------------------------

// "FrameBgHovered" -> "frame-bg-hovered"
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

// "#RRGGBB", "#RRGGBBAA" or (when allowed) "auto".
bool parseColor(const Json& value, bool allowAuto, ImVec4& out)
{
    if (!value.is_string())
        return false;

    const std::string& text = value.get_ref<const std::string&>();
    if (allowAuto && text == "auto")
    {
        out = IMPLOT_AUTO_COL;
        return true;
    }

    if ((text.size() != 7 && text.size() != 9) || text.front() != '#')
        return false;

    unsigned int      rgba = 0;
    const char* const last = text.data() + text.size();
    const auto [end, error] = std::from_chars(text.data() + 1, last, rgba, 16);
    if (error != std::errc{} || end != last)
        return false;

    if (text.size() == 7)
        rgba = (rgba << 8) | 0xFFu;

    const auto channel = [rgba](unsigned int shift) {
        return static_cast<float>((rgba >> shift) & 0xFFu) / 255.0f;
    };
    out = ImVec4(channel(24), channel(16), channel(8), channel(0));
    return true;
}

const Json& child(const Json& parent, const char* key)
{
    static const Json empty = Json::object();
    const auto        it    = parent.find(key);
    return (it != parent.end() && it->is_object()) ? *it : empty;
}

void applyColors(const char* section, const Json& json, std::span<const ColorEntry> entries, bool allowAuto)
{
    for (const auto& [key, value] : json.items())
    {
        const auto entry = std::ranges::find(entries, key, &ColorEntry::key);
        if (entry == entries.end())
        {
            VIGIL_LOG_WARN("Theme: unknown {} color '{}'", section, key);
            continue;
        }

        ImVec4 color;
        if (parseColor(value, allowAuto, color))
            *entry->target = color;
        else
            VIGIL_LOG_WARN("Theme: invalid value for {} color '{}'", section, key);
    }
}

void applyStyles(const char* section, const Json& json, std::span<const StyleEntry> entries)
{
    for (const auto& [key, value] : json.items())
    {
        const auto entry = std::ranges::find_if(entries, [&key](const StyleEntry& e) { return key == e.key; });
        if (entry == entries.end())
        {
            VIGIL_LOG_WARN("Theme: unknown {} style '{}'", section, key);
            continue;
        }

        if (entry->scalar && value.is_number())
        {
            *entry->scalar = value.get<float>();
        }
        else if (entry->vector && value.is_array() && value.size() == 2 && value[0].is_number() && value[1].is_number())
        {
            *entry->vector = ImVec2(value[0].get<float>(), value[1].get<float>());
        }
        else
        {
            VIGIL_LOG_WARN("Theme: invalid value for {} style '{}'", section, key);
        }
    }
}

// ---------------------------------------------------------------------------
// Bindings. Supporting another library = one colors() + one styles() function
// here, and two lines in applyBindings().
// ---------------------------------------------------------------------------

std::vector<ColorEntry> imguiColors(ImGuiStyle& style)
{
    std::vector<ColorEntry> out;
    for (int i = 0; i < ImGuiCol_COUNT; ++i)
        out.push_back({ toKebab(ImGui::GetStyleColorName(i)), &style.Colors[i] });
    return out;
}

std::vector<ColorEntry> implotColors(ImPlotStyle& style)
{
    std::vector<ColorEntry> out;
    for (int i = 0; i < ImPlotCol_COUNT; ++i)
        out.push_back({ toKebab(ImPlot::GetStyleColorName(i)), &style.Colors[i] });
    return out;
}

std::vector<ColorEntry> sortlyColors()
{
    return {
        { "bar-idle",    &colors::BarIdle },    { "accent",      &colors::Accent },
        { "warning",     &colors::Warning },    { "danger",      &colors::Danger },
        { "info",        &colors::Info },       { "violet",      &colors::Violet },
        { "success",     &colors::Success },    { "muted",       &colors::Muted },
        { "bar",         &colors::Bar },        { "surface",     &colors::Surface },
        { "surface-hi",  &colors::SurfaceHi },  { "clear",       &colors::Clear },
        { "text-dim",    &colors::TextDim },    { "border",      &colors::Border },
        { "card-border", &colors::CardBorder }, { "track",       &colors::Track },
        { "text-bright", &colors::TextBright }, { "on-accent",   &colors::OnAccent },
        { "slot-0",      &colors::Slot0 },      { "slot-1",      &colors::Slot1 },
        { "slot-2",      &colors::Slot2 },      { "slot-3",      &colors::Slot3 },
    };
}

// The values compiled in UiTheme.hpp are the defaults: captured before any theme touches them,
// so a theme that omits a token never inherits it from the previous one.
void restoreDefaults(std::span<const ColorEntry> entries)
{
    static const std::vector<ImVec4> defaults = [&] {
        std::vector<ImVec4> values;
        for (const ColorEntry& entry : entries)
            values.push_back(*entry.target);
        return values;
    }();

    for (std::size_t i = 0; i < entries.size(); ++i)
        *entries[i].target = defaults[i];
}

std::vector<StyleEntry> imguiStyles(ImGuiStyle& s)
{
    return {
        { "alpha", &s.Alpha },                         { "disabled-alpha", &s.DisabledAlpha },
        { "window-padding", nullptr, &s.WindowPadding }, { "window-rounding", &s.WindowRounding },
        { "window-border-size", &s.WindowBorderSize },
        { "child-rounding", &s.ChildRounding },        { "child-border-size", &s.ChildBorderSize },
        { "popup-rounding", &s.PopupRounding },        { "popup-border-size", &s.PopupBorderSize },
        { "frame-padding", nullptr, &s.FramePadding }, { "frame-rounding", &s.FrameRounding },
        { "frame-border-size", &s.FrameBorderSize },
        { "item-spacing", nullptr, &s.ItemSpacing },   { "item-inner-spacing", nullptr, &s.ItemInnerSpacing },
        { "cell-padding", nullptr, &s.CellPadding },   { "indent-spacing", &s.IndentSpacing },
        { "scrollbar-size", &s.ScrollbarSize },        { "scrollbar-rounding", &s.ScrollbarRounding },
        { "grab-min-size", &s.GrabMinSize },           { "grab-rounding", &s.GrabRounding },
        { "tab-rounding", &s.TabRounding },
    };
}

std::vector<StyleEntry> implotStyles(ImPlotStyle& s)
{
    return {
        { "plot-border-size", &s.PlotBorderSize },
        { "minor-alpha",      &s.MinorAlpha },
        { "major-tick-len",   nullptr, &s.MajorTickLen },
        { "minor-tick-len",   nullptr, &s.MinorTickLen },
        { "major-tick-size",  nullptr, &s.MajorTickSize },
        { "minor-tick-size",  nullptr, &s.MinorTickSize },
        { "major-grid-size",  nullptr, &s.MajorGridSize },
        { "minor-grid-size",  nullptr, &s.MinorGridSize },
        { "plot-padding",     nullptr, &s.PlotPadding },
        { "label-padding",    nullptr, &s.LabelPadding },
        { "legend-padding",   nullptr, &s.LegendPadding },
        { "legend-inner-padding", nullptr, &s.LegendInnerPadding },
        { "legend-spacing",   nullptr, &s.LegendSpacing },
        { "mouse-pos-padding", nullptr, &s.MousePosPadding },
        { "annotation-padding", nullptr, &s.AnnotationPadding },
        { "fit-padding",      nullptr, &s.FitPadding },
        { "plot-default-size", nullptr, &s.PlotDefaultSize },
        { "plot-min-size",    nullptr, &s.PlotMinSize },
    };
}

} // namespace

void applyBindings(const Json& data)
{
    const bool  light      = data.value("base", "dark") == "light";
    const Json& jsonColors = child(data, "colors");
    const Json& jsonStyles = child(data, "styles");

    ImGuiStyle& imgui = ImGui::GetStyle();
    imgui = ImGuiStyle();
    if (light)
        ImGui::StyleColorsLight(&imgui);
    else
        ImGui::StyleColorsDark(&imgui);
    applyColors("imgui", child(jsonColors, "imgui"), imguiColors(imgui), false);
    applyStyles("imgui", child(jsonStyles, "imgui"), imguiStyles(imgui));
    imgui.ScaleAllSizes(g_Scale);

    ImPlotStyle& implot = ImPlot::GetStyle();
    implot = ImPlotStyle();
    if (light)
        ImPlot::StyleColorsLight(&implot);
    else
        ImPlot::StyleColorsDark(&implot);
    applyColors("implot", child(jsonColors, "implot"), implotColors(implot), true);
    applyStyles("implot", child(jsonStyles, "implot"), implotStyles(implot));

    const std::vector<ColorEntry> tokens = sortlyColors();
    restoreDefaults(tokens);
    applyColors("sortly", child(jsonColors, "sortly"), tokens, false);
}

} // namespace Sortly::ui::theme
