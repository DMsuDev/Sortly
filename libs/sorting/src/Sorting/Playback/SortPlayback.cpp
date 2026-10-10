// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "Sortly/Sorting/Playback/SortPlayback.hpp"

#include <vigil/vigil.h>

namespace Sortly {

namespace {
    std::string_view toString(SortOperation::Type type)
    {
        switch (type)
        {
            case SortOperation::Type::Compare:   return "Compare";
            case SortOperation::Type::Swap:      return "Swap";
            case SortOperation::Type::Overwrite: return "Overwrite";
            case SortOperation::Type::Pivot:     return "Pivot";
            case SortOperation::Type::Sorted:    return "Sorted";
        }
        return "Unknown";
    }
} // namespace

// ================================================
// INTERNAL FUNCTIONS
// ================================================

void SortPlayback::applyOperation(const SortOperation& op)
{
    if (!m_VisualBuffer)
    {
        VIGIL_LOG_ERROR("SortPlayback: No visual buffer bound - cannot apply operation");
        return;
    }

    auto& buf = *m_VisualBuffer;

    const auto isValidIndex = [&buf](int index) {
        return index >= 0 && static_cast<size_t>(index) < buf.size();
    };

    switch (op.type)
    {
        case SortOperation::Type::Swap:
        {
            if (!isValidIndex(op.indexA) || !isValidIndex(op.indexB))
            {
                VIGIL_LOG_ERROR("SortPlayback: Swap indices out of range (indexA={}, indexB={}, size={})",
                          op.indexA, op.indexB, buf.size());
                break;
            }
            std::swap(buf[op.indexA], buf[op.indexB]);
            break;
        }

        case SortOperation::Type::Overwrite:
        {
            if (!isValidIndex(op.indexA))
            {
                VIGIL_LOG_ERROR("SortPlayback: Overwrite index out of range (indexA={}, size={})",
                          op.indexA, buf.size());
                break;
            }
            buf[op.indexA] = op.valueA;
            break;
        }

        case SortOperation::Type::Compare:
        case SortOperation::Type::Pivot:
        case SortOperation::Type::Sorted:
            // No action needed for these types; for now.
            break;

        default:
            VIGIL_LOG_WARN("SortPlayback: Unknown operation type encountered: {}", toString(op.type));
            break;
    }
}

void SortPlayback::undoOperation(const SortOperation& op)
{
    if (!m_VisualBuffer)
    {
        VIGIL_LOG_ERROR("SortPlayback: No visual buffer bound - cannot undo operation");
        return;
    }

    auto& buf = *m_VisualBuffer;

    const auto isValidIndex = [&buf](int index) {
        return index >= 0 && static_cast<size_t>(index) < buf.size();
    };

    switch (op.type)
    {
        case SortOperation::Type::Swap:
        {
            if (!isValidIndex(op.indexA) || !isValidIndex(op.indexB))
            {
                VIGIL_LOG_ERROR("SortPlayback: Swap indices out of range while undoing (indexA={}, indexB={}, size={})",
                          op.indexA, op.indexB, buf.size());
                break;
            }
            // A swap is its own inverse.
            std::swap(buf[op.indexA], buf[op.indexB]);
            break;
        }

        case SortOperation::Type::Overwrite:
        {
            if (!isValidIndex(op.indexA))
            {
                VIGIL_LOG_ERROR("SortPlayback: Overwrite index out of range while undoing (indexA={}, size={})",
                          op.indexA, buf.size());
                break;
            }
            // valueB holds the value that was overwritten; restore it.
            buf[op.indexA] = op.valueB;
            break;
        }

        case SortOperation::Type::Compare:
        case SortOperation::Type::Pivot:
        case SortOperation::Type::Sorted:
            // No action needed for these types; for now.
            break;

        default:
            VIGIL_LOG_WARN("SortPlayback: Unknown operation type encountered while undoing: {}", toString(op.type));
            break;
    }
}

void SortPlayback::resetIndex()
{
    if (!m_Operations.empty())
    {
        m_CurrentIndex = 0;
    }
    else
    {
        VIGIL_LOG_TRACE("SortPlayback: Reset index skipped (no operations loaded)");
    }
}

void SortPlayback::setState(PlaybackState newState)
{
    m_State = newState;
}

// ================================================
// SETUP
// ================================================

void SortPlayback::bind(std::vector<int>& visualBuffer)
{
    m_VisualBuffer = &visualBuffer;
    VIGIL_LOG_DEBUG("SortPlayback: Bound to visual buffer ({} elements)", visualBuffer.size());
}

bool SortPlayback::loadOperations(std::span<const SortOperation> ops)
{
    if (!m_VisualBuffer)
    {
        VIGIL_LOG_ERROR("SortPlayback: Cannot load operations - no visual buffer bound. Call bind() first.");
        return false;
    }

    if (ops.empty())
    {
        VIGIL_LOG_WARN("SortPlayback: No operations to load - received empty operation list");
        return false;
    }

    m_InitialData = *m_VisualBuffer;

    m_Operations.assign(ops.begin(), ops.end());
    m_CurrentIndex = 0;
    m_State = PlaybackState::Stopped;

    VIGIL_LOG_DEBUG("SortPlayback: Loaded {} operations", m_Operations.size());

    return true;
}

// ================================================
// UPDATE
// ================================================

void SortPlayback::update(float deltaTime)
{
    if (m_State != PlaybackState::Playing)
        return;

    if (m_Operations.empty())
    {
        VIGIL_LOG_WARN("SortPlayback: No operations loaded - cannot update playback");
        setState(PlaybackState::Finished);
        return;
    }

    // Instant mode: apply all remaining operations in one frame
    if (!m_EnableDelay)
    {
        while (stepForward()) {}
        return;
    }

    m_ElapsedTime += deltaTime;

    // Convert milliseconds to seconds
    const float delaySeconds = m_DelayMs / 1000.0f;

    // Allow multiple steps per frame if enough time has accumulated
    while (m_ElapsedTime >= delaySeconds)
    {
        if (!stepForward())
            break;

        m_ElapsedTime -= delaySeconds;
    }
}

// ================================================
// PLAYBACK CONTROL
// ================================================

void SortPlayback::play()
{
    if (m_Operations.empty())
    {
        VIGIL_LOG_WARN("SortPlayback: Cannot play - no operations loaded");
        return;
    }

    if (!m_VisualBuffer)
    {
        VIGIL_LOG_ERROR("SortPlayback: Cannot play - no visual buffer bound");
        return;
    }

    // Restore the initial state to avoid applying operations over the completed result.
    if (m_State == PlaybackState::Finished)
    {
        *m_VisualBuffer = m_InitialData;
        resetIndex();
    }

    setState(PlaybackState::Playing);
    VIGIL_LOG_DEBUG("SortPlayback: Playing {} operations from index {}",
              m_Operations.size(), m_CurrentIndex);
}

void SortPlayback::pause()
{
    if (m_State != PlaybackState::Playing)
        return;

    setState(PlaybackState::Paused);
    VIGIL_LOG_DEBUG("SortPlayback: Paused at index {}", m_CurrentIndex);
}

void SortPlayback::resume()
{
    if (m_State != PlaybackState::Paused)
        return;

    setState(PlaybackState::Playing);
    VIGIL_LOG_DEBUG("SortPlayback: Resumed at index {}", m_CurrentIndex);
}

void SortPlayback::restoreInitialState()
{
    if (!m_VisualBuffer)
    {
        VIGIL_LOG_ERROR("SortPlayback: Cannot restore initial state - no visual buffer bound");
        return;
    }
    *m_VisualBuffer = m_InitialData;
}

void SortPlayback::stop()
{
    resetIndex();
    setState(PlaybackState::Stopped);
    VIGIL_LOG_DEBUG("SortPlayback: Stopped playback and reset index to {}", m_CurrentIndex);
}

// ================================================
// STEP FUNCTIONS
// ================================================

bool SortPlayback::stepForward()
{
    // Current Index + 1 >= total operations means we've reached the end of the operation list
    if (m_CurrentIndex >= m_Operations.size())
    {
        setState(PlaybackState::Finished);
        return false;
    }

    applyOperation(m_Operations[m_CurrentIndex]);
    ++m_CurrentIndex;

    if (m_CurrentIndex >= m_Operations.size())
    {
        setState(PlaybackState::Finished);
        VIGIL_LOG_DEBUG("SortPlayback: Reached final operation, index {}", m_CurrentIndex);
    }

    return true;
}

bool SortPlayback::stepBackward()
{
    if (m_State == PlaybackState::Stopped)
        return false;

    if (m_CurrentIndex == 0)
    {
        setState(PlaybackState::Stopped);
        return false;
    }

    --m_CurrentIndex;
    undoOperation(m_Operations[m_CurrentIndex]);

    if (m_State == PlaybackState::Finished)
        setState(PlaybackState::Paused);

    return true;
}

const SortOperation* SortPlayback::getCurrentOperation() const
{
    if (m_Operations.empty() || m_CurrentIndex >= m_Operations.size())
        return nullptr;

    return &m_Operations[m_CurrentIndex];
}

// ================================================
// DELAY
// ================================================

void SortPlayback::enableDelay(bool value)
{
    if (!value)
    {
        m_OldDelayMs = m_DelayMs;
        m_DelayMs    = 0.0f;
    }
    else
    {
        m_DelayMs = m_OldDelayMs;
    }

    m_EnableDelay = value;
}

// ================================================
// PROGRESS
// ================================================

float SortPlayback::getProgress() const
{
    if (m_Operations.empty())
        return 0.0f;

    return static_cast<float>(m_CurrentIndex) /
           static_cast<float>(m_Operations.size());
}

} // namespace Sortly
