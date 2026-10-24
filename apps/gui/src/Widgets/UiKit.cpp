// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "Sortly/Gui/Widgets/UiKit.hpp"
#include "Sortly/Gui/UiTheme.hpp"

#include <algorithm>
#include <cstddef>

namespace Sortly::ui {

namespace {

// Shared by pill() and pillWidth() so they can never disagree.
constexpr float kPillPadX = 12.0f;
constexpr float kPillPadY = 4.0f;

// glyphButton metrics.
constexpr float kGlyphBox  = 14.0f;
constexpr float kGlyphGap  = 8.0f;
constexpr float kGlyphPadX = 14.0f;

// Padding added to one text line to get a compact button height.
constexpr float kCompactPadY = 12.0f;

const ImVec4 kPrimary       { 0.18f, 0.74f, 0.65f, 1.0f };
const ImVec4 kPrimaryHover  { 0.25f, 0.84f, 0.74f, 1.0f };
const ImVec4 kPrimaryActive { 0.14f, 0.62f, 0.55f, 1.0f };

} // namespace

std::string formatCount(std::uint64_t value)
{
    std::string digits = std::to_string(value);
    for (std::size_t i = digits.size(); i > 3; i -= 3)
        digits.insert(i - 3, ",");
    return digits;
}

void centeredText(const char* text, const ImVec4& color, ImFont* font)
{
    const ScopedFont scopedFont(font);
    const float offset = (ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(text).x) * 0.5f;
    if (offset > 0.0f)
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);
    ImGui::TextColored(color, "%s", text);
}

void rightAlignedText(const char* text, const ImVec4& color)
{
    const float offset = ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(text).x;
    if (offset > 0.0f)
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);
    ImGui::TextColored(color, "%s", text);
}

void sectionLabel(const char* icon, const char* label)
{
    const ScopedFont font(fonts::Label);
    ImGui::TextColored(colors::Accent, "%s", icon);
    ImGui::SameLine(0.0f, dp(8.0f));
    ImGui::TextColored(colors::Muted, "%s", label);
}

ImVec2 buttonSize(const char* label, ImFont* font)
{
    const ScopedFont scoped(font ? font : fonts::Heading);
    const ImVec2 padding = ImGui::GetStyle().FramePadding;
    const ImVec2 text    = ImGui::CalcTextSize(label);
    return ImVec2(text.x + padding.x * 2.0f, text.y + padding.y * 2.0f);
}

float compactButtonHeight()
{
    const ScopedFont font(fonts::Body);
    return ImGui::GetTextLineHeight() + dp(kCompactPadY);
}

bool iconButton(const char* label, const ImVec2& size, ButtonKind kind, ImFont* font)
{
    const bool primary = kind == ButtonKind::Primary;

    const ScopedColor base   (ImGuiCol_Button,        kPrimary,         primary);
    const ScopedColor hovered(ImGuiCol_ButtonHovered, kPrimaryHover,    primary);
    const ScopedColor active (ImGuiCol_ButtonActive,  kPrimaryActive,   primary);
    const ScopedColor text   (ImGuiCol_Text,          colors::OnAccent, primary);
    const ScopedFont  scoped (font ? font : fonts::Heading);

    return ImGui::Button(label, size);
}

float glyphButtonWidth(const char* label)
{
    const ScopedFont font(fonts::Body);
    return dp(kGlyphPadX) * 2.0f + dp(kGlyphBox) + dp(kGlyphGap) + ImGui::CalcTextSize(label).x;
}

bool glyphButton(const char* id, const char* label, GlyphDrawFn glyph, const ImVec4& tint, const ImVec2& size)
{
    const ImVec2 pos = ImGui::GetCursorScreenPos();
    const ImVec2 end(pos.x + size.x, pos.y + size.y);

    const bool pressed = ImGui::InvisibleButton(id, size);
    const bool hovered = ImGui::IsItemHovered();
    const bool held    = ImGui::IsItemActive();

    const float fillAlpha   = held ? 0.30f : hovered ? 0.22f : 0.12f;
    const float borderAlpha = hovered ? 0.85f : 0.50f;
    const float rounding    = size.y * 0.5f;

    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->AddRectFilled(pos, end, toU32(colors::withAlpha(tint, fillAlpha)), rounding);
    dl->AddRect(pos, end, toU32(colors::withAlpha(tint, borderAlpha)), rounding);

    const ScopedFont font(fonts::Body);
    const ImVec2 textSize = ImGui::CalcTextSize(label);
    const float  box      = dp(kGlyphBox);
    const float  content  = box + dp(kGlyphGap) + textSize.x;
    const float  startX   = pos.x + (size.x - content) * 0.5f;
    const float  centerY  = pos.y + size.y * 0.5f;

    if (glyph)
        glyph(dl, ImVec2(startX + box * 0.5f, centerY), box * 0.5f, toU32(tint));
    dl->AddText(ImVec2(startX + box + dp(kGlyphGap), centerY - textSize.y * 0.5f), toU32(tint), label);

    return pressed;
}

void verticalDivider(float height)
{
    const ImVec2 pos   = ImGui::GetCursorScreenPos();
    const float  width = dp(1.0f);
    const float  inset = height * 0.2f;

    ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(pos.x, pos.y + inset), ImVec2(pos.x + width, pos.y + height - inset),
                                              toU32(colors::Border));
    ImGui::Dummy(ImVec2(width, height));
}

float pillWidth(const char* text)
{
    const ScopedFont font(fonts::Label);
    return ImGui::CalcTextSize(text).x + dp(kPillPadX) * 2.0f;
}

void pill(const char* text, const ImVec4& color)
{
    const float padX = dp(kPillPadX);
    const float padY = dp(kPillPadY);

    const ScopedFont font(fonts::Label);
    const ImVec2 textSize = ImGui::CalcTextSize(text);
    const ImVec2 size(textSize.x + padX * 2.0f, textSize.y + padY * 2.0f);
    const ImVec2 pos = ImGui::GetCursorScreenPos();
    const ImVec2 end(pos.x + size.x, pos.y + size.y);

    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->AddRectFilled(pos, end, toU32(colors::withAlpha(color, 0.16f)), size.y * 0.5f);
    dl->AddRect(pos, end, toU32(colors::withAlpha(color, 0.45f)), size.y * 0.5f);
    dl->AddText(ImVec2(pos.x + padX, pos.y + padY), toU32(color), text);

    ImGui::Dummy(size);
}

void statCard(const char* icon, const char* label, const std::string& value,
              const char* unit, const ImVec4& color, const ImVec2& size)
{
    const ImVec2 pos = ImGui::GetCursorScreenPos();
    const ImVec2 end(pos.x + size.x, pos.y + size.y);
    ImDrawList* dl = ImGui::GetWindowDrawList();

    const float rounding = dp(10.0f);
    dl->AddRectFilled(pos, end, toU32(colors::SurfaceHi), rounding);
    dl->AddRect(pos, end, toU32(colors::CardBorder), rounding);
    // Thin accent strip on the left edge ties the card to its metric color.
    dl->AddRectFilled(ImVec2(pos.x + 1.0f, pos.y + dp(10.0f)), ImVec2(pos.x + dp(4.0f), end.y - dp(10.0f)),
                      toU32(color), dp(2.0f));

    const float padX = dp(14.0f);
    const float padY = dp(9.0f);

    {
        // Caption row: tinted icon followed by the label.
        const ScopedFont font(fonts::Label);
        const ImVec2 iconSize = ImGui::CalcTextSize(icon);
        dl->AddText(ImVec2(pos.x + padX, pos.y + padY), toU32(color), icon);
        dl->AddText(ImVec2(pos.x + padX + iconSize.x + dp(6.0f), pos.y + padY), toU32(colors::Muted), label);
    }

    ImVec2 valuePos;
    ImVec2 valueSize;
    {
        const ScopedFont font(fonts::Value);
        valueSize = ImGui::CalcTextSize(value.c_str());
        valuePos  = ImVec2(pos.x + padX, end.y - padY - valueSize.y);
        dl->AddText(valuePos, toU32(colors::TextBright), value.c_str());
    }

    if (unit && *unit)
    {
        const ScopedFont font(fonts::Body);
        const ImVec2 unitSize = ImGui::CalcTextSize(unit);
        dl->AddText(ImVec2(valuePos.x + valueSize.x + dp(6.0f), end.y - padY - unitSize.y - dp(2.0f)),
                    toU32(colors::TextDim), unit);
    }

    ImGui::Dummy(size);
}

void progressBar(float fraction, float height, const ImVec4* color)
{
    const float  width    = ImGui::GetContentRegionAvail().x;
    const ImVec2 pos      = ImGui::GetCursorScreenPos();
    const float  rounding = height * 0.5f;
    fraction = std::clamp(fraction, 0.0f, 1.0f);

    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->AddRectFilled(pos, ImVec2(pos.x + width, pos.y + height), toU32(colors::Track), rounding);
    if (fraction > 0.0f)
        dl->AddRectFilled(pos, ImVec2(pos.x + std::max(height, width * fraction), pos.y + height),
                          toU32(color ? *color : colors::Accent), rounding);

    ImGui::Dummy(ImVec2(width, height));
}

} // namespace Sortly::ui
