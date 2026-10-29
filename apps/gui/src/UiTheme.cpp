// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "Sortly/Gui/UiTheme.hpp"
#include "Sortly/Gui/Icons.hpp"

#include <foundry/platform/paths.h>
#include <vigil/vigil.h>

#include <filesystem>
#include <string>

namespace Sortly::ui {

namespace {

namespace fs = std::filesystem;

// Bundled in assets/fonts and copied next to the executable by apps/gui/CMakeLists.txt.
// Static TTF files only: variable fonts are not supported.
constexpr const char* kRegularFont  = "Inter-Regular.ttf";
constexpr const char* kSemiboldFont = "Inter-SemiBold.ttf";
constexpr const char* kIconFont     = "lucide.ttf";

// Vertical nudge of the merged icons, as a fraction of the text size. It is tuned for the text
// font: raise it if the icons sit too high next to the text, lower it if they sit too low.
// (Switching fonts is the usual reason to touch it.)
constexpr float kIconBaselineShift = 0.10f;

// ImGui expects UTF-8 paths. On Windows path::string() uses the ANSI code page,
// which breaks as soon as the install path has an accented character.
std::string toUtf8(const fs::path& path)
{
    const auto u8 = path.u8string();
    return std::string(reinterpret_cast<const char*>(u8.data()), u8.size());
}

// Looks for a bundled font next to the executable, then in the working directory.
// Returns an empty string when it is in neither place.
std::string findFontFile(const char* name)
{
    std::error_code ec;
    const fs::path fontsDir = fs::path("assets") / "fonts";

    const fs::path dirs[] = {
        foundry::platform::executable_directory(ec) / fontsDir,
        fontsDir,
    };

    for (const auto& dir : dirs)
    {
        const fs::path candidate = dir / name;
        if (fs::exists(candidate, ec))
            return toUtf8(candidate);
    }
    return {};
}

ImFont* addFont(const std::string& textFont, const std::string& iconFont, float size, float iconScale)
{
    ImGuiIO& io = ImGui::GetIO();

    ImFont* font = textFont.empty() ? nullptr : io.Fonts->AddFontFromFileTTF(textFont.c_str(), size);
    if (!font)
    {
        ImFontConfig fallback;
        fallback.SizePixels = size;
        font = io.Fonts->AddFontDefault(&fallback);
    }

    if (!iconFont.empty())
    {
        // The ranges must stay alive until the atlas is built.
        static ImVector<ImWchar> ranges;
        if (ranges.empty())
        {
            ImFontGlyphRangesBuilder builder;
            for (const unsigned short cp : kIconCodepoints)
                builder.AddChar(cp);
            builder.BuildRanges(&ranges);
        }

        const float iconSize = size * iconScale;
        ImFontConfig cfg;
        cfg.MergeMode        = true;
        cfg.PixelSnapH       = true;
        cfg.GlyphMinAdvanceX = iconSize;
        cfg.GlyphOffset.y    = (size - iconSize) * 0.5f + size * kIconBaselineShift;
        io.Fonts->AddFontFromFileTTF(iconFont.c_str(), iconSize, &cfg, ranges.Data);
    }

    return font;
}

} // namespace

ImVec4 operationColor(SortOperation::Type type)
{
    switch (type)
    {
        case SortOperation::Type::Compare:   return colors::Warning;
        case SortOperation::Type::Swap:
        case SortOperation::Type::Overwrite: return colors::Danger;
        case SortOperation::Type::Pivot:     return colors::Violet;
        case SortOperation::Type::Sorted:    return colors::Success;
    }
    return colors::Accent;
}

void loadFonts(float scale)
{
    g_Scale = scale > 0.0f ? scale : 1.0f;

    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->Clear();

    const std::string regular  = findFontFile(kRegularFont);
    std::string       semibold = findFontFile(kSemiboldFont);
    const std::string icons    = findFontFile(kIconFont);

    if (regular.empty())
    {
        VIGIL_LOG_WARN("{} not found in assets/fonts: falling back to the built-in bitmap font",
                       kRegularFont);
    }
    else if (semibold.empty())
    {
        VIGIL_LOG_WARN("{} not found in assets/fonts: using the regular face instead", kSemiboldFont);
        semibold = regular;
    }

    if (icons.empty())
        VIGIL_LOG_WARN("{} not found in assets/fonts: icons will be missing", kIconFont);
    fonts::Body    = addFont(regular,  icons, dp(17.0f), 1.00f);
    fonts::Label   = addFont(semibold, icons, dp(12.5f), 1.00f);
    fonts::Heading = addFont(semibold, icons, dp(18.0f), 1.00f);
    fonts::Title   = addFont(semibold, icons, dp(24.0f), 0.95f);
    fonts::Value   = addFont(semibold, icons, dp(22.0f), 0.90f);

    io.FontDefault = fonts::Body;
}

void applyTheme()
{
    auto& style = ImGui::GetStyle();
    style = ImGuiStyle(); // Start from the defaults: ScaleAllSizes() would otherwise compound on every call.
    style.WindowPadding       = ImVec2(20.0f, 18.0f);
    style.FramePadding        = ImVec2(14.0f, 9.0f);
    style.ItemSpacing         = ImVec2(12.0f, 12.0f);
    style.ItemInnerSpacing    = ImVec2(8.0f, 6.0f);
    style.WindowRounding      = 0.0f;
    style.ChildRounding       = 14.0f;
    style.FrameRounding       = 10.0f;
    style.PopupRounding       = 10.0f;
    style.ScrollbarRounding   = 10.0f;
    style.GrabRounding        = 10.0f;
    style.GrabMinSize         = 16.0f;
    style.WindowBorderSize    = 0.0f;
    style.ChildBorderSize     = 1.0f;
    style.FrameBorderSize     = 0.0f;
    style.PopupBorderSize     = 1.0f;
    style.ScrollbarSize     = 8.0f;
    style.ScrollbarRounding = 10.0f;
    style.ScaleAllSizes(g_Scale);

    auto* c = style.Colors;
    c[ImGuiCol_ScrollbarBg]          = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    c[ImGuiCol_ScrollbarGrab]        = colors::withAlpha(colors::Muted, 0.22f);
    c[ImGuiCol_ScrollbarGrabHovered] = colors::withAlpha(colors::Muted, 0.45f);
    c[ImGuiCol_ScrollbarGrabActive]  = colors::withAlpha(colors::Accent, 0.85f);
    c[ImGuiCol_WindowBg]             = colors::Clear;
    c[ImGuiCol_ChildBg]              = colors::Surface;
    c[ImGuiCol_PopupBg]              = ImVec4(0.085f, 0.102f, 0.133f, 0.98f);
    c[ImGuiCol_Border]               = colors::Border;
    c[ImGuiCol_Text]                 = ImVec4(0.91f, 0.94f, 0.97f, 1.00f);
    c[ImGuiCol_TextDisabled]         = colors::TextDim;
    c[ImGuiCol_FrameBg]              = ImVec4(0.11f, 0.13f, 0.17f, 1.00f);
    c[ImGuiCol_FrameBgHovered]       = ImVec4(0.15f, 0.19f, 0.23f, 1.00f);
    c[ImGuiCol_FrameBgActive]        = ImVec4(0.18f, 0.23f, 0.27f, 1.00f);
    c[ImGuiCol_Button]               = ImVec4(0.14f, 0.17f, 0.22f, 1.00f);
    c[ImGuiCol_ButtonHovered]        = ImVec4(0.19f, 0.23f, 0.29f, 1.00f);
    c[ImGuiCol_ButtonActive]         = ImVec4(0.12f, 0.15f, 0.19f, 1.00f);
    c[ImGuiCol_Header]               = ImVec4(0.10f, 0.43f, 0.39f, 0.55f);
    c[ImGuiCol_HeaderHovered]        = ImVec4(0.12f, 0.57f, 0.51f, 0.70f);
    c[ImGuiCol_HeaderActive]         = ImVec4(0.10f, 0.51f, 0.46f, 1.00f);
    c[ImGuiCol_SliderGrab]           = colors::Accent;
    c[ImGuiCol_SliderGrabActive]     = ImVec4(0.46f, 0.95f, 0.83f, 1.00f);
    c[ImGuiCol_Separator]            = colors::Border;
}

} // namespace Sortly::ui
