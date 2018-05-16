/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2018, Daniel Roux, EPSITEC SA & TerranovaTeam
 * http://epsitec.ch; http://colobot.info; http://github.com/colobot
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://gnu.org/licenses
 */

#pragma once

#include "graphics/engine/camera.h"

#include <string>

class CEventQueue;
class CScript;
class CSoundInterface;
class CSettings;
struct Program;
class CPauseManager;
struct ActivePause;
struct Event;

namespace Ui
{

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

    void        StartEditScript(CScript *script, std::string name, Program* program);
    bool        StopEditScript(bool closeWithErrors);

    void        SetInfoText(std::string text, bool bClickable);

protected:
    bool        EventFrame(const Event &event);
    void        SearchToken(CEdit* edit);
    void        ColorizeScript(CEdit* edit);
    void        AdjustEditScript();
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

    void        SetFilenameField(CEdit* edit, const std::string& filename);

protected:
    Gfx::CEngine*    m_engine;
    CEventQueue*     m_event;
    CRobotMain*      m_main;
    Gfx::CCamera*    m_camera;
    CSoundInterface* m_sound;
    CInterface*      m_interface;
    CApplication*    m_app;
    CPauseManager*   m_pause;
    CSettings*       m_settings;

    Program*    m_program;
    CScript*    m_script;
    Gfx::CameraType m_editCamera;

    bool        m_bEditMaximized;
    bool        m_bEditMinimized;

    Math::Point     m_editActualPos;
    Math::Point     m_editActualDim;
    Math::Point     m_editFinalPos;
    Math::Point     m_editFinalDim;
    Math::Point     m_windowPos;
    Math::Point     m_windowDim;
    Math::Point     m_dialogPos;
    Math::Point     m_dialogDim;

    float        m_time;
    float        m_fixInfoTextTime;
    bool         m_bRunning;
    bool         m_bRealTime;
    ActivePause* m_editorPause = nullptr;
    ActivePause* m_runningPause = nullptr;
    std::string  m_helpFilename;

    StudioDialog m_dialog;
};


} // namespace Ui
