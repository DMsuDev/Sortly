// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "Sortly/Gui/ImGuiLayer.hpp"
#include "Sortly/Gui/UiTheme.hpp"

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <implot.h>

#include <vigil/vigil.h>
#include <foundry/memory/scope_guard.h>

namespace Sortly {

namespace {

constexpr const char* kGlslVersion = "#version 330";

} // namespace

bool ImGuiLayer::init(GLFWwindow* window, float uiScale)
{
    shutdown();
    FOUNDRY_SCOPE_EXIT_NAMED(shutdownGuard) { shutdown(); };

    // Font and theme loading may throw; init() reports failure instead.
    try
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        m_Stage = Stage::ImGui;

        ImPlot::CreateContext();
        m_Stage = Stage::ImPlot;

        ui::loadFonts(uiScale);

        if (!ImGui_ImplGlfw_InitForOpenGL(window, true))
        {
            VIGIL_LOG_ERROR("ImGui GLFW backend initialization failed");
            return false;
        }
        m_Stage = Stage::GlfwBackend;

        if (!ImGui_ImplOpenGL3_Init(kGlslVersion))
        {
            VIGIL_LOG_ERROR("ImGui OpenGL3 backend initialization failed");
            return false;
        }
        m_Stage = Stage::OpenGlBackend;

        ui::applyTheme();

        ImGuiIO &io = ImGui::GetIO();
        io.IniFilename = nullptr; // Disable saving .ini file

        shutdownGuard.dismiss();
        return true;
    }
    catch (...)
    {
        VIGIL_LOG_ERROR("UI layer initialization threw an exception");
        return false;
    }
}

void ImGuiLayer::shutdown() noexcept
{
    switch (m_Stage)
    {
        case Stage::OpenGlBackend: ImGui_ImplOpenGL3_Shutdown(); [[fallthrough]];
        case Stage::GlfwBackend:   ImGui_ImplGlfw_Shutdown();    [[fallthrough]];
        case Stage::ImPlot:        ImPlot::DestroyContext();     [[fallthrough]];
        case Stage::ImGui:         ImGui::DestroyContext();      [[fallthrough]];
        case Stage::None:          break;
    }
    m_Stage = Stage::None;
}

void ImGuiLayer::beginFrame()
{
    VIGIL_ASSERT_MSG(m_Stage == Stage::OpenGlBackend, "ImGuiLayer::beginFrame called before initialization");

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void ImGuiLayer::endFrame()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

} // namespace Sortly
