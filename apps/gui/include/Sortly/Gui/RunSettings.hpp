// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#pragma once

#include "Sortly/Sorting/Algorithms/SortAlgorithmFactory.hpp"

#include <cstdint>
#include <random>

namespace Sortly {

struct RunSettings
{
    static constexpr int kMinElements = 16;
    static constexpr int kMaxElements = 512;

    SortAlgorithmType algorithm    = SortAlgorithmType::BubbleSort;
    int               elementCount = 128;
    float             stepDelayMs  = 32.0f;

    // Same seed + same element count = same dataset, so different algorithms can be compared
    // on identical input. reshuffle() picks a new one.
    std::uint32_t     seed         = std::random_device{}();

    void reshuffle() { seed = std::random_device{}(); }
};

} // namespace Sortly
