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

// autosafe.h

#ifndef _AUTOSAFE_H_
#define _AUTOSAFE_H_


#include "auto.h"
#include "misc.h"


class CInstanceManager;
class CD3DEngine;
class CParticule;
class CTerrain;
class CCamera;
class CObject;



enum AutoSafePhase
{
    ASAP_WAIT       = 1,
    ASAP_OPEN       = 2,
    ASAP_FINISH     = 3,
};



class CAutoSafe : public CAuto
{
public:
    CAutoSafe(CInstanceManager* iMan, CObject* object);
    ~CAutoSafe();

    void        DeleteObject(BOOL bAll=FALSE);

    void        Init();
    BOOL        EventProcess(const Event &event);
    Error       RetError();

    BOOL        CreateInterface(BOOL bSelect);

    BOOL        Write(char *line);
    BOOL        Read(char *line);

protected:
    int         CountKeys();
    void        LockKeys();
    void        DownKeys(float progress);
    void        DeleteKeys();
    CObject*    SearchVehicle();

protected:
    AutoSafePhase   m_phase;
    float           m_progress;
    float           m_speed;
    float           m_timeVirus;
    float           m_lastParticule;
    int             m_channelSound;
    BOOL            m_bLock;
    int             m_countKeys;
    float           m_actualAngle;
    float           m_finalAngle;
    BOOL            m_bKey[4];
    D3DVECTOR       m_keyPos[4];
    int             m_keyParti[4];
};


#endif //_AUTOSAFE_H_
