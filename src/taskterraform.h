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

// taskterraform.h

#ifndef _TASKSTERRAFORM_H_
#define _TASKSTERRAFORM_H_


#include "misc.h"
#include "d3dengine.h"


class CInstanceManager;
class CTerrain;
class CBrain;
class CPhysics;
class CObject;



enum TaskTerraPhase
{
    TTP_CHARGE  = 1,    // charge of energy
    TTP_DOWN    = 2,    // down
    TTP_TERRA   = 3,    // strike
    TTP_UP      = 4,    // up
};



class CTaskTerraform : public CTask
{
public:
    CTaskTerraform(CInstanceManager* iMan, CObject* object);
    ~CTaskTerraform();

    BOOL        EventProcess(const Event &event);

    Error       Start();
    Error       IsEnded();
    BOOL        Abort();

protected:
    BOOL        Terraform();

protected:
    TaskTerraPhase  m_phase;
    float           m_progress;
    float           m_speed;
    float           m_time;
    float           m_lastParticule;
    int             m_soundChannel;
    BOOL            m_bError;
    D3DVECTOR       m_terraPos;
};


#endif //_TASKSTERRAFORM_H_
