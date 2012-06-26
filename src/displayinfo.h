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

#ifndef _DISPLAYINFO_H_
#define _DISPLAYINFO_H_


#include "struct.h"
#include "camera.h"


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
    CDisplayInfo(CInstanceManager* iMan);
    ~CDisplayInfo();

    BOOL        EventProcess(const Event &event);

    void        StartDisplayInfo(char *filename, int index, BOOL bSoluce);
    void        StopDisplayInfo();

    void        SetPosition(int pos);
    int         RetPosition();

protected:
    BOOL        EventFrame(const Event &event);
    void        HyperUpdate();
    void        AdjustDisplayInfo(FPOINT wpos, FPOINT wdim);
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

    BOOL        m_bInfoMaximized;
    BOOL        m_bInfoMinimized;

    int         m_index;
    CameraType  m_infoCamera;
    FPOINT      m_infoNormalPos;
    FPOINT      m_infoNormalDim;
    FPOINT      m_infoActualPos;
    FPOINT      m_infoActualDim;
    FPOINT      m_infoFinalPos;
    FPOINT      m_infoFinalDim;
    int         m_lightSuppl;
    BOOL        m_bEditLock;
    BOOL        m_bInitPause;
    BOOL        m_bSoluce;
    CObject*    m_toto;
};


#endif //_DISPLAYINFO_H_
