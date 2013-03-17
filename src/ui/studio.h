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

// studio.h

#pragma once
#include "common/event.h"

#include "graphics/engine/camera.h"

#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

#include <string>

class CEventQueue;
class CRobotMain;
class CScript;
class CSoundInterface;

namespace Gfx {
class CEngine;
class CCamera;
};

namespace Ui {

class CEdit;
class CInterface;

enum StudioDialog
{
    SD_NULL,
    SD_OPEN,
    SD_SAVE,
    SD_FIND,
    SD_REPLACE,
};



class CStudio
{
public:
    CStudio();
    ~CStudio();

    bool        EventProcess(const Event &event);

    void        StartEditScript(CScript *script, std::string name, int rank);
    bool        StopEditScript(bool bCancel);

protected:
    bool        EventFrame(const Event &event);
    void        SearchToken(CEdit* edit);
    void        ColorizeScript(CEdit* edit);
    void        AdjustEditScript();
    void        SetInfoText(std::string text, bool bClickable);
    void        ViewEditScript();
    void        UpdateFlux();
    void        UpdateButtons();

    void        StartDialog(StudioDialog type);
    void        StopDialog();
    void        AdjustDialog();
    bool        EventDialog(const Event &event);
    void        UpdateChangeList();
    void        UpdateChangeEdit();
    void        UpdateDialogAction();
    void        UpdateDialogPublic();
    void        UpdateDialogList();
    std::string  SearchDirectory(bool bCreate);
    bool        ReadProgram();
    bool        WriteProgram();

protected:
    Gfx::CEngine*    m_engine;
    CEventQueue*     m_event;
    CRobotMain*      m_main;
    Gfx::CCamera*    m_camera;
    CSoundInterface* m_sound;
    CInterface*      m_interface;
    CApplication*    m_app;

    int         m_rank;
    CScript*    m_script;
    Gfx::CameraType m_editCamera;

    bool        m_bEditMaximized;
    bool        m_bEditMinimized;

    Math::Point     m_editActualPos;
    Math::Point     m_editActualDim;
    Math::Point     m_editFinalPos;
    Math::Point     m_editFinalDim;

    float       m_time;
    float       m_fixInfoTextTime;
    bool        m_bRunning;
    bool        m_bRealTime;
    bool        m_bInitPause;
    std::string  m_helpFilename;

    StudioDialog m_dialog;
};


} // namespace Ui

