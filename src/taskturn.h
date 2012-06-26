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

// taskturn.h

#ifndef _TASKTURN_H_
#define _TASKTURN_H_


#include "misc.h"


class CInstanceManager;
class CTerrain;
class CBrain;
class CPhysics;
class CObject;


class CTaskTurn : public CTask
{
public:
    CTaskTurn(CInstanceManager* iMan, CObject* object);
    ~CTaskTurn();

    BOOL    EventProcess(const Event &event);

    Error   Start(float angle);
    Error   IsEnded();

protected:

protected:
    float       m_angle;
    float       m_startAngle;
    float       m_finalAngle;
    BOOL        m_bLeft;
    BOOL        m_bError;
};


#endif //_TASKTURN_H_
