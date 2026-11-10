// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "Sortly/Gui/Theme/FontManager.hpp"
#include "Sortly/Gui/Scaling.hpp"
#include "Sortly/Gui/System.hpp"

#include <vigil/vigil.h>

#include <map>
#include <vector>

namespace Sortly::ui {

namespace {

// Vertical nudge of the merged icons, as a fraction of the text size. Tuned for the text
// font: raise it if the icons sit too high next to the text, lower it if they sit too low.
constexpr float kIconBaselineShift = 0.10f;

struct State
{
    std::map<std::string, FontManager::Font> fonts;
    std::map<std::string, ImFont*> baked;

    std::string iconFile;
    std::vector<ImWchar> iconRanges; // Pairs, zero-terminated. Must outlive the atlas build.
};

State& state()
{
    static State instance;
    return instance;
}

// ImGui expects UTF-8 paths. On Windows path::string() uses the ANSI code page,
// which breaks as soon as the install path has an accented character.
std::string resolve(const std::string& file)
{
    if (file.empty())
        return {};

    const auto found = System::findAsset(file);
    if (!found)
        return {};

    const auto u8 = found->u8string();
    return std::string(reinterpret_cast<const char*>(u8.data()), u8.size());
}

ImFont* bake(const std::string& textPath, const std::string& iconPath, const std::vector<ImWchar>& ranges,
             float size, float iconScale)
{
    ImGuiIO& io = ImGui::GetIO();

    ImFont* font = textPath.empty() ? nullptr : io.Fonts->AddFontFromFileTTF(textPath.c_str(), size);
    if (!font)
    {
        ImFontConfig fallback;
        fallback.SizePixels = size;
        font = io.Fonts->AddFontDefault(&fallback);
    }

    if (!iconPath.empty() && !ranges.empty())
    {
        const float iconSize = size * iconScale;
        ImFontConfig cfg;
        cfg.MergeMode = true;
        cfg.PixelSnapH = true;
        cfg.GlyphMinAdvanceX = iconSize;
        cfg.GlyphOffset.y = (size - iconSize) * 0.5f + size * kIconBaselineShift;
        io.Fonts->AddFontFromFileTTF(iconPath.c_str(), iconSize, &cfg, ranges.data());
    }

    return font;
}

} // namespace

void FontManager::setIcons(const std::string& file, std::span<const std::uint16_t> codepoints)
{
    State& s = state();
    s.iconFile = file;
    s.iconRanges.clear();
    for (const std::uint16_t codepoint : codepoints)
    {
        s.iconRanges.push_back(codepoint);
        s.iconRanges.push_back(codepoint);
    }
    s.iconRanges.push_back(0);
}

void FontManager::addFont(const std::string& name, const Font& font)
{
    state().fonts[name] = font;
}

bool FontManager::load()
{
    State& s = state();

    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->Clear();
    s.baked.clear();

    const std::string icons = resolve(s.iconFile);
    if (!s.iconFile.empty() && icons.empty())
        VIGIL_LOG_WARN("{} not found in assets: icons will be missing", s.iconFile);

    for (const auto& [name, font] : s.fonts)
    {
        std::string text = resolve(font.file);
        if (text.empty() && !font.fallback.empty())
        {
            VIGIL_LOG_WARN("{} not found in assets: using {} for font '{}'", font.file, font.fallback, name);
            text = resolve(font.fallback);
        }
        if (text.empty())
            VIGIL_LOG_WARN("No file found for font '{}': using the built-in bitmap font", name);

        ImFont* baked = bake(text, icons, s.iconRanges, scaled(font.size), font.iconScale);
        s.baked[name] = baked;
        if (font.target)
            *font.target = baked;
    }

    if (ImFont* body = get("body"))
        io.FontDefault = body;

    return !s.baked.empty();
}

ImFont* FontManager::get(const std::string& name)
{
    const State& s = state();
    const auto found = s.baked.find(name);
    return found != s.baked.end() ? found->second : nullptr;
}

void FontManager::clear()
{
    state() = State{};
}

} // namespace Sortly::ui
