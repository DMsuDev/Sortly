// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#pragma once

#include "Sortly/Sorting/Algorithms/BubbleSort.hpp"
#include "Sortly/Sorting/Algorithms/CocktailSort.hpp"

#include "Sortly/Sorting/Algorithms/SortAlgorithmType.hpp"

#include <vigil/vigil.h>

#include <memory>
#include <string_view>

namespace Sortly {

class SortAlgorithmFactory
{
public:
    [[nodiscard]] static std::unique_ptr<SortAlgorithm> create(SortAlgorithmType type)
    {
        switch (type)
        {
            case SortAlgorithmType::BubbleSort:
                return std::make_unique<BubbleSort>();
            case SortAlgorithmType::CocktailSort:
                return std::make_unique<CocktailSort>();
            default:
                VIGIL_LOG_ERROR("SortAlgorithmFactory: algorithm type {} is not yet implemented", static_cast<int>(type));
                return nullptr;
        }
    }

    [[nodiscard]] static const std::string_view toString(SortAlgorithmType type)
    {
        switch (type)
        {
            case SortAlgorithmType::BubbleSort: return "Bubble Sort";
            case SortAlgorithmType::CocktailSort: return "Cocktail Sort";
            default:
                VIGIL_LOG_ERROR("SortAlgorithmFactory: algorithm type {} is not yet implemented", static_cast<int>(type));
                return "Unknown Algorithm";
        }
    }

    [[nodiscard]] static std::vector<SortAlgorithmType> getAvailableAlgorithms()
    {
        return {
            SortAlgorithmType::BubbleSort,
            SortAlgorithmType::CocktailSort
        };
    }
};

} // namespace Sortly
