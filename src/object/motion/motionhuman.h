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

#pragma once


#include "object/motion/motion.h"



enum MotionHumanAction
{
    MH_MARCH        = 0,
    MH_MARCHTAKE    = 1,
    MH_TURN         = 2,
    MH_STOP         = 3,
    MH_FLY          = 4,
    MH_SWIM         = 5,
    MH_SPEC         = 6
};

enum MotionHumanSpecialAction
{
    MHS_FIRE        = 0,
    MHS_GUN         = 1,
    MHS_TAKE        = 2,
    MHS_TAKEOTHER   = 3,
    MHS_TAKEHIGH    = 4,
    MHS_UPRIGHT     = 5,
    MHS_WIN         = 6,
    MHS_LOST        = 7,
    MHS_DEADg       = 8,
    MHS_DEADg1      = 9,
    MHS_DEADg2      = 10,
    MHS_DEADg3      = 11,
    MHS_DEADg4      = 12,
    MHS_DEADw       = 13,
    MHS_FLAG        = 14,
    MHS_SATCOM      = 15
};


class CMotionHuman : public CMotion
{
public:
    CMotionHuman(CObject* object);
    ~CMotionHuman();

    void    DeleteObject(bool bAll=false);
    bool    Create(Math::Vector pos, float angle, ObjectType type, float power);
    bool    EventProcess(const Event &event);
    Error   SetAction(int action, float time=0.2f);

    void    StartDisplayPerso();
    void    StopDisplayPerso();

protected:
    void    CreatePhysics(ObjectType type);
    bool    EventFrame(const Event &event);

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
    bool        m_bArmStop;
    float       m_lastSoundMarch;
    float       m_lastSoundHhh;
    float       m_time;
    float       m_tired;
    bool        m_bDisplayPerso;
};

