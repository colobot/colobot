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

#pragma once

#include "common/timeutils.h"

#include <stack>

class CSystemUtils;

/**
 * \enum PerformanceCounter
 * \brief Type of counter testing performance
 */
enum PerformanceCounter
{
    PCNT_EVENT_PROCESSING,      //! < event processing (except update events)

    PCNT_UPDATE_ALL,            //! < the whole frame update process
    PCNT_UPDATE_ENGINE,         //! < frame update in CEngine
    PCNT_UPDATE_PARTICLE,       //! < frame update in CParticle
    PCNT_UPDATE_GAME,           //! < frame update in CRobotMain
    PCNT_UPDATE_CBOT,           //! < running CBot code (part of CRobotMain update)

    PCNT_RENDER_ALL,            //! < the whole rendering process
    PCNT_RENDER_PARTICLE_WORLD, //! < rendering the particles in 3D
    PCNT_RENDER_PARTICLE_IFACE, //! < rendering the particles in 2D interface
    PCNT_RENDER_WATER,          //! < rendering the water
    PCNT_RENDER_TERRAIN,        //! < rendering the terrain
    PCNT_RENDER_OBJECTS,        //! < rendering the 3D objects
    PCNT_RENDER_INTERFACE,      //! < rendering 2D interface
    PCNT_RENDER_SHADOW_MAP,     //! < rendering shadow map

    PCNT_SWAP_BUFFERS,          //! < swapping buffers and vsync

    PCNT_ALL,                   //! < all counters together

    PCNT_MAX
};

class CProfiler
{
public:
    static void SetSystemUtils(CSystemUtils* systemUtils);

    static void StartPerformanceCounter(PerformanceCounter counter);
    static void StopPerformanceCounter(PerformanceCounter counter);
    static long long GetPerformanceCounterTime(PerformanceCounter counter);
    static float GetPerformanceCounterFraction(PerformanceCounter counter);

private:
    static void ResetPerformanceCounters();
    static void SavePerformanceCounters();

private:
    static CSystemUtils* m_systemUtils;

    static long long m_performanceCounters[PCNT_MAX];
    static long long m_prevPerformanceCounters[PCNT_MAX];
    static std::stack<TimeUtils::TimeStamp> m_runningPerformanceCounters;
    static std::stack<PerformanceCounter> m_runningPerformanceCountersType;
};


