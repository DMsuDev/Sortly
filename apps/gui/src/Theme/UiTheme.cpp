// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "Sortly/Gui/Theme/UiTheme.hpp"
#include "Sortly/Gui/Theme/Palette.hpp"
#include "Sortly/Gui/Theme/ThemeManager.hpp"
#include "Sortly/Gui/System.hpp"

#include <vigil/vigil.h>

namespace Sortly::ui {

ImVec4 operationColor(sortrace::OperationType type)
{
    switch (type)
    {
        case sortrace::OperationType::Compare:   return palette::getColor(palette::Color::Warning);
        case sortrace::OperationType::Swap:
        case sortrace::OperationType::Overwrite: return palette::getColor(palette::Color::Danger);
        case sortrace::OperationType::MarkSet:
        case sortrace::OperationType::MarkClear: return palette::getColor(palette::Color::Violet);
        case sortrace::OperationType::Sorted:    return palette::getColor(palette::Color::Success);
    }
    return palette::getColor(palette::Color::Accent);
}

void applyTheme()
{
    registerThemeHandlers();

    if (const auto dir = System::findAsset("themes"))
        ThemeManager::loadDirectory(*dir);
    else
        VIGIL_LOG_WARN("assets/themes not found: using the library defaults");

    if (!ThemeManager::changeTheme("Dark"))
        VIGIL_LOG_WARN("No 'Dark' theme available");
}

} // namespace Sortly::ui
