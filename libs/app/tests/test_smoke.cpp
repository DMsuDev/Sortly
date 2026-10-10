// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include <Sortly/version.hpp>

#include <gtest/gtest.h>

TEST(SortlyVersion, CurrentVersionCanBeFormatted)
{
    EXPECT_FALSE(Sortly::version::current.to_string().empty());
}
