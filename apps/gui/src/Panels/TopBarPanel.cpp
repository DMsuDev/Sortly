// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "TopBarPanel.hpp"
#include "Sortly/Gui/Icons.hpp"
#include "Sortly/Gui/UiTheme.hpp"
#include "Sortly/Gui/Widgets/UiKit.hpp"

#include <imgui.h>

namespace Sortly {

void TopBarPanel::draw(UiContext&)
{
    const float height = ui::dp(76.0f);
    const float badge  = ui::dp(40.0f);
    const float margin = ui::dp(26.0f);

    ImGui::BeginChild("TopBar", ImVec2(0.0f, height), ImGuiChildFlags_None);

    const ImVec2 origin = ImGui::GetWindowPos();
    ImDrawList* dl = ImGui::GetWindowDrawList();

    // Logo badge, vertically centered in the bar.
    const ImVec2 badgeMin(origin.x + margin, origin.y + (height - badge) * 0.5f);
    const ImVec2 badgeMax(badgeMin.x + badge, badgeMin.y + badge);
    dl->AddRectFilled(badgeMin, badgeMax, ImGui::ColorConvertFloat4ToU32(ui::colors::withAlpha(ui::colors::Accent, 0.18f)), ui::dp(12.0f));
    dl->AddRect(badgeMin, badgeMax, ImGui::ColorConvertFloat4ToU32(ui::colors::withAlpha(ui::colors::Accent, 0.5f)), ui::dp(12.0f));

    ImGui::PushFont(ui::fonts::Title);
    const ImVec2 iconSize = ImGui::CalcTextSize(ICON_CHART);
    dl->AddText(ImVec2(badgeMin.x + (badge - iconSize.x) * 0.5f, badgeMin.y + (badge - iconSize.y) * 0.5f),
                ImGui::ColorConvertFloat4ToU32(ui::colors::Accent), ICON_CHART);
    const float titleHeight = ImGui::GetTextLineHeight();
    ImGui::PopFont();

    const float textX = margin + badge + ui::dp(14.0f);
    ImGui::PushFont(ui::fonts::Label);
    const float captionHeight = ImGui::GetTextLineHeight();
    ImGui::PopFont();

    const float blockHeight = titleHeight + captionHeight;
    ImGui::SetCursorPos(ImVec2(textX, (height - blockHeight) * 0.5f - ui::dp(2.0f)));
    ImGui::PushFont(ui::fonts::Title);
    ImGui::TextUnformatted("Sortly");
    ImGui::PopFont();

    ImGui::SetCursorPosX(textX);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetStyle().ItemSpacing.y + ui::dp(2.0f));
    ImGui::PushFont(ui::fonts::Label);
    ImGui::TextDisabled("SORTING LAB  \xC2\xB7  VISUALIZER");
    ImGui::PopFont();

    const float pillW = ui::pillWidth("DESKTOP APP");
    ImGui::SetCursorPos(ImVec2(ImGui::GetWindowWidth() - pillW - margin, (height - ui::dp(26.0f)) * 0.5f));
    ui::pill("DESKTOP APP", ui::colors::Accent);

    ImGui::EndChild();
}

} // namespace Sortly
