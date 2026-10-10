// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#pragma once

#include <Sortly/Sorting/Algorithms/SortAlgorithm.hpp>

#include <span>
#include <vector>

namespace Sortly::test {

struct Recording
{
    std::vector<SortOperation> operations;
    SortStatistics             statistics;
};

/// Runs @p algorithm to completion and returns everything it recorded.
inline Recording record(SortAlgorithm& algorithm, std::span<const int> data)
{
    Recording result;
    SortRecorder recorder(result.statistics);

    algorithm.load(data);
    while (auto op = algorithm.step(recorder))
        result.operations.push_back(*op);

    return result;
}

/// Inputs every algorithm must handle.
inline std::vector<std::vector<int>> standardInputs()
{
    return {
        {},
        {1},
        {2, 1},
        {1, 2, 3, 4, 5},
        {5, 4, 3, 2, 1},
        {3, 1, 3, 2, 1, 2},   // duplicates
        {7, 7, 7, 7},
        {9, 2, 7, 4, 5, 6, 3, 8, 1, 10, 0, -4},
    };
}

} // namespace Sortly::test
