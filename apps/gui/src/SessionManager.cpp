// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "Sortly/Gui/SessionManager.hpp"

#include <utility>

namespace Sortly {

SessionResult SessionManager::createSession(SortAlgorithmType type, const DataSetConfig& config)
{
    auto algorithm = SortAlgorithmFactory::create(type);
    if (!algorithm)
        return { nullptr, SessionError::NoAlgorithm };

    auto session = std::make_unique<SortSession>();
    session->setDataSetConfig(config);
    session->setAlgorithm(std::move(algorithm));

    auto rng = m_Seeds.createEngine(m_NextIndex++);
    if (!session->buildDataSet(rng))
        return { nullptr, SessionError::NoData };

    return { std::move(session), SessionError::None };
}

} // namespace Sortly
