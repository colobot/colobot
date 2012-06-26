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

// displayinfo.cpp

#define STRICT
#define D3D_OVERLOADS

#include <windows.h>
#include <stdio.h>
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
#include "camera.h"
#include "object.h"
#include "motion.h"
#include "motiontoto.h"
#include "interface.h"
#include "button.h"
#include "slider.h"
#include "edit.h"
#include "group.h"
#include "window.h"
#include "particule.h"
#include "light.h"
#include "text.h"
#include "cbottoken.h"
#include "displayinfo.h"




// Object's constructor.

CDisplayInfo::CDisplayInfo(CInstanceManager* iMan)
{
    m_iMan = iMan;
    m_iMan->AddInstance(CLASS_STUDIO, this);

    m_engine    = (CD3DEngine*)m_iMan->SearchInstance(CLASS_ENGINE);
    m_event     = (CEvent*)m_iMan->SearchInstance(CLASS_EVENT);
    m_interface = (CInterface*)m_iMan->SearchInstance(CLASS_INTERFACE);
    m_main      = (CRobotMain*)m_iMan->SearchInstance(CLASS_MAIN);
    m_camera    = (CCamera*)m_iMan->SearchInstance(CLASS_CAMERA);
    m_particule = (CParticule*)m_iMan->SearchInstance(CLASS_PARTICULE);
    m_light     = (CLight*)m_iMan->SearchInstance(CLASS_LIGHT);

    m_bInfoMaximized = TRUE;
    m_bInfoMinimized = FALSE;

    m_infoFinalPos = m_infoActualPos = m_infoNormalPos = FPOINT(0.00f, 0.00f);
    m_infoFinalDim = m_infoActualPos = m_infoNormalDim = FPOINT(1.00f, 1.00f);

    m_lightSuppl = -1;
    m_toto = 0;
}

// Object's destructor.

CDisplayInfo::~CDisplayInfo()
{
    m_iMan->DeleteInstance(CLASS_STUDIO, this);
}


// Management of an event.

BOOL CDisplayInfo::EventProcess(const Event &event)
{
    CWindow*        pw;
    CEdit*          edit;
    CSlider*        slider;
    CMotionToto*    toto;

    if ( event.event == EVENT_FRAME )
    {
        EventFrame(event);
        HyperUpdate();
    }

    if ( event.event == EVENT_MOUSEMOVE )
    {
        if ( m_toto != 0 )
        {
            toto = (CMotionToto*)m_toto->RetMotion();
            if ( toto != 0 )
            {
                toto->SetMousePos(event.pos);
            }
        }
    }

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW4);
    if ( pw != 0 )
    {
        if ( event.event == pw->RetEventMsgClose() )
        {
            Event newEvent = event;
            newEvent.event = EVENT_OBJECT_INFOOK;
            m_event->AddEvent(newEvent);
        }

        if ( event.event == EVENT_SATCOM_HUSTON )
        {
            ChangeIndexButton(SATCOM_HUSTON);
        }
        if ( event.event == EVENT_SATCOM_SAT )
        {
            ChangeIndexButton(SATCOM_SAT);
        }
//?     if ( event.event == EVENT_SATCOM_OBJECT )
//?     {
//?         ChangeIndexButton(SATCOM_OBJECT);
//?     }
        if ( event.event == EVENT_SATCOM_LOADING )
        {
            ChangeIndexButton(SATCOM_LOADING);
        }
        if ( event.event == EVENT_SATCOM_PROG )
        {
            ChangeIndexButton(SATCOM_PROG);
        }
        if ( event.event == EVENT_SATCOM_SOLUCE )
        {
            ChangeIndexButton(SATCOM_SOLUCE);
        }

        if ( event.event == EVENT_HYPER_HOME ||
             event.event == EVENT_HYPER_PREV ||
             event.event == EVENT_HYPER_NEXT )
        {
            edit = (CEdit*)pw->SearchControl(EVENT_EDIT1);
            if ( edit != 0 )
            {
                edit->HyperGo(event.event);
                HyperUpdate();
            }
        }

        if ( event.event == EVENT_HYPER_SIZE1 )  // size 1?
        {
            m_main->SetFontSize(9.0f);
            slider = (CSlider*)pw->SearchControl(EVENT_STUDIO_SIZE);
            if ( slider != 0 )  slider->SetVisibleValue((m_main->RetFontSize()-9.0f)/6.0f);
            ViewDisplayInfo();
        }
        if ( event.event == EVENT_HYPER_SIZE2 )  // size 2?
        {
            m_main->SetFontSize(10.0f);
            slider = (CSlider*)pw->SearchControl(EVENT_STUDIO_SIZE);
            if ( slider != 0 )  slider->SetVisibleValue((m_main->RetFontSize()-9.0f)/6.0f);
            ViewDisplayInfo();
        }
        if ( event.event == EVENT_HYPER_SIZE3 )  // size 3?
        {
            m_main->SetFontSize(12.0f);
            slider = (CSlider*)pw->SearchControl(EVENT_STUDIO_SIZE);
            if ( slider != 0 )  slider->SetVisibleValue((m_main->RetFontSize()-9.0f)/6.0f);
            ViewDisplayInfo();
        }
        if ( event.event == EVENT_HYPER_SIZE4 )  // size 4?
        {
            m_main->SetFontSize(15.0f);
            slider = (CSlider*)pw->SearchControl(EVENT_STUDIO_SIZE);
            if ( slider != 0 )  slider->SetVisibleValue((m_main->RetFontSize()-9.0f)/6.0f);
            ViewDisplayInfo();
        }

        if ( event.event == EVENT_STUDIO_SIZE )  // size?
        {
            slider = (CSlider*)pw->SearchControl(EVENT_STUDIO_SIZE);
            if ( slider == 0 )  return FALSE;
            m_main->SetFontSize(9.0f+slider->RetVisibleValue()*6.0f);
            ViewDisplayInfo();
        }

        if ( event.event == EVENT_HYPER_COPY )  // copy ?
        {
            edit = (CEdit*)pw->SearchControl(EVENT_EDIT1);
            if ( edit != 0 )
            {
                edit->Copy();
            }
        }

        if ( event.event == EVENT_LBUTTONDOWN ||
             event.event == EVENT_LBUTTONUP   )
        {
            UpdateCopyButton();
        }

        if ( event.event == EVENT_WINDOW4 )  // window moved?
        {
            m_infoNormalPos = m_infoActualPos = m_infoFinalPos = pw->RetPos();
            m_infoNormalDim = m_infoActualDim = m_infoFinalDim = pw->RetDim();
            AdjustDisplayInfo(m_infoActualPos, m_infoActualDim);
        }
        if ( event.event == pw->RetEventMsgReduce() )
        {
            if ( m_bInfoMinimized )
            {
                m_infoFinalPos = m_infoNormalPos;
                m_infoFinalDim = m_infoNormalDim;
                m_bInfoMinimized = FALSE;
                m_bInfoMaximized = FALSE;
            }
            else
            {
                m_infoFinalPos.x =  0.00f;
                m_infoFinalPos.y = -0.34f;
                m_infoFinalDim.x =  1.00f;
                m_infoFinalDim.y =  0.40f;
                m_bInfoMinimized = TRUE;
                m_bInfoMaximized = FALSE;
            }
//?         m_main->SetEditFull(m_bInfoMaximized);
            pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW4);
            if ( pw != 0 )
            {
                pw->SetMaximized(m_bInfoMaximized);
                pw->SetMinimized(m_bInfoMinimized);
            }
        }
        if ( event.event == pw->RetEventMsgFull() )
        {
            if ( m_bInfoMaximized )
            {
                m_infoFinalPos = m_infoNormalPos;
                m_infoFinalDim = m_infoNormalDim;
                m_bInfoMinimized = FALSE;
                m_bInfoMaximized = FALSE;
            }
            else
            {
                m_infoFinalPos.x = 0.00f;
                m_infoFinalPos.y = 0.00f;
                m_infoFinalDim.x = 1.00f;
                m_infoFinalDim.y = 1.00f;
                m_bInfoMinimized = FALSE;
                m_bInfoMaximized = TRUE;
            }
//?         m_main->SetEditFull(m_bInfoMaximized);
            pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW4);
            if ( pw != 0 )
            {
                pw->SetMaximized(m_bInfoMaximized);
                pw->SetMinimized(m_bInfoMinimized);
            }
        }
    }
    return TRUE;
}


// The brain is changing by time.

BOOL CDisplayInfo::EventFrame(const Event &event)
{
    float       time;

    if ( m_infoFinalPos.x != m_infoActualPos.x ||
         m_infoFinalPos.y != m_infoActualPos.y ||
         m_infoFinalDim.x != m_infoActualDim.x ||
         m_infoFinalDim.y != m_infoActualDim.y )
    {
        time = event.rTime*20.0f;
        m_infoActualPos.x += (m_infoFinalPos.x-m_infoActualPos.x)*time;
        m_infoActualPos.y += (m_infoFinalPos.y-m_infoActualPos.y)*time;
        m_infoActualDim.x += (m_infoFinalDim.x-m_infoActualDim.x)*time;
        m_infoActualDim.y += (m_infoFinalDim.y-m_infoActualDim.y)*time;
        AdjustDisplayInfo(m_infoActualPos, m_infoActualDim);
    }

    return TRUE;
}


// Updates the buttons for hyperlinks.

void CDisplayInfo::HyperUpdate()
{
    CWindow*    pw;
    CEdit*      edit;
    CButton*    button;
    BOOL        bEnable;

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW4);
    if ( pw == 0 )  return;
    edit = (CEdit*)pw->SearchControl(EVENT_EDIT1);
    if ( edit == 0 )  return;

    button = (CButton*)pw->SearchControl(EVENT_HYPER_HOME);
    if ( button != 0 )
    {
        bEnable = edit->HyperTest(EVENT_HYPER_HOME);
        button->SetState(STATE_ENABLE, bEnable);
    }

    button = (CButton*)pw->SearchControl(EVENT_HYPER_PREV);
    if ( button != 0 )
    {
        bEnable = edit->HyperTest(EVENT_HYPER_PREV);
        button->SetState(STATE_ENABLE, bEnable);
    }

    button = (CButton*)pw->SearchControl(EVENT_HYPER_NEXT);
    if ( button != 0 )
    {
        bEnable = edit->HyperTest(EVENT_HYPER_NEXT);
        button->SetState(STATE_ENABLE, bEnable);
    }
}


// Beginning of the display of information.

void CDisplayInfo::StartDisplayInfo(char *filename, int index, BOOL bSoluce)
{
    D3DLIGHT7       light;
    FPOINT          pos, dim;
    CWindow*        pw;
    CEdit*          edit;
    CButton*        button;
    CSlider*        slider;
    CMotionToto*    toto;

    m_index = index;
    m_bSoluce = bSoluce;

//? CreateObjectsFile();

    m_bEditLock = m_main->RetEditLock();
    if ( m_bEditLock )  // edition running program?
    {
        pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW3);
        if ( pw != 0 )
        {
            pw->ClearState(STATE_ENABLE);  // CStudio inactive
        }
    }

    m_main->SetEditLock(TRUE, FALSE);
    m_main->SetEditFull(FALSE);
    m_bInitPause = m_engine->RetPause();
    m_engine->SetPause(TRUE);
    m_infoCamera = m_camera->RetType();
    m_camera->SetType(CAMERA_INFO);

    pos = m_infoActualPos = m_infoFinalPos;
    dim = m_infoActualDim = m_infoFinalDim;
    pw = m_interface->CreateWindows(pos, dim, 4, EVENT_WINDOW4);
    if ( pw == 0 )  return;
//? pw->SetClosable(TRUE);
//? GetResource(RES_TEXT, RT_DISINFO_TITLE, res);
//? pw->SetName(res);
//? pw->SetMinDim(FPOINT(0.56f, 0.40f));
//? pw->SetMaximized(m_bInfoMaximized);
//? pw->SetMinimized(m_bInfoMinimized);
//? m_main->SetEditFull(m_bInfoMaximized);

    edit = pw->CreateEdit(pos, dim, 0, EVENT_EDIT1);
    if ( edit == 0 )  return;
    edit->SetState(STATE_SHADOW);
    edit->SetMultiFont(TRUE);
    edit->SetMaxChar(10000);
    edit->SetFontType(FONT_COLOBOT);
    edit->SetSoluceMode(bSoluce);
    edit->ReadText(filename);
    edit->HyperHome(filename);
    edit->SetEditCap(FALSE);  // just to see!
    edit->SetHiliteCap(FALSE);
    edit->SetFocus(TRUE);

    ViewDisplayInfo();

    button = pw->CreateButton(pos, dim, 128+57, EVENT_SATCOM_HUSTON);
    button->SetState(STATE_SHADOW);
#if _TEEN
#if !_ENGLISH
    button = pw->CreateButton(pos, dim, 46, EVENT_SATCOM_SAT);
#endif
#else
    button = pw->CreateButton(pos, dim, 128+58, EVENT_SATCOM_SAT);
#endif
    button->SetState(STATE_SHADOW);
//? button = pw->CreateButton(pos, dim, 128+59, EVENT_SATCOM_OBJECT);
//? button->SetState(STATE_SHADOW);
    button = pw->CreateButton(pos, dim, 53, EVENT_SATCOM_LOADING);
    button->SetState(STATE_SHADOW);
    button = pw->CreateButton(pos, dim, 128+60, EVENT_SATCOM_PROG);
    button->SetState(STATE_SHADOW);
    button = pw->CreateButton(pos, dim, 20, EVENT_SATCOM_SOLUCE);
    button->SetState(STATE_SHADOW);

    pw->CreateGroup(pos, dim, 18, EVENT_LABEL1);  // arrow >
    pw->CreateGroup(pos, dim, 19, EVENT_LABEL2);  // symbol SatCom

    button = pw->CreateButton(pos, dim, 55, EVENT_HYPER_PREV);
    button->SetState(STATE_SHADOW);
    button = pw->CreateButton(pos, dim, 48, EVENT_HYPER_NEXT);
    button->SetState(STATE_SHADOW);
    button = pw->CreateButton(pos, dim, 30, EVENT_HYPER_HOME);
    button->SetState(STATE_SHADOW);
    button = pw->CreateButton(pos, dim, 82, EVENT_HYPER_SIZE1);
    button->SetState(STATE_SHADOW);
    button = pw->CreateButton(pos, dim, 83, EVENT_HYPER_SIZE2);
    button->SetState(STATE_SHADOW);
    button = pw->CreateButton(pos, dim, 90, EVENT_HYPER_SIZE3);
    button->SetState(STATE_SHADOW);
    button = pw->CreateButton(pos, dim, 91, EVENT_HYPER_SIZE4);
    button->SetState(STATE_SHADOW);
    slider = pw->CreateSlider(pos, dim, 0, EVENT_STUDIO_SIZE);
    slider->SetState(STATE_SHADOW);
    slider->SetVisibleValue((m_main->RetFontSize()-9.0f)/6.0f);
    button = pw->CreateButton(pos, dim, 61, EVENT_HYPER_COPY);
    button->SetState(STATE_SHADOW);
    HyperUpdate();

    button = pw->CreateButton(pos, dim, -1, EVENT_OBJECT_INFOOK);
    button->SetState(STATE_SHADOW);
    button->SetState(STATE_SIMPLY);
    button->SetState(STATE_DEFAULT);
    pw->CreateGroup(pos, dim, 21, EVENT_LABEL3);  // symbol stand-by

    AdjustDisplayInfo(m_infoActualPos, m_infoActualDim);
    UpdateIndexButton();

    m_engine->SetDrawWorld(FALSE);  // doesn't draw anything in the interface
    m_engine->SetDrawFront(TRUE);  // toto draws on the interface
    m_particule->SetFrameUpdate(SH_WORLD, FALSE);  // particles break into world

    m_toto = SearchToto();
    if ( m_toto != 0 )
    {
        m_toto->SetDrawFront(TRUE);

        toto = (CMotionToto*)m_toto->RetMotion();
        if ( toto != 0 )
        {
            toto->StartDisplayInfo();
        }
    }

    ZeroMemory(&light, sizeof(light));
    light.dltType      = D3DLIGHT_DIRECTIONAL;
    light.dcvDiffuse.r = 1.0f;
    light.dcvDiffuse.g = 1.0f;
    light.dcvDiffuse.b = 1.0f;
    light.dvDirection  = D3DVECTOR(1.0f, 0.0f, 1.0f);
    m_lightSuppl = m_light->CreateLight();
    m_light->SetLight(m_lightSuppl, light);
    m_light->SetLightExcluType(m_lightSuppl, TYPETERRAIN);
}

// Repositions all controls editing.

void CDisplayInfo::AdjustDisplayInfo(FPOINT wpos, FPOINT wdim)
{
    CWindow*    pw;
    CEdit*      edit;
    CButton*    button;
    CSlider*    slider;
    CGroup*     group;
    FPOINT      pos, dim;

    wpos.x =  50.0f/640.0f;
    wpos.y =  30.0f/480.0f;
    wdim.x = 540.0f/640.0f;
    wdim.y = 420.0f/480.0f;

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW4);
    if ( pw != 0 )
    {
        pw->SetPos(wpos);
        pw->SetDim(wdim);
        wdim = pw->RetDim();
    }

    pos.x = (50.0f+10.0f)/640.0f;
    pos.y = (30.0f+10.0f+24.0f+10.0f+324.0f-48.0f)/480.0f;
    dim.x = 48.0f/640.0f;
    dim.y = 48.0f/480.0f;
    button = (CButton*)pw->SearchControl(EVENT_SATCOM_HUSTON);
    if ( button != 0 )
    {
        button->SetPos(pos);
        button->SetDim(dim);
    }
    pos.y -= (48.0f+4.0f)/480.0f;
    button = (CButton*)pw->SearchControl(EVENT_SATCOM_SAT);
    if ( button != 0 )
    {
        button->SetPos(pos);
        button->SetDim(dim);
    }
//? pos.y -= (48.0f+4.0f)/480.0f;
//? button = (CButton*)pw->SearchControl(EVENT_SATCOM_OBJECT);
//? if ( button != 0 )
//? {
//?     button->SetPos(pos);
//?     button->SetDim(dim);
//? }
    pos.y -= (48.0f+4.0f)/480.0f;
    button = (CButton*)pw->SearchControl(EVENT_SATCOM_LOADING);
    if ( button != 0 )
    {
        button->SetPos(pos);
        button->SetDim(dim);
    }
    pos.y -= (48.0f+4.0f)/480.0f;
    button = (CButton*)pw->SearchControl(EVENT_SATCOM_PROG);
    if ( button != 0 )
    {
        button->SetPos(pos);
        button->SetDim(dim);
    }
    pos.y -= (48.0f+4.0f)/480.0f;
    button = (CButton*)pw->SearchControl(EVENT_SATCOM_SOLUCE);
    if ( button != 0 )
    {
        button->SetPos(pos);
        button->SetDim(dim);
    }

    pos.x = (50.0f+10.0f+5.0f)/640.0f;
    pos.y = (30.0f+10.0f+4.0f)/480.0f;
    dim.x = (48.0f-10.0f)/640.0f;
    dim.y = 24.0f/480.0f;
    button = (CButton*)pw->SearchControl(EVENT_OBJECT_INFOOK);
    if ( button != 0 )
    {
        button->SetPos(pos);
        button->SetDim(dim);
    }

    pos.x = (50.0f+10.0f+48.0f+10.0f)/640.0f;
    pos.y = (30.0f+10.0f)/480.0f;
    dim.x = 462.0f/640.0f;
    dim.y = 358.0f/480.0f;
    edit = (CEdit*)pw->SearchControl(EVENT_EDIT1);
    if ( edit != 0 )
    {
        edit->SetPos(pos);
        edit->SetDim(dim);
    }

    pos.x = (50.0f+10.0f+48.0f+10.0f)/640.0f;
    pos.y = (30.0f+10.0f+358.0f+10.0f)/480.0f;
    dim.x = 32.0f/640.0f;
    dim.y = 32.0f/480.0f;
    button = (CButton*)pw->SearchControl(EVENT_HYPER_PREV);
    if ( button != 0 )
    {
        button->SetPos(pos);
        button->SetDim(dim);
    }
    pos.x += 35.0f/640.0f;
    button = (CButton*)pw->SearchControl(EVENT_HYPER_NEXT);
    if ( button != 0 )
    {
        button->SetPos(pos);
        button->SetDim(dim);
    }
    pos.x += 35.0f/640.0f;
    button = (CButton*)pw->SearchControl(EVENT_HYPER_HOME);
    if ( button != 0 )
    {
        button->SetPos(pos);
        button->SetDim(dim);
    }

    pos.x += 50.0f/640.0f;
    button = (CButton*)pw->SearchControl(EVENT_HYPER_SIZE1);
    if ( button != 0 )
    {
        button->SetPos(pos);
        button->SetDim(dim);
    }
    pos.x += 35.0f/640.0f;
    button = (CButton*)pw->SearchControl(EVENT_HYPER_SIZE2);
    if ( button != 0 )
    {
        button->SetPos(pos);
        button->SetDim(dim);
    }
    pos.x += 35.0f/640.0f;
    button = (CButton*)pw->SearchControl(EVENT_HYPER_SIZE3);
    if ( button != 0 )
    {
        button->SetPos(pos);
        button->SetDim(dim);
    }
    pos.x += 35.0f/640.0f;
    button = (CButton*)pw->SearchControl(EVENT_HYPER_SIZE4);
    if ( button != 0 )
    {
        button->SetPos(pos);
        button->SetDim(dim);
    }
    pos.x += 35.0f/640.0f;
    dim.x = 18.0f/640.0f;
    slider = (CSlider*)pw->SearchControl(EVENT_STUDIO_SIZE);
    if ( slider != 0 )
    {
        slider->SetPos(pos);
        slider->SetDim(dim);
    }
    pos.x += 50.0f/640.0f;
    dim.x = 32.0f/640.0f;
    button = (CButton*)pw->SearchControl(EVENT_HYPER_COPY);
    if ( button != 0 )
    {
        button->SetPos(pos);
        button->SetDim(dim);
    }

    pos.x = (50.0f+10.0f)/640.0f;
    pos.y = (30.0f+10.0f+24.0f+10.0f+324.0f+6.0f)/480.0f;
    dim.x = 48.0f/640.0f;
    dim.y = 40.0f/480.0f;
    group = (CGroup*)pw->SearchControl(EVENT_LABEL2);  // symbol SatCom
    if ( group != 0 )
    {
        group->SetPos(pos);
        group->SetDim(dim);
    }

    pos.x = (50.0f+10.0f+14.0f)/640.0f;
    pos.y = (30.0f+10.0f+6.0f)/480.0f;
    dim.x = 20.0f/640.0f;
    dim.y = 20.0f/480.0f;
    group = (CGroup*)pw->SearchControl(EVENT_LABEL3);  // symbol stand-by
    if ( group != 0 )
    {
        group->SetPos(pos);
        group->SetDim(dim);
    }
}

// Change the index button.

void CDisplayInfo::ChangeIndexButton(int index)
{
    CWindow*    pw;
    CEdit*      edit;
    char*       filename;

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW4);
    if ( pw == 0 )  return;

    if ( m_index != -1 )
    {
        m_main->SetDisplayInfoPosition(m_index, RetPosition());
    }
    m_index = index;

    edit = (CEdit*)pw->SearchControl(EVENT_EDIT1);
    if ( edit != 0 )
    {
        filename = m_main->RetDisplayInfoName(m_index);
        edit->ReadText(filename);
        edit->HyperHome(filename);
        SetPosition(m_main->RetDisplayInfoPosition(m_index));
    }

    UpdateIndexButton();
}

// Adapts the index buttons.

void CDisplayInfo::UpdateIndexButton()
{
    CWindow*    pw;
    CButton*    button;
    CGroup*     group;
    CEdit*      edit;
    FPOINT      pos, dim;
    char*       filename;
    char*       loading;

    static int table[SATCOM_MAX] =
    {
        0,  // SATCOM_HUSTON
        1,  // SATCOM_SAT
        -1, // SATCOM_OBJECT
        2,  // SATCOM_LOADING
        3,  // SATCOM_PROG
        4,  // SATCOM_SOLUCE
    };

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW4);
    if ( pw == 0 )  return;

    button = (CButton*)pw->SearchControl(EVENT_SATCOM_HUSTON);
    if ( button != 0 )
    {
        button->SetState(STATE_CHECK, m_index==SATCOM_HUSTON);
        filename = m_main->RetDisplayInfoName(SATCOM_HUSTON);
        button->SetState(STATE_VISIBLE, filename[0]!=0);
    }

    button = (CButton*)pw->SearchControl(EVENT_SATCOM_SAT);
    if ( button != 0 )
    {
        button->SetState(STATE_CHECK, m_index==SATCOM_SAT);
        filename = m_main->RetDisplayInfoName(SATCOM_SAT);
        button->SetState(STATE_VISIBLE, filename[0]!=0);
    }

//? button = (CButton*)pw->SearchControl(EVENT_SATCOM_OBJECT);
//? if ( button != 0 )
//? {
//?     button->SetState(STATE_CHECK, m_index==SATCOM_OBJECT);
//?     filename = m_main->RetDisplayInfoName(SATCOM_OBJECT);
//?     button->SetState(STATE_VISIBLE, filename[0]!=0);
//? }

    loading = 0;
    button = (CButton*)pw->SearchControl(EVENT_SATCOM_LOADING);
    if ( button != 0 )
    {
        button->SetState(STATE_CHECK, m_index==SATCOM_LOADING);
        loading = m_main->RetDisplayInfoName(SATCOM_LOADING);
        button->SetState(STATE_VISIBLE, loading[0]!=0);
    }

    button = (CButton*)pw->SearchControl(EVENT_SATCOM_PROG);
    if ( button != 0 )
    {
        button->SetState(STATE_CHECK, m_index==SATCOM_PROG);
        filename = m_main->RetDisplayInfoName(SATCOM_PROG);
        button->SetState(STATE_VISIBLE, filename[0]!=0 && (m_index==SATCOM_LOADING||m_index==SATCOM_PROG||(loading!=0&&loading[0]==0)));
    }

    button = (CButton*)pw->SearchControl(EVENT_SATCOM_SOLUCE);
    if ( button != 0 )
    {
        button->SetState(STATE_CHECK, m_index==SATCOM_SOLUCE);
        filename = m_main->RetDisplayInfoName(SATCOM_SOLUCE);
        button->SetState(STATE_VISIBLE, filename[0]!=0 && m_bSoluce);
    }

    group = (CGroup*)pw->SearchControl(EVENT_LABEL1);
    if ( group != 0 )
    {
        if ( m_index == -1 )
        {
            group->ClearState(STATE_VISIBLE);
        }
        else
        {
            group->SetState(STATE_VISIBLE);

            pos.x = (50.0f+10.0f+48.0f-3.0f)/640.0f;
            pos.y = (30.0f+10.0f+24.0f+10.0f+324.0f-48.0f-1.0f)/480.0f;
            pos.y -= (48.0f+4.0f)/480.0f*table[m_index];
            dim.x = 15.0f/640.0f;
            dim.y = 48.0f/480.0f;
            group->SetPos(pos);
            group->SetDim(dim);
        }
    }

#if 0
    button = (CButton*)pw->SearchControl(EVENT_HYPER_COPY);
    if ( button != 0 )
    {
        button->SetState(STATE_VISIBLE, m_index==SATCOM_LOADING);
    }
#endif

    edit = (CEdit*)pw->SearchControl(EVENT_EDIT1);
    if ( edit != 0 )
    {
//?     edit->SetHiliteCap(m_index==SATCOM_LOADING);
        edit->SetHiliteCap(TRUE);
    }

    UpdateCopyButton();
}

// Adjusts the copy button.

void CDisplayInfo::UpdateCopyButton()
{
    CWindow*    pw;
    CButton*    button;
    CEdit*      edit;
    int         c1, c2;

//? if ( m_index != SATCOM_LOADING )  return;

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW4);
    if ( pw == 0 )  return;

    button = (CButton*)pw->SearchControl(EVENT_HYPER_COPY);
    if ( button == 0 )  return;

    edit = (CEdit*)pw->SearchControl(EVENT_EDIT1);
    if ( edit == 0 )  return;

    edit->GetCursor(c1, c2);
    button->SetState(STATE_ENABLE, c1!=c2);

}

// End of the display of information.

void CDisplayInfo::StopDisplayInfo()
{
    CWindow*        pw;
    CMotionToto*    toto;

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW4);
    if ( pw == 0 )  return;

    m_interface->DeleteControl(EVENT_WINDOW4);

    if ( m_bEditLock )  // editing running program?
    {
        pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW3);
        if ( pw != 0 )
        {
            pw->SetState(STATE_ENABLE);  // CStudio operating
        }
    }
    else
    {
        if ( !m_bInitPause )  m_engine->SetPause(FALSE);
        m_main->SetEditLock(FALSE, FALSE);
    }
    m_camera->SetType(m_infoCamera);

    m_engine->SetDrawWorld(TRUE);  // draws all on the interface
    m_engine->SetDrawFront(FALSE);  // draws nothing on the interface
    m_particule->SetFrameUpdate(SH_WORLD, TRUE);
    m_particule->FlushParticule(SH_FRONT);
    m_particule->FlushParticule(SH_INTERFACE);

    if ( m_toto != 0 )
    {
        toto = (CMotionToto*)m_toto->RetMotion();
        if ( toto != 0 )
        {
            toto->StopDisplayInfo();
        }
    }

    m_light->DeleteLight(m_lightSuppl);
    m_lightSuppl = -1;
}


// Specifies the position.

void CDisplayInfo::SetPosition(int pos)
{
    CWindow*        pw;
    CEdit*          edit;

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW4);
    if ( pw == 0 )  return;

    edit = (CEdit*)pw->SearchControl(EVENT_EDIT1);
    if ( edit == 0 )  return;

    edit->SetFirstLine(pos);
}

// Returns the position.

int CDisplayInfo::RetPosition()
{
    CWindow*        pw;
    CEdit*          edit;

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW4);
    if ( pw == 0 )  return 0;

    edit = (CEdit*)pw->SearchControl(EVENT_EDIT1);
    if ( edit == 0 )  return 0;

    return edit->RetFirstLine();
}



// Changing the size of the display of information.

void CDisplayInfo::ViewDisplayInfo()
{
    CWindow*    pw;
    CEdit*      edit;
    POINT       dim;

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW4);
    if ( pw == 0 )  return;

    edit = (CEdit*)pw->SearchControl(EVENT_EDIT1);
    if ( edit == 0 )  return;

    dim = m_engine->RetDim();
    edit->SetFontSize(m_main->RetFontSize()/(dim.x/640.0f));
}

// Returns the object human.

CObject* CDisplayInfo::SearchToto()
{
    ObjectType  type;
    CObject*    pObj;
    int         i;

    for ( i=0 ; i<1000000 ; i++ )
    {
        pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
        if ( pObj == 0 )  break;

        type = pObj->RetType();
        if ( type == OBJECT_TOTO )
        {
            return pObj;
        }
    }
    return 0;
}


// Creating the list of objects.

typedef struct
{
    int         total;
    ObjectType  type;
}
ObjectList;

void ObjectAdd(ObjectList list[], ObjectType type)
{
    int     i;

    for ( i=0 ; i<200 ; i++ )
    {
        if ( list[i].total == 0 )
        {
            list[i].total ++;
            list[i].type = type;
            list[i+1].total = 0;
            return;
        }
        if ( list[i].type == type )
        {
            list[i].total ++;
            return;
        }
    }
}

void ObjectWrite(FILE* file, ObjectList list[], int i)
{
    char        line[100];
    char        res[100];
    char*       p;

    if ( list[i].total < 10 )
    {
        sprintf(line, "\\c; %dx \\n;\\l;", list[i].total);
    }
    else
    {
        sprintf(line, "\\c;%dx \\n;\\l;", list[i].total);
    }

    GetResource(RES_OBJECT, list[i].type, res);
    if ( res[0] == 0 )  return;
    strcat(line, res);

    strcat(line, "\\u ");
    p = RetHelpFilename(list[i].type);
    if ( p[0] == 0 )  return;
    strcat(line, p+5);  // skip "help\"
    p = strstr(line, ".txt");
    if ( p != 0 )  *p = 0;
    strcat(line, ";\n");
    fputs(line, file);
}

// Creates the file containing the list of objects.

void CDisplayInfo::CreateObjectsFile()
{
    FILE*       file;
    CObject*    pObj;
    ObjectType  type;
    ObjectList  list[200];
    char        line[100];
    int         i;
    BOOL        bRadar, bAtLeast;

    file = fopen("help\\objects.txt", "w");
    if ( file == 0 )  return;

    list[0].total = 0;  // empty list
    bRadar = FALSE;
    for ( i=0 ; i<1000000 ; i++ )
    {
        pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
        if ( pObj == 0 )  break;

        if ( !pObj->RetActif() )  continue;
        if ( !pObj->RetSelectable() )  continue;
        if ( pObj->RetProxyActivate() )  continue;

        type = pObj->RetType();
        if ( type == OBJECT_NULL )  continue;
        if ( type == OBJECT_FIX )  continue;

        ObjectAdd(list, type);

        if ( type == OBJECT_RADAR )  bRadar = TRUE;
    }

    if ( bRadar )
    {
        GetResource(RES_TEXT, RT_SATCOM_LIST, line);
        fputs(line, file);
        bAtLeast = FALSE;
        for ( i=0 ; i<200 ; i++ )
        {
            if ( list[i].total == 0 )  break;  // end of the list?

            if ( list[i].type == OBJECT_BASE  ||
                 list[i].type == OBJECT_HUMAN )
            {
                ObjectWrite(file, list, i);
                bAtLeast = TRUE;
            }
        }
        if ( !bAtLeast )
        {
            GetResource(RES_TEXT, RT_SATCOM_NULL, line);
            fputs(line, file);
        }

        strcpy(line, "\n");
        fputs(line, file);
        GetResource(RES_TEXT, RT_SATCOM_BOT, line);
        fputs(line, file);
        bAtLeast = FALSE;
        for ( i=0 ; i<200 ; i++ )
        {
            if ( list[i].total == 0 )  break;  // end of the list?

            if ( list[i].type == OBJECT_MOBILEwt ||
                 list[i].type == OBJECT_MOBILEtt ||
                 list[i].type == OBJECT_MOBILEft ||
                 list[i].type == OBJECT_MOBILEit ||
                 list[i].type == OBJECT_MOBILEwa ||
                 list[i].type == OBJECT_MOBILEta ||
                 list[i].type == OBJECT_MOBILEfa ||
                 list[i].type == OBJECT_MOBILEia ||
                 list[i].type == OBJECT_MOBILEwc ||
                 list[i].type == OBJECT_MOBILEtc ||
                 list[i].type == OBJECT_MOBILEfc ||
                 list[i].type == OBJECT_MOBILEic ||
                 list[i].type == OBJECT_MOBILEwi ||
                 list[i].type == OBJECT_MOBILEti ||
                 list[i].type == OBJECT_MOBILEfi ||
                 list[i].type == OBJECT_MOBILEii ||
                 list[i].type == OBJECT_MOBILEws ||
                 list[i].type == OBJECT_MOBILEts ||
                 list[i].type == OBJECT_MOBILEfs ||
                 list[i].type == OBJECT_MOBILEis ||
                 list[i].type == OBJECT_MOBILErt ||
                 list[i].type == OBJECT_MOBILErc ||
                 list[i].type == OBJECT_MOBILErr ||
                 list[i].type == OBJECT_MOBILErs ||
                 list[i].type == OBJECT_MOBILEsa ||
                 list[i].type == OBJECT_MOBILEtg ||
                 list[i].type == OBJECT_MOBILEdr )
            {
                ObjectWrite(file, list, i);
                bAtLeast = TRUE;
            }
        }
        if ( !bAtLeast )
        {
            GetResource(RES_TEXT, RT_SATCOM_NULL, line);
            fputs(line, file);
        }

        strcpy(line, "\n");
        fputs(line, file);
        GetResource(RES_TEXT, RT_SATCOM_BUILDING, line);
        fputs(line, file);
        bAtLeast = FALSE;
        for ( i=0 ; i<200 ; i++ )
        {
            if ( list[i].total == 0 )  break;  // end of the list?

            if ( list[i].type == OBJECT_DERRICK  ||
                 list[i].type == OBJECT_FACTORY  ||
                 list[i].type == OBJECT_STATION  ||
                 list[i].type == OBJECT_CONVERT  ||
                 list[i].type == OBJECT_REPAIR   ||
                 list[i].type == OBJECT_DESTROYER||
                 list[i].type == OBJECT_TOWER    ||
                 list[i].type == OBJECT_NEST     ||
                 list[i].type == OBJECT_RESEARCH ||
                 list[i].type == OBJECT_RADAR    ||
                 list[i].type == OBJECT_ENERGY   ||
                 list[i].type == OBJECT_LABO     ||
                 list[i].type == OBJECT_NUCLEAR  ||
                 list[i].type == OBJECT_START    ||
                 list[i].type == OBJECT_END      ||
                 list[i].type == OBJECT_INFO     ||
                 list[i].type == OBJECT_PARA     ||
                 list[i].type == OBJECT_TARGET1  ||
                 list[i].type == OBJECT_TARGET2  ||
                 list[i].type == OBJECT_SAFE     ||
                 list[i].type == OBJECT_HUSTON   )
            {
                ObjectWrite(file, list, i);
                bAtLeast = TRUE;
            }
        }
        if ( !bAtLeast )
        {
            GetResource(RES_TEXT, RT_SATCOM_NULL, line);
            fputs(line, file);
        }

        strcpy(line, "\n");
        fputs(line, file);
        GetResource(RES_TEXT, RT_SATCOM_FRET, line);
        fputs(line, file);
        bAtLeast = FALSE;
        for ( i=0 ; i<200 ; i++ )
        {
            if ( list[i].total == 0 )  break;  // end of the list?

            if ( list[i].type == OBJECT_STONE   ||
                 list[i].type == OBJECT_URANIUM ||
                 list[i].type == OBJECT_METAL   ||
                 list[i].type == OBJECT_POWER   ||
                 list[i].type == OBJECT_ATOMIC  ||
                 list[i].type == OBJECT_BULLET  ||
                 list[i].type == OBJECT_BBOX    ||
                 list[i].type == OBJECT_TNT     )
            {
                ObjectWrite(file, list, i);
                bAtLeast = TRUE;
            }
        }
        if ( !bAtLeast )
        {
            GetResource(RES_TEXT, RT_SATCOM_NULL, line);
            fputs(line, file);
        }

        strcpy(line, "\n");
        fputs(line, file);
        GetResource(RES_TEXT, RT_SATCOM_ALIEN, line);
        fputs(line, file);
        bAtLeast = FALSE;
        for ( i=0 ; i<200 ; i++ )
        {
            if ( list[i].total == 0 )  break;  // end of the list?

            if ( list[i].type == OBJECT_MOTHER ||
                 list[i].type == OBJECT_ANT    ||
                 list[i].type == OBJECT_BEE    ||
                 list[i].type == OBJECT_WORM   ||
                 list[i].type == OBJECT_SPIDER )
            {
                ObjectWrite(file, list, i);
                bAtLeast = TRUE;
            }
        }
        if ( !bAtLeast )
        {
            GetResource(RES_TEXT, RT_SATCOM_NULL, line);
            fputs(line, file);
        }
    }
    else
    {
        GetResource(RES_TEXT, RT_SATCOM_ERROR1, line);
        fputs(line, file);
        GetResource(RES_TEXT, RT_SATCOM_ERROR2, line);
        fputs(line, file);
    }

    strcpy(line, "\n");
    fputs(line, file);

    fclose(file);
}


