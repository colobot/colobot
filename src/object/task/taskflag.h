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
// taskflag.h

#pragma once


#include "object/task/task.h"
#include "object/object.h"

#include "math/vector.h"



enum TaskFlagOrder
{
    TFL_CREATE  = 0,    // sets
    TFL_DELETE  = 1,    // removes
};



class CTaskFlag : public CTask
{
public:
    CTaskFlag(CObject* object);
    ~CTaskFlag();

    bool        EventProcess(const Event &event);

    Error       Start(TaskFlagOrder order, int rank);
    Error       IsEnded();
    bool        Abort();

protected:
    Error       CreateFlag(int rank);
    Error       DeleteFlag();
    CObject*    SearchNearest(Math::Vector pos, ObjectType type);
    int         CountObject(ObjectType type);

protected:
    TaskFlagOrder   m_order;
    float           m_time;
    bool            m_bError;
};

