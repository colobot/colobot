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

#pragma once


#include "object/motion/motion.h"



enum MotionSpiderAction
{
    MS_MARCH        = 0,
    MS_STOP         = 1,
    MS_SPEC         = 2
};

enum MotionSpiderSpecialAction
{
    MSS_BURN        = 0,
    MSS_RUIN        = 1,
    MSS_EXPLO       = 2,
    MSS_BACK1       = 3,
    MSS_BACK2       = 4,
    MSS_BACK3       = 5
};


class CMotionSpider : public CMotion
{
public:
    CMotionSpider(CObject* object);
    ~CMotionSpider();

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
    short       m_armAngles[3*4*4*3*3 + 3*4*4*6];
    int         m_armTimeIndex;
    int         m_armPartIndex;
    int         m_armMemberIndex;
    int         m_armLastAction;
    bool        m_bArmStop;
    float       m_lastParticle;
};

