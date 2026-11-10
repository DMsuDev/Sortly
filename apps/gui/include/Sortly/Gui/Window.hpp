// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#pragma once

#include <memory>

struct GLFWwindow;

namespace Sortly {

/// @brief Owns GLFW, one undecorated window and its OpenGL context. Knows nothing about ImGui.
/// With no OS frame it also provides what the frame gave: move, edge resize, minimize/maximize/fullscreen.
/// On Wayland, where windows cannot be moved or resized from code, the OS frame is kept instead.
class Window
{
public:
    struct Rect
    {
        int x = 0, y = 0, width = 0, height = 0;

        [[nodiscard]] bool valid() const noexcept { return width > 0 && height > 0; }
    };

    using Edges = unsigned; // Bit mask of the kEdge* values.
    static constexpr Edges kEdgeNone   = 0;
    static constexpr Edges kEdgeLeft   = 1;
    static constexpr Edges kEdgeRight  = 2;
    static constexpr Edges kEdgeTop    = 4;
    static constexpr Edges kEdgeBottom = 8;

    Window() = default;
    ~Window() { destroy(); }

    Window(const Window&)            = delete;
    Window& operator=(const Window&) = delete;

    /// @brief Sizes are in design pixels. The UI scale is reduced when the minimum size would not
    /// fit the work area, so read uiScale() afterwards.
    /// @return true when the window is shown and its GL context is current. On failure nothing stays alive.
    [[nodiscard]] bool create(int width, int height, const char* title, int minWidth, int minHeight);

    /// @brief Destroys the window, then terminates GLFW. Safe to call repeatedly.
    void destroy() noexcept;

    /// @brief Processes events, advances an active move/resize and re-evaluates the UI scale.
    void pollEvents() noexcept;
    void waitEvents() noexcept; // Use while minimized.

    /// @return true when there is no window or a close was requested.
    [[nodiscard]] bool isCloseRequested() const noexcept;
    [[nodiscard]] bool isMinimized() const noexcept;
    [[nodiscard]] bool isMaximized() const noexcept;
    [[nodiscard]] bool isFullscreen() const noexcept;

    /// @brief True when the OS draws the frame (Wayland). The custom title bar must then hide its
    /// minimize/maximize/close buttons and must not start drags.
    [[nodiscard]] bool hasOsFrame() const noexcept { return m_OsFrame; }

    void requestClose() noexcept;
    void minimize() noexcept;
    void toggleMaximize() noexcept;
    void toggleFullscreen() noexcept; // Borderless: covers the monitor, no video mode switch.

    /// @brief Starts moving the window with the mouse. Call on the click; ignored if a resize is
    /// active, the window is maximized/fullscreen or the OS owns the frame.
    void beginDrag() noexcept { begin(Drag::Move, kEdgeNone); }

    /// @brief Border(s) being resized, or under the cursor. Used to pick the cursor shape.
    [[nodiscard]] Edges activeEdges() const noexcept;

    [[nodiscard]] GLFWwindow* handle() const noexcept { return m_Handle.get(); }

    /// @brief Current UI scale. It changes when the window moves to a monitor with another
    /// resolution or DPI; see consumeScaleChange().
    [[nodiscard]] float uiScale() const noexcept { return m_UiScale; }

    /// @return true once after uiScale() changed. The caller must then rebuild fonts and theme,
    /// between frames.
    [[nodiscard]] bool consumeScaleChange() noexcept;

    /// @brief Sets the viewport to the framebuffer size and clears the color buffer.
    void clear(float r, float g, float b, float a) const;
    void swapBuffers() const noexcept;

private:
    struct Deleter
    {
        void operator()(GLFWwindow* window) const noexcept;
    };

    enum class Drag { None, Move, Resize };

    void begin(Drag drag, Edges edges) noexcept;
    void updateInteraction() noexcept;

    /// Sets the UI scale and the minimum window size (design size * scale).
    void applyScale(float scale) noexcept;

    std::unique_ptr<GLFWwindow, Deleter> m_Handle;
    bool m_OsFrame = false;

    // Scale
    float m_UiScale         = 1.0f;
    int   m_DesignMinWidth  = 1; // Unscaled.
    int   m_DesignMinHeight = 1;
    int   m_MinWidth        = 0; // Scaled, in screen coordinates.
    int   m_MinHeight       = 0;
    bool  m_ScaleDirty      = false; // The window moved or its display scale changed.
    bool  m_ScaleChanged    = false; // uiScale() changed and was not consumed yet.

    // Fullscreen
    Rect m_Windowed;               // Last normal geometry, restored after fullscreen.
    bool m_Borderless   = false;   // Fullscreen done by covering the monitor, not by GLFW.
    bool m_WasMaximized = false;

    // Move / resize
    Drag   m_Drag        = Drag::None;
    Edges  m_ResizeEdges = kEdgeNone;
    Rect   m_Start;                // Geometry when the move/resize began.
    double m_AnchorX     = 0.0;    // Cursor position (screen) when it began.
    double m_AnchorY     = 0.0;
    bool   m_LeftDown    = false;
};

} // namespace Sortly
