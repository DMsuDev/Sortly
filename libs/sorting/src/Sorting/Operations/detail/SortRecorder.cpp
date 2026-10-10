// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "Sortly/Sorting/Operations/detail/SortRecorder.hpp"

namespace Sortly {

SortOperation SortRecorder::record(SortOperation op)
{
    ++m_Statistics.operationCount;
    return op;
}

SortOperation SortRecorder::compare(int indexA, int indexB, int valueA, int valueB)
{
    ++m_Statistics.comparisons;
    m_Statistics.reads += 2;

    return record(SortOperation{
        .type = SortOperation::Type::Compare,
        .indexA = indexA, .indexB = indexB,
        .valueA = valueA, .valueB = valueB
    });
}

SortOperation SortRecorder::swap(int indexA, int indexB, int valueA, int valueB)
{
    ++m_Statistics.swaps;
    m_Statistics.reads  += 2;
    m_Statistics.writes += 2;

    return record(SortOperation{
        .type = SortOperation::Type::Swap,
        .indexA = indexA, .indexB = indexB,
        .valueA = valueA, .valueB = valueB
    });
}

SortOperation SortRecorder::overwrite(std::span<int> buffer, int index, int newValue)
{
    const int oldValue = buffer[index];
    buffer[index] = newValue;

    ++m_Statistics.reads;
    ++m_Statistics.writes;

    return record(SortOperation{
        .type = SortOperation::Type::Overwrite,
        .indexA = index,
        .valueA = newValue,
        .valueB = oldValue
    });
}

SortOperation SortRecorder::pivot(int index, int value)
{
    return record(SortOperation{
        .type = SortOperation::Type::Pivot,
        .indexA = index,
        .valueA = value
    });
}

SortOperation SortRecorder::sorted()
{
    return record(SortOperation{ .type = SortOperation::Type::Sorted });
}

} // namespace Sortly
