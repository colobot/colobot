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

#include "common/error.h"
#include "common/event.h"

#include "object/object_type.h"

#include <string>

class CRobotMain;
class CSoundInterface;
class CLevelParserLine;
class COldObject;

namespace Ui
{
class CInterface;
class CWindow;
} // namespace Ui

namespace Gfx
{

class CEngine;
class CParticle;
class CTerrain;
class CWater;
class CCloud;
class CCamera;
class CPlanet;
class CLightning;
} // namespace Gfx


class CAuto
{
public:
    CAuto(COldObject* object);
    virtual ~CAuto();

    virtual void    DeleteObject(bool bAll=false);

    virtual void    Init();
    virtual void    Start(int param);
    virtual bool    EventProcess(const Event &event);
    virtual Error   IsEnded();
    virtual bool    Abort();

    virtual Error   StartAction(int param);

    virtual bool    SetType(ObjectType type);
    virtual bool    SetValue(int rank, float value);
    virtual bool    SetString(const std::string& string);

    virtual bool    CreateInterface(bool bSelect);
    virtual Error   GetError();

    virtual bool    GetBusy();
    virtual void    SetBusy(bool busy);
    virtual void    InitProgressTotal(float total);
    virtual void    EventProgress(float rTime);

    virtual bool    GetMotor();
    virtual void    SetMotor(bool bMotor);

    virtual bool    Write(CLevelParserLine* line);
    virtual bool    Read(CLevelParserLine* line);

protected:
    void        CheckInterface(Ui::CWindow *pw, EventType event, bool bState);
    void        EnableInterface(Ui::CWindow *pw, EventType event, bool bState);
    void        VisibleInterface(Ui::CWindow *pw, EventType event, bool bState);
    void        DeadInterface(Ui::CWindow *pw, EventType event, bool bState);
    void        UpdateInterface();
    void        UpdateInterface(float rTime);

protected:
    CEventQueue*        m_eventQueue = nullptr;
    Gfx::CEngine*       m_engine = nullptr;
    Gfx::CParticle*     m_particle = nullptr;
    Gfx::CTerrain*      m_terrain = nullptr;
    Gfx::CWater*        m_water = nullptr;
    Gfx::CCloud*        m_cloud = nullptr;
    Gfx::CPlanet*       m_planet = nullptr;
    Gfx::CLightning*    m_lightning = nullptr;
    Gfx::CCamera*       m_camera = nullptr;
    Ui::CInterface*     m_interface = nullptr;
    CRobotMain*         m_main = nullptr;
    COldObject*         m_object = nullptr;
    CSoundInterface*    m_sound = nullptr;

    ObjectType  m_type = OBJECT_NULL;
    bool        m_bBusy = false;
    bool        m_bMotor = false;
    float       m_time = 0.0f;
    float       m_lastUpdateTime = 0.0f;
    float       m_progressTime = 0.0f;
    float       m_progressTotal = 0.0f;
};
