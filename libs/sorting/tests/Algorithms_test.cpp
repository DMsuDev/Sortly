// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "support/TestUtils.hpp"

#include <Sortly/Sorting/Algorithms/SortAlgorithmFactory.hpp>
#include <Sortly/Sorting/Operations/SortPlayback.hpp>

#include <gtest/gtest.h>

#include <algorithm>
#include <cstdint>
#include <ostream>
#include <string>
#include <vector>

using namespace Sortly;

// Lets gtest (and the test names CTest shows) print the algorithm instead of "4-byte object".
namespace Sortly {
void PrintTo(SortAlgorithmType type, std::ostream* os)
{
    std::string name{ SortAlgorithmFactory::toString(type) };
    std::erase(name, ' ');
    *os << name;
}
} // namespace Sortly

namespace {

class AlgorithmTest : public ::testing::TestWithParam<SortAlgorithmType> {};

std::string paramName(const ::testing::TestParamInfo<SortAlgorithmType>& info)
{
    std::string name{ SortAlgorithmFactory::toString(info.param) };
    std::erase(name, ' ');
    return name;
}

} // namespace

TEST_P(AlgorithmTest, SortsEveryStandardInput)
{
    for (const auto& input : test::standardInputs())
    {
        SCOPED_TRACE(::testing::PrintToString(input));

        auto algorithm = SortAlgorithmFactory::create(GetParam());
        ASSERT_NE(algorithm, nullptr);

        test::record(*algorithm, input);

        const auto result = algorithm->getData();
        EXPECT_TRUE(std::ranges::is_sorted(result));
        EXPECT_TRUE(std::ranges::is_permutation(result, input));
    }
}

// The recorded operations alone must be enough to reproduce the result (forward)
// and to return to the starting state (backward).
TEST_P(AlgorithmTest, OperationsReplayForwardAndBackward)
{
    for (const auto& input : test::standardInputs())
    {
        SCOPED_TRACE(::testing::PrintToString(input));

        auto algorithm = SortAlgorithmFactory::create(GetParam());
        const auto recording = test::record(*algorithm, input);

        SortPlayback playback;
        ASSERT_TRUE(playback.load(input, recording.operations));

        while (playback.stepForward()) {}
        EXPECT_TRUE(std::ranges::equal(playback.getVisualData(), algorithm->getData()));

        while (playback.stepBackward()) {}
        EXPECT_EQ(playback.getVisualData(), input);
        EXPECT_TRUE(playback.isStopped());
    }
}

// Contract of SortAlgorithm::step(): exactly one terminal Sorted operation, for any input
// (including empty and single-element ones), and nothing afterwards.
TEST_P(AlgorithmTest, EmitsExactlyOneTerminalSortedOperation)
{
    for (const auto& input : test::standardInputs())
    {
        SCOPED_TRACE(::testing::PrintToString(input));

        auto algorithm = SortAlgorithmFactory::create(GetParam());
        const auto recording = test::record(*algorithm, input);

        ASSERT_FALSE(recording.operations.empty());
        EXPECT_EQ(recording.operations.back().type, SortOperation::Type::Sorted);

        const auto sortedCount = std::ranges::count_if(recording.operations,
            [](const SortOperation& op) { return op.type == SortOperation::Type::Sorted; });
        EXPECT_EQ(sortedCount, 1);

        SortStatistics unused;
        SortRecorder recorder(unused);
        EXPECT_FALSE(algorithm->step(recorder).has_value());
    }
}

TEST_P(AlgorithmTest, LoadResetsTheAlgorithmForReuse)
{
    auto algorithm = SortAlgorithmFactory::create(GetParam());

    const std::vector<int> input{ 4, 2, 5, 1, 3 };
    const auto first  = test::record(*algorithm, input);
    const auto second = test::record(*algorithm, input);

    EXPECT_EQ(first.operations.size(), second.operations.size());
    EXPECT_TRUE(std::ranges::is_sorted(algorithm->getData()));
}

TEST_P(AlgorithmTest, StatisticsMatchRecordedOperations)
{
    auto algorithm = SortAlgorithmFactory::create(GetParam());
    const auto recording = test::record(*algorithm, std::vector<int>{5, 3, 4, 1, 2});

    const auto countOf = [&](SortOperation::Type type) {
        return static_cast<uint64_t>(std::ranges::count_if(recording.operations,
            [type](const SortOperation& op) { return op.type == type; }));
    };

    EXPECT_EQ(recording.statistics.operationCount, recording.operations.size());
    EXPECT_EQ(recording.statistics.swaps,          countOf(SortOperation::Type::Swap));
    EXPECT_EQ(recording.statistics.comparisons,    countOf(SortOperation::Type::Compare));
}

// A pivot mark never nests: each one is cleared before the next is set, it always points at
// a real element, and the run does not finish with one still set.
TEST_P(AlgorithmTest, PivotMarksNeverOverlap)
{
    for (const auto& input : test::standardInputs())
    {
        SCOPED_TRACE(::testing::PrintToString(input));

        auto algorithm = SortAlgorithmFactory::create(GetParam());
        const auto recording = test::record(*algorithm, input);

        bool marked = false;
        for (const SortOperation& op : recording.operations)
        {
            if (op.type != SortOperation::Type::Pivot)
                continue;

            if (op.indexA >= 0)
            {
                EXPECT_FALSE(marked) << "a pivot was set while another one was still marked";
                EXPECT_LT(static_cast<std::size_t>(op.indexA), input.size());
                marked = true;
            }
            else
            {
                EXPECT_TRUE(marked) << "clearPivot without a pivot";
                marked = false;
            }
        }
        EXPECT_FALSE(marked) << "the run ended with a pivot still marked";
    }
}

// While the playback reports a pivot, that element must really hold the pivot's value:
// the mark never drifts onto another element when the algorithm moves data around.
TEST_P(AlgorithmTest, ThePivotMarkAlwaysPointsAtThePivotValue)
{
    for (const auto& input : test::standardInputs())
    {
        SCOPED_TRACE(::testing::PrintToString(input));

        auto algorithm = SortAlgorithmFactory::create(GetParam());
        const auto recording = test::record(*algorithm, input);

        SortPlayback playback;
        ASSERT_TRUE(playback.load(input, recording.operations));

        int pivotValue = 0;
        while (playback.stepForward())
        {
            const SortOperation& applied = recording.operations[playback.getCurrentOperationIndex() - 1];
            if (applied.type == SortOperation::Type::Pivot && applied.indexA >= 0)
                pivotValue = applied.valueA;

            const int pivot = playback.getPivotIndex();
            if (pivot >= 0)
            {
                EXPECT_EQ(playback.getVisualData()[static_cast<std::size_t>(pivot)], pivotValue);
            }
        }
        EXPECT_EQ(playback.getPivotIndex(), -1);
    }
}

INSTANTIATE_TEST_SUITE_P(
    AllAlgorithms, AlgorithmTest,
    ::testing::ValuesIn(SortAlgorithmFactory::getAvailableAlgorithms()),
    paramName);

TEST(QuickSort, MarksOnePivotPerPartitionAndReadsItFromTheData)
{
    auto algorithm = SortAlgorithmFactory::create(SortAlgorithmType::QuickSort);
    ASSERT_NE(algorithm, nullptr);

    const std::vector<int> input{ 9, 2, 7, 4, 5, 6, 3, 8, 1, 10, 0, -4 };
    const auto recording = test::record(*algorithm, input);

    const auto marks = std::ranges::count_if(recording.operations,
        [](const SortOperation& op) { return op.type == SortOperation::Type::Pivot && op.indexA >= 0; });
    const auto clears = std::ranges::count_if(recording.operations,
        [](const SortOperation& op) { return op.type == SortOperation::Type::Pivot && op.indexA < 0; });

    EXPECT_GT(marks, 0);
    EXPECT_EQ(marks, clears);

    EXPECT_EQ(recording.statistics.reads,
          2 * recording.statistics.comparisons
              + 2 * recording.statistics.swaps);
}
