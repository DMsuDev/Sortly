// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#pragma once

#include "Sortly/Sorting/Operations/SortOperation.hpp"

#include <span>
#include <vector>

namespace Sortly {

/// @brief Controls the playback of recorded sorting operations over a data buffer.
///
/// Applies recorded operations to the current dataset, allowing the sequence
/// to be replayed step by step or executed instantly. Playback behavior is
/// managed independently from the sorting algorithm that generated the
/// operations.
class SortPlayback
{
public:
    /// @brief Default delay (in milliseconds) applied between each played-back operation.
    static constexpr float DEFAULT_STEP_DELAY_MS = 10.0f;

    /// @brief Lifecycle states a playback session can be in.
    enum class PlaybackState
    {
        Stopped,  // Playback is stopped and can be started
        Playing,  // Playback is actively playing through the operations
        Paused,   // Playback is paused and can be resumed
        Finished  // Playback has reached the end of the operations
    };

    /// @brief Binds the visual buffer that operations will be applied to.
    /// @param visualBuffer Buffer to bind; must outlive this SortPlayback instance
    /// (or be rebound) as long as playback is in use.
    void bind(std::vector<int>& visualBuffer);

    /// @brief Stores a snapshot of the current buffer and loads the operation list.
    /// @param ops Operations to load; must not be empty.
    /// @return False if @p ops is empty or no buffer is bound (call bind() first).
    [[nodiscard]] bool loadOperations(std::span<const SortOperation> ops);

    /// @brief Advances playback based on elapsed time, applying operations as needed.
    /// @param deltaTime Time elapsed since the last update, in seconds.
    void update(float deltaTime);

    /// @brief Starts or resumes playback from the current index.
    /// If playback had finished, the buffer is restored to its initial state first.
    void play();

    /// @brief Pauses playback, preserving the current index.
    void pause();

    /// @brief Resumes playback from a paused state.
    void resume();

    /// @brief Stops playback and resets the current index to the beginning of the operation list.
    void stop();

    /// @brief Restores the buffer to the pre-playback snapshot and resets the cursor.
    void restoreInitialState();

    /// @brief Applies the next operation, advancing the current index.
    /// @return False if playback is stopped or there are no more operations to apply.
    bool stepForward();

    /// @brief Reverts the previous operation, moving the current index back.
    /// @return False if playback is stopped or there are no more operations to apply.
    bool stepBackward();

    /// @brief Enables or disables the delay between steps.
    /// Disabling sets the delay to 0 while preserving the previous value for restoration.
    /// @param value True to restore the previous delay, false to disable it.
    void enableDelay(bool value);

    /// @brief Sets the delay (in milliseconds) applied between each played-back operation.
    void setDelayMs(float ms) { m_DelayMs = ms; m_OldDelayMs = ms; }

    /// @brief Gets the current delay (in milliseconds) between played-back operations.
    [[nodiscard]] float getDelayMs() const { return m_DelayMs; }

    /// @brief Gets the current state of the visual buffer, reflecting all applied operations so far.
    /// @return The bound buffer, or an empty vector if none is bound.
    [[nodiscard]] const std::vector<int>& getVisualData()  const
    {
        static const std::vector<int> s_Empty;
        return m_VisualBuffer ? *m_VisualBuffer : s_Empty;
    }

    /// @brief Gets the buffer snapshot captured by the last loadOperations() call.
    [[nodiscard]] const std::vector<int>& getInitialData() const { return m_InitialData; }

    /// @brief Gets the full list of operations currently loaded for playback.
    [[nodiscard]] const std::vector<SortOperation>& getOperations() const { return m_Operations; }

    /// @brief Gets the operation at the current index.
    /// @return Pointer to the current operation, or nullptr if there are no operations loaded
    /// or the index is out of range.
    [[nodiscard]] const SortOperation* getCurrentOperation() const;

    /// @brief Gets the index of the operation that would be applied by the next stepForward().
    [[nodiscard]] size_t getCurrentOperationIndex()          const { return m_CurrentIndex; }

    /// @brief Gets the total number of operations currently loaded.
    [[nodiscard]] size_t getTotalOperations()                const { return m_Operations.size(); }

    /// @brief Gets the current playback lifecycle state.
    [[nodiscard]] PlaybackState getState() const { return m_State; }

    /// @brief Gets playback progress as a ratio in [0, 1], based on the current
    /// operation index relative to the total loaded operations.
    /// @return 0.0f if no operations are loaded.
    [[nodiscard]] float getProgress() const;

    [[nodiscard]] bool isPlaying()   const { return m_State == PlaybackState::Playing; }
    [[nodiscard]] bool isPaused()    const { return m_State == PlaybackState::Paused; }
    [[nodiscard]] bool isStopped()   const { return m_State == PlaybackState::Stopped; }
    [[nodiscard]] bool isFinished()  const { return m_State == PlaybackState::Finished; }

private:
    void setState(PlaybackState newState);

    /// @brief Applies a single operation to the bound visual buffer.
    /// Logs and no-ops if no buffer is bound or if the operation's indices are out of range.
    void applyOperation(const SortOperation& op);

    /// @brief Inverts a single operation on the bound visual buffer (the counterpart to
    /// applyOperation()). Swap is its own inverse; Overwrite restores the previous value
    /// captured by SortRecorder::overwrite(); Compare/Pivot/Sorted are no-ops either way.
    void undoOperation(const SortOperation& op);

    /// @brief Resets the current index to the beginning of the operation list.
    void resetIndex();

    std::vector<SortOperation> m_Operations;

    std::vector<int>* m_VisualBuffer{nullptr};
    std::vector<int>  m_InitialData;

    size_t m_CurrentIndex{0};

    float m_ElapsedTime{0.0f};

    float m_DelayMs   {DEFAULT_STEP_DELAY_MS};
    float m_OldDelayMs{DEFAULT_STEP_DELAY_MS};
    bool m_EnableDelay{true};

    PlaybackState m_State{PlaybackState::Stopped};
};

} // namespace Sortly
