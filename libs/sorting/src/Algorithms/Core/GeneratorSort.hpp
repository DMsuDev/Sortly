// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#pragma once

#include "Sortly/Sorting/Algorithms/SortAlgorithm.hpp"

#include "Step.hpp"
#include "StepGenerator.hpp"

#include <optional>
#include <span>
#include <string_view>
#include <vector>

namespace Sortly {

/// @brief Base for algorithms written as straight-line code that yields one Step per operation
/// (`co_yield Step::compare(i, j)`), instead of an explicit state machine.
///
/// Rules for run():
///  - Read m_Data freely, but change it ONLY through yielded Steps (swap / overwrite).
///  - Yield a compare before every decision based on two elements, so statistics stay honest.
///  - After a yield returns, the step has already been applied to m_Data.
///  - Do not emit the final Sorted operation: step() does it when run() ends.
///  - Indices are checked before each step; a bad one throws std::out_of_range, which
///    SortSession reports as ComputeError::AlgorithmThrew.
class GeneratorSort : public SortAlgorithm
{
public:
    void load(std::span<const int> data) final;

    std::optional<SortOperation> step(SortRecorder& recorder) final;

    [[nodiscard]] std::span<const int> getData() const final { return m_Data; }

protected:
    /// @brief The algorithm itself. Runs on m_Data, with m_Aux available as scratch space.
    virtual StepGenerator run() = 0;

    std::vector<int> m_Data; // The data being sorted.
    std::vector<int> m_Aux;  // Scratch buffer, same size as m_Data, initially a copy of it.

private:
    /// Throws std::out_of_range if @p step refers to an index outside its buffer.
    void validate(const Step& step) const;

    StepGenerator m_Generator;
    bool          m_Finished = false;
};

/// @brief CRTP helper that implements getName/getComplexity/getDescription from the derived
/// class's `static constexpr AlgorithmMeta kMeta`, so an algorithm declares its metadata once.
///
///     class FooSort final : public GeneratorSortOf<FooSort>
///     {
///     public:
///         static constexpr AlgorithmMeta kMeta{ "Foo Sort", "O(n²)", "One sentence." };
///     protected:
///         StepGenerator run() override;
///     };
template <class Derived>
class GeneratorSortOf : public GeneratorSort
{
public:
    [[nodiscard]] std::string_view getName() const final        { return Derived::kMeta.name; }
    [[nodiscard]] std::string_view getComplexity() const final  { return Derived::kMeta.complexity; }
    [[nodiscard]] std::string_view getDescription() const final { return Derived::kMeta.description; }
};

} // namespace Sortly
