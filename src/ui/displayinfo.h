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

// displayinfo.h

#pragma once


#include "common/struct.h"
#include "old/camera.h"


class CInstanceManager;
class CD3DEngine;
class CEvent;
class CRobotMain;
class CCamera;
class CInterface;
class CObject;
class CParticule;
class CLight;


class CDisplayInfo
{
public:
    CDisplayInfo();
    ~CDisplayInfo();

    bool        EventProcess(const Event &event);

    void        StartDisplayInfo(char *filename, int index, bool bSoluce);
    void        StopDisplayInfo();

    void        SetPosition(int pos);
    int         RetPosition();

protected:
    bool        EventFrame(const Event &event);
    void        HyperUpdate();
    void        AdjustDisplayInfo(Math::Point wpos, Math::Point wdim);
    void        ChangeIndexButton(int index);
    void        UpdateIndexButton();
    void        UpdateCopyButton();
    void        ViewDisplayInfo();
    CObject*    SearchToto();
    void        CreateObjectsFile();

protected:
    CInstanceManager* m_iMan;
    CD3DEngine* m_engine;
    CEvent*     m_event;
    CRobotMain* m_main;
    CCamera*    m_camera;
    CInterface* m_interface;
    CParticule* m_particule;
    CLight*     m_light;

    bool        m_bInfoMaximized;
    bool        m_bInfoMinimized;

    int         m_index;
    CameraType  m_infoCamera;
    Math::Point     m_infoNormalPos;
    Math::Point     m_infoNormalDim;
    Math::Point     m_infoActualPos;
    Math::Point     m_infoActualDim;
    Math::Point     m_infoFinalPos;
    Math::Point     m_infoFinalDim;
    int         m_lightSuppl;
    bool        m_bEditLock;
    bool        m_bInitPause;
    bool        m_bSoluce;
    CObject*    m_toto;
};


