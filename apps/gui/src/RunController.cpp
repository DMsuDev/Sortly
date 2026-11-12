// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "Sortly/Gui/RunController.hpp"

#include <vigil/logging/log_system.h>

#include <chrono>
#include <utility>

namespace Sortly {

namespace {

constexpr float kMotionSettle = 0.45f; // Fraction of a step delay the bars take to settle.
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

const char* describe(sortrace::ComputeError error)
{
    switch (error)
    {
        case sortrace::ComputeError::NoAlgorithm:         return "Choose an algorithm first.";
        case sortrace::ComputeError::NoData:              return "The dataset is empty.";
        case sortrace::ComputeError::TooLargeForBlocking: return "The dataset is too large for this algorithm.";
        case sortrace::ComputeError::AlgorithmThrew:      return "The algorithm failed unexpectedly.";
    }
    return "Could not build this run.";
}

} // namespace

void RunController::start(const RunSettings& settings)
{
    m_Runs.clear();
    m_Applied = settings;
    m_Smooth  = settings.smoothMotion;
    m_Runs.reserve(settings.algorithms.size());

    for (const SortAlgorithmType type : settings.algorithms)
    {
        Run run;
        run.type = type;

        SessionResult result = m_Factory ? m_Factory(type, settings) : SessionResult{ nullptr, SessionError::NoAlgorithm };
        if (result.session)
        {
            run.session = std::move(result.session);
            run.playback.setLogSink(vigilLogSink());
            run.playback.setDelayMs(m_DelayMs);
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

        const auto result = run.session->compute(share);
        if (!result)
        {
            const auto error = result.error();
            VIGIL_LOG_ERROR("Computing the operations failed: {}", sortrace::toString(error));
            run.computing = false;
            run.error     = describe(error);
        }
        else if (*result == sortrace::Progress::Complete)
        {
            run.computing = false;
            const auto loaded = run.playback.load(run.session->getRecording());
            if (!loaded)
            {
                VIGIL_LOG_ERROR("Could not prepare playback for the completed sorting session");
                run.error = "Could not prepare playback.";
            }
            else // Build the settled animation timeline for this run.
            {
                const auto recording = run.session->getRecording();
                run.settledAt.assign(recording->initial.size(), 0);

                for (std::size_t i = 0; i < recording->operations.size(); ++i)
                {
                    const auto& operation = recording->operations[i];
                    const auto settledAt = i + 1;
                    if (operation.type == sortrace::OperationType::Swap)
                    {
                        run.settledAt[static_cast<std::size_t>(operation.indexA)] = settledAt;
                        run.settledAt[static_cast<std::size_t>(operation.indexB)] = settledAt;
                    }
                    else if (operation.type == sortrace::OperationType::Overwrite)
                    {
                        run.settledAt[static_cast<std::size_t>(operation.indexA)] = settledAt;
                    }
                }
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

    // Time constant follows the step delay, so motion stays proportional at any speed.
    const float easeSeconds = m_DelayMs * 0.001f * kMotionSettle;

    for (Run& run : m_Runs)
    {
        if (!run.ready())
            continue;

        run.playback.update(deltaTime);
        run.motion.update(run.playback, deltaTime, easeSeconds, m_Smooth);

        // Stepping back, Stop or a restart leave Finished, which rewinds the sweep.
        run.sweepSeconds = run.playback.isFinished() ? run.sweepSeconds + deltaTime : 0.0f;
    }
}

void RunController::setDelayMs(float delayMs)
{
    // Called every frame by the application: skip the loop when nothing changed.
    if (delayMs == m_DelayMs)
        return;

    m_DelayMs = delayMs;
    for (Run& run : m_Runs)
        if (run.session)
            run.playback.setDelayMs(delayMs);
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
        if (run.playback.isFinished() && !restart)
            continue;
        run.playback.play();
    }
}

void RunController::pauseAll()
{
    m_Autoplay = false;
    for (Run& run : m_Runs)
        if (run.ready())
            run.playback.pause();
}

void RunController::stopAll()
{
    m_Autoplay = false;
    for (Run& run : m_Runs)
        if (run.ready())
            run.playback.stop();
}

void RunController::stepAll(bool forward)
{
    m_Autoplay = false;
    for (Run& run : m_Runs)
    {
        if (!run.ready())
            continue;
        // Manual steps already leave Stopped by themselves; they only need automatic play halted.
        run.playback.pause();
        if (forward) run.playback.stepForward();
        else         run.playback.stepBackward();
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
        if (run.ready() && run.playback.isPlaying())
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
        if (!run.playback.isFinished())
            return false;
        any = true;
    }
    return any;
}

} // namespace Sortly
