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
    m_Runs.clear();
    m_Applied = settings;

    for (const SortAlgorithmType type : settings.algorithms)
    {
        Run run;
        run.type = type;

        SessionResult result = m_Factory ? m_Factory(type, settings) : SessionResult{ nullptr, SessionError::NoAlgorithm };
        if (result.session)
        {
            run.session = std::move(result.session);
            run.session->getPlayback().setDelayMs(m_DelayMs);
            run.computing = true;
        }
        else
        {
            VIGIL_LOG_ERROR("Could not create a sorting session: {}", describe(result.error));
            run.error = describe(result.error);
        }
        m_Runs.push_back(std::move(run));
    }
}

void RunController::updateComputation()
{
    std::size_t pending = 0;
    for (const Run& run : m_Runs)
        pending += run.computing ? 1 : 0;
    if (pending == 0)
        return;

    // Every run gets an equal share so none of them starves the others (or the frame).
    const auto share = kComputeBudget / static_cast<std::chrono::microseconds::rep>(pending);

    for (Run& run : m_Runs)
    {
        if (!run.computing)
            continue;

        const auto result = run.session->computeOperations(share);
        if (result == SortSession::ComputeStatus::Failed)
        {
            const auto error = run.session->getLastComputeError();
            VIGIL_LOG_ERROR("Computing the operations failed: {}", toString(error));
            run.computing = false;
            run.error     = describe(error);
        }
        else if (result == SortSession::ComputeStatus::Complete)
        {
            run.computing = false;
            if (!run.session->loadPlayback())
            {
                VIGIL_LOG_ERROR("Could not prepare playback for the completed sorting session");
                run.error = "Could not prepare playback.";
            }
        }
    }

    if (!computing() && m_Autoplay)
    {
        m_Autoplay = false;
        playAll();
    }
}

void RunController::update(float deltaTime)
{
    if (m_Pending)
    {
        const RunSettings settings = *m_Pending;
        m_Autoplay = m_PendingAutoplay;
        m_Pending.reset();
        start(settings);
    }

    updateComputation();

    for (Run& run : m_Runs)
        if (run.ready())
            run.session->getPlayback().update(deltaTime);
}

void RunController::setDelayMs(float delayMs)
{
    m_DelayMs = delayMs;
    for (Run& run : m_Runs)
        if (run.session)
            run.session->getPlayback().setDelayMs(delayMs);
}

void RunController::playAll()
{
    if (computing())
    {
        m_Autoplay = true;
        return;
    }

    // Finished runs only restart when the whole group is done; otherwise they wait for the others.
    const bool restart = allFinished();
    for (Run& run : m_Runs)
    {
        if (!run.ready())
            continue;
        SortPlayback& playback = run.session->getPlayback();
        if (playback.isFinished() && !restart)
            continue;
        playback.play();
    }
}

void RunController::pauseAll()
{
    m_Autoplay = false;
    for (Run& run : m_Runs)
        if (run.ready())
            run.session->getPlayback().pause();
}

void RunController::stopAll()
{
    m_Autoplay = false;
    for (Run& run : m_Runs)
        if (run.ready())
            run.session->getPlayback().stop();
}

void RunController::stepAll(bool forward)
{
    m_Autoplay = false;
    for (Run& run : m_Runs)
    {
        if (!run.ready())
            continue;
        SortPlayback& playback = run.session->getPlayback();
        // Manual steps already leave Stopped by themselves; they only need automatic play halted.
        playback.pause();
        if (forward) playback.stepForward();
        else         playback.stepBackward();
    }
}

bool RunController::computing() const
{
    for (const Run& run : m_Runs)
        if (run.computing)
            return true;
    return false;
}

bool RunController::allReady() const
{
    if (m_Runs.empty())
        return false;
    for (const Run& run : m_Runs)
        if (!run.ready())
            return false;
    return true;
}

bool RunController::anyPlaying() const
{
    for (const Run& run : m_Runs)
        if (run.ready() && run.session->getPlayback().isPlaying())
            return true;
    return false;
}

bool RunController::allFinished() const
{
    bool any = false;
    for (const Run& run : m_Runs)
    {
        if (!run.ready())
            continue;
        if (!run.session->getPlayback().isFinished())
            return false;
        any = true;
    }
    return any;
}

} // namespace Sortly
