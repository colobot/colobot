// * This file is part of the COLOBOT source code
// * Copyright (C) 2001-2008, Daniel ROUX & EPSITEC SA, www.epsitec.ch
// * Copyright (C) 2012, Polish Portal of Colobot (PPC)
// *
// * This program is free software: you can redistribute it and/or modify
// * it under the terms of the GNU General Public License as published by
// * the Free Software Foundation, either version 3 of the License, or
// * (at your option) any later version.
// *
// * This program is distributed in the hope that it will be useful,
// * but WITHOUT ANY WARRANTY; without even the implied warranty of
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// * GNU General Public License for more details.
// *
// * You should have received a copy of the GNU General Public License
// * along with this program. If not, see  http://www.gnu.org/licenses/.

/**
 * \file graphics/engine/lightning.h
 * \brief Lightning rendering - CLightning class (aka blitz)
 */

#pragma once


#include "common/event.h"

#include "math/vector.h"


class CObject;
class CSoundInterface;


// Graphics module namespace
namespace Gfx {

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

protected:
    //! Updates lightning
    bool        EventFrame(const Event &event);
    //! Seeks for the object closest to the lightning
    CObject*    SearchObject(Math::Vector pos);

protected:
    CEngine*          m_engine;
    CTerrain*         m_terrain;
    CCamera*          m_camera;
    CSoundInterface*  m_sound;

    bool            m_lightningExists;
    float           m_sleep;
    float           m_delay;
    float           m_magnetic;

    float           m_speed;
    float           m_progress;
    Math::Vector    m_pos;

    enum LightningPhase
    {
        LP_WAIT,
        LP_FLASH,
    };
    LightningPhase  m_phase;

    static const short FLASH_SEGMENTS = 50;
    Math::Point     m_shift[FLASH_SEGMENTS];
    float           m_width[FLASH_SEGMENTS];
};


} // namespace Gfx

