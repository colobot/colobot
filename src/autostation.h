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

// autostation.h

#ifndef _AUTOSTATION_H_
#define _AUTOSTATION_H_


#include "auto.h"
#include "misc.h"


class CInstanceManager;
class CD3DEngine;
class CParticule;
class CTerrain;
class CCamera;
class CObject;



class CAutoStation : public CAuto
{
public:
    CAutoStation(CInstanceManager* iMan, CObject* object);
    ~CAutoStation();

    void        DeleteObject(BOOL bAll=FALSE);

    void        Init();
    BOOL        EventProcess(const Event &event);
    Error       RetError();

    BOOL        CreateInterface(BOOL bSelect);

protected:
    void        UpdateInterface(float rTime);

    CObject*    SearchVehicle();

protected:
    float           m_progress;
    float           m_speed;
    float           m_timeVirus;
    float           m_lastUpdateTime;
    float           m_lastParticule;
    int             m_soundChannel;
    D3DVECTOR       m_fretPos;
    BOOL            m_bLastVirus;
    float           m_energyVirus;
};


#endif //_AUTOSTATION_H_
