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

// motionspider.h

#ifndef _MOTIONSPIDER_H_
#define _MOTIONSPIDER_H_


#include "motion.h"


class CInstanceManager;
class CEngine;
class CLight;
class CParticule;
class CTerrain;
class CCamera;
class CBrain;
class CPhysics;
class CObject;


#define MS_MARCH        0
#define MS_STOP         1
#define MS_SPEC         2

#define MSS_BURN        0
#define MSS_RUIN        1
#define MSS_EXPLO       2
#define MSS_BACK1       3
#define MSS_BACK2       4
#define MSS_BACK3       5


class CMotionSpider : public CMotion
{
public:
    CMotionSpider(CInstanceManager* iMan, CObject* object);
    ~CMotionSpider();

    void    DeleteObject(BOOL bAll=FALSE);
    BOOL    Create(D3DVECTOR pos, float angle, ObjectType type, float power);
    BOOL    EventProcess(const Event &event);

protected:
    void    CreatePhysics();
    BOOL    EventFrame(const Event &event);

protected:
    float       m_armMember;
    float       m_armTimeAbs;
    float       m_armTimeMarch;
    float       m_armTimeAction;
    short       m_armAngles[3*4*4*3*3 + 3*4*4*6];
    int         m_armTimeIndex;
    int         m_armPartIndex;
    int         m_armMemberIndex;
    int         m_armLastAction;
    BOOL        m_bArmStop;
    float       m_lastParticule;
};


#endif //_MOTIONSPIDER_H_
