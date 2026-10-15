// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "Sortly/Gui/SessionManager.hpp"
#include "Sortly/Sorting/Algorithms/SortAlgorithmFactory.hpp"

namespace Sortly {

SortSession* SessionManager::createSession(SortAlgorithmType type, const DataSetConfig& config)
{
    auto algorithm = SortAlgorithmFactory::create(type);
    if (!algorithm)
        return nullptr;

    auto session = std::make_unique<SortSession>();
    session->setDataSetConfig(config);
    session->setAlgorithm(std::move(algorithm));

    auto rng = m_Seeds.createEngine(m_NextIndex++);
    if (!session->buildDataSet(rng))
        return nullptr;

    m_Sessions.push_back(std::move(session));
    return m_Sessions.back().get();
}

} // namespace Sortly
