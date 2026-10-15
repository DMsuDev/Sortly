// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#pragma once

struct GLFWwindow;

namespace Sortly {

/// @brief RAII owner of the GLFW window, OpenGL context and ImGui/ImPlot backends.
class GlfwWindow
{
public:
    GlfwWindow() = default;
    ~GlfwWindow() { destroy(); }

    GlfwWindow(const GlfwWindow&)            = delete;
    GlfwWindow& operator=(const GlfwWindow&) = delete;

    [[nodiscard]] bool create(int width, int height, const char* title, int minWidth, int minHeight);
    void destroy();

    [[nodiscard]] bool shouldClose() const;
    void pollEvents();
    void beginFrame();
    void endFrame();

private:
    GLFWwindow* m_Handle      = nullptr;
    bool        m_Initialized = false;
};

} // namespace Sortly
