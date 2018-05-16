/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2018, Daniel Roux, EPSITEC SA & TerranovaTeam
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
 * \file graphics/engine/lightning.h
 * \brief Lightning rendering - CLightning class (aka blitz)
 */

#pragma once

#include "math/point.h"
#include "math/vector.h"

#include <vector>

class CObject;
class CSoundInterface;
struct Event;


// Graphics module namespace
namespace Gfx
{

class CEngine;
class CTerrain;
class CCamera;

//! Radius of lightning protection
const float LTNG_PROTECTION_RADIUS = 200.0f;


/**
 * \class CLightning
 * \brief Lightning effect renderer
 *
 * TODO: documentation
 */
class CLightning
{
public:
    CLightning(CEngine* engine);
    ~CLightning();

    //! Triggers lightning
    bool        Create(float sleep, float delay, float magnetic);

    //! Removes lightning
    void        Flush();

    //! Gives the status of lightning
    bool        GetStatus(float &sleep, float &delay, float &magnetic, float &progress);
    //! Specifies the status of lightning
    bool        SetStatus(float sleep, float delay, float magnetic, float progress);

    //! Management of an event
    bool        EventProcess(const Event &event);

    //! Draws lightning
    void        Draw();

    //! Shoots lightning strike at given position
    void        StrikeAtPos(Math::Vector pos);

protected:
    //! Updates lightning
    bool        EventFrame(const Event &event);
    //! Seeks for the object closest to the lightning
    CObject*    SearchObject(Math::Vector pos);

protected:
    CEngine*          m_engine = nullptr;
    CTerrain*         m_terrain = nullptr;
    CCamera*          m_camera = nullptr;
    CSoundInterface*  m_sound = nullptr;

    bool            m_lightningExists = false;
    float           m_sleep = 0.0f;
    float           m_delay = 0.0f;
    float           m_magnetic = 0.0f;

    float           m_speed = 0.0f;
    float           m_progress = 0.0f;
    Math::Vector    m_pos;

    enum class LightningPhase
    {
        Wait,
        Flash,
    };
    LightningPhase  m_phase = LightningPhase::Wait;

    struct LightningSegment
    {
        Math::Point shift;
        float width = 0.0f;
    };
    std::vector<LightningSegment> m_segments;
};


} // namespace Gfx

