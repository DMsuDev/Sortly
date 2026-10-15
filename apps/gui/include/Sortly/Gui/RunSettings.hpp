// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#pragma once

#include "Sortly/Sorting/Algorithms/SortAlgorithmFactory.hpp"
#include "Sortly/Sorting/Operations/SortPlayback.hpp"

namespace Sortly {

struct RunSettings
{
    static constexpr int kMinElements = 16;
    static constexpr int kMaxElements = 512;

    SortAlgorithmType algorithm    = SortAlgorithmType::BubbleSort;
    int               elementCount = 128;
    float             stepDelayMs  = 32.0f;
};

} // namespace Sortly
