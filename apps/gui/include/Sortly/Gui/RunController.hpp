// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#pragma once

#include "Sortly/Gui/RunSettings.hpp"
#include "Sortly/Gui/SessionManager.hpp"

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <utility>

namespace Sortly {

/// @brief UI-independent run logic: creates a session, computes its operations
/// cooperatively (time-budgeted) and drives playback. It owns the active session.
class RunController
{
public:
    using SessionFactory = std::function<SessionResult(const RunSettings&)>;

    explicit RunController(SessionFactory factory) : m_Factory(std::move(factory)) {}

    /// @brief Queues a new run. It is applied at the start of the next update(), so the
    /// session never changes underneath panels that are mid-frame.
    void requestStart(const RunSettings& settings) { m_Pending = settings; }

    void update(float deltaTime);

    [[nodiscard]] SortSession*       session()   const { return m_Active.get(); }
    [[nodiscard]] bool               computing() const { return m_Computing; }
    [[nodiscard]] const std::string& status()    const { return m_Status; }

private:
    void start(const RunSettings& settings);

    SessionFactory                m_Factory;
    std::unique_ptr<SortSession>  m_Active;
    std::optional<RunSettings>    m_Pending;
    bool                          m_Computing = false;
    std::string                   m_Status    = "Create a run to inspect the algorithm.";
};

} // namespace Sortly
