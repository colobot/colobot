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

// autoresearch.h

#ifndef _AUTORESEARCH_H_
#define _AUTORESEARCH_H_


#include "auto.h"
#include "misc.h"


class CInstanceManager;
class CD3DEngine;
class CParticule;
class CTerrain;
class CCamera;
class CObject;



enum AutoResearchPhase
{
    ALP_WAIT        = 1,
    ALP_SEARCH      = 2,    // research in progress
};



class CAutoResearch : public CAuto
{
public:
    CAutoResearch(CInstanceManager* iMan, CObject* object);
    ~CAutoResearch();

    void        DeleteObject(BOOL bAll=FALSE);

    void        Init();
    BOOL        EventProcess(const Event &event);
    Error       RetError();

    BOOL        CreateInterface(BOOL bSelect);

    BOOL        Write(char *line);
    BOOL        Read(char *line);

protected:
    void        UpdateInterface();
    void        UpdateInterface(float rTime);
    void        OkayButton(CWindow *pw, EventMsg event);
    BOOL        TestResearch(EventMsg event);
    void        SetResearch(EventMsg event);
    void        FireStopUpdate(float progress, BOOL bLightOn);

protected:
    AutoResearchPhase   m_phase;
    float               m_progress;
    float               m_speed;
    float               m_timeVirus;
    float               m_lastUpdateTime;
    float               m_lastParticule;
    EventMsg            m_research;
    int                 m_partiStop[6];
    int                 m_channelSound;
};


#endif //_AUTORESEARCH_H_
