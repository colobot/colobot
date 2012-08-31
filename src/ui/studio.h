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


#include <object/object.h>

#include <script/script.h>

#include <graphics/engine/engine.h>
#include <graphics/engine/camera.h>

#include <common/event.h>
#include <common/struct.h>
#include <common/misc.h>
#include <common/iman.h>

#include <sound/sound.h>

#include <ui/control.h>
#include <ui/button.h>
#include <ui/color.h>
#include <ui/check.h>
#include <ui/key.h>
#include <ui/group.h>
#include <ui/image.h>
#include <ui/label.h>
#include <ui/edit.h>
#include <ui/editvalue.h>
#include <ui/scroll.h>
#include <ui/slider.h>
#include <ui/list.h>
#include <ui/shortcut.h>
#include <ui/compass.h>
#include <ui/target.h>
#include <ui/map.h>
#include <ui/window.h>
#include <ui/interface.h>


namespace Ui {

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

        void        StartEditScript(CScript *script, char* name, int rank);
        bool        StopEditScript(bool bCancel);

    protected:
        bool        EventFrame(const Event &event);
        void        SearchToken(CEdit* edit);
        void        ColorizeScript(CEdit* edit);
        void        AdjustEditScript();
        void        SetInfoText(char *text, bool bClickable);
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
        void        SearchDirectory(char *dir, bool bCreate);
        bool        ReadProgram();
        bool        WriteProgram();

    protected:
        CInstanceManager* m_iMan;
        Gfx::CEngine* m_engine;
        CEventQueue*     m_event;
        CRobotMain* m_main;
        CCamera*    m_camera;
        CSoundInterface* m_sound;
        CInterface* m_interface;

        int         m_rank;
        CScript*    m_script;

        bool        m_bEditMaximized;
        bool        m_bEditMinimized;

        CameraType  m_editCamera;
        Math::Point     m_editActualPos;
        Math::Point     m_editActualDim;
        Math::Point     m_editFinalPos;
        Math::Point     m_editFinalDim;

        float       m_time;
        float       m_fixInfoTextTime;
        bool        m_bRunning;
        bool        m_bRealTime;
        bool        m_bInitPause;
        char        m_helpFilename[100];

    StudioDialog m_dialog;
};


}

