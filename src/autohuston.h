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

// autohuston.h

#ifndef _AUTOHUSTON_H_
#define _AUTOHUSTON_H_


#include "auto.h"
#include "particule.h"


class CInstanceManager;
class CD3DEngine;
class CParticule;
class CTerrain;
class CCamera;
class CObject;



typedef struct
{
    int             parti;
    ParticuleType   type;
    D3DVECTOR       pos;
    float           dim;
    float           total;
    float           off;
}
HustonLens;


#define HUSTONMAXLENS   20


class CAutoHuston : public CAuto
{
public:
    CAutoHuston(CInstanceManager* iMan, CObject* object);
    ~CAutoHuston();

    void        DeleteObject(BOOL bAll=FALSE);

    void        Init();
    void        Start(int param);
    BOOL        EventProcess(const Event &event);
    BOOL        Abort();
    Error       RetError();

    BOOL        CreateInterface(BOOL bSelect);

protected:

protected:
    float       m_progress;
    float       m_speed;
    HustonLens  m_lens[HUSTONMAXLENS];
    int         m_lensTotal;
};


#endif //_AUTOHUSTON_H_
