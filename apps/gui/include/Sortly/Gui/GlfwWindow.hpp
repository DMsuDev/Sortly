// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#pragma once

#include <memory>

struct GLFWwindow;

namespace Sortly {

/// @brief Owns the GLFW library, one window and its OpenGL context.
/// Knows nothing about ImGui.
class GlfwWindow
{
public:
    GlfwWindow() = default;
    ~GlfwWindow() { destroy(); }

    GlfwWindow(const GlfwWindow&)            = delete;
    GlfwWindow& operator=(const GlfwWindow&) = delete;

    /// @return true when the window is ready and its GL context is current.
    /// On failure nothing stays alive.
    [[nodiscard]] bool create(int width, int height, const char* title, int minWidth, int minHeight);

    /// @brief Destroys the window, then terminates GLFW. Safe to call repeatedly.
    void destroy() noexcept;

    /// @return true when there is no window or the user asked to close it.
    [[nodiscard]] bool shouldClose() const noexcept;

    [[nodiscard]] GLFWwindow* handle() const noexcept { return m_Handle.get(); }
    [[nodiscard]] float       uiScale() const noexcept { return m_UiScale; }

    void pollEvents() noexcept;

    /// @brief Sleeps until at least one event arrives. Use while the window is minimized.
    void waitEvents() noexcept;

    /// @return true while the window is iconified (its framebuffer is 0x0).
    [[nodiscard]] bool isMinimized() const noexcept;

    /// @brief Sets the viewport to the framebuffer size and clears the color buffer.
    void clear(float r, float g, float b, float a) const;

    void swapBuffers() const noexcept;

private:
    /// @brief Tells unique_ptr how to free a GLFWwindow (it cannot be `delete`d).
    struct Deleter
    {
        void operator()(GLFWwindow* window) const noexcept;
    };

    float m_UiScale = 1.0f;
    std::unique_ptr<GLFWwindow, Deleter> m_Handle;
};

} // namespace Sortly
