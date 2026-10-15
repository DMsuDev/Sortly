// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "Sortly/Gui/Panels/TopBarPanel.hpp"
#include "Sortly/Gui/Icons.hpp"
#include "Sortly/Gui/UiTheme.hpp"
#include "Sortly/Gui/Widgets/UiKit.hpp"

#include <imgui.h>

namespace Sortly {

void TopBarPanel::draw(UiContext&)
{
    constexpr float kHeight = 76.0f;
    constexpr float kBadge  = 40.0f;

    ImGui::BeginChild("TopBar", ImVec2(0.0f, kHeight), false);

    const ImVec2 origin = ImGui::GetWindowPos();
    ImDrawList* dl = ImGui::GetWindowDrawList();

    // Logo badge, vertically centered in the bar.
    const ImVec2 badgeMin(origin.x + 26.0f, origin.y + (kHeight - kBadge) * 0.5f);
    const ImVec2 badgeMax(badgeMin.x + kBadge, badgeMin.y + kBadge);
    dl->AddRectFilled(badgeMin, badgeMax, ImGui::ColorConvertFloat4ToU32(ui::colors::withAlpha(ui::colors::Accent, 0.18f)), 12.0f);
    dl->AddRect(badgeMin, badgeMax, ImGui::ColorConvertFloat4ToU32(ui::colors::withAlpha(ui::colors::Accent, 0.5f)), 12.0f);

    ImGui::PushFont(ui::fonts::Title);
    const ImVec2 iconSize = ImGui::CalcTextSize(ICON_CHART);
    dl->AddText(ImVec2(badgeMin.x + (kBadge - iconSize.x) * 0.5f, badgeMin.y + (kBadge - iconSize.y) * 0.5f),
                ImGui::ColorConvertFloat4ToU32(ui::colors::Accent), ICON_CHART);
    const float titleHeight = ImGui::GetTextLineHeight();
    ImGui::PopFont();

    const float textX = 26.0f + kBadge + 14.0f;
    ImGui::PushFont(ui::fonts::Label);
    const float captionHeight = ImGui::GetTextLineHeight();
    ImGui::PopFont();

    const float blockHeight = titleHeight + captionHeight;
    ImGui::SetCursorPos(ImVec2(textX, (kHeight - blockHeight) * 0.5f - 2.0f));
    ImGui::PushFont(ui::fonts::Title);
    ImGui::TextUnformatted("Sortly");
    ImGui::PopFont();

    ImGui::SetCursorPosX(textX);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetStyle().ItemSpacing.y + 2.0f);
    ImGui::PushFont(ui::fonts::Label);
    ImGui::TextDisabled("SORTING LAB  \xC2\xB7  VISUALIZER");
    ImGui::PopFont();

    const float pillW = ui::pillWidth("DESKTOP APP");
    ImGui::SetCursorPos(ImVec2(ImGui::GetWindowWidth() - pillW - 26.0f, (kHeight - 26.0f) * 0.5f));
    ui::pill("DESKTOP APP", ui::colors::Accent);

    ImGui::EndChild();
}

} // namespace Sortly
