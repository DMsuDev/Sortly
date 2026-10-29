// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "TopBarPanel.hpp"

#include "Sortly/Gui/Icons.hpp"
#include "Sortly/Gui/UiTheme.hpp"
#include "Sortly/Gui/Widgets/UiKit.hpp"
#include "Sortly/Gui/Window.hpp"
#include "Sortly/Sorting/Algorithms/SortAlgorithmFactory.hpp"

#include <imgui.h>

#include <algorithm>
#include <array>
#include <cstddef>

namespace Sortly {

using namespace ui::literals; // 10_dp, 1.5_dp, ...

namespace {

constexpr float kBarHeight = 60.0f; // Minimum; grows with the font (see barHeight()).
constexpr float kMargin    = 20.0f;
constexpr float kBadge     = 34.0f;

constexpr float kViewPadX = 14.0f; // Frame padding of the segmented buttons.
constexpr float kViewPadY = 5.0f;
constexpr float kViewGap  = 2.0f;
constexpr float kViewPad  = 3.0f;  // Track padding around the segmented buttons.

constexpr float kCtrlWidth  = 34.0f;
constexpr float kCtrlGap    = 2.0f;
constexpr float kCtrlMargin = 10.0f;
constexpr float kDividerGap = 10.0f;

constexpr const char* kEnterFullscreenLabel = "Fullscreen";
constexpr const char* kExitFullscreenLabel  = "Exit fullscreen";

struct ViewOption
{
    const char* label;
    const char* tooltip;
    ViewMode    mode;
};

constexpr std::array<ViewOption, 4> kViewOptions = {{
    { ICON_SINGLE    "  Single",    "One algorithm, full size",    ViewMode::Single   },
    { ICON_COLUMNS_2 "  Compare 2", "Two algorithms side by side", ViewMode::Compare2 },
    { ICON_COLUMNS_3 "  Compare 3", "Three algorithms at once",    ViewMode::Compare3 },
    { ICON_GRID      "  Compare 4", "Four algorithms in a grid",   ViewMode::Compare4 },
}};

enum class Glyph { Minimize, Maximize, Restore, Close };

// Tall enough for the controls whatever the font size.
float barHeight() { return std::max(ui::dp(kBarHeight), ui::compactButtonHeight() + 24_dp); }

float centeredY(float height) { return (barHeight() - height) * 0.5f; }

void drawLogo()
{
    const float bar    = barHeight();
    const float badge  = ui::dp(kBadge);
    const float margin = ui::dp(kMargin);

    const ImVec2 origin = ImGui::GetWindowPos();
    ImDrawList*  dl     = ImGui::GetWindowDrawList();

    const ImVec2 min(origin.x + margin, origin.y + (bar - badge) * 0.5f);
    const ImVec2 max(min.x + badge, min.y + badge);
    const float  rounding = 10_dp;
    dl->AddRectFilled(min, max, ui::toU32(ui::colors::withAlpha(ui::colors::Accent, 0.18f)), rounding);
    dl->AddRect(min, max, ui::toU32(ui::colors::withAlpha(ui::colors::Accent, 0.5f)), rounding);

    float titleHeight = 0.0f;
    {
        const ui::ScopedFont font(ui::fonts::Title);
        const ImVec2 icon = ImGui::CalcTextSize(ICON_CHART);
        titleHeight = ImGui::GetTextLineHeight();
        dl->AddText(ImVec2(min.x + (badge - icon.x) * 0.5f, min.y + (badge - icon.y) * 0.5f),
                    ui::toU32(ui::colors::Accent), ICON_CHART);
    }

    float captionHeight = 0.0f;
    {
        const ui::ScopedFont font(ui::fonts::Label);
        captionHeight = ImGui::GetTextLineHeight();
    }

    const float textX = margin + badge + 10_dp;
    ImGui::SetCursorPos(ImVec2(textX, centeredY(titleHeight + captionHeight)));
    {
        const ui::ScopedFont font(ui::fonts::Title);
        ImGui::TextUnformatted("Sortly");
    }

    ImGui::SetCursorPosX(textX);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetStyle().ItemSpacing.y + 1_dp);
    const ui::ScopedFont font(ui::fonts::Label);
    ImGui::TextDisabled("SORTING LAB  \xC2\xB7  VISUALIZER");
}

// Segmented control: a rounded track with the active view highlighted.
void drawViewSelector(UiContext& ctx)
{
    RunSettings& settings = ctx.settings;

    // Width and height follow the font: a bigger font grows the buttons instead of overflowing them.
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ui::dp(kViewPadX, kViewPadY));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 9_dp);

    ImVec2 size(0.0f, ui::compactButtonHeight()); // Equal-width segments: the widest label decides.
    for (const ViewOption& option : kViewOptions)
    {
        const ImVec2 natural = ui::buttonSize(option.label, ui::fonts::Body);
        size.x = std::max(size.x, natural.x);
        size.y = std::max(size.y, natural.y);
    }

    const float gap   = ui::dp(kViewGap);
    const float pad   = ui::dp(kViewPad);
    const float count = static_cast<float>(kViewOptions.size());
    const float total = size.x * count + gap * (count - 1.0f);

    ImGui::SetCursorPos(ImVec2((ImGui::GetWindowWidth() - total) * 0.5f, centeredY(size.y)));

    const ImVec2 origin = ImGui::GetCursorScreenPos();
    ImGui::GetWindowDrawList()->AddRectFilled(
        ImVec2(origin.x - pad, origin.y - pad), ImVec2(origin.x + total + pad, origin.y + size.y + pad),
        ui::toU32(ui::colors::Track), 12_dp);

    for (std::size_t i = 0; i < kViewOptions.size(); ++i)
    {
        if (i > 0)
            ImGui::SameLine(0.0f, gap);

        const ViewOption& option    = kViewOptions[i];
        const bool        active    = settings.view == option.mode;
        const bool        available = RunSettings::isViewAvailable(option.mode);

        bool pressed = false;
        {
            const ui::ScopedColor transparent(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f), !active);
            ImGui::BeginDisabled(!available);
            pressed = ui::iconButton(option.label, size,
                                     active ? ui::ButtonKind::Primary : ui::ButtonKind::Secondary,
                                     ui::fonts::Body);
            ImGui::EndDisabled();
        }

        if (pressed && !active)
        {
            settings.setView(option.mode);
            if (settings.complete())
                ctx.run.requestStart(settings);
        }

        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled | ImGuiHoveredFlags_ForTooltip))
        {
            if (available)
                ui::setTooltip("%s", option.tooltip);
            else
                ui::setTooltip("Needs %zu algorithms; only %zu are implemented so far.",
                                  slotCount(option.mode), SortAlgorithmFactory::all().size());
        }
    }
    ImGui::PopStyleVar(2);
}

// Four corner brackets: arms point inward to enter fullscreen, outward to leave it.
void drawBrackets(ImDrawList* dl, const ImVec2& c, float half, ImU32 color, bool exit)
{
    const float corner = exit ? half * 0.4f : half;
    const float arm    = exit ? half * 0.6f : -half * 0.6f;
    const float stroke = 1.5_dp;

    for (const float sx : { -1.0f, 1.0f })
    {
        for (const float sy : { -1.0f, 1.0f })
        {
            const ImVec2 p(c.x + sx * corner, c.y + sy * corner);
            dl->AddLine(p, ImVec2(p.x + sx * arm, p.y), color, stroke);
            dl->AddLine(p, ImVec2(p.x, p.y + sy * arm), color, stroke);
        }
    }
}

void drawEnterFullscreen(ImDrawList* dl, const ImVec2& c, float half, ImU32 color) { drawBrackets(dl, c, half, color, false); }
void drawExitFullscreen(ImDrawList* dl, const ImVec2& c, float half, ImU32 color)  { drawBrackets(dl, c, half, color, true); }

void drawGlyph(Glyph glyph, const ImVec2& c, ImU32 color)
{
    ImDrawList* dl     = ImGui::GetWindowDrawList();
    const float half   = 5_dp;
    const float stroke = 1.5_dp;

    switch (glyph)
    {
        case Glyph::Minimize:
            dl->AddLine(ImVec2(c.x - half, c.y + 2_dp), ImVec2(c.x + half, c.y + 2_dp), color, stroke);
            break;
        case Glyph::Maximize:
            dl->AddRect(ImVec2(c.x - half, c.y - half), ImVec2(c.x + half, c.y + half), color, 0.0f, 0, stroke);
            break;
        case Glyph::Restore:
        {
            const float o = 2_dp;
            dl->AddRect(ImVec2(c.x - half + o, c.y - half - o), ImVec2(c.x + half + o, c.y + half - o), color, 0.0f, 0, stroke);
            dl->AddRect(ImVec2(c.x - half - o, c.y - half + o), ImVec2(c.x + half - o, c.y + half + o), color, 0.0f, 0, stroke);
            break;
        }
        case Glyph::Close:
            dl->AddLine(ImVec2(c.x - half, c.y - half), ImVec2(c.x + half, c.y + half), color, stroke);
            dl->AddLine(ImVec2(c.x + half, c.y - half), ImVec2(c.x - half, c.y + half), color, stroke);
            break;
    }
}

// Flat, icon-only button (minimize / maximize / close).
bool windowButton(const char* id, const char* tooltip, Glyph glyph, float height)
{
    const bool close = glyph == Glyph::Close;

    const ui::ScopedColor idle (ImGuiCol_Button,        ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    const ui::ScopedColor hover(ImGuiCol_ButtonHovered, ImVec4(0.75f, 0.16f, 0.20f, 1.0f), close);
    const ui::ScopedColor press(ImGuiCol_ButtonActive,  ImVec4(0.60f, 0.12f, 0.16f, 1.0f), close);

    const bool pressed = ImGui::Button(id, ImVec2(ui::dp(kCtrlWidth), height));

    const ImVec2 min = ImGui::GetItemRectMin();
    const ImVec2 max = ImGui::GetItemRectMax();
    drawGlyph(glyph, ImVec2((min.x + max.x) * 0.5f, (min.y + max.y) * 0.5f), ImGui::GetColorU32(ImGuiCol_Text));

    if (ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip))
        ui::setTooltip("%s", tooltip);
    return pressed;
}

// [Fullscreen pill] | [minimize] [maximize] [close]
// When the OS draws the frame (Wayland) only the fullscreen pill is shown: the frame has the rest.
void drawWindowControls(Window& window)
{
    const bool  osFrame = window.hasOsFrame();
    const float height  = ui::compactButtonHeight();
    const float gap     = ui::dp(kCtrlGap);
    const float divider = ui::dp(kDividerGap);

    // Sized for the longer label, so the group never shifts when the label changes.
    const float pillWidth     = ui::glyphButtonWidth(kExitFullscreenLabel);
    const float controlsWidth = ui::dp(kCtrlWidth) * 3.0f + gap * 2.0f;
    const float total         = osFrame ? pillWidth : pillWidth + divider * 2.0f + 1_dp + controlsWidth;

    ImGui::SetCursorPos(ImVec2(ImGui::GetWindowWidth() - ui::dp(kCtrlMargin) - total, centeredY(height)));

    const bool fullscreen = window.isFullscreen();
    if (ui::glyphButton("##Fullscreen", fullscreen ? kExitFullscreenLabel : kEnterFullscreenLabel,
                        fullscreen ? &drawExitFullscreen : &drawEnterFullscreen,
                        ui::colors::Accent, ImVec2(pillWidth, height)))
        window.toggleFullscreen();
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip))
        ui::setTooltip("%s (F11)", fullscreen ? "Leave fullscreen" : "Fill the whole screen");

    if (osFrame)
        return;

    ImGui::SameLine(0.0f, divider);
    ui::verticalDivider(height);
    ImGui::SameLine(0.0f, divider);

    if (windowButton("##Minimize", "Minimize", Glyph::Minimize, height))
        window.minimize();

    ImGui::SameLine(0.0f, gap);
    const bool maximized = window.isMaximized();
    if (windowButton("##Maximize", maximized ? "Restore" : "Maximize",
                     maximized ? Glyph::Restore : Glyph::Maximize, height))
        window.toggleMaximize();

    ImGui::SameLine(0.0f, gap);
    if (windowButton("##Close", "Close", Glyph::Close, height))
        window.requestClose();
}

} // namespace

void TopBarPanel::draw(UiContext& ctx)
{
    ImGui::BeginChild("TopBar", ImVec2(0.0f, barHeight()), ImGuiChildFlags_None);

    drawLogo();
    drawViewSelector(ctx);
    drawWindowControls(m_Window);

    // Free space in the bar moves the window; double click maximizes it. With an OS frame, the OS does both.
    if (!m_Window.hasOsFrame() && ImGui::IsWindowHovered() && !ImGui::IsAnyItemHovered() &&
        ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    {
        if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
            m_Window.toggleMaximize();
        else
            m_Window.beginDrag();
    }

    ImGui::EndChild();
}

} // namespace Sortly
