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

#include <cstddef>
#include <iterator>

namespace Sortly {

namespace {

template <class T>
std::unique_ptr<SortAlgorithm> make() { return std::make_unique<T>(); }

// Builds a registry entry from the algorithm's own `kMeta`, so metadata is never repeated.
template <class T>
constexpr AlgorithmInfo entry(SortAlgorithmType type)
{
    return { type, T::kMeta.name, T::kMeta.complexity, T::kMeta.description, &make<T> };
}

// The registry. Registering a new algorithm = one line here, in SortAlgorithmType order.
constexpr AlgorithmInfo kRegistry[] = {
    entry<BubbleSort>   (SortAlgorithmType::BubbleSort),
    entry<CocktailSort> (SortAlgorithmType::CocktailSort),
    entry<SelectionSort>(SortAlgorithmType::SelectionSort),
    entry<InsertionSort>(SortAlgorithmType::InsertionSort),
    entry<QuickSort>    (SortAlgorithmType::QuickSort),
    entry<MergeSort>    (SortAlgorithmType::MergeSort),
    entry<HeapSort>     (SortAlgorithmType::HeapSort),
    entry<ShellSort>    (SortAlgorithmType::ShellSort),
};

constexpr bool registryMatchesEnum()
{
    if (std::size(kRegistry) != static_cast<std::size_t>(SortAlgorithmType::Count))
        return false;

    for (std::size_t i = 0; i < std::size(kRegistry); ++i)
        if (static_cast<std::size_t>(kRegistry[i].type) != i)
            return false;

    return true;
}

static_assert(registryMatchesEnum(),
              "SortAlgorithmFactory registry must list every SortAlgorithmType (before Count), in enum order.");

} // namespace

std::span<const AlgorithmInfo> SortAlgorithmFactory::all()
{
    return kRegistry;
}

const AlgorithmInfo* SortAlgorithmFactory::find(SortAlgorithmType type)
{
    // The registry is indexed by type (static_assert above); negative values wrap to huge indices.
    const auto index = static_cast<std::size_t>(type);
    return index < std::size(kRegistry) ? &kRegistry[index] : nullptr;
}

std::unique_ptr<SortAlgorithm> SortAlgorithmFactory::create(SortAlgorithmType type)
{
    const auto* info = find(type);
    if (!info)
    {
        VIGIL_LOG_ERROR("SortAlgorithmFactory: algorithm type {} is not registered", static_cast<int>(type));
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
    types.reserve(std::size(kRegistry));
    for (const auto& info : kRegistry)
        types.push_back(info.type);
    return types;
}

} // namespace Sortly
