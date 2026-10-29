# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Conventional Commits](https://www.conventionalcommits.org/) and this project adheres to [Semantic Versioning](https://semver.org/).

## [0.3.0] - 2026-10-29

### 🚀 Features

- **gui**: Add cornerGlow widget helper ([0ab28f3](https://github.com/DMsuDev/Sortly/commit/0ab28f34b7c2ecb8c14ebd666f16c82c559d1ac8))

- **gui**: Add BarMotion to animate bars between slots ([344d531](https://github.com/DMsuDev/Sortly/commit/344d531eba29834a8366aaae56387f06d02dec7e))

  Each bar keeps its identity: a Swap exchanges the slots of two bars and each one slides toward its new slot. Overwrites only change heights.

- **gui**: Paint plot bars by state ([95cb494](https://github.com/DMsuDev/Sortly/commit/95cb494468c1c14b47b9f2cc8f6ae7bc2cf61ee1))

  - Classify every bar as idle, placed, pivot or active and draw one batch per state
  - Remove the outline around active bars
  - Reuse the batch buffers between frames

- **gui**: Animate bars and track in-place values per run ([955f7c4](https://github.com/DMsuDev/Sortly/commit/955f7c4f46253c4b5227fdd4fa80b1c4e8c7d83f))

  - Each Run owns its BarMotion and a sorted copy of the dataset
  - Add the smoothMotion setting and RunController::setSmooth
  - Skip setDelayMs work when the delay did not change

- **gui**: Start in single view with 64 elements ([7ceb363](https://github.com/DMsuDev/Sortly/commit/7ceb363dbc50a7dcf3f6b03c2ee63fff9c0e06a5))

- **gui**: Add styled tooltips and popups ([1ab3fbc](https://github.com/DMsuDev/Sortly/commit/1ab3fbc335c9ffec2bfe33de92bea5a7ff666fb7))

  Replace ImGui::SetTooltip with ui::setTooltip and add a slim themed scrollbar.

- **gui**: Paint bars green with a sweep when a run finishes ([aa08a68](https://github.com/DMsuDev/Sortly/commit/aa08a6834b88a0f65a880dc6572ab066f14017cd))

  Add Run::sweepProgress(), a Sorted bar state and faint Y guides; remove the axes option.

- **gui**: Make RunCard responsive with side, inline and popup stats ([ba21f8d](https://github.com/DMsuDev/Sortly/commit/ba21f8d9d555865d9d65ad48ad2830541c63f263))

  Add maxUsefulCardHeight() and drop the 1-1-1 layout for three algorithms.

### 🐛 Bug Fixes

- **gui**: Keep bars idle until the run starts ([3045175](https://github.com/DMsuDev/Sortly/commit/30451755c1ee7d4ddd5d712b849bc5fbc7c8583a))

  - Split drawRunCard into header, body, stats column and footer
  - Add the corner glow and drive the statistics from kMetrics

- **gui**: Maintain pivot color during comparisons in ArrayPlot ([985f5ef](https://github.com/DMsuDev/Sortly/commit/985f5ef6fef73186124f13e360f1d2de07100060))

- **tests**: Include private headers for white-box tests in CMakeLists.txt ([4f855a9](https://github.com/DMsuDev/Sortly/commit/4f855a9eb952584a05d8048448f7b1934c6abdf5))

### 🚜 Refactor

- **build**: Simplify library source file management in CMakeLists.txt ([cf0ebeb](https://github.com/DMsuDev/Sortly/commit/cf0ebeb2332480c4133c73e4d1ec98de7c408a41))

### 🛠️ Build System

- **cmake**: Set WIN32_EXECUTABLE property for SortlyGui target ([4e27119](https://github.com/DMsuDev/Sortly/commit/4e27119cae13fc13f3a253d32657220d3cc65265))

## [0.2.0] - 2026-10-24

### 💥 Breaking Changes

- **sorting**: Read pivot value from the buffer and add clearPivot ([df68ead](https://github.com/DMsuDev/Sortly/commit/df68eada712078de6f67790c6aae131d1c2891ac))

### 🚀 Features

- **fonts**: Add new font files and update icon font path ([1bf8446](https://github.com/DMsuDev/Sortly/commit/1bf8446d0a88a56cde2c062211cfeb522cb8f4ef))

- **sorting**: Add SortPlayback::getPivotIndex ([35bcecb](https://github.com/DMsuDev/Sortly/commit/35bcecba3614b61f197f3ffc7ba1cd0e85fb4d16))

- **gui**: Keep the pivot bar marked while its partition runs ([bd9642d](https://github.com/DMsuDev/Sortly/commit/bd9642dd12213a85c2b0f00d4f92119a17665d97))

- **gui**: Replace the OS frame with a custom title bar ([f97d688](https://github.com/DMsuDev/Sortly/commit/f97d68876fdd67f360915117191e6b2252448a7f))

  Adds move, edge resize, minimize/maximize/fullscreen (F11). The view selector moves from the side panel to the top bar.

### 🐛 Bug Fixes

- **seedProvider**: Correct return type for random seed generation ([44f9d01](https://github.com/DMsuDev/Sortly/commit/44f9d015863c523c5fc8c950115dd9706cdea50e))

- **workflow**: Update branch name from `master` to `main` in CI build ([1063efa](https://github.com/DMsuDev/Sortly/commit/1063efadbc21afc215a17ae254c4eb59562484e0))

- **algorithms**: Clear the QuickSort pivot mark after each partition ([d8a5f11](https://github.com/DMsuDev/Sortly/commit/d8a5f11690f8d1591b35f0e2c4524cdcc6845b39))

- **release**: Ship assets with the binary and validate the tag against VERSION ([c530be9](https://github.com/DMsuDev/Sortly/commit/c530be9a27ae041f31022a9197a27d4453fabd81))

  The package copied Sortly instead of sortly and left out assets/, and passed an old SORTLY_BUILD_SHARED flag.

### 🚜 Refactor

- **release**: Disable unit tests in build ([c5ddb7c](https://github.com/DMsuDev/Sortly/commit/c5ddb7c66a0b350ca7502d13499ffdaa1a8b5eec))

- **gui**: Use foundry paths and bundled Inter fonts ([6c493a3](https://github.com/DMsuDev/Sortly/commit/6c493a31557fd9241f11101659ae480650d97d0a))

- **gui**: Split ImGui setup out of GlfwWindow into ImGuiLayer ([b21c366](https://github.com/DMsuDev/Sortly/commit/b21c366e4d2634a1a273a14fe9f736916d9696a1))

- **gui**: Rename `GlfwWindow` to `Window` ([f6e8ced](https://github.com/DMsuDev/Sortly/commit/f6e8ced0f6421d8f76bf99d0fc395af380c680b9))

- **gui**: Tidy Application interface and entry points ([c0d7f60](https://github.com/DMsuDev/Sortly/commit/c0d7f60e333a9b7dcbe3aa5a9145f36be0d775be))

- **cmake**: Disable strict warnings and update asset copying for SortlyGui ([babe963](https://github.com/DMsuDev/Sortly/commit/babe963ce7eaa25bfe7012e7eaea0add424ecf2a))

- **algorithms**: Improve median-of-three pivot selection in QuickSort ([bd45a91](https://github.com/DMsuDev/Sortly/commit/bd45a91ccbba78b0c306ebdb3a32029826a710a3))

- **gui**: Add ScopedFont/ScopedColor and shared UiKit widgets ([b2ab064](https://github.com/DMsuDev/Sortly/commit/b2ab0643c2736aed5ae27ec72d33b44004277666))

  Also adds glyphButton, verticalDivider and compactButtonHeight, and fixes formatCount's int-based loop.

### 🔧 Maintenance

- **build**: Fix stale path filters and keep every main run ([0c11a75](https://github.com/DMsuDev/Sortly/commit/0c11a75544944f181688e2268e1be4ff1fc647ed))

- **codeql**: Analyze apps and libs, run on push, PR and monthly ([24713e6](https://github.com/DMsuDev/Sortly/commit/24713e6335a58db05b6dfa75043a2e57da3012e5))

- **dependabot**: Track git submodules and use the chore(deps) prefix ([b4e95f9](https://github.com/DMsuDev/Sortly/commit/b4e95f9539da6631e04c67e70a04efd74c8b230e))

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
