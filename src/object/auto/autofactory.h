// * This file is part of the COLOBOT source code
// * Copyright (C) 2001-2008, Daniel ROUX & EPSITEC SA, www.epsitec.ch
// * Copyright (C) 2012 Polish Portal of Colobot (PPC)
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

#pragma once


#include "object/auto/auto.h"



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
    CAutoFactory(CObject* object);
    ~CAutoFactory();

    void        DeleteObject(bool bAll=false);

    void        Init();
    bool        EventProcess(const Event &event);

    bool        CreateInterface(bool bSelect);

    bool        Write(char *line);
    bool        Read(char *line);

protected:
    void        UpdateInterface();
    void        UpdateButton(Ui::CWindow *pw, EventType event, bool bBusy);

    CObject*    SearchFret();
    bool        NearestVehicle();
    bool        CreateVehicle();
    CObject*    SearchVehicle();

    void        SoundManip(float time, float amplitude, float frequency);

protected:
    AutoFactoryPhase    m_phase;
    float               m_progress;
    float               m_speed;
    float               m_lastParticle;
    Math::Vector            m_fretPos;
    int                 m_channelSound;
};

