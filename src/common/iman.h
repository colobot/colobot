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

/**
 * \file common/iman.h
 * \brief Instance manager for managed classes
 */

#pragma once


#include "common/singleton.h"


/**
 * \enum ManagedClassType
 * \brief Type of class managed by CInstanceManager
 */

/*
 * TODO: Non-unique classes have already been removed.
 *  The other class instances along with CInstanceManager will be removed in due course.
 */
enum ManagedClassType
{
    //! CObject
    CLASS_OBJECT        = 0,
    //! CPhysics
    CLASS_PHYSICS       = 1,
    //! CBrain
    CLASS_BRAIN         = 2,
    //! Gfx::CPyro
    CLASS_PYRO          = 3,

    //! Maximum (number of managed classes)
    CLASS_MAX           = 4
};


/**
 * \struct ManagedClassInstances
 * \brief Instances of class managed by CInstanceManager
 */
struct ManagedClassInstances
{
    int     maxCount;
    int     usedCount;
    void**  instances;
};

/**
 * \class CInstanceManager
 * \brief Manager for instances of certain classes
 *
 * Instance manager (often shortened to iMan) allows to register instances of
 * classes and search them.
 */
class CInstanceManager : public CSingleton<CInstanceManager>
{
public:
    CInstanceManager();
    virtual ~CInstanceManager();

    //! Remove all managed instances
    void    Flush();
    //! Removes instances of one type of class
    void    Flush(ManagedClassType classType);
    //! Registers new instance of class type
    bool    AddInstance(ManagedClassType classType, void* instance, int max=1);
    //! Deletes the registered instance of class type
    bool    DeleteInstance(ManagedClassType classType, void* instance);
    //! Seeks a class instance of given type
    void*   SearchInstance(ManagedClassType classType, int rank=0);

protected:
    //! Fills holes in instance table
    void    Compress(ManagedClassType classType);

protected:
    ManagedClassInstances m_table[CLASS_MAX];
};


