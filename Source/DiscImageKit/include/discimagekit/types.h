// Minimal type aliases for DiscImageKit consumers.
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <chrono>
#include <cstdint>

using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

using s8 = std::int8_t;
using s16 = std::int16_t;
using s32 = std::int32_t;
using s64 = std::int64_t;

using Clock = std::chrono::steady_clock;
using TimePoint = Clock::time_point;
using DT = Clock::duration;
using DT_us = std::chrono::duration<double, std::micro>;
using DT_ms = std::chrono::duration<double, std::milli>;
using DT_s = std::chrono::duration<double, std::ratio<1>>;
