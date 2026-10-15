// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "Sortly/Gui/RunController.hpp"

#include <vigil/vigil.h>

#include <chrono>

namespace Sortly {

namespace { constexpr std::chrono::microseconds kComputeBudget{2500}; }

bool RunController::start(const RunSettings& settings)
{
    const DataSetConfig config{ settings.elementCount, 2, 4096, 500 };

    // The factory may destroy previous sessions, so drop the old pointer first.
    m_Active    = nullptr;
    m_Computing = false;

    SortSession* session = m_Factory ? m_Factory(settings.algorithm, config) : nullptr;
    if (!session)
    {
        VIGIL_LOG_ERROR("Could not create a sorting session");
        m_Status = "Could not create this run. Check the application log.";
        return false;
    }

    m_Active = session;
    m_Active->getPlayback().setDelayMs(settings.stepDelayMs);
    m_Computing = true;
    m_Status = "Building the operation timeline...";
    return true;
}

void RunController::update(float deltaTime)
{
    if (!m_Active)
        return;

    if (m_Computing)
    {
        const auto result = m_Active->computeOperations(kComputeBudget);

        if (result == SortSession::ComputeStatus::Failed)
        {
            m_Computing = false;
            m_Status = "Could not build this run. Check the application log.";
            return;
        }

        if (result == SortSession::ComputeStatus::Complete)
        {
            m_Computing = false;
            if (!m_Active->loadPlayback())
            {
                VIGIL_LOG_ERROR("Could not prepare playback for the completed sorting session");
                m_Status = "Could not prepare playback. Check the application log.";
                return;
            }
            m_Status = "Run ready. Play it or inspect it one step at a time.";
        }
    }

    if (!m_Computing)
        m_Active->getPlayback().update(deltaTime);
}

} // namespace Sortly
