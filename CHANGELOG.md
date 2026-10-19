# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Conventional Commits](https://www.conventionalcommits.org/) and this project adheres to [Semantic Versioning](https://semver.org/).

## [Unreleased]

### 🚀 Features

- **fonts**: Add new font files and update icon font path ([1bf8446](https://github.com/DMsuDev/Sortly/commit/1bf8446d0a88a56cde2c062211cfeb522cb8f4ef))

### 🐛 Bug Fixes

- **seedProvider**: Correct return type for random seed generation ([44f9d01](https://github.com/DMsuDev/Sortly/commit/44f9d015863c523c5fc8c950115dd9706cdea50e))

### 🚜 Refactor

- **release**: Disable unit tests in build ([c5ddb7c](https://github.com/DMsuDev/Sortly/commit/c5ddb7c66a0b350ca7502d13499ffdaa1a8b5eec))

- **gui**: Use foundry paths and bundled Inter fonts ([6c493a3](https://github.com/DMsuDev/Sortly/commit/6c493a31557fd9241f11101659ae480650d97d0a))

- **gui**: Split ImGui setup out of GlfwWindow into ImGuiLayer ([b21c366](https://github.com/DMsuDev/Sortly/commit/b21c366e4d2634a1a273a14fe9f736916d9696a1))

- **gui**: Rename `GlfwWindow` to `Window` ([f6e8ced](https://github.com/DMsuDev/Sortly/commit/f6e8ced0f6421d8f76bf99d0fc395af380c680b9))

- **gui**: Tidy Application interface and entry points ([c0d7f60](https://github.com/DMsuDev/Sortly/commit/c0d7f60e333a9b7dcbe3aa5a9145f36be0d775be))

- **cmake**: Disable strict warnings and update asset copying for SortlyGui ([babe963](https://github.com/DMsuDev/Sortly/commit/babe963ce7eaa25bfe7012e7eaea0add424ecf2a))

- **algorithms**: Improve median-of-three pivot selection in QuickSort ([bd45a91](https://github.com/DMsuDev/Sortly/commit/bd45a91ccbba78b0c306ebdb3a32029826a710a3))

## [0.1.0] - 2026-10-16

### 💥 Breaking Changes

- **sorting**: Flatten module layout and fix playback/session bugs ([f6277bc](https://github.com/DMsuDev/Sortly/commit/f6277bc888ec6321bfc1996b7ad328e8d2fd4911))

  Collapse the 7 near-empty folders into Algorithms, Data and Operations, and make the concrete algorithms private (src/), reachable only through SortAlgorithmFactory. SortSession.hpp moves to the module root.

  Fix several bugs found along the way:
  - SortPlayback aliased the session's DataSet through a raw pointer, so playback sorted the dataset itself and later recomputations ran on already-sorted data. It now owns its buffer, which also lets SortSession use the defaulted move.
  - stop() left the buffer half-sorted, and a manual stepForward() left the state Stopped, which made stepBackward() fail.
  - BubbleSort underflowed size_t on empty input; both algorithms now emit exactly one terminal Sorted operation.
  - std::clamp could be called with an inverted range (UB).

  Report failures through return values instead of only logs: add SortSession::ComputeError, validate operation indices once in SortPlayback::load(), and demote expected conditions from ERROR/WARN to DEBUG.

> ⚠️ BubbleSort/CocktailSort headers are no longer public; SortAlgorithmType moved into SortAlgorithmFactory.hpp; getData() returns std::span; SortPlayback::bind()/loadOperations()/restoreInitialState() are replaced by load()/clear(); SortSession::getMutableDataSet() is removed; public headers moved out of detail/, Metrics/, Playback/ and Session/.

### 🏗️ Project Setup

- **project**: Scaffold initial project chore ([07a133e](https://github.com/DMsuDev/Sortly/commit/07a133e32006e785491b5585cb42ddda8108f239))

### 🚀 Features

- **gui**: Add sorting visualizer UI built on ImGui and ImPlot ([ca03abd](https://github.com/DMsuDev/Sortly/commit/ca03abdeb39bc787aee5716607cf12d8c59a0bc8))

  Replace the placeholder window with the real app interface. You can now pick an algorithm and a dataset size, generate a run, and watch the values being sorted bar by bar, with live statistics and play, pause and step controls.

  The app is now split into a reusable part and a thin launcher, and it ships with its icon font. The executable is now called Sortly instead of sortly.

- **sorting**: Add Selection, Insertion, Quick, Merge, Heap and Shell sort ([3f27893](https://github.com/DMsuDev/Sortly/commit/3f27893b1a315a77e5b7f0f41f4a5621c75b5f22))

  - Introduce coroutine-based StepGenerator and GeneratorSort base class, allowing algorithms to yield steps sequentially using C++20 co_yield.
  - Add 6 new sorting algorithms: Selection, Insertion, Quick, Merge, Heap, and Shell sort.
  - Extend SortAlgorithmType and factory mappings, and initialize Vigil globally for test execution while cleaning up redundant tests.

- **gui**: Add multi-algorithm comparison views, playback controls, and summary panel ([70b68a1](https://github.com/DMsuDev/Sortly/commit/70b68a15e4273baf419950f1c3b0abcb417592ba))

  - Add multi-slot comparison support (Single, Compare 2, 3, and 4 views) and update RunController to manage and synchronize multiple algorithm sessions.
  - Implement RunCard, SummaryPanel, and PlaybackControls for real-time comparative metrics, side-by-side array plots, and global animation playback/speed control.
  - Extend UI theme, icons, and layout handling in GuiApplication to support flexible multi-panel docking (Left, Fill, Right).

### 🐛 Bug Fixes

- **gui**: Harden cross-platform behavior and improve run feedback ([ff8f798](https://github.com/DMsuDev/Sortly/commit/ff8f798564abbfe645da3caee19c9a8deab009e2))

  - Scale the UI with the display and locate fonts and icons across platforms
  - Add macOS OpenGL compatibility and keep the initial window within the screen
  - Cap frame delta time to prevent playback jumps after a stalled frame
  - Report session and computation errors directly in the GUI
  - Write diagnostics to a log file when no console is available
  - Defer run replacement between frames and let RunController own its session
  - Reuse the same dataset between runs, with an option to reshuffle it
  - Move panel layout into GuiApplication and compact the statistics cards
  - Apply compiler warnings to the GUI library and remove an unused dependency
  - Restore the executable name to sortly

### 🚜 Refactor

- Remove CLI application and related dependencies. ([ff76a51](https://github.com/DMsuDev/Sortly/commit/ff76a51b744dda21378d26d5ad0df03a8db56601))

- Remove app library and related files ([0e2a28f](https://github.com/DMsuDev/Sortly/commit/0e2a28f719f417cc49d79778f72db42923689f61))

- **changelog**: Update commit parser for project setup to include chore ([f72c5f2](https://github.com/DMsuDev/Sortly/commit/f72c5f2fb41e3b30321ad8ee312c40dd105ae8ef))

### 🛠️ Build System

- **third_party**: Add submodules for Foundry, GLFW, and Vigil ([a21024b](https://github.com/DMsuDev/Sortly/commit/a21024b81a65c8e926e98ccad8f397ee4a77d1a4))

### 🔧 Maintenance

- **workflow**: Include additional paths for build triggers ([a08ee51](https://github.com/DMsuDev/Sortly/commit/a08ee51b748923220e18e57ac8b9433228590252))

- Add initial changelog documenting project changes and versioning ([01a6ae5](https://github.com/DMsuDev/Sortly/commit/01a6ae54cbdebe7b5fa4eedec44d47372d2af614))

<!-- generated by git-cliff -->
