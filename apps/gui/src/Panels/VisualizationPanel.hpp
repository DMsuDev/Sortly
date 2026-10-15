// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#pragma once

#include "Sortly/Gui/Panels/IPanel.hpp"

namespace Sortly {

class VisualizationPanel final : public IPanel
{
public:
    [[nodiscard]] PanelDock dock() const override { return PanelDock::Fill; }
    void draw(UiContext& ctx) override;
};

} // namespace Sortly
