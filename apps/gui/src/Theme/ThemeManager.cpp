// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "Sortly/Gui/Theme/ThemeManager.hpp"
#include "ThemeBindings.hpp"

#include <nlohmann/json.hpp>
#include <vigil/vigil.h>

#include <algorithm>
#include <fstream>
#include <string>
#include <vector>

namespace Sortly::ui::theme {

namespace {

namespace fs = std::filesystem;

struct Theme
{
    std::string    name;
    nlohmann::json data;
};

std::vector<Theme> g_Themes;
std::size_t        g_Current       = 0;
bool               g_NextRequested = false;

} // namespace

std::size_t loadDirectory(const fs::path& dir)
{
    g_Themes.clear();
    g_Current       = 0;
    g_NextRequested = false;

    std::error_code       ec;
    std::vector<fs::path> files;
    for (const auto& item : fs::directory_iterator(dir, ec))
        if (item.is_regular_file(ec) && item.path().extension() == ".json")
            files.push_back(item.path());
    std::ranges::sort(files);

    for (const fs::path& file : files)
    {
        std::ifstream  in(file);
        nlohmann::json data = nlohmann::json::parse(in, nullptr, false);
        if (!data.is_object())
        {
            VIGIL_LOG_WARN("Theme '{}' is not a valid JSON object: skipped", file.filename().string());
            continue;
        }
        g_Themes.push_back({ data.value("name", file.stem().string()), std::move(data) });
    }

    VIGIL_LOG_INFO("Loaded {} theme(s) from '{}'", g_Themes.size(), dir.string());
    return g_Themes.size();
}

bool apply(std::string_view name)
{
    const auto found = std::ranges::find_if(g_Themes, [name](const Theme& t) { return t.name == name; });
    if (found == g_Themes.end())
        return false;

    g_Current = static_cast<std::size_t>(found - g_Themes.begin());
    applyBindings(found->data);
    VIGIL_LOG_INFO("Theme applied: {}", found->name);
    return true;
}

void requestNext()
{
    g_NextRequested = g_Themes.size() > 1;
}

void update()
{
    if (!g_NextRequested)
        return;

    g_NextRequested = false;
    g_Current       = (g_Current + 1) % g_Themes.size();
    applyBindings(g_Themes[g_Current].data);
    VIGIL_LOG_INFO("Theme applied: {}", g_Themes[g_Current].name);
}

std::size_t count()
{
    return g_Themes.size();
}

} // namespace Sortly::ui::theme
