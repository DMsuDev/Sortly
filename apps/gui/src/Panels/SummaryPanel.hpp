// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#pragma once

#include "Sortly/Gui/Panels/IPanel.hpp"

namespace Sortly {

/// @brief Right column: metric table, comparison chart and the winner.
class SummaryPanel final : public IPanel
{
public:
    [[nodiscard]] PanelDock dock() const override { return PanelDock::Right; }
    [[nodiscard]] float width() const override;
    void draw(UiContext& ctx) override;

private:
    int m_ChartMetric = 0;
};

} // namespace Sortly
