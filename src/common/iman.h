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

// iman.h

#pragma once

#include "common/singleton.h"
#include "common/misc.h"



struct BaseClass
{
    int     totalPossible;
    int     totalUsed;
    void**  classPointer;
};



class CInstanceManager : public CSingleton<CInstanceManager>
{
public:
    CInstanceManager();
    ~CInstanceManager();

    void    Flush();
    void    Flush(ClassType classType);
    bool    AddInstance(ClassType classType, void* pointer, int max=1);
    bool    DeleteInstance(ClassType classType, void* pointer);
    void*   SearchInstance(ClassType classType, int rank=0);

    static CInstanceManager& GetInstance();
    static CInstanceManager* GetInstancePointer();

protected:
    void    Compress(ClassType classType);

protected:
    BaseClass   m_table[CLASS_MAX];
};


