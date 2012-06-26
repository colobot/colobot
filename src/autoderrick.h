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

// autoderrick.h

#ifndef _AUTODERRICK_H_
#define _AUTODERRICK_H_


#include "object.h"
#include "misc.h"


class CInstanceManager;
class CD3DEngine;
class CParticule;
class CTerrain;
class CCamera;
class CObject;




enum AutoDerrickPhase
{
    ADP_WAIT        = 1,
    ADP_EXCAVATE        = 2,    // down the drill
    ADP_ASCEND      = 3,    // up the drill
    ADP_EXPORT      = 4,    // exports matter
    ADP_ISFREE      = 5,    // expected material loss
};



class CAutoDerrick : public CAuto
{
public:
    CAutoDerrick(CInstanceManager* iMan, CObject* object);
    ~CAutoDerrick();

    void        DeleteObject(BOOL bAll=FALSE);

    void        Init();
    BOOL        EventProcess(const Event &event);
    Error       RetError();

    BOOL        CreateInterface(BOOL bSelect);

    BOOL        Write(char *line);
    BOOL        Read(char *line);

protected:
    CObject*    SearchFret();
    BOOL        SearchFree(D3DVECTOR pos);
    void        CreateFret(D3DVECTOR pos, float angle, ObjectType type, float height);
    BOOL        ExistKey();

protected:
    AutoDerrickPhase    m_phase;
    float               m_progress;
    float               m_speed;
    float               m_timeVirus;
    float               m_lastParticule;
    float               m_lastTrack;
    D3DVECTOR           m_fretPos;
    int                 m_soundChannel;
    BOOL                m_bSoundFall;
};


#endif //_AUTODERRICK_H_
