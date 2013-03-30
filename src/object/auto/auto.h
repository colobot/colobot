// * This file is part of the COLOBOT source code
// * Copyright (C) 2001-2008, Daniel ROUX & EPSITEC SA, www.epsitec.ch
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

// auto.h

#pragma once


#include "common/global.h"
#include "object/object.h"


class CInstanceManager;
class CRobotMain;
class CSoundInterface;

namespace Ui {
class CDisplayText;
class CInterface;
class CWindow;
} /* Ui */ 

namespace Gfx {

class CEngine;
class CParticle;
class CLightManager;
class CTarrain;
class CWater;
class CCloud;
class CCamera;
class CPlanet;
class CLightning;
} /* Gfx */ 


class CAuto
{
public:
    CAuto(CObject* object);
    virtual ~CAuto();

    virtual void    DeleteObject(bool bAll=false);

    virtual void    Init();
    virtual void    Start(int param);
    virtual bool    EventProcess(const Event &event);
    virtual Error   IsEnded();
    virtual bool    Abort();

    virtual bool    SetType(ObjectType type);
    virtual bool    SetValue(int rank, float value);
    virtual bool    SetString(char *string);

    virtual bool    CreateInterface(bool bSelect);
    virtual Error   GetError();

    virtual bool    GetBusy();
    virtual void    SetBusy(bool bBuse);
    virtual void    InitProgressTotal(float total);
    virtual void    EventProgress(float rTime);

    virtual bool    GetMotor();
    virtual void    SetMotor(bool bMotor);

    virtual bool    Write(char *line);
    virtual bool    Read(char *line);

protected:
    void        CheckInterface(Ui::CWindow *pw, EventType event, bool bState);
    void        EnableInterface(Ui::CWindow *pw, EventType event, bool bState);
    void        VisibleInterface(Ui::CWindow *pw, EventType event, bool bState);
    void        DeadInterface(Ui::CWindow *pw, EventType event, bool bState);
    void        UpdateInterface();
    void        UpdateInterface(float rTime);

protected:
    CInstanceManager*   m_iMan; // TODO: to be removed
    CEventQueue*        m_eventQueue;
    Gfx::CEngine*       m_engine;
    Gfx::CParticle*     m_particle;
    Gfx::CTerrain*      m_terrain;
    Gfx::CWater*        m_water;
    Gfx::CCloud*        m_cloud;
    Gfx::CPlanet*       m_planet;
    Gfx::CLightning*    m_lightning;
    Gfx::CCamera*       m_camera;
    Ui::CInterface*     m_interface;
    Ui::CDisplayText*   m_displayText;
    CRobotMain*         m_main;
    CObject*            m_object;
    CSoundInterface*    m_sound;

    ObjectType  m_type;
    bool        m_bBusy;
    bool        m_bMotor;
    float       m_time;
    float       m_lastUpdateTime;
    float       m_progressTime;
    float       m_progressTotal;
};

