// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#pragma once

#include "Sortly/Gui/RunSettings.hpp"
#include "Sortly/Gui/SessionManager.hpp"

#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <utility>
#include <vector>

namespace Sortly {

/// @brief One algorithm of a run: its session plus the state of building its timeline.
struct Run
{
    SortAlgorithmType            type = SortAlgorithmType::BubbleSort;
    std::unique_ptr<SortSession> session;
    bool                         computing = false;
    std::string                  error; // Non-empty when the run could not be created or computed.

    /// @brief Timeline built and loaded: statistics are final and playback can be used.
    [[nodiscard]] bool ready() const
    {
        return session && !computing && error.empty() && session->getPlayback().isLoaded();
    }
};

/// @brief UI-independent run logic: creates one session per selected algorithm (all on the
/// same data), computes their operations cooperatively (time-budgeted) and drives their
/// playback together. It owns the sessions.
class RunController
{
public:
    using SessionFactory = std::function<SessionResult(SortAlgorithmType, const RunSettings&)>;

    explicit RunController(SessionFactory factory) : m_Factory(std::move(factory)) {}

    /// @brief Queues a new run. It is applied at the start of the next update(), so the
    /// sessions never change underneath panels that are mid-frame.
    /// @param autoplay Start playing as soon as every timeline is ready.
    void requestStart(const RunSettings& settings, bool autoplay = false)
    {
        m_Pending         = settings;
        m_PendingAutoplay = autoplay;
    }

    void update(float deltaTime);

    /// @brief Applies to the current runs and to the ones created later.
    void setDelayMs(float delayMs);

    // Global playback controls: they act on every ready run at once.
    void playAll();
    void pauseAll();
    void stopAll();
    void stepAll(bool forward);

    [[nodiscard]] std::span<const Run> runs() const { return m_Runs; }

    /// @brief True if the current runs were built from settings equivalent to @p settings.
    [[nodiscard]] bool matches(const RunSettings& settings) const
    {
        return m_Applied && m_Applied->sameRunAs(settings);
    }

    [[nodiscard]] bool computing()   const;
    [[nodiscard]] bool allReady()    const;
    [[nodiscard]] bool anyPlaying()  const;
    [[nodiscard]] bool allFinished() const;

private:
    void start(const RunSettings& settings);
    void updateComputation();

    SessionFactory             m_Factory;
    std::vector<Run>           m_Runs;
    std::optional<RunSettings> m_Applied;
    std::optional<RunSettings> m_Pending;
    bool                       m_PendingAutoplay = false;
    bool                       m_Autoplay        = false; // Applies once the timelines finish building.
    float                      m_DelayMs         = 32.0f;
};

} // namespace Sortly
