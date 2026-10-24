// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "Sortly/Sorting/Operations/SortPlayback.hpp"

#include <vigil/vigil.h>

#include <algorithm>
#include <cassert>
#include <iterator>
#include <optional>
#include <utility>

namespace Sortly {

namespace {

bool inRange(int index, std::size_t size) noexcept
{
    return index >= 0 && static_cast<std::size_t>(index) < size;
}

/// Position of the first operation that uses an index outside [0, size), if any.
std::optional<std::size_t> findInvalidOperation(std::span<const SortOperation> ops, std::size_t size) noexcept
{
    for (std::size_t i = 0; i < ops.size(); ++i)
    {
        const SortOperation& op = ops[i];

        switch (op.type)
        {
            case SortOperation::Type::Swap:
                if (!inRange(op.indexA, size) || !inRange(op.indexB, size))
                    return i;
                break;

            case SortOperation::Type::Overwrite:
                if (!inRange(op.indexA, size))
                    return i;
                break;

            case SortOperation::Type::Pivot:
                // -1 clears the mark; anything else must point at a real element.
                if (op.indexA != -1 && !inRange(op.indexA, size))
                    return i;
                break;

            case SortOperation::Type::Compare:
            case SortOperation::Type::Sorted:
                break; // Never touch the data; their indices are for display only.
        }
    }

    return std::nullopt;
}

} // namespace

// ================================================
// INTERNAL FUNCTIONS
// ================================================

void SortPlayback::execute(const SortOperation& op, bool reverse)
{
    // load() validated every index, so this hot path only asserts (debug builds) and never logs:
    // a corrupt operation list can't flood the log with one error per step.
    switch (op.type)
    {
        case SortOperation::Type::Swap:
            assert(inRange(op.indexA, m_Visual.size()) && inRange(op.indexB, m_Visual.size()));
            // A swap is its own inverse.
            std::swap(m_Visual[static_cast<std::size_t>(op.indexA)],
                      m_Visual[static_cast<std::size_t>(op.indexB)]);
            break;

        case SortOperation::Type::Overwrite:
            assert(inRange(op.indexA, m_Visual.size()));
            // valueA is the new value, valueB the one it replaced.
            m_Visual[static_cast<std::size_t>(op.indexA)] = reverse ? op.valueB : op.valueA;
            break;

        case SortOperation::Type::Compare:
        case SortOperation::Type::Pivot:  // The pivot mark is not data: see getPivotIndex().
        case SortOperation::Type::Sorted:
            break;
    }
}

void SortPlayback::reset()
{
    m_Visual = m_Initial;
    m_CurrentIndex = 0;
    m_ElapsedTime = 0.0f;
    m_State = PlaybackState::Stopped;
}

void SortPlayback::buildPivotTimeline()
{
    m_PivotTimeline.clear();

    // Only real changes are stored, so a long run with few pivots keeps a tiny timeline.
    const auto change = [this](std::size_t appliedOps, int index) {
        const int current = m_PivotTimeline.empty() ? -1 : m_PivotTimeline.back().index;
        if (index != current)
            m_PivotTimeline.push_back({ appliedOps, index });
    };

    for (std::size_t i = 0; i < m_Operations.size(); ++i)
    {
        const SortOperation& op = m_Operations[i];

        if (op.type == SortOperation::Type::Pivot)
            change(i + 1, op.indexA); // indexA == -1 is "cleared".
        else if (op.type == SortOperation::Type::Sorted && op.indexA < 0)
            change(i + 1, -1);        // The end of the run drops a mark nobody cleared.
    }

    m_PivotTimeline.shrink_to_fit();
}

// ================================================
// SETUP
// ================================================

bool SortPlayback::load(std::span<const int> initialData, std::span<const SortOperation> ops)
{
    if (ops.empty())
    {
        // load() returns false, so the caller already knows.
        VIGIL_LOG_DEBUG("SortPlayback: load() ignored: empty operation list");
        return false;
    }

    // Validate once at the boundary, and report once: this is a bug in whoever built the list.
    if (const auto bad = findInvalidOperation(ops, initialData.size()))
    {
        VIGIL_LOG_ERROR("SortPlayback: Rejected operation list: operation {} of {} ({}) uses an index outside the {} elements",
                        *bad, ops.size(), toString(ops[*bad].type), initialData.size());
        return false;
    }

    m_Initial.assign(initialData.begin(), initialData.end());
    m_Operations.assign(ops.begin(), ops.end());
    buildPivotTimeline();
    reset();

    VIGIL_LOG_DEBUG("SortPlayback: Loaded {} operations over {} elements", m_Operations.size(), m_Initial.size());
    return true;
}

void SortPlayback::clear()
{
    m_Operations.clear();
    m_Operations.shrink_to_fit();
    m_PivotTimeline.clear();
    m_PivotTimeline.shrink_to_fit();
    m_Initial.clear();
    reset(); // m_Visual follows m_Initial (now empty)
}

// ================================================
// UPDATE
// ================================================

void SortPlayback::update(float deltaTime)
{
    if (m_State != PlaybackState::Playing)
        return;

    // Instant mode: apply all remaining operations in one frame.
    if (!m_DelayEnabled || m_DelayMs <= 0.0f)
    {
        while (stepForward()) {}
        return;
    }

    m_ElapsedTime += deltaTime;

    const float delaySeconds = m_DelayMs / 1000.0f;

    // Allow multiple steps per frame if enough time has accumulated.
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

bool SortPlayback::play()
{
    if (m_Operations.empty())
    {
        // Pressing play with nothing loaded is a normal UI situation, not an anomaly.
        VIGIL_LOG_DEBUG("SortPlayback: play() ignored: no operations loaded");
        return false;
    }

    // Restart from the initial state instead of applying operations over the completed result.
    if (m_State == PlaybackState::Finished)
        reset();

    m_State = PlaybackState::Playing;
    VIGIL_LOG_DEBUG("SortPlayback: Playing {} operations from index {}", m_Operations.size(), m_CurrentIndex);
    return true;
}

void SortPlayback::pause()
{
    if (m_State != PlaybackState::Playing)
        return;

    m_State = PlaybackState::Paused;
    VIGIL_LOG_DEBUG("SortPlayback: Paused at index {}", m_CurrentIndex);
}

void SortPlayback::resume()
{
    if (m_State != PlaybackState::Paused)
        return;

    m_State = PlaybackState::Playing;
    VIGIL_LOG_DEBUG("SortPlayback: Resumed at index {}", m_CurrentIndex);
}

void SortPlayback::stop()
{
    reset();
    VIGIL_LOG_DEBUG("SortPlayback: Stopped; buffer and cursor restored to the initial state");
}

// ================================================
// STEP FUNCTIONS
// ================================================

bool SortPlayback::stepForward()
{
    if (m_CurrentIndex >= m_Operations.size())
    {
        m_State = PlaybackState::Finished;
        return false;
    }

    // A manual step leaves the pristine Stopped state, so stepBackward() works afterwards.
    if (m_State == PlaybackState::Stopped)
        m_State = PlaybackState::Paused;

    execute(m_Operations[m_CurrentIndex], /*reverse=*/false);
    ++m_CurrentIndex;

    if (m_CurrentIndex >= m_Operations.size())
    {
        m_State = PlaybackState::Finished;
        VIGIL_LOG_DEBUG("SortPlayback: Reached final operation, index {}", m_CurrentIndex);
    }

    return true;
}

bool SortPlayback::stepBackward()
{
    if (m_CurrentIndex == 0)
        return false;

    --m_CurrentIndex;
    execute(m_Operations[m_CurrentIndex], /*reverse=*/true);

    // Stepping back always stops automatic playback; at the first operation we are pristine again.
    m_State = (m_CurrentIndex == 0) ? PlaybackState::Stopped : PlaybackState::Paused;
    return true;
}

const SortOperation* SortPlayback::getCurrentOperation() const
{
    if (m_CurrentIndex >= m_Operations.size())
        return nullptr;

    return &m_Operations[m_CurrentIndex];
}

int SortPlayback::getPivotIndex() const
{
    // The change in force is the last one whose operation is already applied: the first one
    // that still needs more operations than the cursor has passed, minus one.
    const auto next = std::upper_bound(
        m_PivotTimeline.begin(), m_PivotTimeline.end(), m_CurrentIndex,
        [](std::size_t applied, const PivotChange& change) { return applied < change.appliedOps; });

    return next == m_PivotTimeline.begin() ? -1 : std::prev(next)->index;
}

// ================================================
// PROGRESS
// ================================================

float SortPlayback::getProgress() const
{
    if (m_Operations.empty())
        return 0.0f;

    return static_cast<float>(m_CurrentIndex) / static_cast<float>(m_Operations.size());
}

} // namespace Sortly
