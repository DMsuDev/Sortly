// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "Sortly/Gui/GuiApplication.hpp"

#if defined(_WIN32)
#include <Windows.h>
#endif

namespace {

[[nodiscard]] int runSortly()
{
    Sortly::GuiApplication app;
    return app.Run();
}

} // namespace

int main()
{
    return runSortly();
}

#if defined(_WIN32)

// Entry point for the GUI subsystem (add_executable WIN32).
// WinMain (not wWinMain) so it links with both MSVC and MinGW without -municode.
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    return runSortly();
}

#endif
