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

// autojostle.h

#ifndef _AUTOJOSTLE_H_
#define _AUTOJOSTLE_H_


#include "auto.h"
#include "misc.h"


class CInstanceManager;
class CD3DEngine;
class CParticule;
class CTerrain;
class CCamera;
class CObject;



class CAutoJostle : public CAuto
{
public:
    CAutoJostle(CInstanceManager* iMan, CObject* object);
    ~CAutoJostle();

    void        DeleteObject(BOOL bAll=FALSE);

    void        Init();
    void        Start(int param, float force);
    BOOL        EventProcess(const Event &event);
    Error       IsEnded();

protected:

protected:
    float           m_force;
    float           m_progress;
    float           m_speed;
    float           m_lastParticule;
    Error           m_error;
};


#endif //_AUTOJOSTLE_H_
