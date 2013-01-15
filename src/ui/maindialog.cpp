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

// maindialog.cpp

#include "ui/maindialog.h"

#include "app/app.h"
#include "common/global.h"
#include "common/event.h"
#include "common/logger.h"
#include "common/misc.h"
#include "common/profile.h"
#include "common/iman.h"
#include "common/restext.h"
#include "common/logger.h"
#include "object/robotmain.h"
#include "script/cmdtoken.h"
#include "sound/sound.h"
#include "ui/interface.h"
#include "ui/button.h"
#include "ui/color.h"
#include "ui/check.h"
#include "ui/key.h"
#include "ui/group.h"
#include "ui/image.h"
#include "ui/scroll.h"
#include "ui/slider.h"
#include "ui/list.h"
#include "ui/label.h"
#include "ui/window.h"
#include "ui/edit.h"
#include "ui/editvalue.h"

#include <stdio.h>
#include <string.h>
#include <sstream>
#include <iomanip>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>

//TODO Get rid of all sprintf's

namespace Ui
{

const int KEY_VISIBLE = 6;      // number of visible keys redefinable

const int KEY_TOTAL = 21;       // total number of keys redefinable

const float WELCOME_LENGTH = 2.0f;

const int MAX_FNAME = 255; // TODO: remove after rewrite to std::string

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

namespace fs = boost::filesystem;

// Constructor of robot application.

CMainDialog::CMainDialog(CInstanceManager* iMan)
{
    m_iMan = iMan;
    m_iMan->AddInstance(CLASS_DIALOG, this);

    m_app = CApplication::GetInstancePointer();

    m_main       = static_cast<CRobotMain*>(m_iMan->SearchInstance(CLASS_MAIN));
    m_interface  = static_cast<CInterface*>(m_iMan->SearchInstance(CLASS_INTERFACE));
    m_eventQueue = static_cast<CEventQueue*>(m_iMan->SearchInstance(CLASS_EVENT));
    m_engine     = static_cast<Gfx::CEngine*>(m_iMan->SearchInstance(CLASS_ENGINE));
    m_particle   = static_cast<Gfx::CParticle*>(m_iMan->SearchInstance(CLASS_PARTICULE));
    m_camera     = static_cast<Gfx::CCamera*>(m_iMan->SearchInstance(CLASS_CAMERA));
    m_sound      = static_cast<CSoundInterface*>(m_iMan->SearchInstance(CLASS_SOUND));

    m_phase        = PHASE_NAME;
    m_phaseSetup   = PHASE_SETUPg;
    m_phaseTerm    = PHASE_TRAINER;
    m_sceneRead[0] = 0;
    m_stackRead[0] = 0;
    m_sceneName[0] = 0;
    m_sceneRank    = 0;
    m_bSceneSoluce = false;
    m_bSimulSetup  = false;

    m_accessEnable = true;
    m_accessMission= true;
    m_accessUser   = true;

    m_bDeleteGamer = true;

    for (int i = 0; i < 10; i++)
    {
        m_chap[i] = 0;
        m_sel[i] = 0;
    }
    m_index = 0;
    m_maxList = 0;

    memset(&m_perso, 0, sizeof(GamerPerso));
    DefPerso();

    m_bTooltip       = true;
    m_bGlint         = true;
    m_bRain          = true;
    m_bSoluce4       = true;
    m_bMovies        = true;
    m_bNiceReset     = true;
    m_bHimselfDamage = true;
/* TODO: #if _TEEN
    m_bCameraScroll  = false;
#else*/
    m_bCameraScroll  = true;

    m_bCameraInvertX = false;
    m_bCameraInvertY = false;
    m_bEffect        = true;
    m_shotDelay      = 0;

    m_glintMouse = Math::Point(0.0f, 0.0f);
    m_glintTime  = 1000.0f;

    for (int i = 0; i < 10; i++)
    {
        m_partiPhase[i] = 0;
        m_partiTime[i]  = 0.0f;
    }


    m_sceneDir = "levels";
    m_savegameDir = "savegame";
    m_publicDir = "program";
    m_userDir = "user";
    m_filesDir = "files";

    m_bDialog = false;
}

// Destructor of robot application.

CMainDialog::~CMainDialog()
{
}


// Changes phase.

void CMainDialog::ChangePhase(Phase phase)
{
    CWindow*        pw;
    CEdit*          pe;
    CEditValue*     pv;
    CLabel*         pl;
    CList*          pli;
    CCheck*         pc;
    CScroll*        ps;
    CSlider*        psl;
    CButton*        pb;
    CColor*         pco;
    CGroup*         pg;
    CImage*         pi;
    Math::Point         pos, dim, ddim;
    float           ox, oy, sx, sy;
    char            name[100];
    char*           gamer;
    int             res, i, j;

    m_camera->SetType(Gfx::CAM_TYPE_DIALOG);
    m_engine->SetOverFront(false);
    m_engine->SetOverColor(Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f), Gfx::ENG_RSTATE_TCOLOR_BLACK); // TODO: color ok?

    if ( phase == PHASE_TERM )
    {
        phase = m_phaseTerm;
    }
    m_phase = phase;  // copy the info to CRobotMain
    m_phaseTime = 0.0f;

    dim.x = 32.0f/640.0f;
    dim.y = 32.0f/480.0f;
    ox = 3.0f/640.0f;
    oy = 3.0f/480.0f;
    sx = (32.0f+2.0f)/640.0f;
    sy = (32.0f+2.0f)/480.0f;

    if ( m_phase == PHASE_INIT )
    {
        pos.x  = 0.35f;
        pos.y  = 0.10f;
        ddim.x = 0.30f;
        ddim.y = 0.80f;
        /* TODO: #if _TEEN
           pw = m_interface->CreateWindows(pos, ddim, 12, EVENT_WINDOW5);
#else*/
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

        /* TODO: #if _SCHOOL
           ddim.x = 0.20f;
           ddim.y = dim.y*2.4f;
           pos.x = 0.40f;
           pos.y = oy+sy*7.9f;
           pg = pw->CreateGroup(pos, ddim, 24, EVENT_LABEL1);  // orange
           pg->SetState(STATE_SHADOW);
           pos.y = oy+sy*3.9f;
           pg = pw->CreateGroup(pos, ddim, 25, EVENT_LABEL1);  // orange
           pg->SetState(STATE_SHADOW);
           ddim.y = dim.y*1.2f;
           pos.y = oy+sy*1.9f;
           pg = pw->CreateGroup(pos, ddim, 26, EVENT_LABEL1);  // red
           pg->SetState(STATE_SHADOW);
#else */
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

        /* TODO: #if _SCHOOL
           ddim.x = 0.18f;
           ddim.y = dim.y*1;
           pos.x = 0.41f;
           pos.y = oy+sy*9.1f;
           pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_TRAINER);
           pb->SetState(STATE_SHADOW);

           pos.y = oy+sy*8.0f;
#if _TEEN
pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_TEEN);
#else
pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_DEFI);
#endif
#if _CEEBOTDEMO
pb->ClearState(STATE_ENABLE);
#endif
pb->SetState(STATE_SHADOW);
#else */
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

        // TODO: #if !_DEMO & !_SCHOOL
        if ( m_accessEnable && m_accessUser )
        {
            pos.x  = 447.0f/640.0f;
            pos.y  = 313.0f/480.0f;
            ddim.x = 0.09f;
            /*#if _POLISH
              pos.x  -=  5.0f/640.0f;
              ddim.x += 10.0f/640.0f;
#endif*/
            pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_USER);
            pb->SetState(STATE_SHADOW);
        }
        // #endif

        if ( m_app->GetDebugMode() )
        {
            pos.x  = 139.0f/640.0f;
            pos.y  = 313.0f/480.0f;
            ddim.x = 0.09f;
            pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_PROTO);
            pb->SetState(STATE_SHADOW);
        }

        pos.x  = 0.40f;
        ddim.x = 0.20f;
        pos.y  =  26.0f/480.0f;
        ddim.y =  12.0f/480.0f;
        pg = pw->CreateGroup(pos, ddim, 1, EVENT_LABEL1);
        pg->SetState(STATE_SHADOW);
        pos.y -=  5.0f/480.0f;
        pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL1, "PPC Team");
        pl->SetFontType(Gfx::FONT_COURIER);
        pl->SetFontSize(8.0f);

        m_engine->SetBackground("interface.png",
                Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
                Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
                Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
                Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
                true, Math::Point(1.0f, 768.0f / 1024.0f));
        m_engine->SetBackForce(true);
    }

    if ( m_phase == PHASE_NAME )
    {
        pos.x = 0.10f;
        pos.y = 0.10f;
        ddim.x = 0.80f;
        ddim.y = 0.80f;
        pw = m_interface->CreateWindows(pos, ddim, 12, EVENT_WINDOW5);
        GetResource(RES_TEXT, RT_TITLE_NAME, name);
        pw->SetName(name);

        /* TODO: #if _NEWLOOK
           pos.x  =  80.0f/640.0f;
           pos.y  =  93.0f/480.0f;
           ddim.x = 285.0f/640.0f;
           ddim.y = 266.0f/480.0f;
           pg = pw->CreateGroup(pos, ddim, 23, EVENT_LABEL1);  // blue
           pg->SetState(STATE_SHADOW);
           pos.x  = 372.0f/640.0f;
           ddim.x = 188.0f/640.0f;
           pg = pw->CreateGroup(pos, ddim, 26, EVENT_LABEL1);  // violet
           pg->SetState(STATE_SHADOW);
#endif*/

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
        gamer = m_main->GetGamerName();
        if ( gamer[0] == 0 )
        {
            GetResource(RES_TEXT, RT_NAME_DEFAULT, name);
        }
        else
        {
            strcpy(name, gamer);
        }
        pe->SetText(name);
        pe->SetCursor(strlen(name), 0);
        pe->SetFocus(true);

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
        UpdateNameFace();

        m_engine->SetBackground("interface.png",
                Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
                Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
                Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
                Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
                true, Math::Point(1.0f, 768.0f / 1024.0f));
        m_engine->SetBackForce(true);
    }

    if ( m_phase == PHASE_PERSO )
    {
        pos.x = 0.10f;
        pos.y = 0.10f;
        ddim.x = 0.80f;
        ddim.y = 0.80f;
        pw = m_interface->CreateWindows(pos, ddim, 12, EVENT_WINDOW5);
        GetResource(RES_TEXT, RT_TITLE_PERSO, name);
        pw->SetName(name);

        /* TODO: #if _NEWLOOK
           pos.x  =  95.0f/640.0f;
           pos.y  =  66.0f/480.0f;
           ddim.x = 443.0f/640.0f;
           ddim.y =  42.0f/480.0f;
           pg = pw->CreateGroup(pos, ddim, 26, EVENT_LABEL1);  // violet
           pg->SetState(STATE_SHADOW);
#endif*/

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

        m_persoCopy = m_perso;
        m_persoTab = 0;
        m_persoAngle = -0.6f;
        UpdatePerso();
        m_main->ScenePerso();
        CameraPerso();
    }

    if ( m_phase == PHASE_TRAINER ||
            m_phase == PHASE_DEFI    ||
            m_phase == PHASE_MISSION ||
            m_phase == PHASE_FREE    ||
            m_phase == PHASE_TEEN    ||
            m_phase == PHASE_USER    ||
            m_phase == PHASE_PROTO   )
    {
        if ( m_phase == PHASE_TRAINER )  m_index = 0;
        if ( m_phase == PHASE_DEFI    )  m_index = 1;
        if ( m_phase == PHASE_MISSION )  m_index = 2;
        if ( m_phase == PHASE_FREE    )  m_index = 3;
        if ( m_phase == PHASE_USER    )  m_index = 4;
        if ( m_phase == PHASE_PROTO   )  m_index = 5;
        if ( m_phase == PHASE_TEEN    )  m_index = 6;

        if ( m_phase == PHASE_FREE )
        {
            strcpy(m_sceneName, "scene");
            ReadGamerInfo();
            m_accessChap = GetChapPassed();
        }

        if ( m_phase == PHASE_TRAINER )  strcpy(m_sceneName, "train");
        if ( m_phase == PHASE_DEFI    )  strcpy(m_sceneName, "defi" );
        if ( m_phase == PHASE_MISSION )  strcpy(m_sceneName, "scene");
        if ( m_phase == PHASE_FREE    )  strcpy(m_sceneName, "free");
        if ( m_phase == PHASE_TEEN    )  strcpy(m_sceneName, "teen");
        if ( m_phase == PHASE_USER    )  strcpy(m_sceneName, "user");
        if ( m_phase == PHASE_PROTO   )  strcpy(m_sceneName, "proto");

        ReadGamerInfo();

        pos.x = 0.10f;
        pos.y = 0.10f;
        ddim.x = 0.80f;
        ddim.y = 0.80f;
        pw = m_interface->CreateWindows(pos, ddim, 12, EVENT_WINDOW5);
        pw->SetClosable(true);
        if ( m_phase == PHASE_TRAINER )  res = RT_TITLE_TRAINER;
        if ( m_phase == PHASE_DEFI    )  res = RT_TITLE_DEFI;
        if ( m_phase == PHASE_MISSION )  res = RT_TITLE_MISSION;
        if ( m_phase == PHASE_FREE    )  res = RT_TITLE_FREE;
        if ( m_phase == PHASE_TEEN    )  res = RT_TITLE_TEEN;
        if ( m_phase == PHASE_USER    )  res = RT_TITLE_USER;
        if ( m_phase == PHASE_PROTO   )  res = RT_TITLE_PROTO;
        GetResource(RES_TEXT, res, name);
        pw->SetName(name);

        /* TODO: #if _NEWLOOK
           pos.x  = 100.0f/640.0f;
           pos.y  = 226.0f/480.0f;
           ddim.x = 216.0f/640.0f;
           ddim.y = 160.0f/480.0f;
           pg = pw->CreateGroup(pos, ddim, 23, EVENT_LABEL1);  // blue
           pg->SetState(STATE_SHADOW);
           pos.x  = 322.0f/640.0f;
           pg = pw->CreateGroup(pos, ddim, 24, EVENT_LABEL1);  // cyan
           pg->SetState(STATE_SHADOW);

           pos.x  = 100.0f/640.0f;
           pos.y  = 122.0f/480.0f;
           ddim.x = 438.0f/640.0f;
           ddim.y =  98.0f/480.0f;
           pg = pw->CreateGroup(pos, ddim, 25, EVENT_LABEL1);  // green
           pg->SetState(STATE_SHADOW);
           pos.y  =  66.0f/480.0f;
           ddim.y =  42.0f/480.0f;
           pg = pw->CreateGroup(pos, ddim, 26, EVENT_LABEL1);  // violet
           pg->SetState(STATE_SHADOW);
#endif */

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
        if ( m_phase == PHASE_TRAINER )  res = RT_PLAY_CHAPt;
        if ( m_phase == PHASE_DEFI    )  res = RT_PLAY_CHAPd;
        if ( m_phase == PHASE_MISSION )  res = RT_PLAY_CHAPm;
        if ( m_phase == PHASE_FREE    )  res = RT_PLAY_CHAPf;
        if ( m_phase == PHASE_TEEN    )  res = RT_PLAY_CHAPte;
        if ( m_phase == PHASE_USER    )  res = RT_PLAY_CHAPu;
        if ( m_phase == PHASE_PROTO   )  res = RT_PLAY_CHAPp;
        GetResource(RES_TEXT, res, name);
        pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL11, name);
        pl->SetTextAlign(Gfx::TEXT_ALIGN_LEFT);

        pos.y = oy+sy*6.7f;
        ddim.y = dim.y*4.5f;
        ddim.x = dim.x*6.5f;
        pli = pw->CreateList(pos, ddim, 0, EVENT_INTERFACE_CHAP);
        pli->SetState(STATE_SHADOW);
        UpdateSceneChap(m_chap[m_index]);
        if ( m_phase != PHASE_USER )  pli->SetState(STATE_EXTEND);

        // Displays a list of missions:
        pos.x = ox+sx*9.5f;
        pos.y = oy+sy*10.5f;
        ddim.x = dim.x*7.5f;
        ddim.y = dim.y*0.6f;
        if ( m_phase == PHASE_TRAINER )  res = RT_PLAY_LISTt;
        if ( m_phase == PHASE_DEFI    )  res = RT_PLAY_LISTd;
        if ( m_phase == PHASE_MISSION )  res = RT_PLAY_LISTm;
        if ( m_phase == PHASE_FREE    )  res = RT_PLAY_LISTf;
        if ( m_phase == PHASE_TEEN    )  res = RT_PLAY_LISTk;
        if ( m_phase == PHASE_USER    )  res = RT_PLAY_LISTu;
        if ( m_phase == PHASE_PROTO   )  res = RT_PLAY_LISTp;
        GetResource(RES_TEXT, res, name);
        pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL12, name);
        pl->SetTextAlign(Gfx::TEXT_ALIGN_LEFT);

        pos.y = oy+sy*6.7f;
        ddim.y = dim.y*4.5f;
        ddim.x = dim.x*6.5f;
        pli = pw->CreateList(pos, ddim, 0, EVENT_INTERFACE_LIST);
        pli->SetState(STATE_SHADOW);
        UpdateSceneList(m_chap[m_index], m_sel[m_index]);
        if ( m_phase != PHASE_USER )  pli->SetState(STATE_EXTEND);
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
        pe->SetHiliteCap(false);

        // Button displays the "soluce":
        if ( m_phase != PHASE_TRAINER &&
                m_phase != PHASE_FREE    &&
                m_phase != PHASE_TEEN    )
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

        UpdateSceneResume((m_chap[m_index]+1)*100+(m_sel[m_index]+1));

        if ( m_phase == PHASE_MISSION ||
                m_phase == PHASE_FREE    ||
                m_phase == PHASE_USER    )
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

        m_engine->SetBackground("interface.png",
                Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
                Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
                Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
                Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
                true, Math::Point(1.0f, 768.0f / 1024.0f));
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

        /* TODO: #if _NEWLOOK
           if ( m_phase == PHASE_SETUPd  || // setup/display ?
           m_phase == PHASE_SETUPds )
           {
           pos.x  = 100.0f/640.0f;
           pos.y  = 130.0f/480.0f;
           ddim.x = 216.0f/640.0f;
           ddim.y = 212.0f/480.0f;
           pg = pw->CreateGroup(pos, ddim, 23, EVENT_LABEL1);  // blue
           pg->SetState(STATE_SHADOW);
           pos.x  = 324.0f/640.0f;
           pg = pw->CreateGroup(pos, ddim, 24, EVENT_LABEL1);  // cyan
           pg->SetState(STATE_SHADOW);
           }
           if ( m_phase == PHASE_SETUPg  || // setup/graphic ?
           m_phase == PHASE_SETUPgs )
           {
           pos.x  = 100.0f/640.0f;
           pos.y  = 130.0f/480.0f;
           ddim.x = 174.0f/640.0f;
           ddim.y = 212.0f/480.0f;
           pg = pw->CreateGroup(pos, ddim, 23, EVENT_LABEL1);  // blue
           pg->SetState(STATE_SHADOW);
           pos.x  = 282.0f/640.0f;
           ddim.x = 258.0f/640.0f;
           pg = pw->CreateGroup(pos, ddim, 24, EVENT_LABEL1);  // cyan
           pg->SetState(STATE_SHADOW);
           }
           if ( m_phase == PHASE_SETUPp  || // setup/game ?
           m_phase == PHASE_SETUPps )
           {
           pos.x  = 100.0f/640.0f;
           pos.y  = 130.0f/480.0f;
           ddim.x = 226.0f/640.0f;
           ddim.y = 212.0f/480.0f;
           pg = pw->CreateGroup(pos, ddim, 23, EVENT_LABEL1);  // blue
           pg->SetState(STATE_SHADOW);
           pos.x  = 334.0f/640.0f;
           ddim.x = 206.0f/640.0f;
           pg = pw->CreateGroup(pos, ddim, 24, EVENT_LABEL1);  // cyan
           pg->SetState(STATE_SHADOW);
           }
           if ( m_phase == PHASE_SETUPc  || // setup/command ?
           m_phase == PHASE_SETUPcs )
           {
           pos.x  = 100.0f/640.0f;
           pos.y  = 125.0f/480.0f;
           ddim.x = 440.0f/640.0f;
           ddim.y = 222.0f/480.0f;
           pg = pw->CreateGroup(pos, ddim, 23, EVENT_LABEL1);  // blue
           pg->SetState(STATE_SHADOW);
           }
           if ( m_phase == PHASE_SETUPs  || // setup/sound ?
           m_phase == PHASE_SETUPss )
           {
           pos.x  = 100.0f/640.0f;
           pos.y  = 130.0f/480.0f;
           ddim.x = 216.0f/640.0f;
           ddim.y = 212.0f/480.0f;
           pg = pw->CreateGroup(pos, ddim, 23, EVENT_LABEL1);  // blue
           pg->SetState(STATE_SHADOW);
           pos.x  = 324.0f/640.0f;
           pg = pw->CreateGroup(pos, ddim, 24, EVENT_LABEL1);  // cyan
           pg->SetState(STATE_SHADOW);
           }

           pos.x  = 100.0f/640.0f;
           pos.y  =  66.0f/480.0f;
           ddim.x = 440.0f/640.0f;
           ddim.y =  42.0f/480.0f;
           pg = pw->CreateGroup(pos, ddim, 26, EVENT_LABEL1);  // violet
        pg->SetState(STATE_SHADOW);
#endif */

        ddim.x = 0.78f/5-0.01f;
        ddim.y = 0.06f;
        pos.x = 0.115f;
        pos.y = 0.76f;
        pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_SETUPd);
        pb->SetState(STATE_SHADOW);
        pb->SetState(STATE_CARD);
        pb->SetState(STATE_CHECK, (m_phase == PHASE_SETUPd || m_phase == PHASE_SETUPds));

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
            m_engine->SetBackground("interface.png",
                    Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
                    Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
                    Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
                    Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
                    true, Math::Point(1.0f, 768.0f / 1024.0f));
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
        if ( !m_bSimulSetup )
        {
            pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_GROUND);
            pc->SetState(STATE_SHADOW);
            // TODO: video 8 MB?
            //if ( m_engine->IsVideo8MB() )  pc->ClearState(STATE_ENABLE);
        }
        pos.y -= 0.048f;
        pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_DIRTY);
        pc->SetState(STATE_SHADOW);
        pos.y -= 0.048f;
        pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_SKY);
        pc->SetState(STATE_SHADOW);
        // TODO: video 8 MB?
        //if ( m_engine->IsVideo8MB() )  pc->ClearState(STATE_ENABLE);
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
        if ( !m_bSimulSetup )
        {
            pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_LIGHT);
            pc->SetState(STATE_SHADOW);
        }

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

        if ( !m_bSimulSetup )
        {
            pos.x = ox+sx*8.5f;
            pos.y = 0.47f;
            ddim.x = dim.x*2.2f;
            ddim.y = 18.0f/480.0f;
            pv = pw->CreateEditValue(pos, ddim, 0, EVENT_INTERFACE_GADGET);
            pv->SetState(STATE_SHADOW);
            pv->SetMinValue(0.0f);
            pv->SetMaxValue(1.0f);
            pos.x += 0.13f;
            pos.y -= 0.015f;
            ddim.x = 0.40f;
            GetResource(RES_EVENT, EVENT_INTERFACE_GADGET, name);
            pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL13, name);
            pl->SetTextAlign(Gfx::TEXT_ALIGN_LEFT);
        }

#if 0
        if ( !m_bSimulSetup )
        {
            pos.x = ox+sx*8.5f;
            pos.y = 0.41f;
            ddim.x = dim.x*2.2f;
            ddim.y = 18.0f/480.0f;
            pv = pw->CreateEditValue(pos, ddim, 0, EVENT_INTERFACE_TEXTURE);
            pv->SetState(STATE_SHADOW);
            pv->SetType(EVT_INT);
            pv->SetMinValue(0.0f);
            pv->SetMaxValue(2.0f);
            pv->SetStepValue(1.0f);
            pos.x += 0.13f;
            pos.y -= 0.015f;
            ddim.x = 0.40f;
            GetResource(RES_EVENT, EVENT_INTERFACE_TEXTURE, name);
            pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL14, name);
            pl->SetTextAlign(Gfx::TEXT_ALIGN_LEFT);
        }
#endif

        ddim.x = dim.x*2;
        ddim.y = dim.y*1;
        pos.x = ox+sx*10;
        pos.y = oy+sy*2;
        /* TODO: #if _POLISH
           ddim.x += 20.0f/640.0f;
           pos.x -= 20.0f/640.0f*3.0f;
#endif*/
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
        /*TODO: #if _SCHOOL
#if _EDU
pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_SOLUCE4);
pc->SetState(STATE_SHADOW);
pos.y -= 0.048f;
#endif
#else*/
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
        if ( !m_bSimulSetup )
        {
            pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_EDITMODE);
            pc->SetState(STATE_SHADOW);
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
        ps->SetVisibleRatio(static_cast<float>(KEY_VISIBLE/KEY_TOTAL));
        ps->SetArrowStep(1.0f/(static_cast<float>(KEY_TOTAL-KEY_VISIBLE)));
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

        // TODO: #if (_FULL | _NET) & _SOUNDTRACKS
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
        // #endif

        ddim.x = dim.x*6;
        ddim.y = dim.y*0.5f;
        pos.x = ox+sx*10;
        pos.y = 0.55f;
        pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_SOUND3D);
        pc->SetState(STATE_SHADOW);

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

        /* TODO: #if _NEWLOOK
           pos.x  = 100.0f/640.0f;
           pos.y  =  66.0f/480.0f;
           ddim.x = 438.0f/640.0f;
           ddim.y =  42.0f/480.0f;
           pg = pw->CreateGroup(pos, ddim, 26, EVENT_LABEL1);  // violet
           pg->SetState(STATE_SHADOW);
#endif */

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

        /* TODO: #if _NEWLOOK
           pos.x  = 100.0f/640.0f;
           pos.y  =  66.0f/480.0f;
           ddim.x = 438.0f/640.0f;
           ddim.y =  42.0f/480.0f;
           pg = pw->CreateGroup(pos, ddim, 26, EVENT_LABEL1);  // violet
           pg->SetState(STATE_SHADOW);
#endif*/

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
            m_engine->SetBackground("interface.png",
                    Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
                    Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
                    Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
                    Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
                    true, Math::Point(1.0f, 768.0f / 1024.0f));
            m_engine->SetBackForce(true);
        }
    }

    if ( m_phase == PHASE_LOADING )
    {
        pos.x  = 0.35f;
        pos.y  = 0.10f;
        ddim.x = 0.30f;
        ddim.y = 0.80f;
        /*TODO: #if _TEEN
          pw = m_interface->CreateWindows(pos, ddim, 12, EVENT_WINDOW5);
#else*/
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

        m_engine->SetBackground("interface.png",
                Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
                Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
                Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
                Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
                true, Math::Point(1.0f, 768.0f / 1024.0f));
        m_engine->SetBackForce(true);

        m_loadingCounter = 1;  // enough time to display!
    }

    if ( m_phase == PHASE_WELCOME1 )
    {
        m_sound->StopMusic();
        m_sound->PlayMusic(11, false);

        pos.x  = 0.0f;
        pos.y  = 0.0f;
        ddim.x = 0.0f;
        ddim.y = 0.0f;
        pw = m_interface->CreateWindows(pos, ddim, -1, EVENT_WINDOW5);

        m_engine->SetOverColor(Gfx::Color(1.0f, 1.0f, 1.0f, 1.0f), Gfx::ENG_RSTATE_TCOLOR_BLACK); // TODO: color ok?
        m_engine->SetOverFront(true);

        m_engine->SetBackground("ppc.png",
                Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
                Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
                Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
                Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
                true, Math::Point(861.0f / 1024.0f, 646.0f / 1024.0f));
        m_engine->SetBackForce(true);
    }
    if ( m_phase == PHASE_WELCOME2 )
    {
        m_sound->StopMusic();
        m_sound->PlayMusic(11, false);

        pos.x  = 0.0f;
        pos.y  = 0.0f;
        ddim.x = 0.0f;
        ddim.y = 0.0f;
        pw = m_interface->CreateWindows(pos, ddim, -1, EVENT_WINDOW5);

        m_engine->SetOverColor(Gfx::Color(1.0f, 1.0f, 1.0f, 1.0f), Gfx::ENG_RSTATE_TCOLOR_BLACK); // TODO: color ok?
        m_engine->SetOverFront(true);

        m_engine->SetBackground("colobot.png",
                Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
                Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
                Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
                Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
                true, Math::Point(640.0f / 1024.0f, 480.0f / 512.0f));
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

        m_engine->SetBackground("epsitec.png",
                Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
                Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
                Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
                Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
                true, Math::Point(640.0f / 1024.0f, 480.0f / 512.0f));
        m_engine->SetBackForce(true);
    }

    if ( m_phase == PHASE_GENERIC )
    {
        pos.x  = 0.0f;
        pos.y  = 0.0f;
        ddim.x = 0.0f;
        ddim.y = 0.0f;
        pw = m_interface->CreateWindows(pos, ddim, -1, EVENT_WINDOW5);

        // TODO: #if _FULL | _NET
        pos.x  =  80.0f/640.0f;
        pos.y  = 240.0f/480.0f;
        ddim.x = 490.0f/640.0f;
        ddim.y = 110.0f/480.0f;
        pe = pw->CreateEdit(pos, ddim, 0, EVENT_EDIT1);
        pe->SetGenericMode(true);
        pe->SetEditCap(false);
        pe->SetHiliteCap(false);
        pe->SetFontType(Gfx::FONT_COURIER);
        pe->SetFontSize(8.0f);
        pe->ReadText("help/authors.txt");

        pos.x  =  80.0f/640.0f;
        pos.y  = 140.0f/480.0f;
        ddim.x = 490.0f/640.0f;
        ddim.y = 100.0f/480.0f;
        pe = pw->CreateEdit(pos, ddim, 0, EVENT_EDIT2);
        pe->SetGenericMode(true);
        pe->SetEditCap(false);
        pe->SetHiliteCap(false);
        pe->SetFontType(Gfx::FONT_COURIER);
        pe->SetFontSize(6.5f);
        pe->ReadText("help/licences.txt");
        // #endif
        /* TODO: #if _SCHOOL
#if _CEEBOTDEMO
pos.x  =  80.0f/640.0f;
pos.y  = 210.0f/480.0f;
ddim.x = 490.0f/640.0f;
ddim.y = 150.0f/480.0f;
#else
pos.x  =  80.0f/640.0f;
pos.y  = 200.0f/480.0f;
ddim.x = 490.0f/640.0f;
ddim.y = 150.0f/480.0f;
#endif*/
        pe = pw->CreateEdit(pos, ddim, 0, EVENT_EDIT1);
        pe->SetGenericMode(true);
        pe->SetEditCap(false);
        pe->SetHiliteCap(false);
        pe->SetFontType(Gfx::FONT_COURIER);
        pe->SetFontSize(8.0f);
        pe->ReadText("help/authors.txt");

        /* #if _DEMO
        //?     pos.x  =  80.0f/640.0f;
        //?     pos.y  = 240.0f/480.0f;
        //?     ddim.x = 490.0f/640.0f;
        //?     ddim.y = 110.0f/480.0f;
        //?     pe = pw->CreateEdit(pos, ddim, 0, EVENT_EDIT1);
        //?     pe->SetGenericMode(true);
        //?     pe->SetEditCap(false);
        //?     pe->SetHiliteCap(false);
        //?     pe->SetFontType(Gfx::FONT_COURIER);
        //?     pe->SetFontSize(8.0f);
        //?     pe->ReadText("help/demo.txt");

        //?     pos.x  =  80.0f/640.0f;
        //?     pos.y  = 140.0f/480.0f;
        //?     ddim.x = 490.0f/640.0f;
        //?     ddim.y = 100.0f/480.0f;
        //?     pe = pw->CreateEdit(pos, ddim, 0, EVENT_EDIT2);
        //?     pe->SetGenericMode(true);
        //?     pe->SetEditCap(false);
        //?     pe->SetHiliteCap(false);
        //?     pe->SetFontType(Gfx::FONT_COURIER);
        //?     pe->SetFontSize(8.0f);
        //?     pe->ReadText("help/authors.txt");
#endif */

        // TODO: #if !_DEMO
        pos.x  =  40.0f/640.0f;
        pos.y  =  83.0f/480.0f;
        ddim.x = 246.0f/640.0f;
        ddim.y =  16.0f/480.0f;
        GetResource(RES_TEXT, RT_GENERIC_DEV1, name);
        pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL1, name);
        pl->SetFontType(Gfx::FONT_COURIER);
        pl->SetFontSize(8.0f);

        pos.y  =  13.0f/480.0f;
        GetResource(RES_TEXT, RT_GENERIC_DEV2, name);
        pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL2, name);
        pl->SetFontType(Gfx::FONT_COURIER);
        pl->SetFontSize(8.0f);

        pos.x  = 355.0f/640.0f;
        pos.y  =  83.0f/480.0f;
        ddim.x = 246.0f/640.0f;
        ddim.y =  16.0f/480.0f;
        GetResource(RES_TEXT, RT_GENERIC_EDIT1, name);
        pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL3, name);
        pl->SetFontType(Gfx::FONT_COURIER);
        pl->SetFontSize(8.0f);

        pos.y  =  13.0f/480.0f;
        GetResource(RES_TEXT, RT_GENERIC_EDIT2, name);
        pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL4, name);
        pl->SetFontType(Gfx::FONT_COURIER);
        pl->SetFontSize(8.0f);
        // #endif

        /* TODO: #if _DEMO
           pos.x  = 481.0f/640.0f;
           pos.y  =  51.0f/480.0f;
           ddim.x =  30.0f/640.0f;
           ddim.y =  30.0f/480.0f;
           pb = pw->CreateButton(pos, ddim, 49, EVENT_INTERFACE_ABORT);
           pb->SetState(STATE_SHADOW);
#else */
        pos.x  = 306.0f/640.0f;
        pos.y  =  17.0f/480.0f;
        ddim.x =  30.0f/640.0f;
        ddim.y =  30.0f/480.0f;
        pb = pw->CreateButton(pos, ddim, 49, EVENT_INTERFACE_ABORT);
        pb->SetState(STATE_SHADOW);
        // #endif

        m_engine->SetBackground("generic.png",
                Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
                Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
                Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
                Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
                true, Math::Point(1.0f, 768.0f / 1024.0f));
        m_engine->SetBackForce(true);
    }

    if ( m_phase == PHASE_INIT    ||
            m_phase == PHASE_NAME    ||
            m_phase == PHASE_TRAINER ||
            m_phase == PHASE_DEFI    ||
            m_phase == PHASE_MISSION ||
            m_phase == PHASE_FREE    ||
            m_phase == PHASE_TEEN    ||
            m_phase == PHASE_USER    ||
            m_phase == PHASE_PROTO   ||
            m_phase == PHASE_SETUPd  ||
            m_phase == PHASE_SETUPg  ||
            m_phase == PHASE_SETUPp  ||
            m_phase == PHASE_SETUPc  ||
            m_phase == PHASE_SETUPs  ||
            m_phase == PHASE_READ    ||
            m_phase == PHASE_LOADING )
    {
        /*TODO: #if _SCHOOL
#if _TEEN
pos.x  =  50.0f/640.0f;
pos.y  = 430.0f/480.0f;
ddim.x = 200.0f/640.0f;
ddim.y =  10.0f/480.0f;
#else
pos.x  = 450.0f/640.0f;
pos.y  =   0.0f/480.0f;
ddim.x = 170.0f/640.0f;
ddim.y =   9.0f/480.0f;
#endif
#else */
        pos.x  = 540.0f/640.0f;
        pos.y  =   9.0f/480.0f;
        ddim.x =  90.0f/640.0f;
        ddim.y =  10.0f/480.0f;
        //#endif
        GetResource(RES_TEXT, RT_VERSION_ID, name);
        pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL1, name);
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

        if ( m_phase == PHASE_WELCOME1 ||
                m_phase == PHASE_WELCOME2 ||
                m_phase == PHASE_WELCOME3 )
        {
            float   intensity;
            int     mode = Gfx::ENG_RSTATE_TCOLOR_BLACK;

            if ( m_phaseTime < 1.5f )
            {
                intensity = 1.0f-(m_phaseTime-0.5f);
            }
            else if ( m_phaseTime < welcomeLength-1.0f )
            {
                intensity = 0.0f;
            }
            else
            {
                intensity = m_phaseTime-(welcomeLength-1.0f);
            }
            if ( intensity < 0.0f )  intensity = 0.0f;
            if ( intensity > 1.0f )  intensity = 1.0f;

            if ( (m_phase == PHASE_WELCOME2 && m_phaseTime > welcomeLength/2.0f) ||
                    m_phase == PHASE_WELCOME3 )
            {
                intensity = 1.0f-intensity;
                mode = Gfx::ENG_RSTATE_TCOLOR_WHITE;
            }

            m_engine->SetOverColor(Gfx::Color(intensity, intensity, intensity, intensity), mode); // TODO: color ok?
        }

        if ( m_phase == PHASE_WELCOME1 && m_phaseTime >= welcomeLength )
        {
            ChangePhase(PHASE_WELCOME2);
            return true;
        }
        if ( m_phase == PHASE_WELCOME2 && m_phaseTime >= welcomeLength )
        {
            ChangePhase(PHASE_WELCOME3);
            return true;
        }
        if ( m_phase == PHASE_WELCOME3 && m_phaseTime >= welcomeLength )
        {
            ChangePhase(PHASE_NAME);
            return true;
        }

        if ( m_shotDelay > 0 && !m_bDialog )  // screenshot done?
        {
            m_shotDelay --;
            if ( m_shotDelay == 0 )
            {
                m_engine->WriteScreenShot(m_shotName, 320, 240);
                //?             m_engine->WriteScreenShot(m_shotName, 160, 120);
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
            if ( m_phase == PHASE_NAME )
            {
                NameDelete();
            }
            if ( m_phase == PHASE_INIT )
            {
                //?             m_eventQueue->MakeEvent(newEvent, EVENT_QUIT);
                //?             m_eventQueue->AddEvent(newEvent);
                m_main->ChangePhase(PHASE_GENERIC);
            }
            if ( m_phase == PHASE_SIMUL )
            {
                if ( m_bDialogDelete )
                {
                    m_main->DeleteObject();
                }
                else
                {
                    m_main->ChangePhase(PHASE_TERM);
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

    if ( m_phase == PHASE_INIT )
    {
        switch( event.type )
        {
            case EVENT_KEY_DOWN:
                if ( event.key.key == KEY(ESCAPE) )
                {
                    //?                 StartQuit();  // would you leave?
                    m_sound->Play(SOUND_TZOING);
                    m_main->ChangePhase(PHASE_GENERIC);
                }
                break;

            case EVENT_INTERFACE_QUIT:
                //?             StartQuit();  // would you leave?
                m_sound->Play(SOUND_TZOING);
                m_main->ChangePhase(PHASE_GENERIC);
                break;

            case EVENT_INTERFACE_TRAINER:
                m_main->ChangePhase(PHASE_TRAINER);
                break;

            case EVENT_INTERFACE_DEFI:
                m_main->ChangePhase(PHASE_DEFI);
                break;

            case EVENT_INTERFACE_MISSION:
                m_main->ChangePhase(PHASE_MISSION);
                break;

            case EVENT_INTERFACE_FREE:
                m_main->ChangePhase(PHASE_FREE);
                break;

            case EVENT_INTERFACE_TEEN:
                m_main->ChangePhase(PHASE_TEEN);
                break;

            case EVENT_INTERFACE_USER:
                m_main->ChangePhase(PHASE_USER);
                break;

            case EVENT_INTERFACE_PROTO:
                m_main->ChangePhase(PHASE_PROTO);
                break;

            case EVENT_INTERFACE_SETUP:
                m_main->ChangePhase(m_phaseSetup);
                break;

            case EVENT_INTERFACE_NAME:
                m_main->ChangePhase(PHASE_NAME);
                break;

            default:
                break;
        }
        return false;
    }

    if ( m_phase == PHASE_NAME )
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
                        m_main->ChangePhase(PHASE_INIT);
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
                m_main->ChangePhase(PHASE_PERSO);
                break;

            case EVENT_INTERFACE_NCANCEL:
                m_main->ChangePhase(PHASE_INIT);
                break;

            case EVENT_INTERFACE_NDELETE:
                pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
                if ( pw == 0 )  break;
                pl = static_cast<CList*>(pw->SearchControl(EVENT_INTERFACE_NLIST));
                if ( pl == 0 )  break;
                StartDeleteGame(pl->GetName(pl->GetSelect()));
                break;

            default:
                break;
        }
    }

    if ( m_phase == PHASE_PERSO )
    {
        switch( event.type )
        {
            case EVENT_KEY_DOWN:
                if ( event.key.key == KEY(RETURN) )
                {
                    m_main->ChangePhase(PHASE_INIT);
                }
                if ( event.key.key == KEY(ESCAPE) )
                {
                    m_main->ChangePhase(PHASE_NAME);
                }
                break;

            case EVENT_INTERFACE_PHEAD:
                m_persoTab = 0;
                UpdatePerso();
                m_main->ScenePerso();
                CameraPerso();
                break;
            case EVENT_INTERFACE_PBODY:
                m_persoTab = 1;
                UpdatePerso();
                m_main->ScenePerso();
                CameraPerso();
                break;

            case EVENT_INTERFACE_PFACE1:
            case EVENT_INTERFACE_PFACE2:
            case EVENT_INTERFACE_PFACE3:
            case EVENT_INTERFACE_PFACE4:
                m_perso.face = event.type-EVENT_INTERFACE_PFACE1;
                WriteGamerPerso(m_main->GetGamerName());
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
                m_perso.glasses = event.type-EVENT_INTERFACE_PGLASS0;
                WriteGamerPerso(m_main->GetGamerName());
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
                WriteGamerPerso(m_main->GetGamerName());
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
                WriteGamerPerso(m_main->GetGamerName());
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
                WriteGamerPerso(m_main->GetGamerName());
                UpdatePerso();
                m_main->ScenePerso();
                break;

            case EVENT_INTERFACE_PDEF:
                DefPerso();
                WriteGamerPerso(m_main->GetGamerName());
                UpdatePerso();
                m_main->ScenePerso();
                break;

            case EVENT_INTERFACE_PLROT:
                m_persoAngle += 0.2f;
                break;
            case EVENT_INTERFACE_PRROT:
                m_persoAngle -= 0.2f;
                break;

            case EVENT_INTERFACE_POK:
                m_main->ChangePhase(PHASE_INIT);
                break;

            case EVENT_INTERFACE_PCANCEL:
                m_perso = m_persoCopy;
                WriteGamerPerso(m_main->GetGamerName());
                m_main->ChangePhase(PHASE_NAME);
                break;

            default:
                break;
        }
    }

    if ( m_phase == PHASE_TRAINER ||
            m_phase == PHASE_DEFI    ||
            m_phase == PHASE_MISSION ||
            m_phase == PHASE_FREE    ||
            m_phase == PHASE_TEEN    ||
            m_phase == PHASE_USER    ||
            m_phase == PHASE_PROTO   )
    {
        pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
        if ( pw == 0 )  return false;

        if ( event.type == pw->GetEventTypeClose() ||
                event.type == EVENT_INTERFACE_BACK   ||
                (event.type == EVENT_KEY_DOWN && event.key.key == KEY(ESCAPE)) )
        {
            m_main->ChangePhase(PHASE_INIT);
            return false;
        }
    }

    if ( m_phase == PHASE_TRAINER ||
            m_phase == PHASE_DEFI    ||
            m_phase == PHASE_MISSION ||
            m_phase == PHASE_FREE    ||
            m_phase == PHASE_TEEN    ||
            m_phase == PHASE_USER    ||
            m_phase == PHASE_PROTO   )
    {
        switch( event.type )
        {
            case EVENT_INTERFACE_CHAP:
                pl = static_cast<CList*>(pw->SearchControl(EVENT_INTERFACE_CHAP));
                if ( pl == 0 )  break;
                m_chap[m_index] = pl->GetSelect();
                UpdateSceneList(m_chap[m_index], m_sel[m_index]);
                UpdateSceneResume((m_chap[m_index]+1)*100+(m_sel[m_index]+1));
                break;

            case EVENT_INTERFACE_LIST:
                pl = static_cast<CList*>(pw->SearchControl(EVENT_INTERFACE_LIST));
                if ( pl == 0 )  break;
                m_sel[m_index] = pl->GetSelect();
                UpdateSceneResume((m_chap[m_index]+1)*100+(m_sel[m_index]+1));
                break;

            case EVENT_INTERFACE_SOLUCE:
                pb = static_cast<CButton*>(pw->SearchControl(EVENT_INTERFACE_SOLUCE));
                if ( pb == 0 )  break;
                m_bSceneSoluce = !m_bSceneSoluce;
                pb->SetState(STATE_CHECK, m_bSceneSoluce);
                break;

            case EVENT_INTERFACE_PLAY:
                if ( m_phase == PHASE_PROTO && m_chap[m_index] == 0 && m_sel[m_index] == 0 )
                {
                    m_main->ChangePhase(PHASE_MODEL);
                    break;
                }
                m_sceneRank = (m_chap[m_index]+1)*100+(m_sel[m_index]+1);
                m_phaseTerm = m_phase;
                m_main->ChangePhase(PHASE_LOADING);
                break;

            case EVENT_INTERFACE_READ:
                m_phaseTerm = m_phase;
                m_main->ChangePhase(PHASE_READ);
                break;

            default:
                break;
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
            m_main->ChangePhase(PHASE_INIT);
            return false;
        }

        switch( event.type )
        {
            case EVENT_INTERFACE_SETUPd:
                m_main->ChangePhase(PHASE_SETUPd);
                break;

            case EVENT_INTERFACE_SETUPg:
                m_main->ChangePhase(PHASE_SETUPg);
                break;

            case EVENT_INTERFACE_SETUPp:
                m_main->ChangePhase(PHASE_SETUPp);
                break;

            case EVENT_INTERFACE_SETUPc:
                m_main->ChangePhase(PHASE_SETUPc);
                break;

            case EVENT_INTERFACE_SETUPs:
                m_main->ChangePhase(PHASE_SETUPs);
                break;

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
                break;

            case EVENT_INTERFACE_SETUPg:
                ChangePhase(PHASE_SETUPgs);
                break;

            case EVENT_INTERFACE_SETUPp:
                ChangePhase(PHASE_SETUPps);
                break;

            case EVENT_INTERFACE_SETUPc:
                ChangePhase(PHASE_SETUPcs);
                break;

            case EVENT_INTERFACE_SETUPs:
                ChangePhase(PHASE_SETUPss);
                break;

            default:
                break;
        }
    }

    if ( m_phase == PHASE_SETUPd  ||  // setup/display ?
            m_phase == PHASE_SETUPds )
    {
        switch( event.type )
        {
            case EVENT_LIST1:
            case EVENT_LIST2:
                UpdateApply();
                break;

            case EVENT_INTERFACE_FULL:
                pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
                if ( pw == 0 )  break;
                pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_FULL));
		if ( pc == 0 )  break;
		
		if ( pc->TestState(STATE_CHECK) ) {
		    m_setupFull = false;
		    pc->ClearState(STATE_CHECK);
		} else {
		    m_setupFull = true;
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
		// TODO: uncomment when changing display is implemented
                //ChangeDisplay();
                UpdateApply();
                break;

            default:
                break;
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
            case EVENT_INTERFACE_TEXTURE:
                ChangeSetupButtons();
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
                break;
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
                if (m_app->GetMouseMode() == MOUSE_ENGINE)
                    m_app->SetMouseMode(MOUSE_SYSTEM);
                else if (m_app->GetMouseMode() == MOUSE_SYSTEM)
                    m_app->SetMouseMode(MOUSE_ENGINE);

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

            default:
                break;
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

            case EVENT_INTERFACE_KLEFT:
            case EVENT_INTERFACE_KRIGHT:
            case EVENT_INTERFACE_KUP:
            case EVENT_INTERFACE_KDOWN:
            case EVENT_INTERFACE_KGUP:
            case EVENT_INTERFACE_KGDOWN:
            case EVENT_INTERFACE_KCAMERA:
            case EVENT_INTERFACE_KDESEL:
            case EVENT_INTERFACE_KACTION:
            case EVENT_INTERFACE_KNEAR:
            case EVENT_INTERFACE_KAWAY:
            case EVENT_INTERFACE_KNEXT:
            case EVENT_INTERFACE_KHUMAN:
            case EVENT_INTERFACE_KQUIT:
            case EVENT_INTERFACE_KHELP:
            case EVENT_INTERFACE_KPROG:
            case EVENT_INTERFACE_KCBOT:
            case EVENT_INTERFACE_KSPEED10:
            case EVENT_INTERFACE_KSPEED15:
            case EVENT_INTERFACE_KSPEED20:
            case EVENT_INTERFACE_KSPEED30:
            case EVENT_INTERFACE_KVISIT:
                ChangeKey(event.type);
                UpdateKey();
                break;

            case EVENT_INTERFACE_KDEF:
                m_app->ResetKeyStates();
                UpdateKey();
                break;

            case EVENT_INTERFACE_JOYSTICK:
                m_app->SetJoystickEnabled(!m_app->GetJoystickEnabled());
                UpdateSetupButtons();
                break;

            default:
                break;
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

            case EVENT_INTERFACE_SOUND3D:
                m_sound->SetSound3D(!m_sound->GetSound3D());
                ChangeSetupButtons();
                UpdateSetupButtons();
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
                break;
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
            ChangePhase(m_phaseTerm);
        }

        if ( event.type == EVENT_INTERFACE_IOLIST )
        {
            IOUpdateList();
        }
        if ( event.type == EVENT_INTERFACE_IODELETE )
        {
            IODeleteScene();
            IOUpdateList();
        }
        if ( event.type == EVENT_INTERFACE_IOREAD )
        {
            if ( IOReadScene() )
            {
                m_main->ChangePhase(PHASE_LOADING);
            }
        }

        return false;
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
        }

        if ( event.type == EVENT_INTERFACE_IOLIST )
        {
            IOUpdateList();
        }
        if ( event.type == EVENT_INTERFACE_IODELETE )
        {
            IODeleteScene();
            IOUpdateList();
        }
        if ( event.type == EVENT_INTERFACE_IOWRITE )
        {
            IOWriteScene();
            m_interface->DeleteControl(EVENT_WINDOW5);
            ChangePhase(PHASE_SIMUL);
            StopSuspend();
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
        }

        return false;
    }

    if ( m_phase == PHASE_WELCOME1 )
    {
        if ( event.type == EVENT_KEY_DOWN     ||
                event.type == EVENT_MOUSE_BUTTON_DOWN )
        {
            ChangePhase(PHASE_WELCOME2);
            return true;
        }
    }
    if ( m_phase == PHASE_WELCOME2 )
    {
        if ( event.type == EVENT_KEY_DOWN     ||
                event.type == EVENT_MOUSE_BUTTON_DOWN )
        {
            ChangePhase(PHASE_WELCOME3);
            return true;
        }
    }
    if ( m_phase == PHASE_WELCOME3 )
    {
        if ( event.type == EVENT_KEY_DOWN     ||
                event.type == EVENT_MOUSE_BUTTON_DOWN )
        {
            ChangePhase(PHASE_NAME);
            return true;
        }
    }

    if ( m_phase == PHASE_GENERIC )
    {
        if ( event.type == EVENT_INTERFACE_ABORT )
        {
            ChangePhase(PHASE_INIT);
        }

        if ( event.type == EVENT_KEY_DOWN )
        {
            if ( event.key.key == KEY(ESCAPE) )
            {
                ChangePhase(PHASE_INIT);
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

    if ( m_phase == PHASE_INIT )
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

    if ( m_phase == PHASE_NAME    ||
            m_phase == PHASE_TRAINER ||
            m_phase == PHASE_MISSION ||
            m_phase == PHASE_FREE    ||
            m_phase == PHASE_TEEN    ||
            m_phase == PHASE_USER    ||
            m_phase == PHASE_PROTO   )
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
    /* TODO: #if _NEWLOOK
#else */
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

    if ( m_phase == PHASE_INIT )
    {
        pParti = partiPosInit;
        pGlint = glintPosInit;
    }
    else if ( m_phase == PHASE_NAME    ||
            m_phase == PHASE_TRAINER ||
            m_phase == PHASE_DEFI    ||
            m_phase == PHASE_MISSION ||
            m_phase == PHASE_FREE    ||
            m_phase == PHASE_TEEN    ||
            m_phase == PHASE_USER    ||
            m_phase == PHASE_PROTO   ||
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
                m_phase == PHASE_LOST  ||
                m_phase == PHASE_MODEL ) &&
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



// Specifies the special user folder if needed.

void CMainDialog::SetUserDir(char *base, int rank)
{
    std::string dir;

    if ( strcmp(base, "user") == 0 && rank >= 100 )
    {
        dir = m_userDir + "/" + m_userList.at(rank/100-1);
        UserDir(true, dir.c_str());
    }
    else
    {
        UserDir(false, "");
    }
}

// Builds the file name of a mission.

void CMainDialog::BuildSceneName(std::string &filename, char *base, int rank)
{
    std::ostringstream rankStream;
    if ( strcmp(base, "user") == 0 )
    {
        //TODO: Change this to point user dir according to operating system
        rankStream << std::setfill('0') << std::setw(2) << rank%100;
        filename = m_userDir + "/" + m_userList[rank/100-1] + "/" + rankStream.str() + ".txt";
    }
    else
    {
        rankStream << std::setfill('0') << std::setw(3) << rank;
        filename = base + rankStream.str() + ".txt";
        filename = CApplication::GetInstance().GetDataFilePath(DIR_LEVEL, filename);
    }
}

// Built the default descriptive name of a mission.

void CMainDialog::BuildResumeName(char *filename, char *base, int rank)
{
    sprintf(filename, "Scene %s %d", base, rank);
}

// Returns the name of the file or save the files.

std::string & CMainDialog::GetFilesDir()
{
    return m_filesDir;
}


// Updates the list of players after checking the files on disk.

void CMainDialog::ReadNameList()
{
    CWindow*            pw;
    CList*              pl;
    //struct _finddata_t  fBuffer;
    char                dir[MAX_FNAME];
    // char                filenames[MAX_FNAME][100];
    std::vector<std::string> fileNames;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == 0 )  return;
    pl = static_cast<CList*>(pw->SearchControl(EVENT_INTERFACE_NLIST));
    if ( pl == 0 )  return;
    pl->Flush();


    try
    {
        if (! fs::exists(m_savegameDir) && fs::is_directory(m_savegameDir))
        {
            GetLogger()->Error("Savegame dir does not exist %s\n",dir);
        }
        else
        {
            fs::directory_iterator dirIt(m_savegameDir), dirEndIt;

            BOOST_FOREACH (const fs::path & p, std::make_pair(dirIt, dirEndIt))
            {
                if (fs::is_directory(p))
                {
                    fileNames.push_back(p.leaf().string());
                }
            }
        }
    }
    catch (std::exception & e)
    {
        GetLogger()->Error("Error on listing savegame directory : %s\n", e.what());
        return;
    }


    for (size_t i=0 ; i<fileNames.size() ; ++i )
    {
        pl->SetName(i, fileNames.at(i).c_str());
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
    char*       gamer;
    int         total, sel;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == 0 )  return;
    pl = static_cast<CList*>(pw->SearchControl(EVENT_INTERFACE_NLIST));
    if ( pl == 0 )  return;
    pe = static_cast<CEdit*>(pw->SearchControl(EVENT_INTERFACE_NEDIT));
    if ( pe == 0 )  return;

    gamer = m_main->GetGamerName();
    total = pl->GetTotal();
    sel   = pl->GetSelect();
    pe->GetText(name, 100);

    pb = static_cast<CButton*>(pw->SearchControl(EVENT_INTERFACE_NCANCEL));
    if ( pb != 0 )
    {
        pb->SetState(STATE_ENABLE, gamer[0]!=0);
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
        if ( strcmp(name, pl->GetName(i)) == 0 )
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
        name = pl->GetName(sel);
        pe->SetText(name);
        pe->SetCursor(strlen(name), 0);
    }

    UpdateNameControl();
}

// Updates the representation of the player depending on the selected list.

void CMainDialog::UpdateNameFace()
{
    CWindow*    pw;
    CList*      pl;
    char*       name;
    int         sel;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == 0 )  return;
    pl = static_cast<CList*>(pw->SearchControl(EVENT_INTERFACE_NLIST));
    if ( pl == 0 )  return;

    sel = pl->GetSelect();
    if ( sel == -1 )  return;
    name = pl->GetName(sel);

    ReadGamerPerso(name);
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
        m_main->SetGamerName(pl->GetName(sel));
        m_main->ChangePhase(PHASE_INIT);
    }

    GetGamerFace(m_main->GetGamerName());

    GetProfile().SetLocalProfileString("Gamer", "LastName", m_main->GetGamerName());
}

// Creates a new player.

void CMainDialog::NameCreate()
{
    CWindow*    pw;
    CEdit*      pe;
    char        name[100];
    std::string dir;
    char        c;
    int         len, i, j;

    GetLogger()->Debug("Creating new player\n");
    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == 0 )  return;
    pe = static_cast<CEdit*>(pw->SearchControl(EVENT_INTERFACE_NEDIT));
    if ( pe == 0 )  return;

    pe->GetText(name, 100);
    if ( name[0] == 0 )
    {
        m_sound->Play(SOUND_TZOING);
        return;
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
        return;
    }

    // TODO: _mkdir(m_savegameDir);  // if does not exist yet!


    dir = m_savegameDir + "/" + name;
    if (!fs::exists(dir))
    {
        fs::create_directories(dir);
    }
    else
    {
        m_sound->Play(SOUND_TZOING);
        pe->SetText(name);
        pe->SetCursor(strlen(name), 0);
        pe->SetFocus(true);
        return;
    }

    SetGamerFace(name, 0);

    m_main->SetGamerName(name);
    m_main->ChangePhase(PHASE_INIT);
}

// Deletes a folder and all its offspring.

bool RemoveDir(char *dirName)
{
    try
    {

        if (!fs::exists(dirName) && fs::is_directory(dirName))
        {
            GetLogger()->Error("Directory does not exist %s\n",dirName);
            return false;
        }
        else
        {
            fs::remove_all(dirName);
        }

    }
    catch (std::exception & e)
    {
        GetLogger()->Error("Error on removing directory %s : %s\n", dirName, e.what());
        return false;
    }
    return true;
}

// Removes a player.

void CMainDialog::NameDelete()
{
    CWindow*    pw;
    CList*      pl;
    int         sel;
    char*       gamer;
    char        dir[100];

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == 0 )  return;
    pl = static_cast<CList*>(pw->SearchControl(EVENT_INTERFACE_NLIST));
    if ( pl == 0 )  return;

    sel = pl->GetSelect();
    if ( sel == -1 )
    {
        m_sound->Play(SOUND_TZOING);
        return;
    }
    gamer = pl->GetName(sel);

    // Deletes all the contents of the file.
    sprintf(dir, "%s/%s", m_savegameDir.c_str(), gamer);
    if ( !RemoveDir(dir) )
    {
        m_sound->Play(SOUND_TZOING);
        return;
    }

    m_main->SetGamerName("");
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
    char            name[100];
    int             i;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == 0 )  return;

    pb = static_cast<CButton*>(pw->SearchControl(EVENT_INTERFACE_PHEAD));
    if ( pb != 0 )
    {
        pb->SetState(STATE_CHECK, m_persoTab==0);
    }
    pb = static_cast<CButton*>(pw->SearchControl(EVENT_INTERFACE_PBODY));
    if ( pb != 0 )
    {
        pb->SetState(STATE_CHECK, m_persoTab==1);
    }

    pl = static_cast<CLabel*>(pw->SearchControl(EVENT_LABEL11));
    if ( pl != 0 )
    {
        if ( m_persoTab == 0 )
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
        if ( m_persoTab == 0 )
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
        if ( m_persoTab == 0 )  GetResource(RES_TEXT, RT_PERSO_HAIR, name);
        else                    GetResource(RES_TEXT, RT_PERSO_BAND, name);
        pl->SetName(name);
    }

    pl = static_cast<CLabel*>(pw->SearchControl(EVENT_LABEL14));
    if ( pl != 0 )
    {
        if ( m_persoTab == 0 )
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
        pb->SetState(STATE_VISIBLE, m_persoTab==0);
        pb->SetState(STATE_CHECK, i==m_perso.face);
    }

    for ( i=0 ; i<10 ; i++ )
    {
        pb = static_cast<CButton*>(pw->SearchControl(static_cast<EventType>(EVENT_INTERFACE_PGLASS0+i)));
        if ( pb == 0 )  break;
        pb->SetState(STATE_VISIBLE, m_persoTab==0);
        pb->SetState(STATE_CHECK, i==m_perso.glasses);
    }

    for ( i=0 ; i<3*3 ; i++ )
    {
        pc = static_cast<CColor*>(pw->SearchControl(static_cast<EventType>(EVENT_INTERFACE_PC0a+i)));
        if ( pc == 0 )  break;
        if ( m_persoTab == 0 )
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
            pc->SetState(STATE_CHECK, EqColor(color, m_perso.colorCombi));
        }

        pc = static_cast<CColor*>(pw->SearchControl(static_cast<EventType>(EVENT_INTERFACE_PC0b+i)));
        if ( pc == 0 )  break;
        color.r = perso_color[3*10*2*m_persoTab+3*i+0]/255.0f;
        color.g = perso_color[3*10*2*m_persoTab+3*i+1]/255.0f;
        color.b = perso_color[3*10*2*m_persoTab+3*i+2]/255.0f;
        color.a = 0.0f;
        pc->SetColor(color);
        pc->SetState(STATE_CHECK, EqColor(color, m_persoTab?m_perso.colorBand:m_perso.colorHair));
    }

    for ( i=0 ; i<3 ; i++ )
    {
        ps = static_cast<CSlider*>(pw->SearchControl(static_cast<EventType>(EVENT_INTERFACE_PCRa+i)));
        if ( ps == 0 )  break;
        ps->SetState(STATE_VISIBLE, m_persoTab==1);
    }

    if ( m_persoTab == 1 )
    {
        color = m_perso.colorCombi;
        ps = static_cast<CSlider*>(pw->SearchControl(EVENT_INTERFACE_PCRa));
        if ( ps != 0 )  ps->SetVisibleValue(color.r*255.0f);
        ps = static_cast<CSlider*>(pw->SearchControl(EVENT_INTERFACE_PCGa));
        if ( ps != 0 )  ps->SetVisibleValue(color.g*255.0f);
        ps = static_cast<CSlider*>(pw->SearchControl(EVENT_INTERFACE_PCBa));
        if ( ps != 0 )  ps->SetVisibleValue(color.b*255.0f);
    }

    if ( m_persoTab == 0 )  color = m_perso.colorHair;
    else                    color = m_perso.colorBand;
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
    if ( m_persoTab == 0 )
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
    if ( m_persoTab == 0 )
    {
        if ( index == 1 )
        {
            m_perso.colorHair.r = perso_color[3*10*0+rank*3+0]/255.0f;
            m_perso.colorHair.g = perso_color[3*10*0+rank*3+1]/255.0f;
            m_perso.colorHair.b = perso_color[3*10*0+rank*3+2]/255.0f;
        }
    }
    if ( m_persoTab == 1 )
    {
        if ( index == 0 )
        {
            m_perso.colorCombi.r = perso_color[3*10*1+rank*3+0]/255.0f;
            m_perso.colorCombi.g = perso_color[3*10*1+rank*3+1]/255.0f;
            m_perso.colorCombi.b = perso_color[3*10*1+rank*3+2]/255.0f;
        }
        if ( index == 1 )
        {
            m_perso.colorBand.r = perso_color[3*10*2+rank*3+0]/255.0f;
            m_perso.colorBand.g = perso_color[3*10*2+rank*3+1]/255.0f;
            m_perso.colorBand.b = perso_color[3*10*2+rank*3+2]/255.0f;
        }
    }
}

// Updates the color of the character.

void CMainDialog::ColorPerso()
{
    CWindow*        pw;
    CSlider*        ps;
    Gfx::Color   color;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == 0 )  return;

    color.a = 0.0f;

    ps = static_cast<CSlider*>(pw->SearchControl(EVENT_INTERFACE_PCRa));
    if ( ps != 0 )  color.r = ps->GetVisibleValue()/255.0f;
    ps = static_cast<CSlider*>(pw->SearchControl(EVENT_INTERFACE_PCGa));
    if ( ps != 0 )  color.g = ps->GetVisibleValue()/255.0f;
    ps = static_cast<CSlider*>(pw->SearchControl(EVENT_INTERFACE_PCBa));
    if ( ps != 0 )  color.b = ps->GetVisibleValue()/255.0f;
    if ( m_persoTab == 1 )  m_perso.colorCombi = color;

    ps = static_cast<CSlider*>(pw->SearchControl(EVENT_INTERFACE_PCRb));
    if ( ps != 0 )  color.r = ps->GetVisibleValue()/255.0f;
    ps = static_cast<CSlider*>(pw->SearchControl(EVENT_INTERFACE_PCGb));
    if ( ps != 0 )  color.g = ps->GetVisibleValue()/255.0f;
    ps = static_cast<CSlider*>(pw->SearchControl(EVENT_INTERFACE_PCBb));
    if ( ps != 0 )  color.b = ps->GetVisibleValue()/255.0f;
    if ( m_persoTab == 0 )  m_perso.colorHair = color;
    else                    m_perso.colorBand = color;
}

// Updates the default settings of the character.

void CMainDialog::DefPerso()
{
    m_perso.colorCombi.r = 206.0f/256.0f;
    m_perso.colorCombi.g = 206.0f/256.0f;
    m_perso.colorCombi.b = 204.0f/256.0f;  // ~white
    m_perso.colorBand.r  = 255.0f/256.0f;
    m_perso.colorBand.g  = 132.0f/256.0f;
    m_perso.colorBand.b  =   1.0f/256.0f;  // orange

    if ( m_perso.face == 0 )  // normal ?
    {
        m_perso.glasses = 0;
        m_perso.colorHair.r =  90.0f/256.0f;
        m_perso.colorHair.g =  95.0f/256.0f;
        m_perso.colorHair.b =  85.0f/256.0f;  // black
    }
    if ( m_perso.face == 1 )  // bald ?
    {
        m_perso.glasses = 0;
        m_perso.colorHair.r =  83.0f/256.0f;
        m_perso.colorHair.g =  64.0f/256.0f;
        m_perso.colorHair.b =  51.0f/256.0f;  // brown
    }
    if ( m_perso.face == 2 )  // carlos ?
    {
        m_perso.glasses = 1;
        m_perso.colorHair.r =  85.0f/256.0f;
        m_perso.colorHair.g =  48.0f/256.0f;
        m_perso.colorHair.b =   9.0f/256.0f;  // brown
    }
    if ( m_perso.face == 3 )  // blond ?
    {
        m_perso.glasses = 4;
        m_perso.colorHair.r = 255.0f/256.0f;
        m_perso.colorHair.g = 255.0f/256.0f;
        m_perso.colorHair.b = 181.0f/256.0f;  // yellow
    }

    m_perso.colorHair.a  = 0.0f;
    m_perso.colorCombi.a = 0.0f;
    m_perso.colorBand.a  = 0.0f;
}


// Indicates if there is at least one backup.

bool CMainDialog::IsIOReadScene()
{
    FILE*   file;
    std::string filename;

    //TODO: Change this to point user dir acocrding to operating system
    filename = m_savegameDir + "/" + m_main->GetGamerName() + "/" + "save" + m_sceneName[0] + "000/data.sav";
    file = fopen(filename.c_str(), "r");
    if ( file == NULL )  return false;
    fclose(file);
    return true;
}

// Builds the file name by default.

void CMainDialog::IOReadName()
{
    FILE*       file;
    CWindow*    pw;
    CEdit*      pe;
    std::string filename;
    char        op[100];
    char        op_i18n[100];
    char        line[500];
    char        resume[100];
    char        name[100];
    time_t      now;
    int         i;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == 0 )  return;
    pe = static_cast<CEdit*>(pw->SearchControl(EVENT_INTERFACE_IONAME));
    if ( pe == 0 )  return;

    sprintf(resume, "%s %d", m_sceneName, m_chap[m_index]+1);
    BuildSceneName(filename, m_sceneName, (m_chap[m_index]+1)*100);
    sprintf(op, "Title.E");
    sprintf(op_i18n, "Title.%c", m_app->GetLanguageChar() );

    file = fopen(filename.c_str(), "r");
    if ( file != NULL )
    {
        while ( fgets(line, 500, file) != NULL )
        {
            for ( i=0 ; i<500 ; i++ )
            {
                if ( line[i] == '\t' )  line[i] = ' ';  // replaces tab by space
                if ( line[i] == '/' && line[i+1] == '/' )
                {
                    line[i] = 0;
                    break;
                }
            }

            if ( Cmd(line, op) )
            {
                OpString(line, "resume", resume);
            }
            if ( Cmd(line, op_i18n) )
            {
                OpString(line, "resume", resume);
                break;
            }
        }
        fclose(file);
    }

    time(&now);
    TimeToAscii(now, line);
    sprintf(name, "%s %d - %s", resume, m_sel[m_index]+1, line);
    pe->SetText(name);
    pe->SetCursor(strlen(name), 0);
    pe->SetFocus(true);
}

// Updates the list of games recorded on disk.

void CMainDialog::IOReadList()
{
    FILE*       file = NULL;
    CWindow*    pw;
    CList*      pl;
    char        line[500];
    char        name[100];
    int         i, j;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == 0 )  return;
    pl = static_cast<CList*>(pw->SearchControl(EVENT_INTERFACE_IOLIST));
    if ( pl == 0 )  return;

    pl->Flush();

    for ( j=0 ; j<999 ; j++ )
    {
        std::string filename;
        std::ostringstream rankStream;
        rankStream << std::setfill('0') << std::setw(3) << j;
        filename = m_savegameDir + "/" + m_main->GetGamerName() + "/save" + m_sceneName[0] + rankStream.str()+ "/data.sav";

        // sprintf(filename, "%s\\%s\\save%c%.3d\\data.sav", m_savegameDir, m_main->GetGamerName(), m_sceneName[0], j);
        file = fopen(filename.c_str(), "r");
        if ( file == NULL )  break;

        while ( fgets(line, 500, file) != NULL )
        {
            for ( i=0 ; i<500 ; i++ )
            {
                if ( line[i] == '\t' )  line[i] = ' ';  // replaces tab by space
                if ( line[i] == '/' && line[i+1] == '/' )
                {
                    line[i] = 0;
                    break;
                }
            }

            if ( Cmd(line, "Title") )
            {
                OpString(line, "text", name);
                break;
            }
        }
        fclose(file);

        pl->SetName(j, name);
    }

    if ( m_phase == PHASE_WRITE  ||
         m_phase == PHASE_WRITEs )
    {
        GetResource(RES_TEXT, RT_IO_NEW, name);
        pl->SetName(j, name);
        j ++;
    }

    pl->SetSelect(j-1);
    pl->ShowSelect(false);  // shows the selected columns
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
    if ( pw == 0 )  return;
    pl = static_cast<CList*>(pw->SearchControl(EVENT_INTERFACE_IOLIST));
    if ( pl == 0 )  return;
    pi = static_cast<CImage*>(pw->SearchControl(EVENT_INTERFACE_IOIMAGE));
    if ( pi == 0 )  return;

    sel = pl->GetSelect();
    max = pl->GetTotal();

    std::string filename;
    std::ostringstream rankStream;
    rankStream << std::setfill('0') << std::setw(3) << sel;
    filename = m_savegameDir + "/" + m_main->GetGamerName() + "/save" + m_sceneName[0] + rankStream.str()+ "/screen.png";

    if ( m_phase == PHASE_WRITE  ||
         m_phase == PHASE_WRITEs )
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
        if ( pb != 0 )
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
    if ( sel == -1 )
    {
        m_sound->Play(SOUND_TZOING);
        return;
    }


    std::ostringstream rankStream;
    std::string fileName;
    rankStream << std::setfill('0') << std::setw(3) << sel;
    fileName = m_savegameDir + "/" + m_main->GetGamerName() + "/save" + m_sceneName[0] + rankStream.str();
    try
    {
        if (fs::exists(fileName) && fs::is_directory(fileName))
        {
            fs::remove_all(fileName);
        }
    }
    catch (std::exception & e)
    {
        GetLogger()->Error("Error on removing directory %s : %s\n", e.what());
    }

    IOReadList();
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
    if ( pw == 0 )  return false;
    pl = static_cast<CList*>(pw->SearchControl(EVENT_INTERFACE_IOLIST));
    if ( pl == 0 )  return false;
    pe = static_cast<CEdit*>(pw->SearchControl(EVENT_INTERFACE_IONAME));
    if ( pe == 0 )  return false;

    sel = pl->GetSelect();
    if ( sel == -1 )  return false;

    std::string directoryName;
    std::string fileName;
    std::string fileCBot;
    std::ostringstream selectStream;

    //TODO: Change this to point user dir according to operating system
    GetLogger()->Debug("Creating save directory\n");
    selectStream << std::setfill('0') << std::setw(3) << sel;
    directoryName =  m_savegameDir + "/" + m_main->GetGamerName() + "/" + "save" + m_sceneName[0] + selectStream.str();
    if (!fs::exists(directoryName))
    {
        fs::create_directories(directoryName);
    }

    fileName = directoryName + "/data.sav";
    fileCBot = directoryName + "/cbot.run";
    pe->GetText(info, 100);
    m_main->IOWriteScene(fileName.c_str(), fileCBot.c_str(), info);

    m_shotDelay = 3;
    m_shotName = directoryName + "/screen.png";

    return true;
}

// Reads the scene.

bool CMainDialog::IOReadScene()
{
    FILE*       file;
    CWindow*    pw;
    CList*      pl;
    char        line[500];
    char        dir[100];
    int         sel, i;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == 0 )  return false;
    pl = static_cast<CList*>(pw->SearchControl(EVENT_INTERFACE_IOLIST));
    if ( pl == 0 )  return false;

    sel = pl->GetSelect();
    if ( sel == -1 )  return false;

    //TODO: Change this to point user dir according to operating system
    std::string fileName;
    std::string fileCbot;
    std::string directoryName;
    std::ostringstream selectStream;
    selectStream << std::setfill('0') << std::setw(3) << sel;
    directoryName = m_savegameDir + "/" + m_main->GetGamerName() + "/" + "save" + m_sceneName[0] + selectStream.str();

    fileName = directoryName + "/data.sav";
    fileCbot = directoryName + "/cbot.run";

    file = fopen(fileName.c_str(), "r");
    if ( file == NULL )  return false;

    while ( fgets(line, 500, file) != NULL )
    {
        for ( i=0 ; i<500 ; i++ )
        {
            if ( line[i] == '\t' )  line[i] = ' ';  // replaces tab by space
            if ( line[i] == '/' && line[i+1] == '/' )
            {
                line[i] = 0;
                break;
            }
        }

        if ( Cmd(line, "Mission") )
        {
            OpString(line, "base", m_sceneName);
            m_sceneRank = OpInt(line, "rank", 0);

            if ( strcmp(m_sceneName, "user") == 0 )
            {
                m_sceneRank = m_sceneRank%100;
                OpString(line, "dir", dir);
                for ( i=0 ; i<m_userTotal ; i++ )
                {
                    if ( strcmp(m_userList[i].c_str(), dir) == 0 )
                    {
                        m_sceneRank += (i+1)*100;
                        break;
                    }
                }
                if ( m_sceneRank/100 == 0 )
                {
                    fclose(file);
                    return false;
                }
            }
        }
    }
    fclose(file);

    m_chap[m_index] = (m_sceneRank/100)-1;
    m_sel[m_index]  = (m_sceneRank%100)-1;

    m_sceneRead = fileName;
    m_stackRead = fileCbot;
    return true;
}


// Returns the number of accessible chapters.

int CMainDialog::GetChapPassed()
{
    int         j;

    if ( m_main->GetShowAll() )  return 9;

    for ( j=0 ; j<9 ; j++ )
    {
        if ( !GetGamerInfoPassed((j+1)*100) )
        {
            return j;
        }
    }
    return 9;
}

// Updates the lists according to the cheat code.

void CMainDialog::AllMissionUpdate()
{
    if ( m_phase == PHASE_TRAINER ||
         m_phase == PHASE_DEFI    ||
         m_phase == PHASE_MISSION ||
         m_phase == PHASE_FREE    ||
         m_phase == PHASE_TEEN    ||
         m_phase == PHASE_USER    ||
         m_phase == PHASE_PROTO   )
    {
        UpdateSceneChap(m_chap[m_index]);
        UpdateSceneList(m_chap[m_index], m_sel[m_index]);
    }
}

// Updates the chapters of exercises or missions.

void CMainDialog::UpdateSceneChap(int &chap)
{
    FILE*       file = NULL;
    CWindow*    pw;
    CList*      pl;
    //struct _finddata_t fileBuffer;
    std::string fileName;
    char        op[100];
    char        op_i18n[100];
    char        line[500];
    char        name[100];
    int         i, j;
    bool        bPassed;

    memset(op, 0, 100);
    memset(op_i18n, 0, 100);
    memset(line, 0, 500);
    memset(name, 0, 100);

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == 0 )  return;
    pl = static_cast<CList*>(pw->SearchControl(EVENT_INTERFACE_CHAP));
    if ( pl == 0 )  return;

    pl->Flush();

    if ( m_phase == PHASE_USER )
    {
        j = 0;
        fs::directory_iterator dirIt(m_savegameDir), dirEndIt;
        m_userList.clear();

        BOOST_FOREACH (const fs::path & p, std::make_pair(dirIt, dirEndIt))
        {
            if (fs::is_directory(p))
            {
                m_userList.push_back(p.leaf().string());
            }
        }
        m_userTotal = m_userList.size();

        for ( j=0 ; j<m_userTotal ; j++ )
        {
            BuildSceneName(fileName, m_sceneName, (j+1)*100);
            file = fopen(fileName.c_str(), "r");
            if ( file == NULL )
            {
                strcpy(name, m_userList[j].c_str());
            }
            else
            {
                BuildResumeName(name, m_sceneName, j+1);  // default name
                sprintf(op, "Title.E");
                sprintf(op_i18n, "Title.%c", m_app->GetLanguageChar());

                while ( fgets(line, 500, file) != NULL )
                {
                    for ( i=0 ; i<500 ; i++ )
                    {
                        if ( line[i] == '\t' )  line[i] = ' ';  // replaces tab by space
                        if ( line[i] == '/' && line[i+1] == '/' )
                        {
                            line[i] = 0;
                            break;
                        }
                    }

                    if ( Cmd(line, op) )
                    {
                        OpString(line, "text", name);
                    }
                    if ( Cmd(line, op_i18n) )
                    {
                        OpString(line, "text", name);
                        break;
                    }
                }
                fclose(file);
            }

            pl->SetName(j, name);
            pl->SetEnable(j, true);
        }
    }
    else
    {
        for ( j=0 ; j<9 ; j++ )
        {
/* TODO: #if _SCHOOL
            if ( m_phase == PHASE_MISSION )  break;
            if ( m_phase == PHASE_FREE    )  break;
#if _CEEBOTDEMO
            if ( m_phase == PHASE_TRAINER && j >= 2 )  break;
#endif
#endif
#if _DEMO
            if ( m_phase == PHASE_MISSION && j >= 4 )  break;
            if ( m_phase == PHASE_TRAINER && j >= 1 )  break;
#endif */
            BuildSceneName(fileName, m_sceneName, (j+1)*100);
            file = fopen(fileName.c_str(), "r");
            if ( file == NULL )  break;

            BuildResumeName(name, m_sceneName, j+1);  // default name
            sprintf(op, "Title.E");
            sprintf(op_i18n, "Title.%c", m_app->GetLanguageChar());

            while ( fgets(line, 500, file) != NULL )
            {
                for ( i=0 ; i<500 ; i++ )
                {
                    if ( line[i] == '\t' )  line[i] = ' ';  // replaces tab by space
                    if ( line[i] == '/' && line[i+1] == '/' )
                    {
                        line[i] = 0;
                        break;
                    }
                }

                if ( Cmd(line, op) )
                {
                    OpString(line, "text", name);
                }
                if ( Cmd(line, op_i18n) )
                {
                    OpString(line, "text", name);
                    break;
                }
            }
            fclose(file);

            bPassed = GetGamerInfoPassed((j+1)*100);
            sprintf(line, "%d: %s", j+1, name);
            pl->SetName(j, line);
            pl->SetCheck(j, bPassed);
            pl->SetEnable(j, true);

            if ( m_phase == PHASE_MISSION && !m_main->GetShowAll() && !bPassed )
            {
                j ++;
                break;
            }

/* TODO: #if _TEEN
            if ( m_phase == PHASE_TRAINER && !m_main->GetShowAll() && !bPassed )
            {
                j ++;
                break;
            }
#endif*/

            if ( m_phase == PHASE_FREE && j == m_accessChap )
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
    FILE*       file = NULL;
    CWindow*    pw;
    CList*      pl;
    std::string fileName;
    char        op[100];
    char        op_i18n[100];
    char        line[500];
    char        name[100];
    int         i, j;
    bool        bPassed;

    memset(op, 0, 100);
    memset(op_i18n, 0, 100);
    memset(line, 0, 500);
    memset(name, 0, 100);

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == 0 )  return;
    pl = static_cast<CList*>(pw->SearchControl(EVENT_INTERFACE_LIST));
    if ( pl == 0 )  return;

    pl->Flush();

    for ( j=0 ; j<99 ; j++ )
    {
/* TODO: #if _SCHOOL
        if ( m_phase == PHASE_MISSION )  break;
        if ( m_phase == PHASE_FREE    )  break;
#if _CEEBOTDEMO
#if _TEEN
        if ( m_phase == PHASE_TRAINER && j >= 5 )  break;
#else
        if ( m_phase == PHASE_TRAINER && j >= 3 )  break;
#endif
#endif
#endif
#if _DEMO
        if ( m_phase == PHASE_MISSION && j >= 3 )  break;
        if ( m_phase == PHASE_TRAINER && j >= 5 )  break;
#endif */
        BuildSceneName(fileName, m_sceneName, (chap+1)*100+(j+1));
        file = fopen(fileName.c_str(), "r");
        if ( file == NULL )  break;

        BuildResumeName(name, m_sceneName, j+1);  // default name
        sprintf(op, "Title.E");
        sprintf(op_i18n, "Title.%c", m_app->GetLanguageChar());

        while ( fgets(line, 500, file) != NULL )
        {
            for ( i=0 ; i<500 ; i++ )
            {
                if ( line[i] == '\t' )  line[i] = ' ';  // replaces tab by space
                if ( line[i] == '/' && line[i+1] == '/' )
                {
                    line[i] = 0;
                    break;
                }
            }

            if ( Cmd(line, op) )
            {
                OpString(line, "text", name);
            }
            if ( Cmd(line, op_i18n) )
            {
                OpString(line, "text", name);
                break;
            }
        }
        fclose(file);

        bPassed = GetGamerInfoPassed((chap+1)*100+(j+1));
        sprintf(line, "%d: %s", j+1, name);
        pl->SetName(j, line);
        pl->SetCheck(j, bPassed);
        pl->SetEnable(j, true);

        if ( m_phase == PHASE_MISSION && !m_main->GetShowAll() && !bPassed )
        {
            j ++;
            break;
        }

/* TODO: #if _TEEN
        if ( m_phase == PHASE_TRAINER && !m_main->GetShowAll() && !bPassed )
        {
            j ++;
            break;
        }
#endif*/
    }

    BuildSceneName(fileName, m_sceneName, (chap+1)*100+(j+1));
    file = fopen(fileName.c_str(), "r");
    if ( file == NULL )
    {
        m_maxList = j;
    }
    else
    {
        m_maxList = j+1;  // this is not the last!
        fclose(file);
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

    if ( m_phase == PHASE_TRAINER ||
         m_phase == PHASE_DEFI    ||
         m_phase == PHASE_MISSION ||
         m_phase == PHASE_FREE    ||
         m_phase == PHASE_TEEN    ||
         m_phase == PHASE_USER    ||
         m_phase == PHASE_PROTO   )
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

void CMainDialog::UpdateSceneResume(int rank)
{
    FILE*       file = NULL;
    CWindow*    pw;
    CEdit*      pe;
    CCheck*     pc;
    std::string fileName;
    char        op[100];
    char        op_i18n[100];
    char        line[500];
    char        name[500];
    int         i, numTry;
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
        numTry  = GetGamerInfoTry(rank);
        bPassed = GetGamerInfoPassed(rank);
        bVisible = ( numTry > 2 || bPassed || m_main->GetShowSoluce() );
        if ( !GetSoluce4() )  bVisible = false;
        pc->SetState(STATE_VISIBLE, bVisible);
        if ( !bVisible )
        {
            pc->ClearState(STATE_CHECK);
            m_bSceneSoluce = false;
        }
    }

    BuildSceneName(fileName, m_sceneName, rank);
    sprintf(op, "Resume.E");
    sprintf(op_i18n, "Resume.%c", m_app->GetLanguageChar());

    file = fopen(fileName.c_str(), "r");
    if ( file == NULL )  return;

    name[0] = 0;
    while ( fgets(line, 500, file) != NULL )
    {
        for ( i=0 ; i<500 ; i++ )
        {
            if ( line[i] == '\t' )  line[i] = ' ';  // replaces tab by space
            if ( line[i] == '/' && line[i+1] == '/' )
            {
                line[i] = 0;
                break;
            }
        }

        if ( Cmd(line, op) )
        {
            OpString(line, "text", name);
        }
        if ( Cmd(line, op_i18n) )
        {
            OpString(line, "text", name);
            break;
        }
    }
    fclose(file);

    pe->SetText(name);
}

// Updates the list of devices.

void CMainDialog::UpdateDisplayDevice()
{
    CWindow*    pw;
    CList*      pl;
    char        bufDevices[1000];
    char        bufModes[5000];
    int         i, j, totalDevices, selectDevices, totalModes, selectModes;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == 0 )  return;
    pl = static_cast<CList*>(pw->SearchControl(EVENT_LIST1));
    if ( pl == 0 )  return;
    pl->Flush();

    bufModes[0] = 0;
    /* TODO: remove device choice
    m_engine->EnumDevices(bufDevices, 1000,
                          bufModes,   5000,
                          totalDevices, selectDevices,
                          totalModes,   selectModes);*/

    i = 0;
    j = 0;
    while ( bufDevices[i] != 0 )
    {
        pl->SetName(j++, bufDevices+i);
        while ( bufDevices[i++] != 0 );
    }

    pl->SetSelect(selectDevices);
    pl->ShowSelect(false);

    m_setupSelDevice = selectDevices;
}

// Updates the list of modes.

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
    for (Math::IntPoint mode : modes) {
	mode_text.str("");
	mode_text << mode.x << "x" << mode.y;
	pl->SetName(i++, mode_text.str().c_str());
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
    char*       device;
    char*       mode;
    bool        bFull;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == 0 )  return;

    pl = static_cast<CList*>(pw->SearchControl(EVENT_LIST1));
    if ( pl == 0 )  return;
    m_setupSelDevice = pl->GetSelect();
    device = pl->GetName(m_setupSelDevice);

    pl = static_cast<CList*>(pw->SearchControl(EVENT_LIST2));
    if ( pl == 0 )  return;
    m_setupSelMode = pl->GetSelect();
    mode = pl->GetName(m_setupSelMode);

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_FULL));
    if ( pc == 0 )  return;
    bFull = pc->TestState(STATE_CHECK);
    m_setupFull = bFull;

    // TODO: remove device choice
    // m_engine->ChangeDevice(device, mode, bFull);

    if ( m_bSimulSetup )
    {
        m_main->ChangeColor();
        m_main->UpdateMap();
    }
}



// Updates the "apply" button.

void CMainDialog::UpdateApply()
{
    CWindow*    pw;
    CButton*    pb;
    CList*      pl;
    CCheck*     pc;
    int         sel1, sel2;
    bool        bFull;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == 0 )  return;

    pb = static_cast<CButton*>(pw->SearchControl(EVENT_INTERFACE_APPLY));
    if ( pb == 0 )  return;

    pl = static_cast<CList*>(pw->SearchControl(EVENT_LIST1));
    if ( pl == 0 )  return;
    sel1 = pl->GetSelect();

    pl = static_cast<CList*>(pw->SearchControl(EVENT_LIST2));
    if ( pl == 0 )  return;
    sel2 = pl->GetSelect();

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_FULL));
    bFull = pc->TestState(STATE_CHECK);

    if ( sel1 == m_setupSelDevice &&
         sel2 == m_setupSelMode   &&
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
        /* TODO: nice mouse?
        pc->SetState(STATE_CHECK, m_engine->GetNiceMouse());
        pc->SetState(STATE_ENABLE, m_engine->GetNiceMouseCap());*/
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

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_SHADOW));
    if ( pc != 0 )
    {
        pc->SetState(STATE_CHECK, m_engine->GetShadow());
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

    pv = static_cast<CEditValue*>(pw->SearchControl(EVENT_INTERFACE_TEXTURE));
    if ( pv != 0 )
    {
        value = static_cast<float>(m_engine->GetTextureQuality());
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

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_SOUND3D));
    if ( pc != 0 )
    {
        pc->SetState(STATE_CHECK, m_sound->GetSound3D());
        pc->SetState(STATE_ENABLE, m_sound->GetSound3DCap());
    }
}

// Updates the engine function of the buttons after the setup phase.

void CMainDialog::ChangeSetupButtons()
{
    CWindow*    pw;
    CEditValue* pv;
    CSlider*    ps;
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

    pv = static_cast<CEditValue*>(pw->SearchControl(EVENT_INTERFACE_TEXTURE));
    if ( pv != 0 )
    {
        value = pv->GetValue();
        m_engine->SetTextureQuality(static_cast<int>(value));
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
}


// Memorizes all the settings.

void CMainDialog::SetupMemorize()
{
    GetProfile().SetLocalProfileString("Directory", "scene",    m_sceneDir);
    GetProfile().SetLocalProfileString("Directory", "savegame", m_savegameDir);
    GetProfile().SetLocalProfileString("Directory", "public",   m_publicDir);
    GetProfile().SetLocalProfileString("Directory", "user",     m_userDir);
    GetProfile().SetLocalProfileString("Directory", "files",    m_filesDir);
    GetProfile().SetLocalProfileInt("Setup", "Tooltips", m_bTooltip);
    GetProfile().SetLocalProfileInt("Setup", "InterfaceGlint", m_bGlint);
    GetProfile().SetLocalProfileInt("Setup", "InterfaceGlint", m_bRain);
    GetProfile().SetLocalProfileInt("Setup", "Soluce4", m_bSoluce4);
    GetProfile().SetLocalProfileInt("Setup", "Movies", m_bMovies);
    GetProfile().SetLocalProfileInt("Setup", "NiceReset", m_bNiceReset);
    GetProfile().SetLocalProfileInt("Setup", "HimselfDamage", m_bHimselfDamage);
    GetProfile().SetLocalProfileInt("Setup", "CameraScroll", m_bCameraScroll);
    GetProfile().SetLocalProfileInt("Setup", "CameraInvertX", m_bCameraInvertX);
    GetProfile().SetLocalProfileInt("Setup", "InterfaceEffect", m_bEffect);
    GetProfile().SetLocalProfileInt("Setup", "GroundShadow", m_engine->GetShadow());
    GetProfile().SetLocalProfileInt("Setup", "GroundSpot", m_engine->GetGroundSpot());
    GetProfile().SetLocalProfileInt("Setup", "ObjectDirty", m_engine->GetDirty());
    GetProfile().SetLocalProfileInt("Setup", "FogMode", m_engine->GetFog());
    GetProfile().SetLocalProfileInt("Setup", "LensMode", m_engine->GetLensMode());
    GetProfile().SetLocalProfileInt("Setup", "SkyMode", m_engine->GetSkyMode());
    GetProfile().SetLocalProfileInt("Setup", "PlanetMode", m_engine->GetPlanetMode());
    GetProfile().SetLocalProfileInt("Setup", "LightMode", m_engine->GetLightMode());
    GetProfile().SetLocalProfileFloat("Setup", "ParticleDensity", m_engine->GetParticleDensity());
    GetProfile().SetLocalProfileFloat("Setup", "ClippingDistance", m_engine->GetClippingDistance());
    GetProfile().SetLocalProfileFloat("Setup", "ObjectDetail", m_engine->GetObjectDetail());
    GetProfile().SetLocalProfileFloat("Setup", "GadgetQuantity", m_engine->GetGadgetQuantity());
    GetProfile().SetLocalProfileInt("Setup", "TextureQuality", m_engine->GetTextureQuality());
    GetProfile().SetLocalProfileInt("Setup", "TotoMode", m_engine->GetTotoMode());
    GetProfile().SetLocalProfileInt("Setup", "AudioVolume", m_sound->GetAudioVolume());
    GetProfile().SetLocalProfileInt("Setup", "MusicVolume", m_sound->GetMusicVolume());
    GetProfile().SetLocalProfileInt("Setup", "Sound3D", m_sound->GetSound3D());
    GetProfile().SetLocalProfileInt("Setup", "EditIndentMode", m_engine->GetEditIndentMode());
    GetProfile().SetLocalProfileInt("Setup", "EditIndentValue", m_engine->GetEditIndentValue());
    
    /* screen setup */
    if (m_setupFull)
	GetProfile().SetLocalProfileInt("Setup", "Fullscreen", 1);
    else
	GetProfile().SetLocalProfileInt("Setup", "Fullscreen", 0);
    
    CList *pl;
    CWindow *pw;
    pw = static_cast<CWindow *>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw != 0 ) {
	pl = static_cast<CList *>(pw->SearchControl(EVENT_LIST2));
	if ( pl != 0 ) {
	    GetProfile().SetLocalProfileInt("Setup", "Resolution", pl->GetSelect());
	}
    }
    
    std::stringstream key;
    for (int i = 0; i < INPUT_SLOT_MAX; i++)
    {
        InputBinding b = m_main->GetInputBinding(static_cast<InputSlot>(i));

        key << b.primary << " ";
        key << b.secondary << "  ";
    }

    GetProfile().SetLocalProfileString("Setup", "KeyMap", key.str());

#if _NET
    if ( m_accessEnable )
    {
        iValue = m_accessMission;
        SetLocalProfileInt("Setup", "AccessMission", iValue);

        iValue = m_accessUser;
        SetLocalProfileInt("Setup", "AccessUser", iValue);
    }
#endif

    GetProfile().SetLocalProfileInt("Setup", "DeleteGamer", m_bDeleteGamer);

    // TODO: write graphic engine profile
    //m_engine->WriteProfile();
}

// Remember all the settings.

void CMainDialog::SetupRecall()
{
    float       fValue;
    int         iValue;
    std::string key;

    if ( GetProfile().GetLocalProfileString("Directory", "scene", key) )
    {
        m_sceneDir = key;
    }

    if ( GetProfile().GetLocalProfileString("Directory", "savegame", key) )
    {
        m_savegameDir = key;
    }

    if ( GetProfile().GetLocalProfileString("Directory", "public", key) )
    {
        m_publicDir = key;
    }

    if ( GetProfile().GetLocalProfileString("Directory", "user", key) )
    {
        m_userDir = key;
    }

    if ( GetProfile().GetLocalProfileString("Directory", "files", key) )
    {
        m_filesDir = key;
    }


    if ( GetProfile().GetLocalProfileInt("Setup", "TotoMode", iValue) )
    {
        m_engine->SetTotoMode(iValue);
    }

    if ( GetProfile().GetLocalProfileInt("Setup", "Tooltips", iValue) )
    {
        m_bTooltip = iValue;
    }

    if ( GetProfile().GetLocalProfileInt("Setup", "InterfaceGlint", iValue) )
    {
        m_bGlint = iValue;
    }

    if ( GetProfile().GetLocalProfileInt("Setup", "InterfaceGlint", iValue) )
    {
        m_bRain = iValue;
    }

    // TODO
    // if ( GetProfile().GetLocalProfileInt("Setup", "NiceMouse", iValue) )
    // {
    //     m_engine->SetNiceMouse(iValue);
    // }

    if ( GetProfile().GetLocalProfileInt("Setup", "Soluce4", iValue) )
    {
        m_bSoluce4 = iValue;
    }

    if ( GetProfile().GetLocalProfileInt("Setup", "Movies", iValue) )
    {
        m_bMovies = iValue;
    }

    if ( GetProfile().GetLocalProfileInt("Setup", "NiceReset", iValue) )
    {
        m_bNiceReset = iValue;
    }

    if ( GetProfile().GetLocalProfileInt("Setup", "HimselfDamage", iValue) )
    {
        m_bHimselfDamage = iValue;
    }

    if ( GetProfile().GetLocalProfileInt("Setup", "CameraScroll", iValue) )
    {
        m_bCameraScroll = iValue;
        m_camera->SetCameraScroll(m_bCameraScroll);
    }

    if ( GetProfile().GetLocalProfileInt("Setup", "CameraInvertX", iValue) )
    {
        m_bCameraInvertX = iValue;
        m_camera->SetCameraInvertX(m_bCameraInvertX);
    }

    if ( GetProfile().GetLocalProfileInt("Setup", "CameraInvertY", iValue) )
    {
        m_bCameraInvertY = iValue;
        m_camera->SetCameraInvertY(m_bCameraInvertY);
    }

    if ( GetProfile().GetLocalProfileInt("Setup", "InterfaceEffect", iValue) )
    {
        m_bEffect = iValue;
    }

    if ( GetProfile().GetLocalProfileInt("Setup", "GroundShadow", iValue) )
    {
        m_engine->SetShadow(iValue);
    }

    if ( GetProfile().GetLocalProfileInt("Setup", "GroundSpot", iValue) )
    {
        m_engine->SetGroundSpot(iValue);
    }

    if ( GetProfile().GetLocalProfileInt("Setup", "ObjectDirty", iValue) )
    {
        m_engine->SetDirty(iValue);
    }

    if ( GetProfile().GetLocalProfileInt("Setup", "FogMode", iValue) )
    {
        m_engine->SetFog(iValue);
        m_camera->SetOverBaseColor(Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f)); // TODO: color ok?
    }

    if ( GetProfile().GetLocalProfileInt("Setup", "LensMode", iValue) )
    {
        m_engine->SetLensMode(iValue);
    }

    if ( GetProfile().GetLocalProfileInt("Setup", "SkyMode", iValue) )
    {
        m_engine->SetSkyMode(iValue);
    }

    if ( GetProfile().GetLocalProfileInt("Setup", "PlanetMode", iValue) )
    {
        m_engine->SetPlanetMode(iValue);
    }

    if ( GetProfile().GetLocalProfileInt("Setup", "LightMode", iValue) )
    {
        m_engine->SetLightMode(iValue);
    }
    // TODO
    // if ( GetProfile().GetLocalProfileInt("Setup", "UseJoystick", iValue) )
    // {
    //     m_engine->SetJoystick(iValue);
    // }

    if ( GetProfile().GetLocalProfileFloat("Setup", "ParticleDensity", fValue) )
    {
        m_engine->SetParticleDensity(fValue);
    }

    if ( GetProfile().GetLocalProfileFloat("Setup", "ClippingDistance", fValue) )
    {
        m_engine->SetClippingDistance(fValue);
    }

    if ( GetProfile().GetLocalProfileFloat("Setup", "ObjectDetail", fValue) )
    {
        m_engine->SetObjectDetail(fValue);
    }

    if ( GetProfile().GetLocalProfileFloat("Setup", "GadgetQuantity", fValue) )
    {
        m_engine->SetGadgetQuantity(fValue);
    }

    if ( GetProfile().GetLocalProfileInt("Setup", "TextureQuality", iValue) )
    {
        m_engine->SetTextureQuality(iValue);
    }

    if ( GetProfile().GetLocalProfileInt("Setup", "AudioVolume", iValue) )
    {
        m_sound->SetAudioVolume(iValue);
    }

    if ( GetProfile().GetLocalProfileInt("Setup", "MusicVolume", iValue) )
    {
        m_sound->SetMusicVolume(iValue);
    }

    if ( GetProfile().GetLocalProfileInt("Setup", "EditIndentMode", iValue) )
    {
        m_engine->SetEditIndentMode(iValue);
    }

    if ( GetProfile().GetLocalProfileInt("Setup", "EditIndentValue", iValue) )
    {
        m_engine->SetEditIndentValue(iValue);
    }

    if (GetProfile().GetLocalProfileString("Setup", "KeyMap", key))
    {
        std::stringstream skey;
        skey.str(key);
        for (int i = 0; i < INPUT_SLOT_MAX; i++)
        {
            InputBinding b;
            skey >> b.primary;
            skey >> b.secondary;
            m_main->SetInputBinding(static_cast<InputSlot>(i), b);
         }
    }

#if _NET
    if ( m_accessEnable )
    {
        if ( GetProfile().GetLocalProfileInt("Setup", "AccessMission", iValue) )
        {
            m_accessMission = iValue;
        }

        if ( GetProfile().GetLocalProfileInt("Setup", "AccessUser", iValue) )
        {
            m_accessUser = iValue;
        }
    }
#endif

    if ( GetProfile().GetLocalProfileInt("Setup", "DeleteGamer", iValue) )
    {
        m_bDeleteGamer = iValue;
    }
    
    if ( GetProfile().GetLocalProfileInt("Setup", "Resolution", iValue) ) {
	m_setupSelMode = iValue;
    }
    
    if ( GetProfile().GetLocalProfileInt("Setup", "Fullscreen", iValue) ) {
	m_setupFull = (iValue == 1);
    }
}


// Changes the general level of quality.

void CMainDialog::ChangeSetupQuality(int quality)
{
    bool    bEnable;
    float   value;
    int     iValue;

    bEnable = (quality >= 0);
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

    if ( quality <  0 )  iValue = 0;
    if ( quality == 0 )  iValue = 1;
    if ( quality >  0 )  iValue = 2;
    m_engine->SetTextureQuality(iValue);

    // TODO: first execute adapt?
    //m_engine->FirstExecuteAdapt(false);
}


// Redefinable keys:

static InputSlot key_table[KEY_TOTAL] =
{
    INPUT_SLOT_LEFT,
    INPUT_SLOT_RIGHT,
    INPUT_SLOT_UP,
    INPUT_SLOT_DOWN,
    INPUT_SLOT_GUP,
    INPUT_SLOT_GDOWN,
    INPUT_SLOT_ACTION,
    INPUT_SLOT_CAMERA,
    INPUT_SLOT_VISIT,
    INPUT_SLOT_NEXT,
    INPUT_SLOT_HUMAN,
    INPUT_SLOT_DESEL,
    INPUT_SLOT_NEAR,
    INPUT_SLOT_AWAY,
    INPUT_SLOT_HELP,
    INPUT_SLOT_PROG,
    INPUT_SLOT_CBOT,
    INPUT_SLOT_SPEED10,
    INPUT_SLOT_SPEED15,
    INPUT_SLOT_SPEED20,
    INPUT_SLOT_QUIT,
};

static EventType key_event[KEY_TOTAL] =
{
    EVENT_INTERFACE_KLEFT,
    EVENT_INTERFACE_KRIGHT,
    EVENT_INTERFACE_KUP,
    EVENT_INTERFACE_KDOWN,
    EVENT_INTERFACE_KGUP,
    EVENT_INTERFACE_KGDOWN,
    EVENT_INTERFACE_KACTION,
    EVENT_INTERFACE_KCAMERA,
    EVENT_INTERFACE_KVISIT,
    EVENT_INTERFACE_KNEXT,
    EVENT_INTERFACE_KHUMAN,
    EVENT_INTERFACE_KDESEL,
    EVENT_INTERFACE_KNEAR,
    EVENT_INTERFACE_KAWAY,
    EVENT_INTERFACE_KHELP,
    EVENT_INTERFACE_KPROG,
    EVENT_INTERFACE_KCBOT,
    EVENT_INTERFACE_KSPEED10,
    EVENT_INTERFACE_KSPEED15,
    EVENT_INTERFACE_KSPEED20,
    EVENT_INTERFACE_KQUIT,
};

// Updates the list of keys.

void CMainDialog::UpdateKey()
{
    CWindow* pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if (pw == nullptr) return;

    CScroll* ps = static_cast<CScroll*>(pw->SearchControl(EVENT_INTERFACE_KSCROLL));
    if (ps == nullptr) return;

    int first = static_cast<int>(ps->GetVisibleValue()*(KEY_TOTAL-KEY_VISIBLE));

    for (int i = 0; i < KEY_TOTAL; i++)
        pw->DeleteControl(key_event[i]);

    Math::Point dim;
    dim.x = 400.0f/640.0f;
    dim.y =  20.0f/480.0f;
    Math::Point pos;
    pos.x = 110.0f/640.0f;
    pos.y = 168.0f/480.0f + dim.y*(KEY_VISIBLE-1);
    for (int i = 0; i < KEY_VISIBLE; i++)
    {
        pw->CreateKey(pos, dim, -1, key_event[first+i]);
        CKey* pk = static_cast<CKey*>(pw->SearchControl(key_event[first+i]));
        if (pk == nullptr) break;

        pk->SetBinding(m_main->GetInputBinding(key_table[first+i]));
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

    for (int i = 0; i < KEY_TOTAL; i++)
    {
        if ( key_event[i] == event )
        {
            CKey* pk = static_cast<CKey*>(pw->SearchControl(key_event[i]));
            if (pk == nullptr) break;

            m_main->SetInputBinding(key_table[i], pk->GetBinding());
        }
    }
}



// Do you want to quit the current mission?

void CMainDialog::StartAbort()
{
    CWindow*    pw;
    CButton*    pb;
    Math::Point     pos, dim;
    char        name[100];

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
/* TODO: #if _POLISH
    pos.x -=  7.0f/640.0f;
    dim.x += 14.0f/640.0f;
#endif*/
    dim.y = 32.0f/480.0f;

    pos.y = 0.74f;
    pb = pw->CreateButton(pos, dim, -1, EVENT_DIALOG_CANCEL);
    pb->SetState(STATE_SHADOW);
    GetResource(RES_TEXT, RT_DIALOG_NO, name);
    pb->SetName(name);

    if ( m_index == 2 ||  // missions ?
         m_index == 3 ||  // free games?
         m_index == 4 )   // user ?
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
    char        name[100];

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
    char        name[100];
    char        text[100];

    StartDialog(Math::Point(0.7f, 0.3f), false, true, true);
    m_bDialogDelete = true;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW9));
    if ( pw == 0 )  return;

    pos.x = 0.00f;
    pos.y = 0.50f;
    dim.x = 1.00f;
    dim.y = 0.05f;
    GetResource(RES_TEXT, RT_DIALOG_DELGAME, name);
    sprintf(text, name, gamer);
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
    char        name[100];

    StartDialog(Math::Point(0.6f, 0.3f), false, true, true);

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW9));
    if ( pw == 0 )  return;

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
    char        name[100];

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

    pb = static_cast<CButton*>(m_interface->SearchControl(EVENT_BUTTON_QUIT));
    if ( pb != 0 )
    {
        pb->ClearState(STATE_VISIBLE);
    }

    m_bDialogFire = bFire;

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
    CButton*    pb;

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

    pb = static_cast<CButton*>(m_interface->SearchControl(EVENT_BUTTON_QUIT));
    if ( pb != 0 )
    {
        pb->SetState(STATE_VISIBLE);
    }

    StopSuspend();
    m_interface->DeleteControl(EVENT_WINDOW9);
    m_bDialog = false;
}

// Suspends the simulation for a dialog phase.

void CMainDialog::StartSuspend()
{
    m_sound->MuteAll(true);
    m_main->ClearInterface();
    m_bInitPause = m_engine->GetPause();
    m_engine->SetPause(true);
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
    if ( !m_bInitPause )  m_engine->SetPause(false);
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

// Specifies the name of the chosen to play scene.

void CMainDialog::SetSceneName(const char* name)
{
    strcpy(m_sceneName, name);
}

// Returns the name of the chosen to play scene.

char* CMainDialog::GetSceneName()
{
    return m_sceneName;
}

// Specifies the rank of the chosen to play scene.

void CMainDialog::SetSceneRank(int rank)
{
    m_sceneRank = rank;
}

// Returns the rank of the chosen to play scene.

int CMainDialog::GetSceneRank()
{
    return m_sceneRank;
}

// Returns folder name of the scene that user selected to play.

const char* CMainDialog::GetSceneDir()
{
    int     i;

    i = (m_sceneRank/100)-1;

    if ( i < 0 || i >= m_userTotal )  return 0;
    return m_userList[i].c_str();
}

// Whether to show the solution.

bool CMainDialog::GetSceneSoluce()
{
    return m_bSceneSoluce;
}

// Returns the name of the folder to save.

std::string & CMainDialog::GetSavegameDir()
{
    return m_savegameDir;
}

// Returns the name of public folder.

std::string & CMainDialog::GetPublicDir()
{
    return m_publicDir;
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

// IWhether to make an animation in CTaskReset.

bool CMainDialog::GetNiceReset()
{
    return m_bNiceReset;
}

// Indicates whether the fire causes damage to its own units.

bool CMainDialog::GetHimselfDamage()
{
    return m_bHimselfDamage;
}



// Saves the personalized player.

void CMainDialog::WriteGamerPerso(char *gamer)
{
    FILE*   file;
    char    filename[100];
    char    line[100];

    sprintf(filename, "%s/%s/face.gam", m_savegameDir.c_str(), gamer);
    file = fopen(filename, "w");
    if ( file == NULL )  return;

    sprintf(line, "Head face=%d glasses=%d hair=%.2f;%.2f;%.2f;%.2f\n",
                m_perso.face, m_perso.glasses,
                m_perso.colorHair.r, m_perso.colorHair.g, m_perso.colorHair.b, m_perso.colorHair.a);
    fputs(line, file);

    sprintf(line, "Body combi=%.2f;%.2f;%.2f;%.2f band=%.2f;%.2f;%.2f;%.2f\n",
                m_perso.colorCombi.r, m_perso.colorCombi.g, m_perso.colorCombi.b, m_perso.colorCombi.a,
                m_perso.colorBand.r, m_perso.colorBand.g, m_perso.colorBand.b, m_perso.colorBand.a);
    fputs(line, file);

    fclose(file);
}

// Reads the personalized player.

void CMainDialog::ReadGamerPerso(char *gamer)
{
    FILE*           file;
    char            filename[100];
    char            line[100];
    Gfx::Color   color;

    m_perso.face = 0;
    DefPerso();

    sprintf(filename, "%s/%s/face.gam", m_savegameDir.c_str(), gamer);
    file = fopen(filename, "r");
    if ( file == NULL )  return;

    while ( fgets(line, 100, file) != NULL )
    {
        if ( Cmd(line, "Head") )
        {
            m_perso.face = OpInt(line, "face", 0);
            m_perso.glasses = OpInt(line, "glasses", 0);

            color.r = 0.0f;
            color.g = 0.0f;
            color.b = 0.0f;
            color.a = 0.0f;
            m_perso.colorHair = OpColor(line, "hair", color);
        }

        if ( Cmd(line, "Body") )
        {
            color.r = 0.0f;
            color.g = 0.0f;
            color.b = 0.0f;
            color.a = 0.0f;
            m_perso.colorCombi = OpColor(line, "combi", color);

            color.r = 0.0f;
            color.g = 0.0f;
            color.b = 0.0f;
            color.a = 0.0f;
            m_perso.colorBand = OpColor(line, "band", color);
        }
    }

    fclose(file);
}

// Specifies the face of the player.

void CMainDialog::SetGamerFace(char *gamer, int face)
{
    m_perso.face = face;
    WriteGamerPerso(gamer);
}

// Gives the face of the player.

int CMainDialog::GetGamerFace(char *gamer)
{
    ReadGamerPerso(gamer);
    return m_perso.face;
}

// Gives the face of the player.

int CMainDialog::GetGamerFace()
{
    return m_perso.face;
}

int CMainDialog::GetGamerGlasses()
{
    return m_perso.glasses;
}

bool CMainDialog::GetGamerOnlyHead()
{
    return (m_phase == PHASE_PERSO && m_persoTab == 0);
}

float CMainDialog::GetPersoAngle()
{
    return m_persoAngle;
}

Gfx::Color CMainDialog::GetGamerColorHair()
{
    return m_perso.colorHair;
}

Gfx::Color CMainDialog::GetGamerColorCombi()
{
    return m_perso.colorCombi;
}

Gfx::Color CMainDialog::GetGamerColorBand()
{
    return m_perso.colorBand;
}


// Reads the file of the player.

bool CMainDialog::ReadGamerInfo()
{
    FILE*   file;
    char    line[100];
    int     chap, i, numTry, passed;

    for ( i=0 ; i<MAXSCENE ; i++ )
    {
        m_sceneInfo[i].numTry = 0;
        m_sceneInfo[i].bPassed = false;
    }

    sprintf(line, "%s/%s/%s.gam", m_savegameDir.c_str(), m_main->GetGamerName(), m_sceneName);
    file = fopen(line, "r");
    if ( file == NULL )  return false;

    if ( fgets(line, 100, file) != NULL )
    {
        sscanf(line, "CurrentChapter=%d CurrentSel=%d\n", &chap, &i);
        m_chap[m_index] = chap-1;
        m_sel[m_index]  = i-1;
    }

    while ( fgets(line, 100, file) != NULL )
    {
        sscanf(line, "Chapter %d: Scene %d: numTry=%d passed=%d\n",
                &chap, &i, &numTry, &passed);

        i += chap*100;
        if ( i >= 0 && i < MAXSCENE )
        {
            m_sceneInfo[i].numTry  = numTry;
            m_sceneInfo[i].bPassed = passed;
        }
    }

    fclose(file);
    return true;
}

// Writes the file of the player.

bool CMainDialog::WriteGamerInfo()
{
    FILE*   file;
    char    line[100];
    int     i;

    sprintf(line, "%s/%s/%s.gam", m_savegameDir.c_str(), m_main->GetGamerName(), m_sceneName);
    file = fopen(line, "w");
    if ( file == NULL )  return false;

    sprintf(line, "CurrentChapter=%d CurrentSel=%d\n",
            m_chap[m_index]+1, m_sel[m_index]+1);
    fputs(line, file);

    for ( i=0 ; i<MAXSCENE ; i++ )
    {
        if ( m_sceneInfo[i].numTry == 0 )  continue;

        sprintf(line, "Chapter %d: Scene %d: numTry=%d passed=%d\n",
                i/100, i%100, m_sceneInfo[i].numTry, m_sceneInfo[i].bPassed);
        fputs(line, file);
    }

    fclose(file);
    return true;
}

void CMainDialog::SetGamerInfoTry(int rank, int numTry)
{
    if ( rank < 0 || rank >= MAXSCENE )  return;
    if ( numTry > 100 )  numTry = 100;
    m_sceneInfo[rank].numTry = numTry;
}

int CMainDialog::GetGamerInfoTry(int rank)
{
    if ( rank < 0 || rank >= MAXSCENE )  return 0;
    return m_sceneInfo[rank].numTry;
}

void CMainDialog::SetGamerInfoPassed(int rank, bool bPassed)
{
    int     chap, i;
    bool    bAll;

    if ( rank < 0 || rank >= MAXSCENE )  return;
    m_sceneInfo[rank].bPassed = bPassed;

    if ( bPassed )
    {
        bAll = true;
        chap = rank/100;
        for ( i=0 ; i<m_maxList ; i++ )
        {
            bAll &= m_sceneInfo[chap*100+i+1].bPassed;
        }
        m_sceneInfo[chap*100].numTry ++;
        m_sceneInfo[chap*100].bPassed = bAll;
    }
}

bool CMainDialog::GetGamerInfoPassed(int rank)
{
    if ( rank < 0 || rank >= MAXSCENE )  return false;
    return m_sceneInfo[rank].bPassed;
}


// Passes to the next mission, and possibly in the next chapter.

bool CMainDialog::NextMission()
{
    m_sel[m_index] ++;  // next mission

    if ( m_sel[m_index] >= m_maxList )  // last mission of the chapter?
    {
        m_chap[m_index] ++;  // next chapter
        m_sel[m_index] = 0;  // first mission
    }

    return true;
}


} // namespace Ui
