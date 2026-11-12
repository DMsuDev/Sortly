// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "Sortly/Gui/Theme/ThemeManager.hpp"
#include "Sortly/Gui/Scaling.hpp"

#include <vigil/logging/log_system.h>

#include <algorithm>
#include <charconv>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <system_error>

namespace Sortly::ui {

namespace {

using Json = nlohmann::json;
using StyleValue = std::variant<float, ImVec2>;

std::map<std::string, Json> s_themes;
std::map<std::string, ThemeManager::ThemeHandler> s_themeHandlers;
std::map<std::string, ThemeManager::StyleHandler> s_styleHandlers;

// What every handler held when it was registered: the library's own defaults. Each theme
// change starts from them, so a theme never inherits what the previous one set.
std::map<std::string, std::map<std::string, ImVec4>> s_nativeColors;
std::map<std::string, std::map<std::string, StyleValue>> s_nativeStyles;

std::string s_current;
std::optional<std::string> s_pending;

float scaleOf(const ThemeManager::Style& style)
{
    return style.needsScaling ? scaled(1.0f) : 1.0f;
}

// Design pixels -> clamped to the style's range -> scaled to the display.
void writeFloat(const ThemeManager::Style& style, float* target, float design)
{
    *target = std::clamp(design, style.min, style.max) * scaleOf(style);
}

void writeVector(const ThemeManager::Style& style, ImVec2* target, const ImVec2& design)
{
    const float scale = scaleOf(style);
    *target = ImVec2(std::clamp(design.x, style.min, style.max) * scale,
                     std::clamp(design.y, style.min, style.max) * scale);
}

void restoreNative()
{
    for (const auto& [section, values] : s_nativeColors)
    {
        const auto handler = s_themeHandlers.find(section);
        if (handler == s_themeHandlers.end())
            continue;

        for (const auto& [key, color] : values)
            handler->second.setFunction(handler->second.colorMap.at(key), color);
    }

    for (const auto& [section, values] : s_nativeStyles)
    {
        const auto handler = s_styleHandlers.find(section);
        if (handler == s_styleHandlers.end())
            continue;

        for (const auto& [key, native] : values)
        {
            const ThemeManager::Style& style = handler->second.styleMap.at(key);
            const float scale = scaleOf(style);

            if (const auto* scalar = std::get_if<float*>(&style.value))
            {
                **scalar = std::get<float>(native) * scale;
            }
            else if (const auto* vector = std::get_if<ImVec2*>(&style.value))
            {
                const ImVec2& design = std::get<ImVec2>(native);
                **vector = ImVec2(design.x * scale, design.y * scale);
            }
        }
    }
}

void applyColors(const std::string& theme, const Json& colorsJson)
{
    for (const auto& [section, content] : colorsJson.items())
    {
        const auto handler = s_themeHandlers.find(section);
        if (handler == s_themeHandlers.end())
        {
            VIGIL_LOG_WARN("Theme '{}': no color handler for '{}'", theme, section);
            continue;
        }
        if (!content.is_object())
            continue;

        for (const auto& [key, value] : content.items())
        {
            const auto id = handler->second.colorMap.find(key);
            if (id == handler->second.colorMap.end())
            {
                VIGIL_LOG_WARN("Theme '{}': unknown color '{}.{}'", theme, section, key);
                continue;
            }

            std::optional<ImVec4> color;
            if (value.is_string())
                color = ThemeManager::parseColorString(value.get<std::string>());

            if (!color)
            {
                VIGIL_LOG_WARN("Theme '{}': invalid color for '{}.{}'", theme, section, key);
                continue;
            }
            handler->second.setFunction(id->second, *color);
        }
    }
}

void applyStyles(const std::string& theme, const Json& stylesJson)
{
    for (const auto& [section, content] : stylesJson.items())
    {
        const auto handler = s_styleHandlers.find(section);
        if (handler == s_styleHandlers.end())
        {
            VIGIL_LOG_WARN("Theme '{}': no style handler for '{}'", theme, section);
            continue;
        }
        if (!content.is_object())
            continue;

        for (const auto& [key, value] : content.items())
        {
            const auto found = handler->second.styleMap.find(key);
            if (found == handler->second.styleMap.end())
            {
                VIGIL_LOG_WARN("Theme '{}': unknown style '{}.{}'", theme, section, key);
                continue;
            }
            const ThemeManager::Style& style = found->second;

            if (value.is_number())
            {
                if (const auto* scalar = std::get_if<float*>(&style.value))
                    writeFloat(style, *scalar, value.get<float>());
                else
                    VIGIL_LOG_WARN("Theme '{}': '{}.{}' expects two numbers", theme, section, key);
            }
            else if (value.is_array() && value.size() == 2 && value[0].is_number() && value[1].is_number())
            {
                if (const auto* vector = std::get_if<ImVec2*>(&style.value))
                    writeVector(style, *vector, ImVec2(value[0].get<float>(), value[1].get<float>()));
                else
                    VIGIL_LOG_WARN("Theme '{}': '{}.{}' expects one number", theme, section, key);
            }
            else
            {
                VIGIL_LOG_WARN("Theme '{}': invalid value for '{}.{}'", theme, section, key);
            }
        }
    }
}

// The base goes first, so the theme only has to state what differs from it.
void applyWithBase(const std::string& name, std::vector<std::string>& chain)
{
    if (std::ranges::find(chain, name) != chain.end())
    {
        VIGIL_LOG_ERROR("Theme '{}' is part of a base cycle", name);
        return;
    }

    const auto found = s_themes.find(name);
    if (found == s_themes.end())
    {
        VIGIL_LOG_ERROR("Base theme '{}' does not exist", name);
        return;
    }
    chain.push_back(name);

    const Json& theme = found->second;

    if (const auto base = theme.find("base"); base != theme.end())
    {
        if (base->is_string())
            applyWithBase(base->get<std::string>(), chain);
        else
            VIGIL_LOG_ERROR("Theme '{}' has an invalid base", name);
    }

    if (const auto colorsJson = theme.find("colors"); colorsJson != theme.end() && colorsJson->is_object())
        applyColors(name, *colorsJson);

    if (const auto stylesJson = theme.find("styles"); stylesJson != theme.end() && stylesJson->is_object())
        applyStyles(name, *stylesJson);
}

} // namespace

// ================================================
// REGISTRATION
// ================================================

void ThemeManager::addThemeHandler(const std::string& name, const ColorMap& colorMap,
                                   const std::function<ImVec4(std::uint32_t)>& getFunction,
                                   const std::function<void(std::uint32_t, ImVec4)>& setFunction)
{
    s_themeHandlers[name] = { colorMap, getFunction, setFunction };

    auto& native = s_nativeColors[name];
    native.clear();
    for (const auto& [key, id] : colorMap)
        native[key] = getFunction(id);
}

void ThemeManager::addStyleHandler(const std::string& name, const StyleMap& styleMap)
{
    s_styleHandlers[name] = { styleMap };

    auto& native = s_nativeStyles[name];
    native.clear();
    for (const auto& [key, style] : styleMap)
    {
        if (const auto* scalar = std::get_if<float*>(&style.value))
            native[key] = **scalar;
        else if (const auto* vector = std::get_if<ImVec2*>(&style.value))
            native[key] = **vector;
    }
}

void ThemeManager::clearHandlers()
{
    s_themeHandlers.clear();
    s_styleHandlers.clear();
    s_nativeColors.clear();
    s_nativeStyles.clear();
}

// ================================================
// THEMES
// ================================================

bool ThemeManager::addTheme(const std::string& content)
{
    Json theme = Json::parse(content, nullptr, false);
    if (!theme.is_object() || !theme.contains("name") || !theme["name"].is_string())
    {
        VIGIL_LOG_ERROR("Invalid theme: it must be a JSON object with a \"name\"");
        return false;
    }

    const std::string name = theme["name"].get<std::string>();
    s_themes[name] = std::move(theme);
    return true;
}

std::size_t ThemeManager::loadDirectory(const std::filesystem::path& dir)
{
    namespace fs = std::filesystem;

    std::error_code ec;
    std::vector<fs::path> files;
    for (const auto& entry : fs::directory_iterator(dir, ec))
        if (entry.is_regular_file(ec) && entry.path().extension() == ".json")
            files.push_back(entry.path());
    std::ranges::sort(files);

    std::size_t loaded = 0;
    for (const fs::path& file : files)
    {
        std::ifstream in(file);
        std::ostringstream content;
        content << in.rdbuf();

        if (addTheme(content.str()))
            ++loaded;
        else
            VIGIL_LOG_WARN("Theme file '{}' skipped", file.filename().string());
    }

    VIGIL_LOG_INFO("Loaded {} theme(s) from '{}'", loaded, dir.string());
    return loaded;
}

bool ThemeManager::changeTheme(const std::string& name)
{
    if (!s_themes.contains(name))
    {
        VIGIL_LOG_ERROR("Theme '{}' does not exist", name);
        return false;
    }

    restoreNative();

    std::vector<std::string> chain;
    applyWithBase(name, chain);

    s_current = name;
    VIGIL_LOG_DEBUG("Theme applied: {}", name);
    return true;
}

void ThemeManager::reapplyCurrentTheme()
{
    if (!s_current.empty())
        changeTheme(s_current);
}

void ThemeManager::requestChange(const std::string& name)
{
    s_pending = name;
}

void ThemeManager::requestNext()
{
    const std::vector<std::string> names = getThemeNames();
    if (names.size() < 2)
        return;

    const auto current = std::ranges::find(names, s_current);
    const std::size_t next = current == names.end() ? 0 : static_cast<std::size_t>(current - names.begin()) + 1;
    s_pending = names[next % names.size()];
}

void ThemeManager::update()
{
    if (!s_pending)
        return;

    const std::string name = std::move(*s_pending);
    s_pending.reset();
    changeTheme(name);
}

// ================================================
// QUERIES / EXPORT
// ================================================

std::vector<std::string> ThemeManager::getThemeNames()
{
    std::vector<std::string> names;
    for (const auto& [name, theme] : s_themes)
        names.push_back(name);
    return names;
}

std::size_t ThemeManager::getThemeCount()
{
    return s_themes.size();
}

const std::string& ThemeManager::getCurrentThemeName()
{
    return s_current;
}

Json ThemeManager::exportCurrentTheme(const std::string& name)
{
    Json theme = Json::object();
    theme["name"] = name;
    theme["colors"] = Json::object();
    theme["styles"] = Json::object();

    for (const auto& [section, handler] : s_themeHandlers)
        for (const auto& [key, id] : handler.colorMap)
            theme["colors"][section][key] = colorToString(handler.getFunction(id));

    const float scale = scaled(1.0f);
    for (const auto& [section, handler] : s_styleHandlers)
    {
        for (const auto& [key, style] : handler.styleMap)
        {
            const float divisor = style.needsScaling ? scale : 1.0f;

            if (const auto* scalar = std::get_if<float*>(&style.value))
                theme["styles"][section][key] = **scalar / divisor;
            else if (const auto* vector = std::get_if<ImVec2*>(&style.value))
                theme["styles"][section][key] = { (*vector)->x / divisor, (*vector)->y / divisor };
        }
    }

    return theme;
}

std::optional<ImVec4> ThemeManager::parseColorString(const std::string& text)
{
    if (text == "auto")
        return ImVec4(0.0f, 0.0f, 0.0f, -1.0f);

    if ((text.size() != 7 && text.size() != 9) || text.front() != '#')
        return std::nullopt;

    std::uint32_t rgba = 0;
    const char* const first = text.data() + 1;
    const char* const last = text.data() + text.size();
    const auto [end, error] = std::from_chars(first, last, rgba, 16);
    if (error != std::errc{} || end != last)
        return std::nullopt;

    if (text.size() == 7)
        rgba = (rgba << 8) | 0xFFu;

    const auto channel = [rgba](unsigned shift) { return static_cast<float>((rgba >> shift) & 0xFFu) / 255.0f; };
    return ImVec4(channel(24), channel(16), channel(8), channel(0));
}

std::string ThemeManager::colorToString(const ImVec4& color)
{
    if (color.w < 0.0f)
        return "auto";

    const auto byte = [](float value) { return static_cast<unsigned>(std::clamp(value, 0.0f, 1.0f) * 255.0f + 0.5f); };

    char buffer[10];
    std::snprintf(buffer, sizeof(buffer), "#%02X%02X%02X%02X", byte(color.x), byte(color.y), byte(color.z), byte(color.w));
    return buffer;
}

const std::map<std::string, ThemeManager::ThemeHandler>& ThemeManager::getThemeHandlers()
{
    return s_themeHandlers;
}

const std::map<std::string, ThemeManager::StyleHandler>& ThemeManager::getStyleHandlers()
{
    return s_styleHandlers;
}

} // namespace Sortly::ui
