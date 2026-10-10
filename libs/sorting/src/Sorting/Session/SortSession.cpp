// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "Sortly/Sorting/Session/SortSession.hpp"
#include "Sortly/Sorting/Data/DataSetBuilder.hpp"
#include "Sortly/Sorting/Operations/detail/SortRecorder.hpp"

#include <vigil/vigil.h>

#include <algorithm>
#include <chrono>
#include <stdexcept>

namespace Sortly {

//================================================//
//            CONSTRUCTORS / ASSIGNMENT
//================================================//

SortSession::SortSession(SortSession&& other) noexcept
    : m_SessionId(other.m_SessionId)
    , m_DataSet(std::move(other.m_DataSet))
    , m_Config(std::move(other.m_Config))
    , m_Algorithm(std::move(other.m_Algorithm))
    , m_Operations(std::move(other.m_Operations))
    , m_WorkBuffer(std::move(other.m_WorkBuffer))
    , m_Statistics(other.m_Statistics)
    , m_Playback(std::move(other.m_Playback))
    , m_LastComputeStatus(other.m_LastComputeStatus)
    , m_ComputeInProgress(other.m_ComputeInProgress)
{
    // SortPlayback stores a raw pointer into m_DataSet; rebind after the move.
    m_Playback.bind(m_DataSet.mutableValues());
}

SortSession& SortSession::operator=(SortSession&& other) noexcept
{
    if (this == &other)
        return *this;

    m_SessionId = other.m_SessionId;
    m_DataSet = std::move(other.m_DataSet);
    m_Config = std::move(other.m_Config);
    m_Algorithm = std::move(other.m_Algorithm);
    m_Operations = std::move(other.m_Operations);
    m_WorkBuffer = std::move(other.m_WorkBuffer);
    m_Statistics = other.m_Statistics;
    m_Playback = std::move(other.m_Playback);
    m_LastComputeStatus = other.m_LastComputeStatus;
    m_ComputeInProgress = other.m_ComputeInProgress;

    m_Playback.bind(m_DataSet.mutableValues());

    return *this;
}

//================================================//
//               CONFIGURATION
//================================================//

void SortSession::setDataSetConfig(const DataSetConfig& config)
{
    m_Config = std::make_unique<DataSetConfig>(config);
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
    if (!m_Config)
    {
        VIGIL_LOG_WARN("SortSession: No dataset configuration provided. Creating default configuration.");
        setDataSetConfig(DataSetConfig{});
    }

    // Clamp the count to the configured min/max range before building the dataset.
    // This is an early sanity check to avoid generating an empty dataset or exceeding the max count.
    int clampedCount = std::clamp(m_Config->count, m_Config->minCount, m_Config->maxCount);
    if (clampedCount <= 0)
        return false;

    DataSet newDataSet;
    try {
        newDataSet = DataSetBuilder::buildShuffled(*m_Config, rng);
    }
    catch (const std::exception& e) {
        VIGIL_LOG_ERROR("SortSession: Failed to build dataset (count={}): {}", clampedCount, e.what());
        return false;
    }

    clearOperations();
    m_DataSet = std::move(newDataSet);
    m_Playback.bind(m_DataSet.mutableValues());

    return true;
}

void SortSession::shuffleData(std::mt19937& rng)
{
    DataSetBuilder::shuffle(m_DataSet, rng);
    m_Playback.bind(m_DataSet.mutableValues());
    clearOperations();

    VIGIL_LOG_DEBUG("SortSession: Shuffled data ({} elements)", m_DataSet.size());
}

//================================================//
//              OPERATION MANAGEMENT
//================================================//

SortSession::ComputeStatus SortSession::computeOperations(std::chrono::microseconds budget)
{
    if (!m_Algorithm || !m_Config)
    {
        VIGIL_LOG_WARN("SortSession: Cannot compute operations. Algorithm or dataset configuration is not set.");
        return m_LastComputeStatus = ComputeStatus::Failed;
    }

    const bool unlimited = (budget == std::chrono::microseconds::zero());

    // Blocking calls always re-check the safe limit, whether starting fresh or
    // force-completing a computation that was left in progress by a prior
    // budgeted call - resuming unlimited must never bypass this guard.
    if (unlimited && m_DataSet.size() > DataSetConfig::kQuadraticSafeLimit)
    {
        VIGIL_LOG_ERROR("SortSession: data size ({}) exceeds safe generation limit ({}). "
                    "Reduce the dataset size, choose a different algorithm, or pass a "
                    "budget to computeOperations() to compute it cooperatively instead.",
                    m_DataSet.size(), DataSetConfig::kQuadraticSafeLimit);
        return m_LastComputeStatus = ComputeStatus::Failed;
    }

    // Starting a fresh computation (not resuming a budgeted one already in progress).
    if (!m_ComputeInProgress)
    {
        VIGIL_LOG_DEBUG("SortSession: Computing operations for algorithm '{}' on {} elements",
                  getAlgorithmName(), m_DataSet.size());

        clearOperations();

        // Work on a local copy so the DataSet stays in its shuffled state.
        // The SortPlayback will replay operations on top of the DataSet directly.
        m_WorkBuffer.assign(m_DataSet.values().begin(), m_DataSet.values().end());

        try
        {
            m_Algorithm->load(m_WorkBuffer);
        }
        catch (const std::exception& e)
        {
            VIGIL_LOG_ERROR("SortSession: Algorithm '{}' threw an exception while loading: {}", getAlgorithmName(), e.what());
            clearOperations();
            return m_LastComputeStatus = ComputeStatus::Failed;
        }

        m_ComputeInProgress = true;
    }

    const auto t0 = std::chrono::steady_clock::now();

    try
    {
        SortRecorder recorder(m_Statistics);

        while (true)
        {
            if (!unlimited && (std::chrono::steady_clock::now() - t0) >= budget)
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
        return m_LastComputeStatus = ComputeStatus::Failed;
    }

    const auto t1 = std::chrono::steady_clock::now();
    m_Statistics.computeTimeMs += std::chrono::duration<double, std::milli>(t1 - t0).count();

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
    }

    m_Statistics.reset();
    m_ComputeInProgress = false;
    m_LastComputeStatus = ComputeStatus::Complete;
}

bool SortSession::preparePlayback()
{
    // Blocking (unlimited-budget) computation - always runs to completion or fails outright.
    if (computeOperations() != ComputeStatus::Complete)
        return false;

    // Bind the visual buffer and load the operations into playback.
    // This insures that the playback is always in sync with the current dataset and operations.
    m_Playback.bind(m_DataSet.mutableValues());
    return m_Playback.loadOperations(m_Operations);
}

} // namespace Sortly
