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

#include <chrono>

namespace TimeUtils
{

enum class TimeUnit
{
    SECONDS,
    MILLISECONDS,
    MICROSECONDS
};

using TimeStamp = std::chrono::time_point<std::chrono::high_resolution_clock>;

//! Linearly interpolates between two timestamps.
TimeStamp Lerp(TimeStamp a, TimeStamp b, float t);

//! Returns a difference between two timestamps in given time unit
/** The difference is \a after - \a before. */
float Diff(TimeStamp before, TimeStamp after, TimeUnit unit = TimeUnit::SECONDS);

//! Returns the exact (in nanosecond units) difference between two timestamps
/** The difference is \a after - \a before. */
long long ExactDiff(TimeStamp before, TimeStamp after);

} // namespace TimeUtils
