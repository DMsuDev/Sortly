// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "Sortly/Gui/Theme/UiTheme.hpp"
#include "Sortly/Gui/Theme/ThemeHandlers.hpp"
#include "Sortly/Gui/Theme/ThemeManager.hpp"
#include "Sortly/Gui/System.hpp"

#include <vigil/vigil.h>

namespace Sortly::ui {

ImVec4 operationColor(sortrace::OperationType type)
{
    switch (type)
    {
        case sortrace::OperationType::Compare:   return colors::Warning;
        case sortrace::OperationType::Swap:
        case sortrace::OperationType::Overwrite: return colors::Danger;
        case sortrace::OperationType::MarkSet:
        case sortrace::OperationType::MarkClear: return colors::Violet;
        case sortrace::OperationType::Sorted:    return colors::Success;
    }
    return colors::Accent;
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
