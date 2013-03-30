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

// motionant.h

#pragma once


#include "object/motion/motion.h"



enum MotionAntAction
{
    MA_MARCH    = 0,
    MA_STOP     = 1,
    MA_SPEC     = 2
};

enum MotionAntSpecialAction
{
    MAS_PREPARE     = 0,
    MAS_FIRE        = 1,
    MAS_TERMINATE   = 2,
    MAS_BURN        = 3,
    MAS_RUIN        = 4,
    MAS_BACK1       = 5,
    MAS_BACK2       = 6,
    MAS_BACK3       = 7
};


class CMotionAnt : public CMotion
{
public:
    CMotionAnt(CObject* object);
    ~CMotionAnt();

    void    DeleteObject(bool bAll=false);
    bool    Create(Math::Vector pos, float angle, ObjectType type, float power);
    bool    EventProcess(const Event &event);

protected:
    void    CreatePhysics();
    bool    EventFrame(const Event &event);

protected:
    float       m_armMember;
    float       m_armTimeAbs;
    float       m_armTimeMarch;
    float       m_armTimeAction;
    short       m_armAngles[3*3*3*3*3 + 3*3*3*8];
    int         m_armTimeIndex;
    int         m_armPartIndex;
    int         m_armMemberIndex;
    int         m_armLastAction;
    bool        m_bArmStop;
    float       m_lastParticle;
};

