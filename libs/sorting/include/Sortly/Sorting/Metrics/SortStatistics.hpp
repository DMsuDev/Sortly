// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>

namespace Sortly {

// Per-session sorting statistics. Every SortSession owns its own instance -
// never store these as globals, since multiple sessions can run at the same time.
struct SortStatistics
{
    uint64_t comparisons    = 0;
    uint64_t swaps          = 0;
    uint64_t reads          = 0;
    uint64_t writes         = 0;
    uint64_t operationCount = 0;

    // Time spent inside SortSession::computeOperations() actually running the algorithm.
    // Never includes SortPlayback time - playback speed is a UI/UX concern, not a
    // measure of the algorithm's performance, so it must stay out of this metric.
    double computeTimeMs = 0.0;

    void reset() { *this = SortStatistics{}; }
};

} // namespace Sortly
