<div align="center">
  <picture>
    <img width="320" alt="sortly-logo" src="docs/brand/sortly-logo.png">
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

![C++23](https://img.shields.io/badge/C%2B%2B-23-00599C?style=flat&logo=cplusplus&logoColor=white)
[![License MIT](https://img.shields.io/github/license/DMsuDev/Sortly?style=flat&logo=open-source-initiative&label=License&logoColor=white)](https://github.com/DMsuDev/Sortly/blob/main/LICENSE)
[![Sortly Version](https://img.shields.io/github/v/release/DMsuDev/Sortly?style=flat&label=Version&color=blue)](https://github.com/DMsuDev/Sortly/releases/latest)
[![Sortly Build](https://github.com/DMsuDev/Sortly/actions/workflows/sortly-build.yml/badge.svg?event=push)](https://github.com/DMsuDev/Sortly/actions/workflows/sortly-build.yml)

![Platform Linux](https://img.shields.io/badge/Platform-Linux-2d2d2d?style=flat&logo=linux&logoColor=white)
![Platform Windows](https://img.shields.io/badge/Platform-Windows-0078D6?style=flat&logo=windows&logoColor=white)

</div>

## Overview

**Sortly** is a modular **C++** sorting algorithm visualizer based on a precomputed step-by-step execution model.

It generates all intermediate states of sorting algorithms in advance and replays them through an interactive UI, enabling smooth visualization, comparison, and analysis of different techniques.

Built with **GLFW**, **OpenGL**, **Dear ImGui** and **ImPlot**, it focuses on clarity, extensibility, and ease of experimentation with sorting algorithms.

> [!NOTE]
> This project is currently under active development. The application is reasonably stable at this stage, but future releases may introduce significant changes.

<div align="center">
  <img width="760" alt="Sortly comparing four sorting algorithms" src="docs/images/preview.png">
</div>

## Algorithms

| Algorithm      | Complexity  | Idea                                                          |
| -------------- | ----------- | ------------------------------------------------------------- |
| Bubble Sort    | O(n²)       | Swaps neighbors that are out of order, pass after pass.       |
| Cocktail Sort  | O(n²)       | Bidirectional variation of Bubble Sort.                       |
| Selection Sort | O(n²)       | Repeatedly moves the smallest remaining element into place.   |
| Insertion Sort | O(n²)       | Grows a sorted prefix by sliding each element into its place. |
| Quick Sort     | O(n log n)  | Partitions around a pivot (Lomuto) and sorts each side.       |
| Merge Sort     | O(n log n)  | Bottom-up merging of sorted runs through an auxiliary buffer. |
| Heap Sort      | O(n log n)  | Builds a max-heap, then moves its root to the end.            |
| Shell Sort     | O(n log² n) | Insertion sort over shrinking gaps.                           |

## How it works

Sortly separates *computing* a sort from *showing* it:

1. An algorithm runs once over a copy of the data and records every step as an operation (compare, swap, overwrite, pivot).
2. The recorder builds each operation from the real buffer, so recorded values never disagree with what happened.
3. A playback replays those operations over its own copy of the data, forward or backward, at any speed.

## Contributing

Issues and pull requests are welcome. If you plan a big change, please open an issue first.

- Commits follow [Conventional Commits](https://www.conventionalcommits.org/) (`feat`, `fix`, `refactor`, ...).
- Install the [pre-commit](https://pre-commit.com) hooks with `pre-commit install --hook-type pre-commit --hook-type commit-msg`.

## License

This project is licensed under the **MIT License**.<br>
See the [LICENSE](LICENSE) file for more information.

## Notices

Sortly bundles the following fonts in [`assets/fonts/`](assets/fonts). The license text of each one is shipped next to the fonts, in [`assets/fonts/licenses/`](assets/fonts/licenses).

| Component        | Files                                     | License                          | License file                                                                   |
| ---------------- | ----------------------------------------- | -------------------------------- | ------------------------------------------------------------------------------ |
| Inter            | `Inter-Regular.ttf`, `Inter-SemiBold.ttf` | SIL Open Font License 1.1        | [`Inter-license.txt`](assets/fonts/licenses/Inter-license.txt)                 |
| JetBrains Mono   | `JetBrainsMono-Medium.ttf`                | SIL Open Font License 1.1        | [`JetBrainsMono-OFL.txt.txt`](assets/fonts/licenses/JetBrainsMono-OFL.txt.txt) |
| Lucide icon font | `lucide.ttf`                              | ISC (portions MIT, from Feather) | [`Lucide-LICENSE.txt`](assets/fonts/licenses/Lucide-LICENSE.txt)               |
