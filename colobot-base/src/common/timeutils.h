/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2023, Daniel Roux, EPSITEC SA & TerranovaTeam
 * http://epsitec.ch; http://colobot.info; http://github.com/colobot
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://gnu.org/licenses
 */

/**
 * \file common/timeutils.h
 * \brief Some useful cross-platform operations on timestamps
 */

#pragma once

#include <cassert>
#include <chrono>

namespace TimeUtils
{

enum class TimeUnit
{
    SECONDS,
    MILLISECONDS,
    MICROSECONDS
};

using TimeStamp = std::chrono::time_point<std::chrono::steady_clock>;

//! Linearly interpolates between two timestamps.
inline constexpr TimeStamp Lerp(TimeStamp a, TimeStamp b, float t)
{
    return a + std::chrono::duration_cast<TimeStamp::duration>((b - a) * t);
}

//! Returns the exact (in nanosecond units) difference between two timestamps
/** The difference is \a after - \a before. */
inline constexpr long long ExactDiff(TimeStamp before, TimeStamp after)
{
    return std::chrono::duration_cast<std::chrono::nanoseconds>(after - before).count();
}

//! Returns a difference between two timestamps in given time unit
/** The difference is \a after - \a before. */
inline constexpr float Diff(TimeStamp before, TimeStamp after, TimeUnit unit)
{
    long long exact = ExactDiff(before, after);

    float result = 0.0f;
    if (unit == TimeUnit::SECONDS)
        result = exact * 1e-9;
    else if (unit == TimeUnit::MILLISECONDS)
        result = exact * 1e-6;
    else if (unit == TimeUnit::MICROSECONDS)
        result = exact * 1e-3;
    else
        assert(false);

    return result;
}

} // namespace TimeUtils
