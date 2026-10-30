// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "Sortly/Gui/Widgets/UiKit.hpp"
#include "Sortly/Gui/UiTheme.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>

namespace Sortly::ui {

namespace {

// Shared by pill() and pillWidth() so they can never disagree.
constexpr float kPillPadX = 12.0f;
constexpr float kPillPadY = 4.0f;

// pillButton metrics.
constexpr float kGlyphBox  = 14.0f;
constexpr float kGlyphGap  = 8.0f;
constexpr float kGlyphPadX = 14.0f;

// Padding added to one text line to get a compact button height.
constexpr float kCompactPadY = 12.0f;

// cornerGlow mesh resolution.
constexpr int   kGlowRings    = 35; // 12 OPTIMAL VALUES
constexpr int   kGlowSegments = 16; // 16 OPTIMAL VALUES
constexpr float kQuarterTurn  = 1.5707964f;

const ImVec4 kPrimary       { 0.18f, 0.74f, 0.65f, 1.0f };
const ImVec4 kPrimaryHover  { 0.25f, 0.84f, 0.74f, 1.0f };
const ImVec4 kPrimaryActive { 0.14f, 0.62f, 0.55f, 1.0f };

void pushOverlayStyle()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,    dp(12.0f, 9.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding,   dp(10.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding,    dp(10.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, dp(1.0f));
    ImGui::PushStyleColor(ImGuiCol_Border, colors::withAlpha(colors::Accent, 0.30f));
}

void popOverlayStyle()
{
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(4);
}

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

void rightAlignedText(std::string_view text, const ImVec4& color)
{
    const char* begin = text.data();
    const char* end   = begin + text.size();

    const float offset = ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(begin, end).x;
    if (offset > 0.0f)
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);
    ImGui::PushStyleColor(ImGuiCol_Text, color);
    ImGui::TextUnformatted(begin, end);
    ImGui::PopStyleColor();
}

void rightAlignedOnLine(std::string_view text, const ImVec4& color)
{
    const char* begin = text.data();
    const char* end   = begin + text.size();

    ImGui::SameLine(ImGui::GetContentRegionMax().x - ImGui::CalcTextSize(begin, end).x);
    ImGui::PushStyleColor(ImGuiCol_Text, color);
    ImGui::TextUnformatted(begin, end);
    ImGui::PopStyleColor();
}

void sectionLabel(const char* icon, const char* label)
{
    const ScopedFont font(fonts::Label);
    ImGui::TextColored(colors::Accent, "%s", icon);
    ImGui::SameLine(0.0f, dp(8.0f));
    ImGui::TextColored(colors::Muted, "%s", label);
}

ImVec2 calculateButtonSize(const char* label, ImFont* font)
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

float calculatePillWidth(const char* label)
{
    const ScopedFont font(fonts::Body);
    return dp(kGlyphPadX) * 2.0f + dp(kGlyphBox) + dp(kGlyphGap) + ImGui::CalcTextSize(label).x;
}

bool pillButton(const char* id, const char* label, const char* icon, const ImVec4& tint, const ImVec2& size)
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

    const ImVec2 iconSize     = ImGui::CalcTextSize(icon);
    const ImVec2 textSize     = ImGui::CalcTextSize(label);
    const float  gap          = dp(kGlyphGap);
    const float  contentWidth = iconSize.x + gap + textSize.x;

    const float  startX   = pos.x + (size.x - contentWidth) * 0.5f;
    const float  centerY  = pos.y + size.y * 0.5f;

    dl->AddText(ImVec2(startX, centerY - iconSize.y * 0.5f), toU32(tint), icon);
    dl->AddText(ImVec2(startX + iconSize.x + gap, centerY - textSize.y * 0.5f), toU32(tint), label);

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

void cornerGlow(const ImVec2& min, float radius, float rounding, const ImVec4& color, float peakAlpha)
{
    constexpr int stride = kGlowSegments + 1;

    ImDrawList*  dl = ImGui::GetWindowDrawList();
    const ImVec2 uv = ImGui::GetFontTexUvWhitePixel();
    const ImVec2 arc(min.x + rounding, min.y + rounding); // Center of the rounded corner's arc.

    dl->PrimReserve(kGlowRings * kGlowSegments * 6, (kGlowRings + 1) * stride);
    const int first = static_cast<int>(dl->_VtxCurrentIdx);

    for (int ring = 0; ring <= kGlowRings; ++ring)
    {
        const float t    = static_cast<float>(ring) / static_cast<float>(kGlowRings);
        const float rho  = radius * t;
        const float fade = (1.0f - t) * (1.0f - t);
        const ImU32 col  = toU32(colors::withAlpha(color, peakAlpha * fade));

        for (int seg = 0; seg <= kGlowSegments; ++seg)
        {
            const float angle = kQuarterTurn * static_cast<float>(seg) / static_cast<float>(kGlowSegments);
            ImVec2 p(min.x + rho * std::cos(angle), min.y + rho * std::sin(angle));

            // Anything outside the rounded corner is projected back onto its arc.
            if (p.x < arc.x && p.y < arc.y)
            {
                const float dx  = p.x - arc.x;
                const float dy  = p.y - arc.y;
                const float len = std::sqrt(dx * dx + dy * dy);
                if (len > rounding)
                    p = ImVec2(arc.x + dx * rounding / len, arc.y + dy * rounding / len);
            }
            dl->PrimWriteVtx(p, uv, col);
        }
    }

    for (int ring = 0; ring < kGlowRings; ++ring)
    {
        for (int seg = 0; seg < kGlowSegments; ++seg)
        {
            const int a = first + ring * stride + seg;
            const int b = a + 1;
            const int c = a + stride + 1;
            const int d = a + stride;
            for (const int index : { a, b, c, a, c, d })
                dl->PrimWriteIdx(static_cast<ImDrawIdx>(index));
        }
    }
}

bool beginTooltip()
{
    pushOverlayStyle();
    const bool open = ImGui::BeginTooltip();
    popOverlayStyle();
    return open;
}

bool beginPopup(const char* id)
{
    pushOverlayStyle();
    const bool open = ImGui::BeginPopup(id);
    popOverlayStyle();
    return open;
}

void setTooltip(const char* fmt, ...)
{
    if (!beginTooltip())
        return;

    ImGui::PushTextWrapPos(dp(280.0f));
    va_list args;
    va_start(args, fmt);
    ImGui::TextV(fmt, args);
    va_end(args);
    ImGui::PopTextWrapPos();
    ImGui::EndTooltip();
}

} // namespace Sortly::ui
