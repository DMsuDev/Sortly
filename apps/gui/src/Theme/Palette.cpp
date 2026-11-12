// Palette.cpp
// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "Sortly/Gui/Theme/Palette.hpp"

#include <array>
#include <cassert>

namespace Sortly::ui::palette {
namespace {

constexpr std::size_t kColorCount = static_cast<std::size_t>(Color::Count);

constexpr std::array<ColorInfo, kColorCount> kColors{{
    { .id=Color::BarIdle,    .name="bar-idle",    .defaultValue={0.36f,  0.41f,  0.48f,  1.00f} },
    { .id=Color::Accent,     .name="accent",      .defaultValue={0.31f,  0.86f,  0.75f,  1.00f} },
    { .id=Color::Warning,    .name="warning",     .defaultValue={1.00f,  0.72f,  0.31f,  1.00f} },
    { .id=Color::Danger,     .name="danger",      .defaultValue={1.00f,  0.42f,  0.46f,  1.00f} },
    { .id=Color::Info,       .name="info",        .defaultValue={0.40f,  0.65f,  1.00f,  1.00f} },
    { .id=Color::Violet,     .name="violet",      .defaultValue={0.68f,  0.55f,  1.00f,  1.00f} },
    { .id=Color::Success,    .name="success",     .defaultValue={0.40f,  0.88f,  0.52f,  1.00f} },
    { .id=Color::Muted,      .name="muted",       .defaultValue={0.67f,  0.77f,  0.84f,  1.00f} },
    { .id=Color::Bar,        .name="bar",         .defaultValue={0.20f,  0.75f,  0.66f,  0.88f} },
    { .id=Color::Surface,    .name="surface",     .defaultValue={0.075f, 0.090f, 0.118f, 1.00f} },
    { .id=Color::SurfaceHi,  .name="surface-hi",  .defaultValue={0.105f, 0.125f, 0.160f, 1.00f} },
    { .id=Color::Clear,      .name="clear",       .defaultValue={0.047f, 0.059f, 0.082f, 1.00f} },
    { .id=Color::TextDim,    .name="text-dim",    .defaultValue={0.48f,  0.54f,  0.62f,  1.00f} },
    { .id=Color::Border,     .name="border",      .defaultValue={0.17f,  0.20f,  0.25f,  0.80f} },
    { .id=Color::CardBorder, .name="card-border", .defaultValue={0.20f,  0.24f,  0.30f,  0.70f} },
    { .id=Color::Track,      .name="track",       .defaultValue={0.13f,  0.16f,  0.21f,  1.00f} },
    { .id=Color::TextBright, .name="text-bright", .defaultValue={0.94f,  0.97f,  1.00f,  1.00f} },
    { .id=Color::OnAccent,   .name="on-accent",   .defaultValue={0.03f,  0.10f,  0.10f,  1.00f} },
    { .id=Color::Slot0,      .name="slot-0",      .defaultValue={0.40f,  0.65f,  1.00f,  1.00f} },
    { .id=Color::Slot1,      .name="slot-1",      .defaultValue={0.96f,  0.52f,  0.80f,  1.00f} },
    { .id=Color::Slot2,      .name="slot-2",      .defaultValue={0.31f,  0.86f,  0.75f,  1.00f} },
    { .id=Color::Slot3,      .name="slot-3",      .defaultValue={0.68f,  0.55f,  1.00f,  1.00f} },
}};

// Every color listed once, in enum order, with a non-empty unique name.
constexpr bool isTableValid()
{
    for (std::size_t i = 0; i < kColors.size(); ++i)
    {
        if (static_cast<std::size_t>(kColors[i].id) != i || kColors[i].name.empty())
            return false;
        for (std::size_t j = 0; j < i; ++j)
            if (kColors[i].name == kColors[j].name)
                return false;
    }
    return true;
}
static_assert(isTableValid(), "kColors must list every Color once, in enum order, with unique names.");

constexpr std::size_t indexOf(Color color) noexcept
{
    assert(color < Color::Count);
    return static_cast<std::size_t>(color);
}

constexpr std::array<ImVec4, kColorCount> makeDefaults() noexcept
{
    std::array<ImVec4, kColorCount> values{};
    for (std::size_t i = 0; i < values.size(); ++i)
        values[i] = kColors[i].defaultValue;
    return values;
}

std::array<ImVec4, kColorCount> s_colors = makeDefaults();

} // namespace

// ---- Metadata ---------------------------------------------------------------------------

std::span<const ColorInfo> getColors()                { return kColors; }
std::string_view getColorName(Color color)            { return kColors[indexOf(color)].name; }
const ImVec4& getDefaultColor(Color color)            { return kColors[indexOf(color)].defaultValue; }

std::optional<Color> findColor(std::string_view name)
{
    for (const ColorInfo& info : kColors)
        if (info.name == name)
            return info.id;
    return std::nullopt;
}

// ---- Current palette --------------------------------------------------------------------

const ImVec4& getColor(Color color)                   { return s_colors[indexOf(color)]; }
void setColor(Color color, const ImVec4& value)       { s_colors[indexOf(color)] = value; }
void resetColor(Color color)                          { s_colors[indexOf(color)] = getDefaultColor(color); }
void resetAllColors()                                 { s_colors = makeDefaults(); }

// ---- Helpers ----------------------------------------------------------------------------

const ImVec4& getSlotColor(std::size_t index)
{
    constexpr std::array slots{ Color::Slot0, Color::Slot1, Color::Slot2, Color::Slot3 };
    return getColor(slots[index % slots.size()]);
}

ImU32 getColorU32(Color color, float alphaMultiplier)
{
    ImVec4 value = getColor(color);
    value.w *= alphaMultiplier;
    return ImGui::ColorConvertFloat4ToU32(value);
}

} // namespace Sortly::ui::palette
