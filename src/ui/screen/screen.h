// * This file is part of the COLOBOT source code
// * Copyright (C) 2014 Polish Portal of Colobot (PPC)
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

#pragma once

#include "object/robotmain.h"
#include "common/event.h"

#include <set>

namespace Ui {

class CScreen
{
public:
    virtual ~CScreen()
    {
    }
    virtual void Start() = 0;
    virtual void Stop() = 0;
    virtual bool EventProcess(const Event &event) = 0;
    virtual void ChangePhase(Phase phase) = 0;
    
    std::set<Phase> m_enabledPhases;
    bool m_enabled = false;
};

}

