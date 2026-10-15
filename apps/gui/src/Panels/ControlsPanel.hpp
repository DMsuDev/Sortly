// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#pragma once

#include "Sortly/Gui/Panels/IPanel.hpp"

namespace Sortly {

class ControlsPanel final : public IPanel
{
public:
    [[nodiscard]] PanelDock dock() const override { return PanelDock::Left; }
    [[nodiscard]] float width() const override;
    void draw(UiContext& ctx) override;

private:
    float m_FooterHeight = 200.0f; // Pinned controls; replaced by the measured height after the first frame.
};

} // namespace Sortly
