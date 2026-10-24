// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "Sortly/Gui/Window.hpp"

#include <GLFW/glfw3.h>

#include <vigil/vigil.h>
#include <foundry/memory/scope_guard.h>

#include <algorithm>
#include <cstring>

namespace Sortly {

namespace {

constexpr int kAreaPercent  = 90; // The first window uses at most 90% of the work area.
constexpr int kResizeBorder = 6;  // Invisible resize border, in design pixels.

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

// Monitor showing most of the window; the primary one as a fallback.
GLFWmonitor* monitorOf(GLFWwindow* window)
{
    int wx = 0, wy = 0, ww = 0, wh = 0;
    glfwGetWindowPos(window, &wx, &wy);
    glfwGetWindowSize(window, &ww, &wh);

    int count = 0;
    GLFWmonitor** monitors = glfwGetMonitors(&count);

    GLFWmonitor* best        = nullptr;
    int          bestOverlap = 0;
    for (int i = 0; i < count; ++i)
    {
        const GLFWvidmode* mode = glfwGetVideoMode(monitors[i]);
        if (!mode)
            continue;

        int mx = 0, my = 0;
        glfwGetMonitorPos(monitors[i], &mx, &my);

        const int overlapX = std::max(0, std::min(wx + ww, mx + mode->width)  - std::max(wx, mx));
        const int overlapY = std::max(0, std::min(wy + wh, my + mode->height) - std::max(wy, my));
        if (overlapX * overlapY > bestOverlap)
        {
            bestOverlap = overlapX * overlapY;
            best        = monitors[i];
        }
    }
    return best ? best : glfwGetPrimaryMonitor();
}

} // namespace

void Window::Deleter::operator()(GLFWwindow* window) const noexcept
{
    glfwDestroyWindow(window);
}

bool Window::create(int width, int height, const char* title, int minWidth, int minHeight)
{
    destroy(); // Makes a repeated create() safe.
    FOUNDRY_SCOPE_EXIT_NAMED(destroyGuard) { destroy(); };

    glfwSetErrorCallback([](int error, const char* desc) {
        // Ignore Wayland error spam that occurs when querying or moving windows.
        if (error == GLFW_PLATFORM_ERROR && desc != nullptr && std::strstr(desc, "Wayland") != nullptr)
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
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE); // Custom ImGui title bar.
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);   // Shown once centered: no flash in a corner.

    // Keeps the first window inside the usable desktop area, whatever the display scale.
    int areaX = 0, areaY = 0, areaW = 0, areaH = 0;
    if (GLFWmonitor* monitor = glfwGetPrimaryMonitor())
        glfwGetMonitorWorkarea(monitor, &areaX, &areaY, &areaW, &areaH);

    const bool hasArea = areaW > 0 && areaH > 0;
    if (hasArea)
    {
        width  = std::min(width,  areaW * kAreaPercent / 100);
        height = std::min(height, areaH * kAreaPercent / 100);
    }

    m_Handle.reset(glfwCreateWindow(width, height, title, nullptr, nullptr));
    if (!m_Handle)
    {
        VIGIL_LOG_ERROR("Window creation failed");
        return false;
    }

    glfwSetWindowOpacity(m_Handle.get(), 1.0f);

    m_UiScale   = queryUiScale(m_Handle.get());
    m_MinWidth  = scaled(minWidth,  m_UiScale);
    m_MinHeight = scaled(minHeight, m_UiScale);
    glfwSetWindowSizeLimits(m_Handle.get(), m_MinWidth, m_MinHeight, GLFW_DONT_CARE, GLFW_DONT_CARE);

    if (hasArea)
        glfwSetWindowPos(m_Handle.get(), areaX + (areaW - width) / 2, areaY + (areaH - height) / 2);

    glfwMakeContextCurrent(m_Handle.get());
    glfwSwapInterval(1); // VSync.
    glfwShowWindow(m_Handle.get());

    destroyGuard.dismiss();
    return true;
}

void Window::destroy() noexcept
{
    if (m_Handle && glfwGetCurrentContext() == m_Handle.get())
        glfwMakeContextCurrent(nullptr);

    m_Handle.reset();
    glfwTerminate(); // This class owns GLFW's lifetime. No-op if it was not initialized.

    m_UiScale      = 1.0f;
    m_Windowed     = {};
    m_WasMaximized = false;
    m_Drag         = Drag::None;
    m_ResizeEdges  = kEdgeNone;
    m_LeftDown     = false;
}

// ================================================
// EVENTS AND STATE
// ================================================

void Window::pollEvents() noexcept
{
    if (!m_Handle)
        return;
    glfwPollEvents();
    updateInteraction();
}

void Window::waitEvents() noexcept
{
    if (m_Handle)
        glfwWaitEvents();
}

bool Window::isCloseRequested() const noexcept
{
    return !m_Handle || glfwWindowShouldClose(m_Handle.get()) == GLFW_TRUE;
}

bool Window::isMinimized() const noexcept
{
    return m_Handle && glfwGetWindowAttrib(m_Handle.get(), GLFW_ICONIFIED) == GLFW_TRUE;
}

bool Window::isMaximized() const noexcept
{
    return m_Handle && glfwGetWindowAttrib(m_Handle.get(), GLFW_MAXIMIZED) == GLFW_TRUE;
}

bool Window::isFullscreen() const noexcept
{
    return m_Handle && glfwGetWindowMonitor(m_Handle.get()) != nullptr;
}

// ================================================
// WINDOW CONTROLS
// ================================================

void Window::requestClose() noexcept
{
    if (m_Handle)
        glfwSetWindowShouldClose(m_Handle.get(), GLFW_TRUE);
}

void Window::minimize() noexcept
{
    if (m_Handle)
        glfwIconifyWindow(m_Handle.get());
}

void Window::toggleMaximize() noexcept
{
    if (!m_Handle || isFullscreen())
        return;

    m_Drag = Drag::None;
    if (isMaximized())
        glfwRestoreWindow(m_Handle.get());
    else
        glfwMaximizeWindow(m_Handle.get());
}

void Window::toggleFullscreen() noexcept
{
    GLFWwindow* window = m_Handle.get();
    if (!window)
        return;

    m_Drag = Drag::None;

    if (isFullscreen())
    {
        if (m_Windowed.width <= 0 || m_Windowed.height <= 0)
            return;

        glfwSetWindowMonitor(window, nullptr, m_Windowed.x, m_Windowed.y,
                             m_Windowed.width, m_Windowed.height, GLFW_DONT_CARE);
        if (m_WasMaximized)
            glfwMaximizeWindow(window);
        return;
    }

    GLFWmonitor*       monitor = monitorOf(window);
    const GLFWvidmode* mode    = monitor ? glfwGetVideoMode(monitor) : nullptr;
    if (!mode)
        return;

    glfwGetWindowPos(window, &m_Windowed.x, &m_Windowed.y);
    glfwGetWindowSize(window, &m_Windowed.width, &m_Windowed.height);
    m_WasMaximized = isMaximized();

    int mx = 0, my = 0;
    glfwGetMonitorPos(monitor, &mx, &my);
    glfwSetWindowMonitor(window, monitor, mx, my, mode->width, mode->height, mode->refreshRate);
}

// ================================================
// MOVE / RESIZE
// ================================================

void Window::beginDrag() noexcept
{
    begin(Drag::Move, kEdgeNone);
}

Window::Edges Window::activeEdges() const noexcept
{
    switch (m_Drag)
    {
        case Drag::Resize: return m_ResizeEdges;
        case Drag::Move:   return kEdgeNone;
        case Drag::None:   break;
    }
    return edgesUnderCursor();
}

Window::Edges Window::edgesUnderCursor() const noexcept
{
    if (!m_Handle || isMaximized() || isFullscreen())
        return kEdgeNone;

    double cx = 0.0, cy = 0.0;
    int    w  = 0,   h  = 0;
    glfwGetCursorPos(m_Handle.get(), &cx, &cy);
    glfwGetWindowSize(m_Handle.get(), &w, &h);

    if (cx < 0.0 || cy < 0.0 || cx >= w || cy >= h)
        return kEdgeNone;

    const double border = scaled(kResizeBorder, m_UiScale);
    Edges edges = kEdgeNone;
    if (cx < border)           edges |= kEdgeLeft;
    else if (cx >= w - border) edges |= kEdgeRight;
    if (cy < border)           edges |= kEdgeTop;
    else if (cy >= h - border) edges |= kEdgeBottom;
    return edges;
}

// Window-relative cursor + window position = screen position, stable while the window moves.
void Window::cursorOnScreen(double& x, double& y) const noexcept
{
    int    wx = 0,   wy = 0;
    double cx = 0.0, cy = 0.0;
    glfwGetWindowPos(m_Handle.get(), &wx, &wy);
    glfwGetCursorPos(m_Handle.get(), &cx, &cy);
    x = wx + cx;
    y = wy + cy;
}

void Window::begin(Drag drag, Edges edges) noexcept
{
    GLFWwindow* window = m_Handle.get();
    // A resize already started on this click wins over the title-bar move.
    if (!window || m_Drag != Drag::None || isMaximized() || isFullscreen())
        return;

    glfwGetWindowPos(window, &m_Start.x, &m_Start.y);
    glfwGetWindowSize(window, &m_Start.width, &m_Start.height);
    cursorOnScreen(m_AnchorX, m_AnchorY);
    m_Drag        = drag;
    m_ResizeEdges = edges;
}

void Window::updateInteraction() noexcept
{
    GLFWwindow* window = m_Handle.get();

    const bool down    = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    const bool pressed = down && !m_LeftDown;
    m_LeftDown = down;

    if (!down || isMaximized() || isFullscreen())
    {
        m_Drag        = Drag::None;
        m_ResizeEdges = kEdgeNone;
        return;
    }

    if (pressed)
    {
        const Edges edges = edgesUnderCursor();
        if (edges != kEdgeNone)
            begin(Drag::Resize, edges);
    }

    if (m_Drag == Drag::None)
        return;

    double cx = 0.0, cy = 0.0;
    cursorOnScreen(cx, cy);
    const int dx = static_cast<int>(cx - m_AnchorX);
    const int dy = static_cast<int>(cy - m_AnchorY);

    Rect r = m_Start;
    if (m_Drag == Drag::Move)
    {
        glfwSetWindowPos(window, r.x + dx, r.y + dy);
        return;
    }

    if (m_ResizeEdges & kEdgeRight)  r.width  += dx;
    if (m_ResizeEdges & kEdgeBottom) r.height += dy;
    if (m_ResizeEdges & kEdgeLeft)   { r.x += dx; r.width  -= dx; }
    if (m_ResizeEdges & kEdgeTop)    { r.y += dy; r.height -= dy; }

    // At the minimum size the opposite edge stays put instead of the window sliding away.
    if (r.width < m_MinWidth)
    {
        r.width = m_MinWidth;
        if (m_ResizeEdges & kEdgeLeft)
            r.x = m_Start.x + m_Start.width - m_MinWidth;
    }
    if (r.height < m_MinHeight)
    {
        r.height = m_MinHeight;
        if (m_ResizeEdges & kEdgeTop)
            r.y = m_Start.y + m_Start.height - m_MinHeight;
    }

    glfwSetWindowPos(window, r.x, r.y);
    glfwSetWindowSize(window, r.width, r.height);
}

// ================================================
// RENDERING
// ================================================

void Window::clear(float r, float g, float b, float a) const
{
    if (!m_Handle)
        return;

    int width = 0, height = 0;
    glfwGetFramebufferSize(m_Handle.get(), &width, &height);
    glViewport(0, 0, width, height);
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT);
}

void Window::swapBuffers() const noexcept
{
    if (m_Handle)
        glfwSwapBuffers(m_Handle.get());
}

} // namespace Sortly
