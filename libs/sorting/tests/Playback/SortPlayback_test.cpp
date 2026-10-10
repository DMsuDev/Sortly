// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include <Sortly/Sorting/Operations/detail/SortRecorder.hpp>
#include <Sortly/Sorting/Playback/SortPlayback.hpp>

#include <gtest/gtest.h>

#include <vector>

using namespace Sortly;

namespace {

class PlaybackTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        SortStatistics statistics;
        SortRecorder recorder(statistics);
        m_Operations = {
            recorder.swap(0, 1, 3, 1),
            recorder.swap(1, 2, 3, 2),
            recorder.sorted()
        };

        m_Playback.bind(m_Buffer);
        ASSERT_TRUE(m_Playback.loadOperations(m_Operations));
    }

    std::vector<int> m_Buffer{ 3, 1, 2 };
    std::vector<SortOperation> m_Operations;
    SortPlayback m_Playback;
};

} // namespace

TEST_F(PlaybackTest, UpdateAppliesOneStepPerDelay)
{
    m_Playback.setDelayMs(10.0f);
    m_Playback.play();
    m_Playback.update(0.025f);

    EXPECT_EQ(m_Playback.getCurrentOperationIndex(), 2u);
}

TEST_F(PlaybackTest, PlayAfterFinishedRestoresInitialState)
{
    m_Playback.play();
    m_Playback.update(1.0f);

    ASSERT_TRUE(m_Playback.isFinished());
    EXPECT_EQ(m_Buffer, (std::vector<int>{ 1, 2, 3 }));

    m_Playback.play();

    EXPECT_TRUE(m_Playback.isPlaying());
    EXPECT_EQ(m_Playback.getCurrentOperationIndex(), 0u);
    EXPECT_EQ(m_Buffer, (std::vector<int>{ 3, 1, 2 }));
}

TEST_F(PlaybackTest, StepBackwardIsRejectedWhileStopped)
{
    EXPECT_FALSE(m_Playback.stepBackward());
}

TEST(SortPlaybackUndo, OverwriteRestoresPreviousValue)
{
    std::vector<int> buffer{ 4, 5, 6 };
    std::vector<int> work = buffer;
    SortStatistics statistics;
    SortRecorder recorder(statistics);
    const std::vector<SortOperation> operations{ recorder.overwrite(work, 1, 99) };

    SortPlayback playback;
    playback.bind(buffer);
    ASSERT_TRUE(playback.loadOperations(operations));

    ASSERT_TRUE(playback.stepForward());
    EXPECT_EQ(buffer, (std::vector<int>{ 4, 99, 6 }));

    ASSERT_TRUE(playback.stepBackward());
    EXPECT_EQ(buffer, (std::vector<int>{ 4, 5, 6 }));
}
