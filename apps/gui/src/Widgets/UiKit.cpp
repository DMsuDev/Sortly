// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "Sortly/Gui/Widgets/UiKit.hpp"
#include "Sortly/Gui/UiTheme.hpp"

#include <algorithm>

namespace Sortly::ui {

namespace {

ImU32 toU32(const ImVec4& c) { return ImGui::ColorConvertFloat4ToU32(c); }

} // namespace

std::string formatCount(std::uint64_t value)
{
    std::string digits = std::to_string(value);
    for (int i = static_cast<int>(digits.size()) - 3; i > 0; i -= 3)
        digits.insert(static_cast<std::size_t>(i), ",");
    return digits;
}

void centeredText(const char* text, const ImVec4& color, ImFont* font)
{
    if (font) ImGui::PushFont(font);
    const float textWidth = ImGui::CalcTextSize(text).x;
    const float offset    = (ImGui::GetContentRegionAvail().x - textWidth) * 0.5f;
    if (offset > 0.0f)
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);
    ImGui::TextColored(color, "%s", text);
    if (font) ImGui::PopFont();
}

void sectionLabel(const char* icon, const char* label)
{
    ImGui::PushFont(fonts::Label);
    ImGui::TextColored(colors::Accent, "%s", icon);
    ImGui::SameLine(0.0f, dp(8.0f));
    ImGui::TextColored(colors::Muted, "%s", label);
    ImGui::PopFont();
}

bool iconButton(const char* label, const ImVec2& size, ButtonKind kind)
{
    if (kind == ButtonKind::Primary)
    {
        ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.18f, 0.74f, 0.65f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.84f, 0.74f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.14f, 0.62f, 0.55f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Text,          colors::OnAccent);
    }

    ImGui::PushFont(fonts::Heading);
    const bool pressed = ImGui::Button(label, size);
    ImGui::PopFont();

    if (kind == ButtonKind::Primary)
        ImGui::PopStyleColor(4);
    return pressed;
}

float pillWidth(const char* text)
{
    ImGui::PushFont(fonts::Label);
    const float width = ImGui::CalcTextSize(text).x + dp(12.0f) * 2.0f;
    ImGui::PopFont();
    return width;
}

void pill(const char* text, const ImVec4& color)
{
    const float padX = dp(12.0f);
    const float padY = dp(4.0f);

    ImGui::PushFont(fonts::Label);
    const ImVec2 textSize = ImGui::CalcTextSize(text);
    const ImVec2 size(textSize.x + padX * 2.0f, textSize.y + padY * 2.0f);
    const ImVec2 pos = ImGui::GetCursorScreenPos();

    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y),
                      toU32(colors::withAlpha(color, 0.16f)), size.y * 0.5f);
    dl->AddRect(pos, ImVec2(pos.x + size.x, pos.y + size.y),
                toU32(colors::withAlpha(color, 0.45f)), size.y * 0.5f);
    dl->AddText(ImVec2(pos.x + padX, pos.y + padY), toU32(color), text);
    ImGui::PopFont();

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

    // Caption row: tinted icon followed by the label.
    ImGui::PushFont(fonts::Label);
    const ImVec2 iconSize = ImGui::CalcTextSize(icon);
    dl->AddText(ImVec2(pos.x + padX, pos.y + padY), toU32(color), icon);
    dl->AddText(ImVec2(pos.x + padX + iconSize.x + dp(6.0f), pos.y + padY), toU32(colors::Muted), label);
    ImGui::PopFont();

    ImGui::PushFont(fonts::Value);
    const ImVec2 valueSize = ImGui::CalcTextSize(value.c_str());
    const ImVec2 valuePos(pos.x + padX, end.y - padY - valueSize.y);
    dl->AddText(valuePos, toU32(colors::TextBright), value.c_str());
    ImGui::PopFont();

    if (unit && *unit)
    {
        ImGui::PushFont(fonts::Body);
        const ImVec2 unitSize = ImGui::CalcTextSize(unit);
        dl->AddText(ImVec2(valuePos.x + valueSize.x + dp(6.0f), end.y - padY - unitSize.y - dp(2.0f)),
                    toU32(colors::TextDim), unit);
        ImGui::PopFont();
    }

    ImGui::Dummy(size);
}

void progressBar(float fraction, float height)
{
    const float width = ImGui::GetContentRegionAvail().x;
    const ImVec2 pos  = ImGui::GetCursorScreenPos();
    const float rounding = height * 0.5f;
    fraction = std::clamp(fraction, 0.0f, 1.0f);

    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->AddRectFilled(pos, ImVec2(pos.x + width, pos.y + height), toU32(colors::Track), rounding);
    if (fraction > 0.0f)
        dl->AddRectFilled(pos, ImVec2(pos.x + std::max(height, width * fraction), pos.y + height),
                          toU32(colors::Accent), rounding);

    ImGui::Dummy(ImVec2(width, height));
}

} // namespace Sortly::ui
