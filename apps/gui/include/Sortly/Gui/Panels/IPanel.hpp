// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#pragma once

#include "Sortly/Gui/UiContext.hpp"

namespace Sortly {

class IPanel
{
public:
    virtual ~IPanel() = default;
    virtual void draw(UiContext& ctx) = 0;
};

} // namespace Sortly
