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


#include "ui/displayinfo.h"

#include "app/app.h"
#include "app/pausemanager.h"

#include "common/restext.h"
#include "common/settings.h"
#include "common/stringutils.h"

#include "graphics/core/light.h"

#include "graphics/engine/engine.h"
#include "graphics/engine/lightman.h"
#include "graphics/engine/particle.h"

#include "level/robotmain.h"

#include "object/object.h"
#include "object/object_manager.h"

#include "object/interface/movable_object.h"

#include "object/motion/motion.h"
#include "object/motion/motiontoto.h"

#include "script/cbottoken.h"

#include "ui/controls/button.h"
#include "ui/controls/edit.h"
#include "ui/controls/group.h"
#include "ui/controls/interface.h"
#include "ui/controls/slider.h"
#include "ui/controls/window.h"

#include <string.h>


namespace Ui
{
// Object's constructor.

CDisplayInfo::CDisplayInfo()
{
    m_event     = CApplication::GetInstancePointer()->GetEventQueue();
    m_engine    = Gfx::CEngine::GetInstancePointer();
    m_particle  = m_engine->GetParticle();
    m_light     = m_engine->GetLightManager();
    m_main      = CRobotMain::GetInstancePointer();
    m_interface = m_main->GetInterface();
    m_camera    = m_main->GetCamera();
    m_pause     = m_main->GetPauseManager();

    m_bInfoMaximized = true;
    m_bInfoMinimized = false;

    m_infoFinalPos = m_infoActualPos = m_infoNormalPos = { 0.00f, 0.00f };
    m_infoFinalDim = m_infoActualPos = m_infoNormalDim = { 1.00f, 1.00f };

    m_lightSuppl = -1;
    m_toto = nullptr;
    m_bSoluce = false;
    m_bEditLock = false;
    m_index = -1;
}

// Object's destructor.

CDisplayInfo::~CDisplayInfo()
{
}


// Management of an event.

bool CDisplayInfo::EventProcess(const Event &event)
{
    Ui::CWindow*        pw;
    Ui::CEdit*          edit;
    Ui::CSlider*        slider;

    if ( event.type == EVENT_FRAME )
    {
        EventFrame(event);
        HyperUpdate();
    }

    if ( event.type == EVENT_MOUSE_MOVE )
    {
        if ( m_toto != nullptr )
        {
            assert(m_toto->Implements(ObjectInterfaceType::Movable));
            CMotionToto* toto = static_cast<CMotionToto*>(dynamic_cast<CMovableObject&>(*m_toto).GetMotion());
            assert(toto != nullptr);
            toto->SetMousePos(event.mousePos);
        }
    }

    pw = static_cast<Ui::CWindow*>(m_interface->SearchControl(EVENT_WINDOW4));
    if ( pw != nullptr )
    {
        if ( event.type == pw->GetEventTypeClose() )
        {
            m_event->AddEvent(Event(EVENT_OBJECT_INFOOK));
        }

        if ( event.type == EVENT_SATCOM_HUSTON )
        {
            ChangeIndexButton(SATCOM_HUSTON);
        }
        if ( event.type == EVENT_SATCOM_SAT )
        {
            ChangeIndexButton(SATCOM_SAT);
        }
//?     if ( event.event == EVENT_SATCOM_OBJECT )
//?     {
//?         ChangeIndexButton(SATCOM_OBJECT);
//?     }
        if ( event.type == EVENT_SATCOM_LOADING )
        {
            ChangeIndexButton(SATCOM_LOADING);
        }
        if ( event.type == EVENT_SATCOM_PROG )
        {
            ChangeIndexButton(SATCOM_PROG);
        }
        if ( event.type == EVENT_SATCOM_SOLUCE )
        {
            ChangeIndexButton(SATCOM_SOLUCE);
        }

        if ( event.type == EVENT_HYPER_HOME ||
             event.type == EVENT_HYPER_PREV ||
             event.type == EVENT_HYPER_NEXT )
        {
            edit = static_cast<Ui::CEdit*>(pw->SearchControl(EVENT_EDIT1));
            if ( edit != nullptr )
            {
                edit->HyperGo(event.type);
                HyperUpdate();
            }
        }

        if ( event.type == EVENT_HYPER_SIZE1 )  // size 1?
        {
            CSettings::GetInstancePointer()->SetFontSize(9.0f);
            slider = static_cast<Ui::CSlider*>(pw->SearchControl(EVENT_STUDIO_SIZE));
            if ( slider != nullptr )  slider->SetVisibleValue((CSettings::GetInstancePointer()->GetFontSize()-9.0f)/15.0f);
            ViewDisplayInfo();
        }
        if ( event.type == EVENT_HYPER_SIZE2 )  // size 2?
        {
            CSettings::GetInstancePointer()->SetFontSize(14.0f);
            slider = static_cast<Ui::CSlider*>(pw->SearchControl(EVENT_STUDIO_SIZE));
            if ( slider != nullptr )  slider->SetVisibleValue((CSettings::GetInstancePointer()->GetFontSize()-9.0f)/15.0f);
            ViewDisplayInfo();
        }
        if ( event.type == EVENT_HYPER_SIZE3 )  // size 3?
        {
            CSettings::GetInstancePointer()->SetFontSize(19.0f);
            slider = static_cast<Ui::CSlider*>(pw->SearchControl(EVENT_STUDIO_SIZE));
            if ( slider != nullptr )  slider->SetVisibleValue((CSettings::GetInstancePointer()->GetFontSize()-9.0f)/15.0f);
            ViewDisplayInfo();
        }
        if ( event.type == EVENT_HYPER_SIZE4 )  // size 4?
        {
            CSettings::GetInstancePointer()->SetFontSize(24.0f);
            slider = static_cast<Ui::CSlider*>(pw->SearchControl(EVENT_STUDIO_SIZE));
            if ( slider != nullptr )  slider->SetVisibleValue((CSettings::GetInstancePointer()->GetFontSize()-9.0f)/15.0f);
            ViewDisplayInfo();
        }

        if ( event.type == EVENT_STUDIO_SIZE )  // size?
        {
            slider = static_cast<Ui::CSlider*>(pw->SearchControl(EVENT_STUDIO_SIZE));
            if ( slider == nullptr )  return false;
            CSettings::GetInstancePointer()->SetFontSize(9.0f+slider->GetVisibleValue()*15.0f);
            ViewDisplayInfo();
        }

        if ( event.type == EVENT_HYPER_COPY )  // copy ?
        {
            edit = static_cast<Ui::CEdit*>(pw->SearchControl(EVENT_EDIT1));
            if ( edit != nullptr )
            {
                edit->Copy();
            }
        }

        if ((event.type == EVENT_MOUSE_BUTTON_DOWN || event.type == EVENT_MOUSE_BUTTON_UP) &&
             event.GetData<MouseButtonEventData>()->button == MOUSE_BUTTON_LEFT)
        {
            UpdateCopyButton();
        }

        if ( event.type == EVENT_WINDOW4 )  // window moved?
        {
            m_infoNormalPos = m_infoActualPos = m_infoFinalPos = pw->GetPos();
            m_infoNormalDim = m_infoActualDim = m_infoFinalDim = pw->GetDim();
            AdjustDisplayInfo(m_infoActualPos, m_infoActualDim);
        }
        if ( event.type == pw->GetEventTypeReduce() )
        {
            if ( m_bInfoMinimized )
            {
                m_infoFinalPos = m_infoNormalPos;
                m_infoFinalDim = m_infoNormalDim;
                m_bInfoMinimized = false;
                m_bInfoMaximized = false;
            }
            else
            {
                m_infoFinalPos.x =  0.00f;
                m_infoFinalPos.y = -0.34f;
                m_infoFinalDim.x =  1.00f;
                m_infoFinalDim.y =  0.40f;
                m_bInfoMinimized = true;
                m_bInfoMaximized = false;
            }
//?         m_main->SetEditFull(m_bInfoMaximized);
            pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW4));
            if ( pw != nullptr )
            {
                pw->SetMaximized(m_bInfoMaximized);
                pw->SetMinimized(m_bInfoMinimized);
            }
        }
        if ( event.type == pw->GetEventTypeFull() )
        {
            if ( m_bInfoMaximized )
            {
                m_infoFinalPos = m_infoNormalPos;
                m_infoFinalDim = m_infoNormalDim;
                m_bInfoMinimized = false;
                m_bInfoMaximized = false;
            }
            else
            {
                m_infoFinalPos.x = 0.00f;
                m_infoFinalPos.y = 0.00f;
                m_infoFinalDim.x = 1.00f;
                m_infoFinalDim.y = 1.00f;
                m_bInfoMinimized = false;
                m_bInfoMaximized = true;
            }
//?         m_main->SetEditFull(m_bInfoMaximized);
            pw = static_cast<Ui::CWindow*>(m_interface->SearchControl(EVENT_WINDOW4));
            if ( pw != nullptr )
            {
                pw->SetMaximized(m_bInfoMaximized);
                pw->SetMinimized(m_bInfoMinimized);
            }
        }
    }
    return true;
}


// The brain is changing by time.

bool CDisplayInfo::EventFrame(const Event &event)
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

    return true;
}


// Updates the buttons for hyperlinks.

void CDisplayInfo::HyperUpdate()
{
    Ui::CWindow*    pw;
    Ui::CEdit*      edit;
    Ui::CButton*    button;
    bool            bEnable;

    pw = static_cast<Ui::CWindow*>(m_interface->SearchControl(EVENT_WINDOW4));
    if ( pw == nullptr )  return;
    edit = static_cast<Ui::CEdit*>(pw->SearchControl(EVENT_EDIT1));
    if ( edit == nullptr )  return;

    button = static_cast<Ui::CButton*>(pw->SearchControl(EVENT_HYPER_HOME));
    if ( button != nullptr )
    {
        bEnable = edit->HyperTest(EVENT_HYPER_HOME);
        button->SetState(STATE_ENABLE, bEnable);
    }

    button = static_cast<Ui::CButton*>(pw->SearchControl(EVENT_HYPER_PREV));
    if ( button != nullptr )
    {
        bEnable = edit->HyperTest(EVENT_HYPER_PREV);
        button->SetState(STATE_ENABLE, bEnable);
    }

    button = static_cast<Ui::CButton*>(pw->SearchControl(EVENT_HYPER_NEXT));
    if ( button != nullptr )
    {
        bEnable = edit->HyperTest(EVENT_HYPER_NEXT);
        button->SetState(STATE_ENABLE, bEnable);
    }
}


// Beginning of the display of information.

void CDisplayInfo::StartDisplayInfo(const std::filesystem::path& filename, int index, bool bSoluce)
{
    Gfx::Light          light;
    glm::vec2           pos, dim;
    Ui::CWindow*        pw;
    Ui::CEdit*          edit;
    Ui::CButton*        button;
    Ui::CSlider*        slider;

    m_index = index;
    m_bSoluce = bSoluce;

    m_bEditLock = m_main->GetEditLock();
    if ( m_bEditLock )  // edition running program?
    {
        pw = static_cast<Ui::CWindow*>(m_interface->SearchControl(EVENT_WINDOW3));
        if ( pw != nullptr )
        {
            pw->ClearState(STATE_ENABLE);  // CStudio inactive
        }
    }

    m_main->SetEditLock(true, false);
    m_main->SetEditFull(false);
    m_satcomPause = m_pause->ActivatePause(PAUSE_ENGINE|PAUSE_HIDE_SHORTCUTS|PAUSE_MUTE_SOUND|PAUSE_CAMERA, PAUSE_MUSIC_SATCOM);

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW6));
    if (pw != nullptr) pw->ClearState(STATE_VISIBLE | STATE_ENABLE);

    pos = m_infoActualPos = m_infoFinalPos;
    dim = m_infoActualDim = m_infoFinalDim;
    pw = m_interface->CreateWindows(pos, dim, 4, EVENT_WINDOW4);
    if ( pw == nullptr )  return;
//? pw->SetClosable(true);
//? GetResource(RES_TEXT, RT_DISINFO_TITLE, res);
//? pw->SetName(res);
//? pw->SetMinDim(glm::vec2(0.56f, 0.40f));
//? pw->SetMaximized(m_bInfoMaximized);
//? pw->SetMinimized(m_bInfoMinimized);
//? m_main->SetEditFull(m_bInfoMaximized);

    edit = pw->CreateEdit(pos, dim, 0, EVENT_EDIT1);
    if ( edit == nullptr )  return;
    edit->SetState(STATE_SHADOW);
    edit->SetMultiFont(true);
    edit->SetMaxChar(10000);
    edit->SetFontType(Gfx::FONT_SATCOM);
    edit->SetSoluceMode(bSoluce);
    edit->ReadText(filename);
    edit->HyperHome(filename);
    edit->SetEditCap(false);  // just to see!
    edit->SetHighlightCap(false);
    m_interface->SetFocus(edit);

    ViewDisplayInfo();

    button = pw->CreateButton(pos, dim, 128+57, EVENT_SATCOM_HUSTON);
    button->SetState(STATE_SHADOW);
    button = pw->CreateButton(pos, dim, 128+58, EVENT_SATCOM_SAT);
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
    slider->SetVisibleValue((CSettings::GetInstancePointer()->GetFontSize()-9.0f)/15.0f);
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

    m_engine->SetDrawWorld(false);  // doesn't draw anything in the interface
    m_engine->SetDrawFront(true);  // toto draws on the interface
    m_particle->SetFrameUpdate(Gfx::SH_WORLD, false);  // particles break into world

    m_toto = SearchToto();
    if ( m_toto != nullptr )
    {
        m_toto->SetDrawFront(true);

        assert(m_toto->Implements(ObjectInterfaceType::Movable));
        CMotionToto* toto = static_cast<CMotionToto*>(dynamic_cast<CMovableObject&>(*m_toto).GetMotion());
        assert(toto != nullptr);
        toto->StartDisplayInfo();
    }

    light.type    = Gfx::LIGHT_DIRECTIONAL;
    light.ambient = Gfx::Color(0.0f, 0.0f, 0.0f);
    light.diffuse = Gfx::Color(1.0f, 0.1f, 0.1f);
    light.direction  = glm::vec3(1.0f, 0.0f, 1.0f);

    m_lightSuppl = m_light->CreateLight();
    m_light->SetLight(m_lightSuppl, light);
    m_light->SetLightExcludeType(m_lightSuppl, Gfx::ENG_OBJTYPE_TERRAIN);
}

// Repositions all controls editing.

void CDisplayInfo::AdjustDisplayInfo(glm::vec2 wpos, glm::vec2 wdim)
{
    CWindow*    pw;
    CEdit*      edit;
    CButton*    button;
    CSlider*    slider;
    CGroup*     group;
    glm::vec2   pos, dim;

    wpos.x =  50.0f / 640.0f;
    wpos.y =  30.0f / 480.0f;
    wdim.x = 540.0f / 640.0f;
    wdim.y = 420.0f / 480.0f;

    pw = static_cast<Ui::CWindow*>(m_interface->SearchControl(EVENT_WINDOW4));
    if ( pw != nullptr )
    {
        pw->SetPos(wpos);
        pw->SetDim(wdim);
        wdim = pw->GetDim();
    }

    pos.x = (50.0f + 10.0f) / 640.0f;
    pos.y = (30.0f + 10.0f + 24.0f + 10.0f + 324.0f - 48.0f) / 480.0f;
    dim.x = 48.0f / 640.0f;
    dim.y = 48.0f / 480.0f;
    button = static_cast<Ui::CButton*>(pw->SearchControl(EVENT_SATCOM_HUSTON));
    if ( button != nullptr )
    {
        button->SetPos(pos);
        button->SetDim(dim);
    }
    pos.y -= (48.0f + 4.0f) / 480.0f;
    button = static_cast<Ui::CButton*>(pw->SearchControl(EVENT_SATCOM_SAT));
    if ( button != nullptr )
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
    pos.y -= (48.0f + 4.0f) / 480.0f;
    button = static_cast<Ui::CButton*>(pw->SearchControl(EVENT_SATCOM_LOADING));
    if ( button != nullptr )
    {
        button->SetPos(pos);
        button->SetDim(dim);
    }
    pos.y -= (48.0f + 4.0f) / 480.0f;
    button = static_cast<Ui::CButton*>(pw->SearchControl(EVENT_SATCOM_PROG));
    if ( button != nullptr )
    {
        button->SetPos(pos);
        button->SetDim(dim);
    }
    pos.y -= (48.0f + 4.0f) / 480.0f;
    button = static_cast<Ui::CButton*>(pw->SearchControl(EVENT_SATCOM_SOLUCE));
    if ( button != nullptr )
    {
        button->SetPos(pos);
        button->SetDim(dim);
    }

    pos.x = (50.0f + 10.0f + 5.0f) / 640.0f;
    pos.y = (30.0f + 10.0f + 4.0f) / 480.0f;
    dim.x = (48.0f - 10.0f) / 640.0f;
    dim.y = 24.0f / 480.0f;
    button = static_cast<Ui::CButton*>(pw->SearchControl(EVENT_OBJECT_INFOOK));
    if ( button != nullptr )
    {
        button->SetPos(pos);
        button->SetDim(dim);
    }

    pos.x = (50.0f + 10.0f + 48.0f + 10.0f) / 640.0f;
    pos.y = (30.0f + 10.0f) / 480.0f;
    dim.x = 462.0f / 640.0f;
    dim.y = 358.0f / 480.0f;
    edit = static_cast<Ui::CEdit*>(pw->SearchControl(EVENT_EDIT1));
    if ( edit != nullptr )
    {
        edit->SetPos(pos);
        edit->SetDim(dim);
    }

    pos.x = (50.0f + 10.0f +  48.0f + 10.0f) / 640.0f;
    pos.y = (30.0f + 10.0f + 358.0f + 10.0f) / 480.0f;
    dim.x = 32.0f / 640.0f;
    dim.y = 32.0f / 480.0f;
    button = static_cast<Ui::CButton*>(pw->SearchControl(EVENT_HYPER_PREV));
    if ( button != nullptr )
    {
        button->SetPos(pos);
        button->SetDim(dim);
    }
    pos.x += 35.0f / 640.0f;
    button = static_cast<Ui::CButton*>(pw->SearchControl(EVENT_HYPER_NEXT));
    if ( button != nullptr )
    {
        button->SetPos(pos);
        button->SetDim(dim);
    }
    pos.x += 35.0f / 640.0f;
    button = static_cast<Ui::CButton*>(pw->SearchControl(EVENT_HYPER_HOME));
    if ( button != nullptr )
    {
        button->SetPos(pos);
        button->SetDim(dim);
    }

    pos.x += 50.0f / 640.0f;
    button = static_cast<Ui::CButton*>(pw->SearchControl(EVENT_HYPER_SIZE1));
    if ( button != nullptr )
    {
        button->SetPos(pos);
        button->SetDim(dim);
    }
    pos.x += 35.0f / 640.0f;
    button = static_cast<Ui::CButton*>(pw->SearchControl(EVENT_HYPER_SIZE2));
    if ( button != nullptr )
    {
        button->SetPos(pos);
        button->SetDim(dim);
    }
    pos.x += 35.0f / 640.0f;
    button = static_cast<Ui::CButton*>(pw->SearchControl(EVENT_HYPER_SIZE3));
    if ( button != nullptr )
    {
        button->SetPos(pos);
        button->SetDim(dim);
    }
    pos.x += 35.0f / 640.0f;
    button = static_cast<Ui::CButton*>(pw->SearchControl(EVENT_HYPER_SIZE4));
    if ( button != nullptr )
    {
        button->SetPos(pos);
        button->SetDim(dim);
    }
    pos.x += 35.0f / 640.0f;
    dim.x = 18.0f / 640.0f;
    slider = static_cast<Ui::CSlider*>(pw->SearchControl(EVENT_STUDIO_SIZE));
    if ( slider != nullptr )
    {
        slider->SetPos(pos);
        slider->SetDim(dim);
    }
    pos.x += 50.0f / 640.0f;
    dim.x = 32.0f / 640.0f;
    button = static_cast<Ui::CButton*>(pw->SearchControl(EVENT_HYPER_COPY));
    if ( button != nullptr )
    {
        button->SetPos(pos);
        button->SetDim(dim);
    }

    pos.x = (50.0f + 10.0f) / 640.0f;
    pos.y = (30.0f + 10.0f + 24.0f + 10.0f + 324.0f + 6.0f) / 480.0f;
    dim.x = 48.0f / 640.0f;
    dim.y = 40.0f / 480.0f;
    group = static_cast<Ui::CGroup*>(pw->SearchControl(EVENT_LABEL2));  // symbol SatCom
    if ( group != nullptr )
    {
        group->SetPos(pos);
        group->SetDim(dim);
    }

    pos.x = (50.0f + 10.0f + 14.0f) / 640.0f;
    pos.y = (30.0f + 10.0f + 6.0f) / 480.0f;
    dim.x = 20.0f / 640.0f;
    dim.y = 20.0f / 480.0f;
    group = static_cast<Ui::CGroup*>(pw->SearchControl(EVENT_LABEL3));  // symbol stand-by
    if ( group != nullptr )
    {
        group->SetPos(pos);
        group->SetDim(dim);
    }
}

// Change the index button.

void CDisplayInfo::ChangeIndexButton(int index)
{
    Ui::CWindow*    pw;
    Ui::CEdit*      edit;

    pw = static_cast<Ui::CWindow*>(m_interface->SearchControl(EVENT_WINDOW4));
    if ( pw == nullptr )  return;

    m_index = index;

    edit = static_cast<Ui::CEdit*>(pw->SearchControl(EVENT_EDIT1));
    if ( edit != nullptr )
    {
        const std::filesystem::path& filename = m_main->GetDisplayInfoName(m_index);
        edit->ReadText(filename);
        edit->HyperHome(filename);
        SetPosition(0);
    }

    UpdateIndexButton();
}

// Adapts the index buttons.

void CDisplayInfo::UpdateIndexButton()
{
    Ui::CWindow*    pw;
    Ui::CButton*    button;
    Ui::CGroup*     group;
    Ui::CEdit*      edit;
    glm::vec2       pos, dim;

    static int table[SATCOM_MAX] =
    {
        0,  // SATCOM_HUSTON
        1,  // SATCOM_SAT
        -1, // SATCOM_OBJECT
        2,  // SATCOM_LOADING
        3,  // SATCOM_PROG
        4,  // SATCOM_SOLUCE
    };

    pw = static_cast<Ui::CWindow*>(m_interface->SearchControl(EVENT_WINDOW4));
    if ( pw == nullptr )  return;

    button = static_cast<Ui::CButton*>(pw->SearchControl(EVENT_SATCOM_HUSTON));
    if ( button != nullptr )
    {
        button->SetState(STATE_CHECK, m_index==SATCOM_HUSTON);
        const auto& filename = m_main->GetDisplayInfoName(SATCOM_HUSTON);
        button->SetState(STATE_VISIBLE, !filename.empty());
    }

    button = static_cast<Ui::CButton*>(pw->SearchControl(EVENT_SATCOM_SAT));
    if ( button != nullptr )
    {
        button->SetState(STATE_CHECK, m_index==SATCOM_SAT);
        const auto& filename = m_main->GetDisplayInfoName(SATCOM_SAT);
        button->SetState(STATE_VISIBLE, !filename.empty());
    }

//? button = (CButton*)pw->SearchControl(EVENT_SATCOM_OBJECT);
//? if ( button != 0 )
//? {
//?     button->SetState(STATE_CHECK, m_index==SATCOM_OBJECT);
//?     filename = m_main->GetDisplayInfoName(SATCOM_OBJECT);
//?     button->SetState(STATE_VISIBLE, !filename.empty());
//? }

    button = static_cast<Ui::CButton*>(pw->SearchControl(EVENT_SATCOM_LOADING));
    if ( button != nullptr )
    {
        button->SetState(STATE_CHECK, m_index==SATCOM_LOADING);
        const auto& filename = m_main->GetDisplayInfoName(SATCOM_LOADING);
        button->SetState(STATE_VISIBLE, !filename.empty());
    }

    button = static_cast<Ui::CButton*>(pw->SearchControl(EVENT_SATCOM_PROG));
    if ( button != nullptr )
    {
        button->SetState(STATE_CHECK, m_index==SATCOM_PROG);
        const auto& filename = m_main->GetDisplayInfoName(SATCOM_PROG);
        button->SetState(STATE_VISIBLE, !filename.empty());
    }

    button = static_cast<Ui::CButton*>(pw->SearchControl(EVENT_SATCOM_SOLUCE));
    if ( button != nullptr )
    {
        button->SetState(STATE_CHECK, m_index==SATCOM_SOLUCE);
        const auto& filename = m_main->GetDisplayInfoName(SATCOM_SOLUCE);
        button->SetState(STATE_VISIBLE, !filename.empty() && m_bSoluce);
    }

    group = static_cast<Ui::CGroup*>(pw->SearchControl(EVENT_LABEL1));
    if ( group != nullptr )
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

    edit = static_cast<Ui::CEdit*>(pw->SearchControl(EVENT_EDIT1));
    if ( edit != nullptr )
    {
//?     edit->SetHiliteCap(m_index==SATCOM_LOADING);
        edit->SetHighlightCap(true);
    }

    UpdateCopyButton();
}

// Adjusts the copy button.

void CDisplayInfo::UpdateCopyButton()
{
    Ui::CWindow*    pw;
    Ui::CButton*    button;
    Ui::CEdit*      edit;
    int         c1, c2;

//? if ( m_index != SATCOM_LOADING )  return;

    pw = static_cast<Ui::CWindow*>(m_interface->SearchControl(EVENT_WINDOW4));
    if ( pw == nullptr )  return;

    button = static_cast<Ui::CButton*>(pw->SearchControl(EVENT_HYPER_COPY));
    if ( button == nullptr )  return;

    edit = static_cast<Ui::CEdit*>(pw->SearchControl(EVENT_EDIT1));
    if ( edit == nullptr )  return;

    edit->GetCursor(c1, c2);
    button->SetState(STATE_ENABLE, c1!=c2);

}

// End of the display of information.

void CDisplayInfo::StopDisplayInfo()
{
    Ui::CWindow*        pw;

    pw = static_cast<Ui::CWindow*>(m_interface->SearchControl(EVENT_WINDOW4));
    if ( pw == nullptr )  return;

    m_interface->DeleteControl(EVENT_WINDOW4);

    if ( m_bEditLock )  // editing running program?
    {
        pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW3));
        if ( pw != nullptr )
        {
            pw->SetState(STATE_ENABLE);  // CStudio operating
        }
    }
    else
    {
        m_main->SetEditLock(false, false);

        pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW6));
        if (pw != nullptr) pw->SetState(STATE_VISIBLE | STATE_ENABLE);
    }
    m_pause->DeactivatePause(m_satcomPause);
    m_satcomPause = nullptr;

    m_engine->SetDrawWorld(true);  // draws all on the interface
    m_engine->SetDrawFront(false);  // draws nothing on the interface
    m_particle->SetFrameUpdate(Gfx::SH_WORLD, true);
    m_particle->FlushParticle(Gfx::SH_FRONT);
    m_particle->FlushParticle(Gfx::SH_INTERFACE);

    if ( m_toto != nullptr )
    {
        assert(m_toto->Implements(ObjectInterfaceType::Movable));
        CMotionToto* toto = static_cast<CMotionToto*>(dynamic_cast<CMovableObject&>(*m_toto).GetMotion());
        assert(toto != nullptr);
        toto->StopDisplayInfo();
    }

    m_light->DeleteLight(m_lightSuppl);
    m_lightSuppl = -1;
}


// Specifies the position.

void CDisplayInfo::SetPosition(int pos)
{
    Ui::CWindow*        pw;
    Ui::CEdit*          edit;

    pw = static_cast<Ui::CWindow*>(m_interface->SearchControl(EVENT_WINDOW4));
    if ( pw == nullptr )  return;

    edit = static_cast<Ui::CEdit*>(pw->SearchControl(EVENT_EDIT1));
    if ( edit == nullptr )  return;

    edit->SetFirstLine(pos);
}

// Returns the position.

int CDisplayInfo::GetPosition()
{
    Ui::CWindow*        pw;
    Ui::CEdit*          edit;

    pw = static_cast<Ui::CWindow*>(m_interface->SearchControl(EVENT_WINDOW4));
    if ( pw == nullptr )  return 0;

    edit = static_cast<Ui::CEdit*>(pw->SearchControl(EVENT_EDIT1));
    if ( edit == nullptr )  return 0;

    return edit->GetFirstLine();
}



// Changing the size of the display of information.

void CDisplayInfo::ViewDisplayInfo()
{
    Ui::CWindow*    pw;
    Ui::CEdit*      edit;

    pw = static_cast<Ui::CWindow*>(m_interface->SearchControl(EVENT_WINDOW4));
    if ( pw == nullptr )  return;

    edit = static_cast<Ui::CEdit*>(pw->SearchControl(EVENT_EDIT1));
    if ( edit == nullptr )  return;

    auto dim = m_engine->GetWindowSize();
    edit->SetFontSize(CSettings::GetInstancePointer()->GetFontSize()/(dim.x / 640.0f));
}

// Returns the object human.

CObject* CDisplayInfo::SearchToto()
{
    return CObjectManager::GetInstancePointer()->FindNearest(nullptr, OBJECT_TOTO);
}

}
