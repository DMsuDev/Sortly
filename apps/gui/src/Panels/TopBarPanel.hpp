// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#pragma once

#include "Sortly/Gui/Panels/IPanel.hpp"

namespace Sortly {

class Window;

class TopBarPanel final : public IPanel
{
public:
    explicit TopBarPanel(Window& window) noexcept
        : m_Window(window)
    {}

    [[nodiscard]] PanelDock dock() const override { return PanelDock::Top; }
    void draw(UiContext& ctx) override;

private:
    Window& m_Window;
};

} // namespace Sortly
