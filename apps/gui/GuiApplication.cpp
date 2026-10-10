// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "GuiApplication.hpp"

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <vigil/vigil.h>

namespace Sortly {

bool GuiApplication::onInit()
{
    if (!glfwInit())
    {
        VIGIL_LOG_ERROR("GLFW initialization failed");
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    m_Window = glfwCreateWindow(1280, 720, "Sortly", nullptr, nullptr);
    if (!m_Window)
    {
        VIGIL_LOG_ERROR("Window creation failed");
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(m_Window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    return true;
}

int GuiApplication::onRun(int /*argc*/, char** /*argv*/)
{
    while (!glfwWindowShouldClose(m_Window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        drawUi();

        ImGui::Render();

        int width = 0, height = 0;
        glfwGetFramebufferSize(m_Window, &width, &height);
        glViewport(0, 0, width, height);
        glClearColor(0.10f, 0.10f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(m_Window);
    }

    return 0;
}

void GuiApplication::onShutdown()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(m_Window);
    glfwTerminate();
    m_Window = nullptr;
}

void GuiApplication::drawUi()
{
    // TODO: move to apps/gui/ui/ panels; read sessions() from here.
    ImGui::Begin("Sortly");
    ImGui::Text("Sessions: %zu", sessions().sessions().size());
    ImGui::End();
}

} // namespace Sortly
