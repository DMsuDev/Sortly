// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "Sortly/Gui/Window.hpp"

#include <GLFW/glfw3.h>

#include <vigil/vigil.h>
#include <foundry/memory/scope_guard.h>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <utility>

namespace Sortly {

namespace {

constexpr float kMinUiScale    = 0.6f;
constexpr float kScaleEpsilon  = 0.01f;
constexpr int   kAreaPercent   = 90; // A window and the UI scale use at most 90% of the work area.
constexpr int   kResizeBorder  = 6;  // Invisible resize border, in design pixels.

struct WorkArea
{
    int x = 0, y = 0, width = 0, height = 0;

    [[nodiscard]] bool valid() const noexcept { return width > 0 && height > 0; }
};

WorkArea workAreaOf(GLFWmonitor* monitor)
{
    WorkArea area;
    if (monitor)
        glfwGetMonitorWorkarea(monitor, &area.x, &area.y, &area.width, &area.height);
    return area;
}

// On macOS the backend already handles the framebuffer scale and window
// coordinates are in points; elsewhere the UI and size limits must be scaled.
float queryUiScale([[maybe_unused]] GLFWwindow* window)
{
#if defined(__APPLE__)
    return 1.0f;
#else
    float scaleX = 1.0f, scaleY = 1.0f;
    glfwGetWindowContentScale(window, &scaleX, &scaleY);

    // Use one scale for UI-related window dimensions.
    return std::max(scaleX, scaleY);
#endif
}

// Largest scale not above `wanted` at which the design minimum still fits the work area.
// max/min instead of std::clamp: clamp is undefined when its bounds are inverted.
float fitScale(float wanted, int minWidth, int minHeight, const WorkArea& area)
{
    if (!area.valid())
        return wanted;

    const float fitW = static_cast<float>(area.width * kAreaPercent / 100) / static_cast<float>(std::max(1, minWidth));
    const float fitH = static_cast<float>(area.height * kAreaPercent / 100) / static_cast<float>(std::max(1, minHeight));
    return std::max(std::min({ wanted, fitW, fitH }), std::min(kMinUiScale, wanted));
}

int scaled(int value, float scale)
{
    return static_cast<int>(std::lround(static_cast<float>(value) * scale));
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
    glfwWindowHint(GLFW_AUTO_ICONIFY, GLFW_FALSE); // Native fullscreen (Wayland) must not minimize on focus loss.
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#if defined(__APPLE__)
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif

    // Wayland cannot move or resize windows from code, so there the OS keeps its frame.
#if GLFW_VERSION_MAJOR > 3 || (GLFW_VERSION_MAJOR == 3 && GLFW_VERSION_MINOR >= 4)
    m_OsFrame = glfwGetPlatform() == GLFW_PLATFORM_WAYLAND;
#endif
    glfwWindowHint(GLFW_DECORATED, m_OsFrame ? GLFW_TRUE : GLFW_FALSE); // Otherwise: custom ImGui title bar.
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE); // Shown once centered: no flash in a corner.

    m_Handle.reset(glfwCreateWindow(width, height, title, nullptr, nullptr));
    if (!m_Handle)
    {
        VIGIL_LOG_ERROR("Window creation failed");
        return false;
    }

    GLFWwindow* window = m_Handle.get();
    glfwSetWindowOpacity(window, 1.0f);
    glfwSetWindowUserPointer(window, this);

    // The first window is centered on the primary monitor.
    m_Monitor = glfwGetPrimaryMonitor();
    const WorkArea area = workAreaOf(m_Monitor);

    m_DesignMinWidth  = std::max(1, minWidth);
    m_DesignMinHeight = std::max(1, minHeight);
    m_UiScale         = fitScale(queryUiScale(window), m_DesignMinWidth, m_DesignMinHeight, area);
    VIGIL_LOG_INFO("Initial UI scale: {:.2f}", m_UiScale);
    applyMinSize();

    // Sizes are given in design pixels. The window is still hidden, so resizing it is invisible.
    width  = scaled(width,  m_UiScale);
    height = scaled(height, m_UiScale);
    if (area.valid())
    {
        const float percent = static_cast<float>(kAreaPercent) / 100.0f;

        // Compute the maximum allowed width and height based on the area percentage.
        const float maxW = static_cast<float>(area.width * percent);
        const float maxH = static_cast<float>(area.height * percent);

        // Fit the window within the maximum allowed area while preserving aspect ratio.
        const float fit = std::min({
            1.0f,
            maxW / static_cast<float>(width),
            maxH / static_cast<float>(height)
        });

        width  = std::max(1, scaled(width, fit));
        height = std::max(1, scaled(height, fit));
    }
    glfwSetWindowSize(window, width, height);

    if (area.valid())
        glfwSetWindowPos(window, area.x + (area.width - width) / 2, area.y + (area.height - height) / 2);

    trackWindowedGeometry();

    // From here on, moving the window or changing its display scale re-evaluates the monitor
    // (handled in pollEvents(), once no drag is in progress).
    glfwSetWindowPosCallback(window, [](GLFWwindow* w, int, int) {
        if (auto* self = static_cast<Window*>(glfwGetWindowUserPointer(w)))
            self->m_MonitorDirty = true;
    });
    glfwSetWindowContentScaleCallback(window, [](GLFWwindow* w, float, float) {
        if (auto* self = static_cast<Window*>(glfwGetWindowUserPointer(w)))
            self->m_MonitorDirty = true;
    });

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // VSync.
    glfwShowWindow(window);

    destroyGuard.dismiss();
    return true;
}

void Window::destroy() noexcept
{
    if (m_Handle && glfwGetCurrentContext() == m_Handle.get())
        glfwMakeContextCurrent(nullptr);

    m_Handle.reset();
    glfwTerminate(); // This class owns GLFW's lifetime. No-op if it was not initialized.

    m_UiScale         = 1.0f;
    m_DesignMinWidth  = 1;
    m_DesignMinHeight = 1;
    m_OsFrame         = false;
    m_Monitor         = nullptr;
    m_MonitorDirty    = false;
    m_ScaleChanged    = false;
    m_Windowed        = {};
    m_Borderless      = false;
    m_WasMaximized    = false;
    m_Drag            = Drag::None;
    m_ResizeEdges     = kEdgeNone;
    m_LeftDown        = false;
}

// ================================================
// EVENTS AND STATE
// ================================================

void Window::pollEvents() noexcept
{
    if (!m_Handle)
        return;

    glfwPollEvents();
    trackWindowedGeometry();
    updateInteraction();

    // Waits for the end of a drag: the monitor is decided where the window is dropped.
    if (m_MonitorDirty && m_Drag == Drag::None && !isMinimized())
        refreshMonitor();
}

void Window::waitEvents() noexcept
{
    if (m_Handle)
        glfwWaitEvents();
}

bool Window::consumeScaleChange() noexcept
{
    return std::exchange(m_ScaleChanged, false);
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
    return m_Handle && (m_Borderless || glfwGetWindowMonitor(m_Handle.get()) != nullptr);
}

void Window::trackWindowedGeometry() noexcept
{
    if (!m_Handle || isMaximized() || isFullscreen() || isMinimized())
        return;

    glfwGetWindowPos(m_Handle.get(), &m_Windowed.x, &m_Windowed.y);
    glfwGetWindowSize(m_Handle.get(), &m_Windowed.width, &m_Windowed.height);
}

// ================================================
// MONITOR CHANGES
// ================================================

void Window::applyMinSize() noexcept
{
    m_MinWidth  = scaled(m_DesignMinWidth,  m_UiScale);
    m_MinHeight = scaled(m_DesignMinHeight, m_UiScale);
    glfwSetWindowSizeLimits(m_Handle.get(), m_MinWidth, m_MinHeight, GLFW_DONT_CARE, GLFW_DONT_CARE);
}

void Window::refreshMonitor() noexcept
{
    m_MonitorDirty = false;

    GLFWwindow*  window  = m_Handle.get();
    // Without an OS-reported position (Wayland) the monitor cannot be told apart: the scale callback is enough.
    GLFWmonitor* monitor = m_OsFrame ? m_Monitor : monitorOf(window);
    const WorkArea area  = workAreaOf(monitor);
    const float  scale   = fitScale(queryUiScale(window), m_DesignMinWidth, m_DesignMinHeight, area);

    const bool scaleChanged = std::abs(scale - m_UiScale) > kScaleEpsilon;
    if (monitor == m_Monitor && !scaleChanged)
        return;

    const float ratio = scale / m_UiScale;
    m_Monitor = monitor;
    if (scaleChanged)
    {
        VIGIL_LOG_INFO("UI scale {:.2f} -> {:.2f}", m_UiScale, scale);
        m_UiScale      = scale;
        m_ScaleChanged = true;
    }
    applyMinSize();

    // The OS keeps maximized and fullscreen windows on their screen.
    if (m_OsFrame || !area.valid() || isMaximized() || isFullscreen())
        return;

    // Same proportions on the new scale, never bigger than the monitor allows, and fully inside it.
    Rect r;
    glfwGetWindowPos(window, &r.x, &r.y);
    glfwGetWindowSize(window, &r.width, &r.height);

    r.width  = std::max(m_MinWidth,  std::min(scaled(r.width,  ratio), area.width  * kAreaPercent / 100));
    r.height = std::max(m_MinHeight, std::min(scaled(r.height, ratio), area.height * kAreaPercent / 100));
    r.x      = std::max(area.x, std::min(r.x, area.x + area.width  - r.width));
    r.y      = std::max(area.y, std::min(r.y, area.y + area.height - r.height));

    glfwSetWindowSize(window, r.width, r.height);
    glfwSetWindowPos(window, r.x, r.y);
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
    if (!m_Handle)
        return;

    m_Drag = Drag::None;

    if (isFullscreen())
        leaveFullscreen();
    else
        enterFullscreen();
}

void Window::enterFullscreen() noexcept
{
    GLFWwindow*        window  = m_Handle.get();
    GLFWmonitor*       monitor = monitorOf(window);
    const GLFWvidmode* mode    = monitor ? glfwGetVideoMode(monitor) : nullptr;
    if (!mode)
        return;

    // m_Windowed already holds the last normal geometry (see trackWindowedGeometry()).
    m_WasMaximized = isMaximized();

    int mx = 0, my = 0;
    glfwGetMonitorPos(monitor, &mx, &my);

    if (m_OsFrame)
    {
        // Wayland cannot position windows: ask the compositor, with the current mode so nothing is switched.
        glfwSetWindowMonitor(window, monitor, mx, my, mode->width, mode->height, mode->refreshRate);
        return;
    }

    // Set first, so trackWindowedGeometry() does not record the monitor-sized geometry.
    m_Borderless = true;
    if (m_WasMaximized)
        glfwRestoreWindow(window); // A maximized window ignores explicit sizes.

    glfwSetWindowPos(window, mx, my);
    glfwSetWindowSize(window, mode->width, mode->height);
}

void Window::leaveFullscreen() noexcept
{
    GLFWwindow* window = m_Handle.get();
    if (m_Windowed.width <= 0 || m_Windowed.height <= 0)
        return;

    if (m_Borderless)
    {
        m_Borderless = false;
        glfwSetWindowPos(window, m_Windowed.x, m_Windowed.y);
        glfwSetWindowSize(window, m_Windowed.width, m_Windowed.height);
    }
    else
    {
        glfwSetWindowMonitor(window, nullptr, m_Windowed.x, m_Windowed.y,
                             m_Windowed.width, m_Windowed.height, GLFW_DONT_CARE);
    }

    if (m_WasMaximized)
        glfwMaximizeWindow(window);
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
    if (!m_Handle || m_OsFrame || isMaximized() || isFullscreen())
        return kEdgeNone;

    double cx = 0.0, cy = 0.0;
    int    w  = 0,   h  = 0;
    glfwGetCursorPos(m_Handle.get(), &cx, &cy);
    glfwGetWindowSize(m_Handle.get(), &w, &h);

    if (cx < 0.0 || cy < 0.0 || cx >= w || cy >= h)
        return kEdgeNone;

    const double border = static_cast<double>(kResizeBorder) * static_cast<double>(m_UiScale);
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
    if (!window || m_OsFrame || m_Drag != Drag::None || isMaximized() || isFullscreen())
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
