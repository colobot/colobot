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

// autoinfo.h

#ifndef _AUTOINFO_H_
#define _AUTOINFO_H_


#include "auto.h"
#include "misc.h"


class CInstanceManager;
class CD3DEngine;
class CParticule;
class CTerrain;
class CCamera;
class CObject;



enum AutoInfoPhase
{
    AIP_WAIT        = 1,
    AIP_EMETTE      = 2,
    AIP_RECEIVE     = 3,
    AIP_ERROR       = 4,
};



class CAutoInfo : public CAuto
{
public:
    CAutoInfo(CInstanceManager* iMan, CObject* object);
    ~CAutoInfo();

    void        DeleteObject(BOOL bAll=FALSE);

    void        Init();
    void        Start(int param);
    BOOL        EventProcess(const Event &event);
    Error       RetError();

    BOOL        CreateInterface(BOOL bSelect);

    BOOL        Write(char *line);
    BOOL        Read(char *line);

protected:
    void        UpdateInterface(float rTime);
    void        UpdateList();
    void        UpdateListVirus();

protected:
    AutoInfoPhase   m_phase;
    float           m_progress;
    float           m_speed;
    float           m_timeVirus;
    float           m_lastParticule;
    D3DVECTOR       m_goal;
    BOOL            m_bLastVirus;
};


#endif //_AUTOINFO_H_
