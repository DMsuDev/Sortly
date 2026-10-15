// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "support/TestUtils.hpp"

#include <Sortly/Sorting/Algorithms/SortAlgorithmFactory.hpp>
#include <Sortly/Sorting/Operations/SortPlayback.hpp>

#include <gtest/gtest.h>

#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

using namespace Sortly;

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

INSTANTIATE_TEST_SUITE_P(
    AllAlgorithms, AlgorithmTest,
    ::testing::ValuesIn(SortAlgorithmFactory::getAvailableAlgorithms()),
    paramName);
