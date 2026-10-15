// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "Sortly/Gui/GuiApplication.hpp"

namespace {

int runSortly()
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
#include <Windows.h>

// Entry point for the GUI subsystem (add_executable WIN32)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    return runSortly();
}
#endif
