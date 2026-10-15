// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "Sortly/Gui/RunSettings.hpp"

#include <algorithm>

namespace Sortly {

RunSettings::RunSettings()
{
    for (const ViewMode mode : { ViewMode::Compare4, ViewMode::Compare3, ViewMode::Compare2 })
    {
        if (isViewAvailable(mode))
        {
            setView(mode);
            return;
        }
    }
    setView(ViewMode::Single);
}

bool RunSettings::isViewAvailable(ViewMode mode)
{
    return slotCount(mode) <= SortAlgorithmFactory::all().size();
}

bool RunSettings::isSelected(SortAlgorithmType type) const
{
    return std::find(algorithms.begin(), algorithms.end(), type) != algorithms.end();
}

bool RunSettings::toggle(SortAlgorithmType type)
{
    const auto found = std::find(algorithms.begin(), algorithms.end(), type);
    if (found != algorithms.end())
    {
        // Single view always keeps one algorithm; clicking the current one changes nothing.
        if (capacity() == 1)
            return false;
        algorithms.erase(found);
        return true;
    }

    if (algorithms.size() < capacity())
    {
        algorithms.push_back(type);
        return true;
    }

    if (capacity() == 1)
    {
        algorithms.front() = type;
        return true;
    }
    return false;
}

void RunSettings::setView(ViewMode mode)
{
    if (!isViewAvailable(mode))
        return;

    view = mode;
    if (algorithms.size() > capacity())
        algorithms.resize(capacity());

    for (const auto& info : SortAlgorithmFactory::all())
    {
        if (algorithms.size() >= capacity())
            break;
        if (!isSelected(info.type))
            algorithms.push_back(info.type);
    }
}

} // namespace Sortly
