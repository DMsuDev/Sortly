// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "Sortly/Gui/UiTheme.hpp"
#include "Sortly/Gui/Icons.hpp"

#include <vigil/vigil.h>

#include <cstdint>
#include <filesystem>
#include <string>

#if defined(_WIN32)
#include <Windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#endif

#ifndef SORTLY_ASSETS_DIR
#define SORTLY_ASSETS_DIR ""
#endif

namespace Sortly::ui {

namespace {

namespace fs = std::filesystem;

fs::path executableDir()
{
    std::error_code ec;
#if defined(_WIN32)
    wchar_t buffer[MAX_PATH] = {};
    const DWORD len = GetModuleFileNameW(nullptr, buffer, MAX_PATH);
    if (len > 0 && len < MAX_PATH)
        return fs::path(buffer).parent_path();
#elif defined(__APPLE__)
    char buffer[4096] = {};
    std::uint32_t size = sizeof(buffer);
    if (_NSGetExecutablePath(buffer, &size) == 0)
        return fs::weakly_canonical(fs::path(buffer), ec).parent_path();
#else
    const fs::path exe = fs::read_symlink("/proc/self/exe", ec);
    if (!ec)
        return exe.parent_path();
#endif
    return {};
}

// Looks for the Lucide font next to the executable (post-build copy), in the working
// directory and finally in the source tree.
std::string findIconFont()
{
    const fs::path candidates[] = {
        executableDir() / "assets" / "fonts" / "lucide.ttf",
        fs::path("assets") / "fonts" / "lucide.ttf",
        fs::path(SORTLY_ASSETS_DIR) / "lucide.ttf",
    };

    std::error_code ec;
    for (const auto& path : candidates)
        if (!path.empty() && fs::exists(path, ec))
            return path.string();
    return {};
}

struct FontPair
{
    const char* regular;
    const char* semibold;
};

// First pair whose regular face exists wins; the semibold face falls back to the regular one.
constexpr FontPair kSystemFonts[] = {
#if defined(_WIN32)
    { "C:/Windows/Fonts/segoeui.ttf", "C:/Windows/Fonts/seguisb.ttf" },
#elif defined(__APPLE__)
    { "/System/Library/Fonts/Helvetica.ttc", "/System/Library/Fonts/Helvetica.ttc" },
    { "/Library/Fonts/Arial.ttf",            "/Library/Fonts/Arial Bold.ttf"       },
#else
    { "/usr/share/fonts/noto/NotoSans-Regular.ttf",                 "/usr/share/fonts/noto/NotoSans-SemiBold.ttf" },
    { "/usr/share/fonts/truetype/noto/NotoSans-Regular.ttf",        "/usr/share/fonts/truetype/noto/NotoSans-SemiBold.ttf" },
    { "/usr/share/fonts/truetype/ubuntu/Ubuntu-R.ttf",              "/usr/share/fonts/truetype/ubuntu/Ubuntu-M.ttf" },
    { "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",            "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf" },
    { "/usr/share/fonts/TTF/DejaVuSans.ttf",                        "/usr/share/fonts/TTF/DejaVuSans-Bold.ttf" },
    { "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf", "/usr/share/fonts/truetype/liberation/LiberationSans-Bold.ttf" },
#endif
};

struct TextFonts
{
    std::string regular;
    std::string semibold;
};

TextFonts findTextFonts()
{
    std::error_code ec;
    for (const auto& pair : kSystemFonts)
    {
        if (!fs::exists(pair.regular, ec))
            continue;

        TextFonts fonts{ pair.regular, pair.regular };
        if (fs::exists(pair.semibold, ec))
            fonts.semibold = pair.semibold;
        return fonts;
    }
    return {};
}

ImFont* addFont(const std::string& textFont, const std::string& iconFont, float size, float iconScale)
{
    ImGuiIO& io = ImGui::GetIO();

    ImFont* font = nullptr;
    if (!textFont.empty())
    {
        font = io.Fonts->AddFontFromFileTTF(textFont.c_str(), size);
    }
    else
    {
        ImFontConfig fallback;
        fallback.SizePixels = size;
        font = io.Fonts->AddFontDefault(&fallback);
    }

    if (!iconFont.empty())
    {
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
        cfg.GlyphOffset.y    = (size - iconSize) * 0.5f + size * 0.10f;
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

    const TextFonts text  = findTextFonts();
    const std::string icons = findIconFont();

    if (text.regular.empty())
        VIGIL_LOG_WARN("No system UI font found: falling back to the built-in bitmap font");
    if (icons.empty())
        VIGIL_LOG_WARN("lucide.ttf not found: icons will be missing");

    fonts::Body    = addFont(text.regular,  icons, dp(17.0f), 1.00f);
    fonts::Label   = addFont(text.semibold, icons, dp(12.5f), 1.00f);
    fonts::Heading = addFont(text.semibold, icons, dp(18.0f), 1.00f);
    fonts::Title   = addFont(text.semibold, icons, dp(24.0f), 0.95f);
    fonts::Value   = addFont(text.semibold, icons, dp(22.0f), 0.90f);

    io.FontDefault = fonts::Body;
}

void applyTheme()
{
    auto& style = ImGui::GetStyle();
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
    style.ScaleAllSizes(g_Scale);

    auto* c = style.Colors;
    c[ImGuiCol_WindowBg]         = colors::Clear;
    c[ImGuiCol_ChildBg]          = colors::Surface;
    c[ImGuiCol_PopupBg]          = ImVec4(0.085f, 0.102f, 0.133f, 0.98f);
    c[ImGuiCol_Border]           = colors::Border;
    c[ImGuiCol_Text]             = ImVec4(0.91f, 0.94f, 0.97f, 1.00f);
    c[ImGuiCol_TextDisabled]     = colors::TextDim;
    c[ImGuiCol_FrameBg]          = ImVec4(0.11f, 0.13f, 0.17f, 1.00f);
    c[ImGuiCol_FrameBgHovered]   = ImVec4(0.15f, 0.19f, 0.23f, 1.00f);
    c[ImGuiCol_FrameBgActive]    = ImVec4(0.18f, 0.23f, 0.27f, 1.00f);
    c[ImGuiCol_Button]           = ImVec4(0.14f, 0.17f, 0.22f, 1.00f);
    c[ImGuiCol_ButtonHovered]    = ImVec4(0.19f, 0.23f, 0.29f, 1.00f);
    c[ImGuiCol_ButtonActive]     = ImVec4(0.12f, 0.15f, 0.19f, 1.00f);
    c[ImGuiCol_Header]           = ImVec4(0.10f, 0.43f, 0.39f, 0.55f);
    c[ImGuiCol_HeaderHovered]    = ImVec4(0.12f, 0.57f, 0.51f, 0.70f);
    c[ImGuiCol_HeaderActive]     = ImVec4(0.10f, 0.51f, 0.46f, 1.00f);
    c[ImGuiCol_SliderGrab]       = colors::Accent;
    c[ImGuiCol_SliderGrabActive] = ImVec4(0.46f, 0.95f, 0.83f, 1.00f);
    c[ImGuiCol_Separator]        = colors::Border;
}

} // namespace Sortly::ui
