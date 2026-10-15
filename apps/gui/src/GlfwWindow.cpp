// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "Sortly/Gui/GlfwWindow.hpp"
#include "Sortly/Gui/UiTheme.hpp"

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <implot.h>

#include <vigil/vigil.h>

namespace Sortly {

bool GlfwWindow::create(int width, int height, const char* title, int minWidth, int minHeight)
{
    if (!glfwInit())
    {
        VIGIL_LOG_ERROR("GLFW initialization failed");
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    m_Handle = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!m_Handle)
    {
        VIGIL_LOG_ERROR("Window creation failed");
        glfwTerminate();
        return false;
    }

    glfwSetWindowSizeLimits(m_Handle, minWidth, minHeight, GLFW_DONT_CARE, GLFW_DONT_CARE);
    glfwMakeContextCurrent(m_Handle);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ui::loadFonts();
    ImGui_ImplGlfw_InitForOpenGL(m_Handle, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    ui::applyTheme();

    m_Initialized = true;
    return true;
}

void GlfwWindow::destroy()
{
    if (m_Initialized)
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImPlot::DestroyContext();
        ImGui::DestroyContext();
        m_Initialized = false;
    }

    if (m_Handle)
    {
        glfwDestroyWindow(m_Handle);
        glfwTerminate();
        m_Handle = nullptr;
    }
}

bool GlfwWindow::shouldClose() const { return !m_Handle || glfwWindowShouldClose(m_Handle); }

void GlfwWindow::pollEvents() { glfwPollEvents(); }

void GlfwWindow::beginFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void GlfwWindow::endFrame()
{
    ImGui::Render();

    int width = 0, height = 0;
    glfwGetFramebufferSize(m_Handle, &width, &height);
    glViewport(0, 0, width, height);

    const ImVec4& c = ui::colors::Clear;
    glClearColor(c.x, c.y, c.z, c.w);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(m_Handle);
}

} // namespace Sortly
