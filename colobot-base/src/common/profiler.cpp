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

#include "common/profiler.h"

#include "common/system/system.h"

#include <cassert>

using TimeUtils::TimeStamp;

CSystemUtils* CProfiler::m_systemUtils = nullptr;
long long CProfiler::m_performanceCounters[PCNT_MAX] = {0};
long long CProfiler::m_prevPerformanceCounters[PCNT_MAX] = {0};
std::stack<TimeStamp> CProfiler::m_runningPerformanceCounters;
std::stack<PerformanceCounter> CProfiler::m_runningPerformanceCountersType;

void CProfiler::SetSystemUtils(CSystemUtils* systemUtils)
{
    m_systemUtils = systemUtils;
}

void CProfiler::StartPerformanceCounter(PerformanceCounter counter)
{
    if (counter == PCNT_ALL)
        ResetPerformanceCounters();

    TimeStamp timeStamp = TimeUtils::GetCurrentTimeStamp();
    m_runningPerformanceCounters.push(timeStamp);
    m_runningPerformanceCountersType.push(counter);
}

void CProfiler::StopPerformanceCounter(PerformanceCounter counter)
{
    assert(m_runningPerformanceCountersType.top() == counter);
    m_runningPerformanceCountersType.pop();

    TimeStamp timeStamp = TimeUtils::GetCurrentTimeStamp();
    m_performanceCounters[counter] += TimeUtils::ExactDiff(m_runningPerformanceCounters.top(), timeStamp);
    m_runningPerformanceCounters.pop();

    if (counter == PCNT_ALL)
        SavePerformanceCounters();
}

long long CProfiler::GetPerformanceCounterTime(PerformanceCounter counter)
{
    return m_prevPerformanceCounters[counter];
}

float CProfiler::GetPerformanceCounterFraction(PerformanceCounter counter)
{
    return static_cast<float>(m_prevPerformanceCounters[counter]) / static_cast<float>(m_prevPerformanceCounters[PCNT_ALL]);
}

void CProfiler::ResetPerformanceCounters()
{
    for (int i = 0; i < PCNT_MAX; ++i)
    {
        m_performanceCounters[i] = 0;
    }
}

void CProfiler::SavePerformanceCounters()
{
    assert(m_runningPerformanceCounters.empty());

    for (int i = 0; i < PCNT_MAX; ++i)
    {
        m_prevPerformanceCounters[i] = m_performanceCounters[i];
    }
}
