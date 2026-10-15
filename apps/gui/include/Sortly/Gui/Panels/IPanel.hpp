// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#pragma once

#include "Sortly/Gui/UiContext.hpp"

namespace Sortly {

/// @brief Where a panel sits in the workspace. GuiApplication owns the layout: panels only
/// draw their own content and never position themselves relative to their siblings.
enum class PanelDock
{
    Top,   // Full-width row above everything else.
    Left,  // Row below, fixed width (width()).
    Fill,  // Takes the space left between the Left and Right panels.
    Right  // End of the row below, fixed width (width()).
};

class IPanel
{
public:
    virtual ~IPanel() = default;

    [[nodiscard]] virtual PanelDock dock() const = 0;

    /// @brief Fixed width of Left/Right panels, in scaled pixels. Ignored for the others.
    [[nodiscard]] virtual float width() const { return 0.0f; }

    virtual void draw(UiContext& ctx) = 0;
};

} // namespace Sortly
