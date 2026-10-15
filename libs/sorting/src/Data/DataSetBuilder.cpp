// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "Sortly/Sorting/Data/DataSetBuilder.hpp"

#include <vigil/vigil.h>

#include <algorithm>

namespace Sortly {

DataSet DataSetBuilder::build(const DataSetConfig& config)
{
    const int count = config.effectiveCount();
    const int maxValue = config.effectiveMaxValue();

    if (count <= 0)
        return DataSet{};

    std::vector<int> values;
    values.reserve(static_cast<size_t>(count));

    if (count == 1)
    {
        // Special case: only one value, which must be the maximum.
        values.push_back(maxValue);
        return DataSet(std::move(values));
    }

    // Distribute values evenly across the range [1, maxValue].
    for (int i = 0; i < count; ++i)
    {
        const int value = 1 + static_cast<int>((static_cast<long long>(i) * (maxValue - 1)) / (count - 1));
        values.push_back(value);
    }

    VIGIL_LOG_TRACE("DataSetBuilder: Generated {} values (maxValue={})", count, maxValue);

    return DataSet(std::move(values));
}

DataSet DataSetBuilder::buildShuffled(const DataSetConfig& config, std::mt19937& rng)
{
    DataSet dataSet = build(config);
    shuffle(dataSet, rng);
    return dataSet;
}

void DataSetBuilder::shuffle(DataSet& dataSet, std::mt19937& rng)
{
    auto& values = dataSet.mutableValues();
    std::shuffle(values.begin(), values.end(), rng);

    VIGIL_LOG_TRACE("DataSetBuilder: Shuffled {} values", dataSet.size());
}

} // namespace Sortly
