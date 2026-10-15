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

TEST(DataSetBuilder, CountIsClampedToMaximum)
{
    const auto dataSet = DataSetBuilder::build(DataSetConfig{ 500, 2, 64, 100 });

    EXPECT_EQ(dataSet.size(), 64u);
}

TEST(DataSetBuilder, InvertedCountRangeFavoursTheMinimum)
{
    // maxCount < minCount: std::clamp would be undefined behavior here.
    const DataSetConfig config{ 10, 8, 4, 100 };

    EXPECT_EQ(config.effectiveCount(), 8);
    EXPECT_EQ(DataSetBuilder::build(config).size(), 8u);
}

TEST(DataSetBuilder, NonPositiveCountYieldsAnEmptyDataSet)
{
    EXPECT_TRUE(DataSetBuilder::build(DataSetConfig{ 0, 0, 10, 100 }).empty());
}

TEST(DataSetBuilder, MaxValueBelowOneIsRaisedToOne)
{
    const auto dataSet = DataSetBuilder::build(DataSetConfig{ 4, 2, 4096, 0 });

    ASSERT_EQ(dataSet.size(), 4u);
    EXPECT_EQ(dataSet.values().back(), 1);
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

TEST(DataSetConfig, EqualityComparesEveryField)
{
    const DataSetConfig base{ 10, 2, 100, 50 };

    EXPECT_EQ(base, (DataSetConfig{ 10, 2, 100, 50 }));
    EXPECT_NE(base, (DataSetConfig{ 11, 2, 100, 50 }));
    EXPECT_NE(base, (DataSetConfig{ 10, 3, 100, 50 }));
    EXPECT_NE(base, (DataSetConfig{ 10, 2, 101, 50 }));
    EXPECT_NE(base, (DataSetConfig{ 10, 2, 100, 51 }));
}
