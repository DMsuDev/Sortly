// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include <Sortly/Sorting/Algorithms/SortAlgorithmFactory.hpp>
#include <Sortly/Sorting/SortSession.hpp>

#include <gtest/gtest.h>

#include <algorithm>
#include <chrono>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

using namespace Sortly;
using namespace std::chrono_literals;

namespace {

SortSession makeSession(int count, int maxCount = 4096,
                        SortAlgorithmType type = SortAlgorithmType::BubbleSort)
{
    SortSession session;
    session.setDataSetConfig(DataSetConfig{ count, 2, maxCount, 500 });
    session.setAlgorithm(SortAlgorithmFactory::create(type));

    std::mt19937 rng{ 42 };
    EXPECT_TRUE(session.buildDataSet(rng));
    return session;
}

std::vector<int> valuesOf(const SortSession& session)
{
    const auto values = session.getDataSet().values();
    return { values.begin(), values.end() };
}

// A custom algorithm that throws, to exercise the session's error handling.
class ThrowingAlgorithm final : public SortAlgorithm
{
public:
    enum class When { OnLoad, OnStep };
    explicit ThrowingAlgorithm(When when) : m_When(when) {}

    void load(std::span<const int>) override
    {
        if (m_When == When::OnLoad)
            throw std::runtime_error("load failed");
    }

    std::optional<SortOperation> step(SortRecorder&) override
    {
        throw std::runtime_error("step failed");
    }

    std::span<const int> getData() const override { return {}; }
    std::string_view getName() const override { return "Throwing"; }
    std::string_view getComplexity() const override { return "N/A"; }
    std::string_view getDescription() const override { return "Always throws."; }

private:
    When m_When;
};

void playToTheEnd(SortSession& session)
{
    auto& playback = session.getPlayback();
    playback.enableDelay(false);
    playback.play();
    playback.update(0.0f);
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

TEST(SortSession, BudgetedComputeProducesTheSameOperationsAsBlocking)
{
    auto blocking = makeSession(128);
    ASSERT_EQ(blocking.computeOperations(), SortSession::ComputeStatus::Complete);

    auto budgeted = makeSession(128);
    auto status = SortSession::ComputeStatus::InProgress;
    while (status == SortSession::ComputeStatus::InProgress)
        status = budgeted.computeOperations(1us);
    ASSERT_EQ(status, SortSession::ComputeStatus::Complete);

    ASSERT_EQ(blocking.getOperations().size(), budgeted.getOperations().size());
    EXPECT_EQ(blocking.getStatistics().swaps, budgeted.getStatistics().swaps);
}

TEST(SortSession, ComputeReportsWhyItFailedWithoutAnAlgorithm)
{
    SortSession session;

    EXPECT_EQ(session.computeOperations(), SortSession::ComputeStatus::Failed);
    EXPECT_EQ(session.getLastComputeError(), SortSession::ComputeError::NoAlgorithm);
}

TEST(SortSession, ComputeReportsWhyItFailedWithoutData)
{
    SortSession session;
    session.setAlgorithm(SortAlgorithmFactory::create(SortAlgorithmType::BubbleSort));

    EXPECT_EQ(session.computeOperations(), SortSession::ComputeStatus::Failed);
    EXPECT_EQ(session.getLastComputeError(), SortSession::ComputeError::NoData);
    EXPECT_FALSE(session.preparePlayback());
}

TEST(SortSession, ComputeReportsWhyItFailedForOversizedBlockingRuns)
{
    const int tooBig = static_cast<int>(DataSetConfig::kQuadraticSafeLimit) + 1;
    auto session = makeSession(tooBig, tooBig * 2);

    EXPECT_EQ(session.computeOperations(), SortSession::ComputeStatus::Failed);
    EXPECT_EQ(session.getLastComputeError(), SortSession::ComputeError::TooLargeForBlocking);
}

TEST(SortSession, ComputeReportsAnAlgorithmThatThrowsWhileStepping)
{
    auto session = makeSession(16);
    session.setAlgorithm(std::make_unique<ThrowingAlgorithm>(ThrowingAlgorithm::When::OnStep));

    EXPECT_EQ(session.computeOperations(), SortSession::ComputeStatus::Failed);
    EXPECT_EQ(session.getLastComputeError(), SortSession::ComputeError::AlgorithmThrew);
    EXPECT_TRUE(session.getOperations().empty()); // partial results are discarded
    EXPECT_FALSE(session.isComputing());
}

TEST(SortSession, ComputeReportsAnAlgorithmThatThrowsWhileLoading)
{
    auto session = makeSession(16);
    session.setAlgorithm(std::make_unique<ThrowingAlgorithm>(ThrowingAlgorithm::When::OnLoad));

    EXPECT_EQ(session.computeOperations(), SortSession::ComputeStatus::Failed);
    EXPECT_EQ(session.getLastComputeError(), SortSession::ComputeError::AlgorithmThrew);
}

TEST(SortSession, TheErrorIsClearedByASuccessfulRun)
{
    auto session = makeSession(32);
    session.setAlgorithm(std::make_unique<ThrowingAlgorithm>(ThrowingAlgorithm::When::OnStep));
    ASSERT_EQ(session.computeOperations(), SortSession::ComputeStatus::Failed);

    session.setAlgorithm(SortAlgorithmFactory::create(SortAlgorithmType::BubbleSort));
    EXPECT_EQ(session.getLastComputeError(), SortSession::ComputeError::None); // cleared with the algorithm change

    EXPECT_EQ(session.computeOperations(), SortSession::ComputeStatus::Complete);
    EXPECT_EQ(session.getLastComputeError(), SortSession::ComputeError::None);
}

TEST(SortSession, RefusingABlockingResumeDoesNotLeaveAStaleErrorBehind)
{
    const int big = static_cast<int>(DataSetConfig::kQuadraticSafeLimit) + 100;
    auto session = makeSession(big, big * 2);

    // Start cooperatively, then get refused when trying to force-complete a huge run.
    ASSERT_EQ(session.computeOperations(1us), SortSession::ComputeStatus::InProgress);
    ASSERT_EQ(session.computeOperations(), SortSession::ComputeStatus::Failed);
    ASSERT_EQ(session.getLastComputeError(), SortSession::ComputeError::TooLargeForBlocking);

    // The cooperative run is still resumable, and resuming it is not a failure.
    EXPECT_EQ(session.computeOperations(1us), SortSession::ComputeStatus::InProgress);
    EXPECT_EQ(session.getLastComputeError(), SortSession::ComputeError::None);
}

TEST(SortSession, ComputeErrorsHaveNames)
{
    EXPECT_EQ(toString(SortSession::ComputeError::None), "None");
    EXPECT_EQ(toString(SortSession::ComputeError::NoAlgorithm), "NoAlgorithm");
    EXPECT_EQ(toString(SortSession::ComputeError::NoData), "NoData");
    EXPECT_EQ(toString(SortSession::ComputeError::TooLargeForBlocking), "TooLargeForBlocking");
    EXPECT_EQ(toString(SortSession::ComputeError::AlgorithmThrew), "AlgorithmThrew");
}

TEST(SortSession, SettingTheSameConfigAgainChangesNothing)
{
    auto session = makeSession(64);
    ASSERT_TRUE(session.preparePlayback());

    session.setDataSetConfig(session.getDataSetConfig()); // what a per-frame UI slider does

    EXPECT_TRUE(session.getPlayback().isLoaded()); // data untouched, nothing was invalidated
    EXPECT_EQ(session.getDataSetConfig(), (DataSetConfig{ 64, 2, 4096, 500 }));
}

// Regression: the playback bound to the session's own buffer through a raw pointer,
// so a naive move left it dangling.
TEST(SortSession, MovedSessionKeepsPlayingBack)
{
    auto original = makeSession(64);
    ASSERT_TRUE(original.preparePlayback());

    SortSession moved = std::move(original);
    playToTheEnd(moved);

    EXPECT_TRUE(moved.getPlayback().isFinished());
    EXPECT_TRUE(std::ranges::is_sorted(moved.getPlayback().getVisualData()));
}

TEST(SortSession, MoveAssignedSessionKeepsPlayingBack)
{
    auto source = makeSession(64);
    ASSERT_TRUE(source.preparePlayback());

    SortSession target;
    target = std::move(source);
    playToTheEnd(target);

    EXPECT_TRUE(std::ranges::is_sorted(target.getPlayback().getVisualData()));
}

TEST(SortSession, PlaybackNeverModifiesTheDataSet)
{
    auto session = makeSession(64);
    const auto before = valuesOf(session);
    ASSERT_TRUE(session.preparePlayback());

    playToTheEnd(session);

    EXPECT_EQ(valuesOf(session), before);
    EXPECT_TRUE(std::ranges::is_sorted(session.getPlayback().getVisualData()));
}

// Regression: after a finished playback the DataSet used to be sorted, so computing again
// with another algorithm only produced comparisons.
TEST(SortSession, RecomputingAfterPlaybackStillSeesTheShuffledData)
{
    auto session = makeSession(64);
    ASSERT_TRUE(session.preparePlayback());
    playToTheEnd(session);

    session.setAlgorithm(SortAlgorithmFactory::create(SortAlgorithmType::CocktailSort));

    ASSERT_TRUE(session.preparePlayback());
    EXPECT_GT(session.getStatistics().swaps, 0u);
}

TEST(SortSession, ChangingTheAlgorithmClearsOperationsAndPlayback)
{
    auto session = makeSession(64);
    ASSERT_TRUE(session.preparePlayback());

    session.setAlgorithm(SortAlgorithmFactory::create(SortAlgorithmType::CocktailSort));

    EXPECT_TRUE(session.getOperations().empty());
    EXPECT_FALSE(session.getPlayback().isLoaded());
    EXPECT_EQ(session.getStatistics().operationCount, 0u);
}

TEST(SortSession, ShufflingClearsOperationsAndPlayback)
{
    auto session = makeSession(64);
    ASSERT_TRUE(session.preparePlayback());

    std::mt19937 rng{ 1 };
    session.shuffleData(rng);

    EXPECT_TRUE(session.getOperations().empty());
    EXPECT_FALSE(session.getPlayback().isLoaded());
}

TEST(SortSession, LoadPlaybackNeedsComputedOperations)
{
    auto session = makeSession(64);

    EXPECT_FALSE(session.loadPlayback());
}

TEST(SortSession, VisualDataFallsBackToTheDataSetUntilPlaybackIsLoaded)
{
    auto session = makeSession(64);
    EXPECT_TRUE(std::ranges::equal(session.getVisualData(), session.getDataSet().values()));

    ASSERT_TRUE(session.preparePlayback());
    playToTheEnd(session);

    EXPECT_TRUE(std::ranges::is_sorted(session.getVisualData()));
}

TEST(SortSession, RebuildingWithAnEmptyConfigKeepsTheCurrentDataSet)
{
    auto session = makeSession(64);
    const auto before = valuesOf(session);

    session.setDataSetConfig(DataSetConfig{ 0, 0, 10, 100 });
    std::mt19937 rng{ 3 };

    EXPECT_FALSE(session.buildDataSet(rng));
    EXPECT_EQ(valuesOf(session), before);
}

TEST(SortSession, SessionIdsAreUnique)
{
    SortSession first;
    SortSession second;

    EXPECT_NE(first.getSessionId(), second.getSessionId());
}
