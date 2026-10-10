// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#pragma once

#include <span>
#include <vector>

namespace Sortly {

// Owns the numeric values being sorted. It is a plain data holder - it never
// generates or shuffles its own content. That responsibility belongs to
// DataSetBuilder, so DataSet can be reused unchanged by any algorithm.
class DataSet
{
public:
    DataSet() = default;
    explicit DataSet(std::vector<int> values) : m_Values(std::move(values)) {}

    [[nodiscard]] std::span<const int> values() const { return m_Values; }

    [[nodiscard]] std::vector<int>& mutableValues() { return m_Values; }
    [[nodiscard]] const std::vector<int>& mutableValues() const { return m_Values; }

    [[nodiscard]] size_t size() const { return m_Values.size(); }
    [[nodiscard]] bool empty() const { return m_Values.empty(); }

    void clear() { m_Values.clear(); }

private:
    std::vector<int> m_Values;
};

} // namespace Sortly
