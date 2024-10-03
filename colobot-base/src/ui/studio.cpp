/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2023, Daniel Roux, EPSITEC SA & TerranovaTeam
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


#include "ui/studio.h"

#include "app/app.h"
#include "app/pausemanager.h"

#include "common/event.h"
#include "common/logger.h"
#include "common/settings.h"
#include "common/stringutils.h"

#include "common/resources/resourcemanager.h"

#include "graphics/engine/camera.h"
#include "graphics/engine/engine.h"

#include "level/player_profile.h"
#include "level/robotmain.h"

#include "object/object.h"

#include "object/interface/program_storage_object.h"
#include "object/interface/programmable_object.h"

#include "script/cbottoken.h"
#include "script/script.h"

#include "sound/sound.h"

#include "ui/filedialog.h"

#include "ui/controls/check.h"
#include "ui/controls/color.h"
#include "ui/controls/control.h"
#include "ui/controls/edit.h"
#include "ui/controls/group.h"
#include "ui/controls/image.h"
#include "ui/controls/interface.h"
#include "ui/controls/key.h"
#include "ui/controls/label.h"
#include "ui/controls/list.h"
#include "ui/controls/map.h"
#include "ui/controls/shortcut.h"
#include "ui/controls/slider.h"
#include "ui/controls/target.h"
#include "ui/controls/window.h"

#include <stdio.h>
#include <ctime>


namespace Ui
{


// Object's constructor.

CStudio::CStudio()
{
    m_app       = CApplication::GetInstancePointer();
    m_sound     = m_app->GetSound();
    m_event     = m_app->GetEventQueue();
    m_engine    = Gfx::CEngine::GetInstancePointer();
    m_main      = CRobotMain::GetInstancePointer();
    m_interface = m_main->GetInterface();
    m_camera    = m_main->GetCamera();
    m_pause     = m_main->GetPauseManager();
    m_settings  = CSettings::GetInstancePointer();

    m_program = nullptr;
    m_script = nullptr;

    m_bEditMaximized = false;
    m_bEditMinimized = false;

    m_time      = 0.0f;
    m_bRealTime = true;
    m_bRunning  = false;
    m_fixInfoTextTime = 0.0f;
    m_fileDialog = nullptr;
    m_editCamera = Gfx::CAM_TYPE_NULL;
}

// Object's destructor.

CStudio::~CStudio()
{
}


// Management of an event.

bool CStudio::EventProcess(const Event &event)
{
    CWindow*    pw;
    CEdit*      edit;
    CSlider*    slider;

    if (m_fileDialog != nullptr)  // dialogue exists?
    {
        return EventDialog(event);
    }

    if ( event.type == EVENT_FRAME )
    {
        EventFrame(event);
    }

    pw = static_cast< CWindow* >(m_interface->SearchControl(EVENT_WINDOW3));
    if ( pw == nullptr )  return false;

    edit = static_cast<CEdit*>(pw->SearchControl(EVENT_STUDIO_EDIT));
    if ( edit == nullptr )  return false;

    if ( event.type == pw->GetEventTypeClose() )
    {
        m_event->AddEvent(Event(EVENT_STUDIO_OK));
    }

    if ( event.type == EVENT_STUDIO_EDIT )  // text modified?
    {
        ColorizeScript(edit);
    }

    if ( event.type == EVENT_STUDIO_LIST )  // list clicked?
    {
        m_main->StartDisplayInfo(m_helpFilename, -1);
    }

    if ( event.type == EVENT_STUDIO_NEW )  // new?
    {
        m_script->New(edit, "");
    }

    if ( event.type == EVENT_STUDIO_OPEN )  // open?
    {
        StartDialog(event);
        return true;
    }
    if ( event.type == EVENT_STUDIO_SAVE )  // save?
    {
        StartDialog(event);
        return true;
    }

    if ( event.type == EVENT_STUDIO_UNDO )  // undo?
    {
        edit->Undo();
    }
    if ( event.type == EVENT_STUDIO_CUT )  // cut?
    {
        edit->Cut();
    }
    if ( event.type == EVENT_STUDIO_COPY )  // copy?
    {
        edit->Copy();
    }
    if ( event.type == EVENT_STUDIO_PASTE )  // paste?
    {
        edit->Paste();
    }

    if ( event.type == EVENT_STUDIO_SIZE )  // size?
    {
        slider = static_cast< CSlider* >(pw->SearchControl(EVENT_STUDIO_SIZE));
        if ( slider == nullptr )  return false;
        m_settings->SetFontSize(9.0f+slider->GetVisibleValue()*15.0f);
        ViewEditScript();
    }

    if ( event.type == EVENT_STUDIO_TOOL )  // instructions?
    {
        m_main->StartDisplayInfo(SATCOM_HUSTON, false);
    }
    if ( event.type == EVENT_STUDIO_HELP )  // help?
    {
        m_main->StartDisplayInfo(SATCOM_PROG, false);
    }

    if ( event.type == EVENT_STUDIO_COMPILE )  // compile?
    {
        if ( m_script->GetScript(edit) )  // compile
        {
            std::string res;
            GetResource(RES_TEXT, RT_STUDIO_COMPOK, res);
            SetInfoText(res, false);
        }
        else
        {
            std::string error;
            m_script->GetError(error);
            SetInfoText(error, false);
        }
    }

    if ( event.type == EVENT_STUDIO_RUN )  // run/stop?
    {
        if ( m_script->IsRunning() )
        {
            m_event->AddEvent(Event(EVENT_OBJECT_PROGSTOP));
        }
        else
        {
            if ( m_script->GetScript(edit) )  // compile
            {
                SetInfoText("", false);

                m_event->AddEvent(Event(EVENT_OBJECT_PROGSTART));
            }
            else
            {
                std::string error;
                m_script->GetError(error);
                SetInfoText(error, false);
            }
        }
    }

    if ( event.type == EVENT_STUDIO_REALTIME )  // real time?
    {
        m_bRealTime = !m_bRealTime;
        m_script->SetStepMode(!m_bRealTime);
        UpdateFlux();
        UpdateButtons();
    }

    if ( event.type == EVENT_STUDIO_STEP )  // step?
    {
        m_script->Step();
    }

    if ( event.type == EVENT_WINDOW3 )  // window is moved?
    {
        m_editActualPos = m_editFinalPos = pw->GetPos();
        m_editActualDim = m_editFinalDim = pw->GetDim();
        m_windowPos = m_editActualPos;
        m_windowDim = m_editActualDim;
        AdjustEditScript();
    }
    if ( event.type == pw->GetEventTypeReduce() )
    {
        if ( m_bEditMinimized )
        {
            m_editFinalPos = m_windowPos;
            m_editFinalDim = m_windowDim;
            m_bEditMinimized = false;
            m_bEditMaximized = false;
        }
        else
        {
            m_editFinalPos.x =  0.00f;
            m_editFinalPos.y = -0.44f;
            m_editFinalDim.x =  1.00f;
            m_editFinalDim.y =  0.50f;
            m_bEditMinimized = true;
            m_bEditMaximized = false;
        }
        m_main->SetEditFull(m_bEditMaximized);
        pw = static_cast< CWindow* >(m_interface->SearchControl(EVENT_WINDOW3));
        if ( pw != nullptr )
        {
            pw->SetMaximized(m_bEditMaximized);
            pw->SetMinimized(m_bEditMinimized);
        }
    }
    if ( event.type == pw->GetEventTypeFull() )
    {
        if ( m_bEditMaximized )
        {
            m_editFinalPos = m_windowPos;
            m_editFinalDim = m_windowDim;
            m_bEditMinimized = false;
            m_bEditMaximized = false;
        }
        else
        {
            m_editFinalPos.x = 0.00f;
            m_editFinalPos.y = 0.00f;
            m_editFinalDim.x = 1.00f;
            m_editFinalDim.y = 1.00f;
            m_bEditMinimized = false;
            m_bEditMaximized = true;
        }
        m_main->SetEditFull(m_bEditMaximized);
        pw = static_cast< CWindow* >(m_interface->SearchControl(EVENT_WINDOW3));
        if ( pw != nullptr )
        {
            pw->SetMaximized(m_bEditMaximized);
            pw->SetMinimized(m_bEditMinimized);
        }
    }

    return true;
}


// Evolves value with time elapsed.

static float Evolution(float final, float actual, float time)
{
    float   value;

    value = actual + (final-actual)*time;

    if ( final > actual )
    {
        if ( value > final )  value = final;  // does not exceed
    }
    else
    {
        if ( value < final )  value = final;  // does not exceed
    }

    return value;
}

// Makes the studio evolve as time elapsed.

bool CStudio::EventFrame(const Event &event)
{
    CWindow*    pw;
    CEdit*      edit;
    CList*      list;
    float       time;
    int         cursor1, cursor2, iCursor1, iCursor2;

    m_time += event.rTime;
    m_fixInfoTextTime -= event.rTime;

    pw = static_cast< CWindow* >(m_interface->SearchControl(EVENT_WINDOW3));
    if ( pw == nullptr )  return false;

    edit = static_cast< CEdit* >(pw->SearchControl(EVENT_STUDIO_EDIT));
    if ( edit == nullptr )  return false;

    list = static_cast< CList* >(pw->SearchControl(EVENT_STUDIO_LIST));
    if ( list == nullptr )  return false;

    if (m_script->IsRunning() && (!m_script->GetStepMode() || m_script->IsContinue()))
    {
        if (m_runningPause != nullptr)
        {
            m_pause->DeactivatePause(m_runningPause);
            m_runningPause = nullptr;
        }
    }
    else
    {
        if (m_runningPause == nullptr)
        {
            m_runningPause = m_pause->ActivatePause(PAUSE_ENGINE|PAUSE_MUTE_SOUND);
        }
    }

    if ( !m_script->IsRunning() && m_bRunning )  // stop?
    {
        m_bRunning = false;
        UpdateFlux();  // stop
        AdjustEditScript();
        std::string res;
        GetResource(RES_TEXT, RT_STUDIO_PROGSTOP, res);
        SetInfoText(res, false);

        m_event->AddEvent(Event(EVENT_OBJECT_PROGSTOP));
    }

    if ( m_script->IsRunning() && !m_bRunning )  // starting?
    {
        m_bRunning = true;
        UpdateFlux();  // run
        AdjustEditScript();
    }
    UpdateButtons();

    if ( m_bRunning )
    {
        m_script->GetCursor(cursor1, cursor2);
        edit->GetCursor(iCursor1, iCursor2);
        if ( cursor1 != iCursor1 ||
             cursor2 != iCursor2 )  // cursors change?
        {
            edit->SetCursor(cursor1, cursor2);  // shows it on the execution
            edit->ShowSelect();
        }

        m_script->UpdateList(list);  // updates the list of variables
    }
    else
    {
        SearchToken(edit);
    }

    if ( m_editFinalPos.x != m_editActualPos.x ||
         m_editFinalPos.y != m_editActualPos.y ||
         m_editFinalDim.x != m_editActualDim.x ||
         m_editFinalDim.y != m_editActualDim.y )
    {
        time = event.rTime*20.0f;
        m_editActualPos.x = Evolution(m_editFinalPos.x, m_editActualPos.x, time);
        m_editActualPos.y = Evolution(m_editFinalPos.y, m_editActualPos.y, time);
        m_editActualDim.x = Evolution(m_editFinalDim.x, m_editActualDim.x, time);
        m_editActualDim.y = Evolution(m_editFinalDim.y, m_editActualDim.y, time);
        AdjustEditScript();
    }

    return true;
}


// Indicates whether a character is part of a word.

static bool IsToken(char c)
{
    return ( isalnum(c) || c == '_' );
}

// Seeks if the cursor is on a keyword.

void CStudio::SearchToken(CEdit* edit)
{
    ObjectType  type;
    int         len, cursor1, cursor2, i, character, level;
    std::string text;
    std::string token( 100, '\0');

    text = edit->GetText();
    len  = edit->GetTextLength();
    edit->GetCursor(cursor1, cursor2);

    i = cursor1;
    if ( i > 0 )
    {
        character = static_cast< unsigned char > (text[i-1]);
        if ( !IsToken(character) )
        {
            level = 1;
            while ( i > 0 )
            {
                character = static_cast< unsigned char > (text[i-1]);
                if ( character == ')' )
                {
                    level ++;
                }
                else if ( character == '(' )
                {
                    level --;
                    if ( level == 0 )  break;
                }
                i --;
            }
            if ( level > 0 )
            {
                m_helpFilename = "";
                SetInfoText("", true);
                return;
            }
            while ( i > 0 )
            {
                character = static_cast< unsigned char > (text[i-1]);
                if ( IsToken(character) )  break;
                i --;
            }
        }
    }

    while ( i > 0 )
    {
        character = static_cast< unsigned char > (text[i-1]);
        if ( !IsToken(character) )  break;
        i --;
    }
    cursor2 = i;

    while ( i < len )
    {
        character = static_cast< unsigned char > (text[i]);
        if ( !IsToken(character) )  break;
        i ++;
    }
    cursor1 = i;
    len = cursor1-cursor2;

    if ( len > 100-1 )  len = 100-1;
    for ( i=0 ; i<len ; i++ )
    {
        token[i] = text[cursor2+i];
    }
    token[i] = 0;

    m_helpFilename = GetHelpFilename(token.c_str());
    if ( m_helpFilename.empty() )
    {
        for ( i=0 ; i<OBJECT_MAX ; i++ )
        {
            type = static_cast< ObjectType >(i);
            text = const_cast<char *>(GetObjectName(type));
            if ( text[0] != 0 )
            {
                if ( token == text )
                {
                    m_helpFilename = GetHelpFilename(type);
                    SetInfoText(token, true);
                    return;
                }
            }
            text = const_cast<char *>(GetObjectAlias(type));
            if ( text[0] != 0 )
            {
                if ( token == text )
                {
                    m_helpFilename = GetHelpFilename(type);
                    SetInfoText(token, true);
                    return;
                }
            }
        }
    }

    text = const_cast<char *>(GetHelpText(token.c_str()));
    if ( text[0] == 0 && !m_helpFilename.empty() )
    {
        SetInfoText(token, true);
    }
    else
    {
        SetInfoText(text, true);
    }
}

// Colors the text according to syntax.

void CStudio::ColorizeScript(CEdit* edit)
{
    m_script->ColorizeScript(edit);
}


// Starts editing a program.

void CStudio::StartEditScript(CScript *script, std::string name, Program* program)
{
    glm::vec2   pos, dim;
    CWindow*    pw;
    CEdit*      edit;
    CButton*    button;
    CSlider*    slider;
    CList*      list;

    m_script  = script;
    m_program = program;

    m_main->SetEditLock(true, true);
    m_main->SetEditFull(false);
    m_main->SetSpeed(1.0f);
    m_editCamera = m_camera->GetType();
    m_camera->SetType(Gfx::CAM_TYPE_EDIT);
    m_editorPause = m_pause->ActivatePause(PAUSE_HIDE_SHORTCUTS, PAUSE_MUSIC_EDITOR);

    m_bRunning = m_script->IsRunning();
    m_bRealTime = m_bRunning;
    m_script->SetStepMode(!m_bRealTime);

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW6));
    if (pw != nullptr) pw->ClearState(STATE_VISIBLE | STATE_ENABLE);

    m_dialogPos = m_settings->GetIOPos();
    m_dialogDim = m_settings->GetIODim();
    m_windowPos = m_settings->GetWindowPos();
    m_windowDim = m_settings->GetWindowDim();
    m_bEditMaximized = m_settings->GetWindowMax();

    if ( m_bEditMaximized )
    {
        m_editFinalPos.x = 0.00f;
        m_editFinalPos.y = 0.00f;
        m_editFinalDim.x = 1.00f;
        m_editFinalDim.y = 1.00f;
    }
    else
    {
        m_editFinalPos = m_windowPos;
        m_editFinalDim = m_windowDim;
    }
    pos = m_editActualPos = m_editFinalPos;
    dim = m_editActualDim = m_editFinalDim;

    pw = m_interface->CreateWindows(pos, dim, 8, EVENT_WINDOW3);
    if (pw == nullptr)
        return;

    pw->SetState(STATE_SHADOW);
    pw->SetRedim(true);  // before SetName!
    pw->SetMovable(true);
    pw->SetClosable(true);

    std::string res;
    GetResource(RES_TEXT, RT_STUDIO_TITLE, res);
    pw->SetName(res);

    pw->SetMinDim({ 0.49f, 0.50f });
    pw->SetMaximized(m_bEditMaximized);
    pw->SetMinimized(m_bEditMinimized);

    // stop camera control if maximized
    m_main->SetEditFull(m_bEditMaximized);

    edit = pw->CreateEdit(pos, dim, 0, EVENT_STUDIO_EDIT);
    if (edit == nullptr)
        return;

    edit->SetState(STATE_SHADOW);
    edit->SetInsideScroll(false);
//? if ( m_bRunning )  edit->SetEdit(false);
    edit->SetFontType(Gfx::FONT_STUDIO);
    edit->SetFontStretch(1.0f);
    edit->SetDisplaySpec(true);
    edit->SetAutoIndent(m_engine->GetEditIndentMode());

    m_script->PutScript(edit, name.c_str());
    ColorizeScript(edit);

    ViewEditScript();

    list = pw->CreateList(pos, dim, 1, EVENT_STUDIO_LIST, 1.2f);
    list->SetState(STATE_SHADOW);
    list->SetFontType(Gfx::FONT_STUDIO);
    list->SetSelectCap(false);
    list->SetFontSize(Gfx::FONT_SIZE_SMALL*0.85f);
//? list->SetFontStretch(1.0f);

    button = pw->CreateButton(pos, dim, 56, EVENT_STUDIO_NEW);
    button->SetState(STATE_SHADOW);
    button = pw->CreateButton(pos, dim, 57, EVENT_STUDIO_OPEN);
    button->SetState(STATE_SHADOW);
    button = pw->CreateButton(pos, dim, 58, EVENT_STUDIO_SAVE);
    button->SetState(STATE_SHADOW);
    button = pw->CreateButton(pos, dim, 59, EVENT_STUDIO_UNDO);
    button->SetState(STATE_SHADOW);
    button = pw->CreateButton(pos, dim, 60, EVENT_STUDIO_CUT);
    button->SetState(STATE_SHADOW);
    button = pw->CreateButton(pos, dim, 61, EVENT_STUDIO_COPY);
    button->SetState(STATE_SHADOW);
    button = pw->CreateButton(pos, dim, 62, EVENT_STUDIO_PASTE);
    button->SetState(STATE_SHADOW);
    slider = pw->CreateSlider(pos, dim, 0, EVENT_STUDIO_SIZE);
    slider->SetState(STATE_SHADOW);
    slider->SetVisibleValue((m_settings->GetFontSize()-9.0f)/15.0f);
    pw->CreateGroup(pos, dim, 19, EVENT_LABEL1);  // SatCom logo
    button = pw->CreateButton(pos, dim, 128+57, EVENT_STUDIO_TOOL);
    button->SetState(STATE_SHADOW);
    button = pw->CreateButton(pos, dim, 128+60, EVENT_STUDIO_HELP);
    button->SetState(STATE_SHADOW);

    button = pw->CreateButton(pos, dim, -1, EVENT_STUDIO_OK);
    button->SetState(STATE_SHADOW);
    button = pw->CreateButton(pos, dim, 61, EVENT_STUDIO_CLONE);
    button->SetState(STATE_SHADOW);
    button = pw->CreateButton(pos, dim, -1, EVENT_STUDIO_CANCEL);
    button->SetState(STATE_SHADOW);
    button = pw->CreateButton(pos, dim, 64+23, EVENT_STUDIO_COMPILE);
    button->SetState(STATE_SHADOW);
    button = pw->CreateButton(pos, dim, 21, EVENT_STUDIO_RUN);
    button->SetState(STATE_SHADOW);
    button = pw->CreateButton(pos, dim, 64+22, EVENT_STUDIO_REALTIME);
    button->SetState(STATE_SHADOW);
    button = pw->CreateButton(pos, dim, 64+29, EVENT_STUDIO_STEP);
    button->SetState(STATE_SHADOW);

    if (!m_program->runnable)
    {
        GetResource(RES_TEXT, RT_PROGRAM_EXAMPLE, res);
        SetInfoText(res, false);
    }
    else if (m_program->readOnly)
    {
        GetResource(RES_TEXT, RT_PROGRAM_READONLY, res);
        SetInfoText(res, false);
    }

    UpdateFlux();
    UpdateButtons();
    AdjustEditScript();
}

// Repositions all the editing controls.

void CStudio::AdjustEditScript()
{
    CWindow*    pw;
    CEdit*      edit;
    CButton*    button;
    CGroup*     group;
    CSlider*    slider;
    CList*      list;
    glm::vec2   wpos, wdim, pos, dim, ppos, ddim;
    float       hList;

    wpos = m_editActualPos;
    wdim = m_editActualDim;

    pw = static_cast< CWindow* >(m_interface->SearchControl(EVENT_WINDOW3));
    if ( pw != nullptr )
    {
        pw->SetPos(wpos);
        pw->SetDim(wdim);
        wdim = pw->GetDim();
    }

    if ( m_bRunning ) hList = 80.0f/480.0f;
    else              hList = 20.0f/480.0f;

    pos.x = wpos.x+0.01f;
    pos.y = wpos.y+0.09f+hList;
    dim.x = wdim.x-0.02f;
    dim.y = wdim.y-0.22f-hList;
    edit = static_cast< CEdit* >(pw->SearchControl(EVENT_STUDIO_EDIT));
    if ( edit != nullptr )
    {
        edit->SetPos(pos);
        edit->SetDim(dim);
    }

    pos.x = wpos.x+0.01f;
    pos.y = wpos.y+0.09f;
    dim.x = wdim.x-0.02f;
    dim.y = hList;
    list = static_cast< CList* >(pw->SearchControl(EVENT_STUDIO_LIST));
    if ( list != nullptr )
    {
        list->SetPos(pos);
        list->SetDim(dim);
    }

    dim.x = 0.04f;
    dim.y = 0.04f*1.5f;
    dim.y = 25.0f/480.0f;

    pos.y = wpos.y+wdim.y-dim.y-0.06f;
    pos.x = wpos.x+0.01f;
    button = static_cast< CButton* >(pw->SearchControl(EVENT_STUDIO_NEW));
    if ( button != nullptr )
    {
        button->SetPos(pos);
        button->SetDim(dim);
    }
    pos.x = wpos.x+0.05f;
    button = static_cast< CButton* >(pw->SearchControl(EVENT_STUDIO_OPEN));
    if ( button != nullptr )
    {
        button->SetPos(pos);
        button->SetDim(dim);
    }
    pos.x = wpos.x+0.09f;
    button = static_cast< CButton* >(pw->SearchControl(EVENT_STUDIO_SAVE));
    if ( button != nullptr )
    {
        button->SetPos(pos);
        button->SetDim(dim);
    }
    pos.x = wpos.x+0.14f;
    button = static_cast< CButton* >(pw->SearchControl(EVENT_STUDIO_UNDO));
    if ( button != nullptr )
    {
        button->SetPos(pos);
        button->SetDim(dim);
    }
    pos.x = wpos.x+0.19f;
    button = static_cast< CButton* >(pw->SearchControl(EVENT_STUDIO_CUT));
    if ( button != nullptr )
    {
        button->SetPos(pos);
        button->SetDim(dim);
    }
    pos.x = wpos.x+0.23f;
    button = static_cast< CButton* >(pw->SearchControl(EVENT_STUDIO_COPY));
    if ( button != nullptr )
    {
        button->SetPos(pos);
        button->SetDim(dim);
    }
    pos.x = wpos.x+0.27f;
    button = static_cast< CButton* >(pw->SearchControl(EVENT_STUDIO_PASTE));
    if ( button != nullptr )
    {
        button->SetPos(pos);
        button->SetDim(dim);
    }
    pos.x = wpos.x+0.32f;
    slider = static_cast< CSlider* >(pw->SearchControl(EVENT_STUDIO_SIZE));
    if ( slider != nullptr )
    {
        ppos = pos;
        ddim.x = dim.x*0.7f;
        ddim.y = dim.y;
        ppos.y -= 3.0f/480.0f;
        ddim.y += 6.0f/480.0f;
        slider->SetPos(ppos);
        slider->SetDim(ddim);
    }
    pos.x = wpos.x+0.36f;
    group = static_cast< CGroup* >(pw->SearchControl(EVENT_LABEL1));
    if ( group != nullptr )
    {
        group->SetPos(pos);
        group->SetDim(dim);
    }
    pos.x = wpos.x+0.40f;
    button = static_cast< CButton* >(pw->SearchControl(EVENT_STUDIO_TOOL));
    if ( button != nullptr )
    {
        button->SetPos(pos);
        button->SetDim(dim);
    }
    pos.x = wpos.x+0.44f;
    button = static_cast< CButton* >(pw->SearchControl(EVENT_STUDIO_HELP));
    if ( button != nullptr )
    {
        button->SetPos(pos);
        button->SetDim(dim);
    }

    pos.y = wpos.y+0.02f;
    pos.x = wpos.x+0.01f;
    dim.x = 80.0f/640.0f;
    dim.y = 25.0f/480.0f;
    button = static_cast< CButton* >(pw->SearchControl(EVENT_STUDIO_OK));
    if ( button != nullptr )
    {
        button->SetPos(pos);
        button->SetDim(dim);
    }
    dim.x = 25.0f/480.0f;
    dim.y = 25.0f/480.0f;
    pos.x = wpos.x+0.01f+0.125f+0.005f;
    button = static_cast< CButton* >(pw->SearchControl(EVENT_STUDIO_CLONE));
    if ( button != nullptr )
    {
        button->SetPos(pos);
        button->SetDim(dim);
    }
    dim.x = 80.0f/640.0f - 25.0f/480.0f;
    dim.y = 25.0f/480.0f;
    pos.x = wpos.x+0.01f+0.125f+0.005f+(25.0f/480.0f)+0.005f;
    button = static_cast< CButton* >(pw->SearchControl(EVENT_STUDIO_CANCEL));
    if ( button != nullptr )
    {
        button->SetPos(pos);
        button->SetDim(dim);
    }
    pos.x = wpos.x+0.28f;
    dim.x = dim.y*0.75f;
    button = static_cast< CButton* >(pw->SearchControl(EVENT_STUDIO_COMPILE));
    if ( button != nullptr )
    {
        button->SetPos(pos);
        button->SetDim(dim);
    }
    pos.x = wpos.x+0.28f+dim.x*1;
    button = static_cast< CButton* >(pw->SearchControl(EVENT_STUDIO_RUN));
    if ( button != nullptr )
    {
        button->SetPos(pos);
        button->SetDim(dim);
    }
    pos.x = wpos.x+0.28f+dim.x*2;
    button = static_cast< CButton* >(pw->SearchControl(EVENT_STUDIO_REALTIME));
    if ( button != nullptr )
    {
        button->SetPos(pos);
        button->SetDim(dim);
    }
    pos.x = wpos.x+0.28f+dim.x*3;
    button = static_cast< CButton* >(pw->SearchControl(EVENT_STUDIO_STEP));
    if ( button != nullptr )
    {
        button->SetPos(pos);
        button->SetDim(dim);
    }
}

// Ends edition of a program.

bool CStudio::StopEditScript(bool closeWithErrors)
{
    CWindow*    pw;
    CEdit*      edit;

    pw = static_cast< CWindow* >(m_interface->SearchControl(EVENT_WINDOW3));
    if ( pw == nullptr )  return false;

    if ( !closeWithErrors && !m_script->IsRunning() )
    {
        edit = static_cast< CEdit* >(pw->SearchControl(EVENT_STUDIO_EDIT));
        if ( edit != nullptr )
        {
            if ( !m_script->GetScript(edit) )  // compile
            {
                std::string error;
                m_script->GetError(error);
                SetInfoText(error, false);
                return false;
            }
        }
    }
    m_script->SetStepMode(false);

    m_interface->DeleteControl(EVENT_WINDOW3);

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW6));
    if (pw != nullptr) pw->SetState(STATE_VISIBLE | STATE_ENABLE);

    m_pause->DeactivatePause(m_editorPause);
    m_editorPause = nullptr;
    m_pause->DeactivatePause(m_runningPause);
    m_runningPause = nullptr;
    m_main->SetEditLock(false, true);
    m_camera->SetType(m_editCamera);

    m_settings->SetIOPos(m_dialogPos);
    m_settings->SetIODim(m_dialogDim);
    m_settings->SetWindowPos(m_windowPos);
    m_settings->SetWindowDim(m_windowDim);
    m_settings->SetWindowMax(m_bEditMaximized);
    return true;
}

// Specifies the message to display.
// The messages are not clickable 8 seconds,
// even if a message was clickable poster before.

void CStudio::SetInfoText(std::string text, bool bClickable)
{
    if ( bClickable && m_fixInfoTextTime > 0.0f )  return;
    if ( !bClickable )  m_fixInfoTextTime = 8.0f;

    CWindow* pw = static_cast< CWindow* >(m_interface->SearchControl(EVENT_WINDOW3));
    if ( pw == nullptr )  return;

    CList* list = static_cast< CList* >(pw->SearchControl(EVENT_STUDIO_LIST));
    if ( list == nullptr )  return;

    list->Flush();  // just text
    list->SetItemName(0, text.c_str());

    if ( text[0] == 0 )  bClickable = false;
    list->SetSelectCap(bClickable);

    if ( bClickable )
    {
        std::string res;
        GetResource(RES_TEXT, RT_STUDIO_LISTTT, res);
        list->SetTooltip(res);
        list->SetState(STATE_ENABLE);
    }
    else
    {
        list->SetTooltip("");
//?     list->ClearState(STATE_ENABLE);
        list->SetState(STATE_ENABLE, text[0] != 0);
    }
}

// Beginning of the display of a dialogue.

void CStudio::StartDialog(const Event &event)
{
    if ( event.type == EVENT_STUDIO_OPEN )
    {
        m_fileDialog = std::make_unique<CFileDialog>();
        m_fileDialog->SetDialogType(CFileDialog::Type::Open);
    }
    if ( event.type == EVENT_STUDIO_SAVE )
    {
        m_fileDialog = std::make_unique<CFileDialog>();
        m_fileDialog->SetDialogType(CFileDialog::Type::Save);
    }

    if (m_fileDialog != nullptr) // a dialog was created?
    {
        m_main->SetSatComLock(true);  // impossible to use the SatCom

        m_fileDialog->SetWindowEvent(EVENT_WINDOW9);
        m_fileDialog->SetWindowPos(m_dialogPos);
        m_fileDialog->SetWindowDim(m_dialogDim);

        m_fileDialog->SetAutoExtension(".cbot");
        m_fileDialog->AddOptionalExtension(".txt");

        m_fileDialog->SetUsePublicPrivate(true);
        m_fileDialog->SetPublic(m_settings->GetIOPublic());
        m_fileDialog->SetPublicFolder("program");
        m_fileDialog->SetPrivateFolder(m_main->GetPlayerProfile()->GetSaveFile("program"));

        if ( event.type == EVENT_STUDIO_SAVE )
        {
            m_fileDialog->SetConfirmOverwrite(true);

            // filename in CScript may include sub-folder
            std::filesystem::path filename = m_script->GetFilename();
            if (filename.has_parent_path())
            {
                m_fileDialog->SetSubFolderPath(filename.parent_path());
            }
            m_fileDialog->SetFilename(filename);
        }

        m_fileDialog->StartDialog();
    }
}

// End of the display of a dialogue.

void CStudio::StopDialog()
{
    if (m_fileDialog != nullptr) // a dialog exists?
    {
        m_settings->SetIOPublic(m_fileDialog->GetPublic());
        m_dialogPos = m_fileDialog->GetWindowPos();
        m_dialogDim = m_fileDialog->GetWindowDim();
        m_fileDialog->StopDialog();
        m_fileDialog.reset();
    }

    m_main->SetSatComLock(false);  // possible to use the SatCom
}

// Management of events for a dialogue.

bool CStudio::EventDialog(const Event &event)
{
    if (m_fileDialog != nullptr)  // a dialog exists?
    {
        if ( event.type == EVENT_DIALOG_STOP )
        {
            StopDialog();
            return true;
        }

        if (event.type == EVENT_DIALOG_ACTION) // operation complete ?
        {
            CWindow* pw = static_cast< CWindow* >(m_interface->SearchControl(EVENT_WINDOW3));
            if ( pw == nullptr ) return false;
            CEdit* pe = static_cast< CEdit* >(pw->SearchControl(EVENT_STUDIO_EDIT));
            if ( pe == nullptr ) return false;

            std::filesystem::path path = m_fileDialog->GetBasePath();  // public/private folder
            std::filesystem::path subpath = m_fileDialog->GetSubFolderPath();// sub-folder

            // filename in CScript may include sub-folder
            std::filesystem::path filename = subpath / m_fileDialog->GetFilename();
            CFileDialog::Type type = m_fileDialog->GetDialogType();

            if ( type == CFileDialog::Type::Save )
            {
                if (!pe->WriteText(path / filename))
                {
                    m_sound->Play(SOUND_TZOING);
                    return true;
                }
                if ( !m_program->readOnly )
                    m_script->SetFilename(filename);
            }
            else if ( type == CFileDialog::Type::Open )
            {
                if (!pe->ReadText(path / filename))
                {
                    m_sound->Play(SOUND_TZOING);
                    return true;
                }
                m_script->SetFilename(filename);
                ColorizeScript(pe);
            }

            StopDialog();
            return true;
        }

        return m_fileDialog->EventProcess(event);
    }

    return true;
}

// Changing the size of a editing program.

void CStudio::ViewEditScript()
{
    CWindow*    pw;
    CEdit*      edit;

    pw = static_cast< CWindow* >(m_interface->SearchControl(EVENT_WINDOW3));
    if ( pw == nullptr )  return;

    edit = static_cast< CEdit* >(pw->SearchControl(EVENT_STUDIO_EDIT));
    if ( edit == nullptr )  return;

    glm::ivec2 dim = m_engine->GetWindowSize();
    edit->SetFontSize(m_settings->GetFontSize()/(dim.x/640.0f));
}


// Updates the operating mode.

void CStudio::UpdateFlux()
{
    if ( m_bRunning )
    {
        if ( m_bRealTime )  // run?
        {
            m_pause->DeactivatePause(m_runningPause);
            m_runningPause = nullptr;
        }
        else    // step by step?
        {
            if (m_runningPause == nullptr)
            {
                m_runningPause = m_pause->ActivatePause(PAUSE_ENGINE|PAUSE_MUTE_SOUND);
            }
        }
    }
    else    // stop?
    {
        if (m_runningPause == nullptr)
        {
            m_runningPause = m_pause->ActivatePause(PAUSE_ENGINE|PAUSE_MUTE_SOUND);
        }
    }
}

// Updates the buttons.

void CStudio::UpdateButtons()
{
    CWindow*    pw;
    CEdit*      edit;
    CButton*    button;

    pw = static_cast< CWindow* >(m_interface->SearchControl(EVENT_WINDOW3));
    if ( pw == nullptr )  return;

    edit = static_cast< CEdit* >(pw->SearchControl(EVENT_STUDIO_EDIT));
    if ( edit == nullptr )  return;

    if ( m_bRunning )
    {
        edit->SetIcon(1);  // red background
        edit->SetEditCap(false);  // just to see
        edit->SetHighlightCap(true);
    }
    else
    {
        edit->SetIcon(m_program->readOnly ? 1 : 0);  // standard background
        edit->SetEditCap(!m_program->readOnly);
        edit->SetHighlightCap(true);
    }


    button = static_cast< CButton* >(pw->SearchControl(EVENT_STUDIO_CLONE));
    if ( button == nullptr )  return;
    button->SetState(STATE_ENABLE, m_main->CanPlayerInteract() && (m_program->runnable && !m_bRunning));


    button = static_cast< CButton* >(pw->SearchControl(EVENT_STUDIO_COMPILE));
    if ( button == nullptr )  return;
    button->SetState(STATE_ENABLE, m_main->CanPlayerInteract() && (m_program->runnable && !m_bRunning));

    button = static_cast< CButton* >(pw->SearchControl(EVENT_STUDIO_RUN));
    if ( button == nullptr )  return;
    button->SetIcon(m_bRunning?8:21);  // stop/run
    button->SetState(STATE_ENABLE, m_main->CanPlayerInteract() && (m_program->runnable || m_bRunning));

    button = static_cast< CButton* >(pw->SearchControl(EVENT_STUDIO_REALTIME));
    if ( button == nullptr )  return;
    button->SetIcon(m_bRealTime?64+22:64+21);
    button->SetState(STATE_ENABLE, (!m_bRunning || !m_script->IsContinue()));

    button = static_cast< CButton* >(pw->SearchControl(EVENT_STUDIO_STEP));
    if ( button == nullptr )  return;
    button->SetState(STATE_ENABLE, (m_bRunning && !m_bRealTime && !m_script->IsContinue()));


    button = static_cast< CButton* >(pw->SearchControl(EVENT_STUDIO_NEW));
    if ( button == nullptr )  return;
    button->SetState(STATE_ENABLE, !m_program->readOnly);

    button = static_cast< CButton* >(pw->SearchControl(EVENT_STUDIO_OPEN));
    if ( button == nullptr )  return;
    button->SetState(STATE_ENABLE, !m_program->readOnly);

    button = static_cast< CButton* >(pw->SearchControl(EVENT_STUDIO_UNDO));
    if ( button == nullptr )  return;
    button->SetState(STATE_ENABLE, !m_program->readOnly);

    button = static_cast< CButton* >(pw->SearchControl(EVENT_STUDIO_CUT));
    if ( button == nullptr )  return;
    button->SetState(STATE_ENABLE, !m_program->readOnly);

    button = static_cast< CButton* >(pw->SearchControl(EVENT_STUDIO_PASTE));
    if ( button == nullptr )  return;
    button->SetState(STATE_ENABLE, !m_program->readOnly);
}

} // namespace Ui
