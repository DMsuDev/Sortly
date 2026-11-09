// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "Sortly/Gui/SessionManager.hpp"

#include <string>
#include <string_view>
#include <utility>

namespace Sortly {

SessionResult SessionManager::createSession(SortAlgorithmType type, const DataSetConfig& config)
{
    auto algorithm = sortrace::make<SortValue>(type);
    if (!algorithm)
        return { nullptr, SessionError::NoAlgorithm };

    auto rng  = m_Seeds.createEngine(m_NextIndex++);
    auto data = sortrace::DataSetBuilder::buildShuffled<SortValue>(config, rng);
    if (data.empty())
        return { nullptr, SessionError::NoData };

    auto session = std::make_unique<SortSession>();
    session->setAlgorithm(std::move(algorithm));
    session->setData(std::move(data));

    return { std::move(session), SessionError::None };
}

} // namespace Sortly
