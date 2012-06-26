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

#ifndef _AUTO_H_
#define _AUTO_H_


#include "object.h"


class CInstanceManager;
class CD3DEngine;
class CParticule;
class CLight;
class CTerrain;
class CWater;
class CCloud;
class CPlanet;
class CBlitz;
class CCamera;
class CObject;
class CInterface;
class CRobotMain;
class CDisplayText;
class CWindow;
class CSound;




class CAuto
{
public:
    CAuto(CInstanceManager* iMan, CObject* object);
    ~CAuto();

    virtual void    DeleteObject(BOOL bAll=FALSE);

    virtual void    Init();
    virtual void    Start(int param);
    virtual BOOL    EventProcess(const Event &event);
    virtual Error   IsEnded();
    virtual BOOL    Abort();

    virtual BOOL    SetType(ObjectType type);
    virtual BOOL    SetValue(int rank, float value);
    virtual BOOL    SetString(char *string);

    virtual BOOL    CreateInterface(BOOL bSelect);
    virtual Error   RetError();

    virtual BOOL    RetBusy();
    virtual void    SetBusy(BOOL bBuse);
    virtual void    InitProgressTotal(float total);
    virtual void    EventProgress(float rTime);

    virtual BOOL    RetMotor();
    virtual void    SetMotor(BOOL bMotor);

    virtual BOOL    Write(char *line);
    virtual BOOL    Read(char *line);

protected:
    void        CheckInterface(CWindow *pw, EventMsg event, BOOL bState);
    void        EnableInterface(CWindow *pw, EventMsg event, BOOL bState);
    void        VisibleInterface(CWindow *pw, EventMsg event, BOOL bState);
    void        DeadInterface(CWindow *pw, EventMsg event, BOOL bState);
    void        UpdateInterface();
    void        UpdateInterface(float rTime);

protected:
    CInstanceManager* m_iMan;
    CEvent*         m_event;
    CD3DEngine*     m_engine;
    CParticule*     m_particule;
    CLight*         m_light;
    CTerrain*       m_terrain;
    CWater*         m_water;
    CCloud  *       m_cloud;
    CPlanet *       m_planet;
    CBlitz*         m_blitz;
    CCamera*        m_camera;
    CInterface*     m_interface;
    CRobotMain*     m_main;
    CDisplayText*   m_displayText;
    CObject*        m_object;
    CSound*         m_sound;

    ObjectType  m_type;
    BOOL        m_bBusy;
    BOOL        m_bMotor;
    float       m_time;
    float       m_lastUpdateTime;
    float       m_progressTime;
    float       m_progressTotal;
};


#endif //_AUTO_H_
