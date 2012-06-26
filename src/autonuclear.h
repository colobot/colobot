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

// autonuclear.h

#ifndef _AUTONUCLEAR_H_
#define _AUTONUCLEAR_H_


#include "auto.h"
#include "misc.h"


class CInstanceManager;
class CD3DEngine;
class CParticule;
class CTerrain;
class CCamera;
class CObject;



enum AutoNuclearPhase
{
    ANUP_STOP       = 1,
    ANUP_WAIT       = 2,
    ANUP_CLOSE      = 3,
    ANUP_GENERATE   = 4,
    ANUP_OPEN       = 5,
};



class CAutoNuclear : public CAuto
{
public:
    CAutoNuclear(CInstanceManager* iMan, CObject* object);
    ~CAutoNuclear();

    void        DeleteObject(BOOL bAll=FALSE);

    void        Init();
    BOOL        EventProcess(const Event &event);
    Error       RetError();

    BOOL        CreateInterface(BOOL bSelect);

    BOOL        Write(char *line);
    BOOL        Read(char *line);

protected:
    CObject*    SearchUranium();
    BOOL        SearchVehicle();
    void        CreatePower();

protected:
    AutoNuclearPhase    m_phase;
    float               m_progress;
    float               m_speed;
    float               m_timeVirus;
    float               m_lastParticule;
    D3DVECTOR           m_pos;
    int                 m_channelSound;
};


#endif //_AUTONUCLEAR_H_
