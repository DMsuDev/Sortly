// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "Sortly/Sorting/Data/SeedProvider.hpp"

namespace Sortly {

SeedProvider SeedProvider::globalSeed(uint32_t seed)
{
    return SeedProvider(Mode::GlobalSeed, seed);
}

SeedProvider SeedProvider::perSession(uint32_t baseSeed)
{
    return SeedProvider(Mode::PerSessionSeed, baseSeed);
}

SeedProvider SeedProvider::random()
{
    return SeedProvider(Mode::Random, std::random_device{}());
}

uint32_t SeedProvider::seedFor(std::size_t sessionIndex) const
{
    switch (m_Mode)
    {
        case Mode::GlobalSeed:
            return m_BaseSeed;

        case Mode::PerSessionSeed:
        {
            // Combine the base seed with the session index into a distinct
            // but reproducible seed (same base + index always derive the same seed).
            std::seed_seq seq{ m_BaseSeed, static_cast<uint32_t>(sessionIndex) };
            uint32_t derived = 0;
            seq.generate(&derived, &derived + 1);
            return derived;
        }

        case Mode::Random:
            return static_cast<uint32_t>(std::random_device{}());
    }

    return m_BaseSeed;
}

std::mt19937 SeedProvider::createEngine(std::size_t sessionIndex) const
{
    return std::mt19937{ seedFor(sessionIndex) };
}

} // namespace Sortly
