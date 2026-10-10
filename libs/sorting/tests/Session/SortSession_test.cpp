// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include <Sortly/Sorting/Algorithms/SortAlgorithmFactory.hpp>
#include <Sortly/Sorting/Session/SortSession.hpp>

#include <gtest/gtest.h>

#include <chrono>
#include <random>

using namespace Sortly;
using namespace std::chrono_literals;

namespace {

SortSession makeSession(int count, int maxCount = 4096)
{
    SortSession session;
    session.setDataSetConfig(DataSetConfig{ count, 2, maxCount, 500 });
    session.setAlgorithm(SortAlgorithmFactory::create(SortAlgorithmType::BubbleSort));

    std::mt19937 rng{ 42 };
    EXPECT_TRUE(session.buildDataSet(rng));
    return session;
}

} // namespace

TEST(SortSession, PreparePlaybackSucceedsForAValidSession)
{
    auto session = makeSession(64);

    EXPECT_TRUE(session.preparePlayback());
}

TEST(SortSession, BlockingComputeRejectsOversizedDataSets)
{
    const int tooBig = static_cast<int>(DataSetConfig::kQuadraticSafeLimit) + 1;
    auto session = makeSession(tooBig, tooBig * 2);

    EXPECT_EQ(session.computeOperations(), SortSession::ComputeStatus::Failed);
}

TEST(SortSession, BudgetedComputeEventuallyCompletes)
{
    auto session = makeSession(256);

    auto status = SortSession::ComputeStatus::InProgress;
    for (int attempt = 0; status == SortSession::ComputeStatus::InProgress && attempt < 100'000; ++attempt)
        status = session.computeOperations(50us);

    EXPECT_EQ(status, SortSession::ComputeStatus::Complete);
}
