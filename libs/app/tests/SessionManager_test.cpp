// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include <Sortly/App/SessionManager.hpp>

#include <gtest/gtest.h>

using namespace Sortly;

TEST(SessionManager, CreatesIndependentSessions)
{
    SessionManager manager;
    manager.setSeedProvider(SeedProvider::globalSeed(1));

    const DataSetConfig config{ 16, 2, 4096, 100 };
    auto* first = manager.createSession(SortAlgorithmType::BubbleSort, config);
    auto* second = manager.createSession(SortAlgorithmType::CocktailSort, config);

    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);
    EXPECT_NE(first, second);
    EXPECT_EQ(manager.sessions().size(), 2u);
}
