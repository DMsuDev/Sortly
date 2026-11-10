// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "Sortly/Gui/Window.hpp"
#include "Sortly/Gui/System.hpp"

#include <GLFW/glfw3.h>

#include <vigil/vigil.h>
#include <foundry/memory/scope_guard.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <utility>

namespace Sortly {

namespace {

using Rect = Window::Rect;

constexpr float kMinUiScale   = 0.6f;
constexpr float kScaleEpsilon = 0.01f;
constexpr int   kAreaPercent  = 90; // A window and the UI scale use at most 90% of the work area.
constexpr int   kResizeBorder = 6;  // Invisible resize border, in design pixels.

// Part of an extent that windows and the UI scale may use.
int areaLimit(int extent)
{
    return extent * kAreaPercent / 100;
}

int scaled(int value, float scale)
{
    return static_cast<int>(std::lround(static_cast<float>(value) * scale));
}

void place(GLFWwindow* window, const Rect& r)
{
    glfwSetWindowPos(window, r.x, r.y);
    glfwSetWindowSize(window, r.width, r.height);
}

Rect workAreaOf(GLFWmonitor* monitor)
{
    Rect area;
    if (monitor)
        glfwGetMonitorWorkarea(monitor, &area.x, &area.y, &area.width, &area.height);
    return area;
}

// On macOS the backend already handles the framebuffer scale and window
// coordinates are in points; elsewhere the UI and size limits must be scaled.
float queryUiScale([[maybe_unused]] GLFWwindow* window)
{
#if defined(OS_MACOS)
    return 1.0f;
#else
    float scaleX = 1.0f, scaleY = 1.0f;
    glfwGetWindowContentScale(window, &scaleX, &scaleY);
    return std::max(scaleX, scaleY); // One scale for all UI-related dimensions.
#endif
}

// Largest scale not above `wanted` at which the design minimum still fits the work area.
// max/min instead of std::clamp: clamp is undefined when its bounds are inverted.
float fitScale(float wanted, int minWidth, int minHeight, const Rect& area)
{
    if (!area.valid())
        return wanted;

    const float fitW = static_cast<float>(areaLimit(area.width))  / static_cast<float>(std::max(1, minWidth));
    const float fitH = static_cast<float>(areaLimit(area.height)) / static_cast<float>(std::max(1, minHeight));
    return std::max(std::min({ wanted, fitW, fitH }), std::min(kMinUiScale, wanted));
}

// Design-pixel size -> screen pixels, capped to the usable area and centered in it.
Rect initialFrame(int width, int height, float scale, const Rect& area)
{
    Rect f{ 0, 0, std::max(1, scaled(width, scale)), std::max(1, scaled(height, scale)) };
    if (!area.valid())
        return f;

    const float fit = std::min({ 1.0f,
        static_cast<float>(areaLimit(area.width))  / static_cast<float>(f.width),
        static_cast<float>(areaLimit(area.height)) / static_cast<float>(f.height) });

    f.width  = std::max(1, scaled(f.width,  fit));
    f.height = std::max(1, scaled(f.height, fit));
    f.x      = area.x + (area.width  - f.width)  / 2;
    f.y      = area.y + (area.height - f.height) / 2;
    return f;
}

// Monitor showing most of the window; the primary one as a fallback.
GLFWmonitor* monitorOf(GLFWwindow* window, bool osFrame)
{
    if (osFrame)
        return glfwGetPrimaryMonitor(); // Wayland reports no window position (and logs errors if asked).

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

// Window-relative cursor + window position = screen position, stable while the window moves.
void cursorOnScreen(GLFWwindow* window, double& x, double& y)
{
    int    wx = 0,   wy = 0;
    double cx = 0.0, cy = 0.0;
    glfwGetWindowPos(window, &wx, &wy);
    glfwGetCursorPos(window, &cx, &cy);
    x = wx + cx;
    y = wy + cy;
}

// Logs the context information and publishes it to System. Needs a current context.
bool queryOpenGLContext(GLFWwindow* window)
{
    const auto* vendor   = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
    const auto* renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
    const auto* version  = reinterpret_cast<const char*>(glGetString(GL_VERSION));

    if (!vendor || !renderer || !version)
    {
        VIGIL_LOG_ERROR("Failed to query OpenGL context information");
        return false;
    }

    VIGIL_LOG_INFO("OpenGL Vendor: {}", vendor);
    VIGIL_LOG_INFO("OpenGL Renderer: {}", renderer);
    VIGIL_LOG_INFO("OpenGL Version String: {}", version);

    glfwGetError(nullptr); // Discard stale errors so only ours is reported below.
    const int major    = glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MAJOR);
    const int minor    = glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MINOR);
    const int revision = glfwGetWindowAttrib(window, GLFW_CONTEXT_REVISION);

    const char* errorDescription = nullptr;
    const int   error = glfwGetError(&errorDescription);
    if (error != GLFW_NO_ERROR || major <= 0 || minor < 0 || revision < 0)
    {
        VIGIL_LOG_ERROR("Failed to query OpenGL context version (GLFW error 0x{:05X}: {})",
                        error, errorDescription ? errorDescription : "invalid version");
        return false;
    }

    VIGIL_LOG_INFO("OpenGL Version: v{}.{}.{}", major, minor, revision);

    Sortly::System::impl::setGPUVendor(vendor);
    Sortly::System::impl::setGLRenderer(renderer);
    Sortly::System::impl::setGLVersion(SemanticVersion(
        static_cast<std::uint32_t>(major),
        static_cast<std::uint32_t>(minor),
        static_cast<std::uint32_t>(revision)));

    return true;
}

} // namespace

// ================================================
// LIFETIME
// ================================================

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
        if (error == GLFW_PLATFORM_ERROR && desc && std::strstr(desc, "Wayland"))
            return;

        VIGIL_LOG_WARN("GLFW error 0x{:05X}: {}", error, desc);
    });

    if (!glfwInit())
    {
        VIGIL_LOG_ERROR("GLFW initialization failed");
        return false;
    }

    // Wayland cannot move or resize windows from code, so there the OS keeps its frame.
#if GLFW_VERSION_MAJOR > 3 || (GLFW_VERSION_MAJOR == 3 && GLFW_VERSION_MINOR >= 4)
    m_OsFrame = glfwGetPlatform() == GLFW_PLATFORM_WAYLAND;
#endif

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#if defined(OS_MACOS)
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif
    glfwWindowHint(GLFW_AUTO_ICONIFY, GLFW_FALSE); // Native fullscreen (Wayland) must not minimize on focus loss.
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

    // Scale and geometry. Sizes are given in design pixels; the window is hidden, so resizing is invisible.
    const Rect area = workAreaOf(glfwGetPrimaryMonitor()); // The first window is centered on the primary monitor.

    m_DesignMinWidth  = std::max(1, minWidth);
    m_DesignMinHeight = std::max(1, minHeight);
    applyScale(fitScale(queryUiScale(window), m_DesignMinWidth, m_DesignMinHeight, area));
    VIGIL_LOG_INFO("Initial UI scale: {:.2f}", m_UiScale);

    m_Windowed = initialFrame(width, height, m_UiScale, area);
    if (area.valid())
        place(window, m_Windowed);
    else
        glfwSetWindowSize(window, m_Windowed.width, m_Windowed.height);

    // Moving the window or changing its display scale re-evaluates the UI scale in pollEvents().
    const auto markDirty = [](GLFWwindow* w, auto, auto) {
        if (auto* self = static_cast<Window*>(glfwGetWindowUserPointer(w)))
            self->m_ScaleDirty = true;
    };
    glfwSetWindowPosCallback(window, markDirty);
    glfwSetWindowContentScaleCallback(window, markDirty);

    // OpenGL context
    glfwMakeContextCurrent(window);
    if (glfwGetCurrentContext() != window)
    {
        VIGIL_LOG_ERROR("Failed to make the OpenGL context current");
        return false;
    }
    glfwSwapInterval(1); // VSync.

    if (!queryOpenGLContext(window))
        return false;

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

    m_OsFrame         = false;
    m_UiScale         = 1.0f;
    m_DesignMinWidth  = 1;
    m_DesignMinHeight = 1;
    m_ScaleDirty      = false;
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

    GLFWwindow* window = m_Handle.get();
    glfwPollEvents();

    // Remember the last normal geometry, to restore it after fullscreen.
    if (!isMaximized() && !isFullscreen() && !isMinimized())
    {
        if (!m_OsFrame)
            glfwGetWindowPos(window, &m_Windowed.x, &m_Windowed.y);
        glfwGetWindowSize(window, &m_Windowed.width, &m_Windowed.height);
    }

    updateInteraction();

    // Waits for the end of a drag: the scale is decided where the window is dropped.
    if (!m_ScaleDirty || m_Drag != Drag::None || isMinimized())
        return;

    m_ScaleDirty = false;

    const Rect  area  = workAreaOf(monitorOf(window, m_OsFrame));
    const float scale = fitScale(queryUiScale(window), m_DesignMinWidth, m_DesignMinHeight, area);
    if (std::abs(scale - m_UiScale) <= kScaleEpsilon)
        return;

    const float ratio = scale / m_UiScale;
    VIGIL_LOG_INFO("UI scale {:.2f} -> {:.2f}", m_UiScale, scale);
    applyScale(scale);
    m_ScaleChanged = true;

    // The OS keeps maximized and fullscreen windows on their screen.
    if (m_OsFrame || !area.valid() || isMaximized() || isFullscreen())
        return;

    // Same proportions on the new scale, never bigger than the monitor allows, and fully inside it.
    Rect r;
    glfwGetWindowPos(window, &r.x, &r.y);
    glfwGetWindowSize(window, &r.width, &r.height);

    r.width  = std::max(m_MinWidth,  std::min(scaled(r.width,  ratio), areaLimit(area.width)));
    r.height = std::max(m_MinHeight, std::min(scaled(r.height, ratio), areaLimit(area.height)));
    r.x      = std::max(area.x, std::min(r.x, area.x + area.width  - r.width));
    r.y      = std::max(area.y, std::min(r.y, area.y + area.height - r.height));
    place(window, r);
}

void Window::waitEvents() noexcept
{
    if (m_Handle)
        glfwWaitEvents();
}

void Window::applyScale(float scale) noexcept
{
    m_UiScale   = scale;
    m_MinWidth  = scaled(m_DesignMinWidth,  scale);
    m_MinHeight = scaled(m_DesignMinHeight, scale);
    glfwSetWindowSizeLimits(m_Handle.get(), m_MinWidth, m_MinHeight, GLFW_DONT_CARE, GLFW_DONT_CARE);
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

    GLFWwindow* window = m_Handle.get();
    m_Drag = Drag::None;

    if (isFullscreen())
    {
        if (!m_Windowed.valid())
            return;

        if (std::exchange(m_Borderless, false))
            place(window, m_Windowed);
        else
            glfwSetWindowMonitor(window, nullptr, m_Windowed.x, m_Windowed.y,
                                 m_Windowed.width, m_Windowed.height, GLFW_DONT_CARE);

        if (m_WasMaximized)
            glfwMaximizeWindow(window);
        return;
    }

    GLFWmonitor*       monitor = monitorOf(window, m_OsFrame);
    const GLFWvidmode* mode    = monitor ? glfwGetVideoMode(monitor) : nullptr;
    if (!mode)
        return;

    m_WasMaximized = isMaximized(); // m_Windowed already holds the last normal geometry.

    Rect screen{ 0, 0, mode->width, mode->height };
    glfwGetMonitorPos(monitor, &screen.x, &screen.y);

    if (m_OsFrame)
    {
        // Wayland cannot position windows: the compositor's fullscreen with the current mode switches nothing.
        glfwSetWindowMonitor(window, monitor, screen.x, screen.y, screen.width, screen.height, mode->refreshRate);
        return;
    }

    m_Borderless = true; // Before moving, so pollEvents() ignores the monitor-sized geometry.
    if (m_WasMaximized)
        glfwRestoreWindow(window); // A maximized window ignores explicit sizes.
    place(window, screen);
}

// ================================================
// MOVE / RESIZE
// ================================================

Window::Edges Window::activeEdges() const noexcept
{
    if (m_Drag == Drag::Resize)
        return m_ResizeEdges;
    if (m_Drag == Drag::Move || !m_Handle || m_OsFrame || isMaximized() || isFullscreen())
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

void Window::begin(Drag drag, Edges edges) noexcept
{
    GLFWwindow* window = m_Handle.get();
    // A resize already started on this click wins over the title-bar move.
    if (!window || m_OsFrame || m_Drag != Drag::None || isMaximized() || isFullscreen())
        return;

    glfwGetWindowPos(window, &m_Start.x, &m_Start.y);
    glfwGetWindowSize(window, &m_Start.width, &m_Start.height);
    cursorOnScreen(window, m_AnchorX, m_AnchorY);
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
        const Edges edges = activeEdges(); // No drag is active on the press, so this is the border under the cursor.
        if (edges != kEdgeNone)
            begin(Drag::Resize, edges);
    }

    if (m_Drag == Drag::None)
        return;

    double cx = 0.0, cy = 0.0;
    cursorOnScreen(window, cx, cy);
    const int dx = static_cast<int>(cx - m_AnchorX);
    const int dy = static_cast<int>(cy - m_AnchorY);

    Rect r = m_Start;
    if (m_Drag == Drag::Move)
    {
        r.x += dx;
        r.y += dy;
    }
    else
    {
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
    }

    place(window, r);
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
