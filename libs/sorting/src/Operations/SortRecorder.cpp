// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "Sortly/Sorting/Operations/SortRecorder.hpp"

#include <cassert>
#include <utility>

namespace Sortly {

namespace {

// Operations store indices as int so -1 can mean "not applicable"; convert once, here.
constexpr int toIndex(std::size_t index) noexcept
{
    return static_cast<int>(index);
}

} // namespace

SortOperation SortRecorder::record(SortOperation op)
{
    ++m_Statistics.operationCount;
    return op;
}

SortOperation SortRecorder::compare(std::span<const int> buffer, std::size_t i, std::size_t j)
{
    assert(i < buffer.size() && j < buffer.size());

    ++m_Statistics.comparisons;
    m_Statistics.reads += 2;

    return record(SortOperation{
        .type = SortOperation::Type::Compare,
        .indexA = toIndex(i), .indexB = toIndex(j),
        .valueA = buffer[i], .valueB = buffer[j]
    });
}

SortOperation SortRecorder::swap(std::span<int> buffer, std::size_t i, std::size_t j)
{
    assert(i < buffer.size() && j < buffer.size());

    const int valueA = buffer[i];
    const int valueB = buffer[j];
    std::swap(buffer[i], buffer[j]);

    ++m_Statistics.swaps;
    m_Statistics.reads  += 2;
    m_Statistics.writes += 2;

    return record(SortOperation{
        .type = SortOperation::Type::Swap,
        .indexA = toIndex(i), .indexB = toIndex(j),
        .valueA = valueA, .valueB = valueB
    });
}

SortOperation SortRecorder::overwrite(std::span<int> buffer, std::size_t index, int newValue)
{
    assert(index < buffer.size());

    const int oldValue = buffer[index];
    buffer[index] = newValue;

    ++m_Statistics.reads;
    ++m_Statistics.writes;

    return record(SortOperation{
        .type = SortOperation::Type::Overwrite,
        .indexA = toIndex(index),
        .valueA = newValue,
        .valueB = oldValue
    });
}

SortOperation SortRecorder::pivot(std::span<const int> buffer, std::size_t index)
{
    assert(index < buffer.size());

    // Like every other operation, the value comes from the buffer and the read is counted.
    ++m_Statistics.reads;

    return record(SortOperation{
        .type = SortOperation::Type::Pivot,
        .indexA = toIndex(index),
        .valueA = buffer[index]
    });
}

SortOperation SortRecorder::clearPivot()
{
    // indexA stays at -1: "no pivot".
    return record(SortOperation{ .type = SortOperation::Type::Pivot });
}

SortOperation SortRecorder::sorted()
{
    return record(SortOperation{ .type = SortOperation::Type::Sorted });
}

} // namespace Sortly
