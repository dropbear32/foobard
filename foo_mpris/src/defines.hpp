// Copyright (c) 2023 Ally Sommers
// This code is licensed under the BSD 3-Clause License. A copy of this license
// is included in the repository.

#pragma once

#define LOG(fmt, ...)                                       \
    do                                                      \
    {                                                       \
        console::printf("[foo_mpris] " fmt, ##__VA_ARGS__); \
    } while (0)

constexpr double USEC_PER_SEC = 1000000.0;
