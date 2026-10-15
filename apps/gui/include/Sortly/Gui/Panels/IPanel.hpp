// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#pragma once

#include "Sortly/Gui/UiContext.hpp"

namespace Sortly {

/// @brief Where a panel sits in the workspace. GuiApplication owns the layout: panels only
/// draw their own content and never position themselves relative to their siblings.
enum class PanelDock
{
    Top,  // Full-width row above everything else.
    Left, // First item of the row below, fixed width.
    Fill  // Takes the remaining width next to the Left panel.
};

class IPanel
{
public:
    virtual ~IPanel() = default;

    [[nodiscard]] virtual PanelDock dock() const = 0;
    virtual void draw(UiContext& ctx) = 0;
};

} // namespace Sortly
