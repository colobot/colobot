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

// motionhuman.h

#ifndef _MOTIONHUMAN_H_
#define _MOTIONHUMAN_H_


#include "motion.h"
#include "misc.h"


class CInstanceManager;
class CEngine;
class CLight;
class CParticule;
class CTerrain;
class CCamera;
class CBrain;
class CPhysics;
class CObject;


#define MH_MARCH        0
#define MH_MARCHTAKE    1
#define MH_TURN         2
#define MH_STOP         3
#define MH_FLY          4
#define MH_SWIM         5
#define MH_SPEC         6

#define MHS_FIRE        0
#define MHS_GUN         1
#define MHS_TAKE        2
#define MHS_TAKEOTHER   3
#define MHS_TAKEHIGH    4
#define MHS_UPRIGHT     5
#define MHS_WIN         6
#define MHS_LOST        7
#define MHS_DEADg       8
#define MHS_DEADg1      9
#define MHS_DEADg2      10
#define MHS_DEADg3      11
#define MHS_DEADg4      12
#define MHS_DEADw       13
#define MHS_FLAG        14
#define MHS_SATCOM      15


class CMotionHuman : public CMotion
{
public:
    CMotionHuman(CInstanceManager* iMan, CObject* object);
    ~CMotionHuman();

    void    DeleteObject(BOOL bAll=FALSE);
    BOOL    Create(D3DVECTOR pos, float angle, ObjectType type, float power);
    BOOL    EventProcess(const Event &event);
    Error   SetAction(int action, float time=0.2f);

    void    StartDisplayPerso();
    void    StopDisplayPerso();

protected:
    void    CreatePhysics(ObjectType type);
    BOOL    EventFrame(const Event &event);

protected:
    int         m_partiReactor;
    float       m_armMember;
    float       m_armTimeAbs;
    float       m_armTimeAction;
    float       m_armTimeSwim;
    short       m_armAngles[3*3*3*3*7 + 3*3*3*16];
    int         m_armTimeIndex;
    int         m_armPartIndex;
    int         m_armMemberIndex;
    int         m_armLastAction;
    BOOL        m_bArmStop;
    float       m_lastSoundMarch;
    float       m_lastSoundHhh;
    float       m_time;
    float       m_tired;
    BOOL        m_bDisplayPerso;
};


#endif //_MOTIONHUMAN_H_
