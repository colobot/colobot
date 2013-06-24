// * This file is part of the COLOBOT source code
// * Copyright (C) 2001-2008, Daniel ROUX & EPSITEC SA, www.epsitec.ch
// * Copyright (C) 2012, Polish Portal of Colobot (PPC)
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

#include <string>

#include "common/event.h"

#include "graphics/engine/camera.h"

class CRobotMain;
class CObject;
class CEventQueue;

struct Event;

namespace Gfx {
class CEngine;
class CParticle;
class CLightManager;
}

namespace Ui {

class CInterface;

class CDisplayInfo
{
public:
    CDisplayInfo();
    ~CDisplayInfo();

    bool        EventProcess(const Event &event);

    void        StartDisplayInfo(std::string filename, int index, bool bSoluce);
    void        StopDisplayInfo();

    void        SetPosition(int pos);
    int         GetPosition();

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
    Gfx::CEngine* m_engine;
    CEventQueue*     m_event;
    CRobotMain* m_main;
    Gfx::CCamera*    m_camera;
    CInterface* m_interface;
    Gfx::CParticle* m_particle;
    Gfx::CLightManager*     m_light;

    bool        m_bInfoMaximized;
    bool        m_bInfoMinimized;

    int         m_index;
    Gfx::CameraType  m_infoCamera;
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


} // namespace Ui

