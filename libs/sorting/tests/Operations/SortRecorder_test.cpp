// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include <Sortly/Sorting/Operations/detail/SortRecorder.hpp>

#include <gtest/gtest.h>

#include <vector>

using namespace Sortly;

TEST(SortRecorder, OverwriteCapturesOldValueAndMutatesBuffer)
{
    std::vector<int> buffer{ 1, 2, 3 };
    SortStatistics statistics;
    SortRecorder recorder(statistics);

    const auto operation = recorder.overwrite(buffer, 1, 9);

    EXPECT_EQ(buffer[1], 9);
    EXPECT_EQ(operation.valueA, 9);
    EXPECT_EQ(operation.valueB, 2);
    EXPECT_EQ(statistics.writes, 1u);
}

TEST(SortRecorder, SwapUpdatesStatistics)
{
    SortStatistics statistics;
    SortRecorder recorder(statistics);

    (void)recorder.swap(0, 1, 5, 3);

    EXPECT_EQ(statistics.swaps, 1u);
    EXPECT_EQ(statistics.reads, 2u);
    EXPECT_EQ(statistics.writes, 2u);
    EXPECT_EQ(statistics.operationCount, 1u);
}
