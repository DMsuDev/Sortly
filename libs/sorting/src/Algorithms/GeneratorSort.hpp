// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#pragma once

#include "Sortly/Sorting/Algorithms/SortAlgorithm.hpp"

#include <coroutine>
#include <cstddef>
#include <exception>
#include <optional>
#include <span>
#include <utility>
#include <vector>

namespace Sortly {

/// @brief One thing an algorithm wants to do next. The algorithm only describes it; the base
/// class performs it through the SortRecorder, which keeps data, operations and statistics in sync.
struct Step
{
    enum class Kind { Compare, Swap, Overwrite, Pivot, ClearPivot };

    Kind        kind  = Kind::Compare;
    std::size_t a     = 0;
    std::size_t b     = 0;
    int         value = 0;     // Overwrite: the new value.
    bool        aux   = false; // Compare: read the values from the auxiliary buffer.

    static Step compare(std::size_t i, std::size_t j)    { return { Kind::Compare, i, j, 0, false }; }
    static Step compareAux(std::size_t i, std::size_t j) { return { Kind::Compare, i, j, 0, true }; }
    static Step swap(std::size_t i, std::size_t j)       { return { Kind::Swap, i, j, 0, false }; }
    static Step overwrite(std::size_t i, int newValue)   { return { Kind::Overwrite, i, 0, newValue, false }; }

    /// Marks m_Data[i] as the pivot until the next pivot()/clearPivot() (or the end of the run).
    static Step pivot(std::size_t i)                     { return { Kind::Pivot, i, 0, 0, false }; }

    /// Ends the current pivot mark. Yield it as soon as the pivot is done with.
    static Step clearPivot()                             { return { Kind::ClearPivot, 0, 0, 0, false }; }
};

/// @brief Minimal move-only generator of Steps (std::generator is not available everywhere).
class StepGenerator
{
public:
    struct promise_type
    {
        Step               current;
        std::exception_ptr error;

        StepGenerator get_return_object() { return StepGenerator(std::coroutine_handle<promise_type>::from_promise(*this)); }
        std::suspend_always initial_suspend() noexcept { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        std::suspend_always yield_value(Step step) noexcept { current = step; return {}; }
        void return_void() noexcept {}
        void unhandled_exception() { error = std::current_exception(); }
    };

    StepGenerator() = default;
    StepGenerator(StepGenerator&& other) noexcept : m_Handle(std::exchange(other.m_Handle, {})) {}
    StepGenerator& operator=(StepGenerator&& other) noexcept
    {
        if (this != &other)
        {
            reset();
            m_Handle = std::exchange(other.m_Handle, {});
        }
        return *this;
    }
    StepGenerator(const StepGenerator&)            = delete;
    StepGenerator& operator=(const StepGenerator&) = delete;
    ~StepGenerator() { reset(); }

    /// @brief Runs the algorithm up to its next Step. False once it has nothing left to do.
    bool next()
    {
        if (!m_Handle || m_Handle.done())
            return false;

        m_Handle.resume();
        if (m_Handle.promise().error)
            std::rethrow_exception(m_Handle.promise().error);
        return !m_Handle.done();
    }

    [[nodiscard]] const Step& value() const { return m_Handle.promise().current; }

private:
    explicit StepGenerator(std::coroutine_handle<promise_type> handle) : m_Handle(handle) {}

    void reset()
    {
        if (m_Handle)
            m_Handle.destroy();
        m_Handle = {};
    }

    std::coroutine_handle<promise_type> m_Handle;
};

/// @brief Base for algorithms written as straight-line code that yields a Step per operation
/// (co_yield Step::compare(i, j)), instead of an explicit state machine. After a yield the step
/// has already been applied to m_Data, so the algorithm can read the data as usual.
class GeneratorSort : public SortAlgorithm
{
public:
    void load(std::span<const int> data) final;

    std::optional<SortOperation> step(SortRecorder& recorder) final;

    [[nodiscard]] std::span<const int> getData() const final { return m_Data; }

protected:
    /// @brief The algorithm itself. Runs on m_Data (and m_Aux as scratch space).
    virtual StepGenerator run() = 0;

    std::vector<int> m_Data; // The data being sorted.
    std::vector<int> m_Aux;  // Scratch buffer, same size as m_Data; algorithms that need it fill it themselves.

private:
    StepGenerator m_Generator;
    bool          m_Finished = false;
};

} // namespace Sortly
