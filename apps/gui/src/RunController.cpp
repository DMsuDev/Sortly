// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "Sortly/Gui/RunController.hpp"

#include <vigil/vigil.h>

#include <chrono>

namespace Sortly {

namespace {

constexpr std::chrono::microseconds kComputeBudget{2500};

const char* describe(SessionError error)
{
    switch (error)
    {
        case SessionError::NoAlgorithm: return "That algorithm is not available.";
        case SessionError::NoData:      return "The dataset is empty.";
        case SessionError::None:        break;
    }
    return "Could not create this run.";
}

const char* describe(SortSession::ComputeError error)
{
    switch (error)
    {
        case SortSession::ComputeError::NoAlgorithm:         return "Choose an algorithm first.";
        case SortSession::ComputeError::NoData:              return "The dataset is empty.";
        case SortSession::ComputeError::TooLargeForBlocking: return "The dataset is too large for this algorithm.";
        case SortSession::ComputeError::AlgorithmThrew:      return "The algorithm failed unexpectedly.";
        case SortSession::ComputeError::None:                break;
    }
    return "Could not build this run.";
}

} // namespace

void RunController::start(const RunSettings& settings)
{
    m_Active.reset();
    m_Computing = false;

    SessionResult result = m_Factory ? m_Factory(settings) : SessionResult{ nullptr, SessionError::NoAlgorithm };
    if (!result.session)
    {
        VIGIL_LOG_ERROR("Could not create a sorting session: {}", describe(result.error));
        m_Status = describe(result.error);
        return;
    }

    m_Active = std::move(result.session);
    m_Active->getPlayback().setDelayMs(settings.stepDelayMs);
    m_Computing = true;
    m_Status = "Building the operation timeline...";
}

void RunController::update(float deltaTime)
{
    if (m_Pending)
    {
        const RunSettings settings = *m_Pending;
        m_Pending.reset();
        start(settings);
    }

    if (!m_Active)
        return;

    if (m_Computing)
    {
        const auto result = m_Active->computeOperations(kComputeBudget);
        if (result == SortSession::ComputeStatus::Failed)
        {
            const auto error = m_Active->getLastComputeError();
            VIGIL_LOG_ERROR("Computing the operations failed: {}", toString(error));
            m_Computing = false;
            m_Status = describe(error);
            return;
        }

        if (result == SortSession::ComputeStatus::Complete)
        {
            m_Computing = false;
            if (!m_Active->loadPlayback())
            {
                VIGIL_LOG_ERROR("Could not prepare playback for the completed sorting session");
                m_Status = "Could not prepare playback.";
                return;
            }
            m_Status = "Run ready. Play it or inspect it one step at a time.";
        }
    }

    if (!m_Computing)
        m_Active->getPlayback().update(deltaTime);
}

} // namespace Sortly
