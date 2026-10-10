// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include <Sortly/Sorting/Data/detail/SeedProvider.hpp>

#include <gtest/gtest.h>

using namespace Sortly;

TEST(SeedProvider, GlobalSeedIsIdenticalForEverySession)
{
    const auto provider = SeedProvider::globalSeed(7);

    EXPECT_EQ(provider.seedFor(0), provider.seedFor(5));
}

TEST(SeedProvider, PerSessionSeedIsReproducibleAndDistinct)
{
    const auto first = SeedProvider::perSession(7);
    const auto second = SeedProvider::perSession(7);

    EXPECT_EQ(first.seedFor(3), second.seedFor(3));
    EXPECT_NE(first.seedFor(0), first.seedFor(1));
}
