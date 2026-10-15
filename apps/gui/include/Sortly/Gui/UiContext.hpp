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
};

} // namespace Sortly
