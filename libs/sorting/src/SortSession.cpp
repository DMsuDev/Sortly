// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "Sortly/Sorting/SortSession.hpp"
#include "Sortly/Sorting/Data/DataSetBuilder.hpp"
#include "Sortly/Sorting/Operations/SortRecorder.hpp"

#include <vigil/vigil.h>

#include <chrono>
#include <exception>
#include <utility>

namespace Sortly {

namespace {

// How many algorithm steps run between two reads of the clock. Reading steady_clock on every
// step would cost more than the step itself for the cheap steps of the O(n^2) algorithms.
constexpr std::size_t kStepsPerClockCheck = 256;

} // namespace

// Logging policy for this library:
//  - Failures are reported to the caller through return values (ComputeStatus/ComputeError,
//    bool); the logs are diagnostics for developers.
//  - ERROR: bugs and unrecoverable failures (an algorithm threw).
//  - WARN : abnormal but handled situations the caller may not be able to see.
//  - INFO : rare lifecycle events (one per completed run).
//  - DEBUG/TRACE: state transitions and detail. Never inside per-step loops.
//  - Setters that a UI may call every frame only log when something actually changed.

SortSession::ComputeStatus SortSession::fail(ComputeError error)
{
    m_LastComputeError = error;
    return m_LastComputeStatus = ComputeStatus::Failed;
}

//================================================//
//               CONFIGURATION
//================================================//

void SortSession::setDataSetConfig(const DataSetConfig& config)
{
    if (config == m_Config)
        return; // Immediate-mode UIs call setters every frame; nothing to do, nothing to log.

    m_Config = config;
    VIGIL_LOG_DEBUG("SortSession: Dataset configuration updated (count={}, minCount={}, maxCount={}, maxValue={})",
                    config.count, config.minCount, config.maxCount, config.maxValue);
}

void SortSession::setAlgorithm(std::unique_ptr<SortAlgorithm> algorithm)
{
    m_Algorithm = std::move(algorithm);
    clearOperations();

    VIGIL_LOG_DEBUG("SortSession: Algorithm set to {}", getAlgorithmName());
}

//================================================//
//                DATA MANAGEMENT
//================================================//

bool SortSession::buildDataSet(std::mt19937& rng)
{
    DataSet newDataSet;
    try
    {
        newDataSet = DataSetBuilder::buildShuffled(m_Config, rng);
    }
    catch (const std::exception& e)
    {
        VIGIL_LOG_ERROR("SortSession: Failed to build dataset (count={}): {}", m_Config.effectiveCount(), e.what());
        return false;
    }

    if (newDataSet.empty())
    {
        // buildDataSet() returns false, so the caller already knows; this is only a hint for developers.
        VIGIL_LOG_DEBUG("SortSession: Dataset configuration yields no elements (count={}); keeping the current dataset",
                        m_Config.effectiveCount());
        return false;
    }

    clearOperations();
    m_DataSet = std::move(newDataSet);

    return true;
}

void SortSession::shuffleData(std::mt19937& rng)
{
    DataSetBuilder::shuffle(m_DataSet, rng);
    clearOperations();

    VIGIL_LOG_DEBUG("SortSession: Shuffled data ({} elements)", m_DataSet.size());
}

//================================================//
//              OPERATION MANAGEMENT
//================================================//

SortSession::ComputeStatus SortSession::computeOperations(std::chrono::microseconds budget)
{
    if (!m_Algorithm)
    {
        VIGIL_LOG_DEBUG("SortSession: computeOperations() ignored: no algorithm set");
        return fail(ComputeError::NoAlgorithm);
    }

    if (m_DataSet.empty())
    {
        VIGIL_LOG_DEBUG("SortSession: computeOperations() ignored: dataset is empty");
        return fail(ComputeError::NoData);
    }

    const bool unlimited = (budget == std::chrono::microseconds::zero());

    // Blocking calls always re-check the safe limit, whether starting fresh or
    // force-completing a computation that was left in progress by a prior
    // budgeted call - resuming unlimited must never bypass this guard.
    if (unlimited && m_DataSet.size() > DataSetConfig::kQuadraticSafeLimit)
    {
        // An expected situation (the UI can check kQuadraticSafeLimit up front), not a bug: WARN.
        VIGIL_LOG_WARN("SortSession: data size ({}) exceeds the safe limit for blocking runs ({}). "
                       "Reduce the dataset size or pass a budget to computeOperations().",
                       m_DataSet.size(), DataSetConfig::kQuadraticSafeLimit);
        return fail(ComputeError::TooLargeForBlocking);
    }

    // Starting a fresh computation (not resuming a budgeted one already in progress).
    if (!m_ComputeInProgress)
    {
        VIGIL_LOG_DEBUG("SortSession: Computing operations for algorithm '{}' on {} elements",
                        getAlgorithmName(), m_DataSet.size());

        clearOperations();

        try
        {
            // The algorithm copies the data into its own working buffer, so the DataSet
            // stays in its shuffled state.
            m_Algorithm->load(m_DataSet.values());
        }
        catch (const std::exception& e)
        {
            VIGIL_LOG_ERROR("SortSession: Algorithm '{}' threw an exception while loading: {}", getAlgorithmName(), e.what());
            clearOperations();
            return fail(ComputeError::AlgorithmThrew);
        }

        m_ComputeInProgress = true;
    }

    const auto t0 = std::chrono::steady_clock::now();

    try
    {
        SortRecorder recorder(m_Statistics);

        for (std::size_t steps = 0; ; ++steps)
        {
            // Never check on the first step, so every call makes progress even with a tiny budget.
            if (!unlimited && steps != 0 && steps % kStepsPerClockCheck == 0
                && (std::chrono::steady_clock::now() - t0) >= budget)
                break; // Budget exhausted; resume on the next call.

            auto op = m_Algorithm->step(recorder);

            if (!op.has_value())
            {
                m_ComputeInProgress = false;
                break;
            }

            m_Operations.push_back(*op);
        }
    }
    catch (const std::exception& e)
    {
        VIGIL_LOG_ERROR("SortSession: Algorithm '{}' threw an exception while computing operations: {}. "
                        "Discarding partially recorded operations.",
                        getAlgorithmName(), e.what());
        clearOperations();
        return fail(ComputeError::AlgorithmThrew);
    }

    const auto t1 = std::chrono::steady_clock::now();
    m_Statistics.computeTimeMs += std::chrono::duration<double, std::milli>(t1 - t0).count();

    // Reaching this point means this call did not fail, even if an earlier call was refused.
    m_LastComputeError = ComputeError::None;

    if (m_ComputeInProgress)
    {
        VIGIL_LOG_TRACE("SortSession: Computation yielded after budget; {} operations so far", m_Operations.size());
        return m_LastComputeStatus = ComputeStatus::InProgress;
    }

    VIGIL_LOG_INFO("SortSession: Generated {} operations in {:.2f} ms", m_Operations.size(), m_Statistics.computeTimeMs);
    return m_LastComputeStatus = ComputeStatus::Complete;
}

void SortSession::clearOperations()
{
    if (!m_Operations.empty())
    {
        VIGIL_LOG_TRACE("SortSession: Cleared {} operations", m_Operations.size());
        m_Operations.clear();
        // O(n^2) runs can hold hundreds of MB; clear() alone would keep that capacity forever.
        m_Operations.shrink_to_fit();
    }

    m_Statistics.reset();
    m_ComputeInProgress = false;
    m_LastComputeStatus = ComputeStatus::Complete;
    m_LastComputeError  = ComputeError::None;

    // The loaded playback replays operations that no longer apply.
    m_Playback.clear();
}

bool SortSession::loadPlayback()
{
    if (m_ComputeInProgress || m_Operations.empty())
        return false;

    // Playback starts from the session's dataset - the exact data the operations were computed against.
    return m_Playback.load(m_DataSet.values(), m_Operations);
}

bool SortSession::preparePlayback()
{
    // Blocking (unlimited-budget) computation - always runs to completion or fails outright.
    if (computeOperations() != ComputeStatus::Complete)
        return false;

    return loadPlayback();
}

} // namespace Sortly
