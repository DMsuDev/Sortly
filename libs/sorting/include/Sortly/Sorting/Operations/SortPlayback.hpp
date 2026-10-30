// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#pragma once

#include "Sortly/Sorting/Operations/SortOperation.hpp"

#include <cstddef>
#include <span>
#include <vector>

namespace Sortly {

/// @brief Replays recorded sorting operations over its own copy of the data.
///
/// The playback owns the buffer it mutates (see getVisualData()), so it never aliases
/// the session's DataSet and can be moved freely. Operations can be replayed in real
/// time (update), stepped manually in both directions, or applied instantly.
///
/// State invariant: Stopped means "at the first operation, buffer untouched".
/// Any advance leaves Stopped; going back to the first operation returns to it.
class SortPlayback
{
public:
    /// @brief Default delay (in milliseconds) applied between each played-back operation.
    static constexpr float DEFAULT_STEP_DELAY_MS = 10.0f;

    /// @brief Lifecycle states a playback can be in.
    enum class PlaybackState
    {
        Stopped,  // At the first operation, buffer in its initial state.
        Playing,  // Advancing automatically through update().
        Paused,   // Somewhere in the middle; resumable and steppable in both directions.
        Finished  // All operations applied.
    };

    /// @brief Copies @p initialData as the starting state and loads the operations.
    /// Leaves the playback Stopped at the first operation.
    ///
    /// The operations are validated here, once, against @p initialData: every index they
    /// touch (Swap, Overwrite, and Pivot when it marks an element) must be in range.
    /// Replaying then needs no per-step checks.
    /// @return False (and changes nothing) if @p ops is empty or contains an out-of-range index.
    [[nodiscard]] bool load(std::span<const int> initialData, std::span<const SortOperation> ops);

    /// @brief Drops the loaded operations and data. The playback becomes unloaded and Stopped.
    void clear();

    /// @brief Advances playback based on elapsed time, applying operations as needed.
    /// @param deltaTime Time elapsed since the last update, in seconds.
    void update(float deltaTime);

    /// @brief Starts or resumes playback from the current operation.
    /// If playback had finished, it restarts from the initial state.
    /// @return False if there is nothing to play (no operations loaded).
    bool play();

    /// @brief Pauses playback, preserving the current position.
    void pause();

    /// @brief Resumes playback from a paused state.
    void resume();

    /// @brief Stops playback and restores the buffer and cursor to the initial state.
    void stop();

    /// @brief Applies the next operation.
    /// @return False if there are no more operations to apply.
    bool stepForward();

    /// @brief Reverts the previous operation. Pauses automatic playback.
    /// @return False if already at the first operation.
    bool stepBackward();

    /// @brief Enables or disables the delay between steps. While disabled, update() applies
    /// every remaining operation at once. The configured delay is preserved.
    void enableDelay(bool value) { m_DelayEnabled = value; }
    [[nodiscard]] bool isDelayEnabled() const { return m_DelayEnabled; }

    /// @brief Sets the delay (in milliseconds) applied between each played-back operation.
    void setDelayMs(float ms) { m_DelayMs = ms; }
    [[nodiscard]] float getDelayMs() const { return m_DelayMs; }

    /// @brief True if operations are loaded.
    [[nodiscard]] bool isLoaded() const { return !m_Operations.empty(); }

    /// @brief True once no remaining operation changes this slot: it already holds its final value.
    /// Exact in both play directions, and needs no help from the algorithms.
    [[nodiscard]] bool isSettled(std::size_t slot) const
    {
        return slot < m_SettledAt.size() && m_CurrentIndex >= m_SettledAt[slot];
    }

    /// @brief Current state of the data, reflecting every operation applied so far.
    [[nodiscard]] const std::vector<int>& getVisualData() const { return m_Visual; }

    /// @brief Data as it was when load() was called.
    [[nodiscard]] const std::vector<int>& getInitialData() const { return m_Initial; }

    [[nodiscard]] const std::vector<SortOperation>& getOperations() const { return m_Operations; }

    /// @brief Operation that the next stepForward() would apply, or nullptr if there is none.
    [[nodiscard]] const SortOperation* getCurrentOperation() const;

    [[nodiscard]] std::size_t getCurrentOperationIndex() const { return m_CurrentIndex; }
    [[nodiscard]] std::size_t getTotalOperations()       const { return m_Operations.size(); }

    /// @brief Index of the pivot element after the operations applied so far, or -1 if none.
    /// The pivot is an element, not a slot: the index follows it through every Swap, in both
    /// directions. Always a valid index into getVisualData() when it is not -1.
    [[nodiscard]] int getPivotIndex() const { return m_Pivot; }

    [[nodiscard]] PlaybackState getState() const { return m_State; }

    /// @brief Progress in [0, 1]; 0 if nothing is loaded.
    [[nodiscard]] float getProgress() const;

    [[nodiscard]] bool isPlaying()  const { return m_State == PlaybackState::Playing; }
    [[nodiscard]] bool isPaused()   const { return m_State == PlaybackState::Paused; }
    [[nodiscard]] bool isStopped()  const { return m_State == PlaybackState::Stopped; }
    [[nodiscard]] bool isFinished() const { return m_State == PlaybackState::Finished; }

private:
    /// Restores buffer, pivot and cursor to the initial state and goes to Stopped.
    void reset();

    /// Fills m_SettledAt with the operation index at which each slot becomes settled.
    void buildSettledTimeline();

    /// Applies (or, when @p reverse, undoes) one operation on m_Visual and the pivot.
    /// Swap is its own inverse; Overwrite restores valueB; Pivot restores the previous pivot
    /// from m_PivotHistory; Compare/Sorted are no-ops.
    /// Indices were validated by load(), so they are only asserted here.
    void execute(const SortOperation& op, bool reverse);

    std::vector<SortOperation> m_Operations;
    std::vector<int> m_Initial;
    std::vector<int> m_Visual;

    int m_Pivot{-1};                         // Pivot element index, -1 when none.
    std::vector<std::size_t> m_SettledAt;    // Per slot: operations that must be applied before it stops changing.
    std::vector<int>         m_PivotHistory; // Pivot before each applied Pivot operation (LIFO, for undo).

    std::size_t m_CurrentIndex{0};
    float m_ElapsedTime{0.0f};

    float m_DelayMs{DEFAULT_STEP_DELAY_MS};
    bool  m_DelayEnabled{true};

    PlaybackState m_State{PlaybackState::Stopped};
};

} // namespace Sortly
