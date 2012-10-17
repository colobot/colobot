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

// TODO: remove unnecessary, refactor to singletons, move to CRobotMain, keep others?

enum ManagedClassType
{
    //! CEventQueue
    CLASS_EVENT         = 1,
    //! Ui::CInterface
    CLASS_INTERFACE     = 2,
    //! CRobotMain
    CLASS_MAIN          = 3,
    //! Gfx::CEngine
    CLASS_ENGINE        = 4,
    //! Gfx::CTerrain
    CLASS_TERRAIN       = 5,
    //! CObject
    CLASS_OBJECT        = 6,
    //! CPhysics
    CLASS_PHYSICS       = 7,
    //! CBrain
    CLASS_BRAIN         = 8,
    //! Gfx::CCamera
    CLASS_CAMERA        = 9,
    //! Gfx::CLightManager
    CLASS_LIGHT         = 10,
    //! Gfx::CParticle
    CLASS_PARTICULE     = 11,
    //! CAuto; TODO: remove (unused)
    CLASS_AUTO          = 12,
    //! Ui::CDisplayText
    CLASS_DISPLAYTEXT   = 13,
    //! Gfx::CPyro
    CLASS_PYRO          = 14,
    //! Ui::CScript; TODO: remove (unused)
    CLASS_SCRIPT        = 15,
    //! Gfx::CText
    CLASS_TEXT          = 16,
    //! Ui::CStudio, Ui::CDisplayText; TODO: remove (unused)
    CLASS_STUDIO        = 17,
    //! Gfx::CWater
    CLASS_WATER         = 18,
    //! Gfx::CCloud; TODO: remove (unused)
    CLASS_CLOUD         = 19,
    //! CMotion; TODO: remove (unused)
    CLASS_MOTION        = 20,
    //! CSoundInterface
    CLASS_SOUND         = 21,
    //! Gfx::CPlanet
    CLASS_PLANET        = 22,
    //! CTaskManager; TODO: remove (unused)
    CLASS_TASKMANAGER   = 23,
    //! Ui::CMainDialog; TODO: remove (unused)
    CLASS_DIALOG        = 24,
    //! Ui::CMainMap; TODO: remove (unused)
    CLASS_MAP           = 25,
    //! Ui::CMainShort, CMainMovie; TODO: remove (unused)
    CLASS_SHORT         = 26,
    //! Gfx::CLightning; TODO: remove (unused)
    CLASS_BLITZ         = 27,

    //! Maximum (number of managed classes)
    CLASS_MAX           = 30
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
    ~CInstanceManager();

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

    static CInstanceManager& GetInstance();
    static CInstanceManager* GetInstancePointer();

protected:
    //! Fills holes in instance table
    void    Compress(ManagedClassType classType);

protected:
    ManagedClassInstances m_table[CLASS_MAX];
};


