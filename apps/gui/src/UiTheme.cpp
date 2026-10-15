// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "Sortly/Gui/UiTheme.hpp"
#include "Sortly/Gui/Icons.hpp"

#include <vigil/vigil.h>

#include <filesystem>
#include <string>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#endif

#ifndef SORTLY_ASSETS_DIR
#define SORTLY_ASSETS_DIR ""
#endif

namespace Sortly::ui {

namespace {

namespace fs = std::filesystem;

fs::path executableDir()
{
#if defined(_WIN32)
    wchar_t buffer[MAX_PATH] = {};
    const DWORD len = GetModuleFileNameW(nullptr, buffer, MAX_PATH);
    if (len > 0 && len < MAX_PATH)
        return fs::path(buffer).parent_path();
#endif
    return {};
}

// Looks for the Lucide font next to the executable (post-build copy) and then in the source tree.
std::string findIconFont()
{
    const fs::path candidates[] = {
        executableDir() / "assets" / "fonts" / "lucide.ttf",
        fs::path("assets") / "fonts" / "lucide.ttf",
        fs::path(SORTLY_ASSETS_DIR) / "fonts" / "lucide.ttf",
    };

    std::error_code ec;
    for (const auto& path : candidates)
        if (!path.empty() && fs::exists(path, ec))
            return path.string();
    return {};
}

std::string findSystemFont(const char* fileName)
{
#if defined(_WIN32)
    const fs::path path = fs::path("C:/Windows/Fonts") / fileName;
    std::error_code ec;
    if (fs::exists(path, ec))
        return path.string();
#else
    (void)fileName;
#endif
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

void loadFonts(float dpiScale)
{
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->Clear();

    const std::string regular  = findSystemFont("segoeui.ttf");
    const std::string semibold = findSystemFont("seguisb.ttf");
    const std::string& heading = semibold.empty() ? regular : semibold;
    const std::string icons    = findIconFont();

    if (icons.empty())
        VIGIL_LOG_WARN("lucide.ttf not found: icons will be missing");

    const float s = dpiScale;
    fonts::Body    = addFont(regular, icons, 17.0f * s, 1.00f);
    fonts::Label   = addFont(heading, icons, 12.5f * s, 1.00f);
    fonts::Heading = addFont(heading, icons, 18.0f * s, 1.00f);
    fonts::Title   = addFont(heading, icons, 24.0f * s, 0.95f);
    fonts::Value   = addFont(heading, icons, 30.0f * s, 0.90f);

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
    style.SeparatorTextBorderSize = 1.0f;

    auto* c = style.Colors;
    c[ImGuiCol_WindowBg]         = colors::Clear;
    c[ImGuiCol_ChildBg]          = colors::Surface;
    c[ImGuiCol_PopupBg]          = ImVec4(0.085f, 0.102f, 0.133f, 0.98f);
    c[ImGuiCol_Border]           = ImVec4(0.17f, 0.20f, 0.25f, 0.80f);
    c[ImGuiCol_Text]             = ImVec4(0.91f, 0.94f, 0.97f, 1.00f);
    c[ImGuiCol_TextDisabled]     = ImVec4(0.48f, 0.54f, 0.62f, 1.00f);
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
    c[ImGuiCol_PlotHistogram]    = colors::Accent;
    c[ImGuiCol_Separator]        = ImVec4(0.17f, 0.20f, 0.25f, 0.80f);
}

} // namespace Sortly::ui
