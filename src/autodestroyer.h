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

// autodestroyer.h

#ifndef _AUTODESTROYER_H_
#define _AUTODESTROYER_H_


#include "misc.h"


class CInstanceManager;
class CD3DEngine;
class CParticule;
class CTerrain;
class CCamera;
class CObject;



enum AutoDestroyerPhase
{
    ADEP_WAIT       = 1,    // expected metal
    ADEP_DOWN       = 2,    // down the cover
    ADEP_REPAIR     = 3,    // built the vehicle
    ADEP_UP         = 4,    // up the cover
};



class CAutoDestroyer : public CAuto
{
public:
    CAutoDestroyer(CInstanceManager* iMan, CObject* object);
    ~CAutoDestroyer();

    void        DeleteObject(BOOL bAll=FALSE);

    void        Init();
    BOOL        EventProcess(const Event &event);
    Error       RetError();

    BOOL        CreateInterface(BOOL bSelect);

    BOOL        Write(char *line);
    BOOL        Read(char *line);

protected:
    CObject*    SearchPlastic();
    BOOL        SearchVehicle();

protected:
    AutoDestroyerPhase  m_phase;
    float           m_progress;
    float           m_speed;
    float           m_timeVirus;
    float           m_lastParticule;
    BOOL            m_bExplo;
};


#endif //_AUTODESTROYER_H_
