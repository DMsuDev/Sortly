// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include <Sortly/Sorting/Algorithms/SortAlgorithmFactory.hpp>

#include <gtest/gtest.h>

#include <algorithm>

using namespace Sortly;

TEST(SortAlgorithmFactory, EveryRegisteredTypeCreatesAnAlgorithmWithTheRegisteredName)
{
    for (const auto& info : SortAlgorithmFactory::all())
    {
        const auto algorithm = SortAlgorithmFactory::create(info.type);

        ASSERT_NE(algorithm, nullptr);
        EXPECT_EQ(algorithm->getName(), info.name);
        EXPECT_EQ(SortAlgorithmFactory::toString(info.type), info.name);
    }
}

TEST(SortAlgorithmFactory, AvailableAlgorithmsAreUnique)
{
    auto types = SortAlgorithmFactory::getAvailableAlgorithms();

    EXPECT_EQ(types.size(), SortAlgorithmFactory::all().size());

    std::ranges::sort(types);
    EXPECT_EQ(std::ranges::adjacent_find(types), types.end());
}

TEST(SortAlgorithmFactory, UnknownTypeIsNotFound)
{
    const auto unknown = static_cast<SortAlgorithmType>(999);

    EXPECT_EQ(SortAlgorithmFactory::find(unknown), nullptr);
    EXPECT_EQ(SortAlgorithmFactory::create(unknown), nullptr);
    EXPECT_EQ(SortAlgorithmFactory::toString(unknown), "Unknown Algorithm");
}
