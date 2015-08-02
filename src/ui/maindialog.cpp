/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2014, Daniel Roux, EPSITEC SA & TerranovaTeam
 * http://epsiteс.ch; http://colobot.info; http://github.com/colobot
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


#include "ui/maindialog.h"

#include "common/config.h"

#include "app/app.h"
#include "app/input.h"
#include "app/system.h"

#include "common/config_file.h"
#include "common/event.h"
#include "common/global.h"
#include "common/logger.h"
#include "common/make_unique.h"
#include "common/misc.h"
#include "common/restext.h"
#include "common/stringutils.h"

#include "common/resources/inputstream.h"
#include "common/resources/outputstream.h"
#include "common/resources/resourcemanager.h"

#include "object/player_profile.h"
#include "object/robotmain.h"

#include "object/level/parser.h"

#include "sound/sound.h"

#include "ui/button.h"
#include "ui/check.h"
#include "ui/color.h"
#include "ui/edit.h"
#include "ui/editvalue.h"
#include "ui/group.h"
#include "ui/image.h"
#include "ui/interface.h"
#include "ui/key.h"
#include "ui/label.h"
#include "ui/list.h"
#include "ui/scroll.h"
#include "ui/slider.h"
#include "ui/window.h"

#include <stdio.h>
#include <string.h>
#include <sstream>
#include <iomanip>
#include <vector>
#include <boost/lexical_cast.hpp>

//TODO Get rid of all sprintf's

namespace Ui
{

const int KEY_VISIBLE = 6;      // number of visible keys redefinable

const float WELCOME_LENGTH = 3.0f;

static int perso_color[3*10*3] =
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

// Constructor of robot application.

CMainDialog::CMainDialog()
{
    m_app        = nullptr;
    m_eventQueue = nullptr;
    m_sound      = nullptr;
    m_main       = nullptr;
    m_interface  = nullptr;
    m_camera     = nullptr;
    m_engine     = nullptr;
    m_particle   = nullptr;
    m_pause      = nullptr;

    m_phase        = PHASE_PLAYER_SELECT;
    m_phaseSetup   = PHASE_SETUPg;
    m_sceneRead[0] = 0;
    m_stackRead[0] = 0;
    m_levelChap    = 0;
    m_levelRank    = 0;
    m_bSceneSoluce = false;
    m_bSimulSetup  = false;

    m_accessEnable = true;
    m_accessMission= true;
    m_accessUser   = true;

    m_bDeleteGamer = true;

    for(int i = 0; i < static_cast<int>(LevelCategory::Max); i++)
    {
        m_chap[static_cast<LevelCategory>(i)] = 0;
        m_sel[static_cast<LevelCategory>(i)] = 0;
    }

    m_category = LevelCategory::Exercises;
    m_listCategory = m_category;
    m_maxList = 0;

    m_bTooltip       = true;
    m_bGlint         = true;
    m_bRain          = true;
    m_bSoluce4       = true;
    m_bMovies        = true;
    m_bNiceReset     = true;
    m_bHimselfDamage = true;
    m_bCameraScroll  = true;

    m_bCameraInvertX = false;
    m_bCameraInvertY = false;
    m_bEffect        = true;
    m_bBlood         = true;
    m_bAutosave      = true;
    m_shotDelay      = 0;

    m_glintMouse = Math::Point(0.0f, 0.0f);
    m_glintTime  = 1000.0f;

    for (int i = 0; i < 10; i++)
    {
        m_partiPhase[i] = 0;
        m_partiTime[i]  = 0.0f;
    }

    m_setupFull = false;

    m_bDialog = false;
}

void CMainDialog::Create()
{
    m_app        = CApplication::GetInstancePointer();
    m_eventQueue = m_app->GetEventQueue();
    m_sound      = m_app->GetSound();
    m_main       = CRobotMain::GetInstancePointer();
    m_interface  = m_main->GetInterface();
    m_camera     = m_main->GetCamera();
    m_engine     = Gfx::CEngine::GetInstancePointer();
    m_particle   = m_engine->GetParticle();
    m_pause      = CPauseManager::GetInstancePointer();

    m_setupFull = m_app->GetVideoConfig().fullScreen;
}

// Destructor of robot application.

CMainDialog::~CMainDialog()
{
}


// Changes phase.

void CMainDialog::ChangePhase(Phase phase)
{
    CWindow*        pw = nullptr;
    CEdit*          pe;
    CEditValue*     pv;
    CLabel*         pl;
    CList*          pli;
    CCheck*         pc;
    CScroll*        ps;
    CSlider*        psl;
    CEnumSlider*    pes;
    CButton*        pb;
    CColor*         pco;
    CGroup*         pg;
    CImage*         pi;
    Math::Point         pos, dim, ddim;
    float           ox, oy, sx, sy;
    std::string     name;
    int             res, i, j;

    m_camera->SetType(Gfx::CAM_TYPE_DIALOG);
    m_engine->SetOverFront(false);
    m_engine->SetOverColor(Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f), Gfx::ENG_RSTATE_TCOLOR_BLACK); // TODO: color ok?

    m_phase = phase;  // copy the info to CRobotMain
    m_phaseTime = 0.0f;

    dim.x = 32.0f/640.0f;
    dim.y = 32.0f/480.0f;
    ox = 3.0f/640.0f;
    oy = 3.0f/480.0f;
    sx = (32.0f+2.0f)/640.0f;
    sy = (32.0f+2.0f)/480.0f;

    if ( m_phase == PHASE_MAIN_MENU )
    {
        pos.x  = 0.35f;
        pos.y  = 0.10f;
        ddim.x = 0.30f;
        ddim.y = 0.80f;
        pw = m_interface->CreateWindows(pos, ddim, 10, EVENT_WINDOW5);

        GetResource(RES_TEXT, RT_TITLE_INIT, name);
        pw->SetName(name);

        pos.x  = 0.35f;
        pos.y  = 0.60f;
        ddim.x = 0.30f;
        ddim.y = 0.30f;
        pw->CreateGroup(pos, ddim, 5, EVENT_INTERFACE_GLINTl);  // orange corner
        pos.x  = 0.35f;
        pos.y  = 0.10f;
        ddim.x = 0.30f;
        ddim.y = 0.30f;
        pw->CreateGroup(pos, ddim, 4, EVENT_INTERFACE_GLINTr);  // blue corner

        ddim.x = 0.20f;
        ddim.y = dim.y*2.4f;
        pos.x = 0.40f;
        if ( m_accessEnable && m_accessMission )
        {
            pos.y = oy+sy*9.1f;
            pg = pw->CreateGroup(pos, ddim, 23, EVENT_LABEL1);  // yellow
            pg->SetState(STATE_SHADOW);
        }
        pos.y = oy+sy*6.8f;
        pg = pw->CreateGroup(pos, ddim, 24, EVENT_LABEL1);  // orange
        pg->SetState(STATE_SHADOW);
        pos.y = oy+sy*3.9f;
        pg = pw->CreateGroup(pos, ddim, 25, EVENT_LABEL1);  // orange
        pg->SetState(STATE_SHADOW);
        ddim.y = dim.y*1.2f;
        pos.y = oy+sy*1.9f;
        pg = pw->CreateGroup(pos, ddim, 26, EVENT_LABEL1);  // red
        pg->SetState(STATE_SHADOW);

        ddim.x = 0.18f;
        ddim.y = dim.y*1;
        pos.x = 0.41f;

        if ( m_accessEnable && m_accessMission )
        {
            pos.y = oy+sy*10.3f;
            pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_MISSION);
            pb->SetState(STATE_SHADOW);

            pos.y = oy+sy*9.2f;
            pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_FREE);
            pb->SetState(STATE_SHADOW);
        }

        pos.y = oy+sy*8.0f;
        pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_TRAINER);
        pb->SetState(STATE_SHADOW);

        pos.y = oy+sy*6.9f;
        pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_DEFI);
        pb->SetState(STATE_SHADOW);

        pos.y = oy+sy*5.1f;
        pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_SETUP);
        pb->SetState(STATE_SHADOW);

        pos.y = oy+sy*4.0f;
        pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_NAME);
        pb->SetState(STATE_SHADOW);

        pos.y = oy+sy*2.0f;
        pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_QUIT);
        pb->SetState(STATE_SHADOW);

        if ( m_accessEnable && m_accessUser )
        {
            pos.x  = 447.0f/640.0f;
            pos.y  = 313.0f/480.0f;
            ddim.x = 0.09f;
            pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_USER);
            pb->SetState(STATE_SHADOW);

            try
            {
                CLevelParser levelParser("levels/custom/config.txt");
                if (levelParser.Exists())
                {
                    levelParser.Load();
                    CLevelParserLine* line = levelParser.Get("Button");
                    if (line->GetParam("name")->IsDefined())
                        pb->SetName(line->GetParam("name")->AsString());
                    if (line->GetParam("tooltip")->IsDefined())
                        pb->SetTooltip(line->GetParam("tooltip")->AsString());
                }
            }
            catch (CLevelParserException& e)
            {
                GetLogger()->Error("Failed loading userlevel button name: %s\n", e.what());
            }
        }

        /*pos.x  = 139.0f/640.0f;
        pos.y  = 313.0f/480.0f;
        ddim.x = 0.09f;
        pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_PROTO);
        pb->SetState(STATE_SHADOW);*/

        pos.x  = 0.40f;
        ddim.x = 0.20f;
        pos.y  =  26.0f/480.0f;
        ddim.y =  12.0f/480.0f;
        pg = pw->CreateGroup(pos, ddim, 1, EVENT_LABEL1);
        pg->SetState(STATE_SHADOW);
        pos.y -=  5.0f/480.0f;
        pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL1, "TerranovaTeam");
        pl->SetFontType(Gfx::FONT_COURIER);
        pl->SetFontSize(Gfx::FONT_SIZE_SMALL);

        m_engine->SetBackground("interface/interface.png",
                Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
                Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
                Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
                Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
                true);
        m_engine->SetBackForce(true);
    }

    if ( m_phase == PHASE_PLAYER_SELECT )
    {
        pos.x = 0.10f;
        pos.y = 0.10f;
        ddim.x = 0.80f;
        ddim.y = 0.80f;
        pw = m_interface->CreateWindows(pos, ddim, 12, EVENT_WINDOW5);
        GetResource(RES_TEXT, RT_TITLE_NAME, name);
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

        pos.x =  60.0f/640.0f;
        pos.y = 313.0f/480.0f;
        ddim.x = 120.0f/640.0f;
        ddim.y =  32.0f/480.0f;
        GetResource(RES_EVENT, EVENT_INTERFACE_NLABEL, name);
        pl = pw->CreateLabel(pos, ddim, -1, EVENT_INTERFACE_NLABEL, name);
        pl->SetTextAlign(Gfx::TEXT_ALIGN_RIGHT);

        pos.x = 200.0f/640.0f;
        pos.y = 320.0f/480.0f;
        ddim.x = 160.0f/640.0f;
        ddim.y =  32.0f/480.0f;
        pg = pw->CreateGroup(pos, ddim, 7, EVENT_LABEL1);
        pg->SetState(STATE_SHADOW);

        pos.x = 207.0f/640.0f;
        pos.y = 328.0f/480.0f;
        ddim.x = 144.0f/640.0f;
        ddim.y =  18.0f/480.0f;
        pe = pw->CreateEdit(pos, ddim, 0, EVENT_INTERFACE_NEDIT);
        pe->SetMaxChar(15);
        if(m_main->GetPlayerProfile() != nullptr)
        {
            name = m_main->GetPlayerProfile()->GetName();
        }
        else
        {
            name = CPlayerProfile::GetLastName();
        }
        pe->SetText(name.c_str());
        pe->SetCursor(name.length(), 0);
        m_interface->SetFocus(pe);

        pos.x = 380.0f/640.0f;
        pos.y = 320.0f/480.0f;
        ddim.x =100.0f/640.0f;
        ddim.y = 32.0f/480.0f;
        pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_NOK);
        pb->SetState(STATE_SHADOW);

        pos.x = 380.0f/640.0f;
        pos.y = 250.0f/480.0f;
        ddim.x =100.0f/640.0f;
        ddim.y = 52.0f/480.0f;
        pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_PERSO);
        pb->SetState(STATE_SHADOW);

        pos.x = 200.0f/640.0f;
        pos.y = 150.0f/480.0f;
        ddim.x = 160.0f/640.0f;
        ddim.y = 160.0f/480.0f;
        pli = pw->CreateList(pos, ddim, 0, EVENT_INTERFACE_NLIST);
        pli->SetState(STATE_SHADOW);

        if ( m_bDeleteGamer )
        {
            pos.x = 200.0f/640.0f;
            pos.y = 100.0f/480.0f;
            ddim.x = 160.0f/640.0f;
            ddim.y =  32.0f/480.0f;
            pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_NDELETE);
            pb->SetState(STATE_SHADOW);
        }

        pos.x = 380.0f/640.0f;
        pos.y = 100.0f/480.0f;
        ddim.x =100.0f/640.0f;
        ddim.y = 32.0f/480.0f;
        pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_NCANCEL);
        pb->SetState(STATE_SHADOW);

        ReadNameList();
        UpdateNameList();
        UpdateNameControl();

        m_engine->SetBackground("interface/interface.png",
                Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
                Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
                Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
                Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
                true);
        m_engine->SetBackForce(true);
    }

    if ( m_phase == PHASE_APPERANCE )
    {
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
        pb = pw->CreateButton(pos, ddim, 46, EVENT_INTERFACE_PFACE4);
        pb->SetState(STATE_SHADOW);
        pos.x += 50.0f/640.0f;
        pb = pw->CreateButton(pos, ddim, 45, EVENT_INTERFACE_PFACE3);
        pb->SetState(STATE_SHADOW);
        pos.x += 50.0f/640.0f;
        pb = pw->CreateButton(pos, ddim, 44, EVENT_INTERFACE_PFACE2);
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
        for ( i=0 ; i<6 ; i++ )
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
        for ( j=0 ; j<3 ; j++ )
        {
            pos.x  = 340.0f/640.0f;
            for ( i=0 ; i<3 ; i++ )
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
        for ( i=0 ; i<3 ; i++ )
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
        for ( j=0 ; j<3 ; j++ )
        {
            pos.x  = 340.0f/640.0f;
            for ( i=0 ; i<3 ; i++ )
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
        for ( i=0 ; i<3 ; i++ )
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

    if ( m_phase != PHASE_SIMUL   &&
         m_phase != PHASE_WIN     &&
         m_phase != PHASE_LOST    &&
         m_phase != PHASE_WRITE   &&
         m_phase != PHASE_READs   &&
         m_phase != PHASE_WRITEs  &&
         m_phase != PHASE_SETUPds &&
         m_phase != PHASE_SETUPgs &&
         m_phase != PHASE_SETUPps &&
         m_phase != PHASE_SETUPcs &&
         m_phase != PHASE_SETUPss )
    {
        if (!m_sound->IsPlayingMusic() && m_sound->IsCachedMusic("Intro1.ogg"))
        {
            m_sound->PlayMusic("Intro1.ogg", false);
        }
    }

    if ( m_phase == PHASE_LEVEL_LIST )
    {
        if ( static_cast<int>(m_category) >= static_cast<int>(LevelCategory::Max) )
        {
            m_category = m_listCategory;
        }
        else
        {
            m_listCategory = m_category;
        }

        if ( m_category == LevelCategory::FreeGame )
        {
            m_accessChap = m_main->GetPlayerProfile()->GetChapPassed(LevelCategory::Missions);
        }

        pos.x = 0.10f;
        pos.y = 0.10f;
        ddim.x = 0.80f;
        ddim.y = 0.80f;
        pw = m_interface->CreateWindows(pos, ddim, 12, EVENT_WINDOW5);
        pw->SetClosable(true);
        if ( m_category == LevelCategory::Exercises    )  res = RT_TITLE_TRAINER;
        if ( m_category == LevelCategory::Challenges   )  res = RT_TITLE_DEFI;
        if ( m_category == LevelCategory::Missions     )  res = RT_TITLE_MISSION;
        if ( m_category == LevelCategory::FreeGame     )  res = RT_TITLE_FREE;
        if ( m_category == LevelCategory::CustomLevels )  res = RT_TITLE_USER;
        GetResource(RES_TEXT, res, name);
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

        // Displays a list of chapters:
        pos.x = ox+sx*3;
        pos.y = oy+sy*10.5f;
        ddim.x = dim.x*7.5f;
        ddim.y = dim.y*0.6f;
        if ( m_category == LevelCategory::Exercises    )  res = RT_PLAY_CHAPt;
        if ( m_category == LevelCategory::Challenges   )  res = RT_PLAY_CHAPd;
        if ( m_category == LevelCategory::Missions     )  res = RT_PLAY_CHAPm;
        if ( m_category == LevelCategory::FreeGame     )  res = RT_PLAY_CHAPf;
        if ( m_category == LevelCategory::CustomLevels )  res = RT_PLAY_CHAPu;
        GetResource(RES_TEXT, res, name);
        pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL11, name);
        pl->SetTextAlign(Gfx::TEXT_ALIGN_LEFT);

        pos.y = oy+sy*6.7f;
        ddim.y = dim.y*4.5f;
        ddim.x = dim.x*6.5f;
        pli = pw->CreateList(pos, ddim, 0, EVENT_INTERFACE_CHAP);
        pli->SetState(STATE_SHADOW);
        m_chap[m_category] = m_main->GetPlayerProfile()->GetSelectedChap(m_category)-1;
        UpdateSceneChap(m_chap[m_category]);
        if ( m_category != LevelCategory::CustomLevels )  pli->SetState(STATE_EXTEND);

        // Displays a list of missions:
        pos.x = ox+sx*9.5f;
        pos.y = oy+sy*10.5f;
        ddim.x = dim.x*7.5f;
        ddim.y = dim.y*0.6f;
        if ( m_category == LevelCategory::Exercises    )  res = RT_PLAY_LISTt;
        if ( m_category == LevelCategory::Challenges   )  res = RT_PLAY_LISTd;
        if ( m_category == LevelCategory::Missions     )  res = RT_PLAY_LISTm;
        if ( m_category == LevelCategory::FreeGame     )  res = RT_PLAY_LISTf;
        if ( m_category == LevelCategory::CustomLevels )  res = RT_PLAY_LISTu;
        GetResource(RES_TEXT, res, name);
        pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL12, name);
        pl->SetTextAlign(Gfx::TEXT_ALIGN_LEFT);

        pos.y = oy+sy*6.7f;
        ddim.y = dim.y*4.5f;
        ddim.x = dim.x*6.5f;
        pli = pw->CreateList(pos, ddim, 0, EVENT_INTERFACE_LIST);
        pli->SetState(STATE_SHADOW);
        m_sel[m_category] = m_main->GetPlayerProfile()->GetSelectedRank(m_category)-1;
        UpdateSceneList(m_chap[m_category], m_sel[m_category]);
        if ( m_category != LevelCategory::CustomLevels )  pli->SetState(STATE_EXTEND);
        pos = pli->GetPos();
        ddim = pli->GetDim();

        // Displays the summary:
        pos.x = ox+sx*3;
        pos.y = oy+sy*5.4f;
        ddim.x = dim.x*6.5f;
        ddim.y = dim.y*0.6f;
        GetResource(RES_TEXT, RT_PLAY_RESUME, name);
        pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL13, name);
        pl->SetTextAlign(Gfx::TEXT_ALIGN_LEFT);

        pos.x = ox+sx*3;
        pos.y = oy+sy*3.6f;
        ddim.x = dim.x*13.4f;
        ddim.y = dim.y*1.9f;
        pe = pw->CreateEdit(pos, ddim, 0, EVENT_INTERFACE_RESUME);
        pe->SetState(STATE_SHADOW);
        pe->SetMaxChar(500);
        pe->SetEditCap(false);  // just to see
        pe->SetHighlightCap(false);

        // Button displays the "soluce":
        if ( m_category != LevelCategory::Exercises &&
             m_category != LevelCategory::FreeGame   )
        {
            pos.x = ox+sx*9.5f;
            pos.y = oy+sy*5.8f;
            ddim.x = dim.x*6.5f;
            ddim.y = dim.y*0.5f;
            pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_SOLUCE);
            pc->SetState(STATE_SHADOW);
            pc->ClearState(STATE_CHECK);
        }
        m_bSceneSoluce = false;

        UpdateSceneResume(m_chap[m_category]+1, m_sel[m_category]+1);

        if ( m_category == LevelCategory::Missions    ||
             m_category == LevelCategory::FreeGame    ||
             m_category == LevelCategory::CustomLevels )
        {
            pos.x = ox+sx*9.5f;
            pos.y = oy+sy*2;
            ddim.x = dim.x*3.7f;
            ddim.y = dim.y*1;
            pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_PLAY);
            pb->SetState(STATE_SHADOW);
            if ( m_maxList == 0 )
            {
                pb->ClearState(STATE_ENABLE);
            }

            pos.x += dim.x*4.0f;
            ddim.x = dim.x*2.5f;
            pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_READ);
            pb->SetState(STATE_SHADOW);
            if ( !IsIOReadScene() )  // no file to read?
            {
                pb->ClearState(STATE_ENABLE);
            }
        }
        else
        {
            pos.x = ox+sx*9.5f;
            pos.y = oy+sy*2;
            ddim.x = dim.x*6.5f;
            ddim.y = dim.y*1;
            pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_PLAY);
            pb->SetState(STATE_SHADOW);
            if ( m_maxList == 0 )
            {
                pb->ClearState(STATE_ENABLE);
            }
        }

        pos.x = ox+sx*3;
        ddim.x = dim.x*4;
        pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_BACK);
        pb->SetState(STATE_SHADOW);

        m_engine->SetBackground("interface/interface.png",
                Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
                Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
                Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
                Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
                true);
        m_engine->SetBackForce(true);
    }

    if ( m_phase == PHASE_SETUPd  ||
            m_phase == PHASE_SETUPg  ||
            m_phase == PHASE_SETUPp  ||
            m_phase == PHASE_SETUPc  ||
            m_phase == PHASE_SETUPs  ||
            m_phase == PHASE_SETUPds ||
            m_phase == PHASE_SETUPgs ||
            m_phase == PHASE_SETUPps ||
            m_phase == PHASE_SETUPcs ||
            m_phase == PHASE_SETUPss )
    {
        if ( m_phase == PHASE_SETUPds )
        {
            m_phaseSetup = PHASE_SETUPd;
            m_bSimulSetup = true;
        }
        else if ( m_phase == PHASE_SETUPgs )
        {
            m_phaseSetup = PHASE_SETUPg;
            m_bSimulSetup = true;
        }
        else if ( m_phase == PHASE_SETUPps )
        {
            m_phaseSetup = PHASE_SETUPp;
            m_bSimulSetup = true;
        }
        else if ( m_phase == PHASE_SETUPcs )
        {
            m_phaseSetup = PHASE_SETUPc;
            m_bSimulSetup = true;
        }
        else if ( m_phase == PHASE_SETUPss )
        {
            m_phaseSetup = PHASE_SETUPs;
            m_bSimulSetup = true;
        }
        else
        {
            m_phaseSetup = m_phase;
            m_bSimulSetup = false;
        }

        pos.x = 0.10f;
        pos.y = 0.10f;
        ddim.x = 0.80f;
        ddim.y = 0.80f;
        pw = m_interface->CreateWindows(pos, ddim, 12, EVENT_WINDOW5);
        pw->SetClosable(true);
        GetResource(RES_TEXT, RT_TITLE_SETUP, name);
        pw->SetName(name);

        pos.x  = 0.70f;
        pos.y  = 0.10f;
        ddim.x = 0.20f;
        ddim.y = 0.20f;
        pw->CreateGroup(pos, ddim, 4, EVENT_INTERFACE_GLINTr);  // blue corner

        pos.x  = 0.10f;
        ddim.x = 0.80f;
        pos.y  = 0.76f;
        ddim.y = 0.05f;
        pw->CreateGroup(pos, ddim, 3, EVENT_NULL);  // transparent -> gray

        ddim.x = 0.78f/5-0.01f;
        ddim.y = 0.06f;
        pos.x = 0.115f;
        pos.y = 0.76f;
        pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_SETUPd);
        pb->SetState(STATE_SHADOW);
        pb->SetState(STATE_CARD);
        pb->SetState(STATE_CHECK, (m_phase == PHASE_SETUPd || m_phase == PHASE_SETUPds));
        #if PLATFORM_WINDOWS
        pb->SetState(STATE_ENABLE, !m_bSimulSetup);
        #endif

        pos.x += ddim.x+0.01f;
        pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_SETUPg);
        pb->SetState(STATE_SHADOW);
        pb->SetState(STATE_CARD);
        pb->SetState(STATE_CHECK, (m_phase == PHASE_SETUPg || m_phase == PHASE_SETUPgs));

        pos.x += ddim.x+0.01f;
        pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_SETUPp);
        pb->SetState(STATE_SHADOW);
        pb->SetState(STATE_CARD);
        pb->SetState(STATE_CHECK, (m_phase == PHASE_SETUPp || m_phase == PHASE_SETUPps));

        pos.x += ddim.x+0.01f;
        pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_SETUPc);
        pb->SetState(STATE_SHADOW);
        pb->SetState(STATE_CARD);
        pb->SetState(STATE_CHECK, (m_phase == PHASE_SETUPc || m_phase == PHASE_SETUPcs));

        pos.x += ddim.x+0.01f;
        pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_SETUPs);
        pb->SetState(STATE_SHADOW);
        pb->SetState(STATE_CARD);
        pb->SetState(STATE_CHECK, (m_phase == PHASE_SETUPs || m_phase == PHASE_SETUPss));

        pos.x  = 0.10f;
        ddim.x = 0.80f;
        pos.y  = 0.34f;
        ddim.y = 0.42f;
        pw->CreateGroup(pos, ddim, 2, EVENT_INTERFACE_GLINTu);  // orange -> transparent
        pos.x  = 0.10f+(6.0f/640.0f);
        ddim.x = 0.80f-(11.0f/640.0f);
        pos.y  = 0.74f;
        ddim.y = 0.02f;
        pw->CreateGroup(pos, ddim, 1, EVENT_INTERFACE_GLINTb);  // orange bar

        ddim.x = dim.x*4;
        ddim.y = dim.y*1;
        pos.x = ox+sx*3;
        pos.y = oy+sy*2;
        pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_BACK);
        pb->SetState(STATE_SHADOW);

        if ( !m_bSimulSetup )
        {
            m_engine->SetBackground("interface/interface.png",
                    Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
                    Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
                    Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
                    Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
                    true);
            m_engine->SetBackForce(true);
        }
    }

    if ( m_phase == PHASE_SETUPd  || // setup/display ?
            m_phase == PHASE_SETUPds )
    {
        pos.x = ox+sx*3;
        pos.y = oy+sy*9;
        ddim.x = dim.x*6;
        ddim.y = dim.y*1;
        GetResource(RES_TEXT, RT_SETUP_MODE, name);
        pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL2, name);
        pl->SetTextAlign(Gfx::TEXT_ALIGN_LEFT);

        m_setupFull = m_app->GetVideoConfig().fullScreen;
        pos.x = ox+sx*3;
        pos.y = oy+sy*5.2f;
        ddim.x = dim.x*6;
        ddim.y = dim.y*4.5f;
        pli = pw->CreateList(pos, ddim, 0, EVENT_LIST2);
        pli->SetState(STATE_SHADOW);
        UpdateDisplayMode();

        ddim.x = dim.x*4;
        ddim.y = dim.y*0.5f;
        pos.x = ox+sx*3;
        pos.y = oy+sy*4.1f;
        pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_FULL);
        pc->SetState(STATE_SHADOW);
        pc->SetState(STATE_CHECK, m_setupFull);

        #if !PLATFORM_LINUX
        ddim.x = 0.9f;
        ddim.y = 0.1f;
        pos.x = 0.05f;
        pos.y = 0.20f;
        pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL1, "The game will be restarted in order to apply changes. All unsaved progress will be lost.");
        #endif

        ddim.x = dim.x*6;
        ddim.y = dim.y*1;
        pos.x = ox+sx*10;
        pos.y = oy+sy*2;
        pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_APPLY);
        pb->SetState(STATE_SHADOW);
        UpdateApply();
    }

    if ( m_phase == PHASE_SETUPg  ||  // setup/graphic ?
            m_phase == PHASE_SETUPgs )
    {
        ddim.x = dim.x*6;
        ddim.y = dim.y*0.5f;
        pos.x = ox+sx*3;
        pos.y = 0.65f;
        pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_SHADOW);
        pc->SetState(STATE_SHADOW);
        pos.y -= 0.048f;
        pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_GROUND);
        pc->SetState(STATE_SHADOW);
        if ( m_bSimulSetup )
        {
            pc->SetState(STATE_DEAD);
        }
        pos.y -= 0.048f;
        pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_DIRTY);
        pc->SetState(STATE_SHADOW);
        pos.y -= 0.048f;
        pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_SKY);
        pc->SetState(STATE_SHADOW);
        pos.y -= 0.048f;
        pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_LENS);
        pc->SetState(STATE_SHADOW);
        pos.y -= 0.048f;
        pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_PLANET);
        pc->SetState(STATE_SHADOW);
        pos.y -= 0.048f;
        pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_FOG);
        pc->SetState(STATE_SHADOW);
        pos.y -= 0.048f;
        pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_LIGHT);
        pc->SetState(STATE_SHADOW);
        if ( m_bSimulSetup )
        {
            pc->SetState(STATE_DEAD);
        }

        pos.x = ox+sx*3;
        pos.y = 0.245f;
        ddim.x = dim.x*2.2f;
        ddim.y = 18.0f/480.0f;
        pes = pw->CreateEnumSlider(pos, ddim, 0, EVENT_INTERFACE_MSAA);
        pes->SetState(STATE_SHADOW);
        std::vector<float> msaaOptions;
        for(int i = 1; i <= m_engine->GetDevice()->GetMaxSamples(); i *= 2)
            msaaOptions.push_back(i);
        pes->SetPossibleValues(msaaOptions);
        if(m_engine->GetDevice()->GetMaxSamples() < 2)
            pes->ClearState(STATE_ENABLE);
        pos.y += ddim.y/2;
        pos.x += 0.005f;
        ddim.x = 0.40f;
        GetResource(RES_EVENT, EVENT_INTERFACE_MSAA, name);
        pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL12, name);
        pl->SetTextAlign(Gfx::TEXT_ALIGN_LEFT);

        pos.x = ox+sx*8.5f;
        pos.y = 0.65f;
        ddim.x = dim.x*2.2f;
        ddim.y = 18.0f/480.0f;
        pv = pw->CreateEditValue(pos, ddim, 0, EVENT_INTERFACE_PARTI);
        pv->SetState(STATE_SHADOW);
        pv->SetMinValue(0.0f);
        pv->SetMaxValue(2.0f);
        pos.x += 0.13f;
        pos.y -= 0.015f;
        ddim.x = 0.40f;
        GetResource(RES_EVENT, EVENT_INTERFACE_PARTI, name);
        pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL10, name);
        pl->SetTextAlign(Gfx::TEXT_ALIGN_LEFT);

        pos.x = ox+sx*8.5f;
        pos.y = 0.59f;
        ddim.x = dim.x*2.2f;
        ddim.y = 18.0f/480.0f;
        pv = pw->CreateEditValue(pos, ddim, 0, EVENT_INTERFACE_CLIP);
        pv->SetState(STATE_SHADOW);
        pv->SetMinValue(0.5f);
        pv->SetMaxValue(2.0f);
        pos.x += 0.13f;
        pos.y -= 0.015f;
        ddim.x = 0.40f;
        GetResource(RES_EVENT, EVENT_INTERFACE_CLIP, name);
        pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL11, name);
        pl->SetTextAlign(Gfx::TEXT_ALIGN_LEFT);

        pos.x = ox+sx*8.5f;
        pos.y = 0.53f;
        ddim.x = dim.x*2.2f;
        ddim.y = 18.0f/480.0f;
        pv = pw->CreateEditValue(pos, ddim, 0, EVENT_INTERFACE_DETAIL);
        pv->SetState(STATE_SHADOW);
        pv->SetMinValue(0.0f);
        pv->SetMaxValue(2.0f);
        pos.x += 0.13f;
        pos.y -= 0.015f;
        ddim.x = 0.40f;
        GetResource(RES_EVENT, EVENT_INTERFACE_DETAIL, name);
        pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL12, name);
        pl->SetTextAlign(Gfx::TEXT_ALIGN_LEFT);

        pos.x = ox+sx*8.5f;
        pos.y = 0.47f;
        ddim.x = dim.x*2.2f;
        ddim.y = 18.0f/480.0f;
        pv = pw->CreateEditValue(pos, ddim, 0, EVENT_INTERFACE_GADGET);
        pv->SetState(STATE_SHADOW);
        if ( m_bSimulSetup )
        {
            pv->SetState(STATE_DEAD);
        }
        pv->SetMinValue(0.0f);
        pv->SetMaxValue(1.0f);
        pos.x += 0.13f;
        pos.y -= 0.015f;
        ddim.x = 0.40f;
        GetResource(RES_EVENT, EVENT_INTERFACE_GADGET, name);
        pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL13, name);
        pl->SetTextAlign(Gfx::TEXT_ALIGN_LEFT);

        pos.x = ox+sx*8.5f;
        pos.y = 0.385f;
        ddim.x = dim.x*2.2f;
        ddim.y = 18.0f/480.0f;
        pes = pw->CreateEnumSlider(pos, ddim, 0, EVENT_INTERFACE_TEXTURE_FILTER);
        pes->SetState(STATE_SHADOW);
        pes->SetPossibleValues({
            { Gfx::TEX_FILTER_NEAREST,   "Nearest"   },
            { Gfx::TEX_FILTER_BILINEAR,  "Bilinear"  },
            { Gfx::TEX_FILTER_TRILINEAR, "Trilinear" }
        });
        pos.y += ddim.y/2;
        pos.x += 0.005f;
        ddim.x = 0.40f;
        GetResource(RES_EVENT, EVENT_INTERFACE_TEXTURE_FILTER, name);
        pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL12, name);
        pl->SetTextAlign(Gfx::TEXT_ALIGN_LEFT);

        pos.x = ox+sx*8.5f;
        pos.y = 0.315f;
        ddim.x = dim.x*2.2f;
        ddim.y = 18.0f/480.0f;
        pes = pw->CreateEnumSlider(pos, ddim, 0, EVENT_INTERFACE_TEXTURE_MIPMAP);
        pes->SetState(STATE_SHADOW);
        pes->SetPossibleValues({1, 4, 8, 16});
        pos.y += ddim.y/2;
        pos.x += 0.005f;
        ddim.x = 0.40f;
        GetResource(RES_EVENT, EVENT_INTERFACE_TEXTURE_MIPMAP, name);
        pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL12, name);
        pl->SetTextAlign(Gfx::TEXT_ALIGN_LEFT);

        pos.x = ox+sx*8.5f;
        pos.y = 0.245f;
        ddim.x = dim.x*2.2f;
        ddim.y = 18.0f/480.0f;
        pes = pw->CreateEnumSlider(pos, ddim, 0, EVENT_INTERFACE_TEXTURE_ANISOTROPY);
        pes->SetState(STATE_SHADOW);
        std::vector<float> anisotropyOptions;
        for(int i = 1; i <= m_engine->GetDevice()->GetMaxAnisotropyLevel(); i *= 2)
            anisotropyOptions.push_back(i);
        pes->SetPossibleValues(anisotropyOptions);
        if(!m_engine->GetDevice()->IsAnisotropySupported())
            pes->ClearState(STATE_ENABLE);
        pos.y += ddim.y/2;
        pos.x += 0.005f;
        ddim.x = 0.40f;
        GetResource(RES_EVENT, EVENT_INTERFACE_TEXTURE_ANISOTROPY, name);
        pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL12, name);
        pl->SetTextAlign(Gfx::TEXT_ALIGN_LEFT);


        pos.x = ox+sx*12.5;
        pos.y = 0.385f;
        ddim.x = dim.x*2.2f;
        ddim.y = 18.0f/480.0f;
        pes = pw->CreateEnumSlider(pos, ddim, 0, EVENT_INTERFACE_SHADOW_MAPPING);
        pes->SetState(STATE_SHADOW);
        std::map<float, std::string> shadowOptions = {
            { -1, "Disabled" },
        };
        if (m_engine->GetDevice()->IsFramebufferSupported())
        {
            for(int i = 128; i <= m_engine->GetDevice()->GetMaxTextureSize(); i *= 2)
                shadowOptions[i] = StrUtils::ToString<int>(i)+"x"+StrUtils::ToString<int>(i);
        }
        else
        {
            shadowOptions[0] = "Screen buffer"; // TODO: Is this the proper name for this?
        }
        pes->SetPossibleValues(shadowOptions);
        if (!m_engine->GetDevice()->IsShadowMappingSupported())
        {
            pes->ClearState(STATE_ENABLE);
        }
        pos.y += ddim.y/2;
        pos.x += 0.005f;
        ddim.x = 0.40f;
        GetResource(RES_EVENT, EVENT_INTERFACE_SHADOW_MAPPING, name);
        pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL12, name);
        pl->SetTextAlign(Gfx::TEXT_ALIGN_LEFT);

        pos.x = ox+sx*12.5;
        pos.y = 0.315f;
        ddim.x = dim.x*6;
        ddim.y = dim.y*0.5f;
        pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_SHADOW_MAPPING_QUALITY);
        pc->SetState(STATE_SHADOW);

        ddim.x = dim.x*2;
        ddim.y = dim.y*1;
        pos.x = ox+sx*10;
        pos.y = oy+sy*2;

        pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_MIN);
        pb->SetState(STATE_SHADOW);
        pos.x += ddim.x;
        pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_NORM);
        pb->SetState(STATE_SHADOW);
        pos.x += ddim.x;
        pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_MAX);
        pb->SetState(STATE_SHADOW);

        UpdateSetupButtons();
    }

    if ( m_phase == PHASE_SETUPp  ||  // setup/game ?
            m_phase == PHASE_SETUPps )
    {
        ddim.x = dim.x*6;
        ddim.y = dim.y*0.5f;
        pos.x = ox+sx*3;
        pos.y = 0.65f;
        //?     pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_TOTO);
        //?     pc->SetState(STATE_SHADOW);
        //?     pos.y -= 0.048f;

        pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_MOVIES);
        pc->SetState(STATE_SHADOW);
        pos.y -= 0.048f;
        //#endif
        pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_SCROLL);
        pc->SetState(STATE_SHADOW);
        pos.y -= 0.048f;
        pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_INVERTX);
        pc->SetState(STATE_SHADOW);
        pos.y -= 0.048f;
        pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_INVERTY);
        pc->SetState(STATE_SHADOW);
        pos.y -= 0.048f;
        pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_EFFECT);
        pc->SetState(STATE_SHADOW);
        pos.y -= 0.048f;
        pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_BLOOD);
        pc->SetState(STATE_SHADOW);
        pos.y -= 0.048f;
        pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_AUTOSAVE_ENABLE);
        pc->SetState(STATE_SHADOW);
        pos.y -= 0.048f;

        pos.y -= ddim.y;
        ddim.x = dim.x*2.5f;
        psl = pw->CreateSlider(pos, ddim, -1, EVENT_INTERFACE_AUTOSAVE_INTERVAL);
        psl->SetState(STATE_SHADOW);
        psl->SetLimit(1.0f, 30.0f);
        psl->SetArrowStep(1.0f);
        pos.y += ddim.y/2;
        GetResource(RES_EVENT, EVENT_INTERFACE_AUTOSAVE_INTERVAL, name);
        pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL1, name);
        pl->SetTextAlign(Gfx::TEXT_ALIGN_LEFT);
        pos.y -= ddim.y/2;
        pos.x = ox+sx*3+dim.x*3.5f;
        psl = pw->CreateSlider(pos, ddim, -1, EVENT_INTERFACE_AUTOSAVE_SLOTS);
        psl->SetState(STATE_SHADOW);
        psl->SetLimit(1.0f, 10.0f);
        psl->SetArrowStep(1.0f);
        pos.y += ddim.y/2;
        GetResource(RES_EVENT, EVENT_INTERFACE_AUTOSAVE_SLOTS, name);
        pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL1, name);
        pl->SetTextAlign(Gfx::TEXT_ALIGN_LEFT);
        pos.y -= ddim.y/2;


        //?     pos.y -= 0.048f;
        //?     pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_NICERST);
        //?     pc->SetState(STATE_SHADOW);
        //?     pos.y -= 0.048f;
        //?     pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_HIMSELF);
        //?     pc->SetState(STATE_SHADOW);

        ddim.x = dim.x*6;
        ddim.y = dim.y*0.5f;
        pos.x = ox+sx*10;
        pos.y = 0.65f;
        pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_TOOLTIP);
        pc->SetState(STATE_SHADOW);
        pos.y -= 0.048f;
        pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_GLINT);
        pc->SetState(STATE_SHADOW);
        pos.y -= 0.048f;
        pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_RAIN);
        pc->SetState(STATE_SHADOW);
        pos.y -= 0.048f;
        pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_MOUSE);
        pc->SetState(STATE_SHADOW);
        pos.y -= 0.048f;
        pos.y -= 0.048f;
        pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_EDITMODE);
        pc->SetState(STATE_SHADOW);
        if ( m_bSimulSetup )
        {
            pc->SetState(STATE_DEAD);
        }
        pos.y -= 0.048f;
        pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_EDITVALUE);
        pc->SetState(STATE_SHADOW);

        UpdateSetupButtons();
    }

    if ( m_phase == PHASE_SETUPc  ||  // setup/commands ?
            m_phase == PHASE_SETUPcs )
    {
        pos.x = ox+sx*3;
        pos.y = 320.0f/480.0f;
        ddim.x = dim.x*15.0f;
        ddim.y = 18.0f/480.0f;
        GetResource(RES_TEXT, RT_SETUP_KEY1, name);
        pl = pw->CreateLabel(pos, ddim, 0, EVENT_INTERFACE_KINFO1, name);
        pl->SetTextAlign(Gfx::TEXT_ALIGN_LEFT);

        pos.x = ox+sx*3;
        pos.y = 302.0f/480.0f;
        ddim.x = dim.x*15.0f;
        ddim.y = 18.0f/480.0f;
        GetResource(RES_TEXT, RT_SETUP_KEY2, name);
        pl = pw->CreateLabel(pos, ddim, 0, EVENT_INTERFACE_KINFO2, name);
        pl->SetTextAlign(Gfx::TEXT_ALIGN_LEFT);

        ddim.x = 428.0f/640.0f;
        ddim.y = 128.0f/480.0f;
        pos.x = 105.0f/640.0f;
        pos.y = 164.0f/480.0f;
        pg = pw->CreateGroup(pos, ddim, 0, EVENT_INTERFACE_KGROUP);
        pg->ClearState(STATE_ENABLE);
        pg->SetState(STATE_DEAD);
        pg->SetState(STATE_SHADOW);

        ddim.x =  18.0f/640.0f;
        ddim.y = (20.0f/480.0f)*KEY_VISIBLE;
        pos.x = 510.0f/640.0f;
        pos.y = 168.0f/480.0f;
        ps = pw->CreateScroll(pos, ddim, -1, EVENT_INTERFACE_KSCROLL);
        ps->SetVisibleRatio(static_cast<float>(KEY_VISIBLE/INPUT_SLOT_MAX));
        ps->SetArrowStep(1.0f/(static_cast<float>(INPUT_SLOT_MAX-KEY_VISIBLE)));
        UpdateKey();

        ddim.x = dim.x*6;
        ddim.y = dim.y*0.5f;
        pos.x = ox+sx*3;
        pos.y = 130.0f/480.0f;
        pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_JOYSTICK);
        pc->SetState(STATE_SHADOW);

        ddim.x = dim.x*6;
        ddim.y = dim.y*1;
        pos.x = ox+sx*10;
        pos.y = oy+sy*2;
        pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_KDEF);
        pb->SetState(STATE_SHADOW);

        UpdateSetupButtons();
    }

    if ( m_phase == PHASE_SETUPs  ||  // setup/sound ?
            m_phase == PHASE_SETUPss )
    {
        pos.x = ox+sx*3;
        pos.y = 0.55f;
        ddim.x = dim.x*4.0f;
        ddim.y = 18.0f/480.0f;
        psl = pw->CreateSlider(pos, ddim, 0, EVENT_INTERFACE_VOLSOUND);
        psl->SetState(STATE_SHADOW);
        psl->SetLimit(0.0f, MAXVOLUME);
        psl->SetArrowStep(1.0f);
        pos.y += ddim.y;
        GetResource(RES_EVENT, EVENT_INTERFACE_VOLSOUND, name);
        pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL1, name);
        pl->SetTextAlign(Gfx::TEXT_ALIGN_LEFT);

        pos.x = ox+sx*3;
        pos.y = 0.40f;
        ddim.x = dim.x*4.0f;
        ddim.y = 18.0f/480.0f;
        psl = pw->CreateSlider(pos, ddim, 0, EVENT_INTERFACE_VOLMUSIC);
        psl->SetState(STATE_SHADOW);
        psl->SetLimit(0.0f, MAXVOLUME);
        psl->SetArrowStep(1.0f);
        pos.y += ddim.y;
        GetResource(RES_EVENT, EVENT_INTERFACE_VOLMUSIC, name);
        pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL2, name);
        pl->SetTextAlign(Gfx::TEXT_ALIGN_LEFT);

        ddim.x = dim.x*3;
        ddim.y = dim.y*1;
        pos.x = ox+sx*10;
        pos.y = oy+sy*2;
        pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_SILENT);
        pb->SetState(STATE_SHADOW);
        pos.x += ddim.x;
        pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_NOISY);
        pb->SetState(STATE_SHADOW);

        UpdateSetupButtons();
    }

    if ( m_phase == PHASE_WRITE  ||
            m_phase == PHASE_WRITEs )
    {
        pos.x = 0.10f;
        pos.y = 0.10f;
        ddim.x = 0.80f;
        ddim.y = 0.80f;
        pw = m_interface->CreateWindows(pos, ddim, 13, EVENT_WINDOW5);
        pw->SetClosable(true);
        GetResource(RES_TEXT, RT_TITLE_WRITE, name);
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

        pos.x  = 290.0f/640.0f;
        ddim.x = 245.0f/640.0f;

        pos.y  = 146.0f/480.0f;
        ddim.y =  18.0f/480.0f;
        GetResource(RES_EVENT, EVENT_INTERFACE_IOLABEL, name);
        pl = pw->CreateLabel(pos, ddim, 0, EVENT_INTERFACE_IOLABEL, name);
        pl->SetTextAlign(Gfx::TEXT_ALIGN_LEFT);

        pos.y  = 130.0f/480.0f;
        ddim.y =  18.0f/480.0f;
        pe = pw->CreateEdit(pos, ddim, 0, EVENT_INTERFACE_IONAME);
        pe->SetState(STATE_SHADOW);
        pe->SetFontType(Gfx::FONT_COLOBOT);
        pe->SetMaxChar(35);
        IOReadName();

        pos.y  = 190.0f/480.0f;
        ddim.y = 190.0f/480.0f;
        pli = pw->CreateList(pos, ddim, 0, EVENT_INTERFACE_IOLIST);
        pli->SetState(STATE_SHADOW);

        pos.y  = oy+sy*2;
        ddim.y = dim.y*1;
        pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_IOWRITE);
        pb->SetState(STATE_SHADOW);

        pos.x  = 105.0f/640.0f;
        pos.y  = 190.0f/480.0f;
        ddim.x = 170.0f/640.0f;
        ddim.y = dim.y*1;
        pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_IODELETE);
        pb->SetState(STATE_SHADOW);

        pos.x  = 105.0f/640.0f;
        pos.y  = 250.0f/480.0f;
        ddim.x = 170.0f/640.0f;
        ddim.y = 128.0f/480.0f;
        pi = pw->CreateImage(pos, ddim, 0, EVENT_INTERFACE_IOIMAGE);
        pi->SetState(STATE_SHADOW);

        ddim.x = dim.x*4;
        ddim.y = dim.y*1;
        pos.x  = ox+sx*3;
        pos.y  = oy+sy*2;
        pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_BACK);
        pb->SetState(STATE_SHADOW);

        IOReadList();
        IOUpdateList();
    }

    if ( m_phase == PHASE_READ  ||
         m_phase == PHASE_READs )
    {
        pos.x = 0.10f;
        pos.y = 0.10f;
        ddim.x = 0.80f;
        ddim.y = 0.80f;
        pw = m_interface->CreateWindows(pos, ddim, 14, EVENT_WINDOW5);
        pw->SetClosable(true);
        GetResource(RES_TEXT, RT_TITLE_READ, name);
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

        pos.x  = 290.0f/640.0f;
        ddim.x = 245.0f/640.0f;

        pos.y  = 160.0f/480.0f;
        ddim.y = 190.0f/480.0f;
        pli = pw->CreateList(pos, ddim, 0, EVENT_INTERFACE_IOLIST);
        pli->SetState(STATE_SHADOW);

        pos.y  = oy+sy*2;
        ddim.y = dim.y*1;
        pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_IOREAD);
        pb->SetState(STATE_SHADOW);
        if ( m_phase == PHASE_READs )
        {
            pb->SetState(STATE_WARNING);
        }

        pos.x  = 105.0f/640.0f;
        pos.y  = 160.0f/480.0f;
        ddim.x = 170.0f/640.0f;
        ddim.y = dim.y*1;
        pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_IODELETE);
        pb->SetState(STATE_SHADOW);

        pos.x  = 105.0f/640.0f;
        pos.y  = 220.0f/480.0f;
        ddim.x = 170.0f/640.0f;
        ddim.y = 128.0f/480.0f;
        pi = pw->CreateImage(pos, ddim, 0, EVENT_INTERFACE_IOIMAGE);
        pi->SetState(STATE_SHADOW);

        ddim.x = dim.x*4;
        ddim.y = dim.y*1;
        pos.x  = ox+sx*3;
        pos.y  = oy+sy*2;
        pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_BACK);
        pb->SetState(STATE_SHADOW);

        IOReadList();
        IOUpdateList();

        if ( m_phase == PHASE_READ )
        {
            m_engine->SetBackground("interface/interface.png",
                    Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
                    Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
                    Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
                    Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
                    true);
            m_engine->SetBackForce(true);
        }
    }

    if ( m_phase == PHASE_LOADING )
    {
        pos.x  = 0.35f;
        pos.y  = 0.10f;
        ddim.x = 0.30f;
        ddim.y = 0.80f;
        pw = m_interface->CreateWindows(pos, ddim, 10, EVENT_WINDOW5);

        pw->SetName(" ");

        pos.x  = 0.35f;
        pos.y  = 0.60f;
        ddim.x = 0.30f;
        ddim.y = 0.30f;
        pw->CreateGroup(pos, ddim, 5, EVENT_INTERFACE_GLINTl);  // orange corner
        pos.x  = 0.35f;
        pos.y  = 0.10f;
        ddim.x = 0.30f;
        ddim.y = 0.30f;
        pw->CreateGroup(pos, ddim, 4, EVENT_INTERFACE_GLINTr);  // blue corner

        pos.x  = 254.0f/640.0f;
        pos.y  = 208.0f/480.0f;
        ddim.x = 132.0f/640.0f;
        ddim.y =  42.0f/480.0f;
        pg = pw->CreateGroup(pos, ddim, 22, EVENT_NULL);
        pg->SetState(STATE_SHADOW);

        pos.x  = 220.0f/640.0f;
        pos.y  = 210.0f/480.0f;
        ddim.x = 200.0f/640.0f;
        ddim.y =  20.0f/480.0f;
        GetResource(RES_TEXT, RT_DIALOG_LOADING, name);
        pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL1, name);
        pl->SetFontSize(12.0f);
        pl->SetTextAlign(Gfx::TEXT_ALIGN_CENTER);

        m_engine->SetBackground("interface/interface.png",
                Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
                Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
                Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
                Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
                true);
        m_engine->SetBackForce(true);

        m_loadingCounter = 1;  // enough time to display!
    }

    if ( m_phase == PHASE_WELCOME1 )
    {
        pos.x  = 0.0f;
        pos.y  = 0.0f;
        ddim.x = 0.0f;
        ddim.y = 0.0f;
        pw = m_interface->CreateWindows(pos, ddim, -1, EVENT_WINDOW5);

        m_engine->SetOverColor(Gfx::Color(1.0f, 1.0f, 1.0f, 1.0f), Gfx::ENG_RSTATE_TCOLOR_BLACK); // TODO: color ok?
        m_engine->SetOverFront(true);

        m_engine->SetBackground("interface/intro1.png",
                Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
                Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
                Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
                Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
                true, true);
        m_engine->SetBackForce(true);
    }
    if ( m_phase == PHASE_WELCOME2 )
    {
        pos.x  = 0.0f;
        pos.y  = 0.0f;
        ddim.x = 0.0f;
        ddim.y = 0.0f;
        pw = m_interface->CreateWindows(pos, ddim, -1, EVENT_WINDOW5);

        m_engine->SetOverColor(Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f), Gfx::ENG_RSTATE_TCOLOR_WHITE); // TODO: color ok?
        m_engine->SetOverFront(true);

        m_engine->SetBackground("interface/intro2.png",
                Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
                Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
                Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
                Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
                true, true);
        m_engine->SetBackForce(true);
    }
    if ( m_phase == PHASE_WELCOME3 )
    {
        pos.x  = 0.0f;
        pos.y  = 0.0f;
        ddim.x = 0.0f;
        ddim.y = 0.0f;
        pw = m_interface->CreateWindows(pos, ddim, -1, EVENT_WINDOW5);

        m_engine->SetOverColor(Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f), Gfx::ENG_RSTATE_TCOLOR_WHITE); // TODO: color ok?
        m_engine->SetOverFront(true);

        m_engine->SetBackground("interface/intro3.png",
                Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
                Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
                Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
                Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
                true, true);
        m_engine->SetBackForce(true);
    }

    if ( m_phase == PHASE_QUIT_SCREEN )
    {
        pos.x  = 0.0f;
        pos.y  = 0.0f;
        ddim.x = 0.0f;
        ddim.y = 0.0f;
        pw = m_interface->CreateWindows(pos, ddim, -1, EVENT_WINDOW5);

        pos.x  =  80.0f/640.0f;
        pos.y  = 190.0f/480.0f;
        ddim.x = 490.0f/640.0f;
        ddim.y = 160.0f/480.0f;
        pe = pw->CreateEdit(pos, ddim, 0, EVENT_EDIT1);
        pe->SetGenericMode(true);
        pe->SetEditCap(false);
        pe->SetHighlightCap(false);
        pe->SetFontType(Gfx::FONT_COURIER);
        pe->SetFontSize(Gfx::FONT_SIZE_SMALL);
        pe->ReadText(std::string("help/") + m_app->GetLanguageChar() + std::string("/authors.txt"));

        pos.x  =  40.0f/640.0f;
        pos.y  =  83.0f/480.0f;
        ddim.x = 246.0f/640.0f;
        ddim.y =  16.0f/480.0f;
        GetResource(RES_TEXT, RT_GENERIC_DEV1, name);
        pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL1, name);
        pl->SetFontType(Gfx::FONT_COURIER);
        pl->SetFontSize(Gfx::FONT_SIZE_SMALL);

        pos.y  =  13.0f/480.0f;
        GetResource(RES_TEXT, RT_GENERIC_DEV2, name);
        pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL2, name);
        pl->SetFontType(Gfx::FONT_COURIER);
        pl->SetFontSize(Gfx::FONT_SIZE_SMALL);

        pos.x  = 355.0f/640.0f;
        pos.y  =  83.0f/480.0f;
        ddim.x = 246.0f/640.0f;
        ddim.y =  16.0f/480.0f;
        GetResource(RES_TEXT, RT_GENERIC_EDIT1, name);
        pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL3, name);
        pl->SetFontType(Gfx::FONT_COURIER);
        pl->SetFontSize(Gfx::FONT_SIZE_SMALL);

        pos.y  =  13.0f/480.0f;
        GetResource(RES_TEXT, RT_GENERIC_EDIT2, name);
        pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL4, name);
        pl->SetFontType(Gfx::FONT_COURIER);
        pl->SetFontSize(Gfx::FONT_SIZE_SMALL);

        pos.x  = 306.0f/640.0f;
        pos.y  =  17.0f/480.0f;
        ddim.x =  30.0f/640.0f;
        ddim.y =  30.0f/480.0f;
        pb = pw->CreateButton(pos, ddim, 49, EVENT_INTERFACE_ABORT);
        pb->SetState(STATE_SHADOW);

        m_engine->SetBackground("interface/generico.png",
                Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
                Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
                Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
                Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
                true);
        m_engine->SetBackForce(true);
    }

    if ( m_phase == PHASE_MAIN_MENU    ||
            m_phase == PHASE_PLAYER_SELECT    ||
            m_phase == PHASE_LEVEL_LIST ||
            m_phase == PHASE_SETUPd  ||
            m_phase == PHASE_SETUPg  ||
            m_phase == PHASE_SETUPp  ||
            m_phase == PHASE_SETUPc  ||
            m_phase == PHASE_SETUPs  ||
            m_phase == PHASE_READ    ||
            m_phase == PHASE_LOADING )
    {
        pos.x  = 540.0f/640.0f;
        pos.y  =   9.0f/480.0f;
        ddim.x =  90.0f/640.0f;
        ddim.y =  10.0f/480.0f;
        pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL1, COLOBOT_VERSION_DISPLAY);
        pl->SetFontType(Gfx::FONT_COURIER);
        pl->SetFontSize(9.0f);
    }

    m_engine->LoadAllTextures();
}


// Processing an event.
// Returns false if the event has been processed completely.

bool CMainDialog::EventProcess(const Event &event)
{
    CWindow*    pw;
    CList*      pl;
    CButton*    pb;
    CCheck*     pc;
    Event       newEvent;
    float       welcomeLength;

    if ( event.type == EVENT_FRAME )
    {
        m_phaseTime += event.rTime;

        //?     if ( m_phase == PHASE_WELCOME1 )  welcomeLength = WELCOME_LENGTH+2.0f;
        //?     else                              welcomeLength = WELCOME_LENGTH;
        welcomeLength = WELCOME_LENGTH;

        if ( m_phase != PHASE_SIMUL   &&
             m_phase != PHASE_WIN     &&
             m_phase != PHASE_LOST    &&
             m_phase != PHASE_WRITE   &&
             m_phase != PHASE_READs   &&
             m_phase != PHASE_WRITEs  &&
             m_phase != PHASE_SETUPds &&
             m_phase != PHASE_SETUPgs &&
             m_phase != PHASE_SETUPps &&
             m_phase != PHASE_SETUPcs &&
             m_phase != PHASE_SETUPss )
        {
            if (!m_sound->IsPlayingMusic() && m_sound->IsCachedMusic("Intro2.ogg"))
            {
                m_sound->PlayMusic("Intro2.ogg", true);
            }
        }

        if ( m_phase == PHASE_WELCOME1 ||
             m_phase == PHASE_WELCOME2 ||
             m_phase == PHASE_WELCOME3 )
        {
            float   intensity;
            int     mode = Gfx::ENG_RSTATE_TCOLOR_WHITE;

            // 1/4 of display time is animating
            float animatingTime = welcomeLength / 4.0f;

            if ( m_phaseTime <  animatingTime )
            {
                //appearing
                intensity = m_phaseTime / animatingTime;
            }
            else if ( m_phaseTime < welcomeLength - animatingTime )
            {
                //showing
                intensity = 1.0f;
            }
            else
            {
                //hiding
                intensity = (welcomeLength - m_phaseTime) / animatingTime;
            }

            if ( intensity < 0.0f )  intensity = 0.0f;
            if ( intensity > 1.0f )  intensity = 1.0f;

            //white first, others -> black fadding
            if ( (m_phase == PHASE_WELCOME1) && ( m_phaseTime < welcomeLength/2.0f))
            {
                intensity = 1.0f - intensity;
                mode = Gfx::ENG_RSTATE_TCOLOR_BLACK;
            }

            m_engine->SetOverColor(Gfx::Color(intensity, intensity, intensity, intensity), mode); // TODO: color ok?
        }

        if ( m_phase == PHASE_WELCOME1 && m_phaseTime >= welcomeLength )
        {
            m_main->ChangePhase(PHASE_WELCOME2);
            return true;
        }
        if ( m_phase == PHASE_WELCOME2 && m_phaseTime >= welcomeLength )
        {
            m_main->ChangePhase(PHASE_WELCOME3);
            return true;
        }
        if ( m_phase == PHASE_WELCOME3 && m_phaseTime >= welcomeLength )
        {
            m_main->ChangePhase(PHASE_PLAYER_SELECT);
            return true;
        }

        if ( m_shotDelay > 0 && !m_bDialog )  // screenshot done?
        {
            m_shotDelay --;
            if ( m_shotDelay == 0 )
            {
                Math::IntPoint windowSize = m_engine->GetWindowSize();

                m_engine->WriteScreenShot(m_shotName, windowSize.x, windowSize.y);
            }
        }

        if ( m_phase == PHASE_LOADING )
        {
            if ( m_loadingCounter == 0 )
            {
                m_main->ChangePhase(PHASE_SIMUL);
            }
            m_loadingCounter --;
            return false;
        }

        m_glintTime += event.rTime;
        GlintMove();  // moves reflections

        FrameParticle(event.rTime);

        if ( m_bDialog )  // this dialogue?
        {
            FrameDialog(event.rTime);
        }

        return true;
    }

    if ( event.type == EVENT_MOUSE_MOVE )
    {
        m_glintMouse = event.mousePos;
        NiceParticle(event.mousePos, event.mouseButtonsState & MOUSE_BUTTON_LEFT);
    }

    if ( m_bDialog )  // this dialogue?
    {
        m_interface->EventProcess(event);

        if ( event.type == EVENT_DIALOG_OK ||
                (event.type == EVENT_KEY_DOWN && event.key.key == KEY(RETURN) ) )
        {
            StopDialog();
            if ( m_phase == PHASE_PLAYER_SELECT )
            {
                NameDelete();
            }
            if ( m_phase == PHASE_MAIN_MENU )
            {
                //?             m_eventQueue->MakeEvent(newEvent, EVENT_QUIT);
                //?             m_eventQueue->AddEvent(newEvent);
                m_main->ChangePhase(PHASE_QUIT_SCREEN);
            }
            if ( m_phase == PHASE_SIMUL )
            {
                if ( m_bDialogDelete )
                {
                    m_main->DeleteObject();
                }
                else
                {
                    m_main->ChangePhase(PHASE_LEVEL_LIST);
                }
            }
        }
        if ( event.type == EVENT_DIALOG_CANCEL ||
                (event.type == EVENT_KEY_DOWN && event.key.key == KEY(ESCAPE) ) )
        {
            StopDialog();
        }
        if ( event.type == EVENT_INTERFACE_SETUP )
        {
            StopDialog();
            StartSuspend();
            #if PLATFORM_WINDOWS
            if ( m_phaseSetup == PHASE_SETUPd ) m_phaseSetup = PHASE_SETUPg;
            #endif
            if ( m_phaseSetup == PHASE_SETUPd )  ChangePhase(PHASE_SETUPds);
            if ( m_phaseSetup == PHASE_SETUPg )  ChangePhase(PHASE_SETUPgs);
            if ( m_phaseSetup == PHASE_SETUPp )  ChangePhase(PHASE_SETUPps);
            if ( m_phaseSetup == PHASE_SETUPc )  ChangePhase(PHASE_SETUPcs);
            if ( m_phaseSetup == PHASE_SETUPs )  ChangePhase(PHASE_SETUPss);
        }
        if ( event.type == EVENT_INTERFACE_AGAIN )
        {
            StopDialog();
            m_main->ChangePhase(PHASE_LOADING);
        }
        if ( event.type == EVENT_INTERFACE_WRITE )
        {
            StopDialog();
            StartSuspend();
            ChangePhase(PHASE_WRITEs);
        }
        if ( event.type == EVENT_INTERFACE_READ )
        {
            StopDialog();
            StartSuspend();
            ChangePhase(PHASE_READs);
        }

        return false;
    }

    if ( /* m_engine->GetMouseVisible() && TODO: WTF ?! */
            !m_interface->EventProcess(event) )
    {
        return false;
    }

    if ( m_phase == PHASE_MAIN_MENU )
    {
        switch( event.type )
        {
            case EVENT_KEY_DOWN:
                if ( event.key.key == KEY(ESCAPE) )
                {
                    //?                 StartQuit();  // would you leave?
                    m_sound->Play(SOUND_TZOING);
                    m_main->ChangePhase(PHASE_QUIT_SCREEN);
                    return false;
                }
                return true;
                break;

            case EVENT_INTERFACE_QUIT:
                //?             StartQuit();  // would you leave?
                m_sound->Play(SOUND_TZOING);
                m_main->ChangePhase(PHASE_QUIT_SCREEN);
                break;

            case EVENT_INTERFACE_TRAINER:
                m_category = LevelCategory::Exercises;
                m_main->ChangePhase(PHASE_LEVEL_LIST);
                break;

            case EVENT_INTERFACE_DEFI:
                m_category = LevelCategory::Challenges;
                m_main->ChangePhase(PHASE_LEVEL_LIST);
                break;

            case EVENT_INTERFACE_MISSION:
                m_category = LevelCategory::Missions;
                m_main->ChangePhase(PHASE_LEVEL_LIST);
                break;

            case EVENT_INTERFACE_FREE:
                m_category = LevelCategory::FreeGame;
                m_main->ChangePhase(PHASE_LEVEL_LIST);
                break;

            case EVENT_INTERFACE_USER:
                m_category = LevelCategory::CustomLevels;
                m_main->ChangePhase(PHASE_LEVEL_LIST);
                break;

            case EVENT_INTERFACE_SETUP:
                m_main->ChangePhase(m_phaseSetup);
                break;

            case EVENT_INTERFACE_NAME:
                m_main->ChangePhase(PHASE_PLAYER_SELECT);
                break;

            default:
                return true;
        }
        return false;
    }

    if ( m_phase == PHASE_PLAYER_SELECT )
    {
        switch( event.type )
        {
            case EVENT_KEY_DOWN:
                if ( event.key.key == KEY(RETURN) )
                {
                    NameSelect();
                }
                if ( event.key.key == KEY(ESCAPE) )
                {
                    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
                    if ( pw == 0 )  break;
                    pb = static_cast<CButton*>(pw->SearchControl(EVENT_INTERFACE_NCANCEL));
                    if ( pb == 0 )  break;
                    if ( pb->TestState(STATE_ENABLE) )
                    {
                        m_main->ChangePhase(PHASE_MAIN_MENU);
                    }
                }
                break;

            case EVENT_INTERFACE_NEDIT:
                UpdateNameList();
                UpdateNameControl();
                break;

            case EVENT_INTERFACE_NLIST:
                UpdateNameEdit();
                break;

            case EVENT_INTERFACE_NOK:
                NameSelect();
                break;

            case EVENT_INTERFACE_PERSO:
                NameSelect();
                m_main->ChangePhase(PHASE_APPERANCE);
                break;

            case EVENT_INTERFACE_NCANCEL:
                m_main->ChangePhase(PHASE_MAIN_MENU);
                break;

            case EVENT_INTERFACE_NDELETE:
                pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
                if ( pw == 0 )  break;
                pl = static_cast<CList*>(pw->SearchControl(EVENT_INTERFACE_NLIST));
                if ( pl == 0 )  break;
                StartDeleteGame(pl->GetItemName(pl->GetSelect()));
                break;

            default:
                return true;
        }
        return false;
    }

    if ( m_phase == PHASE_APPERANCE )
    {
        PlayerApperance& apperance = m_main->GetPlayerProfile()->GetApperance();
        switch( event.type )
        {
            case EVENT_KEY_DOWN:
                if ( event.key.key == KEY(RETURN) )
                {
                    m_main->ChangePhase(PHASE_MAIN_MENU);
                }
                if ( event.key.key == KEY(ESCAPE) )
                {
                    m_main->ChangePhase(PHASE_PLAYER_SELECT);
                }
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
                break;

            default:
                return true;
        }
        return false;
    }

    if ( m_phase == PHASE_LEVEL_LIST    )
    {
        pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
        if ( pw == 0 )  return false;

        if ( event.type == pw->GetEventTypeClose() ||
                event.type == EVENT_INTERFACE_BACK   ||
                (event.type == EVENT_KEY_DOWN && event.key.key == KEY(ESCAPE)) )
        {
            m_main->ChangePhase(PHASE_MAIN_MENU);
            return false;
        }

        switch( event.type )
        {
            case EVENT_INTERFACE_CHAP:
                pl = static_cast<CList*>(pw->SearchControl(EVENT_INTERFACE_CHAP));
                if ( pl == 0 )  break;
                m_chap[m_category] = pl->GetSelect();
                m_main->GetPlayerProfile()->SetSelectedChap(m_category, m_chap[m_category]+1);
                UpdateSceneList(m_chap[m_category], m_sel[m_category]);
                UpdateSceneResume(m_chap[m_category]+1, m_sel[m_category]+1);
                break;

            case EVENT_INTERFACE_LIST:
                pl = static_cast<CList*>(pw->SearchControl(EVENT_INTERFACE_LIST));
                if ( pl == 0 )  break;
                m_sel[m_category] = pl->GetSelect();
                m_main->GetPlayerProfile()->SetSelectedRank(m_category, m_sel[m_category]+1);
                UpdateSceneResume(m_chap[m_category]+1, m_sel[m_category]+1);
                break;

            case EVENT_INTERFACE_SOLUCE:
                pb = static_cast<CButton*>(pw->SearchControl(EVENT_INTERFACE_SOLUCE));
                if ( pb == 0 )  break;
                m_bSceneSoluce = !m_bSceneSoluce;
                pb->SetState(STATE_CHECK, m_bSceneSoluce);
                break;

            case EVENT_INTERFACE_PLAY:
                m_levelChap = m_chap[m_category]+1;
                m_levelRank = m_sel[m_category]+1;
                m_main->ChangePhase(PHASE_LOADING);
                break;

            case EVENT_INTERFACE_READ:
                m_main->ChangePhase(PHASE_READ);
                break;

            default:
                return true;
        }
        return false;
    }

    if ( m_phase == PHASE_SETUPd ||
            m_phase == PHASE_SETUPg ||
            m_phase == PHASE_SETUPp ||
            m_phase == PHASE_SETUPc ||
            m_phase == PHASE_SETUPs )
    {
        pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
        if ( pw == 0 )  return false;

        if ( event.type == pw->GetEventTypeClose() ||
                event.type == EVENT_INTERFACE_BACK   ||
                (event.type == EVENT_KEY_DOWN && event.key.key == KEY(ESCAPE)) )
        {
            SetupMemorize();
            m_engine->ApplyChange();
            m_main->ChangePhase(PHASE_MAIN_MENU);
            return false;
        }

        switch( event.type )
        {
            case EVENT_INTERFACE_SETUPd:
                m_main->ChangePhase(PHASE_SETUPd);
                return false;

            case EVENT_INTERFACE_SETUPg:
                m_main->ChangePhase(PHASE_SETUPg);
                return false;

            case EVENT_INTERFACE_SETUPp:
                m_main->ChangePhase(PHASE_SETUPp);
                return false;

            case EVENT_INTERFACE_SETUPc:
                m_main->ChangePhase(PHASE_SETUPc);
                return false;

            case EVENT_INTERFACE_SETUPs:
                m_main->ChangePhase(PHASE_SETUPs);
                return false;

            default:
                break;
        }
    }

    if ( m_phase == PHASE_SETUPds ||
            m_phase == PHASE_SETUPgs ||
            m_phase == PHASE_SETUPps ||
            m_phase == PHASE_SETUPcs ||
            m_phase == PHASE_SETUPss )
    {
        pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
        if ( pw == 0 )  return false;

        if ( event.type == pw->GetEventTypeClose() ||
                event.type == EVENT_INTERFACE_BACK   ||
                (event.type == EVENT_KEY_DOWN && event.key.key == KEY(ESCAPE)) )
        {
            SetupMemorize();
            m_engine->ApplyChange();
            m_interface->DeleteControl(EVENT_WINDOW5);
            ChangePhase(PHASE_SIMUL);
            StopSuspend();
            return false;
        }

        switch( event.type )
        {
            case EVENT_INTERFACE_SETUPd:
                ChangePhase(PHASE_SETUPds);
                return false;

            case EVENT_INTERFACE_SETUPg:
                ChangePhase(PHASE_SETUPgs);
                return false;

            case EVENT_INTERFACE_SETUPp:
                ChangePhase(PHASE_SETUPps);
                return false;

            case EVENT_INTERFACE_SETUPc:
                ChangePhase(PHASE_SETUPcs);
                return false;

            case EVENT_INTERFACE_SETUPs:
                ChangePhase(PHASE_SETUPss);
                return false;

            default:
                break;
        }
    }

    if ( m_phase == PHASE_SETUPd  ||  // setup/display ?
            m_phase == PHASE_SETUPds )
    {
        switch( event.type )
        {
            case EVENT_LIST2:
                UpdateApply();
                break;

            case EVENT_INTERFACE_FULL:
                pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
                if ( pw == 0 )  break;
                pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_FULL));
                if ( pc == 0 )  break;

                if ( pc->TestState(STATE_CHECK) )
                {
                    pc->ClearState(STATE_CHECK);
                }
                else
                {
                    pc->SetState(STATE_CHECK);
                }

                UpdateApply();
                break;

            case EVENT_INTERFACE_APPLY:
                pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
                if ( pw == 0 )  break;
                pb = static_cast<CButton*>(pw->SearchControl(EVENT_INTERFACE_APPLY));
                if ( pb == 0 )  break;
                pb->ClearState(STATE_PRESS);
                pb->ClearState(STATE_HILIGHT);
                ChangeDisplay();
                UpdateApply();
                break;

            default:
                return true;
        }
        return false;
    }

    if ( m_phase == PHASE_SETUPg  ||  // setup/graphic ?
            m_phase == PHASE_SETUPgs )
    {
        switch( event.type )
        {
            case EVENT_INTERFACE_SHADOW:
                m_engine->SetShadow(!m_engine->GetShadow());
                ChangeSetupButtons();
                UpdateSetupButtons();
                break;

            case EVENT_INTERFACE_GROUND:
                m_engine->SetGroundSpot(!m_engine->GetGroundSpot());
                ChangeSetupButtons();
                UpdateSetupButtons();
                break;

            case EVENT_INTERFACE_DIRTY:
                m_engine->SetDirty(!m_engine->GetDirty());
                ChangeSetupButtons();
                UpdateSetupButtons();
                break;

            case EVENT_INTERFACE_FOG:
                m_engine->SetFog(!m_engine->GetFog());
                m_camera->SetOverBaseColor(Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f)); // TODO: color ok?
                ChangeSetupButtons();
                UpdateSetupButtons();
                break;

            case EVENT_INTERFACE_LENS:
                m_engine->SetLensMode(!m_engine->GetLensMode());
                ChangeSetupButtons();
                UpdateSetupButtons();
                break;

            case EVENT_INTERFACE_SKY:
                m_engine->SetSkyMode(!m_engine->GetSkyMode());
                ChangeSetupButtons();
                UpdateSetupButtons();
                break;

            case EVENT_INTERFACE_PLANET:
                m_engine->SetPlanetMode(!m_engine->GetPlanetMode());
                ChangeSetupButtons();
                UpdateSetupButtons();
                break;

            case EVENT_INTERFACE_LIGHT:
                m_engine->SetLightMode(!m_engine->GetLightMode());
                ChangeSetupButtons();
                UpdateSetupButtons();
                break;

            case EVENT_INTERFACE_PARTI:
            case EVENT_INTERFACE_CLIP:
            case EVENT_INTERFACE_DETAIL:
            case EVENT_INTERFACE_GADGET:
                ChangeSetupButtons();
                break;

            case EVENT_INTERFACE_TEXTURE_FILTER:
            case EVENT_INTERFACE_TEXTURE_MIPMAP:
            case EVENT_INTERFACE_TEXTURE_ANISOTROPY:
            case EVENT_INTERFACE_MSAA:
            case EVENT_INTERFACE_SHADOW_MAPPING:
                ChangeSetupButtons();
                UpdateSetupButtons();
                break;

            case EVENT_INTERFACE_SHADOW_MAPPING_QUALITY:
                m_engine->SetShadowMappingQuality(!m_engine->GetShadowMappingQuality());
                UpdateSetupButtons();
                break;

            case EVENT_INTERFACE_MIN:
                ChangeSetupQuality(-1);
                UpdateSetupButtons();
                break;
            case EVENT_INTERFACE_NORM:
                ChangeSetupQuality(0);
                UpdateSetupButtons();
                break;
            case EVENT_INTERFACE_MAX:
                ChangeSetupQuality(1);
                UpdateSetupButtons();
                break;

            default:
                return true;
        }
        return false;
    }

    if ( m_phase == PHASE_SETUPp  ||  // setup/game ?
            m_phase == PHASE_SETUPps )
    {
        switch( event.type )
        {
            case EVENT_INTERFACE_TOTO:
                m_engine->SetTotoMode(!m_engine->GetTotoMode());
                ChangeSetupButtons();
                UpdateSetupButtons();
                break;

            case EVENT_INTERFACE_TOOLTIP:
                m_bTooltip = !m_bTooltip;
                ChangeSetupButtons();
                UpdateSetupButtons();
                break;

            case EVENT_INTERFACE_GLINT:
                m_bGlint = !m_bGlint;
                ChangeSetupButtons();
                UpdateSetupButtons();
                break;

            case EVENT_INTERFACE_RAIN:
                m_bRain = !m_bRain;
                ChangeSetupButtons();
                UpdateSetupButtons();
                break;

            case EVENT_INTERFACE_MOUSE:
                if (m_app->GetMouseMode() == MOUSE_SYSTEM)
                    m_app->SetMouseMode(MOUSE_ENGINE);
                else
                    m_app->SetMouseMode(MOUSE_SYSTEM);

                ChangeSetupButtons();
                UpdateSetupButtons();
                break;

            case EVENT_INTERFACE_EDITMODE:
                m_engine->SetEditIndentMode(!m_engine->GetEditIndentMode());
                ChangeSetupButtons();
                UpdateSetupButtons();
                break;

            case EVENT_INTERFACE_EDITVALUE:
                if ( m_engine->GetEditIndentValue() == 2 )
                {
                    m_engine->SetEditIndentValue(4);
                }
                else
                {
                    m_engine->SetEditIndentValue(2);
                }
                ChangeSetupButtons();
                UpdateSetupButtons();
                break;

            case EVENT_INTERFACE_SOLUCE4:
                m_bSoluce4 = !m_bSoluce4;
                ChangeSetupButtons();
                UpdateSetupButtons();
                break;

            case EVENT_INTERFACE_MOVIES:
                m_bMovies = !m_bMovies;
                ChangeSetupButtons();
                UpdateSetupButtons();
                break;

            case EVENT_INTERFACE_NICERST:
                m_bNiceReset = !m_bNiceReset;
                ChangeSetupButtons();
                UpdateSetupButtons();
                break;

            case EVENT_INTERFACE_HIMSELF:
                m_bHimselfDamage = !m_bHimselfDamage;
                ChangeSetupButtons();
                UpdateSetupButtons();
                break;

            case EVENT_INTERFACE_SCROLL:
                m_bCameraScroll = !m_bCameraScroll;
                m_camera->SetCameraScroll(m_bCameraScroll);
                ChangeSetupButtons();
                UpdateSetupButtons();
                break;

            case EVENT_INTERFACE_INVERTX:
                m_bCameraInvertX = !m_bCameraInvertX;
                m_camera->SetCameraInvertX(m_bCameraInvertX);
                ChangeSetupButtons();
                UpdateSetupButtons();
                break;

            case EVENT_INTERFACE_INVERTY:
                m_bCameraInvertY = !m_bCameraInvertY;
                m_camera->SetCameraInvertY(m_bCameraInvertY);
                ChangeSetupButtons();
                UpdateSetupButtons();
                break;

            case EVENT_INTERFACE_EFFECT:
                m_bEffect = !m_bEffect;
                m_camera->SetEffect(m_bEffect);
                ChangeSetupButtons();
                UpdateSetupButtons();
                break;

            case EVENT_INTERFACE_BLOOD:
                m_bBlood = !m_bBlood;
                m_camera->SetBlood(m_bBlood);
                ChangeSetupButtons();
                UpdateSetupButtons();
                break;

            case EVENT_INTERFACE_AUTOSAVE_ENABLE:
                m_bAutosave = !m_bAutosave;
                m_main->SetAutosave(m_bAutosave);
                ChangeSetupButtons();
                UpdateSetupButtons();
                break;

            case EVENT_INTERFACE_AUTOSAVE_INTERVAL:
                ChangeSetupButtons();
                UpdateSetupButtons();
                break;

            case EVENT_INTERFACE_AUTOSAVE_SLOTS:
                ChangeSetupButtons();
                UpdateSetupButtons();
                break;

            default:
                return true;
        }
        return false;
    }

    if ( m_phase == PHASE_SETUPc  ||  // setup/commands ?
            m_phase == PHASE_SETUPcs )
    {
        switch( event.type )
        {
            case EVENT_INTERFACE_KSCROLL:
                UpdateKey();
                break;

            case EVENT_INTERFACE_KDEF:
                CInput::GetInstancePointer()->SetDefaultInputBindings();
                UpdateKey();
                break;

            case EVENT_INTERFACE_JOYSTICK:
                m_app->SetJoystickEnabled(!m_app->GetJoystickEnabled());
                UpdateSetupButtons();
                break;

            default:
                if (event.type >= EVENT_INTERFACE_KEY && event.type <= EVENT_INTERFACE_KEY_END)
                {
                    ChangeKey(event.type);
                    UpdateKey();
                    break;
                }
                return true;
        }
        return false;
    }

    if ( m_phase == PHASE_SETUPs  ||  // setup/sound ?
            m_phase == PHASE_SETUPss )
    {
        switch( event.type )
        {
            case EVENT_INTERFACE_VOLSOUND:
            case EVENT_INTERFACE_VOLMUSIC:
                ChangeSetupButtons();
                break;

            case EVENT_INTERFACE_SILENT:
                m_sound->SetAudioVolume(0);
                m_sound->SetMusicVolume(0);
                UpdateSetupButtons();
                break;
            case EVENT_INTERFACE_NOISY:
                m_sound->SetAudioVolume(MAXVOLUME);
                m_sound->SetMusicVolume(MAXVOLUME*3/4);
                UpdateSetupButtons();
                break;

            default:
                return true;
        }
        return false;
    }

    if ( m_phase == PHASE_READ )
    {
        pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
        if ( pw == 0 )  return false;

        if ( event.type == pw->GetEventTypeClose() ||
                event.type == EVENT_INTERFACE_BACK   ||
                (event.type == EVENT_KEY_DOWN && event.key.key == KEY(ESCAPE)) )
        {
            ChangePhase(PHASE_LEVEL_LIST);
            return false;
        }

        if ( event.type == EVENT_INTERFACE_IOLIST )
        {
            IOUpdateList();
            return false;
        }
        if ( event.type == EVENT_INTERFACE_IODELETE )
        {
            IODeleteScene();
            IOUpdateList();
            return false;
        }
        if ( event.type == EVENT_INTERFACE_IOREAD )
        {
            if ( IOReadScene() )
            {
                m_main->ChangePhase(PHASE_LOADING);
            }
            return false;
        }

        return true;
    }

    if ( m_phase == PHASE_WRITEs ||
            m_phase == PHASE_READs  )
    {
        pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
        if ( pw == 0 )  return false;

        if ( event.type == pw->GetEventTypeClose() ||
                event.type == EVENT_INTERFACE_BACK   ||
                (event.type == EVENT_KEY_DOWN && event.key.key == KEY(ESCAPE)) )
        {
            m_interface->DeleteControl(EVENT_WINDOW5);
            ChangePhase(PHASE_SIMUL);
            StopSuspend();
            return false;
        }

        if ( event.type == EVENT_INTERFACE_IOLIST )
        {
            IOUpdateList();
            return false;
        }
        if ( event.type == EVENT_INTERFACE_IODELETE )
        {
            IODeleteScene();
            IOUpdateList();
            return false;
        }
        if ( event.type == EVENT_INTERFACE_IOWRITE )
        {
            IOWriteScene();
            m_interface->DeleteControl(EVENT_WINDOW5);
            ChangePhase(PHASE_SIMUL);
            StopSuspend();
            return false;
        }
        if ( event.type == EVENT_INTERFACE_IOREAD )
        {
            if ( IOReadScene() )
            {
                m_interface->DeleteControl(EVENT_WINDOW5);
                ChangePhase(PHASE_SIMUL);
                StopSuspend();
                m_main->ChangePhase(PHASE_LOADING);
            }
            return false;
        }

        return true;
    }

    if ( m_phase == PHASE_WELCOME1 )
    {
        if ( event.type == EVENT_KEY_DOWN     ||
                event.type == EVENT_MOUSE_BUTTON_DOWN )
        {
            m_main->ChangePhase(PHASE_WELCOME2);
            return true;
        }
    }
    if ( m_phase == PHASE_WELCOME2 )
    {
        if ( event.type == EVENT_KEY_DOWN     ||
                event.type == EVENT_MOUSE_BUTTON_DOWN )
        {
            m_main->ChangePhase(PHASE_WELCOME3);
            return true;
        }
    }
    if ( m_phase == PHASE_WELCOME3 )
    {
        if ( event.type == EVENT_KEY_DOWN     ||
                event.type == EVENT_MOUSE_BUTTON_DOWN )
        {
            m_main->ChangePhase(PHASE_PLAYER_SELECT);
            return true;
        }
    }

    if ( m_phase == PHASE_QUIT_SCREEN )
    {
        if ( event.type == EVENT_INTERFACE_ABORT )
        {
            ChangePhase(PHASE_MAIN_MENU);
        }

        if ( event.type == EVENT_KEY_DOWN )
        {
            if ( event.key.key == KEY(ESCAPE) )
            {
                ChangePhase(PHASE_MAIN_MENU);
            }
            else
            {
                m_eventQueue->AddEvent(Event(EVENT_QUIT));
            }
        }

        if ( event.type == EVENT_MOUSE_BUTTON_DOWN )
        {
            m_eventQueue->AddEvent(Event(EVENT_QUIT));
        }
    }

    return true;
}


// Moves the reflections.

void CMainDialog::GlintMove()
{
    CWindow*    pw;
    CGroup*     pg;
    Math::Point     pos, dim, zoom;

    if ( m_phase == PHASE_SIMUL )  return;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == 0 )  return;

    if ( m_phase == PHASE_MAIN_MENU )
    {
        pg = static_cast<CGroup*>(pw->SearchControl(EVENT_INTERFACE_GLINTl));
        if ( pg != 0 )
        {
            zoom.x = sinf(m_glintTime*0.23f);
            zoom.y = sinf(m_glintTime*0.37f);
            pos.x = 0.35f;
            pos.y = 0.90f;
            dim.x = 0.30f-0.10f*(zoom.x+1.0f)/2.0f;
            dim.y = 0.50f-0.30f*(zoom.y+1.0f)/2.0f;
            pos.y -= dim.y;
            pg->SetPos(pos);
            pg->SetDim(dim);
        }

        pg = static_cast<CGroup*>(pw->SearchControl(EVENT_INTERFACE_GLINTr));
        if ( pg != 0 )
        {
            zoom.x = sinf(m_glintTime*0.21f);
            zoom.y = sinf(m_glintTime*0.26f);
            pos.x = 0.65f;
            pos.y = 0.10f;
            dim.x = 0.30f-0.10f*(zoom.x+1.0f)/2.0f;
            dim.y = 0.50f-0.30f*(zoom.y+1.0f)/2.0f;
            pos.x -= dim.x;
            pg->SetPos(pos);
            pg->SetDim(dim);
        }
    }

    if ( m_phase == PHASE_PLAYER_SELECT       ||
         m_phase == PHASE_LEVEL_LIST  )
    {
        pg = static_cast<CGroup*>(pw->SearchControl(EVENT_INTERFACE_GLINTl));
        if ( pg != 0 )
        {
            zoom.x = sinf(m_glintTime*0.22f);
            zoom.y = sinf(m_glintTime*0.37f);
            pos.x = 0.10f;
            pos.y = 0.90f;
            dim.x = 0.60f+0.30f*zoom.x;
            dim.y = 0.60f+0.30f*zoom.y;
            pos.y -= dim.y;
            pg->SetPos(pos);
            pg->SetDim(dim);
        }

        pg = static_cast<CGroup*>(pw->SearchControl(EVENT_INTERFACE_GLINTr));
        if ( pg != 0 )
        {
            zoom.x = sinf(m_glintTime*0.19f);
            zoom.y = sinf(m_glintTime*0.28f);
            pos.x = 0.90f;
            pos.y = 0.10f;
            dim.x = 0.60f+0.30f*zoom.x;
            dim.y = 0.60f+0.30f*zoom.y;
            pos.x -= dim.x;
            pg->SetPos(pos);
            pg->SetDim(dim);
        }
    }

    if ( m_phase == PHASE_SETUPd  ||
            m_phase == PHASE_SETUPg  ||
            m_phase == PHASE_SETUPp  ||
            m_phase == PHASE_SETUPc  ||
            m_phase == PHASE_SETUPs  ||
            m_phase == PHASE_SETUPds ||
            m_phase == PHASE_SETUPgs ||
            m_phase == PHASE_SETUPps ||
            m_phase == PHASE_SETUPcs ||
            m_phase == PHASE_SETUPss )
    {
        pg = static_cast<CGroup*>(pw->SearchControl(EVENT_INTERFACE_GLINTu));
        if ( pg != 0 )
        {
            zoom.y = sinf(m_glintTime*0.27f);
            pos.x = 0.10f;
            pos.y = 0.76f;
            dim.x = 0.80f;
            dim.y = 0.32f+0.20f*zoom.y;
            pos.y -= dim.y;
            pg->SetPos(pos);
            pg->SetDim(dim);
        }

        pg = static_cast<CGroup*>(pw->SearchControl(EVENT_INTERFACE_GLINTr));
        if ( pg != 0 )
        {
            zoom.x = sinf(m_glintTime*0.29f);
            zoom.y = sinf(m_glintTime*0.14f);
            pos.x = 0.90f;
            pos.y = 0.10f;
            dim.x = 0.40f+0.20f*zoom.x;
            dim.y = 0.40f+0.20f*zoom.y;
            pos.x -= dim.x;
            pg->SetPos(pos);
            pg->SetDim(dim);
        }
    }

    if ( m_phase == PHASE_WRITE  ||
            m_phase == PHASE_READ   ||
            m_phase == PHASE_WRITEs ||
            m_phase == PHASE_READs  )
    {
        pg = static_cast<CGroup*>(pw->SearchControl(EVENT_INTERFACE_GLINTl));
        if ( pg != 0 )
        {
            zoom.x = sinf(m_glintTime*0.22f);
            zoom.y = sinf(m_glintTime*0.37f);
            pos.x = 0.10f;
            pos.y = 0.90f;
            dim.x = 0.60f+0.30f*zoom.x;
            dim.y = 0.60f+0.30f*zoom.y;
            pos.y -= dim.y;
            pg->SetPos(pos);
            pg->SetDim(dim);
        }

        pg = static_cast<CGroup*>(pw->SearchControl(EVENT_INTERFACE_GLINTr));
        if ( pg != 0 )
        {
            zoom.x = sinf(m_glintTime*0.19f);
            zoom.y = sinf(m_glintTime*0.28f);
            pos.x = 0.90f;
            pos.y = 0.10f;
            dim.x = 0.60f+0.30f*zoom.x;
            dim.y = 0.60f+0.30f*zoom.y;
            pos.x -= dim.x;
            pg->SetPos(pos);
            pg->SetDim(dim);
        }
    }
}


// Returns the position for a sound.

Math::Vector SoundPos(Math::Point pos)
{
    Math::Vector    s;

    s.x = (pos.x-0.5f)*2.0f;
    s.y = (pos.y-0.5f)*2.0f;
    s.z = 0.0f;

    return s;
}

// Returns a random position for a sound.

Math::Vector SoundRand()
{
    Math::Vector    s;

    s.x = (Math::Rand()-0.5f)*2.0f;
    s.y = (Math::Rand()-0.5f)*2.0f;
    s.z = 0.0f;

    return s;
}

// Makes pretty qq particles evolve.

void CMainDialog::FrameParticle(float rTime)
{
    Math::Vector    pos, speed;
    Math::Point     dim;
    float       *pParti, *pGlint;
    int          nParti,  nGlint;
    int         i, r, ii;

    static float partiPosInit[1+5*12] =
    { //  x       x      t2    t2   type
        12.0f,
        607.0f, 164.0f, 0.2f, 0.8f, 1.0f,  // upper cable
        604.0f, 205.0f, 0.1f, 0.3f, 1.0f,  // middle cable
        603.0f, 247.0f, 0.1f, 0.3f, 1.0f,  // lower cable
        119.0f, 155.0f, 0.2f, 0.4f, 2.0f,  // left pipe
        366.0f,  23.0f, 0.5f, 1.5f, 4.0f,  // upper pipe
        560.0f, 414.0f, 0.1f, 0.1f, 1.0f,  // button lower/right
        20.0f, 413.0f, 0.1f, 0.1f, 2.0f,  // button lower/left
        39.0f,  78.0f, 0.1f, 0.2f, 1.0f,  // left pot
        39.0f,  78.0f, 0.5f, 0.9f, 1.0f,  // left pot
        170.0f, 229.0f, 0.5f, 0.5f, 3.0f,  // left smoke
        170.0f, 229.0f, 0.5f, 0.5f, 3.0f,  // left smoke
        474.0f, 229.0f, 0.5f, 0.5f, 3.0f,  // right smoke
    };

    static float glintPosInit[1+2*14] =
    {
        14.0f,
        15.0f, 407.0f,
        68.0f, 417.0f,
        548.0f,  36.0f,
        611.0f,  37.0f,
        611.0f, 100.0f,
        611.0f, 395.0f,
        36.0f,  35.0f,
        166.0f,  55.0f,
        166.0f,  94.0f,
        477.0f,  56.0f,
        31.0f, 190.0f,
        32.0f, 220.0f,
        65.0f, 221.0f,
        65.0f, 250.0f,
    };

    static float partiPosBig[1+5*12] =
    { //  x       x      t2    t2   type
        12.0f,
        607.0f, 164.0f, 0.2f, 0.8f, 1.0f,  // upper cable
        604.0f, 205.0f, 0.1f, 0.3f, 1.0f,  // middle cable
        603.0f, 247.0f, 0.1f, 0.3f, 1.0f,  // lower cable
        64.0f, 444.0f, 0.2f, 0.8f, 1.0f,  // down the left cable
        113.0f, 449.0f, 0.1f, 0.3f, 1.0f,  // down the left cable
        340.0f, 463.0f, 0.2f, 0.8f, 1.0f,  // down the middle cable
        36.0f, 155.0f, 0.2f, 0.4f, 2.0f,  // left pipe
        366.0f,  23.0f, 0.5f, 1.5f, 4.0f,  // upper pipe
        612.0f, 414.0f, 0.1f, 0.1f, 1.0f,  // button lower/right
        20.0f, 413.0f, 0.1f, 0.1f, 2.0f,  // button lower/left
        39.0f,  78.0f, 0.1f, 0.2f, 1.0f,  // left pot
        39.0f,  78.0f, 0.5f, 0.9f, 1.0f,  // left pot
    };

    static float glintPosBig[1+2*12] =
    {
        12.0f,
        15.0f, 407.0f,
        48.0f, 399.0f,
        611.0f,  37.0f,
        611.0f, 100.0f,
        611.0f, 395.0f,
        36.0f,  35.0f,
        31.0f, 190.0f,
        32.0f, 220.0f,
        31.0f, 221.0f,
        31.0f, 189.0f,
        255.0f,  18.0f,
        279.0f,  18.0f,
    };

    if ( m_bDialog || !m_bRain )  return;

    if ( m_phase == PHASE_MAIN_MENU )
    {
        pParti = partiPosInit;
        pGlint = glintPosInit;
    }
    else if ( m_phase == PHASE_PLAYER_SELECT    ||
            m_phase == PHASE_LEVEL_LIST ||
            m_phase == PHASE_SETUPd  ||
            m_phase == PHASE_SETUPg  ||
            m_phase == PHASE_SETUPp  ||
            m_phase == PHASE_SETUPc  ||
            m_phase == PHASE_SETUPs  ||
            m_phase == PHASE_WRITE   ||
            m_phase == PHASE_READ    )
    {
        pParti = partiPosBig;
        pGlint = glintPosBig;
    }
    else
    {
        return;
    }

    nParti = static_cast<int>(*pParti++);
    nGlint = static_cast<int>(*pGlint++);

    for ( i=0 ; i<10 ; i++ )
    {
        if ( m_partiPhase[i] == 0 )  // waiting?
        {
            m_partiTime[i] -= rTime;
            if ( m_partiTime[i] <= 0.0f )
            {
                r = rand()%3;

                if ( r == 0 )
                {
                    ii = rand()%nParti;
                    m_partiPos[i].x = pParti[ii*5+0]/640.0f;
                    m_partiPos[i].y = (480.0f-pParti[ii*5+1])/480.0f;
                    m_partiTime[i] = pParti[ii*5+2]+Math::Rand()*pParti[ii*5+3];
                    m_partiPhase[i] = static_cast<int>(pParti[ii*5+4]);
                    if ( m_partiPhase[i] == 3 )
                    {
                        m_sound->Play(SOUND_PSHHH, SoundPos(m_partiPos[i]), 0.3f+Math::Rand()*0.3f);
                    }
                    else
                    {
                        m_sound->Play(SOUND_GGG, SoundPos(m_partiPos[i]), 0.1f+Math::Rand()*0.4f);
                    }
                }

                if ( r == 1 )
                {
                    ii = rand()%nGlint;
                    pos.x = pGlint[ii*2+0]/640.0f;
                    pos.y = (480.0f-pGlint[ii*2+1])/480.0f;
                    pos.z = 0.0f;
                    speed.x = 0.0f;
                    speed.y = 0.0f;
                    speed.z = 0.0f;
                    dim.x = 0.04f+Math::Rand()*0.04f;
                    dim.y = dim.x/0.75f;
                    m_particle->CreateParticle(pos, speed, dim,
                            rand()%2?Gfx::PARTIGLINT:Gfx::PARTICONTROL,
                            Math::Rand()*0.4f+0.4f, 0.0f, 0.0f,
                            Gfx::SH_INTERFACE);
                    m_partiTime[i] = 0.5f+Math::Rand()*0.5f;
                }

                if ( r == 2 )
                {
                    ii = rand()%7;
                    if ( ii == 0 )
                    {
                        m_sound->Play(SOUND_ENERGY, SoundRand(), 0.2f+Math::Rand()*0.2f);
                        m_partiTime[i] = 1.0f+Math::Rand()*1.0f;
                    }
                    if ( ii == 1 )
                    {
                        m_sound->Play(SOUND_STATION, SoundRand(), 0.2f+Math::Rand()*0.2f);
                        m_partiTime[i] = 1.0f+Math::Rand()*2.0f;
                    }
                    if ( ii == 2 )
                    {
                        m_sound->Play(SOUND_ALARM, SoundRand(), 0.1f+Math::Rand()*0.1f);
                        m_partiTime[i] = 2.0f+Math::Rand()*4.0f;
                    }
                    if ( ii == 3 )
                    {
                        m_sound->Play(SOUND_INFO, SoundRand(), 0.1f+Math::Rand()*0.1f);
                        m_partiTime[i] = 2.0f+Math::Rand()*4.0f;
                    }
                    if ( ii == 4 )
                    {
                        m_sound->Play(SOUND_RADAR, SoundRand(), 0.2f+Math::Rand()*0.2f);
                        m_partiTime[i] = 0.5f+Math::Rand()*1.0f;
                    }
                    if ( ii == 5 )
                    {
                        m_sound->Play(SOUND_GFLAT, SoundRand(), 0.3f+Math::Rand()*0.3f);
                        m_partiTime[i] = 2.0f+Math::Rand()*4.0f;
                    }
                    if ( ii == 6 )
                    {
                        m_sound->Play(SOUND_ALARMt, SoundRand(), 0.1f+Math::Rand()*0.1f);
                        m_partiTime[i] = 2.0f+Math::Rand()*4.0f;
                    }
                }
            }
        }

        if ( m_partiPhase[i] != 0 )  // generates?
        {
            m_partiTime[i] -= rTime;
            if ( m_partiTime[i] > 0.0f )
            {
                if ( m_partiPhase[i] == 1 )  // sparks?
                {
                    pos.x = m_partiPos[i].x;
                    pos.y = m_partiPos[i].y;
                    pos.z = 0.0f;
                    pos.x += (Math::Rand()-0.5f)*0.01f;
                    pos.y += (Math::Rand()-0.5f)*0.01f;
                    speed.x = (Math::Rand()-0.5f)*0.2f;
                    speed.y = (Math::Rand()-0.5f)*0.2f;
                    speed.z = 0.0f;
                    dim.x = 0.005f+Math::Rand()*0.005f;
                    dim.y = dim.x/0.75f;
                    m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIBLITZ,
                            Math::Rand()*0.2f+0.2f, 0.0f, 0.0f,
                            Gfx::SH_INTERFACE);
                    pos.x = m_partiPos[i].x;
                    pos.y = m_partiPos[i].y;
                    pos.z = 0.0f;
                    speed.x = (Math::Rand()-0.5f)*0.5f;
                    speed.y = (0.3f+Math::Rand()*0.3f);
                    speed.z = 0.0f;
                    dim.x = 0.01f+Math::Rand()*0.01f;
                    dim.y = dim.x/0.75f;
                    m_particle->CreateParticle(pos, speed, dim,
                            static_cast<Gfx::ParticleType>(Gfx::PARTILENS1+rand()%3),
                            Math::Rand()*0.5f+0.5f, 2.0f, 0.0f,
                            Gfx::SH_INTERFACE);
                }
                if ( m_partiPhase[i] == 2 )  // sparks?
                {
                    pos.x = m_partiPos[i].x;
                    pos.y = m_partiPos[i].y;
                    pos.z = 0.0f;
                    pos.x += (Math::Rand()-0.5f)*0.01f;
                    pos.y += (Math::Rand()-0.5f)*0.01f;
                    speed.x = (Math::Rand()-0.5f)*0.2f;
                    speed.y = (Math::Rand()-0.5f)*0.2f;
                    speed.z = 0.0f;
                    dim.x = 0.005f+Math::Rand()*0.005f;
                    dim.y = dim.x/0.75f;
                    m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIBLITZ,
                            Math::Rand()*0.2f+0.2f, 0.0f, 0.0f,
                            Gfx::SH_INTERFACE);
                    pos.x = m_partiPos[i].x;
                    pos.y = m_partiPos[i].y;
                    pos.z = 0.0f;
                    speed.x = (Math::Rand()-0.5f)*0.5f;
                    speed.y = (0.3f+Math::Rand()*0.3f);
                    speed.z = 0.0f;
                    dim.x = 0.005f+Math::Rand()*0.005f;
                    dim.y = dim.x/0.75f;
                    m_particle->CreateParticle(pos, speed, dim, Gfx::PARTISCRAPS,
                            Math::Rand()*0.5f+0.5f, 2.0f, 0.0f,
                            Gfx::SH_INTERFACE);
                }
                if ( m_partiPhase[i] == 3 )  // smoke?
                {
                    pos.x = m_partiPos[i].x;
                    pos.y = m_partiPos[i].y;
                    pos.z = 0.0f;
                    pos.x += (Math::Rand()-0.5f)*0.03f;
                    pos.y += (Math::Rand()-0.5f)*0.03f;
                    speed.x = (Math::Rand()-0.5f)*0.2f;
                    speed.y = Math::Rand()*0.5f;
                    speed.z = 0.0f;
                    dim.x = 0.03f+Math::Rand()*0.07f;
                    dim.y = dim.x/0.75f;
                    m_particle->CreateParticle(pos, speed, dim, Gfx::PARTICRASH,
                            Math::Rand()*0.4f+0.4f, 0.0f, 0.0f,
                            Gfx::SH_INTERFACE);
                }
            }
            else
            {
                m_partiPhase[i] = 0;
                m_partiTime[i] = 2.0f+Math::Rand()*4.0f;
            }
        }
    }
    // #endif
}

// Some nice particles following the mouse.

void CMainDialog::NiceParticle(Math::Point mouse, bool bPress)
{
    Math::Vector    pos, speed;
    Math::Point     dim;

    if ( !m_bRain )  return;
    if ( (m_phase == PHASE_SIMUL ||
                m_phase == PHASE_WIN   ||
                m_phase == PHASE_LOST  ) &&
            !m_bDialog             )  return;

    if ( bPress )
    {
        pos.x = mouse.x;
        pos.y = mouse.y;
        pos.z = 0.0f;
        speed.x = (Math::Rand()-0.5f)*0.5f;
        speed.y = (0.3f+Math::Rand()*0.3f);
        speed.z = 0.0f;
        dim.x = 0.005f+Math::Rand()*0.005f;
        dim.y = dim.x/0.75f;
        m_particle->CreateParticle(pos, speed, dim, Gfx::PARTISCRAPS,
                Math::Rand()*0.5f+0.5f, 2.0f, 0.0f,
                Gfx::SH_INTERFACE);
    }
    else
    {
        pos.x = mouse.x;
        pos.y = mouse.y;
        pos.z = 0.0f;
        speed.x = (Math::Rand()-0.5f)*0.5f;
        speed.y = (0.3f+Math::Rand()*0.3f);
        speed.z = 0.0f;
        dim.x = 0.01f+Math::Rand()*0.01f;
        dim.y = dim.x/0.75f;
        m_particle->CreateParticle(pos, speed, dim,
                static_cast<Gfx::ParticleType>(Gfx::PARTILENS1+rand()%3),
                Math::Rand()*0.5f+0.5f, 2.0f, 0.0f,
                Gfx::SH_INTERFACE);
    }
}



// Built the default descriptive name of a mission.

void CMainDialog::BuildResumeName(char *filename, std::string base, int chap, int rank)
{
    sprintf(filename, "%s %d.%d", base.c_str(), chap, rank);
}


// Updates the list of players after checking the files on disk.

void CMainDialog::ReadNameList()
{
    CWindow* pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if (pw == nullptr) return;
    CList* pl = static_cast<CList*>(pw->SearchControl(EVENT_INTERFACE_NLIST));
    if (pl == nullptr) return;
    pl->Flush();

    auto players = CPlayerProfile::GetPlayerList();
    for (int i = 0; i < static_cast<int>(players.size()); ++i)
    {
        pl->SetItemName(i, players.at(i).c_str());
    }
}

// Updates the controls of the players.

void CMainDialog::UpdateNameControl()
{
    CWindow*    pw;
    CList*      pl;
    CButton*    pb;
    CEdit*      pe;
    char        name[100];
    int         total, sel;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == 0 )  return;
    pl = static_cast<CList*>(pw->SearchControl(EVENT_INTERFACE_NLIST));
    if ( pl == 0 )  return;
    pe = static_cast<CEdit*>(pw->SearchControl(EVENT_INTERFACE_NEDIT));
    if ( pe == 0 )  return;

    std::string gamer = m_main->GetPlayerProfile()->GetName();
    total = pl->GetTotal();
    sel   = pl->GetSelect();
    pe->GetText(name, 100);

    pb = static_cast<CButton*>(pw->SearchControl(EVENT_INTERFACE_NCANCEL));
    if ( pb != 0 )
    {
        pb->SetState(STATE_ENABLE, !gamer.empty());
    }

    pb = static_cast<CButton*>(pw->SearchControl(EVENT_INTERFACE_NDELETE));
    if ( pb != 0 )
    {
        pb->SetState(STATE_ENABLE, total>0 && sel!=-1);
    }

    pb = static_cast<CButton*>(pw->SearchControl(EVENT_INTERFACE_NOK));
    if ( pb != 0 )
    {
        pb->SetState(STATE_ENABLE, name[0]!=0 || sel!=-1);
    }

    pb = static_cast<CButton*>(pw->SearchControl(EVENT_INTERFACE_PERSO));
    if ( pb != 0 )
    {
        pb->SetState(STATE_ENABLE, name[0]!=0 || sel!=-1);
    }
}

// Updates the list of players by name frape.

void CMainDialog::UpdateNameList()
{
    CWindow*    pw;
    CList*      pl;
    CEdit*      pe;
    char        name[100];
    int         total, i;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == 0 )  return;
    pl = static_cast<CList*>(pw->SearchControl(EVENT_INTERFACE_NLIST));
    if ( pl == 0 )  return;
    pe = static_cast<CEdit*>(pw->SearchControl(EVENT_INTERFACE_NEDIT));
    if ( pe == 0 )  return;

    pe->GetText(name, 100);
    total = pl->GetTotal();

    for ( i=0 ; i<total ; i++ )
    {
        // TODO: stricmp?
        if ( strcmp(name, pl->GetItemName(i)) == 0 )
        {
            pl->SetSelect(i);
            pl->ShowSelect(false);
            return;
        }
    }

    pl->SetSelect(-1);
}

// Updates the player's name and function of the selected list.

void CMainDialog::UpdateNameEdit()
{
    CWindow*    pw;
    CList*      pl;
    CEdit*      pe;
    char*       name;
    int         sel;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == 0 )  return;
    pl = static_cast<CList*>(pw->SearchControl(EVENT_INTERFACE_NLIST));
    if ( pl == 0 )  return;
    pe = static_cast<CEdit*>(pw->SearchControl(EVENT_INTERFACE_NEDIT));
    if ( pe == 0 )  return;

    sel = pl->GetSelect();
    if ( sel == -1 )
    {
        pe->SetText("");
        pe->SetCursor(0, 0);
    }
    else
    {
        name = pl->GetItemName(sel);
        pe->SetText(name);
        pe->SetCursor(strlen(name), 0);
    }

    UpdateNameControl();
}

// Selects a player.

void CMainDialog::NameSelect()
{
    CWindow*    pw;
    CList*      pl;
    CEdit*      pe;
    char        name[100];
    int         sel;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == 0 )  return;
    pl = static_cast<CList*>(pw->SearchControl(EVENT_INTERFACE_NLIST));
    if ( pl == 0 )  return;
    pe = static_cast<CEdit*>(pw->SearchControl(EVENT_INTERFACE_NEDIT));
    if ( pe == 0 )  return;

    pe->GetText(name, 100);
    sel  = pl->GetSelect();

    if ( sel == -1 )
    {
        NameCreate();
    }
    else
    {
        m_main->SelectPlayer(pl->GetItemName(sel));
    }

    m_main->ChangePhase(PHASE_MAIN_MENU);
}

// Creates a new player.

bool CMainDialog::NameCreate()
{
    CWindow*    pw;
    CEdit*      pe;
    char        name[100];
    char        c;
    int         len, i, j;

    GetLogger()->Info("Creating new player\n");
    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == 0 )  return false;
    pe = static_cast<CEdit*>(pw->SearchControl(EVENT_INTERFACE_NEDIT));
    if ( pe == 0 )  return false;

    pe->GetText(name, 100);
    if ( name[0] == 0 )
    {
        m_sound->Play(SOUND_TZOING);
        return false;
    }

    len = strlen(name);
    j = 0;
    for ( i=0 ; i<len ; i++ )
    {
        c = GetNoAccent(GetToLower(name[i]));
        if ( (c >= '0' && c <= '9') ||
             (c >= 'a' && c <= 'z') ||
             c == ' ' ||
             c == '-' ||
             c == '_' ||
             c == '.' ||
             c == ',' ||
             c == '\'' )
        {
            name[j++] = name[i];
        }
    }
    name[j] = 0;
    if ( j == 0 )
    {
        m_sound->Play(SOUND_TZOING);
        return false;
    }

    m_main->SelectPlayer(name);
    m_main->GetPlayerProfile()->Create();

    return true;
}

// Removes a player.

void CMainDialog::NameDelete()
{
    CWindow* pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if (pw == nullptr) return;
    CList* pl = static_cast<CList*>(pw->SearchControl(EVENT_INTERFACE_NLIST));
    if (pl == nullptr) return;

    int sel = pl->GetSelect();
    if (sel == -1)
    {
        m_sound->Play(SOUND_TZOING);
        return;
    }

    char* gamer = pl->GetItemName(sel);

    m_main->SelectPlayer(gamer);
    if (!m_main->GetPlayerProfile()->Delete())
    {
        m_sound->Play(SOUND_TZOING);
        return;
    }

    pl->SetSelect(-1);

    ReadNameList();
    UpdateNameList();
    UpdateNameControl();
}



// ests whether two colors are equal or nearly are.

bool EqColor(const Gfx::Color &c1, const Gfx::Color &c2)
{
    return (fabs(c1.r-c2.r) < 0.01f &&
            fabs(c1.g-c2.g) < 0.01f &&
            fabs(c1.b-c2.b) < 0.01f );
}

// Updates all the buttons for the character.

void CMainDialog::UpdatePerso()
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
    if ( pw == 0 )  return;

    pb = static_cast<CButton*>(pw->SearchControl(EVENT_INTERFACE_PHEAD));
    if ( pb != 0 )
    {
        pb->SetState(STATE_CHECK, m_apperanceTab==0);
    }
    pb = static_cast<CButton*>(pw->SearchControl(EVENT_INTERFACE_PBODY));
    if ( pb != 0 )
    {
        pb->SetState(STATE_CHECK, m_apperanceTab==1);
    }

    pl = static_cast<CLabel*>(pw->SearchControl(EVENT_LABEL11));
    if ( pl != 0 )
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
    if ( pl != 0 )
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
    if ( pl != 0 )
    {
        if ( m_apperanceTab == 0 )  GetResource(RES_TEXT, RT_PERSO_HAIR, name);
        else                    GetResource(RES_TEXT, RT_PERSO_BAND, name);
        pl->SetName(name);
    }

    pl = static_cast<CLabel*>(pw->SearchControl(EVENT_LABEL14));
    if ( pl != 0 )
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
        if ( pb == 0 )  break;
        pb->SetState(STATE_VISIBLE, m_apperanceTab==0);
        pb->SetState(STATE_CHECK, i==apperance.face);
    }

    for ( i=0 ; i<10 ; i++ )
    {
        pb = static_cast<CButton*>(pw->SearchControl(static_cast<EventType>(EVENT_INTERFACE_PGLASS0+i)));
        if ( pb == 0 )  break;
        pb->SetState(STATE_VISIBLE, m_apperanceTab==0);
        pb->SetState(STATE_CHECK, i==apperance.glasses);
    }

    for ( i=0 ; i<3*3 ; i++ )
    {
        pc = static_cast<CColor*>(pw->SearchControl(static_cast<EventType>(EVENT_INTERFACE_PC0a+i)));
        if ( pc == 0 )  break;
        if ( m_apperanceTab == 0 )
        {
            pc->ClearState(STATE_VISIBLE);
        }
        else
        {
            pc->SetState(STATE_VISIBLE);
            color.r = perso_color[3*10*1+3*i+0]/255.0f;
            color.g = perso_color[3*10*1+3*i+1]/255.0f;
            color.b = perso_color[3*10*1+3*i+2]/255.0f;
            color.a = 0.0f;
            pc->SetColor(color);
            pc->SetState(STATE_CHECK, EqColor(color, apperance.colorCombi));
        }

        pc = static_cast<CColor*>(pw->SearchControl(static_cast<EventType>(EVENT_INTERFACE_PC0b+i)));
        if ( pc == 0 )  break;
        color.r = perso_color[3*10*2*m_apperanceTab+3*i+0]/255.0f;
        color.g = perso_color[3*10*2*m_apperanceTab+3*i+1]/255.0f;
        color.b = perso_color[3*10*2*m_apperanceTab+3*i+2]/255.0f;
        color.a = 0.0f;
        pc->SetColor(color);
        pc->SetState(STATE_CHECK, EqColor(color, m_apperanceTab?apperance.colorBand:apperance.colorHair));
    }

    for ( i=0 ; i<3 ; i++ )
    {
        ps = static_cast<CSlider*>(pw->SearchControl(static_cast<EventType>(EVENT_INTERFACE_PCRa+i)));
        if ( ps == 0 )  break;
        ps->SetState(STATE_VISIBLE, m_apperanceTab==1);
    }

    if ( m_apperanceTab == 1 )
    {
        color = apperance.colorCombi;
        ps = static_cast<CSlider*>(pw->SearchControl(EVENT_INTERFACE_PCRa));
        if ( ps != 0 )  ps->SetVisibleValue(color.r*255.0f);
        ps = static_cast<CSlider*>(pw->SearchControl(EVENT_INTERFACE_PCGa));
        if ( ps != 0 )  ps->SetVisibleValue(color.g*255.0f);
        ps = static_cast<CSlider*>(pw->SearchControl(EVENT_INTERFACE_PCBa));
        if ( ps != 0 )  ps->SetVisibleValue(color.b*255.0f);
    }

    if ( m_apperanceTab == 0 )  color = apperance.colorHair;
    else                    color = apperance.colorBand;
    ps = static_cast<CSlider*>(pw->SearchControl(EVENT_INTERFACE_PCRb));
    if ( ps != 0 )  ps->SetVisibleValue(color.r*255.0f);
    ps = static_cast<CSlider*>(pw->SearchControl(EVENT_INTERFACE_PCGb));
    if ( ps != 0 )  ps->SetVisibleValue(color.g*255.0f);
    ps = static_cast<CSlider*>(pw->SearchControl(EVENT_INTERFACE_PCBb));
    if ( ps != 0 )  ps->SetVisibleValue(color.b*255.0f);
}

// Updates the camera for the character.

void CMainDialog::CameraPerso()
{
    if ( m_apperanceTab == 0 )
    {
//?     m_camera->Init(Math::Vector(4.0f, 0.0f, 0.0f),
//?                    Math::Vector(0.0f, 0.0f, 1.0f), 0.0f);
        m_camera->Init(Math::Vector(6.0f, 0.0f, 0.0f),
                       Math::Vector(0.0f, 0.2f, 1.5f), 0.0f);
    }
    else
    {
        m_camera->Init(Math::Vector(18.0f, 0.0f, 4.5f),
                       Math::Vector(0.0f, 1.6f, 4.5f), 0.0f);
    }

    m_camera->SetType(Gfx::CAM_TYPE_SCRIPT);
    m_camera->FixCamera();
}

// Sets a fixed color.

void CMainDialog::FixPerso(int rank, int index)
{
    PlayerApperance& apperance = m_main->GetPlayerProfile()->GetApperance();
    if ( m_apperanceTab == 0 )
    {
        if ( index == 1 )
        {
            apperance.colorHair.r = perso_color[3*10*0+rank*3+0]/255.0f;
            apperance.colorHair.g = perso_color[3*10*0+rank*3+1]/255.0f;
            apperance.colorHair.b = perso_color[3*10*0+rank*3+2]/255.0f;
        }
    }
    if ( m_apperanceTab == 1 )
    {
        if ( index == 0 )
        {
            apperance.colorCombi.r = perso_color[3*10*1+rank*3+0]/255.0f;
            apperance.colorCombi.g = perso_color[3*10*1+rank*3+1]/255.0f;
            apperance.colorCombi.b = perso_color[3*10*1+rank*3+2]/255.0f;
        }
        if ( index == 1 )
        {
            apperance.colorBand.r = perso_color[3*10*2+rank*3+0]/255.0f;
            apperance.colorBand.g = perso_color[3*10*2+rank*3+1]/255.0f;
            apperance.colorBand.b = perso_color[3*10*2+rank*3+2]/255.0f;
        }
    }
}

// Updates the color of the character.

void CMainDialog::ColorPerso()
{
    CWindow*        pw;
    CSlider*        ps;
    Gfx::Color   color;

    PlayerApperance& apperance = m_main->GetPlayerProfile()->GetApperance();

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == 0 )  return;

    color.a = 0.0f;

    ps = static_cast<CSlider*>(pw->SearchControl(EVENT_INTERFACE_PCRa));
    if ( ps != 0 )  color.r = ps->GetVisibleValue()/255.0f;
    ps = static_cast<CSlider*>(pw->SearchControl(EVENT_INTERFACE_PCGa));
    if ( ps != 0 )  color.g = ps->GetVisibleValue()/255.0f;
    ps = static_cast<CSlider*>(pw->SearchControl(EVENT_INTERFACE_PCBa));
    if ( ps != 0 )  color.b = ps->GetVisibleValue()/255.0f;
    if ( m_apperanceTab == 1 )  apperance.colorCombi = color;

    ps = static_cast<CSlider*>(pw->SearchControl(EVENT_INTERFACE_PCRb));
    if ( ps != 0 )  color.r = ps->GetVisibleValue()/255.0f;
    ps = static_cast<CSlider*>(pw->SearchControl(EVENT_INTERFACE_PCGb));
    if ( ps != 0 )  color.g = ps->GetVisibleValue()/255.0f;
    ps = static_cast<CSlider*>(pw->SearchControl(EVENT_INTERFACE_PCBb));
    if ( ps != 0 )  color.b = ps->GetVisibleValue()/255.0f;
    if ( m_apperanceTab == 0 )  apperance.colorHair = color;
    else                        apperance.colorBand = color;
}


// Indicates if there is at least one backup.

bool CMainDialog::IsIOReadScene()
{
    auto saveDirs = CResourceManager::ListDirectories(m_main->GetPlayerProfile()->GetSaveDir());
    for (auto dir : saveDirs)
    {
        if (CResourceManager::Exists(m_main->GetPlayerProfile()->GetSaveFile(dir + "/data.sav")))
        {
            return true;
        }
    }

    return false;
}

// Builds the file name by default.

void CMainDialog::IOReadName()
{
    CWindow*    pw;
    CEdit*      pe;
    std::string resume;
    char        line[100];
    char        name[100];
    time_t      now;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == nullptr )  return;
    pe = static_cast<CEdit*>(pw->SearchControl(EVENT_INTERFACE_IONAME));
    if ( pe == nullptr )  return;

    resume = std::string(GetLevelCategoryDir(m_category)) + " " + boost::lexical_cast<std::string>(m_chap[m_category]+1);

    CLevelParser levelParser(m_category, m_chap[m_category]+1, 0);
    try
    {
        levelParser.Load();
        resume = levelParser.Get("Title")->GetParam("resume")->AsString();
    }
    catch (CLevelParserException& e)
    {
        GetLogger()->Warn("%s\n", e.what());
    }

    time(&now);
    TimeToAsciiClean(now, line);
    sprintf(name, "%s - %s %d", line, resume.c_str(), m_sel[m_category]+1);

    pe->SetText(name);
    pe->SetCursor(strlen(name), 0);
    m_interface->SetFocus(pe);
}

// Updates the list of games recorded on disk.

void CMainDialog::IOReadList()
{
    CWindow* pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if (pw == nullptr) return;
    CList* pl = static_cast<CList*>(pw->SearchControl(EVENT_INTERFACE_IOLIST));
    if (pl == nullptr) return;

    pl->Flush();

    m_saveList.clear();

    auto saveDirs = CResourceManager::ListDirectories(m_main->GetPlayerProfile()->GetSaveDir());
    //std::sort(saveDirs.begin(), saveDirs.end());

    std::map<int, std::string> sortedSaveDirs;
    std::map<int, std::string> names;

    for (auto dir : saveDirs)
    {
        std::string savegameFile = m_main->GetPlayerProfile()->GetSaveFile(dir+"/data.sav");
        if (CResourceManager::Exists(savegameFile))
        {
            CLevelParser levelParser(savegameFile);
            levelParser.Load();
            int time = levelParser.Get("Created")->GetParam("date")->AsInt();
            sortedSaveDirs[time] = m_main->GetPlayerProfile()->GetSaveFile(dir);
            names[time] = levelParser.Get("Title")->GetParam("text")->AsString();
        }
    }

    for (auto dir : sortedSaveDirs)
    {
        pl->SetItemName(m_saveList.size(), names[dir.first].c_str());
        m_saveList.push_back(dir.second);
    }

    // invalid index
    if ( m_phase == PHASE_WRITE  || m_phase == PHASE_WRITEs )
    {
        std::string nameStr;
        GetResource(RES_TEXT, RT_IO_NEW, nameStr);
        pl->SetItemName(m_saveList.size(), nameStr.c_str());
    }

    pl->SetSelect(m_saveList.size());
    pl->ShowSelect(false);  // shows the selected columns

    unsigned int i;
    std::string  screenName;

    for ( i=0; i < m_saveList.size(); i++ )
    {
        screenName = "textures/../" + m_saveList.at(i) + "/screen.png";
        m_engine->DeleteTexture(screenName);
    }
}

// Updates the buttons according to the selected part in the list.

void CMainDialog::IOUpdateList()
{
    CWindow*    pw;
    CList*      pl;
    CButton*    pb;
    CImage*     pi;
    int         sel, max;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == nullptr )  return;
    pl = static_cast<CList*>(pw->SearchControl(EVENT_INTERFACE_IOLIST));
    if ( pl == nullptr )  return;
    pi = static_cast<CImage*>(pw->SearchControl(EVENT_INTERFACE_IOIMAGE));
    if ( pi == nullptr )  return;

    sel = pl->GetSelect();
    max = pl->GetTotal();

    if (m_saveList.size() <= static_cast<unsigned int>(sel))
        return;

    std::string filename = "../"+m_saveList.at(sel) + "/screen.png";
    if ( m_phase == PHASE_WRITE  || m_phase == PHASE_WRITEs )
    {
        if ( sel < max-1 )
        {
            pi->SetFilenameImage(filename.c_str());
        }
        else
        {
            pi->SetFilenameImage("");
        }

        pb = static_cast<CButton*>(pw->SearchControl(EVENT_INTERFACE_IODELETE));
        if ( pb != nullptr )
        {
            pb->SetState(STATE_ENABLE, sel < max-1);
        }
    }
    else
    {
        pi->SetFilenameImage(filename.c_str());
    }
}

// Deletes the selected scene.

void CMainDialog::IODeleteScene()
{
    CWindow* pw;
    CList*   pl;
    int      sel;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == 0 )  return;
    pl = static_cast<CList*>(pw->SearchControl(EVENT_INTERFACE_IOLIST));
    if ( pl == 0 )  return;

    sel = pl->GetSelect();
    if ( sel == -1 || m_saveList.size() <= static_cast<unsigned int>(sel))
    {
        m_sound->Play(SOUND_TZOING);
        return;
    }

    if (CResourceManager::DirectoryExists(m_saveList.at(sel)))
    {
        CResourceManager::RemoveDirectory(m_saveList.at(sel));
    }

    IOReadList();
}

// clears filename only to leave letter or numbers
std::string clearName(char *name)
{
    std::string ret;
    int len = strlen(name);
    for (int i = 0; i < len; i++)
    {
        if (isalnum(name[i]))
        {
            ret += name[i];
        }
    }
    return ret;
}


// Writes the scene.
bool CMainDialog::IOWriteScene()
{
    CWindow*    pw;
    CList*      pl;
    CEdit*      pe;
    char        info[100];
    int         sel;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == nullptr )  return false;
    pl = static_cast<CList*>(pw->SearchControl(EVENT_INTERFACE_IOLIST));
    if ( pl == nullptr )  return false;
    pe = static_cast<CEdit*>(pw->SearchControl(EVENT_INTERFACE_IONAME));
    if ( pe == nullptr )  return false;

    sel = pl->GetSelect();
    if ( sel == -1 )
    {
        return false;
    }

    std::string dir;
    pe->GetText(info, 100);
    if (static_cast<unsigned int>(sel) >= m_saveList.size())
    {
        dir = m_main->GetPlayerProfile()->GetSaveFile("save"+clearName(info));
    }
    else
    {
        dir = m_saveList.at(sel);
    }

    m_main->GetPlayerProfile()->SaveScene(dir, info);

    return true;
}

void CMainDialog::MakeSaveScreenshot(const std::string& name)
{
    m_shotDelay = 3;
    m_shotName = CResourceManager::GetSaveLocation() + "/" + name; //TODO: Use PHYSFS?
}

// Reads the scene.

bool CMainDialog::IOReadScene()
{
    CWindow*    pw;
    CList*      pl;
    int         sel;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == nullptr )  return false;
    pl = static_cast<CList*>(pw->SearchControl(EVENT_INTERFACE_IOLIST));
    if ( pl == nullptr )  return false;

    sel = pl->GetSelect();
    if ( sel == -1 || m_saveList.size() <= static_cast<unsigned int>(sel) )
    {
        return false;
    }

    std::string fileName = m_saveList.at(sel) + "/" + "data.sav";
    std::string fileCbot = CResourceManager::GetSaveLocation()+"/"+m_saveList.at(sel) + "/" + "cbot.run";

    CLevelParser levelParser(fileName);
    levelParser.Load();

    CLevelParserLine* line = levelParser.Get("Mission");
    m_category = GetLevelCategoryFromDir(line->GetParam("base")->AsString());

    m_levelRank = line->GetParam("rank")->AsInt();
    if (m_category == LevelCategory::CustomLevels)
    {
        m_levelChap = 0;
        std::string dir = line->GetParam("dir")->AsString();
        UpdateCustomLevelList();
        for (unsigned int i = 0; i < m_customLevelList.size(); i++)
        {
            if (m_customLevelList[i] == dir)
            {
                m_levelChap = i+1;
                break;
            }
        }
        if (m_levelChap == 0)
        {
            return false;
        }
    }
    else
    {
        if(line->GetParam("chap")->IsDefined())
        {
            m_levelChap = line->GetParam("chap")->AsInt();
        }
        else
        {
            // Backwards combatibility
            int rank = line->GetParam("rank")->AsInt();
            m_levelChap = rank/100;
            m_levelRank = rank%100;
        }
    }

    m_chap[m_category] = m_levelChap-1;
    m_sel[m_category]  = m_levelRank-1;

    m_sceneRead = fileName;
    m_stackRead = fileCbot;
    return true;
}

// Updates the lists according to the cheat code.

void CMainDialog::AllMissionUpdate()
{
    if ( m_phase == PHASE_LEVEL_LIST )
    {
        UpdateSceneChap(m_chap[m_category]);
        UpdateSceneList(m_chap[m_category], m_sel[m_category]);
    }
}

// Updates the chapters of exercises or missions.

void CMainDialog::UpdateSceneChap(int &chap)
{
    CWindow*    pw;
    CList*      pl;

    std::string fileName;
    char        line[500];
    bool        bPassed;

    memset(line, 0, 500);

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == 0 )  return;
    pl = static_cast<CList*>(pw->SearchControl(EVENT_INTERFACE_CHAP));
    if ( pl == 0 )  return;

    pl->Flush();

    unsigned int j;
    if ( m_category == LevelCategory::CustomLevels )
    {
        UpdateCustomLevelList();

        for ( j=0 ; j<m_customLevelList.size() ; j++ )
        {
            try
            {
                CLevelParser levelParser("custom", j+1, 0);
                levelParser.Load();
                pl->SetItemName(j, levelParser.Get("Title")->GetParam("text")->AsString().c_str());
                pl->SetEnable(j, true);
            }
            catch (CLevelParserException& e)
            {
                pl->SetItemName(j, (std::string("[ERROR]: ")+e.what()).c_str());
                pl->SetEnable(j, false);
            }
        }
    }
    else
    {
        for ( j=0 ; j<MAXSCENE ; j++ )
        {
            CLevelParser levelParser(m_category, j+1, 0);
            if (!levelParser.Exists())
                break;
            try
            {
                levelParser.Load();
                sprintf(line, "%d: %s", j+1, levelParser.Get("Title")->GetParam("text")->AsString().c_str());
            }
            catch (CLevelParserException& e)
            {
                sprintf(line, "%s", (std::string("[ERROR]: ")+e.what()).c_str());
            }

            bPassed = m_main->GetPlayerProfile()->GetLevelPassed(m_category, j+1, 0);
            pl->SetItemName(j, line);
            pl->SetCheck(j, bPassed);
            pl->SetEnable(j, true);

            if ( m_category == LevelCategory::Missions && !m_main->GetShowAll() && !bPassed )
            {
                j ++;
                break;
            }

            if ( m_category == LevelCategory::FreeGame && j == m_accessChap )
            {
                j ++;
                break;
            }
        }
    }

    if ( chap > j-1 )  chap = j-1;

    pl->SetSelect(chap);
    pl->ShowSelect(false);  // shows the selected columns
}

// Updates the list of exercises or missions.

void CMainDialog::UpdateSceneList(int chap, int &sel)
{
    CWindow*    pw;
    CList*      pl;
    std::string fileName;
    char        line[500];
    int         j;
    bool        bPassed;

    memset(line, 0, 500);

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == 0 )  return;
    pl = static_cast<CList*>(pw->SearchControl(EVENT_INTERFACE_LIST));
    if ( pl == 0 )  return;

    pl->Flush();

    bool readAll = true;
    for ( j=0 ; j<MAXSCENE ; j++ )
    {
        CLevelParser levelParser(m_category, chap+1, j+1);
        if (!levelParser.Exists())
        {
            readAll = true;
            break;
        }
        else
        {
            if (!readAll)
                break;
        }
        try
        {
            levelParser.Load();
            sprintf(line, "%d: %s", j+1, levelParser.Get("Title")->GetParam("text")->AsString().c_str());
        }
        catch (CLevelParserException& e)
        {
            sprintf(line, "%s", (std::string("[ERROR]: ")+e.what()).c_str());
        }

        bPassed = m_main->GetPlayerProfile()->GetLevelPassed(m_category, chap+1, j+1);
        pl->SetItemName(j, line);
        pl->SetCheck(j, bPassed);
        pl->SetEnable(j, true);

        if ( m_category == LevelCategory::Missions && !m_main->GetShowAll() && !bPassed )
        {
            readAll = false;
        }
    }

    if (readAll)
    {
        m_maxList = j;
    }
    else
    {
        m_maxList = j+1;  // this is not the last!
    }

    if ( sel > j-1 )  sel = j-1;

    pl->SetSelect(sel);
    pl->ShowSelect(false);  // shows the selected columns
}

// Updates the button "solution" according to cheat code.

void CMainDialog::ShowSoluceUpdate()
{
    CWindow*    pw;
    CEdit*      pe;
    CCheck*     pc;

    if ( m_phase == PHASE_LEVEL_LIST )
    {
        m_bSceneSoluce = false;

        pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
        if ( pw == 0 )  return;
        pe = static_cast<CEdit*>(pw->SearchControl(EVENT_INTERFACE_RESUME));
        if ( pe == 0 )  return;
        pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_SOLUCE));
        if ( pc == 0 )  return;

        if ( m_main->GetShowSoluce() )
        {
            pc->SetState(STATE_VISIBLE);
            pc->SetState(STATE_CHECK);
            m_bSceneSoluce = true;
        }
        else
        {
            pc->ClearState(STATE_VISIBLE);
            pc->ClearState(STATE_CHECK);
            m_bSceneSoluce = false;
        }
    }
}

// Updates a summary of exercise or mission.

void CMainDialog::UpdateSceneResume(int chap, int rank)
{
    CWindow*    pw;
    CEdit*      pe;
    CCheck*     pc;
    std::string fileName;
    int         numTry;
    bool        bPassed, bVisible;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == 0 )  return;
    pe = static_cast<CEdit*>(pw->SearchControl(EVENT_INTERFACE_RESUME));
    if ( pe == 0 )  return;
    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_SOLUCE));

    if ( pc == 0 )
    {
        m_bSceneSoluce = false;
    }
    else
    {
        numTry  = m_main->GetPlayerProfile()->GetLevelTryCount(m_category, chap, rank);
        bPassed = m_main->GetPlayerProfile()->GetLevelPassed(m_category, chap, rank);
        bVisible = ( numTry > 2 || bPassed || m_main->GetShowSoluce() );
        if ( !GetSoluce4() )  bVisible = false;
        pc->SetState(STATE_VISIBLE, bVisible);
        if ( !bVisible )
        {
            pc->ClearState(STATE_CHECK);
            m_bSceneSoluce = false;
        }
    }

    if(chap == 0 || rank == 0) return;

    try
    {
        CLevelParser levelParser(m_category, chap, rank);
        levelParser.Load();
        pe->SetText(levelParser.Get("Resume")->GetParam("text")->AsString().c_str());
    }
    catch (CLevelParserException& e)
    {
        pe->SetText((std::string("[ERROR]: ")+e.what()).c_str());
    }
}

// Updates the list of modes.

int GCD(int a, int b)
{
    return (b == 0) ? a : GCD(b, a%b);
}

Math::IntPoint AspectRatio(Math::IntPoint resolution)
{
    int gcd = GCD(resolution.x, resolution.y);
    return Math::IntPoint(static_cast<float>(resolution.x) / gcd, static_cast<float>(resolution.y) / gcd);
}

void CMainDialog::UpdateDisplayMode()
{
    CWindow*    pw;
    CList*      pl;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == 0 )  return;
    pl = static_cast<CList*>(pw->SearchControl(EVENT_LIST2));
    if ( pl == 0 )  return;
    pl->Flush();

    std::vector<Math::IntPoint> modes;
    m_app->GetVideoResolutionList(modes, true, true);
    int i = 0;
    std::stringstream mode_text;
    for (Math::IntPoint mode : modes)
    {
        mode_text.str("");
        Math::IntPoint aspect = AspectRatio(mode);
        mode_text << mode.x << "x" << mode.y << " [" << aspect.x << ":" << aspect.y << "]";
        pl->SetItemName(i++, mode_text.str().c_str());
    }

    pl->SetSelect(m_setupSelMode);
    pl->ShowSelect(false);
}

// Change the graphics mode.

void CMainDialog::ChangeDisplay()
{
    CWindow*    pw;
    CList*      pl;
    CCheck*     pc;
    bool        bFull;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == 0 )  return;

    pl = static_cast<CList*>(pw->SearchControl(EVENT_LIST2));
    if ( pl == 0 )  return;
    m_setupSelMode = pl->GetSelect();

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_FULL));
    if ( pc == 0 )  return;
    bFull = pc->TestState(STATE_CHECK);
    m_setupFull = bFull;

    SetupMemorize();

    #if !PLATFORM_LINUX
    // Windows causes problems, so we'll restart the game
    // Mac OS was not tested so let's restart just to be sure
    m_app->Restart();
    #else
    std::vector<Math::IntPoint> modes;
    m_app->GetVideoResolutionList(modes, true, true);

    Gfx::DeviceConfig config = m_app->GetVideoConfig();
    config.size = modes[m_setupSelMode];
    config.fullScreen = bFull;
    m_app->ChangeVideoConfig(config);
    #endif
}



// Updates the "apply" button.

void CMainDialog::UpdateApply()
{
    CWindow*    pw;
    CButton*    pb;
    CList*      pl;
    CCheck*     pc;
    int         sel2;
    bool        bFull;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == 0 )  return;

    pb = static_cast<CButton*>(pw->SearchControl(EVENT_INTERFACE_APPLY));

    pl = static_cast<CList*>(pw->SearchControl(EVENT_LIST2));
    if ( pl == 0 )  return;
    sel2 = pl->GetSelect();

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_FULL));
    bFull = pc->TestState(STATE_CHECK);

    if ( sel2 == m_setupSelMode   &&
         bFull == m_setupFull     )
    {
        pb->ClearState(STATE_ENABLE);
    }
    else
    {
        pb->SetState(STATE_ENABLE);
    }
}

// Updates the buttons during the setup phase.

void CMainDialog::UpdateSetupButtons()
{
    CWindow*    pw;
    CCheck*     pc;
    CEditValue* pv;
    CSlider*    ps;
    CEnumSlider* pes;
    float       value;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == 0 )  return;

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_TOTO));
    if ( pc != 0 )
    {
        pc->SetState(STATE_CHECK, m_engine->GetTotoMode());
    }

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_TOOLTIP));
    if ( pc != 0 )
    {
        pc->SetState(STATE_CHECK, m_bTooltip);
    }

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_GLINT));
    if ( pc != 0 )
    {
        pc->SetState(STATE_CHECK, m_bGlint);
    }

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_RAIN));
    if ( pc != 0 )
    {
        pc->SetState(STATE_CHECK, m_bRain);
    }

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_MOUSE));
    if ( pc != 0 )
    {
        pc->SetState(STATE_CHECK, m_app->GetMouseMode() == MOUSE_SYSTEM);
    }

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_EDITMODE));
    if ( pc != 0 )
    {
        pc->SetState(STATE_CHECK, m_engine->GetEditIndentMode());
    }

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_EDITVALUE));
    if ( pc != 0 )
    {
        pc->SetState(STATE_CHECK, m_engine->GetEditIndentValue()>2);
    }

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_SOLUCE4));
    if ( pc != 0 )
    {
        pc->SetState(STATE_CHECK, m_bSoluce4);
    }

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_MOVIES));
    if ( pc != 0 )
    {
        pc->SetState(STATE_CHECK, m_bMovies);
    }

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_NICERST));
    if ( pc != 0 )
    {
        pc->SetState(STATE_CHECK, m_bNiceReset);
    }

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_HIMSELF));
    if ( pc != 0 )
    {
        pc->SetState(STATE_CHECK, m_bHimselfDamage);
    }

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_SCROLL));
    if ( pc != 0 )
    {
        pc->SetState(STATE_CHECK, m_bCameraScroll);
    }

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_INVERTX));
    if ( pc != 0 )
    {
        pc->SetState(STATE_CHECK, m_bCameraInvertX);
    }

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_INVERTY));
    if ( pc != 0 )
    {
        pc->SetState(STATE_CHECK, m_bCameraInvertY);
    }

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_EFFECT));
    if ( pc != 0 )
    {
        pc->SetState(STATE_CHECK, m_bEffect);
    }

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_BLOOD));
    if ( pc != 0 )
    {
        pc->SetState(STATE_CHECK, m_bBlood);
    }

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_AUTOSAVE_ENABLE));
    if ( pc != 0 )
    {
        pc->SetState(STATE_CHECK, m_bAutosave);
    }

    ps = static_cast<CSlider*>(pw->SearchControl(EVENT_INTERFACE_AUTOSAVE_INTERVAL));
    if ( ps != 0 )
    {
        ps->SetState(STATE_ENABLE, m_bAutosave);
        ps->SetVisibleValue(m_main->GetAutosaveInterval());

    }

    ps = static_cast<CSlider*>(pw->SearchControl(EVENT_INTERFACE_AUTOSAVE_SLOTS));
    if ( ps != 0 )
    {
        ps->SetState(STATE_ENABLE, m_bAutosave);
        ps->SetVisibleValue(m_main->GetAutosaveSlots());
    }

    pes = static_cast<CEnumSlider*>(pw->SearchControl(EVENT_INTERFACE_TEXTURE_FILTER));
    if ( pes != 0 )
    {
        pes->SetVisibleValue(m_engine->GetTextureFilterMode());
    }

    pes = static_cast<CEnumSlider*>(pw->SearchControl(EVENT_INTERFACE_TEXTURE_MIPMAP));
    if ( pes != 0 )
    {
        pes->SetState(STATE_ENABLE, m_engine->GetTextureFilterMode() == Gfx::TEX_FILTER_TRILINEAR);
        pes->SetVisibleValue(m_engine->GetTextureMipmapLevel());
    }

    pes = static_cast<CEnumSlider*>(pw->SearchControl(EVENT_INTERFACE_TEXTURE_ANISOTROPY));
    if ( pes != 0 )
    {
        pes->SetVisibleValue(m_engine->GetTextureAnisotropyLevel());
    }

    pes = static_cast<CEnumSlider*>(pw->SearchControl(EVENT_INTERFACE_MSAA));
    if ( pes != 0 )
    {
        pes->SetVisibleValue(m_engine->GetMultiSample());
    }

    pes = static_cast<CEnumSlider*>(pw->SearchControl(EVENT_INTERFACE_SHADOW_MAPPING));
    if ( pes != 0 )
    {
        if (!m_engine->GetShadowMapping())
        {
            pes->SetVisibleValue(-1);
        }
        else if (!m_engine->GetShadowMappingOffscreen())
        {
            pes->SetVisibleValue(0);
        }
        else
        {
            pes->SetVisibleValue(m_engine->GetShadowMappingOffscreenResolution());
        }
    }

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_SHADOW_MAPPING_QUALITY));
    if ( pc != 0 )
    {
        pc->SetState(STATE_ENABLE, m_engine->GetShadowMapping());
        pc->SetState(STATE_CHECK, m_engine->GetShadowMapping() && m_engine->GetShadowMappingQuality());
    }

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_SHADOW));
    if ( pc != 0 )
    {
        pc->SetState(STATE_ENABLE, !m_engine->GetShadowMapping());
        pc->SetState(STATE_CHECK, !m_engine->GetShadowMapping() && m_engine->GetShadow());
    }

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_GROUND));
    if ( pc != 0 )
    {
        pc->SetState(STATE_CHECK, m_engine->GetGroundSpot());
    }

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_DIRTY));
    if ( pc != 0 )
    {
        pc->SetState(STATE_CHECK, m_engine->GetDirty());
    }

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_FOG));
    if ( pc != 0 )
    {
        pc->SetState(STATE_CHECK, m_engine->GetFog());
    }

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_LENS));
    if ( pc != 0 )
    {
        pc->SetState(STATE_CHECK, m_engine->GetLensMode());
    }

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_SKY));
    if ( pc != 0 )
    {
        pc->SetState(STATE_CHECK, m_engine->GetSkyMode());
    }

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_PLANET));
    if ( pc != 0 )
    {
        pc->SetState(STATE_CHECK, m_engine->GetPlanetMode());
    }

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_LIGHT));
    if ( pc != 0 )
    {
        pc->SetState(STATE_CHECK, m_engine->GetLightMode());
    }

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_JOYSTICK));
    if ( pc != 0 )
    {
        pc->SetState(STATE_ENABLE, m_app->GetJoystick().index >= 0);
        pc->SetState(STATE_CHECK, m_app->GetJoystickEnabled());
    }

    pv = static_cast<CEditValue*>(pw->SearchControl(EVENT_INTERFACE_PARTI));
    if ( pv != 0 )
    {
        value = m_engine->GetParticleDensity();
        pv->SetValue(value);
    }

    pv = static_cast<CEditValue*>(pw->SearchControl(EVENT_INTERFACE_CLIP));
    if ( pv != 0 )
    {
        value = m_engine->GetClippingDistance();
        pv->SetValue(value);
    }

    pv = static_cast<CEditValue*>(pw->SearchControl(EVENT_INTERFACE_DETAIL));
    if ( pv != 0 )
    {
        value = m_engine->GetObjectDetail();
        pv->SetValue(value);
    }

    pv = static_cast<CEditValue*>(pw->SearchControl(EVENT_INTERFACE_GADGET));
    if ( pv != 0 )
    {
        value = m_engine->GetGadgetQuantity();
        pv->SetValue(value);
    }

    ps = static_cast<CSlider*>(pw->SearchControl(EVENT_INTERFACE_VOLSOUND));
    if ( ps != 0 )
    {
        value = static_cast<float>(m_sound->GetAudioVolume());
        ps->SetVisibleValue(value);
    }

    ps = static_cast<CSlider*>(pw->SearchControl(EVENT_INTERFACE_VOLMUSIC));
    if ( ps != 0 )
    {
        value = static_cast<float>(m_sound->GetMusicVolume());
        ps->SetVisibleValue(value);
    }
}

// Updates the engine function of the buttons after the setup phase.

void CMainDialog::ChangeSetupButtons()
{
    CWindow*    pw;
    CEditValue* pv;
    CSlider*    ps;
    CEnumSlider* pes;
    float       value;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == 0 )  return;

    pv = static_cast<CEditValue*>(pw->SearchControl(EVENT_INTERFACE_PARTI));
    if ( pv != 0 )
    {
        value = pv->GetValue();
        m_engine->SetParticleDensity(value);
    }

    pv = static_cast<CEditValue*>(pw->SearchControl(EVENT_INTERFACE_CLIP));
    if ( pv != 0 )
    {
        value = pv->GetValue();
        m_engine->SetClippingDistance(value);
    }

    pv = static_cast<CEditValue*>(pw->SearchControl(EVENT_INTERFACE_DETAIL));
    if ( pv != 0 )
    {
        value = pv->GetValue();
        m_engine->SetObjectDetail(value);
    }

    pv = static_cast<CEditValue*>(pw->SearchControl(EVENT_INTERFACE_GADGET));
    if ( pv != 0 )
    {
        value = pv->GetValue();
        m_engine->SetGadgetQuantity(value);
    }

    ps = static_cast<CSlider*>(pw->SearchControl(EVENT_INTERFACE_VOLSOUND));
    if ( ps != 0 )
    {
        value = ps->GetVisibleValue();
        m_sound->SetAudioVolume(static_cast<int>(value));
    }

    ps = static_cast<CSlider*>(pw->SearchControl(EVENT_INTERFACE_VOLMUSIC));
    if ( ps != 0 )
    {
        value = ps->GetVisibleValue();
        m_sound->SetMusicVolume(static_cast<int>(value));
    }

    ps = static_cast<CSlider*>(pw->SearchControl(EVENT_INTERFACE_AUTOSAVE_INTERVAL));
    if ( ps != 0 )
    {
        value = ps->GetVisibleValue();
        m_main->SetAutosaveInterval(static_cast<int>(round(value)));
    }

    ps = static_cast<CSlider*>(pw->SearchControl(EVENT_INTERFACE_AUTOSAVE_SLOTS));
    if ( ps != 0 )
    {
        value = ps->GetVisibleValue();
        m_main->SetAutosaveSlots(static_cast<int>(round(value)));
    }

    pes = static_cast<CEnumSlider*>(pw->SearchControl(EVENT_INTERFACE_TEXTURE_FILTER));
    if ( pes != 0 )
    {
        value = pes->GetVisibleValue();
        m_engine->SetTextureFilterMode(static_cast<Gfx::TexFilter>(value));
    }

    pes = static_cast<CEnumSlider*>(pw->SearchControl(EVENT_INTERFACE_TEXTURE_MIPMAP));
    if ( pes != 0 )
    {
        value = pes->GetVisibleValue();
        m_engine->SetTextureMipmapLevel(static_cast<int>(value));
    }

    pes = static_cast<CEnumSlider*>(pw->SearchControl(EVENT_INTERFACE_TEXTURE_ANISOTROPY));
    if ( pes != 0 )
    {
        value = pes->GetVisibleValue();
        m_engine->SetTextureAnisotropyLevel(static_cast<int>(value));
    }

    pes = static_cast<CEnumSlider*>(pw->SearchControl(EVENT_INTERFACE_MSAA));
    if ( pes != 0 )
    {
        value = pes->GetVisibleValue();
        m_engine->SetMultiSample(static_cast<int>(value));
    }

    pes = static_cast<CEnumSlider*>(pw->SearchControl(EVENT_INTERFACE_SHADOW_MAPPING));
    if ( pes != 0 )
    {
        value = pes->GetVisibleValue();
        if(value == -1)
        {
            m_engine->SetShadowMapping(false);
        }
        else if(value == 0)
        {
            m_engine->SetShadowMapping(true);
            m_engine->SetShadowMappingOffscreen(false);
        }
        else
        {
            m_engine->SetShadowMapping(true);
            m_engine->SetShadowMappingOffscreen(true);
            m_engine->SetShadowMappingOffscreenResolution(value);
        }
    }
}


// Memorizes all the settings.

void CMainDialog::SetupMemorize()
{
    GetConfigFile().SetIntProperty("Setup", "Tooltips", m_bTooltip);
    GetConfigFile().SetIntProperty("Setup", "InterfaceGlint", m_bGlint);
    GetConfigFile().SetIntProperty("Setup", "InterfaceGlint", m_bRain);
    GetConfigFile().SetIntProperty("Setup", "Soluce4", m_bSoluce4);
    GetConfigFile().SetIntProperty("Setup", "Movies", m_bMovies);
    GetConfigFile().SetIntProperty("Setup", "NiceReset", m_bNiceReset);
    GetConfigFile().SetIntProperty("Setup", "HimselfDamage", m_bHimselfDamage);
    GetConfigFile().SetIntProperty("Setup", "CameraScroll", m_bCameraScroll);
    GetConfigFile().SetIntProperty("Setup", "CameraInvertX", m_bCameraInvertX);
    GetConfigFile().SetIntProperty("Setup", "CameraInvertY", m_bCameraInvertY);
    GetConfigFile().SetIntProperty("Setup", "InterfaceEffect", m_bEffect);
    GetConfigFile().SetIntProperty("Setup", "Blood", m_bBlood);
    GetConfigFile().SetIntProperty("Setup", "Autosave", m_bAutosave);
    GetConfigFile().SetIntProperty("Setup", "AutosaveInterval", m_main->GetAutosaveInterval());
    GetConfigFile().SetIntProperty("Setup", "AutosaveSlots", m_main->GetAutosaveSlots());
    GetConfigFile().SetIntProperty("Setup", "GroundShadow", m_engine->GetShadow());
    GetConfigFile().SetIntProperty("Setup", "GroundSpot", m_engine->GetGroundSpot());
    GetConfigFile().SetIntProperty("Setup", "ObjectDirty", m_engine->GetDirty());
    GetConfigFile().SetIntProperty("Setup", "FogMode", m_engine->GetFog());
    GetConfigFile().SetIntProperty("Setup", "LensMode", m_engine->GetLensMode());
    GetConfigFile().SetIntProperty("Setup", "SkyMode", m_engine->GetSkyMode());
    GetConfigFile().SetIntProperty("Setup", "PlanetMode", m_engine->GetPlanetMode());
    GetConfigFile().SetIntProperty("Setup", "LightMode", m_engine->GetLightMode());
    GetConfigFile().SetIntProperty("Setup", "UseJoystick", m_app->GetJoystickEnabled() ? m_app->GetJoystick().index : -1);
    GetConfigFile().SetFloatProperty("Setup", "ParticleDensity", m_engine->GetParticleDensity());
    GetConfigFile().SetFloatProperty("Setup", "ClippingDistance", m_engine->GetClippingDistance());
    GetConfigFile().SetFloatProperty("Setup", "ObjectDetail", m_engine->GetObjectDetail());
    GetConfigFile().SetFloatProperty("Setup", "GadgetQuantity", m_engine->GetGadgetQuantity());
    GetConfigFile().SetIntProperty("Setup", "TotoMode", m_engine->GetTotoMode());
    GetConfigFile().SetIntProperty("Setup", "AudioVolume", m_sound->GetAudioVolume());
    GetConfigFile().SetIntProperty("Setup", "MusicVolume", m_sound->GetMusicVolume());
    GetConfigFile().SetIntProperty("Setup", "EditIndentMode", m_engine->GetEditIndentMode());
    GetConfigFile().SetIntProperty("Setup", "EditIndentValue", m_engine->GetEditIndentValue());
    GetConfigFile().SetIntProperty("Setup", "SystemMouse", m_app->GetMouseMode() == MOUSE_SYSTEM);

    GetConfigFile().SetIntProperty("Setup", "MipmapLevel", m_engine->GetTextureMipmapLevel());
    GetConfigFile().SetIntProperty("Setup", "Anisotropy", m_engine->GetTextureAnisotropyLevel());
    GetConfigFile().SetFloatProperty("Setup", "ShadowColor", m_engine->GetShadowColor());
    GetConfigFile().SetFloatProperty("Setup", "ShadowRange", m_engine->GetShadowRange());
    GetConfigFile().SetIntProperty("Setup", "MSAA", m_engine->GetMultiSample());
    GetConfigFile().SetIntProperty("Setup", "FilterMode", m_engine->GetTextureFilterMode());
    GetConfigFile().SetIntProperty("Setup", "ShadowMapping", m_engine->GetShadowMapping());
    GetConfigFile().SetIntProperty("Setup", "ShadowMappingQuality", m_engine->GetShadowMappingQuality());
    GetConfigFile().SetIntProperty("Setup", "ShadowMappingResolution", m_engine->GetShadowMappingOffscreen() ? m_engine->GetShadowMappingOffscreenResolution() : 0);

    /* screen setup */
    GetConfigFile().SetIntProperty("Setup", "Fullscreen", m_setupFull ? 1 : 0);

    CList *pl;
    CWindow *pw;
    pw = static_cast<CWindow *>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw != 0 )
    {
        pl = static_cast<CList *>(pw->SearchControl(EVENT_LIST2));
        if ( pl != 0 )
        {
            std::vector<Math::IntPoint> modes;
            m_app->GetVideoResolutionList(modes, true, true);
            std::ostringstream ss;
            ss << modes[m_setupSelMode].x << "x" << modes[m_setupSelMode].y;
            GetConfigFile().SetStringProperty("Setup", "Resolution", ss.str());
        }
    }

    CInput::GetInstancePointer()->SaveKeyBindings();

    GetConfigFile().SetIntProperty("Setup", "DeleteGamer", m_bDeleteGamer);

    GetConfigFile().Save();
}

// Remember all the settings.

void CMainDialog::SetupRecall()
{
    float       fValue;
    int         iValue;
    std::string key;

    if ( GetConfigFile().GetIntProperty("Setup", "TotoMode", iValue) )
    {
        m_engine->SetTotoMode(iValue);
    }

    if ( GetConfigFile().GetIntProperty("Setup", "Tooltips", iValue) )
    {
        m_bTooltip = iValue;
    }

    if ( GetConfigFile().GetIntProperty("Setup", "InterfaceGlint", iValue) )
    {
        m_bGlint = iValue;
    }

    if ( GetConfigFile().GetIntProperty("Setup", "InterfaceGlint", iValue) )
    {
        m_bRain = iValue;
    }

    if ( GetConfigFile().GetIntProperty("Setup", "SystemMouse", iValue) )
    {
        m_app->SetMouseMode(iValue ? MOUSE_SYSTEM : MOUSE_ENGINE);
    }

    if ( GetConfigFile().GetIntProperty("Setup", "Soluce4", iValue) )
    {
        m_bSoluce4 = iValue;
    }

    if ( GetConfigFile().GetIntProperty("Setup", "Movies", iValue) )
    {
        m_bMovies = iValue;
    }

    if ( GetConfigFile().GetIntProperty("Setup", "NiceReset", iValue) )
    {
        m_bNiceReset = iValue;
    }

    if ( GetConfigFile().GetIntProperty("Setup", "HimselfDamage", iValue) )
    {
        m_bHimselfDamage = iValue;
    }

    if ( GetConfigFile().GetIntProperty("Setup", "CameraScroll", iValue) )
    {
        m_bCameraScroll = iValue;
        m_camera->SetCameraScroll(m_bCameraScroll);
    }

    if ( GetConfigFile().GetIntProperty("Setup", "CameraInvertX", iValue) )
    {
        m_bCameraInvertX = iValue;
        m_camera->SetCameraInvertX(m_bCameraInvertX);
    }

    if ( GetConfigFile().GetIntProperty("Setup", "CameraInvertY", iValue) )
    {
        m_bCameraInvertY = iValue;
        m_camera->SetCameraInvertY(m_bCameraInvertY);
    }

    if ( GetConfigFile().GetIntProperty("Setup", "InterfaceEffect", iValue) )
    {
        m_bEffect = iValue;
    }

    if ( GetConfigFile().GetIntProperty("Setup", "Blood", iValue) )
    {
        m_bBlood = iValue;
    }

    if ( GetConfigFile().GetIntProperty("Setup", "Autosave", iValue) )
    {
        m_bAutosave = iValue;
    }

    if ( GetConfigFile().GetIntProperty("Setup", "AutosaveInterval", iValue) )
    {
        m_main->SetAutosaveInterval(iValue);
    }

    if ( GetConfigFile().GetIntProperty("Setup", "AutosaveSlots", iValue) )
    {
        m_main->SetAutosaveSlots(iValue);
    }

    if ( GetConfigFile().GetIntProperty("Setup", "GroundShadow", iValue) )
    {
        m_engine->SetShadow(iValue);
    }

    if ( GetConfigFile().GetIntProperty("Setup", "GroundSpot", iValue) )
    {
        m_engine->SetGroundSpot(iValue);
    }

    if ( GetConfigFile().GetIntProperty("Setup", "ObjectDirty", iValue) )
    {
        m_engine->SetDirty(iValue);
    }

    if ( GetConfigFile().GetIntProperty("Setup", "FogMode", iValue) )
    {
        m_engine->SetFog(iValue);
        m_camera->SetOverBaseColor(Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f)); // TODO: color ok?
    }

    if ( GetConfigFile().GetIntProperty("Setup", "LensMode", iValue) )
    {
        m_engine->SetLensMode(iValue);
    }

    if ( GetConfigFile().GetIntProperty("Setup", "SkyMode", iValue) )
    {
        m_engine->SetSkyMode(iValue);
    }

    if ( GetConfigFile().GetIntProperty("Setup", "PlanetMode", iValue) )
    {
        m_engine->SetPlanetMode(iValue);
    }

    if ( GetConfigFile().GetIntProperty("Setup", "LightMode", iValue) )
    {
        m_engine->SetLightMode(iValue);
    }

    if ( GetConfigFile().GetIntProperty("Setup", "UseJoystick", iValue) )
    {
        if(iValue >= 0)
        {
            auto joysticks = m_app->GetJoystickList();
            for(const auto& joystick : joysticks)
            {
                if (joystick.index == iValue)
                {
                    m_app->ChangeJoystick(joystick);
                    m_app->SetJoystickEnabled(true);
                }
            }
        }
        else
        {
            m_app->SetJoystickEnabled(false);
        }
    }

    if ( GetConfigFile().GetFloatProperty("Setup", "ParticleDensity", fValue) )
    {
        m_engine->SetParticleDensity(fValue);
    }

    if ( GetConfigFile().GetFloatProperty("Setup", "ClippingDistance", fValue) )
    {
        m_engine->SetClippingDistance(fValue);
    }

    if ( GetConfigFile().GetFloatProperty("Setup", "ObjectDetail", fValue) )
    {
        m_engine->SetObjectDetail(fValue);
    }

    if ( GetConfigFile().GetFloatProperty("Setup", "GadgetQuantity", fValue) )
    {
        m_engine->SetGadgetQuantity(fValue);
    }

    if ( GetConfigFile().GetIntProperty("Setup", "AudioVolume", iValue) )
    {
        m_sound->SetAudioVolume(iValue);
    }

    if ( GetConfigFile().GetIntProperty("Setup", "MusicVolume", iValue) )
    {
        m_sound->SetMusicVolume(iValue);
    }

    if ( GetConfigFile().GetIntProperty("Setup", "EditIndentMode", iValue) )
    {
        m_engine->SetEditIndentMode(iValue);
    }

    if ( GetConfigFile().GetIntProperty("Setup", "EditIndentValue", iValue) )
    {
        m_engine->SetEditIndentValue(iValue);
    }

    CInput::GetInstancePointer()->LoadKeyBindings();

    if ( GetConfigFile().GetIntProperty("Setup", "DeleteGamer", iValue) )
    {
        m_bDeleteGamer = iValue;
    }

    if ( GetConfigFile().GetStringProperty("Setup", "Resolution", key) )
    {
        std::istringstream resolution(key);
        std::string ws, hs;
        std::getline(resolution, ws, 'x');
        std::getline(resolution, hs, 'x');
        int w = 800, h = 600;
        if (!ws.empty() && !hs.empty())
        {
            w = atoi(ws.c_str());
            h = atoi(hs.c_str());
        }

        std::vector<Math::IntPoint> modes;
        m_app->GetVideoResolutionList(modes, true, true);
        for (auto it = modes.begin(); it != modes.end(); ++it)
        {
            if (it->x == w && it->y == h)
            {
                m_setupSelMode = it - modes.begin();
                break;
            }
        }
    }

    if ( GetConfigFile().GetIntProperty("Setup", "Fullscreen", iValue) )
    {
        m_setupFull = (iValue == 1);
    }

    if ( GetConfigFile().GetIntProperty("Setup", "MipmapLevel", iValue))
    {
        m_engine->SetTextureMipmapLevel(iValue);
    }

    if (GetConfigFile().GetIntProperty("Setup", "Anisotropy", iValue))
    {
        m_engine->SetTextureAnisotropyLevel(iValue);
    }

    if (GetConfigFile().GetIntProperty("Setup", "ShadowMapping", iValue))
    {
        m_engine->SetShadowMapping(iValue);
    }

    if (GetConfigFile().GetIntProperty("Setup", "ShadowMappingQuality", iValue))
    {
        m_engine->SetShadowMappingQuality(iValue);
    }

    if (GetConfigFile().GetIntProperty("Setup", "ShadowMappingResolution", iValue))
    {
        if(iValue == 0)
        {
            m_engine->SetShadowMappingOffscreen(false);
        }
        else
        {
            m_engine->SetShadowMappingOffscreen(true);
            m_engine->SetShadowMappingOffscreenResolution(iValue);
        }
    }

    if (GetConfigFile().GetFloatProperty("Setup", "ShadowColor", fValue))
    {
        m_engine->SetShadowColor(fValue);
    }

    if (GetConfigFile().GetFloatProperty("Setup", "ShadowRange", fValue))
    {
        m_engine->SetShadowRange(fValue);
    }

    if (GetConfigFile().GetIntProperty("Setup", "MSAA", iValue))
    {
        m_engine->SetMultiSample(iValue);
    }

    if (GetConfigFile().GetIntProperty("Setup", "FilterMode", iValue))
    {
        m_engine->SetTextureFilterMode(static_cast<Gfx::TexFilter>(iValue));
    }
}


// Changes the general level of quality.

void CMainDialog::ChangeSetupQuality(int quality)
{
    bool    bEnable;
    float   value;

    bEnable = true; //(quality >= 0);
    m_engine->SetShadow(bEnable);
    m_engine->SetGroundSpot(bEnable);
    m_engine->SetDirty(bEnable);
    m_engine->SetFog(bEnable);
    m_engine->SetLensMode(bEnable);
    m_engine->SetSkyMode(bEnable);
    m_engine->SetPlanetMode(bEnable);
    m_engine->SetLightMode(bEnable);
    m_camera->SetOverBaseColor(Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f)); // TODO: color ok?

    if ( quality <  0 )  value = 0.0f;
    if ( quality == 0 )  value = 1.0f;
    if ( quality >  0 )  value = 2.0f;
    m_engine->SetParticleDensity(value);

    if ( quality <  0 )  value = 0.5f;
    if ( quality == 0 )  value = 1.0f;
    if ( quality >  0 )  value = 2.0f;
    m_engine->SetClippingDistance(value);

    if ( quality <  0 )  value = 0.0f;
    if ( quality == 0 )  value = 1.0f;
    if ( quality >  0 )  value = 2.0f;
    m_engine->SetObjectDetail(value);

    if ( quality <  0 )  value = 0.5f;
    if ( quality == 0 )  value = 1.0f;
    if ( quality >  0 )  value = 1.0f;
    m_engine->SetGadgetQuantity(value);

    if ( quality <  0 ) m_engine->SetMultiSample(1);
    if ( quality == 0 ) m_engine->SetMultiSample(2);
    if ( quality >  0 ) m_engine->SetMultiSample(4);

    if ( quality <  0 ) { m_engine->SetTextureFilterMode(Gfx::TEX_FILTER_BILINEAR); }
    if ( quality == 0 ) { m_engine->SetTextureFilterMode(Gfx::TEX_FILTER_TRILINEAR); m_engine->SetTextureMipmapLevel(4); m_engine->SetTextureAnisotropyLevel(4); }
    if ( quality >  0 ) { m_engine->SetTextureFilterMode(Gfx::TEX_FILTER_TRILINEAR); m_engine->SetTextureMipmapLevel(8); m_engine->SetTextureAnisotropyLevel(8); }

    if ( quality <  0 ) { m_engine->SetShadowMapping(false); m_engine->SetShadowMappingQuality(false); }
    else { m_engine->SetShadowMapping(true); m_engine->SetShadowMappingQuality(true); m_engine->SetShadowMappingOffscreen(true); }
    if ( quality == 0 ) m_engine->SetShadowMappingOffscreenResolution(1024);
    if ( quality >  0 ) m_engine->SetShadowMappingOffscreenResolution(2048);

    // TODO: first execute adapt?
    //m_engine->FirstExecuteAdapt(false);
}

// Updates the list of keys.

void CMainDialog::UpdateKey()
{
    CWindow* pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if (pw == nullptr) return;

    CScroll* ps = static_cast<CScroll*>(pw->SearchControl(EVENT_INTERFACE_KSCROLL));
    if (ps == nullptr) return;

    int first = static_cast<int>(ps->GetVisibleValue()*(INPUT_SLOT_MAX-KEY_VISIBLE));

    for (int i = 0; i < INPUT_SLOT_MAX; i++)
        pw->DeleteControl(static_cast<EventType>(EVENT_INTERFACE_KEY+i));

    Math::Point dim;
    dim.x = 400.0f/640.0f;
    dim.y =  20.0f/480.0f;
    Math::Point pos;
    pos.x = 110.0f/640.0f;
    pos.y = 168.0f/480.0f + dim.y*(KEY_VISIBLE-1);
    for (int i = 0; i < KEY_VISIBLE; i++)
    {
        pw->CreateKey(pos, dim, -1, static_cast<EventType>(EVENT_INTERFACE_KEY+first+i));
        CKey* pk = static_cast<CKey*>(pw->SearchControl(static_cast<EventType>(EVENT_INTERFACE_KEY+first+i)));
        if (pk == nullptr) break;

        pk->SetBinding(CInput::GetInstancePointer()->GetInputBinding(static_cast<InputSlot>(first+i)));
        pos.y -= dim.y;
    }
}

// Change a key.

void CMainDialog::ChangeKey(EventType event)
{
    CWindow* pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if (pw == nullptr) return;

    CScroll* ps = static_cast<CScroll*>(pw->SearchControl(EVENT_INTERFACE_KSCROLL));
    if (ps == nullptr) return;

    for (int i = 0; i < INPUT_SLOT_MAX; i++)
    {
        if ( EVENT_INTERFACE_KEY+i == event )
        {
            CKey* pk = static_cast<CKey*>(pw->SearchControl(static_cast<EventType>(EVENT_INTERFACE_KEY+i)));
            if (pk == nullptr) break;

            CInput::GetInstancePointer()->SetInputBinding(static_cast<InputSlot>(i), pk->GetBinding());
        }
    }
}



// Do you want to quit the current mission?

void CMainDialog::StartAbort()
{
    CWindow*    pw;
    CButton*    pb;
    Math::Point     pos, dim;
    std::string name;

    StartDialog(Math::Point(0.3f, 0.8f), true, false, false);
    m_bDialogDelete = false;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW9));
    if ( pw == 0 )  return;

    pos.x = 0.35f;
    pos.y = 0.60f;
    dim.x = 0.30f;
    dim.y = 0.30f;
    pw->CreateGroup(pos, dim, 5, EVENT_INTERFACE_GLINTl);  // orange corner
    pos.x = 0.35f;
    pos.y = 0.10f;
    dim.x = 0.30f;
    dim.y = 0.30f;
    pw->CreateGroup(pos, dim, 4, EVENT_INTERFACE_GLINTr);  // blue corner

    pos.x = 0.40f;
    dim.x = 0.20f;
    dim.y = 32.0f/480.0f;

    pos.y = 0.74f;
    pb = pw->CreateButton(pos, dim, -1, EVENT_DIALOG_CANCEL);
    pb->SetState(STATE_SHADOW);
    GetResource(RES_TEXT, RT_DIALOG_NO, name);
    pb->SetName(name);

    if ( m_category == LevelCategory::Missions     ||  // missions ?
         m_category == LevelCategory::FreeGame     ||  // free games?
         m_category == LevelCategory::CustomLevels )   // user ?
    {
        pos.y = 0.62f;
        pb = pw->CreateButton(pos, dim, -1, EVENT_INTERFACE_WRITE);
        pb->SetState(STATE_SHADOW);
        if ( m_main->IsBusy() )  // current task?
        {
            pb->ClearState(STATE_ENABLE);
        }

        pos.y = 0.53f;
        pb = pw->CreateButton(pos, dim, -1, EVENT_INTERFACE_READ);
        pb->SetState(STATE_SHADOW);
        if ( !IsIOReadScene() )  // no file to read?
        {
            pb->ClearState(STATE_ENABLE);
        }
        pb->SetState(STATE_WARNING);
    }

    pos.y = 0.39f;
    pb = pw->CreateButton(pos, dim, -1, EVENT_INTERFACE_SETUP);
    pb->SetState(STATE_SHADOW);

    pos.y = 0.25f;
    pb = pw->CreateButton(pos, dim, -1, EVENT_INTERFACE_AGAIN);
    pb->SetState(STATE_SHADOW);
    pb->SetState(STATE_WARNING);

    pos.y = 0.16f;
    pb = pw->CreateButton(pos, dim, -1, EVENT_DIALOG_OK);
    pb->SetState(STATE_SHADOW);
    pb->SetState(STATE_WARNING);
    GetResource(RES_TEXT, RT_DIALOG_YES, name);
    pb->SetName(name);
}

// Do you want to destroy the building?

void CMainDialog::StartDeleteObject()
{
    CWindow*    pw;
    CButton*    pb;
    Math::Point     pos, dim;
    std::string name;

    StartDialog(Math::Point(0.7f, 0.3f), false, true, true);
    m_bDialogDelete = true;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW9));
    if ( pw == 0 )  return;

    pos.x = 0.00f;
    pos.y = 0.50f;
    dim.x = 1.00f;
    dim.y = 0.05f;
    GetResource(RES_TEXT, RT_DIALOG_DELOBJ, name);
    pw->CreateLabel(pos, dim, -1, EVENT_DIALOG_LABEL, name);

    pb = static_cast<CButton*>(pw->SearchControl(EVENT_DIALOG_OK));
    if ( pb == 0 )  return;
    GetResource(RES_TEXT, RT_DIALOG_YESDEL, name);
    pb->SetName(name);
    pb->SetState(STATE_WARNING);

    pb = static_cast<CButton*>(pw->SearchControl(EVENT_DIALOG_CANCEL));
    if ( pb == 0 )  return;
    GetResource(RES_TEXT, RT_DIALOG_NODEL, name);
    pb->SetName(name);
}

// Do you want to delete the player?

void CMainDialog::StartDeleteGame(char *gamer)
{
    CWindow*    pw;
    CButton*    pb;
    Math::Point     pos, dim;

    StartDialog(Math::Point(0.7f, 0.3f), false, true, true);
    m_bDialogDelete = true;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW9));
    if (pw == nullptr)
        return;

    std::string name;

    pos.x = 0.00f;
    pos.y = 0.50f;
    dim.x = 1.00f;
    dim.y = 0.05f;
    GetResource(RES_TEXT, RT_DIALOG_DELGAME, name);
    std::string text = StrUtils::Format(name.c_str(), gamer);
    pw->CreateLabel(pos, dim, -1, EVENT_DIALOG_LABEL, text);

    pb = static_cast<CButton*>(pw->SearchControl(EVENT_DIALOG_OK));
    if ( pb == 0 )  return;
    GetResource(RES_TEXT, RT_DIALOG_YESDEL, name);
    pb->SetName(name);
    pb->SetState(STATE_WARNING);

    pb = static_cast<CButton*>(pw->SearchControl(EVENT_DIALOG_CANCEL));
    if ( pb == 0 )  return;
    GetResource(RES_TEXT, RT_DIALOG_NODEL, name);
    pb->SetName(name);
}

// Would you quit the game?

void CMainDialog::StartQuit()
{
    CWindow*    pw;
    CButton*    pb;
    Math::Point     pos, dim;

    StartDialog(Math::Point(0.6f, 0.3f), false, true, true);

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW9));
    if (pw == nullptr)
        return;

    std::string name;

    pos.x = 0.00f;
    pos.y = 0.50f;
    dim.x = 1.00f;
    dim.y = 0.05f;
    GetResource(RES_TEXT, RT_DIALOG_QUIT, name);
    pw->CreateLabel(pos, dim, -1, EVENT_DIALOG_LABEL, name);

    pb = static_cast<CButton*>(pw->SearchControl(EVENT_DIALOG_OK));
    if ( pb == 0 )  return;
    GetResource(RES_TEXT, RT_DIALOG_YESQUIT, name);
    pb->SetName(name);
    pb->SetState(STATE_WARNING);

    pb = static_cast<CButton*>(pw->SearchControl(EVENT_DIALOG_CANCEL));
    if ( pb == 0 )  return;
    GetResource(RES_TEXT, RT_DIALOG_NOQUIT, name);
    pb->SetName(name);
}

// Beginning of displaying a dialog.

void CMainDialog::StartDialog(Math::Point dim, bool bFire, bool bOK, bool bCancel)
{
    CWindow*    pw;
    CButton*    pb;
    Math::Point     pos, ddim;

    StartSuspend();

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW0));
    if ( pw != 0 )  pw->ClearState(STATE_ENABLE);

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW1));
    if ( pw != 0 )  pw->ClearState(STATE_ENABLE);

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW2));
    if ( pw != 0 )  pw->ClearState(STATE_ENABLE);

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW3));
    if ( pw != 0 )  pw->ClearState(STATE_ENABLE);

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW4));
    if ( pw != 0 )  pw->ClearState(STATE_ENABLE);

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw != 0 )  pw->ClearState(STATE_ENABLE);

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW6));
    if ( pw != 0 )  pw->ClearState(STATE_ENABLE);

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW7));
    if ( pw != 0 )  pw->ClearState(STATE_ENABLE);

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW8));
    if ( pw != 0 )  pw->ClearState(STATE_ENABLE);

    m_bDialogFire = bFire;

    std::string name;

    pos.x = (1.0f-dim.x)/2.0f;
    pos.y = (1.0f-dim.y)/2.0f;
    pw = m_interface->CreateWindows(pos, dim, bFire?12:8, EVENT_WINDOW9);
    pw->SetState(STATE_SHADOW);
    GetResource(RES_TEXT, RT_TITLE_BASE, name);
    pw->SetName(name);

    m_dialogPos = pos;
    m_dialogDim = dim;
    m_dialogTime = 0.0f;
    m_dialogParti = 999.0f;

    if ( bOK )
    {
        pos.x  = 0.50f-0.15f-0.02f;
        pos.y  = 0.50f-dim.y/2.0f+0.03f;
        ddim.x = 0.15f;
        ddim.y = 0.06f;
        pb = pw->CreateButton(pos, ddim, -1, EVENT_DIALOG_OK);
        pb->SetState(STATE_SHADOW);
        GetResource(RES_EVENT, EVENT_DIALOG_OK, name);
        pb->SetName(name);
    }

    if ( bCancel )
    {
        pos.x  = 0.50f+0.02f;
        pos.y  = 0.50f-dim.y/2.0f+0.03f;
        ddim.x = 0.15f;
        ddim.y = 0.06f;
        pb = pw->CreateButton(pos, ddim, -1, EVENT_DIALOG_CANCEL);
        pb->SetState(STATE_SHADOW);
        GetResource(RES_EVENT, EVENT_DIALOG_CANCEL, name);
        pb->SetName(name);
    }

    m_sound->Play(SOUND_TZOING);
    m_bDialog = true;
}

// Animation of a dialog.

void CMainDialog::FrameDialog(float rTime)
{
    CWindow*    pw;
    Math::Vector    pos, speed;
    Math::Point     dim, dpos, ddim;
    float       zoom;
    int         i;

    dpos = m_dialogPos;
    ddim = m_dialogDim;

    m_dialogTime += rTime;
    if ( m_dialogTime < 1.0f )
    {
        pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW9));
        if ( pw != 0 )
        {
            if ( m_dialogTime < 0.50f )
            {
                zoom = Math::Bounce(m_dialogTime/0.50f);
            }
            else
            {
                zoom = 1.0f;
            }

            dpos.x += ddim.x/2.0f;
            dpos.y += ddim.y/2.0f;

            ddim.x *= zoom;
//?         ddim.y *= zoom;

            dpos.x -= ddim.x/2.0f;
            dpos.y -= ddim.y/2.0f;

            pw->SetPos(dpos);
            pw->SetDim(ddim);
        }
    }

    if ( !m_bGlint )  return;

    m_dialogParti += rTime;
    if ( m_dialogParti < m_engine->ParticleAdapt(0.05f) )  return;
    m_dialogParti = 0.0f;

    if ( !m_bDialogFire )  return;

    dpos = m_dialogPos;
    ddim = m_dialogDim;

    pos.z = 0.0f;
    speed = Math::Vector(0.0f, 0.0f, 0.0f);

    for ( i=0 ; i<2 ; i++ )
    {
        // Bottom.
        pos.x = dpos.x + ddim.x*Math::Rand();
        pos.y = dpos.y;
        pos.x += (Math::Rand()-0.5f)*(6.0f/640.0f);
        pos.y += Math::Rand()*(16.0f/480.0f)-(10.0f/480.0f);
        dim.x = 0.01f+Math::Rand()*0.01f;
        dim.y = dim.x/0.75f;
        m_particle->CreateParticle(pos, speed, dim,
                                     static_cast<Gfx::ParticleType>(Gfx::PARTILENS1+rand()%3),
                                     1.0f, 0.0f, 0.0f, Gfx::SH_INTERFACE);

        // Top.
        pos.x = dpos.x + ddim.x*Math::Rand();
        pos.y = dpos.y + ddim.y;
        pos.x += (Math::Rand()-0.5f)*(6.0f/640.0f);
        pos.y -= Math::Rand()*(16.0f/480.0f)-(10.0f/480.0f);
        dim.x = 0.01f+Math::Rand()*0.01f;
        dim.y = dim.x/0.75f;
        m_particle->CreateParticle(pos, speed, dim,
                                     static_cast<Gfx::ParticleType>(Gfx::PARTILENS1+rand()%3),
                                     1.0f, 0.0f, 0.0f, Gfx::SH_INTERFACE);

        // Left.
        pos.y = dpos.y + ddim.y*Math::Rand();
        pos.x = dpos.x;
        pos.x += Math::Rand()*(16.0f/640.0f)-(10.0f/640.0f);
        pos.y += (Math::Rand()-0.5f)*(6.0f/480.0f);
        dim.x = 0.01f+Math::Rand()*0.01f;
        dim.y = dim.x/0.75f;
        m_particle->CreateParticle(pos, speed, dim,
                                     static_cast<Gfx::ParticleType>(Gfx::PARTILENS1+rand()%3),
                                     1.0f, 0.0f, 0.0f, Gfx::SH_INTERFACE);

        // Right.
        pos.y = dpos.y + ddim.y*Math::Rand();
        pos.x = dpos.x + ddim.x;
        pos.x -= Math::Rand()*(16.0f/640.0f)-(10.0f/640.0f);
        pos.y += (Math::Rand()-0.5f)*(6.0f/480.0f);
        dim.x = 0.01f+Math::Rand()*0.01f;
        dim.y = dim.x/0.75f;
        m_particle->CreateParticle(pos, speed, dim,
                                     static_cast<Gfx::ParticleType>(Gfx::PARTILENS1+rand()%3),
                                     1.0f, 0.0f, 0.0f, Gfx::SH_INTERFACE);
    }
}

// End of the display of a dialogue.

void CMainDialog::StopDialog()
{
    CWindow*    pw;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW0));
    if ( pw != 0 )  pw->SetState(STATE_ENABLE);

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW1));
    if ( pw != 0 )  pw->SetState(STATE_ENABLE);

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW2));
    if ( pw != 0 )  pw->SetState(STATE_ENABLE);

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW3));
    if ( pw != 0 )  pw->SetState(STATE_ENABLE);

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW4));
    if ( pw != 0 )  pw->SetState(STATE_ENABLE);

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw != 0 )  pw->SetState(STATE_ENABLE);

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW6));
    if ( pw != 0 )  pw->SetState(STATE_ENABLE);

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW7));
    if ( pw != 0 )  pw->SetState(STATE_ENABLE);

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW8));
    if ( pw != 0 )  pw->SetState(STATE_ENABLE);

    StopSuspend();
    m_interface->DeleteControl(EVENT_WINDOW9);
    m_bDialog = false;
}

// Suspends the simulation for a dialog phase.

void CMainDialog::StartSuspend()
{
    m_sound->MuteAll(true);
    m_main->ClearInterface();
    m_bInitPause = m_pause->GetPauseType();
    m_pause->SetPause(PAUSE_DIALOG);
    m_engine->SetOverFront(false);  // over flat behind
    m_main->CreateShortcuts();
    m_main->StartSuspend();
    m_initCamera = m_camera->GetType();
    m_camera->SetType(Gfx::CAM_TYPE_DIALOG);
}

// Resume the simulation after a period of dialog.

void CMainDialog::StopSuspend()
{
    m_sound->MuteAll(false);
    m_main->ClearInterface();
    m_pause->SetPause(m_bInitPause);
    m_engine->SetOverFront(true);  // over flat front
    m_main->CreateShortcuts();
    m_main->StopSuspend();
    m_camera->SetType(m_initCamera);
}


// Whether to use tooltips.

bool CMainDialog::GetTooltip()
{
    return m_bTooltip;
}

// Specifies whether a dialog is displayed.

bool CMainDialog::IsDialog()
{
    return m_bDialog;
}




// Specifies the name of the scene to read.

void CMainDialog::SetSceneRead(const char* name)
{
    m_sceneRead = name;
}

// Returns the name of the scene to read.

std::string & CMainDialog::GetSceneRead()
{
    return m_sceneRead;
}

// Specifies the name of the scene to read.

void CMainDialog::SetStackRead(const char* name)
{
    m_stackRead = name;
}

// Returns the name of the scene to read.

std::string & CMainDialog::GetStackRead()
{
    return m_stackRead;
}


void CMainDialog::SetLevel(LevelCategory cat, int chap, int rank)
{
    m_category = cat;
    m_levelChap = chap;
    m_levelRank = rank;
}

LevelCategory CMainDialog::GetLevelCategory()
{
    return m_category;
}

int CMainDialog::GetLevelChap()
{
    return m_levelChap;
}

int CMainDialog::GetLevelRank()
{
    return m_levelRank;
}

// Returns folder name of the scene that user selected to play.

std::string CMainDialog::GetCustomLevelDir()
{
    if (m_levelChap-1 < 0 || m_levelChap-1 >= m_customLevelList.size())  return "";
    return m_customLevelList[m_levelChap-1];
}

// Whether to show the solution.

bool CMainDialog::GetSceneSoluce()
{
    return m_bSceneSoluce;
}


// Indicates if there are reflections on the buttons.

bool CMainDialog::GetGlint()
{
    return m_bGlint;
}

// Whether to show 4:solutions.

bool CMainDialog::GetSoluce4()
{
    return m_bSoluce4;
}

// Whether to show the cinematics.

bool CMainDialog::GetMovies()
{
    return m_bMovies;
}

// Whether to make an animation on reset.

bool CMainDialog::GetNiceReset()
{
    return m_bNiceReset;
}

// Indicates whether the fire causes damage to its own units.

bool CMainDialog::GetHimselfDamage()
{
    return m_bHimselfDamage;
}

bool CMainDialog::GetGamerOnlyHead()
{
    return (m_phase == PHASE_APPERANCE && m_apperanceTab == 0);
}

float CMainDialog::GetPersoAngle()
{
    return m_apperanceAngle;
}

void CMainDialog::UpdateChapterPassed()
{
    // TODO: CMainDialog is a bad place for this function
    bool bAll = true;
    for ( int i=0 ; i<m_maxList ; i++ )
    {
        if (!m_main->GetPlayerProfile()->GetLevelPassed(m_category, m_chap[m_category]+1, i+1))
        {
            bAll = false;
            break;
        }
    }
    m_main->GetPlayerProfile()->IncrementLevelTryCount(m_category, m_chap[m_category]+1, 0);
    m_main->GetPlayerProfile()->SetLevelPassed(m_category, m_chap[m_category]+1, 0, bAll);
}


// Passes to the next mission, and possibly in the next chapter.

bool CMainDialog::NextMission()
{
    m_sel[m_category] ++;  // next mission

    if ( m_sel[m_category] >= m_maxList )  // last mission of the chapter?
    {
        m_chap[m_category] ++;  // next chapter
        m_sel[m_category] = 0;  // first mission
    }

    m_main->GetPlayerProfile()->SetSelectedChap(m_category, m_chap[m_category]+1);
    m_main->GetPlayerProfile()->SetSelectedRank(m_category, m_sel[m_category]+1);

    return true;
}

//TODO: Userlevel management should be probably moved out of CMainDialog, along with the listing of normal missions
void CMainDialog::UpdateCustomLevelList()
{
    auto userLevelDirs = CResourceManager::ListDirectories("levels/custom/");
    std::sort(userLevelDirs.begin(), userLevelDirs.end());
    m_customLevelList = userLevelDirs;
}

std::string CMainDialog::GetCustomLevelName(int id)
{
    if(id < 1 || id > m_customLevelList.size()) return "";
    return m_customLevelList[id-1];
}


} // namespace Ui
