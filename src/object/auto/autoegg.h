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

// autoegg.h

#pragma once


#include "object/auto/auto.h"


enum AutoEggPhase
{
    AEP_NULL    = 0,
    AEP_DELAY   = 1,
    AEP_INCUB   = 3,
    AEP_ZOOM    = 4,
    AEP_WAIT    = 5,
};



class CAutoEgg : public CAuto
{
public:
    CAutoEgg(CObject* object);
    ~CAutoEgg();

    void        DeleteObject(bool bAll=false);

    void        Init();
    void        Start(int param);
    bool        EventProcess(const Event &event);
    Error       IsEnded();
    Error       GetError();

    bool        SetType(ObjectType type);
    bool        SetValue(int rank, float value);
    bool        SetString(char *string);

    bool        Write(char *line);
    bool        Read(char *line);

protected:
    CObject*    SearchAlien();

protected:
    ObjectType      m_type;
    float           m_value;
    char            m_string[100];
    int             m_param;
    AutoEggPhase    m_phase;
    float           m_progress;
    float           m_speed;
};

