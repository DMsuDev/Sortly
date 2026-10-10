// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "CliApplication.hpp"

#include <Sortly/Sorting/Algorithms/SortAlgorithmFactory.hpp>
#include <Sortly/version.hpp>

#include <vigil/vigil.h>
#include <CLI/CLI.hpp>

#include <cstdint>
#include <iostream>
#include <map>
#include <string>

namespace Sortly {

int CliApplication::onRun(int argc, char** argv)
{
    const std::map<std::string, SortAlgorithmType> algorithms{
        { "bubble",   SortAlgorithmType::BubbleSort   },
        { "cocktail", SortAlgorithmType::CocktailSort }
    };

    CLI::App cli{ "Sortly CLI - sorting algorithm visualizer" };
    cli.set_version_flag("-v,--version", version::current.to_string());
    cli.require_subcommand(1);
    argv = cli.ensure_utf8(argv);

    auto* list = cli.add_subcommand("list", "List the available algorithms");
    auto* run  = cli.add_subcommand("run",  "Generate a sorting run and report its statistics");

    auto     type  = SortAlgorithmType::BubbleSort;
    int      count = 128;
    uint32_t seed  = 0;

    run->add_option("-a,--algorithm", type, "Algorithm to run")
       ->transform(CLI::CheckedTransformer(algorithms, CLI::ignore_case));
    run->add_option("-n,--count", count, "Number of elements")
       ->check(CLI::Range(2, 4096))
       ->capture_default_str();
    auto* seedOpt = run->add_option("-s,--seed", seed, "RNG seed (random if omitted)");

    try { cli.parse(argc, argv); }
    catch (const CLI::ParseError& e) { return cli.exit(e); }

    if (list->parsed())
    {
        for (const auto t : SortAlgorithmFactory::getAvailableAlgorithms())
            std::cout << SortAlgorithmFactory::toString(t) << '\n';
        return 0;
    }

    if (seedOpt->count() > 0)
        sessions().setSeedProvider(SeedProvider::globalSeed(seed));

    DataSetConfig config;
    config.count = count;

    auto* session = sessions().createSession(type, config);
    if (!session || !session->preparePlayback())
    {
        VIGIL_LOG_ERROR("Run failed");
        return 1;
    }

    // computeOperations() already logs operation count and time at Info level.
    return 0;
}

} // namespace Sortly
