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

// autojostle.h

#pragma once


#include "object/auto/auto.h"



class CAutoJostle : public CAuto
{
public:
    CAutoJostle(CObject* object);
    ~CAutoJostle();

    void        DeleteObject(bool bAll=false);

    void        Init();
    void        Start(int param, float force);
    bool        EventProcess(const Event &event);
    Error       IsEnded();

private:
    // Overriden to avoid warning about hiding virtual function
    virtual void Start(int param) override;

protected:
    float           m_force;
    float           m_progress;
    float           m_speed;
    float           m_lastParticle;
    Error           m_error;
};

