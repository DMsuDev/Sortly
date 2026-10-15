// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include <Sortly/Sorting/Operations/SortPlayback.hpp>
#include <Sortly/Sorting/Operations/SortRecorder.hpp>

#include <gtest/gtest.h>

#include <vector>

using namespace Sortly;

namespace {

// Three operations over {3, 1, 2}:  swap(0,1) -> {1,3,2},  swap(1,2) -> {1,2,3},  sorted.
class PlaybackTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        std::vector<int> work = m_Initial;
        SortStatistics statistics;
        SortRecorder recorder(statistics);
        m_Operations = {
            recorder.swap(work, 0, 1),
            recorder.swap(work, 1, 2),
            recorder.sorted()
        };

        ASSERT_TRUE(m_Playback.load(m_Initial, m_Operations));
    }

    const std::vector<int> m_Initial{ 3, 1, 2 };
    std::vector<SortOperation> m_Operations;
    SortPlayback m_Playback;
};

} // namespace

TEST_F(PlaybackTest, LoadLeavesThePlaybackStoppedOverTheInitialData)
{
    EXPECT_TRUE(m_Playback.isLoaded());
    EXPECT_TRUE(m_Playback.isStopped());
    EXPECT_EQ(m_Playback.getVisualData(), m_Initial);
    EXPECT_EQ(m_Playback.getTotalOperations(), 3u);
}

TEST_F(PlaybackTest, UpdateAppliesOneStepPerDelay)
{
    m_Playback.setDelayMs(10.0f);
    m_Playback.play();
    m_Playback.update(0.025f);

    EXPECT_EQ(m_Playback.getCurrentOperationIndex(), 2u);
}

TEST_F(PlaybackTest, DisablingTheDelayAppliesEverythingInOneUpdate)
{
    m_Playback.enableDelay(false);
    m_Playback.play();
    m_Playback.update(0.0f);

    EXPECT_TRUE(m_Playback.isFinished());
    EXPECT_EQ(m_Playback.getVisualData(), (std::vector<int>{ 1, 2, 3 }));
    EXPECT_FLOAT_EQ(m_Playback.getDelayMs(), SortPlayback::DEFAULT_STEP_DELAY_MS); // preserved
}

TEST_F(PlaybackTest, PlayAfterFinishedRestoresInitialState)
{
    m_Playback.play();
    m_Playback.update(1.0f);

    ASSERT_TRUE(m_Playback.isFinished());
    EXPECT_EQ(m_Playback.getVisualData(), (std::vector<int>{ 1, 2, 3 }));

    m_Playback.play();

    EXPECT_TRUE(m_Playback.isPlaying());
    EXPECT_EQ(m_Playback.getCurrentOperationIndex(), 0u);
    EXPECT_EQ(m_Playback.getVisualData(), m_Initial);
}

// Regression: stop() used to reset the cursor but leave the buffer half-sorted, so the
// next play() applied the operations over already-modified data.
TEST_F(PlaybackTest, StopRestoresTheBufferAndTheCursor)
{
    m_Playback.setDelayMs(10.0f);
    m_Playback.play();
    m_Playback.update(0.015f);
    ASSERT_EQ(m_Playback.getVisualData(), (std::vector<int>{ 1, 3, 2 }));

    m_Playback.stop();

    EXPECT_TRUE(m_Playback.isStopped());
    EXPECT_EQ(m_Playback.getCurrentOperationIndex(), 0u);
    EXPECT_EQ(m_Playback.getVisualData(), m_Initial);

    m_Playback.play();
    m_Playback.update(0.015f);
    EXPECT_EQ(m_Playback.getVisualData(), (std::vector<int>{ 1, 3, 2 })); // not corrupted
}

TEST_F(PlaybackTest, StepBackwardIsRejectedWhileStopped)
{
    EXPECT_FALSE(m_Playback.stepBackward());
}

// Regression: a manual stepForward() used to leave the state Stopped, which made the
// following stepBackward() fail.
TEST_F(PlaybackTest, ManualStepsWorkInBothDirectionsWithoutPlaying)
{
    ASSERT_TRUE(m_Playback.stepForward());
    EXPECT_TRUE(m_Playback.isPaused());
    EXPECT_EQ(m_Playback.getVisualData(), (std::vector<int>{ 1, 3, 2 }));

    ASSERT_TRUE(m_Playback.stepBackward());
    EXPECT_TRUE(m_Playback.isStopped());
    EXPECT_EQ(m_Playback.getVisualData(), m_Initial);
}

TEST_F(PlaybackTest, SteppingBackwardPausesAutomaticPlayback)
{
    m_Playback.setDelayMs(10.0f);
    m_Playback.play();
    m_Playback.update(0.025f);

    ASSERT_TRUE(m_Playback.stepBackward());

    EXPECT_TRUE(m_Playback.isPaused());
    EXPECT_EQ(m_Playback.getCurrentOperationIndex(), 1u);
}

TEST_F(PlaybackTest, ClearUnloadsEverything)
{
    m_Playback.clear();

    EXPECT_FALSE(m_Playback.isLoaded());
    EXPECT_TRUE(m_Playback.isStopped());
    EXPECT_TRUE(m_Playback.getVisualData().empty());
    EXPECT_FLOAT_EQ(m_Playback.getProgress(), 0.0f);
}

TEST_F(PlaybackTest, PlayReportsWhetherThereWasAnythingToPlay)
{
    EXPECT_TRUE(m_Playback.play());

    SortPlayback empty;
    EXPECT_FALSE(empty.play());
    EXPECT_TRUE(empty.isStopped());
}

// Indices are validated once in load(), so a corrupt list is refused up front instead of
// producing an error per step during playback.
TEST(SortPlayback, LoadRejectsOperationsWithOutOfRangeIndices)
{
    const std::vector<int> initial{ 1, 2, 3 };
    SortStatistics statistics;
    SortRecorder recorder(statistics);

    std::vector<int> work = initial;
    std::vector<SortOperation> operations{ recorder.swap(work, 0, 1) };
    operations.push_back(SortOperation{ .type = SortOperation::Type::Swap, .indexA = 0, .indexB = 7 });

    SortPlayback playback;
    EXPECT_FALSE(playback.load(initial, operations));
    EXPECT_FALSE(playback.isLoaded());

    operations.back() = SortOperation{ .type = SortOperation::Type::Overwrite, .indexA = -1, .valueA = 5 };
    EXPECT_FALSE(playback.load(initial, operations));
}

TEST(SortPlayback, ARejectedLoadKeepsThePreviousOne)
{
    const std::vector<int> initial{ 1, 2, 3 };
    SortStatistics statistics;
    SortRecorder recorder(statistics);
    std::vector<int> work = initial;
    const std::vector<SortOperation> good{ recorder.swap(work, 0, 1) };
    const std::vector<SortOperation> bad{ SortOperation{ .type = SortOperation::Type::Swap, .indexA = 0, .indexB = 9 } };

    SortPlayback playback;
    ASSERT_TRUE(playback.load(initial, good));
    EXPECT_FALSE(playback.load(initial, bad));

    EXPECT_TRUE(playback.isLoaded());
    EXPECT_EQ(playback.getTotalOperations(), 1u);
}

TEST(SortPlayback, LoadAcceptsOperationsThatNeverTouchTheData)
{
    // Compare/Pivot/Sorted carry indices for display only; they are not validated against the data.
    const std::vector<SortOperation> operations{
        SortOperation{ .type = SortOperation::Type::Sorted }
    };

    SortPlayback playback;
    EXPECT_TRUE(playback.load(std::vector<int>{}, operations));
}

TEST(SortPlayback, LoadRejectsAnEmptyOperationList)
{
    SortPlayback playback;

    EXPECT_FALSE(playback.load(std::vector<int>{ 1, 2 }, std::vector<SortOperation>{}));
    EXPECT_FALSE(playback.isLoaded());
}

TEST(SortPlaybackUndo, OverwriteRestoresPreviousValue)
{
    const std::vector<int> initial{ 4, 5, 6 };
    std::vector<int> work = initial;
    SortStatistics statistics;
    SortRecorder recorder(statistics);
    const std::vector<SortOperation> operations{ recorder.overwrite(work, 1, 99) };

    SortPlayback playback;
    ASSERT_TRUE(playback.load(initial, operations));

    ASSERT_TRUE(playback.stepForward());
    EXPECT_EQ(playback.getVisualData(), (std::vector<int>{ 4, 99, 6 }));

    ASSERT_TRUE(playback.stepBackward());
    EXPECT_EQ(playback.getVisualData(), (std::vector<int>{ 4, 5, 6 }));
}
