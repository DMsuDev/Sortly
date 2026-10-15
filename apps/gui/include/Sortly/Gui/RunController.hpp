// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#pragma once

#include "Sortly/Gui/RunSettings.hpp"
#include "Sortly/Sorting/Data/DataSetConfig.hpp"
#include "Sortly/Sorting/SortSession.hpp"

#include <functional>
#include <string>

namespace Sortly {

/// @brief UI-independent run logic: creates a session, computes its operations
/// cooperatively (time-budgeted) and drives playback.
class RunController
{
public:
    using SessionFactory = std::function<SortSession*(SortAlgorithmType, const DataSetConfig&)>;

    explicit RunController(SessionFactory factory) : m_Factory(std::move(factory)) {}

    bool start(const RunSettings& settings);
    void update(float deltaTime);

    [[nodiscard]] SortSession*       session()   const { return m_Active; }
    [[nodiscard]] bool               computing() const { return m_Computing; }
    [[nodiscard]] const std::string& status()    const { return m_Status; }

private:
    SessionFactory m_Factory;
    SortSession*   m_Active    = nullptr;
    bool           m_Computing = false;
    std::string    m_Status    = "Create a run to inspect the algorithm.";
};

} // namespace Sortly
