// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#pragma once

#include "Sortly/Sorting/Algorithms/SortAlgorithmType.hpp"
#include "Sortly/Sorting/Data/DataSetConfig.hpp"
#include "Sortly/Sorting/Data/detail/SeedProvider.hpp"
#include "Sortly/Sorting/Session/SortSession.hpp"

#include <cstddef>
#include <memory>
#include <span>
#include <vector>

namespace Sortly {

/// @brief Owns every SortSession and decides where their randomness comes from.
/// Sessions are heap-allocated so references handed to a frontend stay valid
/// while other sessions are added.
class SessionManager
{
public:
    void setSeedProvider(SeedProvider provider) { m_Seeds = provider; }

    /// @brief Creates a session with a freshly built, shuffled dataset.
    /// @return The new session, or nullptr if the algorithm or dataset failed.
    [[nodiscard]] SortSession* createSession(SortAlgorithmType type, const DataSetConfig& config);

    [[nodiscard]] std::span<const std::unique_ptr<SortSession>> sessions() const { return m_Sessions; }

    void clear() { m_Sessions.clear(); }

private:
    SeedProvider m_Seeds = SeedProvider::random();
    std::size_t  m_NextIndex = 0; // Monotonic: stays unique even after clear().

    std::vector<std::unique_ptr<SortSession>> m_Sessions;
};

} // namespace Sortly
