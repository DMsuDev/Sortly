<div align="center">
  <picture>
    <img width="320" alt="sortly-logo" src=".github/assets/logo.png">
  </picture>
  <div>
    <strong>Sorting Algorithm Visualizer built with C++</strong>
  </div>

  <div>
    Cross-platform • Precomputed visualization • Minimal UI • Educational & interactive
  </div>

  <br>
</div>

<div align="center">

![C++20](https://img.shields.io/badge/C%2B%2B-20-00599C?style=flat&logo=cplusplus&logoColor=white)
[![License MIT](https://img.shields.io/github/license/DMsuDev/Sortly?style=flat&logo=open-source-initiative&label=License&logoColor=white)](https://github.com/DMsuDev/Sortly/blob/master/LICENSE)
[![Sortly Version](https://img.shields.io/github/v/release/DMsuDev/Sortly?style=flat&label=Version&color=blue)](https://github.com/DMsuDev/Sortly/releases/latest)
[![Sortly Build](https://github.com/DMsuDev/Sortly/actions/workflows/sortly-build.yml/badge.svg?event=push)](https://github.com/DMsuDev/Sortly/actions/workflows/sortly-build.yml)

![Platform Linux](https://img.shields.io/badge/Platform-Linux-2d2d2d?style=flat&logo=linux&logoColor=white)
![Platform Windows](https://img.shields.io/badge/Platform-Windows-0078D6?style=flat&logo=windows&logoColor=white)

</div>

## Overview

**Sortly** is a modular **C++** sorting algorithm visualizer based on a precomputed step-by-step execution model.

It generates all intermediate states of sorting algorithms in advance and replays them through an interactive UI, enabling smooth visualization, comparison, and analysis of different techniques.

Built with **SFML** and **ImGui**, it focuses on clarity, extensibility, and ease of experimentation with sorting algorithms.

> [!NOTE]
> This project is currently under active development. The application is reasonably stable at this stage, but future releases may introduce significant changes.

## Project layout

Sortly is split into a reusable core and two independent applications:

- `include/Sortly` and `src/Sorting` contain `Sortly::Core`, including datasets,
  sorting algorithms, operation recording, playback, and session management.
- `src/App/GUI` and `src/App/CLI` contain application-level code that is
  distinct from the sorting core.
- `gui` contains the default `sortly` entry point and its ImGui, ImPlot, GLFW,
  and OpenGL integration.
- `cli` contains the `sortly-cli` entry point and command-line behavior.
- Each application owns its tests under its own `tests` directory.

The core target does not link against GUI libraries. GUI dependencies are
configured only when the GUI application is added, so the CLI remains a
console-only consumer of `Sortly::Core`.

## License

This project is licensed under the **MIT License**.<br>
See the [LICENSE](LICENSE) file for more information.
