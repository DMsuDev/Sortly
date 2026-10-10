// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#pragma once

#include "Sortly/Sorting/Data/DataSet.hpp"
#include "Sortly/Sorting/Data/DataSetConfig.hpp"

#include <random>

namespace Sortly {

/// @brief Generates and shuffles DataSet instances from a DataSetConfig.
///
/// Design:
/// - build() is deterministic and never uses randomness.
/// - shuffle() is the only operation that consumes the RNG.
/// - Use buildShuffled() to generate a randomized dataset in one step.
///
/// @note The random engine is supplied by the caller, allowing deterministic
/// tests with a fixed seed and custom seeding strategies.
class DataSetBuilder
{
public:
    /// @brief Builds a sorted dataset of count values in the range [1, maxValue], inclusive.
    /// @return An empty DataSet if the clamped count is <= 0.
    [[nodiscard]] static DataSet build(const DataSetConfig& config);

    /// @brief Builds a dataset and shuffles it in one step.
    [[nodiscard]] static DataSet buildShuffled(const DataSetConfig& config, std::mt19937& rng);

    /// @brief Randomizes the order of dataSet's values in place.
    static void shuffle(DataSet& dataSet, std::mt19937& rng);
};

} // namespace Sortly
