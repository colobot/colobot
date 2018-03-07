/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2016, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include "ui/screen/screen_apperance.h"

#include "app/app.h"

#include "common/restext.h"

#include "graphics/engine/camera.h"

#include "level/player_profile.h"
#include "level/robotmain.h"

#include "math/geometry.h"

#include "ui/controls/button.h"
#include "ui/controls/color.h"
#include "ui/controls/interface.h"
#include "ui/controls/label.h"
#include "ui/controls/slider.h"
#include "ui/controls/window.h"

namespace Ui
{

const int PERSO_COLOR[3*10*3] =
{
    // hair:
    193, 221, 226,  // white
    255, 255, 181,  // yellow
    204, 155,  84,  // blond
    165,  48,  10,  // red
    140,  75,  84,  // brown
     83,  64,  51,  // brown
     90,  95,  85,  // black
     85,  48,   9,  // brown
     60,   0,  23,  // black
      0,   0,   0,  //
    // spacesuit:
    203, 206, 204,  // dirty white
      0, 205, 203,  // bluish
    108, 176,   0,  // greenish
    207, 207,  32,  // yellow
    170, 141,   0,  // orange
    108,  84,   0,  // brown
      0,  84, 136,  // bluish
     56,  61, 146,  // bluish
     56,  56,  56,  // black
      0,   0,   0,  //
    // strips:
    255, 255, 255,  // white
    255, 255,   0,  // yellow
    255, 132,   1,  // orange
    255,   0, 255,  // magenta
    255,   0,   0,  // red
      0, 255,   0,  // green
      0, 255, 255,  // cyan
      0,   0, 255,  // blue
     70,  51,  84,  // dark
      0,   0,   0,  //
};

CScreenApperance::CScreenApperance()
    : CScreen(EVENT_WINDOW5,{
        EVENT_INTERFACE_PHEAD,
        EVENT_INTERFACE_PBODY,
        EVENT_INTERFACE_PFACE1,
        EVENT_INTERFACE_PFACE2,
        EVENT_INTERFACE_PFACE3,
        EVENT_INTERFACE_PFACE4,
        EVENT_INTERFACE_PGLASS0,
        EVENT_INTERFACE_PGLASS1,
        EVENT_INTERFACE_PGLASS2,
        EVENT_INTERFACE_PGLASS3,
        EVENT_INTERFACE_PGLASS4,
        EVENT_INTERFACE_PGLASS5,
        EVENT_INTERFACE_PGLASS6,
        EVENT_INTERFACE_PGLASS7,
        EVENT_INTERFACE_PGLASS8,
        EVENT_INTERFACE_PGLASS9,
        // EVENT_INTERFACE_PC0a+j*3+i  (j:0-2)(i:0-2)   (colors)
        EVENT_INTERFACE_PC0a,
        EVENT_INTERFACE_PC1a,
        EVENT_INTERFACE_PC2a,
        EVENT_INTERFACE_PC3a,
        EVENT_INTERFACE_PC4a,
        EVENT_INTERFACE_PC5a,
        EVENT_INTERFACE_PC6a,
        EVENT_INTERFACE_PC7a,
        EVENT_INTERFACE_PC8a,
        EVENT_INTERFACE_PC9a,
        EVENT_INTERFACE_PC0b,
        // EVENT_INTERFACE_PC0b+j*3+i  (j:0-2)(i:0-2)   (colors B)
        EVENT_INTERFACE_PC1b,
        EVENT_INTERFACE_PC2b,
        EVENT_INTERFACE_PC3b,
        EVENT_INTERFACE_PC4b,
        EVENT_INTERFACE_PC5b,
        EVENT_INTERFACE_PC6b,
        EVENT_INTERFACE_PC7b,
        EVENT_INTERFACE_PC8b,
        EVENT_INTERFACE_PC9b,
        // EVENT_INTERFACE_PCRa+i      (0-2)   (SLIDERS)
        EVENT_INTERFACE_PCRa,
        EVENT_INTERFACE_PCGa,
        EVENT_INTERFACE_PCBa,
        // EVENT_INTERFACE_PCRb+i      (0-2)   (SLIDERS)
        EVENT_INTERFACE_PCRb,
        EVENT_INTERFACE_PCGb,
        EVENT_INTERFACE_PCBb,
        // EVENT_INTERFACE_PLROT,     //(button rotation!)
        // EVENT_INTERFACE_PRROT,     //(button rotation!)

        EVENT_INTERFACE_POK,
        EVENT_INTERFACE_PCANCEL,
        EVENT_INTERFACE_PDEF,         //(button default)
        })
    , m_apperanceTab(0)
    , m_apperanceAngle(0.0f)
{
}

void CScreenApperance::CreateInterface()
{
    CWindow*        pw;
    CLabel*         pl;
    CButton*        pb;
    CColor*         pco;
    CSlider*        psl;
    Math::Point     pos, ddim;
    std::string     name;

    pos.x = 0.10f;
    pos.y = 0.10f;
    ddim.x = 0.80f;
    ddim.y = 0.80f;
    pw = m_interface->CreateWindows(pos, ddim, 12, EVENT_WINDOW5);
    GetResource(RES_TEXT, RT_TITLE_PERSO, name);
    pw->SetName(name);

    pos.x  = 0.10f;
    pos.y  = 0.40f;
    ddim.x = 0.50f;
    ddim.y = 0.50f;
    pw->CreateGroup(pos, ddim, 5, EVENT_INTERFACE_GLINTl);  // orange corner
    pos.x  = 0.40f;
    pos.y  = 0.10f;
    ddim.x = 0.50f;
    ddim.y = 0.50f;
    pw->CreateGroup(pos, ddim, 4, EVENT_INTERFACE_GLINTr);  // blue corner

    pos.x  =  95.0f/640.0f;
    pos.y  = 108.0f/480.0f;
    ddim.x = 220.0f/640.0f;
    ddim.y = 274.0f/480.0f;
    pw->CreateGroup(pos, ddim, 17, EVENT_NULL);  // frame

    pos.x  = 100.0f/640.0f;
    pos.y  = 364.0f/480.0f;
    ddim.x = 210.0f/640.0f;
    ddim.y =  14.0f/480.0f;
    pw->CreateGroup(pos, ddim, 3, EVENT_NULL);  // transparent -> gray

    pos.x  = 120.0f/640.0f;
    pos.y  = 364.0f/480.0f;
    ddim.x =  80.0f/640.0f;
    ddim.y =  28.0f/480.0f;
    pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_PHEAD);
    pb->SetState(STATE_SHADOW);
    pb->SetState(STATE_CARD);

    pos.x  = 210.0f/640.0f;
    pos.y  = 364.0f/480.0f;
    ddim.x =  80.0f/640.0f;
    ddim.y =  28.0f/480.0f;
    pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_PBODY);
    pb->SetState(STATE_SHADOW);
    pb->SetState(STATE_CARD);

    pos.x  = 100.0f/640.0f;
    pos.y  = 354.0f/480.0f;
    ddim.x = 210.0f/640.0f;
    ddim.y =  10.0f/480.0f;
    pw->CreateGroup(pos, ddim, 1, EVENT_INTERFACE_GLINTb);  // orange bar
    pos.x  = 100.0f/640.0f;
    pos.y  = 154.0f/480.0f;
    ddim.x = 210.0f/640.0f;
    ddim.y = 200.0f/480.0f;
    pw->CreateGroup(pos, ddim, 2, EVENT_INTERFACE_GLINTu);  // orange -> transparent

    // Face
    pos.x  = 340.0f/640.0f;
    pos.y  = 356.0f/480.0f;
    ddim.x = 200.0f/640.0f;
    ddim.y =  16.0f/480.0f;
    pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL11, "");
    pl->SetTextAlign(Gfx::TEXT_ALIGN_LEFT);

    pos.x  = 340.0f/640.0f;
    pos.y  = 312.0f/480.0f;
    ddim.x =  44.0f/640.0f;
    ddim.y =  44.0f/480.0f;
    pb = pw->CreateButton(pos, ddim, 43, EVENT_INTERFACE_PFACE1);
    pb->SetState(STATE_SHADOW);
    pos.x += 50.0f/640.0f;
    pb = pw->CreateButton(pos, ddim, 44, EVENT_INTERFACE_PFACE2);
    pb->SetState(STATE_SHADOW);
    pos.x += 50.0f/640.0f;
    pb = pw->CreateButton(pos, ddim, 45, EVENT_INTERFACE_PFACE3);
    pb->SetState(STATE_SHADOW);
    pos.x += 50.0f/640.0f;
    pb = pw->CreateButton(pos, ddim, 46, EVENT_INTERFACE_PFACE4);
    pb->SetState(STATE_SHADOW);

    // Glasses
    pos.x  = 340.0f/640.0f;
    pos.y  = 270.0f/480.0f;
    ddim.x = 200.0f/640.0f;
    ddim.y =  16.0f/480.0f;
    pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL12, "");
    pl->SetTextAlign(Gfx::TEXT_ALIGN_LEFT);

    pos.x  = 340.0f/640.0f;
    pos.y  = 240.0f/480.0f;
    ddim.x =  30.0f/640.0f;
    ddim.y =  30.0f/480.0f;
    for ( int i=0 ; i<6 ; i++ )
    {
        int ti[6] = {11, 179, 180, 181, 182, 183};
        pb = pw->CreateButton(pos, ddim, ti[i], static_cast<EventType>(EVENT_INTERFACE_PGLASS0+i));
        pb->SetState(STATE_SHADOW);
        pos.x += (30.0f+2.8f)/640.0f;
    }

    // Color A
    pos.x  = 340.0f/640.0f;
    pos.y  = 300.0f/480.0f;
    ddim.x = 200.0f/640.0f;
    ddim.y =  16.0f/480.0f;
    pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL14, "");
    pl->SetTextAlign(Gfx::TEXT_ALIGN_LEFT);

    pos.y  = 282.0f/480.0f;
    ddim.x =  18.0f/640.0f;
    ddim.y =  18.0f/480.0f;
    for ( int j=0 ; j<3 ; j++ )
    {
        pos.x  = 340.0f/640.0f;
        for ( int i=0 ; i<3 ; i++ )
        {
            pco = pw->CreateColor(pos, ddim, -1, static_cast<EventType>(EVENT_INTERFACE_PC0a+j*3+i));
            pco->SetState(STATE_SHADOW);
            pos.x += 20.0f/640.0f;
        }
        pos.y -= 20.0f/480.0f;
    }

    pos.x  = 420.0f/640.0f;
    pos.y  = 282.0f/480.0f;
    ddim.x = 100.0f/640.0f;
    ddim.y =  18.0f/480.0f;
    for ( int i=0 ; i<3 ; i++ )
    {
        psl = pw->CreateSlider(pos, ddim, 0, static_cast<EventType>(EVENT_INTERFACE_PCRa+i));
        psl->SetState(STATE_SHADOW);
        psl->SetLimit(0.0f, 255.0f);
        psl->SetArrowStep(16.0f);
        pos.y -= 20.0f/480.0f;
    }

    // Color B
    pos.x  = 340.0f/640.0f;
    pos.y  = 192.0f/480.0f;
    ddim.x = 200.0f/640.0f;
    ddim.y =  16.0f/480.0f;
    pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL13, "");
    pl->SetTextAlign(Gfx::TEXT_ALIGN_LEFT);

    pos.y  = 174.0f/480.0f;
    ddim.x =  18.0f/640.0f;
    ddim.y =  18.0f/480.0f;
    for ( int j=0 ; j<3 ; j++ )
    {
        pos.x  = 340.0f/640.0f;
        for ( int i=0 ; i<3 ; i++ )
        {
            pco = pw->CreateColor(pos, ddim, -1, static_cast<EventType>(EVENT_INTERFACE_PC0b+j*3+i));
            pco->SetState(STATE_SHADOW);
            pos.x += 20.0f/640.0f;
        }
        pos.y -= 20.0f/480.0f;
    }

    pos.x  = 420.0f/640.0f;
    pos.y  = 174.0f/480.0f;
    ddim.x = 100.0f/640.0f;
    ddim.y =  18.0f/480.0f;
    for ( int i=0 ; i<3 ; i++ )
    {
        psl = pw->CreateSlider(pos, ddim, 0, static_cast<EventType>(EVENT_INTERFACE_PCRb+i));
        psl->SetState(STATE_SHADOW);
        psl->SetLimit(0.0f, 255.0f);
        psl->SetArrowStep(16.0f);
        pos.y -= 20.0f/480.0f;
    }

    // Rotation
    pos.x  = 100.0f/640.0f;
    pos.y  = 113.0f/480.0f;
    ddim.x =  20.0f/640.0f;
    ddim.y =  20.0f/480.0f;
    pb = pw->CreateButton(pos, ddim, 55, EVENT_INTERFACE_PLROT);  // <
    pb->SetState(STATE_SHADOW);
    pb->SetRepeat(true);

    pos.x  = 290.0f/640.0f;
    pos.y  = 113.0f/480.0f;
    ddim.x =  20.0f/640.0f;
    ddim.y =  20.0f/480.0f;
    pb = pw->CreateButton(pos, ddim, 48, EVENT_INTERFACE_PRROT);  // >
    pb->SetState(STATE_SHADOW);
    pb->SetRepeat(true);

    pos.x  = 100.0f/640.0f;
    pos.y  =  70.0f/480.0f;
    ddim.x = 100.0f/640.0f;
    ddim.y =  32.0f/480.0f;
    pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_POK);
    pb->SetState(STATE_SHADOW);

    pos.x = 210.0f/640.0f;
    pos.y =  70.0f/480.0f;
    ddim.x =100.0f/640.0f;
    ddim.y = 32.0f/480.0f;
    pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_PCANCEL);
    pb->SetState(STATE_SHADOW);

    pos.x = 340.0f/640.0f;
    pos.y =  70.0f/480.0f;
    ddim.x =194.0f/640.0f;
    ddim.y = 32.0f/480.0f;
    pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_PDEF);
    pb->SetState(STATE_SHADOW);

    m_apperanceTab = 0;
    m_apperanceAngle = -0.6f;
    m_main->GetPlayerProfile()->LoadApperance();
    UpdatePerso();
    m_main->ScenePerso();
    CameraPerso();
}

bool CScreenApperance::EventProcess(const Event &event)
{
    if(!EventProcessTabStop(event))
        return false;   //mgd
    PlayerApperance& apperance = m_main->GetPlayerProfile()->GetApperance();
    switch( event.type )
    {
        case EVENT_KEY_DOWN:
        {
            auto data = event.GetData<KeyEventData>();
            switch (data->key)
            {
            case KEY(RETURN):
                //??? NO SAVE ??
                m_main->ChangePhase(PHASE_MAIN_MENU);
                break;
            case KEY(ESCAPE):
                //??? NO RESTORE ??
                m_main->ChangePhase(PHASE_PLAYER_SELECT);
                break;
            case KEY(LEFT):
                return EventProcess(Event(EVENT_INTERFACE_PLROT));
            case KEY(RIGHT):
                return EventProcess(Event(EVENT_INTERFACE_PRROT));
            }
            break;
        }

        case EVENT_UPDINTERFACE:
            CameraPerso();
            break;

        case EVENT_INTERFACE_PHEAD:
            m_apperanceTab = 0;
            UpdatePerso();
            m_main->ScenePerso();
            CameraPerso();
            break;
        case EVENT_INTERFACE_PBODY:
            m_apperanceTab = 1;
            UpdatePerso();
            m_main->ScenePerso();
            CameraPerso();
            break;

        case EVENT_INTERFACE_PFACE1:
        case EVENT_INTERFACE_PFACE2:
        case EVENT_INTERFACE_PFACE3:
        case EVENT_INTERFACE_PFACE4:
            apperance.face = event.type-EVENT_INTERFACE_PFACE1;
            apperance.DefHairColor();
            UpdatePerso();
            m_main->ScenePerso();
            break;

        case EVENT_INTERFACE_PGLASS0:
        case EVENT_INTERFACE_PGLASS1:
        case EVENT_INTERFACE_PGLASS2:
        case EVENT_INTERFACE_PGLASS3:
        case EVENT_INTERFACE_PGLASS4:
        case EVENT_INTERFACE_PGLASS5:
        case EVENT_INTERFACE_PGLASS6:
        case EVENT_INTERFACE_PGLASS7:
        case EVENT_INTERFACE_PGLASS8:
        case EVENT_INTERFACE_PGLASS9:
            apperance.glasses = event.type-EVENT_INTERFACE_PGLASS0;
            UpdatePerso();
            m_main->ScenePerso();
            break;

        case EVENT_INTERFACE_PC0a:
        case EVENT_INTERFACE_PC1a:
        case EVENT_INTERFACE_PC2a:
        case EVENT_INTERFACE_PC3a:
        case EVENT_INTERFACE_PC4a:
        case EVENT_INTERFACE_PC5a:
        case EVENT_INTERFACE_PC6a:
        case EVENT_INTERFACE_PC7a:
        case EVENT_INTERFACE_PC8a:
        case EVENT_INTERFACE_PC9a:
            FixPerso(event.type-EVENT_INTERFACE_PC0a, 0);
            UpdatePerso();
            m_main->ScenePerso();
            break;

        case EVENT_INTERFACE_PC0b:
        case EVENT_INTERFACE_PC1b:
        case EVENT_INTERFACE_PC2b:
        case EVENT_INTERFACE_PC3b:
        case EVENT_INTERFACE_PC4b:
        case EVENT_INTERFACE_PC5b:
        case EVENT_INTERFACE_PC6b:
        case EVENT_INTERFACE_PC7b:
        case EVENT_INTERFACE_PC8b:
        case EVENT_INTERFACE_PC9b:
            FixPerso(event.type-EVENT_INTERFACE_PC0b, 1);
            UpdatePerso();
            m_main->ScenePerso();
            break;

        case EVENT_INTERFACE_PCRa:
        case EVENT_INTERFACE_PCGa:
        case EVENT_INTERFACE_PCBa:
        case EVENT_INTERFACE_PCRb:
        case EVENT_INTERFACE_PCGb:
        case EVENT_INTERFACE_PCBb:
            ColorPerso();
            UpdatePerso();
            m_main->ScenePerso();
            break;

        case EVENT_INTERFACE_PDEF:
            apperance.DefPerso();
            UpdatePerso();
            m_main->ScenePerso();
            break;

        case EVENT_INTERFACE_PLROT:
            m_apperanceAngle += 0.2f;
            break;
        case EVENT_INTERFACE_PRROT:
            m_apperanceAngle -= 0.2f;
            break;

        case EVENT_INTERFACE_POK:
            m_main->GetPlayerProfile()->SaveApperance();
            m_main->ChangePhase(PHASE_MAIN_MENU);
            break;

        case EVENT_INTERFACE_PCANCEL:
            m_main->GetPlayerProfile()->LoadApperance(); // reload apperance from file
            m_main->ChangePhase(PHASE_PLAYER_SELECT);
            m_main->SelectPlayer(" ");  //used by SatCom from screan_player_select
            break;

        default:
            return true;
    }
    return false;
}

bool CScreenApperance::GetGamerOnlyHead()
{
    return m_apperanceTab == 0;
}

float CScreenApperance::GetPersoAngle()
{
    return m_apperanceAngle;
}

// Tests whether two colors are equal or nearly are.

bool EqColor(const Gfx::Color &c1, const Gfx::Color &c2)
{
    return (fabs(c1.r-c2.r) < 0.01f &&
            fabs(c1.g-c2.g) < 0.01f &&
            fabs(c1.b-c2.b) < 0.01f );
}

// Updates all the buttons for the character.

void CScreenApperance::UpdatePerso()
{
    CWindow*        pw;
    CLabel*         pl;
    CButton*        pb;
    CColor*         pc;
    CSlider*        ps;
    Gfx::Color   color;
    std::string  name;
    int             i;

    PlayerApperance& apperance = m_main->GetPlayerProfile()->GetApperance();

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == nullptr )  return;

    pb = static_cast<CButton*>(pw->SearchControl(EVENT_INTERFACE_PHEAD));
    if ( pb != nullptr )
    {
        pb->SetState(STATE_CHECK, m_apperanceTab==0);
    }
    pb = static_cast<CButton*>(pw->SearchControl(EVENT_INTERFACE_PBODY));
    if ( pb != nullptr )
    {
        pb->SetState(STATE_CHECK, m_apperanceTab==1);
    }

    pl = static_cast<CLabel*>(pw->SearchControl(EVENT_LABEL11));
    if ( pl != nullptr )
    {
        if ( m_apperanceTab == 0 )
        {
            pl->SetState(STATE_VISIBLE);
            GetResource(RES_TEXT, RT_PERSO_FACE, name);
            pl->SetName(name);
        }
        else
        {
            pl->ClearState(STATE_VISIBLE);
        }
    }

    pl = static_cast<CLabel*>(pw->SearchControl(EVENT_LABEL12));
    if ( pl != nullptr )
    {
        if ( m_apperanceTab == 0 )
        {
            pl->SetState(STATE_VISIBLE);
            GetResource(RES_TEXT, RT_PERSO_GLASSES, name);
            pl->SetName(name);
        }
        else
        {
            pl->ClearState(STATE_VISIBLE);
        }
    }

    pl = static_cast<CLabel*>(pw->SearchControl(EVENT_LABEL13));
    if ( pl != nullptr )
    {
        if ( m_apperanceTab == 0 )  GetResource(RES_TEXT, RT_PERSO_HAIR, name);
        else                    GetResource(RES_TEXT, RT_PERSO_BAND, name);
        pl->SetName(name);
    }

    pl = static_cast<CLabel*>(pw->SearchControl(EVENT_LABEL14));
    if ( pl != nullptr )
    {
        if ( m_apperanceTab == 0 )
        {
            pl->ClearState(STATE_VISIBLE);
        }
        else
        {
            pl->SetState(STATE_VISIBLE);
            GetResource(RES_TEXT, RT_PERSO_COMBI, name);
            pl->SetName(name);
        }
    }

    for ( i=0 ; i<4 ; i++ )
    {
        pb = static_cast<CButton*>(pw->SearchControl(static_cast<EventType>(EVENT_INTERFACE_PFACE1+i)));
        if ( pb == nullptr )  break;
        pb->SetState(STATE_VISIBLE, m_apperanceTab==0);
        pb->SetState(STATE_CHECK, i==apperance.face);
    }

    for ( i=0 ; i<10 ; i++ )
    {
        pb = static_cast<CButton*>(pw->SearchControl(static_cast<EventType>(EVENT_INTERFACE_PGLASS0+i)));
        if ( pb == nullptr )  break;
        pb->SetState(STATE_VISIBLE, m_apperanceTab==0);
        pb->SetState(STATE_CHECK, i==apperance.glasses);
    }

    for ( i=0 ; i<3*3 ; i++ )
    {
        pc = static_cast<CColor*>(pw->SearchControl(static_cast<EventType>(EVENT_INTERFACE_PC0a+i)));
        if ( pc == nullptr )  break;
        if ( m_apperanceTab == 0 )
        {
            pc->ClearState(STATE_VISIBLE);
        }
        else
        {
            pc->SetState(STATE_VISIBLE);
            color.r = PERSO_COLOR[3*10*1+3*i+0]/255.0f;
            color.g = PERSO_COLOR[3*10*1+3*i+1]/255.0f;
            color.b = PERSO_COLOR[3*10*1+3*i+2]/255.0f;
            color.a = 0.0f;
            pc->SetColor(color);
            pc->SetState(STATE_CHECK, EqColor(color, apperance.colorCombi));
        }

        pc = static_cast<CColor*>(pw->SearchControl(static_cast<EventType>(EVENT_INTERFACE_PC0b+i)));
        if ( pc == nullptr )  break;
        color.r = PERSO_COLOR[3*10*2*m_apperanceTab+3*i+0]/255.0f;
        color.g = PERSO_COLOR[3*10*2*m_apperanceTab+3*i+1]/255.0f;
        color.b = PERSO_COLOR[3*10*2*m_apperanceTab+3*i+2]/255.0f;
        color.a = 0.0f;
        pc->SetColor(color);
        pc->SetState(STATE_CHECK, EqColor(color, m_apperanceTab?apperance.colorBand:apperance.colorHair));
    }

    for ( i=0 ; i<3 ; i++ )
    {
        ps = static_cast<CSlider*>(pw->SearchControl(static_cast<EventType>(EVENT_INTERFACE_PCRa+i)));
        if ( ps == nullptr )  break;
        ps->SetState(STATE_VISIBLE, m_apperanceTab==1);
    }

    if ( m_apperanceTab == 1 )
    {
        color = apperance.colorCombi;
        ps = static_cast<CSlider*>(pw->SearchControl(EVENT_INTERFACE_PCRa));
        if ( ps != nullptr )  ps->SetVisibleValue(color.r*255.0f);
        ps = static_cast<CSlider*>(pw->SearchControl(EVENT_INTERFACE_PCGa));
        if ( ps != nullptr )  ps->SetVisibleValue(color.g*255.0f);
        ps = static_cast<CSlider*>(pw->SearchControl(EVENT_INTERFACE_PCBa));
        if ( ps != nullptr )  ps->SetVisibleValue(color.b*255.0f);
    }

    if ( m_apperanceTab == 0 )  color = apperance.colorHair;
    else                    color = apperance.colorBand;
    ps = static_cast<CSlider*>(pw->SearchControl(EVENT_INTERFACE_PCRb));
    if ( ps != nullptr )  ps->SetVisibleValue(color.r*255.0f);
    ps = static_cast<CSlider*>(pw->SearchControl(EVENT_INTERFACE_PCGb));
    if ( ps != nullptr )  ps->SetVisibleValue(color.g*255.0f);
    ps = static_cast<CSlider*>(pw->SearchControl(EVENT_INTERFACE_PCBb));
    if ( ps != nullptr )  ps->SetVisibleValue(color.b*255.0f);
}

// Updates the camera for the character.

void CScreenApperance::CameraPerso()
{
    if ( m_apperanceTab == 0 )
    {
        SetCamera(0.325f, -0.15f, 5.0f);
    }
    else
    {
        SetCamera(0.325f, 0.3f, 18.0f);
    }
}

// Sets a fixed color.

void CScreenApperance::FixPerso(int rank, int index)
{
    PlayerApperance& apperance = m_main->GetPlayerProfile()->GetApperance();
    if ( m_apperanceTab == 0 )
    {
        if ( index == 1 )
        {
            apperance.colorHair.r = PERSO_COLOR[3*10*0+rank*3+0]/255.0f;
            apperance.colorHair.g = PERSO_COLOR[3*10*0+rank*3+1]/255.0f;
            apperance.colorHair.b = PERSO_COLOR[3*10*0+rank*3+2]/255.0f;
        }
    }
    if ( m_apperanceTab == 1 )
    {
        if ( index == 0 )
        {
            apperance.colorCombi.r = PERSO_COLOR[3*10*1+rank*3+0]/255.0f;
            apperance.colorCombi.g = PERSO_COLOR[3*10*1+rank*3+1]/255.0f;
            apperance.colorCombi.b = PERSO_COLOR[3*10*1+rank*3+2]/255.0f;
        }
        if ( index == 1 )
        {
            apperance.colorBand.r = PERSO_COLOR[3*10*2+rank*3+0]/255.0f;
            apperance.colorBand.g = PERSO_COLOR[3*10*2+rank*3+1]/255.0f;
            apperance.colorBand.b = PERSO_COLOR[3*10*2+rank*3+2]/255.0f;
        }
    }
}

// Updates the color of the character.

void CScreenApperance::ColorPerso()
{
    CWindow*        pw;
    CSlider*        ps;
    Gfx::Color   color;

    PlayerApperance& apperance = m_main->GetPlayerProfile()->GetApperance();

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == nullptr )  return;

    color.a = 0.0f;

    ps = static_cast<CSlider*>(pw->SearchControl(EVENT_INTERFACE_PCRa));
    if ( ps != nullptr )  color.r = ps->GetVisibleValue()/255.0f;
    ps = static_cast<CSlider*>(pw->SearchControl(EVENT_INTERFACE_PCGa));
    if ( ps != nullptr )  color.g = ps->GetVisibleValue()/255.0f;
    ps = static_cast<CSlider*>(pw->SearchControl(EVENT_INTERFACE_PCBa));
    if ( ps != nullptr )  color.b = ps->GetVisibleValue()/255.0f;
    if ( m_apperanceTab == 1 )  apperance.colorCombi = color;

    ps = static_cast<CSlider*>(pw->SearchControl(EVENT_INTERFACE_PCRb));
    if ( ps != nullptr )  color.r = ps->GetVisibleValue()/255.0f;
    ps = static_cast<CSlider*>(pw->SearchControl(EVENT_INTERFACE_PCGb));
    if ( ps != nullptr )  color.g = ps->GetVisibleValue()/255.0f;
    ps = static_cast<CSlider*>(pw->SearchControl(EVENT_INTERFACE_PCBb));
    if ( ps != nullptr )  color.b = ps->GetVisibleValue()/255.0f;
    if ( m_apperanceTab == 0 )  apperance.colorHair = color;
    else                        apperance.colorBand = color;
}

void CScreenApperance::SetCamera(float x, float y, float cameraDistance)
{
    Gfx::CCamera* camera = m_main->GetCamera();
    Gfx::CEngine* engine = Gfx::CEngine::GetInstancePointer();

    camera->SetType(Gfx::CAM_TYPE_SCRIPT);

    Math::Vector p2D(x, y, cameraDistance);
    Math::Vector p3D;
    Math::Matrix matView;
    Math::Matrix matProj = engine->GetMatProj();

    Math::LoadViewMatrix(matView, Math::Vector(0.0f, 0.0f, -cameraDistance),
                            Math::Vector(0.0f, 0.0f, 0.0f),
                            Math::Vector(0.0f, 0.0f, 1.0f));

    p2D.x = p2D.x * 2.0f - 1.0f;  // [0..1] -> [-1..1]
    p2D.y = p2D.y * 2.0f - 1.0f;

    p3D.x = p2D.x * p2D.z / matProj.Get(1,1);
    p3D.y = p2D.y * p2D.z / matProj.Get(2,2);
    p3D.z = p2D.z;

    p3D = Math::Transform(matView.Inverse(), p3D);
    p3D = -p3D;

    camera->SetScriptCamera(Math::Vector(cameraDistance, p3D.y, p3D.x),
                            Math::Vector(0.0f, p3D.y, p3D.x));
}

} // namespace Ui
