// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include <Sortly/Sorting/Operations/SortRecorder.hpp>

#include <gtest/gtest.h>

#include <vector>

using namespace Sortly;

TEST(SortRecorder, CompareCapturesValuesAndLeavesTheBufferUntouched)
{
    std::vector<int> buffer{ 5, 3, 4 };
    SortStatistics statistics;
    SortRecorder recorder(statistics);

    const auto operation = recorder.compare(buffer, 0, 1);

    EXPECT_EQ(buffer, (std::vector<int>{ 5, 3, 4 }));
    EXPECT_EQ(operation.type, SortOperation::Type::Compare);
    EXPECT_EQ(operation.valueA, 5);
    EXPECT_EQ(operation.valueB, 3);
    EXPECT_EQ(statistics.comparisons, 1u);
    EXPECT_EQ(statistics.reads, 2u);
    EXPECT_EQ(statistics.writes, 0u);
}

TEST(SortRecorder, SwapMutatesTheBufferAndCapturesTheValuesBeforeTheSwap)
{
    std::vector<int> buffer{ 5, 3, 4 };
    SortStatistics statistics;
    SortRecorder recorder(statistics);

    const auto operation = recorder.swap(buffer, 0, 1);

    EXPECT_EQ(buffer, (std::vector<int>{ 3, 5, 4 }));
    EXPECT_EQ(operation.type, SortOperation::Type::Swap);
    EXPECT_EQ(operation.indexA, 0);
    EXPECT_EQ(operation.indexB, 1);
    EXPECT_EQ(operation.valueA, 5);
    EXPECT_EQ(operation.valueB, 3);
}

TEST(SortRecorder, SwapUpdatesStatistics)
{
    std::vector<int> buffer{ 5, 3 };
    SortStatistics statistics;
    SortRecorder recorder(statistics);

    (void)recorder.swap(buffer, 0, 1);

    EXPECT_EQ(statistics.swaps, 1u);
    EXPECT_EQ(statistics.reads, 2u);
    EXPECT_EQ(statistics.writes, 2u);
    EXPECT_EQ(statistics.operationCount, 1u);
}

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

TEST(SortRecorder, ReleasePivotHasNoIndexAndCostsNoRead)
{
    SortStatistics statistics;
    SortRecorder recorder(statistics);

    const auto operation = recorder.releasePivot();

    EXPECT_EQ(operation.type, SortOperation::Type::Pivot);
    EXPECT_EQ(operation.indexA, -1);
    EXPECT_EQ(statistics.reads, 0u);
    EXPECT_EQ(statistics.operationCount, 1u);
}

TEST(SortRecorder, SortedMarksTheEndOfTheRunWithoutAnIndex)
{
    SortStatistics statistics;
    SortRecorder recorder(statistics);

    const auto operation = recorder.sorted();

    EXPECT_EQ(operation.type, SortOperation::Type::Sorted);
    EXPECT_EQ(operation.indexA, -1);
    EXPECT_EQ(statistics.operationCount, 1u);
}
