// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#pragma once

struct GLFWwindow;

namespace Sortly {

/// @brief ImGui and ImPlot contexts plus their GLFW/OpenGL3 backends.
/// Must be destroyed before the window whose GL context it uses.
class ImGuiLayer
{
public:
    ImGuiLayer() = default;
    ~ImGuiLayer() { shutdown(); }

    ImGuiLayer(const ImGuiLayer&)            = delete;
    ImGuiLayer& operator=(const ImGuiLayer&) = delete;

    /// @brief The GL context of `window` must be current. On failure nothing stays alive.
    [[nodiscard]] bool init(GLFWwindow* window, float uiScale);

    /// @brief Tears down only what was built, newest first. Safe to call repeatedly.
    void shutdown() noexcept;

    void beginFrame();

    /// @brief ImGui::Render() followed by the OpenGL3 draw call.
    void endFrame();

    [[nodiscard]] bool isReady() const noexcept { return m_Stage == Stage::OpenGlBackend; }

private:
    // Records how far init() got, so shutdown() undoes only what was built.
    enum class Stage { None, ImGui, ImPlot, GlfwBackend, OpenGlBackend };
    
    Stage m_Stage = Stage::None;
};

} // namespace Sortly
