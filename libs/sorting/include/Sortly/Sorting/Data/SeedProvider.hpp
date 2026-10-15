// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#pragma once

#include <cstddef>
#include <cstdint>
#include <random>

namespace Sortly {

// Provides the random engine each SortSession uses to build/shuffle its DataSet.
// This decouples "where randomness comes from" from dataset generation, so the
// application layer can switch between a single global seed and independent
// per-session seeds without touching Data/DataSetBuilder at all.
class SeedProvider
{
public:
    enum class Mode
    {
        GlobalSeed,     // Every session uses the exact same seed (identical datasets/shuffles).
        PerSessionSeed, // Each session derives its own reproducible seed from a shared base seed.
        Random          // Each session seeds itself from a non-deterministic source.
    };

    [[nodiscard]] static SeedProvider globalSeed(uint32_t seed);
    [[nodiscard]] static SeedProvider perSession(uint32_t baseSeed);
    [[nodiscard]] static SeedProvider random();

    // Returns the seed that would be used for the given session index.
    [[nodiscard]] uint32_t seedFor(std::size_t sessionIndex) const;

    // Builds a ready-to-use RNG engine for the given session index.
    [[nodiscard]] std::mt19937 createEngine(std::size_t sessionIndex) const;

private:
    SeedProvider(Mode mode, uint32_t baseSeed) : m_Mode(mode), m_BaseSeed(baseSeed) {}

    Mode m_Mode;
    uint32_t m_BaseSeed;
};

} // namespace Sortly
