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

// motiontoto.h

#ifndef _MOTIONTOTO_H_
#define _MOTIONTOTO_H_


#include "struct.h"
#include "object.h"
#include "motion.h"


class CInstanceManager;
class CEngine;
class CLight;
class CParticule;
class CTerrain;
class CCamera;
class CBrain;
class CPhysics;


#define MT_ERROR        0
#define MT_WARNING      1
#define MT_INFO         2
#define MT_MESSAGE      3


class CMotionToto : public CMotion
{
public:
    CMotionToto(CInstanceManager* iMan, CObject* object);
    ~CMotionToto();

    void    DeleteObject(BOOL bAll=FALSE);
    BOOL    Create(D3DVECTOR pos, float angle, ObjectType type, float power);
    BOOL    EventProcess(const Event &event);
    Error   SetAction(int action, float time=0.2f);
    void    SetLinkType(ObjectType type);

    void    StartDisplayInfo();
    void    StopDisplayInfo();
    void    SetMousePos(FPOINT pos);

protected:
    BOOL    EventFrame(const Event &event);

protected:
    float       m_time;
    float       m_lastMotorParticule;
    BOOL        m_bDisplayInfo;
    BOOL        m_bQuickPos;
    BOOL        m_bStartAction;
    float       m_speedAction;
    float       m_clownRadius;
    float       m_clownDelay;
    float       m_clownTime;
    float       m_blinkTime;
    float       m_blinkProgress;
    int         m_soundChannel;
    ObjectType  m_type;
    FPOINT      m_mousePos;
};


#endif //_MOTIONTOTO_H_
