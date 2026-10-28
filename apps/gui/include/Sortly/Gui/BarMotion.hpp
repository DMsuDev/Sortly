// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#pragma once

#include "Sortly/Sorting/Operations/SortPlayback.hpp"

#include <cstddef>
#include <span>
#include <vector>

namespace Sortly {

/// @brief Animated position and height of every bar in a run.
/// Each bar keeps its identity: a Swap exchanges the slots of two bars and each one
/// slides toward its new slot. Overwrites only change heights.
class BarMotion
{
public:
    /// @param easeSeconds Time constant of the smoothing.
    /// @param smooth      false = bars jump straight to their target.
    void update(const SortPlayback& playback, float deltaTime, float easeSeconds, bool smooth);

    [[nodiscard]] std::size_t size() const { return m_X.size(); }
    [[nodiscard]] std::size_t owner(std::size_t slot) const { return m_Owner[slot]; }
    [[nodiscard]] float       x(std::size_t bar) const { return m_X[bar]; }
    [[nodiscard]] float       height(std::size_t bar) const { return m_Height[bar]; }

private:
    /// @return true if the playback jumped (first use, Stop, restart): no animation.
    bool sync(const SortPlayback& playback);
    void reset(std::span<const int> initial);
    void applySwap(const SortOperation& op);

    std::vector<std::size_t> m_Owner;  // slot -> bar
    std::vector<std::size_t> m_Slot;   // bar -> slot
    std::vector<float>       m_X;      // per bar
    std::vector<float>       m_Height; // per bar
    std::size_t              m_Cursor = 0;
};

} // namespace Sortly
