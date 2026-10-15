// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#pragma once

#include "Sortly/Sorting/Algorithms/SortAlgorithmFactory.hpp"

#include <cstddef>
#include <cstdint>
#include <random>
#include <vector>

namespace Sortly {

/// @brief How many algorithms are shown side by side.
enum class ViewMode
{
    Single,
    Compare2,
    Compare3,
    Compare4
};

[[nodiscard]] constexpr std::size_t slotCount(ViewMode mode)
{
    switch (mode)
    {
        case ViewMode::Single:   return 1;
        case ViewMode::Compare2: return 2;
        case ViewMode::Compare3: return 3;
        case ViewMode::Compare4: return 4;
    }
    return 1;
}

struct RunSettings
{
    static constexpr int         kMinElements = 16;
    static constexpr int         kMaxElements = 512;
    static constexpr std::size_t kMaxSlots    = 4;

    static constexpr float kMinSpeed = 0.25f;
    static constexpr float kMaxSpeed = 64.0f;

    ViewMode view = ViewMode::Single;

    // Selection order = slot order (slot 0 is the first card). Holds at most capacity() entries.
    std::vector<SortAlgorithmType> algorithms;

    int   elementCount = 128;
    float speed        = 2.0f; // Step-rate multiplier; 1x is one operation every 64 ms.

    // Same seed + same element count = same dataset, so every algorithm of a run (and of
    // later runs) is compared on identical input. reshuffle() picks a new one.
    std::uint32_t seed = std::random_device{}();

    /// @brief Starts in the widest comparison the registered algorithms allow.
    RunSettings();

    void reshuffle() { seed = std::random_device{}(); }

    [[nodiscard]] std::size_t capacity() const { return slotCount(view); }
    [[nodiscard]] bool        complete() const { return algorithms.size() == capacity(); }
    [[nodiscard]] float       stepDelayMs() const { return 64.0f / speed; }

    /// @brief A view needs as many distinct algorithms as it has slots.
    [[nodiscard]] static bool isViewAvailable(ViewMode mode);

    [[nodiscard]] bool isSelected(SortAlgorithmType type) const;

    /// @brief Checks/unchecks an algorithm. In single view the choice is replaced instead.
    /// @return True if the selection changed.
    bool toggle(SortAlgorithmType type);

    /// @brief Changes the view, dropping the extra selections or filling the free slots
    /// with the first algorithms not yet chosen. Unavailable views are ignored.
    void setView(ViewMode mode);

    /// @brief True if @p other would produce the same runs (speed is not part of it).
    [[nodiscard]] bool sameRunAs(const RunSettings& other) const
    {
        return algorithms == other.algorithms && elementCount == other.elementCount && seed == other.seed;
    }
};

} // namespace Sortly
