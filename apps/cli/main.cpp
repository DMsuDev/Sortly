// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "CliApplication.hpp"

int main(int argc, char** argv)
{
    Sortly::CliApplication app;
    return app.Run(argc, argv);
}
