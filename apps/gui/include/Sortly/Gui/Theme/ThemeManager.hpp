// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#pragma once

#include <imgui.h>
#include <nlohmann/json.hpp>

#include <cstdint>
#include <filesystem>
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace Sortly::ui {

/// @brief Loads, applies and exports themes.
class ThemeManager
{
public:
    using ColorMap = std::map<std::string, std::uint32_t>;

    struct Style
    {
        std::variant<ImVec2*, float*> value;
        float min{0.0f};
        float max{1.0f};
        bool needsScaling{false};
    };
    using StyleMap = std::map<std::string, Style>;

    struct ThemeHandler
    {
        ColorMap colorMap;
        std::function<ImVec4(std::uint32_t)> getFunction;
        std::function<void(std::uint32_t, ImVec4)> setFunction;
    };

    struct StyleHandler
    {
        StyleMap styleMap;
    };

    // --- Registration ---------------------------------------------------------

    /// @brief Registers the colors of a library. Its current values become its "native" ones.
    static void addThemeHandler(const std::string& name, const ColorMap& colors,
                                const std::function<ImVec4(std::uint32_t)>& getter,
                                const std::function<void(std::uint32_t, ImVec4)>& setter);

    /// @brief Registers the styles of a library. Its current values become its "native" ones.
    static void addStyleHandler(const std::string& name, const StyleMap& styleMap);

    /// @brief Forgets every handler (their pointers die with the library contexts).
    static void clearHandlers();

    // --- Themes ---------------------------------------------------------------

    /// @brief Adds a theme from JSON text. Replaces one with the same name.
    /// @return False if the JSON is not an object with a "name".
    static bool addTheme(const std::string& content);

    /// @brief Adds every *.json of @p dir. Returns how many were added.
    static std::size_t loadDirectory(const std::filesystem::path& dir);

    /// @brief Applies a theme right now. Only call it outside a frame.
    /// @return False (nothing changes) if no theme has that name.
    static bool changeTheme(const std::string& name);

    /// @brief Applies the current theme again (e.g. after the display scale changed).
    static void reapplyCurrentTheme();

    /// @brief Asks for a theme; the next update() applies it.
    static void requestChange(const std::string& name);
    static void requestNext();

    /// @brief Applies a pending request. Call once per frame, before beginFrame().
    static void update();

    // --- Queries / export -----------------------------------------------------

    [[nodiscard]] static std::vector<std::string> getThemeNames();
    [[nodiscard]] static std::size_t getThemeCount();
    [[nodiscard]] static const std::string& getCurrentThemeName();

    /// @brief Current colors and styles as theme JSON (sizes are stored unscaled).
    [[nodiscard]] static nlohmann::json exportCurrentTheme(const std::string& name);

    /// @brief "#RRGGBB", "#RRGGBBAA" or "auto" (ImPlot's automatic color).
    [[nodiscard]] static std::optional<ImVec4> parseColorString(const std::string& text);
    [[nodiscard]] static std::string colorToString(const ImVec4& color);

    [[nodiscard]] static const std::map<std::string, ThemeHandler>& getThemeHandlers();
    [[nodiscard]] static const std::map<std::string, StyleHandler>& getStyleHandlers();

private:
    ThemeManager() = default;
};

/// @brief Registers the ImGui, ImPlot and Sortly handlers with the ThemeManager.
void registerThemeHandlers();

} // namespace Sortly::ui
