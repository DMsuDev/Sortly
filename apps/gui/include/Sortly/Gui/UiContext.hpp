// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#pragma once

#include "Sortly/Gui/RunController.hpp"
#include "Sortly/Gui/RunSettings.hpp"

namespace Sortly {

struct UiContext
{
    RunSettings&   settings;
    RunController& run;

    // Width the panel should give its root child, set by GuiApplication before each draw().
    // Negative means "remaining space minus this much" (ImGui convention).
    float          width = 0.0f;
};

} // namespace Sortly
