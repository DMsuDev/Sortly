// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#pragma once

#include "Sortly/Sorting/Algorithms/SortAlgorithmFactory.hpp"
#include "Sortly/Sorting/Data/DataSetConfig.hpp"
#include "Sortly/Sorting/Data/SeedProvider.hpp"
#include "Sortly/Sorting/SortSession.hpp"

#include <cstddef>
#include <memory>

namespace Sortly {

/// @brief Why a session could not be created.
enum class SessionError
{
    None,
    NoAlgorithm, // The factory has no algorithm for the requested type.
    NoData       // The dataset came out empty.
};

struct SessionResult
{
    std::unique_ptr<SortSession> session;
    SessionError                 error = SessionError::None;
};

/// @brief Builds sessions and decides where their randomness comes from.
/// Ownership of each session goes to the caller, so nothing here can leave a
/// frontend holding a dangling pointer.
class SessionManager
{
public:
    void setSeedProvider(SeedProvider provider) { m_Seeds = provider; }

    /// @brief Creates a session with a freshly built, shuffled dataset.
    [[nodiscard]] SessionResult createSession(SortAlgorithmType type, const DataSetConfig& config);

private:
    SeedProvider m_Seeds     = SeedProvider::random();
    std::size_t  m_NextIndex = 0; // Monotonic: keeps per-session/random seeds unique.
};

} // namespace Sortly
