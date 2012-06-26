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

// autofactory.h

#ifndef _AUTOFACTORY_H_
#define _AUTOFACTORY_H_


#include "auto.h"


class CInstanceManager;
class CD3DEngine;
class CParticule;
class CTerrain;
class CCamera;
class CObject;



enum AutoFactoryPhase
{
    AFP_WAIT        = 1,    // expected metal
    AFP_CLOSE_S     = 2,    // closes doors (shift)
    AFP_CLOSE_T     = 3,    // closes doors (turn)
    AFP_BUILD       = 4,    // building the vehicle
    AFP_OPEN_T      = 5,    // opens the doors (turn)
    AFP_OPEN_S      = 6,    // opens the doors (shift)
    AFP_ADVANCE     = 7,    // advance at the door
};



class CAutoFactory : public CAuto
{
public:
    CAutoFactory(CInstanceManager* iMan, CObject* object);
    ~CAutoFactory();

    void        DeleteObject(BOOL bAll=FALSE);

    void        Init();
    BOOL        EventProcess(const Event &event);

    BOOL        CreateInterface(BOOL bSelect);

    BOOL        Write(char *line);
    BOOL        Read(char *line);

protected:
    void        UpdateInterface();
    void        UpdateButton(CWindow *pw, EventMsg event, BOOL bBusy);

    CObject*    SearchFret();
    BOOL        NearestVehicle();
    BOOL        CreateVehicle();
    CObject*    SearchVehicle();

    void        SoundManip(float time, float amplitude, float frequency);

protected:
    AutoFactoryPhase    m_phase;
    float               m_progress;
    float               m_speed;
    float               m_lastParticule;
    D3DVECTOR           m_fretPos;
    int                 m_channelSound;
};


#endif //_AUTOFACTORY_H_
