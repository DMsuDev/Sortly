// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#pragma once

// The GUI sorts plain ints. These aliases pin sortrace's value type in one place, so the rest of the
// app never spells `<int>`. Changing what the GUI sorts means changing SortValue.

#include <sortrace/algorithms/registry.hpp>
#include <sortrace/core/log.hpp>
#include <sortrace/core/operation.hpp>
#include <sortrace/core/recording.hpp>
#include <sortrace/core/statistics.hpp>
#include <sortrace/data/data.hpp>
#include <sortrace/data/seed.hpp>
#include <sortrace/execution/session.hpp>
#include <sortrace/replay/playback.hpp>

namespace Sortly {

using SortValue         = int;
using SortAlgorithmType = sortrace::AlgorithmType;
using SortOperationType = sortrace::OperationType;
using SortOperation     = sortrace::Operation<SortValue>;
using SortRecording     = sortrace::Recording<SortValue>;
using SortSession       = sortrace::Session<SortValue>;
using SortPlayback      = sortrace::Playback<SortValue>;
using SortStatistics    = sortrace::Statistics;
using DataSetConfig     = sortrace::DataSetConfig;
using SeedProvider      = sortrace::SeedProvider;

/// @brief Forwards sortrace's diagnostics to Vigil. Use it as the log sink for sessions and playbacks.
[[nodiscard]] sortrace::LogSink vigilLogSink();

} // namespace Sortly
