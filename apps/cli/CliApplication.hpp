// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#pragma once

#include <Sortly/App/Application.hpp>

namespace Sortly {

class CliApplication final : public Application
{
public:
    CliApplication() : Application({ .name = "Sortly-CLI" }) {}

protected:
    int onRun(int argc, char** argv) override;
};

} // namespace Sortly
