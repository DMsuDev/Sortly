// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include <Sortly/Sorting/Data/DataSetBuilder.hpp>

#include <gtest/gtest.h>

#include <algorithm>
#include <random>

using namespace Sortly;

TEST(DataSetBuilder, BuildIsSortedAndSpansTheValueRange)
{
    const auto dataSet = DataSetBuilder::build(DataSetConfig{ 10, 2, 4096, 100 });

    ASSERT_EQ(dataSet.size(), 10u);
    EXPECT_TRUE(std::ranges::is_sorted(dataSet.values()));
    EXPECT_EQ(dataSet.values().front(), 1);
    EXPECT_EQ(dataSet.values().back(), 100);
}

TEST(DataSetBuilder, CountIsClampedToMinimum)
{
    const auto dataSet = DataSetBuilder::build(DataSetConfig{ 1, 2, 4096, 100 });

    EXPECT_EQ(dataSet.size(), 2u);
}

TEST(DataSetBuilder, SameSeedProducesTheSameShuffle)
{
    const DataSetConfig config{ 32, 2, 4096, 100 };
    std::mt19937 firstRng{ 7 };
    std::mt19937 secondRng{ 7 };

    const auto first = DataSetBuilder::buildShuffled(config, firstRng);
    const auto second = DataSetBuilder::buildShuffled(config, secondRng);

    EXPECT_TRUE(std::ranges::equal(first.values(), second.values()));
}
