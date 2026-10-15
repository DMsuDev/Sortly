// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "Sortly/Sorting/Algorithms/SortAlgorithmFactory.hpp"

#include "BubbleSort.hpp"
#include "CocktailSort.hpp"
#include "HeapSort.hpp"
#include "InsertionSort.hpp"
#include "MergeSort.hpp"
#include "QuickSort.hpp"
#include "SelectionSort.hpp"
#include "ShellSort.hpp"

#include <vigil/vigil.h>

namespace Sortly {

namespace {

template <class T>
std::unique_ptr<SortAlgorithm> make() { return std::make_unique<T>(); }

} // namespace

std::span<const AlgorithmInfo> SortAlgorithmFactory::all()
{
    // Single source of truth: registering a new algorithm = one line here.
    static const AlgorithmInfo table[] = {
        { SortAlgorithmType::BubbleSort,   BubbleSort::kName,   &make<BubbleSort>   },
        { SortAlgorithmType::CocktailSort, CocktailSort::kName, &make<CocktailSort> },
        { SortAlgorithmType::SelectionSort, SelectionSort::kName, &make<SelectionSort> },
        { SortAlgorithmType::InsertionSort, InsertionSort::kName, &make<InsertionSort> },
        { SortAlgorithmType::QuickSort,     QuickSort::kName,     &make<QuickSort>     },
        { SortAlgorithmType::MergeSort,     MergeSort::kName,     &make<MergeSort>     },
        { SortAlgorithmType::HeapSort,      HeapSort::kName,      &make<HeapSort>      },
        { SortAlgorithmType::ShellSort,     ShellSort::kName,     &make<ShellSort>     },
    };
    return table;
}

const AlgorithmInfo* SortAlgorithmFactory::find(SortAlgorithmType type)
{
    for (const auto& info : all())
        if (info.type == type)
            return &info;

    return nullptr;
}

std::unique_ptr<SortAlgorithm> SortAlgorithmFactory::create(SortAlgorithmType type)
{
    const auto* info = find(type);
    if (!info)
    {
        VIGIL_LOG_ERROR("SortAlgorithmFactory: algorithm type {} is not yet implemented", static_cast<int>(type));
        return nullptr;
    }

    return info->create();
}

std::string_view SortAlgorithmFactory::toString(SortAlgorithmType type)
{
    const auto* info = find(type);
    return info ? info->name : "Unknown Algorithm";
}

std::vector<SortAlgorithmType> SortAlgorithmFactory::getAvailableAlgorithms()
{
    std::vector<SortAlgorithmType> types;
    for (const auto& info : all())
        types.push_back(info.type);
    return types;
}

} // namespace Sortly
