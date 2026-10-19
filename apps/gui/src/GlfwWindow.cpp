// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "Sortly/Gui/GlfwWindow.hpp"

#include <GLFW/glfw3.h>

#include <vigil/vigil.h>
#include <foundry/memory/scope_guard.h>

#include <algorithm>
#include <string_view>

namespace Sortly {

namespace {

constexpr int kAreaPercent = 90; // The first window uses at most 90% of the work area.

// On macOS the backend already handles the framebuffer scale and window
// coordinates are in points; elsewhere the UI and size limits must be scaled.
float queryUiScale([[maybe_unused]] GLFWwindow* window)
{
#if defined(__APPLE__)
    return 1.0f;
#else
    float scaleX = 1.0f, scaleY = 1.0f;
    glfwGetWindowContentScale(window, &scaleX, &scaleY);
    return scaleX;
#endif
}

int scaled(int value, float scale)
{
    return static_cast<int>(static_cast<float>(value) * scale);
}

} // namespace

void GlfwWindow::Deleter::operator()(GLFWwindow* window) const noexcept
{
    glfwDestroyWindow(window);
}

bool GlfwWindow::create(int width, int height, const char* title, int minWidth, int minHeight)
{
    destroy(); // Makes a repeated create() safe.
    FOUNDRY_SCOPE_EXIT_NAMED(destroyGuard) { destroy(); };

    glfwSetErrorCallback([](int error, const char* desc) {
        // Ignore Wayland error spam that occurs when querying or moving windows.
        if (error == GLFW_PLATFORM_ERROR && std::strstr(desc, "Wayland"))
            return;

        VIGIL_LOG_WARN("GLFW error 0x{:05X}: {}", error, desc);
    });

    if (!glfwInit())
    {
        VIGIL_LOG_ERROR("GLFW initialization failed");
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#if defined(__APPLE__)
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif

    // Keeps the first window inside the usable desktop area, whatever the display scale.
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    if (monitor != nullptr)
    {
        int x = 0, y = 0, areaW = 0, areaH = 0;
        glfwGetMonitorWorkarea(monitor, &x, &y, &areaW, &areaH);
        if (areaW > 0 && areaH > 0)
        {
            width  = std::min(width,  areaW * kAreaPercent / 100);
            height = std::min(height, areaH * kAreaPercent / 100);
        }
    }

    m_Handle.reset(glfwCreateWindow(width, height, title, nullptr, nullptr));
    if (!m_Handle)
    {
        VIGIL_LOG_ERROR("Window creation failed");
        return false;
    }

    // Force window to be fully opaque by default
    glfwSetWindowOpacity(m_Handle.get(), 1.0f);

    m_UiScale = queryUiScale(m_Handle.get());

    glfwSetWindowSizeLimits(m_Handle.get(),
                            scaled(minWidth, m_UiScale), scaled(minHeight, m_UiScale),
                            GLFW_DONT_CARE, GLFW_DONT_CARE);
    glfwMakeContextCurrent(m_Handle.get());

    // Enable VSync.
    glfwSwapInterval(1);

    destroyGuard.dismiss();
    return true;
}

void GlfwWindow::destroy() noexcept
{
    m_Handle.reset();
    // No-op when the window was not created.
    glfwTerminate();
}

bool GlfwWindow::shouldClose() const noexcept
{
    return !m_Handle || glfwWindowShouldClose(m_Handle.get());
}

void GlfwWindow::pollEvents() noexcept
{
    glfwPollEvents();
}

void GlfwWindow::waitEvents() noexcept
{
    glfwWaitEvents();
}

bool GlfwWindow::isMinimized() const noexcept
{
    return m_Handle && glfwGetWindowAttrib(m_Handle.get(), GLFW_ICONIFIED) == GLFW_TRUE;
}

void GlfwWindow::clear(float r, float g, float b, float a) const
{
    int width = 0, height = 0;
    glfwGetFramebufferSize(m_Handle.get(), &width, &height);
    glViewport(0, 0, width, height);
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT);
}

void GlfwWindow::swapBuffers() const noexcept
{
    glfwSwapBuffers(m_Handle.get());
}

} // namespace Sortly
