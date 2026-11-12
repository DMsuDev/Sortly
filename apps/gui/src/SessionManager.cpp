// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "Sortly/Gui/SessionManager.hpp"

#include <sortrace/algorithms/factory.hpp>

#include <vigil/logging/log_system.h>

#include <string>
#include <string_view>
#include <utility>

namespace Sortly {

sortrace::LogSink vigilLogSink()
{
    return [](sortrace::LogLevel level, std::string_view message) {
        const std::string text(message);
        switch (level)
        {
            case sortrace::LogLevel::Trace: VIGIL_LOG_TRACE("{}", text); break;
            case sortrace::LogLevel::Debug: VIGIL_LOG_DEBUG("{}", text); break;
            case sortrace::LogLevel::Info:  VIGIL_LOG_INFO ("{}", text); break;
            case sortrace::LogLevel::Warn:  VIGIL_LOG_WARN ("{}", text); break;
            case sortrace::LogLevel::Error: VIGIL_LOG_ERROR("{}", text); break;
        }
    };
}

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
    session->setLogSink(vigilLogSink());
    session->setAlgorithm(std::move(algorithm));
    session->setData(std::move(data));

    return { std::move(session), SessionError::None };
}

} // namespace Sortly
