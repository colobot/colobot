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

// studio.cpp

#define STRICT
#define D3D_OVERLOADS

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <windows.h>
#include <direct.h>
#include <io.h>
#include <time.h>
#include <d3d.h>

#include "struct.h"
#include "d3dengine.h"
#include "d3dmath.h"
#include "language.h"
#include "event.h"
#include "misc.h"
#include "iman.h"
#include "restext.h"
#include "math3d.h"
#include "robotmain.h"
#include "object.h"
#include "camera.h"
#include "sound.h"
#include "script.h"
#include "interface.h"
#include "button.h"
#include "check.h"
#include "slider.h"
#include "edit.h"
#include "list.h"
#include "label.h"
#include "group.h"
#include "window.h"
#include "text.h"
#include "cbottoken.h"
#include "studio.h"




// Object's constructor.

CStudio::CStudio(CInstanceManager* iMan)
{
    m_iMan = iMan;
    m_iMan->AddInstance(CLASS_STUDIO, this);

    m_engine    = (CD3DEngine*)m_iMan->SearchInstance(CLASS_ENGINE);
    m_event     = (CEvent*)m_iMan->SearchInstance(CLASS_EVENT);
    m_interface = (CInterface*)m_iMan->SearchInstance(CLASS_INTERFACE);
    m_main      = (CRobotMain*)m_iMan->SearchInstance(CLASS_MAIN);
    m_camera    = (CCamera*)m_iMan->SearchInstance(CLASS_CAMERA);
    m_sound     = (CSound*)m_iMan->SearchInstance(CLASS_SOUND);

    m_bEditMaximized = FALSE;
    m_bEditMinimized = FALSE;

    m_time      = 0.0f;
    m_bRealTime = TRUE;
    m_bRunning  = FALSE;
    m_fixInfoTextTime = 0.0f;
    m_helpFilename[0] = 0;
    m_dialog = SD_NULL;
}

// Object's destructor.

CStudio::~CStudio()
{
    m_iMan->DeleteInstance(CLASS_STUDIO, this);
}


// Management of an event.

BOOL CStudio::EventProcess(const Event &event)
{
    CWindow*    pw;
    CEdit*      edit;
    CSlider*    slider;
    char        res[100];

    if ( m_dialog != SD_NULL )  // dialogue exists?
    {
        return EventDialog(event);
    }

    if ( event.event == EVENT_FRAME )
    {
        EventFrame(event);
    }

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW3);
    if ( pw == 0 )  return FALSE;

    edit = (CEdit*)pw->SearchControl(EVENT_STUDIO_EDIT);
    if ( edit == 0 )  return FALSE;

    if ( event.event == pw->RetEventMsgClose() )
    {
        Event newEvent = event;
        newEvent.event = EVENT_STUDIO_OK;
        m_event->AddEvent(newEvent);
    }

    if ( event.event == EVENT_STUDIO_EDIT )  // text modifief?
    {
        ColorizeScript(edit);
    }

    if ( event.event == EVENT_STUDIO_LIST )  // list clicked?
    {
        m_main->StartDisplayInfo(m_helpFilename, -1);
    }

    if ( event.event == EVENT_STUDIO_NEW )  // new?
    {
        m_script->New(edit, "");
    }

    if ( event.event == EVENT_STUDIO_OPEN )  // open?
    {
        StartDialog(SD_OPEN);
    }
    if ( event.event == EVENT_STUDIO_SAVE )  // save?
    {
        StartDialog(SD_SAVE);
    }

    if ( event.event == EVENT_STUDIO_UNDO )  // undo?
    {
        edit->Undo();
    }
    if ( event.event == EVENT_STUDIO_CUT )  // cut?
    {
        edit->Cut();
    }
    if ( event.event == EVENT_STUDIO_COPY )  // copy?
    {
        edit->Copy();
    }
    if ( event.event == EVENT_STUDIO_PASTE )  // paste?
    {
        edit->Paste();
    }

    if ( event.event == EVENT_STUDIO_SIZE )  // size?
    {
        slider = (CSlider*)pw->SearchControl(EVENT_STUDIO_SIZE);
        if ( slider == 0 )  return FALSE;
        m_main->SetFontSize(9.0f+slider->RetVisibleValue()*6.0f);
        ViewEditScript();
    }

    if ( event.event == EVENT_STUDIO_TOOL &&  // instructions?
         m_dialog == SD_NULL )
    {
        m_main->StartDisplayInfo(SATCOM_HUSTON, FALSE);
    }
    if ( event.event == EVENT_STUDIO_HELP &&  // help?
         m_dialog == SD_NULL )
    {
        m_main->StartDisplayInfo(SATCOM_PROG, FALSE);
    }

    if ( event.event == EVENT_STUDIO_COMPILE )  // compile?
    {
        char    buffer[100];

        if ( m_script->GetScript(edit) )  // compile
        {
            GetResource(RES_TEXT, RT_STUDIO_COMPOK, res);
            SetInfoText(res, FALSE);
        }
        else
        {
            m_script->GetError(buffer);
            SetInfoText(buffer, FALSE);
        }
    }

    if ( event.event == EVENT_STUDIO_RUN )  // run/stop?
    {
        if ( m_script->IsRunning() )
        {
            Event newEvent = event;
            newEvent.event = EVENT_OBJECT_PROGSTOP;
            m_event->AddEvent(newEvent);  // stop
        }
        else
        {
            if ( m_script->GetScript(edit) )  // compile
            {
                SetInfoText("", FALSE);

                Event newEvent = event;
                newEvent.event = EVENT_OBJECT_PROGSTART;
                m_event->AddEvent(newEvent);  // start
            }
            else
            {
                char    buffer[100];
                m_script->GetError(buffer);
                SetInfoText(buffer, FALSE);
            }
        }
    }

    if ( event.event == EVENT_STUDIO_REALTIME )  // real time?
    {
        m_bRealTime = !m_bRealTime;
        m_script->SetStepMode(!m_bRealTime);
        UpdateFlux();
        UpdateButtons();
    }

    if ( event.event == EVENT_STUDIO_STEP )  // step?
    {
        m_script->Step(event);
    }

    if ( event.event == EVENT_KEYDOWN )
    {
        if ( event.param == m_engine->RetKey(KEYRANK_CBOT, 0) ||
             event.param == m_engine->RetKey(KEYRANK_CBOT, 1) )
        {
            if ( m_helpFilename[0] != 0 )
            {
                m_main->StartDisplayInfo(m_helpFilename, -1);
            }
        }
    }

    if ( event.event == EVENT_WINDOW3 )  // window is moved?
    {
        m_editActualPos = m_editFinalPos = pw->RetPos();
        m_editActualDim = m_editFinalDim = pw->RetDim();
        m_main->SetWindowPos(m_editActualPos);
        m_main->SetWindowDim(m_editActualDim);
        AdjustEditScript();
    }
    if ( event.event == pw->RetEventMsgReduce() )
    {
        if ( m_bEditMinimized )
        {
            m_editFinalPos = m_main->RetWindowPos();
            m_editFinalDim = m_main->RetWindowDim();
            m_bEditMinimized = FALSE;
            m_bEditMaximized = FALSE;
        }
        else
        {
            m_editFinalPos.x =  0.00f;
            m_editFinalPos.y = -0.44f;
            m_editFinalDim.x =  1.00f;
            m_editFinalDim.y =  0.50f;
            m_bEditMinimized = TRUE;
            m_bEditMaximized = FALSE;
        }
        m_main->SetEditFull(m_bEditMaximized);
        pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW3);
        if ( pw != 0 )
        {
            pw->SetMaximized(m_bEditMaximized);
            pw->SetMinimized(m_bEditMinimized);
        }
    }
    if ( event.event == pw->RetEventMsgFull() )
    {
        if ( m_bEditMaximized )
        {
            m_editFinalPos = m_main->RetWindowPos();
            m_editFinalDim = m_main->RetWindowDim();
            m_bEditMinimized = FALSE;
            m_bEditMaximized = FALSE;
        }
        else
        {
            m_editFinalPos.x = 0.00f;
            m_editFinalPos.y = 0.00f;
            m_editFinalDim.x = 1.00f;
            m_editFinalDim.y = 1.00f;
            m_bEditMinimized = FALSE;
            m_bEditMaximized = TRUE;
        }
        m_main->SetEditFull(m_bEditMaximized);
        pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW3);
        if ( pw != 0 )
        {
            pw->SetMaximized(m_bEditMaximized);
            pw->SetMinimized(m_bEditMinimized);
        }
    }

    return TRUE;
}


// Evolves value with time elapsed.

float Evolution(float final, float actual, float time)
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

BOOL CStudio::EventFrame(const Event &event)
{
    CWindow*    pw;
    CEdit*      edit;
    CList*      list;
    float       time;
    int         cursor1, cursor2, iCursor1, iCursor2;
    char        res[100];

    m_time += event.rTime;
    m_fixInfoTextTime -= event.rTime;

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW3);
    if ( pw == 0 )  return FALSE;

    edit = (CEdit*)pw->SearchControl(EVENT_STUDIO_EDIT);
    if ( edit == 0 )  return FALSE;

    list = (CList*)pw->SearchControl(EVENT_STUDIO_LIST);
    if ( list == 0 )  return FALSE;

    if ( !m_script->IsRunning() && m_bRunning )  // stop?
    {
        m_bRunning = FALSE;
        UpdateFlux();  // stop
        AdjustEditScript();
        GetResource(RES_TEXT, RT_STUDIO_PROGSTOP, res);
        SetInfoText(res, FALSE);

        Event newEvent = event;
        newEvent.event = EVENT_OBJECT_PROGSTOP;
        m_event->AddEvent(newEvent);  // stop
    }

    if ( m_script->IsRunning() && !m_bRunning )  // starting?
    {
        m_bRunning = TRUE;
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

    return TRUE;
}


// Indicates whether a character is part of a word.

BOOL IsToken(int character)
{
    char    c;

    c = tolower(RetNoAccent(character));

    return ( (c >= 'a' && c <= 'z') ||
             (c >= '0' && c <= '9') ||
             c == '_' );
}

// Seeks if the cursor is on a keyword.

void CStudio::SearchToken(CEdit* edit)
{
    ObjectType  type;
    int         len, cursor1, cursor2, i, character, level;
    char*       text;
    char        token[100];

    text = edit->RetText();
    len  = edit->RetTextLength();
    edit->GetCursor(cursor1, cursor2);

    i = cursor1;
    if ( i > 0 )
    {
        character = (unsigned char)text[i-1];
        if ( !IsToken(character) )
        {
            level = 1;
            while ( i > 0 )
            {
                character = (unsigned char)text[i-1];
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
                m_helpFilename[0] = 0;
                SetInfoText("", TRUE);
                return;
            }
            while ( i > 0 )
            {
                character = (unsigned char)text[i-1];
                if ( IsToken(character) )  break;
                i --;
            }
        }
    }

    while ( i > 0 )
    {
        character = (unsigned char)text[i-1];
        if ( !IsToken(character) )  break;
        i --;
    }
    cursor2 = i;

    while ( i < len )
    {
        character = (unsigned char)text[i];
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

    strcpy(m_helpFilename, RetHelpFilename(token));
    if ( m_helpFilename[0] == 0 )
    {
        for ( i=0 ; i<OBJECT_MAX ; i++ )
        {
            type = (ObjectType)i;
            text = RetObjectName(type);
            if ( text[0] != 0 )
            {
                if ( strcmp(token, text) == 0 )
                {
                    strcpy(m_helpFilename, RetHelpFilename(type));
                    SetInfoText(token, TRUE);
                    return;
                }
            }
            text = RetObjectAlias(type);
            if ( text[0] != 0 )
            {
                if ( strcmp(token, text) == 0 )
                {
                    strcpy(m_helpFilename, RetHelpFilename(type));
                    SetInfoText(token, TRUE);
                    return;
                }
            }
        }
    }

    text = RetHelpText(token);
    if ( text[0] == 0 && m_helpFilename[0] != 0 )
    {
        SetInfoText(token, TRUE);
    }
    else
    {
        SetInfoText(text, TRUE);
    }
}

// Colors the text according to syntax.

void CStudio::ColorizeScript(CEdit* edit)
{
    m_script->ColorizeScript(edit);
}


// Starts editing a program.

void CStudio::StartEditScript(CScript *script, char* name, int rank)
{
    FPOINT      pos, dim;
    CWindow*    pw;
    CEdit*      edit;
    CButton*    button;
    CSlider*    slider;
    CList*      list;
    char        res[100];

    m_script = script;
    m_rank   = rank;

    m_main->SetEditLock(TRUE, TRUE);
    m_main->SetEditFull(FALSE);
    m_bInitPause = m_engine->RetPause();
    m_main->SetSpeed(1.0f);
    m_editCamera = m_camera->RetType();
    m_camera->SetType(CAMERA_EDIT);

    m_bRunning = m_script->IsRunning();
    m_bRealTime = m_bRunning;
    m_script->SetStepMode(!m_bRealTime);

    button = (CButton*)m_interface->SearchControl(EVENT_BUTTON_QUIT);
    if ( button != 0 )
    {
        button->ClearState(STATE_VISIBLE);
    }

    pos = m_editFinalPos = m_editActualPos = m_main->RetWindowPos();
    dim = m_editFinalDim = m_editActualDim = m_main->RetWindowDim();
    pw = m_interface->CreateWindows(pos, dim, 8, EVENT_WINDOW3);
    if ( pw == 0 )  return;
    pw->SetState(STATE_SHADOW);
    pw->SetRedim(TRUE);  // before SetName!
    pw->SetMovable(TRUE);
    pw->SetClosable(TRUE);
    GetResource(RES_TEXT, RT_STUDIO_TITLE, res);
    pw->SetName(res);
    pw->SetMinDim(FPOINT(0.49f, 0.50f));
    pw->SetMaximized(m_bEditMaximized);
    pw->SetMinimized(m_bEditMinimized);
    m_main->SetEditFull(m_bEditMaximized);

    edit = pw->CreateEdit(pos, dim, 0, EVENT_STUDIO_EDIT);
    if ( edit == 0 )  return;
    edit->SetState(STATE_SHADOW);
    edit->SetInsideScroll(FALSE);
//? if ( m_bRunning )  edit->SetEdit(FALSE);
    edit->SetMaxChar(EDITSTUDIOMAX);
    edit->SetFontType(FONT_COURIER);
    edit->SetFontStretch(0.7f);
    edit->SetDisplaySpec(TRUE);
    edit->SetAutoIndent(m_engine->RetEditIndentMode());
    m_script->PutScript(edit, name);
    ColorizeScript(edit);

    ViewEditScript();

    list = pw->CreateList(pos, dim, 1, EVENT_STUDIO_LIST, 1.2f);
    list->SetState(STATE_SHADOW);
    list->SetFontType(FONT_COURIER);
    list->SetSelectCap(FALSE);
    list->SetFontSize(SMALLFONT*0.85f);
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
    slider->SetVisibleValue((m_main->RetFontSize()-9.0f)/6.0f);
    pw->CreateGroup(pos, dim, 19, EVENT_LABEL1);  // SatCom logo
    button = pw->CreateButton(pos, dim, 128+57, EVENT_STUDIO_TOOL);
    button->SetState(STATE_SHADOW);
    button = pw->CreateButton(pos, dim, 128+60, EVENT_STUDIO_HELP);
    button->SetState(STATE_SHADOW);

    button = pw->CreateButton(pos, dim, -1, EVENT_STUDIO_OK);
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
    FPOINT      wpos, wdim, pos, dim, ppos, ddim;
    float       hList;

    wpos = m_editActualPos;
    wdim = m_editActualDim;

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW3);
    if ( pw != 0 )
    {
        pw->SetPos(wpos);
        pw->SetDim(wdim);
        wdim = pw->RetDim();
    }

    if ( m_bRunning ) hList = 80.0f/480.0f;
    else              hList = 20.0f/480.0f;

    pos.x = wpos.x+0.01f;
    pos.y = wpos.y+0.09f+hList;
    dim.x = wdim.x-0.02f;
    dim.y = wdim.y-0.22f-hList;
    edit = (CEdit*)pw->SearchControl(EVENT_STUDIO_EDIT);
    if ( edit != 0 )
    {
        edit->SetPos(pos);
        edit->SetDim(dim);
    }

    pos.x = wpos.x+0.01f;
    pos.y = wpos.y+0.09f;
    dim.x = wdim.x-0.02f;
    dim.y = hList;
    list = (CList*)pw->SearchControl(EVENT_STUDIO_LIST);
    if ( list != 0 )
    {
        list->SetPos(pos);
        list->SetDim(dim);
    }

    dim.x = 0.04f;
    dim.y = 0.04f*1.5f;
    dim.y = 25.0f/480.0f;

    pos.y = wpos.y+wdim.y-dim.y-0.06f;
    pos.x = wpos.x+0.01f;
    button = (CButton*)pw->SearchControl(EVENT_STUDIO_NEW);
    if ( button != 0 )
    {
        button->SetPos(pos);
        button->SetDim(dim);
    }
    pos.x = wpos.x+0.05f;
    button = (CButton*)pw->SearchControl(EVENT_STUDIO_OPEN);
    if ( button != 0 )
    {
        button->SetPos(pos);
        button->SetDim(dim);
    }
    pos.x = wpos.x+0.09f;
    button = (CButton*)pw->SearchControl(EVENT_STUDIO_SAVE);
    if ( button != 0 )
    {
        button->SetPos(pos);
        button->SetDim(dim);
    }
    pos.x = wpos.x+0.14f;
    button = (CButton*)pw->SearchControl(EVENT_STUDIO_UNDO);
    if ( button != 0 )
    {
        button->SetPos(pos);
        button->SetDim(dim);
    }
    pos.x = wpos.x+0.19f;
    button = (CButton*)pw->SearchControl(EVENT_STUDIO_CUT);
    if ( button != 0 )
    {
        button->SetPos(pos);
        button->SetDim(dim);
    }
    pos.x = wpos.x+0.23f;
    button = (CButton*)pw->SearchControl(EVENT_STUDIO_COPY);
    if ( button != 0 )
    {
        button->SetPos(pos);
        button->SetDim(dim);
    }
    pos.x = wpos.x+0.27f;
    button = (CButton*)pw->SearchControl(EVENT_STUDIO_PASTE);
    if ( button != 0 )
    {
        button->SetPos(pos);
        button->SetDim(dim);
    }
    pos.x = wpos.x+0.32f;
    slider = (CSlider*)pw->SearchControl(EVENT_STUDIO_SIZE);
    if ( slider != 0 )
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
    group = (CGroup*)pw->SearchControl(EVENT_LABEL1);
    if ( group != 0 )
    {
        group->SetPos(pos);
        group->SetDim(dim);
    }
    pos.x = wpos.x+0.40f;
    button = (CButton*)pw->SearchControl(EVENT_STUDIO_TOOL);
    if ( button != 0 )
    {
        button->SetPos(pos);
        button->SetDim(dim);
    }
    pos.x = wpos.x+0.44f;
    button = (CButton*)pw->SearchControl(EVENT_STUDIO_HELP);
    if ( button != 0 )
    {
        button->SetPos(pos);
        button->SetDim(dim);
    }

    pos.y = wpos.y+0.02f;
    pos.x = wpos.x+0.01f;
    dim.x = 80.0f/640.0f;
    dim.y = 25.0f/480.0f;
    button = (CButton*)pw->SearchControl(EVENT_STUDIO_OK);
    if ( button != 0 )
    {
        button->SetPos(pos);
        button->SetDim(dim);
    }
    pos.x = wpos.x+0.14f;
    button = (CButton*)pw->SearchControl(EVENT_STUDIO_CANCEL);
    if ( button != 0 )
    {
        button->SetPos(pos);
        button->SetDim(dim);
    }
    pos.x = wpos.x+0.28f;
    dim.x = dim.y*0.75f;
    button = (CButton*)pw->SearchControl(EVENT_STUDIO_COMPILE);
    if ( button != 0 )
    {
        button->SetPos(pos);
        button->SetDim(dim);
    }
    pos.x = wpos.x+0.28f+dim.x*1;
    button = (CButton*)pw->SearchControl(EVENT_STUDIO_RUN);
    if ( button != 0 )
    {
        button->SetPos(pos);
        button->SetDim(dim);
    }
    pos.x = wpos.x+0.28f+dim.x*2;
    button = (CButton*)pw->SearchControl(EVENT_STUDIO_REALTIME);
    if ( button != 0 )
    {
        button->SetPos(pos);
        button->SetDim(dim);
    }
    pos.x = wpos.x+0.28f+dim.x*3;
    button = (CButton*)pw->SearchControl(EVENT_STUDIO_STEP);
    if ( button != 0 )
    {
        button->SetPos(pos);
        button->SetDim(dim);
    }
}

// Ends edition of a program.

BOOL CStudio::StopEditScript(BOOL bCancel)
{
    CWindow*    pw;
    CEdit*      edit;
    CButton*    button;
    char        buffer[100];

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW3);
    if ( pw == 0 )  return FALSE;

    if ( !bCancel && !m_script->IsRunning() )
    {
        edit = (CEdit*)pw->SearchControl(EVENT_STUDIO_EDIT);
        if ( edit != 0 )
        {
            if ( !m_script->GetScript(edit) )  // compile
            {
                m_script->GetError(buffer);
                SetInfoText(buffer, FALSE);
                return FALSE;
            }
        }
    }
    m_script->SetStepMode(FALSE);

    m_interface->DeleteControl(EVENT_WINDOW3);

    button = (CButton*)m_interface->SearchControl(EVENT_BUTTON_QUIT);
    if ( button != 0 )
    {
        button->SetState(STATE_VISIBLE);
    }

    if ( !m_bInitPause )  m_engine->SetPause(FALSE);
    m_sound->MuteAll(FALSE);
    m_main->SetEditLock(FALSE, TRUE);
    m_camera->SetType(m_editCamera);
    return TRUE;
}

// Specifies the message to display.
// The messages are not clickable 8 seconds,
// even if a message was clickable poster before.

void CStudio::SetInfoText(char *text, BOOL bClickable)
{
    CWindow*    pw;
    CList*      list;
    char        res[100];

    if ( bClickable && m_fixInfoTextTime > 0.0f )  return;
    if ( !bClickable )  m_fixInfoTextTime = 8.0f;

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW3);
    if ( pw == 0 )  return;

    list = (CList*)pw->SearchControl(EVENT_STUDIO_LIST);
    if ( list == 0 )  return;

    list->Flush();  // just text
    list->SetName(0, text);

    if ( text[0] == 0 )  bClickable = FALSE;
    list->SetSelectCap(bClickable);

    if ( bClickable )
    {
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


// Changing the size of a editing program.

void CStudio::ViewEditScript()
{
    CWindow*    pw;
    CEdit*      edit;
    POINT       dim;

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW3);
    if ( pw == 0 )  return;

    edit = (CEdit*)pw->SearchControl(EVENT_STUDIO_EDIT);
    if ( edit == 0 )  return;

    dim = m_engine->RetDim();
    edit->SetFontSize(m_main->RetFontSize()/(dim.x/640.0f));
}


// Updates the operating mode.

void CStudio::UpdateFlux()
{
    if ( m_bRunning )
    {
#if 1
        if ( m_bRealTime )  // run?
        {
            m_engine->SetPause(FALSE);
            m_sound->MuteAll(FALSE);
        }
        else    // step by step?
        {
            m_engine->SetPause(TRUE);
            m_sound->MuteAll(TRUE);
        }
#else
        m_engine->SetPause(FALSE);
        m_sound->MuteAll(FALSE);
#endif
    }
    else    // stop?
    {
        m_engine->SetPause(TRUE);
        m_sound->MuteAll(TRUE);
    }
}

// Updates the buttons.

void CStudio::UpdateButtons()
{
    CWindow*    pw;
    CEdit*      edit;
    CButton*    button;

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW3);
    if ( pw == 0 )  return;

    edit = (CEdit*)pw->SearchControl(EVENT_STUDIO_EDIT);
    if ( edit == 0 )  return;

    if ( m_bRunning )
    {
        edit->SetIcon(1);  // red background
        edit->SetEditCap(FALSE);  // just to see
        edit->SetHiliteCap(TRUE);
    }
    else
    {
        edit->SetIcon(0);  // standard background
        edit->SetEditCap(TRUE);
        edit->SetHiliteCap(TRUE);
    }

    button = (CButton*)pw->SearchControl(EVENT_STUDIO_COMPILE);
    if ( button == 0 )  return;
    button->SetState(STATE_ENABLE, !m_bRunning);

    button = (CButton*)pw->SearchControl(EVENT_STUDIO_RUN);
    if ( button == 0 )  return;
    button->SetIcon(m_bRunning?8:21);  // stop/run

    button = (CButton*)pw->SearchControl(EVENT_STUDIO_REALTIME);
    if ( button == 0 )  return;
    button->SetIcon(m_bRealTime?64+22:64+21);
    button->SetState(STATE_ENABLE, (!m_bRunning || !m_script->IsContinue()));

    button = (CButton*)pw->SearchControl(EVENT_STUDIO_STEP);
    if ( button == 0 )  return;
    button->SetState(STATE_ENABLE, (m_bRunning && !m_bRealTime && !m_script->IsContinue()));
}


// Beginning of the display of a dialogue.

void CStudio::StartDialog(StudioDialog type)
{
    CWindow*    pw;
    CButton*    pb;
    CCheck*     pc;
    CLabel*     pla;
    CList*      pli;
    CEdit*      pe;
    FPOINT      pos, dim;
    char        name[100];

    m_dialog = type;

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW0);
    if ( pw != 0 )  pw->ClearState(STATE_ENABLE);

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW1);
    if ( pw != 0 )  pw->ClearState(STATE_ENABLE);

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW3);
    if ( pw != 0 )  pw->ClearState(STATE_ENABLE);

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW3);
    if ( pw != 0 )  pw->ClearState(STATE_ENABLE);

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW4);
    if ( pw != 0 )  pw->ClearState(STATE_ENABLE);

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW5);
    if ( pw != 0 )  pw->ClearState(STATE_ENABLE);

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW6);
    if ( pw != 0 )  pw->ClearState(STATE_ENABLE);

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW7);
    if ( pw != 0 )  pw->ClearState(STATE_ENABLE);

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW8);
    if ( pw != 0 )  pw->ClearState(STATE_ENABLE);

    if ( m_dialog == SD_OPEN ||
         m_dialog == SD_SAVE )
    {
        pos = m_main->RetIOPos();
        dim = m_main->RetIODim();
    }
//? pw = m_interface->CreateWindows(pos, dim, 8, EVENT_WINDOW9);
    pw = m_interface->CreateWindows(pos, dim, m_dialog==SD_OPEN?14:13, EVENT_WINDOW9);
    pw->SetState(STATE_SHADOW);
    pw->SetMovable(TRUE);
    pw->SetClosable(TRUE);
    pw->SetMinDim(FPOINT(320.0f/640.0f, (121.0f+18.0f*4)/480.0f));
    if ( m_dialog == SD_OPEN )  GetResource(RES_TEXT, RT_IO_OPEN, name);
    if ( m_dialog == SD_SAVE )  GetResource(RES_TEXT, RT_IO_SAVE, name);
    pw->SetName(name);

    pos = FPOINT(0.0f, 0.0f);
    dim = FPOINT(0.0f, 0.0f);

    if ( m_dialog == SD_OPEN ||
         m_dialog == SD_SAVE )
    {
        GetResource(RES_TEXT, RT_IO_LIST, name);
        pla = pw->CreateLabel(pos, dim, 0, EVENT_DIALOG_LABEL1, name);
        pla->SetJustif(1);

        pli = pw->CreateList(pos, dim, 0, EVENT_DIALOG_LIST);
        pli->SetState(STATE_SHADOW);

        GetResource(RES_TEXT, RT_IO_NAME, name);
        pla = pw->CreateLabel(pos, dim, 0, EVENT_DIALOG_LABEL2, name);
        pla->SetJustif(1);

        pe = pw->CreateEdit(pos, dim, 0, EVENT_DIALOG_EDIT);
        pe->SetState(STATE_SHADOW);
        if ( m_dialog == SD_SAVE )
        {
            pe->SetText(m_script->RetFilename());
        }

        GetResource(RES_TEXT, RT_IO_DIR, name);
        pla = pw->CreateLabel(pos, dim, 0, EVENT_DIALOG_LABEL3, name);
        pla->SetJustif(1);

        pc = pw->CreateCheck(pos, dim, 0, EVENT_DIALOG_CHECK1);
        GetResource(RES_TEXT, RT_IO_PRIVATE, name);
        pc->SetName(name);
        pc->SetState(STATE_SHADOW);
#if _POLISH
        pc->SetFontSize(8.0f);
#endif

        pc = pw->CreateCheck(pos, dim, 0, EVENT_DIALOG_CHECK2);
        GetResource(RES_TEXT, RT_IO_PUBLIC, name);
        pc->SetName(name);
        pc->SetState(STATE_SHADOW);
#if _POLISH
        pc->SetFontSize(8.0f);
#endif

        pb = pw->CreateButton(pos, dim, -1, EVENT_DIALOG_OK);
        pb->SetState(STATE_SHADOW);
        if ( m_dialog == SD_OPEN )  GetResource(RES_TEXT, RT_IO_OPEN, name);
        if ( m_dialog == SD_SAVE )  GetResource(RES_TEXT, RT_IO_SAVE, name);
        pb->SetName(name);

        pb = pw->CreateButton(pos, dim, -1, EVENT_DIALOG_CANCEL);
        pb->SetState(STATE_SHADOW);
        GetResource(RES_EVENT, EVENT_DIALOG_CANCEL, name);
        pb->SetName(name);

        AdjustDialog();
        UpdateDialogList();
        UpdateDialogPublic();
        UpdateDialogAction();

        pe->SetCursor(999, 0);  // selects all
        pe->SetFocus(TRUE);
    }

    m_main->SetSatComLock(TRUE);  // impossible to use the SatCom
}

// End of the display of a dialogue.

void CStudio::StopDialog()
{
    CWindow*    pw;

    if ( m_dialog == SD_NULL )  return;
    m_dialog = SD_NULL;

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW0);
    if ( pw != 0 )  pw->SetState(STATE_ENABLE);

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW1);
    if ( pw != 0 )  pw->SetState(STATE_ENABLE);

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW3);
    if ( pw != 0 )  pw->SetState(STATE_ENABLE);

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW3);
    if ( pw != 0 )  pw->SetState(STATE_ENABLE);

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW4);
    if ( pw != 0 )  pw->SetState(STATE_ENABLE);

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW5);
    if ( pw != 0 )  pw->SetState(STATE_ENABLE);

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW6);
    if ( pw != 0 )  pw->SetState(STATE_ENABLE);

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW7);
    if ( pw != 0 )  pw->SetState(STATE_ENABLE);

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW8);
    if ( pw != 0 )  pw->SetState(STATE_ENABLE);

    m_interface->DeleteControl(EVENT_WINDOW9);
    m_main->SetSatComLock(FALSE);  // possible to use the SatCom
}

// Adjust all controls of dialogue after a change in geometry.

void CStudio::AdjustDialog()
{
    CWindow*    pw;
    CButton*    pb;
    CCheck*     pc;
    CLabel*     pla;
    CList*      pli;
    CEdit*      pe;
    FPOINT      wpos, wdim, ppos, ddim;
    int         nli, nch;
    char        name[100];

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW9);
    if ( pw == 0 )  return;

    wpos = pw->RetPos();
    wdim = pw->RetDim();
    pw->SetPos(wpos);  // to move the buttons on the titlebar

    if ( m_dialog == SD_OPEN ||
         m_dialog == SD_SAVE )
    {
        ppos.x = wpos.x+10.0f/640.0f;
        ppos.y = wpos.y+wdim.y-55.0f/480.0f;
        ddim.x = wdim.x-20.0f/640.0f;
        ddim.y = 20.0f/480.0f;
        pla = (CLabel*)pw->SearchControl(EVENT_DIALOG_LABEL1);
        if ( pla != 0 )
        {
            pla->SetPos(ppos);
            pla->SetDim(ddim);
        }

        nli = (int)((wdim.y-120.0f/480.0f)/(18.0f/480.0f));
        ddim.y = nli*18.0f/480.0f+9.0f/480.0f;
        ppos.y = wpos.y+wdim.y-48.0f/480.0f-ddim.y;
        pli = (CList*)pw->SearchControl(EVENT_DIALOG_LIST);
        if ( pli != 0 )
        {
            pli->SetPos(ppos);
            pli->SetDim(ddim);
            pli->SetTabs(0, ddim.x-(50.0f+130.0f+16.0f)/640.0f);
            pli->SetTabs(1,  50.0f/640.0f, -1);
            pli->SetTabs(2, 130.0f/640.0f);
//?         pli->ShowSelect();
        }

        ppos.y = wpos.y+30.0f/480.0f;
        ddim.x = 50.0f/640.0f;
        ddim.y = 20.0f/480.0f;
        pla = (CLabel*)pw->SearchControl(EVENT_DIALOG_LABEL2);
        if ( pla != 0 )
        {
            pla->SetPos(ppos);
            pla->SetDim(ddim);
        }

        ppos.x += 50.0f/640.0f;
        ppos.y = wpos.y+36.0f/480.0f;
        ddim.x = wdim.x-170.0f/640.0f;
        pe = (CEdit*)pw->SearchControl(EVENT_DIALOG_EDIT);
        if ( pe != 0 )
        {
            pe->SetPos(ppos);
            pe->SetDim(ddim);

            nch = (int)((ddim.x*640.0f-22.0f)/8.0f);
            pe->GetText(name, 100);
            pe->SetMaxChar(nch);
            name[nch] = 0;  // truncates the text according to max
            pe->SetText(name);
        }

        ppos.x = wpos.x+10.0f/640.0f;
        ppos.y = wpos.y+5.0f/480.0f;
        ddim.x = 50.0f/640.0f;
        ddim.y = 16.0f/480.0f;
        pla = (CLabel*)pw->SearchControl(EVENT_DIALOG_LABEL3);
        if ( pla != 0 )
        {
            pla->SetPos(ppos);
            pla->SetDim(ddim);
        }

        ppos.x += 50.0f/640.0f;
        ppos.y = wpos.y+12.0f/480.0f;
        ddim.x = 70.0f/640.0f;
        pc = (CCheck*)pw->SearchControl(EVENT_DIALOG_CHECK1);
        if ( pc != 0 )
        {
            pc->SetPos(ppos);
            pc->SetDim(ddim);
        }

        ppos.x += 80.0f/640.0f;
        pc = (CCheck*)pw->SearchControl(EVENT_DIALOG_CHECK2);
        if ( pc != 0 )
        {
            pc->SetPos(ppos);
            pc->SetDim(ddim);
        }

        ppos.x = wpos.x+wdim.x-100.0f/640.0f;
        ppos.y = wpos.y+34.0f/480.0f;
        ddim.x = 90.0f/640.0f;
        ddim.y = 23.0f/480.0f;
        pb = (CButton*)pw->SearchControl(EVENT_DIALOG_OK);
        if ( pb != 0 )
        {
            pb->SetPos(ppos);
            pb->SetDim(ddim);
        }

        ppos.y -= 26.0f/480.0f;
        pb = (CButton*)pw->SearchControl(EVENT_DIALOG_CANCEL);
        if ( pb != 0 )
        {
            pb->SetPos(ppos);
            pb->SetDim(ddim);
        }
    }
}

// Management of the event of a dialogue.

BOOL CStudio::EventDialog(const Event &event)
{
    CWindow*    pw;
    FPOINT      wpos, wdim;

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW9);
    if ( pw == 0 )  return FALSE;

    if ( event.event == EVENT_WINDOW9 )  // window is moved?
    {
        wpos = pw->RetPos();
        wdim = pw->RetDim();
        m_main->SetIOPos(wpos);
        m_main->SetIODim(wdim);
        AdjustDialog();
    }

    if ( m_dialog == SD_OPEN ||
         m_dialog == SD_SAVE )
    {
        if ( event.event == EVENT_DIALOG_LIST )
        {
            UpdateChangeList();
        }
        if ( event.event == EVENT_DIALOG_EDIT )
        {
            UpdateChangeEdit();
        }

        if ( event.event == EVENT_DIALOG_CHECK1 )  // private?
        {
            m_main->SetIOPublic(FALSE);
            UpdateDialogPublic();
            UpdateDialogList();
        }
        if ( event.event == EVENT_DIALOG_CHECK2 )  // public?
        {
            m_main->SetIOPublic(TRUE);
            UpdateDialogPublic();
            UpdateDialogList();
        }
    }

    if ( event.event == EVENT_DIALOG_OK ||
         (event.event == EVENT_KEYDOWN && event.param == VK_RETURN) )
    {
        if ( m_dialog == SD_OPEN )
        {
            if ( !ReadProgram() )  return TRUE;
        }
        if ( m_dialog == SD_SAVE )
        {
            if ( !WriteProgram() )  return TRUE;
        }

        StopDialog();
        return TRUE;
    }

    if ( event.event == EVENT_DIALOG_CANCEL ||
         (event.event == EVENT_KEYDOWN && event.param == VK_ESCAPE) ||
         event.event == pw->RetEventMsgClose() )
    {
        StopDialog();
        return TRUE;
    }

    return TRUE;
}

// Updates the name after a click in the list.

void CStudio::UpdateChangeList()
{
    CWindow*    pw;
    CList*      pl;
    CEdit*      pe;
    char        name[100];
    char*       p;

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW9);
    if ( pw == 0 )  return;
    pl = (CList*)pw->SearchControl(EVENT_DIALOG_LIST);
    if ( pl == 0 )  return;
    pe = (CEdit*)pw->SearchControl(EVENT_DIALOG_EDIT);
    if ( pe == 0 )  return;

    strcpy(name, pl->RetName(pl->RetSelect()));
    name[pe->RetMaxChar()] = 0;  // truncates according lg max editable
    p = strchr(name, '\t');  // seeks first tab
    if ( p != 0 )  *p = 0;
    pe->SetText(name);
    pe->SetCursor(999, 0);  // selects all
    pe->SetFocus(TRUE);

    UpdateDialogAction();
}

// Updates the list after a change in name.

void CStudio::UpdateChangeEdit()
{
    CWindow*    pw;
    CList*      pl;

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW9);
    if ( pw == 0 )  return;
    pl = (CList*)pw->SearchControl(EVENT_DIALOG_LIST);
    if ( pl == 0 )  return;

    pl->SetSelect(-1);

    UpdateDialogAction();
}

// Updates the action button.

void CStudio::UpdateDialogAction()
{
    CWindow*    pw;
    CEdit*      pe;
    CButton*    pb;
    char        name[100];
    int         len, i;
    BOOL        bError;

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW9);
    if ( pw == 0 )  return;
    pe = (CEdit*)pw->SearchControl(EVENT_DIALOG_EDIT);
    if ( pe == 0 )  return;
    pb = (CButton*)pw->SearchControl(EVENT_DIALOG_OK);
    if ( pb == 0 )  return;

    pe->GetText(name, 100);
    len = strlen(name);
    if ( len == 0 )
    {
        bError = TRUE;
    }
    else
    {
        bError = FALSE;
        for ( i=0 ; i<len ; i++ )
        {
            if ( name[i] == '*'  ||
                 name[i] == '?'  ||
                 name[i] == ':'  ||
                 name[i] == '<'  ||
                 name[i] == '>'  ||
                 name[i] == '"'  ||
                 name[i] == '|'  ||
                 name[i] == '/'  ||
                 name[i] == '\\' )  bError = TRUE;
        }
    }

    pb->SetState(STATE_ENABLE, !bError);
}

// Updates the buttons private/public.

void CStudio::UpdateDialogPublic()
{
    CWindow*    pw;
    CCheck*     pc;
    CLabel*     pl;
    char        name[100];
    char        dir[_MAX_FNAME];
    char        text[_MAX_FNAME+100];

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW9);
    if ( pw == 0 )  return;

    pc = (CCheck*)pw->SearchControl(EVENT_DIALOG_CHECK1);
    if ( pc != 0 )
    {
        pc->SetState(STATE_CHECK, !m_main->RetIOPublic());
    }

    pc = (CCheck*)pw->SearchControl(EVENT_DIALOG_CHECK2);
    if ( pc != 0 )
    {
        pc->SetState(STATE_CHECK, m_main->RetIOPublic());
    }

    pl = (CLabel*)pw->SearchControl(EVENT_DIALOG_LABEL1);
    if ( pl != 0 )
    {
        GetResource(RES_TEXT, RT_IO_LIST, name);
        SearchDirectory(dir, FALSE);
        sprintf(text, name, dir);
        pl->SetName(text, FALSE);
    }
}

// Fills the list with all programs saved.

void CStudio::UpdateDialogList()
{
    CWindow*            pw;
    CList*              pl;
    long                hFile;
    struct _finddata_t  fileBuffer;
    struct _finddata_t* listBuffer;
    BOOL                bDo;
    char                dir[_MAX_FNAME];
    char                temp[_MAX_FNAME];
    int                 nbFilenames, i;

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW9);
    if ( pw == 0 )  return;
    pl = (CList*)pw->SearchControl(EVENT_DIALOG_LIST);
    if ( pl == 0 )  return;
    pl->Flush();

    nbFilenames = 0;
    listBuffer = (_finddata_t*)malloc(sizeof(_finddata_t)*1000);

    SearchDirectory(dir, FALSE);
    strcat(dir, "*");  // list all
    hFile = _findfirst(dir, &fileBuffer);
    if ( hFile != -1 )
    {
        do
        {
            if ( (fileBuffer.attrib & _A_SUBDIR) == 0 )
            {
                listBuffer[nbFilenames++] = fileBuffer;
            }
        }
        while ( _findnext(hFile, &fileBuffer) == 0 && nbFilenames < 1000 );
    }
    do  // sorts all names:
    {
        bDo = FALSE;
        for ( i=0 ; i<nbFilenames-1 ; i++ )
        {
            if ( strcmp(listBuffer[i].name, listBuffer[i+1].name) > 0 )
            {
                fileBuffer = listBuffer[i];  // exchange i and i +1
                listBuffer[i] = listBuffer[i+1];
                listBuffer[i+1] = fileBuffer;
                bDo = TRUE;
            }
        }
    }
    while ( bDo );

    for ( i=0 ; i<nbFilenames ; i++ )
    {
        TimeToAscii(listBuffer[i].time_write, dir);
        sprintf(temp, "%s\t%d  \t%s", listBuffer[i].name, listBuffer[i].size, dir);
        pl->SetName(i, temp);
    }

    free(listBuffer);
}

// Constructs the name of the folder or open/save.
// If the folder does not exist, it will be created.

void CStudio::SearchDirectory(char *dir, BOOL bCreate)
{
    if ( m_main->RetIOPublic() )
    {
        sprintf(dir, "%s\\", m_main->RetPublicDir());
    }
    else
    {
        sprintf(dir, "%s\\%s\\Program\\", m_main->RetSavegameDir(), m_main->RetGamerName());
    }

    if ( bCreate )
    {
        _mkdir(dir);  // if does not exist yet!
    }
}

// Reads a new program.

BOOL CStudio::ReadProgram()
{
    CWindow*    pw;
    CEdit*      pe;
    char        filename[100];
    char        dir[100];
    char*       p;

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW9);
    if ( pw == 0 )  return FALSE;

    pe = (CEdit*)pw->SearchControl(EVENT_DIALOG_EDIT);
    if ( pe == 0 )  return FALSE;
    pe->GetText(filename, 100);
    if ( filename[0] == 0 )  return FALSE;

    p = strstr(filename, ".txt");
    if ( p == 0 || p != filename+strlen(filename)-4 )
    {
        strcat(filename, ".txt");
    }
    SearchDirectory(dir, TRUE);
    strcat(dir, filename);

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW3);
    if ( pw == 0 )  return FALSE;
    pe = (CEdit*)pw->SearchControl(EVENT_STUDIO_EDIT);
    if ( pe == 0 )  return FALSE;

    if ( !pe->ReadText(dir) )  return FALSE;

    m_script->SetFilename(filename);
    ColorizeScript(pe);
    return TRUE;
}

// Writes the current program.

BOOL CStudio::WriteProgram()
{
    CWindow*    pw;
    CEdit*      pe;
    char        filename[100];
    char        dir[100];
    char*       p;

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW9);
    if ( pw == 0 )  return FALSE;

    pe = (CEdit*)pw->SearchControl(EVENT_DIALOG_EDIT);
    if ( pe == 0 )  return FALSE;
    pe->GetText(filename, 100);
    if ( filename[0] == 0 )  return FALSE;

    p = strstr(filename, ".txt");
    if ( p == 0 || p != filename+strlen(filename)-4 )
    {
        strcat(filename, ".txt");
    }
    SearchDirectory(dir, TRUE);
    strcat(dir, filename);

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW3);
    if ( pw == 0 )  return FALSE;
    pe = (CEdit*)pw->SearchControl(EVENT_STUDIO_EDIT);
    if ( pe == 0 )  return FALSE;

    if ( !pe->WriteText(dir) )  return FALSE;

    m_script->SetFilename(filename);
    return TRUE;
}

