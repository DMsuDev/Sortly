// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "Sortly/Gui/BarMotion.hpp"

#include <cmath>
#include <utility>

namespace Sortly {

void BarMotion::reset(std::span<const int> initial)
{
    const std::size_t count = initial.size();
    m_Owner.resize(count);
    m_Slot.resize(count);
    m_X.resize(count);
    m_Height.resize(count);

    for (std::size_t i = 0; i < count; ++i)
    {
        m_Owner[i]  = i;
        m_Slot[i]   = i;
        m_X[i]      = static_cast<float>(i);
        m_Height[i] = static_cast<float>(initial[i]);
    }
    m_Cursor = 0;
}

void BarMotion::applySwap(const SortOperation& op)
{
    if (op.type != SortOperation::Type::Swap)
        return;

    // Indices were validated by SortPlayback::load().
    const auto a = static_cast<std::size_t>(op.indexA);
    const auto b = static_cast<std::size_t>(op.indexB);
    std::swap(m_Owner[a], m_Owner[b]);
    m_Slot[m_Owner[a]] = a;
    m_Slot[m_Owner[b]] = b;
}

bool BarMotion::sync(const SortPlayback& playback)
{
    const auto&       initial = playback.getInitialData();
    const auto&       ops     = playback.getOperations(); // Reference: do not copy the list.
    const std::size_t cursor  = playback.getCurrentOperationIndex();

    // First use or a large step back (Stop, restart): rebuild identity without animating.
    if (m_Owner.size() != initial.size() || m_Cursor > cursor + 1)
    {
        reset(initial);
        while (m_Cursor < cursor)
            applySwap(ops[m_Cursor++]);
        return true;
    }

    while (m_Cursor < cursor)
        applySwap(ops[m_Cursor++]);
    while (m_Cursor > cursor)
        applySwap(ops[--m_Cursor]);
    return false;
}

void BarMotion::update(const SortPlayback& playback, float deltaTime, float easeSeconds, bool smooth)
{
    const bool  snap = sync(playback) || !smooth || easeSeconds <= 0.0f;
    const float k    = snap ? 1.0f : 1.0f - std::exp(-deltaTime / easeSeconds);

    const auto& data = playback.getVisualData(); // Reference: do not copy per frame.
    for (std::size_t bar = 0; bar < m_X.size(); ++bar)
    {
        const std::size_t slot = m_Slot[bar];
        m_X[bar]      += (static_cast<float>(slot) - m_X[bar]) * k;
        m_Height[bar] += (static_cast<float>(data[slot]) - m_Height[bar]) * k;
    }
}

} // namespace Sortly
