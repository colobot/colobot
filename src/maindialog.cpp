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

#define STRICT
#define D3D_OVERLOADS

#include <windows.h>
#include <stdio.h>
#include <time.h>
#include <direct.h>
#include <io.h>
#include <d3d.h>

#include "struct.h"
#include "d3dengine.h"
#include "d3dmath.h"
#include "global.h"
#include "language.h"
#include "event.h"
#include "misc.h"
#include "profile.h"
#include "iman.h"
#include "restext.h"
#include "math3d.h"
#include "particule.h"
#include "interface.h"
#include "button.h"
#include "color.h"
#include "check.h"
#include "key.h"
#include "group.h"
#include "image.h"
#include "scroll.h"
#include "slider.h"
#include "list.h"
#include "label.h"
#include "window.h"
#include "edit.h"
#include "editvalue.h"
#include "text.h"
#include "camera.h"
#include "sound.h"
#include "cmdtoken.h"
#include "robotmain.h"
#include "maindialog.h"



#define KEY_VISIBLE     6       // number of visible keys redefinable

#if _SCHOOL & _TEEN
#define KEY_TOTAL       13      // total number of keys redefinable
#else
#define KEY_TOTAL       21      // total number of keys redefinable
#endif

#define WELCOME_LENGTH  6.0f



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


#if _NET
// Check if the key "school" is present in the registry.

BOOL SchoolCheck()
{
    HKEY    key;
    char    buffer[100];
    LONG    i;
    DWORD   type, len;

    i = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
#if _NEWLOOK
                     "Software\\Epsitec\\CeeBot\\Setup",
#else
                     "Software\\Epsitec\\Colobot\\Setup",
#endif
                     0, KEY_READ, &key);
    if ( i != ERROR_SUCCESS )  return FALSE;

    type = REG_SZ;
    len  = sizeof(buffer);
    i = RegQueryValueEx(key, "School", NULL, &type, (LPBYTE)buffer, &len);
    if ( i != ERROR_SUCCESS || type != REG_SZ )  return FALSE;

    if ( strcmp(buffer, "ToBoLoC") != 0 )  return FALSE;

    return TRUE;
}
#endif


// Constructor of robot application.

CMainDialog::CMainDialog(CInstanceManager* iMan)
{
    int     i;

    m_iMan = iMan;
    m_iMan->AddInstance(CLASS_DIALOG, this);

    m_main      = (CRobotMain*)m_iMan->SearchInstance(CLASS_MAIN);
    m_interface = (CInterface*)m_iMan->SearchInstance(CLASS_INTERFACE);
    m_event     = (CEvent*)m_iMan->SearchInstance(CLASS_EVENT);
    m_engine    = (CD3DEngine*)m_iMan->SearchInstance(CLASS_ENGINE);
    m_particule = (CParticule*)m_iMan->SearchInstance(CLASS_PARTICULE);
    m_camera    = (CCamera*)m_iMan->SearchInstance(CLASS_CAMERA);
    m_sound     = (CSound*)m_iMan->SearchInstance(CLASS_SOUND);

    m_phase        = PHASE_NAME;
    m_phaseSetup   = PHASE_SETUPg;
    m_phaseTerm    = PHASE_TRAINER;
    m_sceneRead[0] = 0;
    m_stackRead[0] = 0;
    m_sceneName[0] = 0;
    m_sceneRank    = 0;
    m_bSceneSoluce = FALSE;
    m_bSimulSetup  = FALSE;
#if _NET
    m_accessEnable = SchoolCheck();
    m_accessMission= FALSE;
    m_accessUser   = FALSE;
#else
    m_accessEnable = TRUE;
    m_accessMission= TRUE;
    m_accessUser   = TRUE;
#endif
    m_bDeleteGamer = TRUE;

    for ( i=0 ; i<10 ; i++ )
    {
        m_chap[i] = 0;
        m_sel[i] = 0;
    }
    m_index = 0;
    m_maxList = 0;

    ZeroMemory(&m_perso, sizeof(GamerPerso));
    DefPerso();

    m_bTooltip       = TRUE;
    m_bGlint         = TRUE;
    m_bRain          = TRUE;
    m_bSoluce4       = TRUE;
    m_bMovies        = TRUE;
    m_bNiceReset     = TRUE;
    m_bHimselfDamage = TRUE;
#if _TEEN
    m_bCameraScroll  = FALSE;
#else
    m_bCameraScroll  = TRUE;
#endif
    m_bCameraInvertX = FALSE;
    m_bCameraInvertY = FALSE;
    m_bEffect        = TRUE;
    m_shotDelay      = 0;

    m_glintMouse = FPOINT(0.0f, 0.0f);
    m_glintTime  = 1000.0f;

    for ( i=0 ; i<10 ; i++ )
    {
        m_partiPhase[i] = 0;
        m_partiTime[i]  = 0.0f;
    }

    strcpy(m_sceneDir,    "scene");
    strcpy(m_savegameDir, "savegame");
    strcpy(m_publicDir,   "program");
    strcpy(m_userDir,     "user");
    strcpy(m_filesDir,    "files");

    m_bDialog = FALSE;
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
    FPOINT          pos, dim, ddim;
    float           ox, oy, sx, sy;
    char            name[100];
    char*           gamer;
    int             res, i, j;

    m_camera->SetType(CAMERA_DIALOG);
    m_engine->SetOverFront(FALSE);
    m_engine->SetOverColor(RetColor(0.0f), D3DSTATETCb);

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
#if _TEEN
        pw = m_interface->CreateWindows(pos, ddim, 12, EVENT_WINDOW5);
#else
        pw = m_interface->CreateWindows(pos, ddim, 10, EVENT_WINDOW5);
#endif
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

#if _SCHOOL
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
#else
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
#endif

#if _SCHOOL
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
#else
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
#endif

        if ( m_engine->RetSetupMode() )
        {
            pos.y = oy+sy*5.1f;
            pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_SETUP);
            pb->SetState(STATE_SHADOW);
        }

        pos.y = oy+sy*4.0f;
        pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_NAME);
        pb->SetState(STATE_SHADOW);

        pos.y = oy+sy*2.0f;
        pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_QUIT);
        pb->SetState(STATE_SHADOW);

#if !_DEMO & !_SCHOOL
        if ( m_accessEnable && m_accessUser )
        {
            pos.x  = 447.0f/640.0f;
            pos.y  = 313.0f/480.0f;
            ddim.x = 0.09f;
#if _POLISH
            pos.x  -=  5.0f/640.0f;
            ddim.x += 10.0f/640.0f;
#endif
            pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_USER);
            pb->SetState(STATE_SHADOW);
        }
#endif

        if ( m_engine->RetDebugMode() )
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
#if _WG
        pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL1, " ");
#else
#if _NEWLOOK
        pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL1, "www.epsitec.ch");
#else
        pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL1, "www.ceebot.com");
#endif
#endif
        pl->SetFontType(FONT_COURIER);
        pl->SetFontSize(8.0f);

        m_engine->SetBackground("inter01.tga", 0,0, 0,0, TRUE, TRUE);
        m_engine->SetBackForce(TRUE);
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

#if _NEWLOOK
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
#endif

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
        pl->SetJustif(-1);

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
        gamer = m_main->RetGamerName();
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
        pe->SetFocus(TRUE);

        pos.x = 380.0f/640.0f;
        pos.y = 320.0f/480.0f;
        ddim.x =100.0f/640.0f;
        ddim.y = 32.0f/480.0f;
        pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_NOK);
        pb->SetState(STATE_SHADOW);

#if !_TEEN
        pos.x = 380.0f/640.0f;
        pos.y = 250.0f/480.0f;
        ddim.x =100.0f/640.0f;
        ddim.y = 52.0f/480.0f;
        pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_PERSO);
        pb->SetState(STATE_SHADOW);
#endif

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

        m_engine->SetBackground("inter01.tga", 0,0, 0,0, TRUE, TRUE);
        m_engine->SetBackForce(TRUE);
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

#if _NEWLOOK
        pos.x  =  95.0f/640.0f;
        pos.y  =  66.0f/480.0f;
        ddim.x = 443.0f/640.0f;
        ddim.y =  42.0f/480.0f;
        pg = pw->CreateGroup(pos, ddim, 26, EVENT_LABEL1);  // violet
        pg->SetState(STATE_SHADOW);
#endif

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
        pl->SetJustif(1);

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
        pl->SetJustif(1);

        pos.x  = 340.0f/640.0f;
        pos.y  = 240.0f/480.0f;
        ddim.x =  30.0f/640.0f;
        ddim.y =  30.0f/480.0f;
        for ( i=0 ; i<6 ; i++ )
        {
            int ti[6] = {11, 179, 180, 181, 182, 183};
            pb = pw->CreateButton(pos, ddim, ti[i], (EventMsg)(EVENT_INTERFACE_PGLASS0+i));
            pb->SetState(STATE_SHADOW);
            pos.x += (30.0f+2.8f)/640.0f;
        }

        // Color A
        pos.x  = 340.0f/640.0f;
        pos.y  = 300.0f/480.0f;
        ddim.x = 200.0f/640.0f;
        ddim.y =  16.0f/480.0f;
        pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL14, "");
        pl->SetJustif(1);

        pos.y  = 282.0f/480.0f;
        ddim.x =  18.0f/640.0f;
        ddim.y =  18.0f/480.0f;
        for ( j=0 ; j<3 ; j++ )
        {
            pos.x  = 340.0f/640.0f;
            for ( i=0 ; i<3 ; i++ )
            {
                pco = pw->CreateColor(pos, ddim, -1, (EventMsg)(EVENT_INTERFACE_PC0a+j*3+i));
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
            psl = pw->CreateSlider(pos, ddim, 0, (EventMsg)(EVENT_INTERFACE_PCRa+i));
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
        pl->SetJustif(1);

        pos.y  = 174.0f/480.0f;
        ddim.x =  18.0f/640.0f;
        ddim.y =  18.0f/480.0f;
        for ( j=0 ; j<3 ; j++ )
        {
            pos.x  = 340.0f/640.0f;
            for ( i=0 ; i<3 ; i++ )
            {
                pco = pw->CreateColor(pos, ddim, -1, (EventMsg)(EVENT_INTERFACE_PC0b+j*3+i));
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
            psl = pw->CreateSlider(pos, ddim, 0, (EventMsg)(EVENT_INTERFACE_PCRb+i));
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
        pb->SetRepeat(TRUE);

        pos.x  = 290.0f/640.0f;
        pos.y  = 113.0f/480.0f;
        ddim.x =  20.0f/640.0f;
        ddim.y =  20.0f/480.0f;
        pb = pw->CreateButton(pos, ddim, 48, EVENT_INTERFACE_PRROT);  // >
        pb->SetState(STATE_SHADOW);
        pb->SetRepeat(TRUE);

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
            m_accessChap = RetChapPassed();
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
        pw->SetClosable(TRUE);
        if ( m_phase == PHASE_TRAINER )  res = RT_TITLE_TRAINER;
        if ( m_phase == PHASE_DEFI    )  res = RT_TITLE_DEFI;
        if ( m_phase == PHASE_MISSION )  res = RT_TITLE_MISSION;
        if ( m_phase == PHASE_FREE    )  res = RT_TITLE_FREE;
        if ( m_phase == PHASE_TEEN    )  res = RT_TITLE_TEEN;
        if ( m_phase == PHASE_USER    )  res = RT_TITLE_USER;
        if ( m_phase == PHASE_PROTO   )  res = RT_TITLE_PROTO;
        GetResource(RES_TEXT, res, name);
        pw->SetName(name);

#if _NEWLOOK
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
#endif

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
        pl->SetJustif(1);

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
        pl->SetJustif(1);

        pos.y = oy+sy*6.7f;
        ddim.y = dim.y*4.5f;
        ddim.x = dim.x*6.5f;
        pli = pw->CreateList(pos, ddim, 0, EVENT_INTERFACE_LIST);
        pli->SetState(STATE_SHADOW);
        UpdateSceneList(m_chap[m_index], m_sel[m_index]);
        if ( m_phase != PHASE_USER )  pli->SetState(STATE_EXTEND);
        pos = pli->RetPos();
        ddim = pli->RetDim();

        // Displays the summary:
        pos.x = ox+sx*3;
        pos.y = oy+sy*5.4f;
        ddim.x = dim.x*6.5f;
        ddim.y = dim.y*0.6f;
        GetResource(RES_TEXT, RT_PLAY_RESUME, name);
        pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL13, name);
        pl->SetJustif(1);

        pos.x = ox+sx*3;
        pos.y = oy+sy*3.6f;
        ddim.x = dim.x*13.4f;
        ddim.y = dim.y*1.9f;
        pe = pw->CreateEdit(pos, ddim, 0, EVENT_INTERFACE_RESUME);
        pe->SetState(STATE_SHADOW);
        pe->SetMaxChar(500);
        pe->SetEditCap(FALSE);  // just to see
        pe->SetHiliteCap(FALSE);

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
        m_bSceneSoluce = FALSE;

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

        m_engine->SetBackground("inter01.tga", 0,0, 0,0, TRUE, TRUE);
        m_engine->SetBackForce(TRUE);
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
            m_bSimulSetup = TRUE;
        }
        else if ( m_phase == PHASE_SETUPgs )
        {
            m_phaseSetup = PHASE_SETUPg;
            m_bSimulSetup = TRUE;
        }
        else if ( m_phase == PHASE_SETUPps )
        {
            m_phaseSetup = PHASE_SETUPp;
            m_bSimulSetup = TRUE;
        }
        else if ( m_phase == PHASE_SETUPcs )
        {
            m_phaseSetup = PHASE_SETUPc;
            m_bSimulSetup = TRUE;
        }
        else if ( m_phase == PHASE_SETUPss )
        {
            m_phaseSetup = PHASE_SETUPs;
            m_bSimulSetup = TRUE;
        }
        else
        {
            m_phaseSetup = m_phase;
            m_bSimulSetup = FALSE;
        }

        pos.x = 0.10f;
        pos.y = 0.10f;
        ddim.x = 0.80f;
        ddim.y = 0.80f;
        pw = m_interface->CreateWindows(pos, ddim, 12, EVENT_WINDOW5);
        pw->SetClosable(TRUE);
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

#if _NEWLOOK
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
#endif

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
            m_engine->SetBackground("inter01.tga", 0,0, 0,0, TRUE, TRUE);
            m_engine->SetBackForce(TRUE);
        }
    }

    if ( m_phase == PHASE_SETUPd  || // setup/display ?
         m_phase == PHASE_SETUPds )
    {
        pos.x = ox+sx*3;
        pos.y = oy+sy*9;
        ddim.x = dim.x*6;
        ddim.y = dim.y*1;
        GetResource(RES_TEXT, RT_SETUP_DEVICE, name);
        pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL1, name);
        pl->SetJustif(1);

        pos.x = ox+sx*3;
        pos.y = oy+sy*5.2f;
        ddim.x = dim.x*6;
        ddim.y = dim.y*4.5f;
        pli = pw->CreateList(pos, ddim, 0, EVENT_LIST1);
        pli->SetState(STATE_SHADOW);
        UpdateDisplayDevice();

        pos.x = ox+sx*10;
        pos.y = oy+sy*9;
        ddim.x = dim.x*6;
        ddim.y = dim.y*1;
        GetResource(RES_TEXT, RT_SETUP_MODE, name);
        pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL2, name);
        pl->SetJustif(1);

        m_setupFull = m_engine->RetFullScreen();
        pos.x = ox+sx*10;
        pos.y = oy+sy*5.2f;
        ddim.x = dim.x*6;
        ddim.y = dim.y*4.5f;
        pli = pw->CreateList(pos, ddim, 0, EVENT_LIST2);
        pli->SetState(STATE_SHADOW);
        UpdateDisplayMode();
        pli->SetState(STATE_ENABLE, m_setupFull);

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
            if ( m_engine->IsVideo8MB() )  pc->ClearState(STATE_ENABLE);
        }
        pos.y -= 0.048f;
        pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_DIRTY);
        pc->SetState(STATE_SHADOW);
        pos.y -= 0.048f;
        pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_SKY);
        pc->SetState(STATE_SHADOW);
        if ( m_engine->IsVideo8MB() )  pc->ClearState(STATE_ENABLE);
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
        pl->SetJustif(1);

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
        pl->SetJustif(1);

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
        pl->SetJustif(1);

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
            pl->SetJustif(1);
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
            pl->SetJustif(1);
        }
#endif

        ddim.x = dim.x*2;
        ddim.y = dim.y*1;
        pos.x = ox+sx*10;
        pos.y = oy+sy*2;
#if _POLISH
        ddim.x += 20.0f/640.0f;
        pos.x -= 20.0f/640.0f*3.0f;
#endif
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
#if _SCHOOL
 #if _EDU
        pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_SOLUCE4);
        pc->SetState(STATE_SHADOW);
        pos.y -= 0.048f;
 #endif
#else
        pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_MOVIES);
        pc->SetState(STATE_SHADOW);
        pos.y -= 0.048f;
#endif
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
        pl->SetJustif(1);

        pos.x = ox+sx*3;
        pos.y = 302.0f/480.0f;
        ddim.x = dim.x*15.0f;
        ddim.y = 18.0f/480.0f;
        GetResource(RES_TEXT, RT_SETUP_KEY2, name);
        pl = pw->CreateLabel(pos, ddim, 0, EVENT_INTERFACE_KINFO2, name);
        pl->SetJustif(1);

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
        ps->SetVisibleRatio((float)KEY_VISIBLE/KEY_TOTAL);
        ps->SetArrowStep(1.0f/((float)KEY_TOTAL-KEY_VISIBLE));
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
        pl->SetJustif(1);

#if (_FULL | _NET) & _SOUNDTRACKS
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
        pl->SetJustif(1);
#endif

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
        pw->SetClosable(TRUE);
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

#if _NEWLOOK
        pos.x  = 100.0f/640.0f;
        pos.y  =  66.0f/480.0f;
        ddim.x = 438.0f/640.0f;
        ddim.y =  42.0f/480.0f;
        pg = pw->CreateGroup(pos, ddim, 26, EVENT_LABEL1);  // violet
        pg->SetState(STATE_SHADOW);
#endif

        pos.x  = 290.0f/640.0f;
        ddim.x = 245.0f/640.0f;

        pos.y  = 146.0f/480.0f;
        ddim.y =  18.0f/480.0f;
        GetResource(RES_EVENT, EVENT_INTERFACE_IOLABEL, name);
        pl = pw->CreateLabel(pos, ddim, 0, EVENT_INTERFACE_IOLABEL, name);
        pl->SetJustif(1);

        pos.y  = 130.0f/480.0f;
        ddim.y =  18.0f/480.0f;
        pe = pw->CreateEdit(pos, ddim, 0, EVENT_INTERFACE_IONAME);
        pe->SetState(STATE_SHADOW);
        pe->SetFontType(FONT_COLOBOT);
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
        pw->SetClosable(TRUE);
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

#if _NEWLOOK
        pos.x  = 100.0f/640.0f;
        pos.y  =  66.0f/480.0f;
        ddim.x = 438.0f/640.0f;
        ddim.y =  42.0f/480.0f;
        pg = pw->CreateGroup(pos, ddim, 26, EVENT_LABEL1);  // violet
        pg->SetState(STATE_SHADOW);
#endif

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
            m_engine->SetBackground("inter01.tga", 0,0, 0,0, TRUE, TRUE);
            m_engine->SetBackForce(TRUE);
        }
    }

    if ( m_phase == PHASE_LOADING )
    {
        pos.x  = 0.35f;
        pos.y  = 0.10f;
        ddim.x = 0.30f;
        ddim.y = 0.80f;
#if _TEEN
        pw = m_interface->CreateWindows(pos, ddim, 12, EVENT_WINDOW5);
#else
        pw = m_interface->CreateWindows(pos, ddim, 10, EVENT_WINDOW5);
#endif
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
        pl->SetJustif(0);

        m_engine->SetBackground("inter01.tga", 0,0, 0,0, TRUE, TRUE);
        m_engine->SetBackForce(TRUE);

        m_loadingCounter = 1;  // enough time to display!
    }

    if ( m_phase == PHASE_WELCOME1 )
    {
        m_sound->StopMusic();
        m_sound->PlayMusic(11, FALSE);

        pos.x  = 0.0f;
        pos.y  = 0.0f;
        ddim.x = 0.0f;
        ddim.y = 0.0f;
        pw = m_interface->CreateWindows(pos, ddim, -1, EVENT_WINDOW5);

        m_engine->SetOverColor(RetColor(1.0f), D3DSTATETCb);
        m_engine->SetOverFront(TRUE);

#if _FRENCH
        m_engine->SetBackground("alsyd.tga", 0,0, 0,0, TRUE, FALSE);
#endif
#if _POLISH
        m_engine->SetBackground("manta.tga", 0,0, 0,0, TRUE, FALSE);
#endif
#if _WG
        m_engine->SetBackground("wg.tga", 0,0, 0,0, TRUE, FALSE);
#endif
        m_engine->SetBackForce(TRUE);
    }
    if ( m_phase == PHASE_WELCOME2 )
    {
#if _ENGLISH
        m_sound->StopMusic();
        m_sound->PlayMusic(11, FALSE);
#endif
#if _POLISH
        m_sound->StopMusic();
        m_sound->PlayMusic(11, FALSE);
#endif

        pos.x  = 0.0f;
        pos.y  = 0.0f;
        ddim.x = 0.0f;
        ddim.y = 0.0f;
        pw = m_interface->CreateWindows(pos, ddim, -1, EVENT_WINDOW5);

        m_engine->SetOverColor(RetColor(1.0f), D3DSTATETCb);
        m_engine->SetOverFront(TRUE);

        m_engine->SetBackground("colobot.tga", 0,0, 0,0, TRUE, FALSE);
        m_engine->SetBackForce(TRUE);
    }
    if ( m_phase == PHASE_WELCOME3 )
    {
        pos.x  = 0.0f;
        pos.y  = 0.0f;
        ddim.x = 0.0f;
        ddim.y = 0.0f;
        pw = m_interface->CreateWindows(pos, ddim, -1, EVENT_WINDOW5);

        m_engine->SetOverColor(RetColor(0.0f), D3DSTATETCw);
        m_engine->SetOverFront(TRUE);

#if _FRENCH
        m_engine->SetBackground("epsitecf.tga", 0,0, 0,0, TRUE, FALSE);
#endif
#if _ENGLISH
        m_engine->SetBackground("epsitece.tga", 0,0, 0,0, TRUE, FALSE);
#endif
#if _GERMAN | _WG
        m_engine->SetBackground("epsitecd.tga", 0,0, 0,0, TRUE, FALSE);
#endif
#if _POLISH
        m_engine->SetBackground("epsitecp.tga", 0,0, 0,0, TRUE, FALSE);
#endif
        m_engine->SetBackForce(TRUE);
    }

    if ( m_phase == PHASE_GENERIC )
    {
        pos.x  = 0.0f;
        pos.y  = 0.0f;
        ddim.x = 0.0f;
        ddim.y = 0.0f;
        pw = m_interface->CreateWindows(pos, ddim, -1, EVENT_WINDOW5);

#if _FULL | _NET
        pos.x  =  80.0f/640.0f;
        pos.y  = 240.0f/480.0f;
        ddim.x = 490.0f/640.0f;
        ddim.y = 110.0f/480.0f;
        pe = pw->CreateEdit(pos, ddim, 0, EVENT_EDIT1);
        pe->SetGenericMode(TRUE);
        pe->SetEditCap(FALSE);
        pe->SetHiliteCap(FALSE);
        pe->SetFontType(FONT_COURIER);
        pe->SetFontSize(8.0f);
        pe->ReadText("help\\authors.txt");

        pos.x  =  80.0f/640.0f;
        pos.y  = 140.0f/480.0f;
        ddim.x = 490.0f/640.0f;
        ddim.y = 100.0f/480.0f;
        pe = pw->CreateEdit(pos, ddim, 0, EVENT_EDIT2);
        pe->SetGenericMode(TRUE);
        pe->SetEditCap(FALSE);
        pe->SetHiliteCap(FALSE);
        pe->SetFontType(FONT_COURIER);
        pe->SetFontSize(6.5f);
        pe->ReadText("help\\licences.txt");
#endif
#if _SCHOOL
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
#endif
        pe = pw->CreateEdit(pos, ddim, 0, EVENT_EDIT1);
        pe->SetGenericMode(TRUE);
        pe->SetEditCap(FALSE);
        pe->SetHiliteCap(FALSE);
        pe->SetFontType(FONT_COURIER);
        pe->SetFontSize(8.0f);
        pe->ReadText("help\\authors.txt");
#endif
#if _DEMO
//?     pos.x  =  80.0f/640.0f;
//?     pos.y  = 240.0f/480.0f;
//?     ddim.x = 490.0f/640.0f;
//?     ddim.y = 110.0f/480.0f;
//?     pe = pw->CreateEdit(pos, ddim, 0, EVENT_EDIT1);
//?     pe->SetGenericMode(TRUE);
//?     pe->SetEditCap(FALSE);
//?     pe->SetHiliteCap(FALSE);
//?     pe->SetFontType(FONT_COURIER);
//?     pe->SetFontSize(8.0f);
//?     pe->ReadText("help\\demo.txt");

//?     pos.x  =  80.0f/640.0f;
//?     pos.y  = 140.0f/480.0f;
//?     ddim.x = 490.0f/640.0f;
//?     ddim.y = 100.0f/480.0f;
//?     pe = pw->CreateEdit(pos, ddim, 0, EVENT_EDIT2);
//?     pe->SetGenericMode(TRUE);
//?     pe->SetEditCap(FALSE);
//?     pe->SetHiliteCap(FALSE);
//?     pe->SetFontType(FONT_COURIER);
//?     pe->SetFontSize(8.0f);
//?     pe->ReadText("help\\authors.txt");
#endif

#if !_DEMO
        pos.x  =  40.0f/640.0f;
        pos.y  =  83.0f/480.0f;
        ddim.x = 246.0f/640.0f;
        ddim.y =  16.0f/480.0f;
        GetResource(RES_TEXT, RT_GENERIC_DEV1, name);
        pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL1, name);
        pl->SetFontType(FONT_COURIER);
        pl->SetFontSize(8.0f);

        pos.y  =  13.0f/480.0f;
        GetResource(RES_TEXT, RT_GENERIC_DEV2, name);
        pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL2, name);
        pl->SetFontType(FONT_COURIER);
        pl->SetFontSize(8.0f);

        pos.x  = 355.0f/640.0f;
        pos.y  =  83.0f/480.0f;
        ddim.x = 246.0f/640.0f;
        ddim.y =  16.0f/480.0f;
        GetResource(RES_TEXT, RT_GENERIC_EDIT1, name);
        pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL3, name);
        pl->SetFontType(FONT_COURIER);
        pl->SetFontSize(8.0f);

        pos.y  =  13.0f/480.0f;
        GetResource(RES_TEXT, RT_GENERIC_EDIT2, name);
        pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL4, name);
        pl->SetFontType(FONT_COURIER);
        pl->SetFontSize(8.0f);
#endif

#if _DEMO
        pos.x  = 481.0f/640.0f;
        pos.y  =  51.0f/480.0f;
        ddim.x =  30.0f/640.0f;
        ddim.y =  30.0f/480.0f;
        pb = pw->CreateButton(pos, ddim, 49, EVENT_INTERFACE_ABORT);
        pb->SetState(STATE_SHADOW);
#else
        pos.x  = 306.0f/640.0f;
        pos.y  =  17.0f/480.0f;
        ddim.x =  30.0f/640.0f;
        ddim.y =  30.0f/480.0f;
        pb = pw->CreateButton(pos, ddim, 49, EVENT_INTERFACE_ABORT);
        pb->SetState(STATE_SHADOW);
#endif

#if _NEWLOOK
#if _CEEBOTDEMO
#if _TEEN
        m_engine->SetBackground("genedt.tga", 0,0, 0,0, TRUE, TRUE);
#else
        m_engine->SetBackground("geneda.tga", 0,0, 0,0, TRUE, TRUE);
#endif
#else
        m_engine->SetBackground("genern.tga", 0,0, 0,0, TRUE, TRUE);
#endif
#else
#if _FRENCH
#if _DEMO
        m_engine->SetBackground("genedf.tga", 0,0, 0,0, TRUE, TRUE);
#else
        m_engine->SetBackground("generf.tga", 0,0, 0,0, TRUE, TRUE);
#endif
#endif
#if _ENGLISH
#if _DEMO
        m_engine->SetBackground("genede.tga", 0,0, 0,0, TRUE, TRUE);
#else
        m_engine->SetBackground("genere.tga", 0,0, 0,0, TRUE, TRUE);
#endif
#endif
#if _GERMAN
#if _DEMO
        m_engine->SetBackground("genedd.tga", 0,0, 0,0, TRUE, TRUE);
#else
        m_engine->SetBackground("genere.tga", 0,0, 0,0, TRUE, TRUE);
#endif
#endif
#if _WG
#if _DEMO
        m_engine->SetBackground("genedd.tga", 0,0, 0,0, TRUE, TRUE);
#else
        m_engine->SetBackground("generd.tga", 0,0, 0,0, TRUE, TRUE);
#endif
#endif
#if _POLISH
#if _DEMO
        m_engine->SetBackground("genedp.tga", 0,0, 0,0, TRUE, TRUE);
#else
        m_engine->SetBackground("generp.tga", 0,0, 0,0, TRUE, TRUE);
#endif
#endif
#endif
        m_engine->SetBackForce(TRUE);
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
#if _SCHOOL
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
#else
        pos.x  = 540.0f/640.0f;
        pos.y  =   9.0f/480.0f;
        ddim.x =  90.0f/640.0f;
        ddim.y =  10.0f/480.0f;
#endif
        GetResource(RES_TEXT, RT_VERSION_ID, name);
        pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL1, name);
        pl->SetFontType(FONT_COURIER);
        pl->SetFontSize(9.0f);
    }

    m_engine->LoadAllTexture();
}


// Processing an event.
// Returns FALSE if the event has been processed completely.

BOOL CMainDialog::EventProcess(const Event &event)
{
    CWindow*    pw;
    CList*      pl;
    CButton*    pb;
    CCheck*     pc;
    Event       newEvent;
    float       welcomeLength;

    if ( event.event == EVENT_FRAME )
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
            int     mode = D3DSTATETCb;

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
                mode = D3DSTATETCw;
            }

            m_engine->SetOverColor(RetColor(intensity), mode);
        }

        if ( m_phase == PHASE_WELCOME1 && m_phaseTime >= welcomeLength )
        {
            ChangePhase(PHASE_WELCOME2);
            return TRUE;
        }
        if ( m_phase == PHASE_WELCOME2 && m_phaseTime >= welcomeLength )
        {
            ChangePhase(PHASE_WELCOME3);
            return TRUE;
        }
        if ( m_phase == PHASE_WELCOME3 && m_phaseTime >= welcomeLength )
        {
            ChangePhase(PHASE_NAME);
            return TRUE;
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
            return FALSE;
        }

        m_glintTime += event.rTime;
        GlintMove();  // moves reflections

        FrameParticule(event.rTime);

        if ( m_bDialog )  // this dialogue?
        {
            FrameDialog(event.rTime);
        }

        return TRUE;
    }

    if ( event.event == EVENT_MOUSEMOVE )
    {
        m_glintMouse = event.pos;
        NiceParticule(event.pos, event.keyState&KS_MLEFT);
    }

    if ( m_bDialog )  // this dialogue?
    {
        m_interface->EventProcess(event);

        if ( event.event == EVENT_DIALOG_OK ||
             (event.event == EVENT_KEYDOWN && event.param == VK_RETURN ) )
        {
            StopDialog();
            if ( m_phase == PHASE_NAME )
            {
                NameDelete();
            }
            if ( m_phase == PHASE_INIT )
            {
//?             m_event->MakeEvent(newEvent, EVENT_QUIT);
//?             m_event->AddEvent(newEvent);
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
        if ( event.event == EVENT_DIALOG_CANCEL ||
             (event.event == EVENT_KEYDOWN && event.param == VK_ESCAPE ) )
        {
            StopDialog();
        }
        if ( event.event == EVENT_INTERFACE_SETUP )
        {
            StopDialog();
            StartSuspend();
            if ( m_phaseSetup == PHASE_SETUPd )  ChangePhase(PHASE_SETUPds);
            if ( m_phaseSetup == PHASE_SETUPg )  ChangePhase(PHASE_SETUPgs);
            if ( m_phaseSetup == PHASE_SETUPp )  ChangePhase(PHASE_SETUPps);
            if ( m_phaseSetup == PHASE_SETUPc )  ChangePhase(PHASE_SETUPcs);
            if ( m_phaseSetup == PHASE_SETUPs )  ChangePhase(PHASE_SETUPss);
        }
        if ( event.event == EVENT_INTERFACE_AGAIN )
        {
            StopDialog();
            m_main->ChangePhase(PHASE_LOADING);
        }
        if ( event.event == EVENT_INTERFACE_WRITE )
        {
            StopDialog();
            StartSuspend();
            ChangePhase(PHASE_WRITEs);
        }
        if ( event.event == EVENT_INTERFACE_READ )
        {
            StopDialog();
            StartSuspend();
            ChangePhase(PHASE_READs);
        }

        return FALSE;
    }

    if ( !m_engine->RetMouseHide() &&
         !m_interface->EventProcess(event) )
    {
        return FALSE;
    }

    if ( m_phase == PHASE_INIT )
    {
        switch( event.event )
        {
            case EVENT_KEYDOWN:
                if ( event.param == VK_ESCAPE )
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
        }
        return FALSE;
    }

    if ( m_phase == PHASE_NAME )
    {
        switch( event.event )
        {
            case EVENT_KEYDOWN:
                if ( event.param == VK_RETURN )
                {
                    NameSelect();
                }
                if ( event.param == VK_ESCAPE )
                {
                    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW5);
                    if ( pw == 0 )  break;
                    pb = (CButton*)pw->SearchControl(EVENT_INTERFACE_NCANCEL);
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
                pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW5);
                if ( pw == 0 )  break;
                pl = (CList*)pw->SearchControl(EVENT_INTERFACE_NLIST);
                if ( pl == 0 )  break;
                StartDeleteGame(pl->RetName(pl->RetSelect()));
                break;
        }
    }

    if ( m_phase == PHASE_PERSO )
    {
        switch( event.event )
        {
            case EVENT_KEYDOWN:
                if ( event.param == VK_RETURN )
                {
                    m_main->ChangePhase(PHASE_INIT);
                }
                if ( event.param == VK_ESCAPE )
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
                m_perso.face = event.event-EVENT_INTERFACE_PFACE1;
                WriteGamerPerso(m_main->RetGamerName());
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
                m_perso.glasses = event.event-EVENT_INTERFACE_PGLASS0;
                WriteGamerPerso(m_main->RetGamerName());
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
                FixPerso(event.event-EVENT_INTERFACE_PC0a, 0);
                WriteGamerPerso(m_main->RetGamerName());
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
                FixPerso(event.event-EVENT_INTERFACE_PC0b, 1);
                WriteGamerPerso(m_main->RetGamerName());
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
                WriteGamerPerso(m_main->RetGamerName());
                UpdatePerso();
                m_main->ScenePerso();
                break;

            case EVENT_INTERFACE_PDEF:
                DefPerso();
                WriteGamerPerso(m_main->RetGamerName());
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
                WriteGamerPerso(m_main->RetGamerName());
                m_main->ChangePhase(PHASE_NAME);
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
        pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW5);
        if ( pw == 0 )  return FALSE;

        if ( event.event == pw->RetEventMsgClose() ||
             event.event == EVENT_INTERFACE_BACK   ||
            (event.event == EVENT_KEYDOWN && event.param == VK_ESCAPE) )
        {
            m_main->ChangePhase(PHASE_INIT);
            return FALSE;
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
        switch( event.event )
        {
            case EVENT_INTERFACE_CHAP:
                pl = (CList*)pw->SearchControl(EVENT_INTERFACE_CHAP);
                if ( pl == 0 )  break;
                m_chap[m_index] = pl->RetSelect();
                UpdateSceneList(m_chap[m_index], m_sel[m_index]);
                UpdateSceneResume((m_chap[m_index]+1)*100+(m_sel[m_index]+1));
                break;

            case EVENT_INTERFACE_LIST:
                pl = (CList*)pw->SearchControl(EVENT_INTERFACE_LIST);
                if ( pl == 0 )  break;
                m_sel[m_index] = pl->RetSelect();
                UpdateSceneResume((m_chap[m_index]+1)*100+(m_sel[m_index]+1));
                break;

            case EVENT_INTERFACE_SOLUCE:
                pb = (CButton*)pw->SearchControl(EVENT_INTERFACE_SOLUCE);
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
        }
        return FALSE;
    }

    if ( m_phase == PHASE_SETUPd ||
         m_phase == PHASE_SETUPg ||
         m_phase == PHASE_SETUPp ||
         m_phase == PHASE_SETUPc ||
         m_phase == PHASE_SETUPs )
    {
        pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW5);
        if ( pw == 0 )  return FALSE;

        if ( event.event == pw->RetEventMsgClose() ||
             event.event == EVENT_INTERFACE_BACK   ||
            (event.event == EVENT_KEYDOWN && event.param == VK_ESCAPE) )
        {
            SetupMemorize();
            m_engine->ApplyChange();
            m_main->ChangePhase(PHASE_INIT);
            return FALSE;
        }

        switch( event.event )
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
        }
    }

    if ( m_phase == PHASE_SETUPds ||
         m_phase == PHASE_SETUPgs ||
         m_phase == PHASE_SETUPps ||
         m_phase == PHASE_SETUPcs ||
         m_phase == PHASE_SETUPss )
    {
        pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW5);
        if ( pw == 0 )  return FALSE;

        if ( event.event == pw->RetEventMsgClose() ||
             event.event == EVENT_INTERFACE_BACK   ||
            (event.event == EVENT_KEYDOWN && event.param == VK_ESCAPE) )
        {
            SetupMemorize();
            m_engine->ApplyChange();
            m_interface->DeleteControl(EVENT_WINDOW5);
            ChangePhase(PHASE_SIMUL);
            StopSuspend();
            return FALSE;
        }

        switch( event.event )
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
        }
    }

    if ( m_phase == PHASE_SETUPd  ||  // setup/display ?
         m_phase == PHASE_SETUPds )
    {
        switch( event.event )
        {
            case EVENT_LIST1:
            case EVENT_LIST2:
                UpdateApply();
                break;

            case EVENT_INTERFACE_FULL:
                pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW5);
                if ( pw == 0 )  break;
                pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_FULL);
                if ( pc == 0 )  break;
                pl = (CList*)pw->SearchControl(EVENT_LIST2);
                if ( pl == 0 )  break;
                if ( pc->TestState(STATE_CHECK) )
                {
                    pc->ClearState(STATE_CHECK);  // window
                    pl->ClearState(STATE_ENABLE);
                }
                else
                {
                    pc->SetState(STATE_CHECK);  // fullscreen
                    pl->SetState(STATE_ENABLE);
                }
                UpdateApply();
                break;

            case EVENT_INTERFACE_APPLY:
                pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW5);
                if ( pw == 0 )  break;
                pb = (CButton*)pw->SearchControl(EVENT_INTERFACE_APPLY);
                if ( pb == 0 )  break;
                pb->ClearState(STATE_PRESS);
                pb->ClearState(STATE_HILIGHT);
                ChangeDisplay();
                UpdateApply();
                break;
        }
        return FALSE;
    }

    if ( m_phase == PHASE_SETUPg  ||  // setup/graphic ?
         m_phase == PHASE_SETUPgs )
    {
        switch( event.event )
        {
            case EVENT_INTERFACE_SHADOW:
                m_engine->SetShadow(!m_engine->RetShadow());
                ChangeSetupButtons();
                UpdateSetupButtons();
                break;

            case EVENT_INTERFACE_GROUND:
                m_engine->SetGroundSpot(!m_engine->RetGroundSpot());
                ChangeSetupButtons();
                UpdateSetupButtons();
                break;

            case EVENT_INTERFACE_DIRTY:
                m_engine->SetDirty(!m_engine->RetDirty());
                ChangeSetupButtons();
                UpdateSetupButtons();
                break;

            case EVENT_INTERFACE_FOG:
                m_engine->SetFog(!m_engine->RetFog());
                m_camera->SetOverBaseColor(RetColor(RetColor(0.0f)));
                ChangeSetupButtons();
                UpdateSetupButtons();
                break;

            case EVENT_INTERFACE_LENS:
                m_engine->SetLensMode(!m_engine->RetLensMode());
                ChangeSetupButtons();
                UpdateSetupButtons();
                break;

            case EVENT_INTERFACE_SKY:
                m_engine->SetSkyMode(!m_engine->RetSkyMode());
                ChangeSetupButtons();
                UpdateSetupButtons();
                break;

            case EVENT_INTERFACE_PLANET:
                m_engine->SetPlanetMode(!m_engine->RetPlanetMode());
                ChangeSetupButtons();
                UpdateSetupButtons();
                break;

            case EVENT_INTERFACE_LIGHT:
                m_engine->SetLightMode(!m_engine->RetLightMode());
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
        }
        return FALSE;
    }

    if ( m_phase == PHASE_SETUPp  ||  // setup/game ?
         m_phase == PHASE_SETUPps )
    {
        switch( event.event )
        {
            case EVENT_INTERFACE_TOTO:
                m_engine->SetTotoMode(!m_engine->RetTotoMode());
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
                m_engine->SetNiceMouse(!m_engine->RetNiceMouse());
                ChangeSetupButtons();
                UpdateSetupButtons();
                break;

            case EVENT_INTERFACE_EDITMODE:
                m_engine->SetEditIndentMode(!m_engine->RetEditIndentMode());
                ChangeSetupButtons();
                UpdateSetupButtons();
                break;

            case EVENT_INTERFACE_EDITVALUE:
                if ( m_engine->RetEditIndentValue() == 2 )
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
        }
        return FALSE;
    }

    if ( m_phase == PHASE_SETUPc  ||  // setup/commands ?
         m_phase == PHASE_SETUPcs )
    {
        switch( event.event )
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
                ChangeKey(event.event);
                UpdateKey();
                break;

            case EVENT_INTERFACE_KDEF:
                m_engine->ResetKey();
                UpdateKey();
                break;

            case EVENT_INTERFACE_JOYSTICK:
                m_engine->SetJoystick(!m_engine->RetJoystick());
                UpdateSetupButtons();
                break;
        }
        return FALSE;
    }

    if ( m_phase == PHASE_SETUPs  ||  // setup/sound ?
         m_phase == PHASE_SETUPss )
    {
        switch( event.event )
        {
            case EVENT_INTERFACE_VOLSOUND:
            case EVENT_INTERFACE_VOLMUSIC:
                ChangeSetupButtons();
                break;

            case EVENT_INTERFACE_SOUND3D:
                m_sound->SetSound3D(!m_sound->RetSound3D());
                ChangeSetupButtons();
                UpdateSetupButtons();
                break;

            case EVENT_INTERFACE_SILENT:
                m_sound->SetAudioVolume(0);
                m_sound->SetMidiVolume(0);
                UpdateSetupButtons();
                break;
            case EVENT_INTERFACE_NOISY:
                m_sound->SetAudioVolume(MAXVOLUME);
                m_sound->SetMidiVolume(MAXVOLUME*3/4);
                UpdateSetupButtons();
                break;
        }
        return FALSE;
    }

    if ( m_phase == PHASE_READ )
    {
        pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW5);
        if ( pw == 0 )  return FALSE;

        if ( event.event == pw->RetEventMsgClose() ||
             event.event == EVENT_INTERFACE_BACK   ||
            (event.event == EVENT_KEYDOWN && event.param == VK_ESCAPE) )
        {
            ChangePhase(m_phaseTerm);
        }

        if ( event.event == EVENT_INTERFACE_IOLIST )
        {
            IOUpdateList();
        }
        if ( event.event == EVENT_INTERFACE_IODELETE )
        {
            IODeleteScene();
            IOUpdateList();
        }
        if ( event.event == EVENT_INTERFACE_IOREAD )
        {
            if ( IOReadScene() )
            {
                m_main->ChangePhase(PHASE_LOADING);
            }
        }

        return FALSE;
    }

    if ( m_phase == PHASE_WRITEs ||
         m_phase == PHASE_READs  )
    {
        pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW5);
        if ( pw == 0 )  return FALSE;

        if ( event.event == pw->RetEventMsgClose() ||
             event.event == EVENT_INTERFACE_BACK   ||
            (event.event == EVENT_KEYDOWN && event.param == VK_ESCAPE) )
        {
            m_interface->DeleteControl(EVENT_WINDOW5);
            ChangePhase(PHASE_SIMUL);
            StopSuspend();
        }

        if ( event.event == EVENT_INTERFACE_IOLIST )
        {
            IOUpdateList();
        }
        if ( event.event == EVENT_INTERFACE_IODELETE )
        {
            IODeleteScene();
            IOUpdateList();
        }
        if ( event.event == EVENT_INTERFACE_IOWRITE )
        {
            IOWriteScene();
            m_interface->DeleteControl(EVENT_WINDOW5);
            ChangePhase(PHASE_SIMUL);
            StopSuspend();
        }
        if ( event.event == EVENT_INTERFACE_IOREAD )
        {
            if ( IOReadScene() )
            {
                m_interface->DeleteControl(EVENT_WINDOW5);
                ChangePhase(PHASE_SIMUL);
                StopSuspend();
                m_main->ChangePhase(PHASE_LOADING);
            }
        }

        return FALSE;
    }

    if ( m_phase == PHASE_WELCOME1 )
    {
        if ( event.event == EVENT_KEYDOWN     ||
             event.event == EVENT_LBUTTONDOWN ||
             event.event == EVENT_RBUTTONDOWN )
        {
            ChangePhase(PHASE_WELCOME2);
            return TRUE;
        }
    }
    if ( m_phase == PHASE_WELCOME2 )
    {
        if ( event.event == EVENT_KEYDOWN     ||
             event.event == EVENT_LBUTTONDOWN ||
             event.event == EVENT_RBUTTONDOWN )
        {
            ChangePhase(PHASE_WELCOME3);
            return TRUE;
        }
    }
    if ( m_phase == PHASE_WELCOME3 )
    {
        if ( event.event == EVENT_KEYDOWN     ||
             event.event == EVENT_LBUTTONDOWN ||
             event.event == EVENT_RBUTTONDOWN )
        {
            ChangePhase(PHASE_NAME);
            return TRUE;
        }
    }

    if ( m_phase == PHASE_GENERIC )
    {
        if ( event.event == EVENT_INTERFACE_ABORT )
        {
            ChangePhase(PHASE_INIT);
        }

        if ( event.event == EVENT_KEYDOWN )
        {
            if ( event.param == VK_ESCAPE )
            {
                ChangePhase(PHASE_INIT);
            }
            else
            {
                m_event->MakeEvent(newEvent, EVENT_QUIT);
                m_event->AddEvent(newEvent);
            }
        }

        if ( event.event == EVENT_LBUTTONDOWN ||
             event.event == EVENT_RBUTTONDOWN )
        {
            m_event->MakeEvent(newEvent, EVENT_QUIT);
            m_event->AddEvent(newEvent);
        }
    }

    return TRUE;
}


// Moves the reflections.

void CMainDialog::GlintMove()
{
    CWindow*    pw;
    CGroup*     pg;
    FPOINT      pos, dim, zoom;

    if ( m_phase == PHASE_SIMUL )  return;

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW5);
    if ( pw == 0 )  return;

    if ( m_phase == PHASE_INIT )
    {
        pg = (CGroup*)pw->SearchControl(EVENT_INTERFACE_GLINTl);
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

        pg = (CGroup*)pw->SearchControl(EVENT_INTERFACE_GLINTr);
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
        pg = (CGroup*)pw->SearchControl(EVENT_INTERFACE_GLINTl);
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

        pg = (CGroup*)pw->SearchControl(EVENT_INTERFACE_GLINTr);
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
        pg = (CGroup*)pw->SearchControl(EVENT_INTERFACE_GLINTu);
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

        pg = (CGroup*)pw->SearchControl(EVENT_INTERFACE_GLINTr);
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
        pg = (CGroup*)pw->SearchControl(EVENT_INTERFACE_GLINTl);
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

        pg = (CGroup*)pw->SearchControl(EVENT_INTERFACE_GLINTr);
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

D3DVECTOR SoundPos(FPOINT pos)
{
    D3DVECTOR   s;

    s.x = (pos.x-0.5f)*2.0f;
    s.y = (pos.y-0.5f)*2.0f;
    s.z = 0.0f;

    return s;
}

// Returns a random position for a sound.

D3DVECTOR SoundRand()
{
    D3DVECTOR   s;

    s.x = (Rand()-0.5f)*2.0f;
    s.y = (Rand()-0.5f)*2.0f;
    s.z = 0.0f;

    return s;
}

// Makes pretty qq particles evolve.

void CMainDialog::FrameParticule(float rTime)
{
#if _NEWLOOK
#else
    D3DVECTOR   pos, speed;
    FPOINT      dim;
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

    nParti = (int)(*pParti++);
    nGlint = (int)(*pGlint++);

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
                    m_partiTime[i] = pParti[ii*5+2]+Rand()*pParti[ii*5+3];
                    m_partiPhase[i] = (int)pParti[ii*5+4];
                    if ( m_partiPhase[i] == 3 )
                    {
                        m_sound->Play(SOUND_PSHHH, SoundPos(m_partiPos[i]), 0.3f+Rand()*0.3f);
                    }
                    else
                    {
                        m_sound->Play(SOUND_GGG, SoundPos(m_partiPos[i]), 0.1f+Rand()*0.4f);
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
                    dim.x = 0.04f+Rand()*0.04f;
                    dim.y = dim.x/0.75f;
                    m_particule->CreateParticule(pos, speed, dim,
                                                 rand()%2?PARTIGLINT:PARTICONTROL,
                                                 Rand()*0.4f+0.4f, 0.0f, 0.0f,
                                                 SH_INTERFACE);
                    m_partiTime[i] = 0.5f+Rand()*0.5f;
                }

                if ( r == 2 )
                {
                    ii = rand()%7;
                    if ( ii == 0 )
                    {
                        m_sound->Play(SOUND_ENERGY, SoundRand(), 0.2f+Rand()*0.2f);
                        m_partiTime[i] = 1.0f+Rand()*1.0f;
                    }
                    if ( ii == 1 )
                    {
                        m_sound->Play(SOUND_STATION, SoundRand(), 0.2f+Rand()*0.2f);
                        m_partiTime[i] = 1.0f+Rand()*2.0f;
                    }
                    if ( ii == 2 )
                    {
                        m_sound->Play(SOUND_ALARM, SoundRand(), 0.1f+Rand()*0.1f);
                        m_partiTime[i] = 2.0f+Rand()*4.0f;
                    }
                    if ( ii == 3 )
                    {
                        m_sound->Play(SOUND_INFO, SoundRand(), 0.1f+Rand()*0.1f);
                        m_partiTime[i] = 2.0f+Rand()*4.0f;
                    }
                    if ( ii == 4 )
                    {
                        m_sound->Play(SOUND_RADAR, SoundRand(), 0.2f+Rand()*0.2f);
                        m_partiTime[i] = 0.5f+Rand()*1.0f;
                    }
                    if ( ii == 5 )
                    {
                        m_sound->Play(SOUND_GFLAT, SoundRand(), 0.3f+Rand()*0.3f);
                        m_partiTime[i] = 2.0f+Rand()*4.0f;
                    }
                    if ( ii == 6 )
                    {
                        m_sound->Play(SOUND_ALARMt, SoundRand(), 0.1f+Rand()*0.1f);
                        m_partiTime[i] = 2.0f+Rand()*4.0f;
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
                    pos.x += (Rand()-0.5f)*0.01f;
                    pos.y += (Rand()-0.5f)*0.01f;
                    speed.x = (Rand()-0.5f)*0.2f;
                    speed.y = (Rand()-0.5f)*0.2f;
                    speed.z = 0.0f;
                    dim.x = 0.005f+Rand()*0.005f;
                    dim.y = dim.x/0.75f;
                    m_particule->CreateParticule(pos, speed, dim, PARTIBLITZ,
                                                 Rand()*0.2f+0.2f, 0.0f, 0.0f,
                                                 SH_INTERFACE);
                    pos.x = m_partiPos[i].x;
                    pos.y = m_partiPos[i].y;
                    pos.z = 0.0f;
                    speed.x = (Rand()-0.5f)*0.5f;
                    speed.y = (0.3f+Rand()*0.3f);
                    speed.z = 0.0f;
                    dim.x = 0.01f+Rand()*0.01f;
                    dim.y = dim.x/0.75f;
                    m_particule->CreateParticule(pos, speed, dim,
                                                 (ParticuleType)(PARTILENS1+rand()%3),
                                                 Rand()*0.5f+0.5f, 2.0f, 0.0f,
                                                 SH_INTERFACE);
                }
                if ( m_partiPhase[i] == 2 )  // sparks?
                {
                    pos.x = m_partiPos[i].x;
                    pos.y = m_partiPos[i].y;
                    pos.z = 0.0f;
                    pos.x += (Rand()-0.5f)*0.01f;
                    pos.y += (Rand()-0.5f)*0.01f;
                    speed.x = (Rand()-0.5f)*0.2f;
                    speed.y = (Rand()-0.5f)*0.2f;
                    speed.z = 0.0f;
                    dim.x = 0.005f+Rand()*0.005f;
                    dim.y = dim.x/0.75f;
                    m_particule->CreateParticule(pos, speed, dim, PARTIBLITZ,
                                                 Rand()*0.2f+0.2f, 0.0f, 0.0f,
                                                 SH_INTERFACE);
                    pos.x = m_partiPos[i].x;
                    pos.y = m_partiPos[i].y;
                    pos.z = 0.0f;
                    speed.x = (Rand()-0.5f)*0.5f;
                    speed.y = (0.3f+Rand()*0.3f);
                    speed.z = 0.0f;
                    dim.x = 0.005f+Rand()*0.005f;
                    dim.y = dim.x/0.75f;
                    m_particule->CreateParticule(pos, speed, dim, PARTISCRAPS,
                                                 Rand()*0.5f+0.5f, 2.0f, 0.0f,
                                                 SH_INTERFACE);
                }
                if ( m_partiPhase[i] == 3 )  // smoke?
                {
                    pos.x = m_partiPos[i].x;
                    pos.y = m_partiPos[i].y;
                    pos.z = 0.0f;
                    pos.x += (Rand()-0.5f)*0.03f;
                    pos.y += (Rand()-0.5f)*0.03f;
                    speed.x = (Rand()-0.5f)*0.2f;
                    speed.y = Rand()*0.5f;
                    speed.z = 0.0f;
                    dim.x = 0.03f+Rand()*0.07f;
                    dim.y = dim.x/0.75f;
                    m_particule->CreateParticule(pos, speed, dim, PARTICRASH,
                                                 Rand()*0.4f+0.4f, 0.0f, 0.0f,
                                                 SH_INTERFACE);
                }
            }
            else
            {
                m_partiPhase[i] = 0;
                m_partiTime[i] = 2.0f+Rand()*4.0f;
            }
        }
    }
#endif
}

// Some nice particles following the mouse.

void CMainDialog::NiceParticule(FPOINT mouse, BOOL bPress)
{
    D3DVECTOR   pos, speed;
    FPOINT      dim;

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
        speed.x = (Rand()-0.5f)*0.5f;
        speed.y = (0.3f+Rand()*0.3f);
        speed.z = 0.0f;
        dim.x = 0.005f+Rand()*0.005f;
        dim.y = dim.x/0.75f;
        m_particule->CreateParticule(pos, speed, dim, PARTISCRAPS,
                                     Rand()*0.5f+0.5f, 2.0f, 0.0f,
                                     SH_INTERFACE);
    }
    else
    {
        pos.x = mouse.x;
        pos.y = mouse.y;
        pos.z = 0.0f;
        speed.x = (Rand()-0.5f)*0.5f;
        speed.y = (0.3f+Rand()*0.3f);
        speed.z = 0.0f;
        dim.x = 0.01f+Rand()*0.01f;
        dim.y = dim.x/0.75f;
        m_particule->CreateParticule(pos, speed, dim,
                                     (ParticuleType)(PARTILENS1+rand()%3),
                                     Rand()*0.5f+0.5f, 2.0f, 0.0f,
                                     SH_INTERFACE);
    }
}



// Specifies the special user folder if needed.

void CMainDialog::SetUserDir(char *base, int rank)
{
    char    dir[100];

    if ( strcmp(base, "user") == 0 && rank >= 100 )
    {
        sprintf(dir, "%s\\%s", m_userDir, m_userList[rank/100-1]);
        UserDir(TRUE, dir);
    }
    else
    {
        UserDir(FALSE, "");
    }
}

// Builds the file name of a mission.

void CMainDialog::BuildSceneName(char *filename, char *base, int rank)
{
    if ( strcmp(base, "user") == 0 )
    {
        sprintf(filename, "%s\\%s\\scene%.2d.txt", m_userDir, m_userList[rank/100-1], rank%100);
    }
    else
    {
        sprintf(filename, "%s\\%s%.3d.txt", m_sceneDir, base, rank);
    }
}

// Built the default descriptive name of a mission.

void CMainDialog::BuildResumeName(char *filename, char *base, int rank)
{
    sprintf(filename, "Scene %s %d", base, rank);
}

// Returns the name of the file or save the files.

char* CMainDialog::RetFilesDir()
{
    return m_filesDir;
}


// Updates the list of players after checking the files on disk.

void CMainDialog::ReadNameList()
{
    CWindow*            pw;
    CList*              pl;
    long                hFile;
    struct _finddata_t  fBuffer;
    BOOL                bDo;
    char                dir[_MAX_FNAME];
    char                temp[_MAX_FNAME];
    char                filenames[_MAX_FNAME][100];
    int                 nbFilenames, i;

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW5);
    if ( pw == 0 )  return;
    pl = (CList*)pw->SearchControl(EVENT_INTERFACE_NLIST);
    if ( pl == 0 )  return;
    pl->Flush();

    nbFilenames = 0;
    sprintf(dir, "%s\\*", m_savegameDir);
    hFile = _findfirst(dir, &fBuffer);
    if ( hFile != -1 )
    {
        do
        {
            if ( (fBuffer.attrib & _A_SUBDIR) && fBuffer.name[0] != '.' )
            {
                strcpy(filenames[nbFilenames++], fBuffer.name);
            }
        }
        while ( _findnext(hFile, &fBuffer) == 0 && nbFilenames < 100 );
    }
    do  // sorts all names:
    {
        bDo = FALSE;
        for ( i=0 ; i<nbFilenames-1 ; i++ )
        {
            if ( strcmp(filenames[i], filenames[i+1]) > 0 )
            {
                strcpy(temp, filenames[i]);
                strcpy(filenames[i], filenames[i+1]);
                strcpy(filenames[i+1], temp);
                bDo = TRUE;
            }
        }
    }
    while ( bDo );

    for ( i=0 ; i<nbFilenames ; i++ )
    {
        pl->SetName(i, filenames[i]);
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

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW5);
    if ( pw == 0 )  return;
    pl = (CList*)pw->SearchControl(EVENT_INTERFACE_NLIST);
    if ( pl == 0 )  return;
    pe = (CEdit*)pw->SearchControl(EVENT_INTERFACE_NEDIT);
    if ( pe == 0 )  return;

    gamer = m_main->RetGamerName();
    total = pl->RetTotal();
    sel   = pl->RetSelect();
    pe->GetText(name, 100);

    pb = (CButton*)pw->SearchControl(EVENT_INTERFACE_NCANCEL);
    if ( pb != 0 )
    {
        pb->SetState(STATE_ENABLE, gamer[0]!=0);
    }

    pb = (CButton*)pw->SearchControl(EVENT_INTERFACE_NDELETE);
    if ( pb != 0 )
    {
        pb->SetState(STATE_ENABLE, total>0 && sel!=-1);
    }

    pb = (CButton*)pw->SearchControl(EVENT_INTERFACE_NOK);
    if ( pb != 0 )
    {
        pb->SetState(STATE_ENABLE, name[0]!=0 || sel!=-1);
    }

    pb = (CButton*)pw->SearchControl(EVENT_INTERFACE_PERSO);
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
    int         total, sel, i;

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW5);
    if ( pw == 0 )  return;
    pl = (CList*)pw->SearchControl(EVENT_INTERFACE_NLIST);
    if ( pl == 0 )  return;
    pe = (CEdit*)pw->SearchControl(EVENT_INTERFACE_NEDIT);
    if ( pe == 0 )  return;

    pe->GetText(name, 100);
    total = pl->RetTotal();
    sel   = pl->RetSelect();

    for ( i=0 ; i<total ; i++ )
    {
        if ( stricmp(name, pl->RetName(i)) == 0 )
        {
            pl->SetSelect(i);
            pl->ShowSelect(FALSE);
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

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW5);
    if ( pw == 0 )  return;
    pl = (CList*)pw->SearchControl(EVENT_INTERFACE_NLIST);
    if ( pl == 0 )  return;
    pe = (CEdit*)pw->SearchControl(EVENT_INTERFACE_NEDIT);
    if ( pe == 0 )  return;

    sel = pl->RetSelect();
    if ( sel == -1 )
    {
        pe->SetText("");
        pe->SetCursor(0, 0);
    }
    else
    {
        name = pl->RetName(sel);
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

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW5);
    if ( pw == 0 )  return;
    pl = (CList*)pw->SearchControl(EVENT_INTERFACE_NLIST);
    if ( pl == 0 )  return;

    sel = pl->RetSelect();
    if ( sel == -1 )  return;
    name = pl->RetName(sel);

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

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW5);
    if ( pw == 0 )  return;
    pl = (CList*)pw->SearchControl(EVENT_INTERFACE_NLIST);
    if ( pl == 0 )  return;
    pe = (CEdit*)pw->SearchControl(EVENT_INTERFACE_NEDIT);
    if ( pe == 0 )  return;

    pe->GetText(name, 100);
    sel  = pl->RetSelect();

    if ( sel == -1 )
    {
        NameCreate();
    }
    else
    {
        m_main->SetGamerName(pl->RetName(sel));
        m_main->ChangePhase(PHASE_INIT);
    }

    RetGamerFace(m_main->RetGamerName());

    SetLocalProfileString("Gamer", "LastName", m_main->RetGamerName());
}

// Creates a new player.

void CMainDialog::NameCreate()
{
    CWindow*    pw;
    CEdit*      pe;
    char        name[100];
    char        dir[100];
    char        c;
    int         len, i, j;

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW5);
    if ( pw == 0 )  return;
    pe = (CEdit*)pw->SearchControl(EVENT_INTERFACE_NEDIT);
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
        c = RetNoAccent(RetToLower(name[i]));
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

    _mkdir(m_savegameDir);  // if does not exist yet!

    sprintf(dir, "%s\\%s", m_savegameDir, name);
    if ( _mkdir(dir) != 0 )
    {
        m_sound->Play(SOUND_TZOING);
        pe->SetText(name);
        pe->SetCursor(strlen(name), 0);
        pe->SetFocus(TRUE);
        return;
    }

    SetGamerFace(name, 0);

    m_main->SetGamerName(name);
    m_main->ChangePhase(PHASE_INIT);
}

// Deletes a folder and all its offspring.

BOOL RemoveDir(char *dirname)
{
    long                hFile;
    struct _finddata_t  fBuffer;
    char                filename[100];

    sprintf(filename, "%s\\*", dirname);
    hFile = _findfirst(filename, &fBuffer);
    if ( hFile != -1 )
    {
        do
        {
            if ( fBuffer.name[0] != '.' )
            {
                if ( fBuffer.attrib & _A_SUBDIR )
                {
                    sprintf(filename, "%s\\%s", dirname, fBuffer.name);
                    RemoveDir(filename);
                }
                else
                {
                    sprintf(filename, "%s\\%s", dirname, fBuffer.name);
                    remove(filename);
                }
            }
        }
        while ( _findnext(hFile, &fBuffer) == 0 );
    }

    if ( _rmdir(dirname) != 0 )
    {
        return FALSE;
    }
    return TRUE;
}

// Removes a player.

void CMainDialog::NameDelete()
{
    CWindow*    pw;
    CList*      pl;
    int         sel;
    char*       gamer;
    char        dir[100];

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW5);
    if ( pw == 0 )  return;
    pl = (CList*)pw->SearchControl(EVENT_INTERFACE_NLIST);
    if ( pl == 0 )  return;

    sel = pl->RetSelect();
    if ( sel == -1 )
    {
        m_sound->Play(SOUND_TZOING);
        return;
    }
    gamer = pl->RetName(sel);

    // Deletes all the contents of the file.
    sprintf(dir, "%s\\%s", m_savegameDir, gamer);
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

BOOL EqColor(const D3DCOLORVALUE &c1, const D3DCOLORVALUE &c2)
{
    return (Abs(c1.r-c2.r) < 0.01f &&
            Abs(c1.g-c2.g) < 0.01f &&
            Abs(c1.b-c2.b) < 0.01f );
}

// Updates all the buttons for the character.

void CMainDialog::UpdatePerso()
{
    CWindow*        pw;
    CLabel*         pl;
    CButton*        pb;
    CColor*         pc;
    CSlider*        ps;
    D3DCOLORVALUE   color;
    char            name[100];
    int             i;

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW5);
    if ( pw == 0 )  return;

    pb = (CButton*)pw->SearchControl(EVENT_INTERFACE_PHEAD);
    if ( pb != 0 )
    {
        pb->SetState(STATE_CHECK, m_persoTab==0);
    }
    pb = (CButton*)pw->SearchControl(EVENT_INTERFACE_PBODY);
    if ( pb != 0 )
    {
        pb->SetState(STATE_CHECK, m_persoTab==1);
    }

    pl = (CLabel*)pw->SearchControl(EVENT_LABEL11);
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

    pl = (CLabel*)pw->SearchControl(EVENT_LABEL12);
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

    pl = (CLabel*)pw->SearchControl(EVENT_LABEL13);
    if ( pl != 0 )
    {
        if ( m_persoTab == 0 )  GetResource(RES_TEXT, RT_PERSO_HAIR, name);
        else                    GetResource(RES_TEXT, RT_PERSO_BAND, name);
        pl->SetName(name);
    }

    pl = (CLabel*)pw->SearchControl(EVENT_LABEL14);
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
        pb = (CButton*)pw->SearchControl((EventMsg)(EVENT_INTERFACE_PFACE1+i));
        if ( pb == 0 )  break;
        pb->SetState(STATE_VISIBLE, m_persoTab==0);
        pb->SetState(STATE_CHECK, i==m_perso.face);
    }

    for ( i=0 ; i<10 ; i++ )
    {
        pb = (CButton*)pw->SearchControl((EventMsg)(EVENT_INTERFACE_PGLASS0+i));
        if ( pb == 0 )  break;
        pb->SetState(STATE_VISIBLE, m_persoTab==0);
        pb->SetState(STATE_CHECK, i==m_perso.glasses);
    }

    for ( i=0 ; i<3*3 ; i++ )
    {
        pc = (CColor*)pw->SearchControl((EventMsg)(EVENT_INTERFACE_PC0a+i));
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

        pc = (CColor*)pw->SearchControl((EventMsg)(EVENT_INTERFACE_PC0b+i));
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
        ps = (CSlider*)pw->SearchControl((EventMsg)(EVENT_INTERFACE_PCRa+i));
        if ( ps == 0 )  break;
        ps->SetState(STATE_VISIBLE, m_persoTab==1);
    }

    if ( m_persoTab == 1 )
    {
        color = m_perso.colorCombi;
        ps = (CSlider*)pw->SearchControl(EVENT_INTERFACE_PCRa);
        if ( ps != 0 )  ps->SetVisibleValue(color.r*255.0f);
        ps = (CSlider*)pw->SearchControl(EVENT_INTERFACE_PCGa);
        if ( ps != 0 )  ps->SetVisibleValue(color.g*255.0f);
        ps = (CSlider*)pw->SearchControl(EVENT_INTERFACE_PCBa);
        if ( ps != 0 )  ps->SetVisibleValue(color.b*255.0f);
    }

    if ( m_persoTab == 0 )  color = m_perso.colorHair;
    else                    color = m_perso.colorBand;
    ps = (CSlider*)pw->SearchControl(EVENT_INTERFACE_PCRb);
    if ( ps != 0 )  ps->SetVisibleValue(color.r*255.0f);
    ps = (CSlider*)pw->SearchControl(EVENT_INTERFACE_PCGb);
    if ( ps != 0 )  ps->SetVisibleValue(color.g*255.0f);
    ps = (CSlider*)pw->SearchControl(EVENT_INTERFACE_PCBb);
    if ( ps != 0 )  ps->SetVisibleValue(color.b*255.0f);
}

// Updates the camera for the character.

void CMainDialog::CameraPerso()
{
    if ( m_persoTab == 0 )
    {
//?     m_camera->Init(D3DVECTOR(4.0f, 0.0f, 0.0f),
//?                    D3DVECTOR(0.0f, 0.0f, 1.0f), 0.0f);
        m_camera->Init(D3DVECTOR(6.0f, 0.0f, 0.0f),
                       D3DVECTOR(0.0f, 0.2f, 1.5f), 0.0f);
    }
    else
    {
        m_camera->Init(D3DVECTOR(18.0f, 0.0f, 4.5f),
                       D3DVECTOR(0.0f, 1.6f, 4.5f), 0.0f);
    }

    m_camera->SetType(CAMERA_SCRIPT);
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
    D3DCOLORVALUE   color;

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW5);
    if ( pw == 0 )  return;

    color.a = 0.0f;

    ps = (CSlider*)pw->SearchControl(EVENT_INTERFACE_PCRa);
    if ( ps != 0 )  color.r = ps->RetVisibleValue()/255.0f;
    ps = (CSlider*)pw->SearchControl(EVENT_INTERFACE_PCGa);
    if ( ps != 0 )  color.g = ps->RetVisibleValue()/255.0f;
    ps = (CSlider*)pw->SearchControl(EVENT_INTERFACE_PCBa);
    if ( ps != 0 )  color.b = ps->RetVisibleValue()/255.0f;
    if ( m_persoTab == 1 )  m_perso.colorCombi = color;

    ps = (CSlider*)pw->SearchControl(EVENT_INTERFACE_PCRb);
    if ( ps != 0 )  color.r = ps->RetVisibleValue()/255.0f;
    ps = (CSlider*)pw->SearchControl(EVENT_INTERFACE_PCGb);
    if ( ps != 0 )  color.g = ps->RetVisibleValue()/255.0f;
    ps = (CSlider*)pw->SearchControl(EVENT_INTERFACE_PCBb);
    if ( ps != 0 )  color.b = ps->RetVisibleValue()/255.0f;
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

BOOL CMainDialog::IsIOReadScene()
{
    FILE*   file;
    char    filename[100];

    sprintf(filename, "%s\\%s\\save%c%.3d\\data.sav", m_savegameDir, m_main->RetGamerName(), m_sceneName[0], 0);
    file = fopen(filename, "r");
    if ( file == NULL )  return FALSE;
    fclose(file);
    return TRUE;
}

// Builds the file name by default.

void CMainDialog::IOReadName()
{
    FILE*       file;
    CWindow*    pw;
    CEdit*      pe;
    char        filename[_MAX_FNAME];
    char        op[100];
    char        line[500];
    char        resume[100];
    char        name[100];
    time_t      now;
    int         i;

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW5);
    if ( pw == 0 )  return;
    pe = (CEdit*)pw->SearchControl(EVENT_INTERFACE_IONAME);
    if ( pe == 0 )  return;

    sprintf(resume, "%s %d", m_sceneName, m_chap[m_index]+1);
    BuildSceneName(filename, m_sceneName, (m_chap[m_index]+1)*100);
    file = fopen(filename, "r");
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

            sprintf(op, "Title.%c", RetLanguageLetter());
            if ( Cmd(line, op) )
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
    pe->SetFocus(TRUE);
}

// Updates the list of games recorded on disk.

void CMainDialog::IOReadList()
{
    FILE*       file = NULL;
    CWindow*    pw;
    CList*      pl;
    char        filename[100];
    char        line[500];
    char        name[100];
    int         i, j;

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW5);
    if ( pw == 0 )  return;
    pl = (CList*)pw->SearchControl(EVENT_INTERFACE_IOLIST);
    if ( pl == 0 )  return;

    pl->Flush();

    for ( j=0 ; j<999 ; j++ )
    {
        sprintf(filename, "%s\\%s\\save%c%.3d\\data.sav", m_savegameDir, m_main->RetGamerName(), m_sceneName[0], j);
        file = fopen(filename, "r");
        if ( file == NULL )  break;

        strcmp(name, filename);  // default name
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
    pl->ShowSelect(FALSE);  // shows the selected columns
}

// Updates the buttons according to the selected part in the list.

void CMainDialog::IOUpdateList()
{
    FILE*       file = NULL;
    CWindow*    pw;
    CList*      pl;
    CButton*    pb;
    CImage*     pi;
    char        filename[100];
    int         sel, max;

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW5);
    if ( pw == 0 )  return;
    pl = (CList*)pw->SearchControl(EVENT_INTERFACE_IOLIST);
    if ( pl == 0 )  return;
    pi = (CImage*)pw->SearchControl(EVENT_INTERFACE_IOIMAGE);
    if ( pi == 0 )  return;

    sel = pl->RetSelect();
    max = pl->RetTotal();

    sprintf(filename, "%s\\%s\\save%c%.3d\\screen.bmp", m_savegameDir, m_main->RetGamerName(), m_sceneName[0], sel);

    if ( m_phase == PHASE_WRITE  ||
         m_phase == PHASE_WRITEs )
    {
        if ( sel < max-1 )
        {
            pi->SetFilenameImage(filename);
        }
        else
        {
            pi->SetFilenameImage("");
        }

        pb = (CButton*)pw->SearchControl(EVENT_INTERFACE_IODELETE);
        if ( pb != 0 )
        {
            pb->SetState(STATE_ENABLE, sel < max-1);
        }
    }
    else
    {
        pi->SetFilenameImage(filename);
    }
}

// Deletes the selected scene.

void CMainDialog::IODeleteScene()
{
    CWindow*            pw;
    CList*              pl;
    char                dir[100];
    char                old[100];
    long                hFile;
    struct _finddata_t  fBuffer;
    int                 sel, max, i;

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW5);
    if ( pw == 0 )  return;
    pl = (CList*)pw->SearchControl(EVENT_INTERFACE_IOLIST);
    if ( pl == 0 )  return;

    sel = pl->RetSelect();
    if ( sel == -1 )
    {
        m_sound->Play(SOUND_TZOING);
        return;
    }

    // Deletes all the contents of the file.
    sprintf(dir, "%s\\%s\\save%c%.3d\\*", m_savegameDir, m_main->RetGamerName(), m_sceneName[0], sel);
    hFile = _findfirst(dir, &fBuffer);
    if ( hFile != -1 )
    {
        do
        {
            if ( fBuffer.name[0] != '.' )
            {
                sprintf(dir, "%s\\%s\\save%c%.3d\\%s", m_savegameDir, m_main->RetGamerName(), m_sceneName[0], sel, fBuffer.name);
                remove(dir);
            }
        }
        while ( _findnext(hFile, &fBuffer) == 0 );
    }

    sprintf(dir, "%s\\%s\\save%c%.3d", m_savegameDir, m_main->RetGamerName(), m_sceneName[0], sel);
    if ( _rmdir(dir) != 0 )
    {
        m_sound->Play(SOUND_TZOING);
        return;
    }

    max = pl->RetTotal();
    for ( i=sel+1 ; i<max ; i++ )
    {
        sprintf(old, "%s\\%s\\save%c%.3d", m_savegameDir, m_main->RetGamerName(), m_sceneName[0], i);
        sprintf(dir, "%s\\%s\\save%c%.3d", m_savegameDir, m_main->RetGamerName(), m_sceneName[0], i-1);
        rename(old, dir);
    }
    IOReadList();
}

// Writes the scene.

BOOL CMainDialog::IOWriteScene()
{
    CWindow*    pw;
    CList*      pl;
    CEdit*      pe;
    char        filename[100];
    char        filecbot[100];
    char        info[100];
    int         sel;

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW5);
    if ( pw == 0 )  return FALSE;
    pl = (CList*)pw->SearchControl(EVENT_INTERFACE_IOLIST);
    if ( pl == 0 )  return FALSE;
    pe = (CEdit*)pw->SearchControl(EVENT_INTERFACE_IONAME);
    if ( pe == 0 )  return FALSE;

    sel = pl->RetSelect();
    if ( sel == -1 )  return FALSE;

    _mkdir("Savegame");  // if doesn't exist yet!
    sprintf(filename, "%s\\%s", m_savegameDir, m_main->RetGamerName());
    _mkdir(filename);
    sprintf(filename, "%s\\%s\\save%c%.3d", m_savegameDir, m_main->RetGamerName(), m_sceneName[0], sel);
    _mkdir(filename);

    sprintf(filename, "%s\\%s\\save%c%.3d\\data.sav", m_savegameDir, m_main->RetGamerName(), m_sceneName[0], sel);
    sprintf(filecbot, "%s\\%s\\save%c%.3d\\cbot.run", m_savegameDir, m_main->RetGamerName(), m_sceneName[0], sel);
    pe->GetText(info, 100);
    m_main->IOWriteScene(filename, filecbot, info);

    m_shotDelay = 3;
    sprintf(m_shotName, "%s\\%s\\save%c%.3d\\screen.bmp", m_savegameDir, m_main->RetGamerName(), m_sceneName[0], sel);

    return TRUE;
}

// Reads the scene.

BOOL CMainDialog::IOReadScene()
{
    FILE*       file;
    CWindow*    pw;
    CList*      pl;
    char        filename[100];
    char        filecbot[100];
    char        line[500];
    char        dir[100];
    int         sel, i;

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW5);
    if ( pw == 0 )  return FALSE;
    pl = (CList*)pw->SearchControl(EVENT_INTERFACE_IOLIST);
    if ( pl == 0 )  return FALSE;

    sel = pl->RetSelect();
    if ( sel == -1 )  return FALSE;

    sprintf(filename, "%s\\%s\\save%c%.3d\\data.sav", m_savegameDir, m_main->RetGamerName(), m_sceneName[0], sel);
    sprintf(filecbot, "%s\\%s\\save%c%.3d\\cbot.run", m_savegameDir, m_main->RetGamerName(), m_sceneName[0], sel);

    file = fopen(filename, "r");
    if ( file == NULL )  return FALSE;

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
                    if ( strcmp(m_userList[i], dir) == 0 )
                    {
                        m_sceneRank += (i+1)*100;
                        break;
                    }
                }
                if ( m_sceneRank/100 == 0 )
                {
                    fclose(file);
                    return FALSE;
                }
            }
        }
    }
    fclose(file);

    m_chap[m_index] = (m_sceneRank/100)-1;
    m_sel[m_index]  = (m_sceneRank%100)-1;

    strcpy(m_sceneRead, filename);
    strcpy(m_stackRead, filecbot);
    return TRUE;
}


// Returns the number of accessible chapters.

int CMainDialog::RetChapPassed()
{
    int         j;

    if ( m_main->RetShowAll() )  return 9;

    for ( j=0 ; j<9 ; j++ )
    {
        if ( !RetGamerInfoPassed((j+1)*100) )
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
    long        hFile;
    struct _finddata_t fileBuffer;
    char        filename[_MAX_FNAME];
    char        op[100];
    char        line[500];
    char        name[100];
    int         i, j;
    BOOL        bPassed, bDo;

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW5);
    if ( pw == 0 )  return;
    pl = (CList*)pw->SearchControl(EVENT_INTERFACE_CHAP);
    if ( pl == 0 )  return;

    pl->Flush();

    if ( m_phase == PHASE_USER )
    {
        j = 0;
        hFile = _findfirst("user\\*", &fileBuffer);
        if ( hFile != -1 )
        {
            do
            {
                if ( (fileBuffer.attrib & _A_SUBDIR) != 0 &&
                     fileBuffer.name[0] != '.' )
                {
                    strcpy(m_userList[j++], fileBuffer.name);
                }
            }
            while ( _findnext(hFile, &fileBuffer) == 0 && j < 100 );
        }
        m_userTotal = j;

        do  // sorts all names:
        {
            bDo = FALSE;
            for ( i=0 ; i<m_userTotal-1 ; i++ )
            {
                if ( strcmp(m_userList[i], m_userList[i+1]) > 0 )
                {
                    strcpy(name, m_userList[i]);
                    strcpy(m_userList[i], m_userList[i+1]);
                    strcpy(m_userList[i+1], name);
                    bDo = TRUE;
                }
            }
        }
        while ( bDo );

        for ( j=0 ; j<m_userTotal ; j++ )
        {
            BuildSceneName(filename, m_sceneName, (j+1)*100);
            file = fopen(filename, "r");
            if ( file == NULL )
            {
                strcpy(name, m_userList[j]);
            }
            else
            {
                BuildResumeName(name, m_sceneName, j+1);  // default name
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

                    sprintf(op, "Title.%c", RetLanguageLetter());
                    if ( Cmd(line, op) )
                    {
                        OpString(line, "text", name);
                        break;
                    }
                }
                fclose(file);
            }

            pl->SetName(j, name);
            pl->SetEnable(j, TRUE);
        }
    }
    else
    {
        for ( j=0 ; j<9 ; j++ )
        {
#if _SCHOOL
            if ( m_phase == PHASE_MISSION )  break;
            if ( m_phase == PHASE_FREE    )  break;
#if _CEEBOTDEMO
            if ( m_phase == PHASE_TRAINER && j >= 2 )  break;
#endif
#endif
#if _DEMO
            if ( m_phase == PHASE_MISSION && j >= 4 )  break;
            if ( m_phase == PHASE_TRAINER && j >= 1 )  break;
#endif
            BuildSceneName(filename, m_sceneName, (j+1)*100);
            file = fopen(filename, "r");
            if ( file == NULL )  break;

            BuildResumeName(name, m_sceneName, j+1);  // default name
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

                sprintf(op, "Title.%c", RetLanguageLetter());
                if ( Cmd(line, op) )
                {
                    OpString(line, "text", name);
                    break;
                }
            }
            fclose(file);

            bPassed = RetGamerInfoPassed((j+1)*100);
            sprintf(line, "%d: %s", j+1, name);
            pl->SetName(j, line);
            pl->SetCheck(j, bPassed);
            pl->SetEnable(j, TRUE);

            if ( m_phase == PHASE_MISSION && !m_main->RetShowAll() && !bPassed )
            {
                j ++;
                break;
            }

#if _TEEN
            if ( m_phase == PHASE_TRAINER && !m_main->RetShowAll() && !bPassed )
            {
                j ++;
                break;
            }
#endif

            if ( m_phase == PHASE_FREE && j == m_accessChap )
            {
                j ++;
                break;
            }
        }
    }

    if ( chap > j-1 )  chap = j-1;

    pl->SetSelect(chap);
    pl->ShowSelect(FALSE);  // shows the selected columns
}

// Updates the list of exercises or missions.

void CMainDialog::UpdateSceneList(int chap, int &sel)
{
    FILE*       file = NULL;
    CWindow*    pw;
    CList*      pl;
    char        filename[_MAX_FNAME];
    char        op[100];
    char        line[500];
    char        name[100];
    int         i, j;
    BOOL        bPassed;

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW5);
    if ( pw == 0 )  return;
    pl = (CList*)pw->SearchControl(EVENT_INTERFACE_LIST);
    if ( pl == 0 )  return;

    pl->Flush();

    for ( j=0 ; j<99 ; j++ )
    {
#if _SCHOOL
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
#endif
        BuildSceneName(filename, m_sceneName, (chap+1)*100+(j+1));
        file = fopen(filename, "r");
        if ( file == NULL )  break;

        BuildResumeName(name, m_sceneName, j+1);  // default name
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

            sprintf(op, "Title.%c", RetLanguageLetter());
            if ( Cmd(line, op) )
            {
                OpString(line, "text", name);
                break;
            }
        }
        fclose(file);

        bPassed = RetGamerInfoPassed((chap+1)*100+(j+1));
        sprintf(line, "%d: %s", j+1, name);
        pl->SetName(j, line);
        pl->SetCheck(j, bPassed);
        pl->SetEnable(j, TRUE);

        if ( m_phase == PHASE_MISSION && !m_main->RetShowAll() && !bPassed )
        {
            j ++;
            break;
        }

#if _TEEN
        if ( m_phase == PHASE_TRAINER && !m_main->RetShowAll() && !bPassed )
        {
            j ++;
            break;
        }
#endif
    }

    BuildSceneName(filename, m_sceneName, (chap+1)*100+(j+1));
    file = fopen(filename, "r");
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
    pl->ShowSelect(FALSE);  // shows the selected columns
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
        m_bSceneSoluce = FALSE;

        pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW5);
        if ( pw == 0 )  return;
        pe = (CEdit*)pw->SearchControl(EVENT_INTERFACE_RESUME);
        if ( pe == 0 )  return;
        pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_SOLUCE);
        if ( pc == 0 )  return;

        if ( m_main->RetShowSoluce() )
        {
            pc->SetState(STATE_VISIBLE);
            pc->SetState(STATE_CHECK);
            m_bSceneSoluce = TRUE;
        }
        else
        {
            pc->ClearState(STATE_VISIBLE);
            pc->ClearState(STATE_CHECK);
            m_bSceneSoluce = FALSE;
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
    char        filename[_MAX_FNAME];
    char        op[100];
    char        line[500];
    char        name[500];
    int         i, numTry;
    BOOL        bPassed, bVisible;

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW5);
    if ( pw == 0 )  return;
    pe = (CEdit*)pw->SearchControl(EVENT_INTERFACE_RESUME);
    if ( pe == 0 )  return;
    pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_SOLUCE);

    if ( pc == 0 )
    {
        m_bSceneSoluce = FALSE;
    }
    else
    {
        numTry  = RetGamerInfoTry(rank);
        bPassed = RetGamerInfoPassed(rank);
        bVisible = ( numTry > 2 || bPassed || m_main->RetShowSoluce() );
        if ( !RetSoluce4() )  bVisible = FALSE;
        pc->SetState(STATE_VISIBLE, bVisible);
        if ( !bVisible )
        {
            pc->ClearState(STATE_CHECK);
            m_bSceneSoluce = FALSE;
        }
    }

    BuildSceneName(filename, m_sceneName, rank);
    file = fopen(filename, "r");
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

        sprintf(op, "Resume.%c", RetLanguageLetter());
        if ( Cmd(line, op) )
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

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW5);
    if ( pw == 0 )  return;
    pl = (CList*)pw->SearchControl(EVENT_LIST1);
    if ( pl == 0 )  return;
    pl->Flush();

    m_engine->EnumDevices(bufDevices, 1000,
                          bufModes,   5000,
                          totalDevices, selectDevices,
                          totalModes,   selectModes);

    i = 0;
    j = 0;
    while ( bufDevices[i] != 0 )
    {
        pl->SetName(j++, bufDevices+i);
        while ( bufDevices[i++] != 0 );
    }

    pl->SetSelect(selectDevices);
    pl->ShowSelect(FALSE);

    m_setupSelDevice = selectDevices;
}

// Updates the list of modes.

void CMainDialog::UpdateDisplayMode()
{
    CWindow*    pw;
    CList*      pl;
    char        bufDevices[1000];
    char        bufModes[5000];
    int         i, j, totalDevices, selectDevices, totalModes, selectModes;

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW5);
    if ( pw == 0 )  return;
    pl = (CList*)pw->SearchControl(EVENT_LIST2);
    if ( pl == 0 )  return;
    pl->Flush();

    m_engine->EnumDevices(bufDevices, 1000,
                          bufModes,   5000,
                          totalDevices, selectDevices,
                          totalModes,   selectModes);

    i = 0;
    j = 0;
    while ( bufModes[i] != 0 )
    {
        pl->SetName(j++, bufModes+i);
        while ( bufModes[i++] != 0 );
    }

    pl->SetSelect(selectModes);
    pl->ShowSelect(FALSE);

    m_setupSelMode = selectModes;
}

// Change the graphics mode.

void CMainDialog::ChangeDisplay()
{
    CWindow*    pw;
    CList*      pl;
    CCheck*     pc;
    char*       device;
    char*       mode;
    BOOL        bFull;

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW5);
    if ( pw == 0 )  return;

    pl = (CList*)pw->SearchControl(EVENT_LIST1);
    if ( pl == 0 )  return;
    m_setupSelDevice = pl->RetSelect();
    device = pl->RetName(m_setupSelDevice);

    pl = (CList*)pw->SearchControl(EVENT_LIST2);
    if ( pl == 0 )  return;
    m_setupSelMode = pl->RetSelect();
    mode = pl->RetName(m_setupSelMode);

    pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_FULL);
    if ( pc == 0 )  return;
    bFull = pc->TestState(STATE_CHECK);
    m_setupFull = bFull;

    m_engine->ChangeDevice(device, mode, bFull);

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
    BOOL        bFull;

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW5);
    if ( pw == 0 )  return;

    pb = (CButton*)pw->SearchControl(EVENT_INTERFACE_APPLY);
    if ( pb == 0 )  return;

    pl = (CList*)pw->SearchControl(EVENT_LIST1);
    if ( pl == 0 )  return;
    sel1 = pl->RetSelect();

    pl = (CList*)pw->SearchControl(EVENT_LIST2);
    if ( pl == 0 )  return;
    sel2 = pl->RetSelect();

    pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_FULL);
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

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW5);
    if ( pw == 0 )  return;

    pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_TOTO);
    if ( pc != 0 )
    {
        pc->SetState(STATE_CHECK, m_engine->RetTotoMode());
    }

    pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_TOOLTIP);
    if ( pc != 0 )
    {
        pc->SetState(STATE_CHECK, m_bTooltip);
    }

    pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_GLINT);
    if ( pc != 0 )
    {
        pc->SetState(STATE_CHECK, m_bGlint);
    }

    pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_RAIN);
    if ( pc != 0 )
    {
        pc->SetState(STATE_CHECK, m_bRain);
    }

    pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_MOUSE);
    if ( pc != 0 )
    {
        pc->SetState(STATE_CHECK, m_engine->RetNiceMouse());
        pc->SetState(STATE_ENABLE, m_engine->RetNiceMouseCap());
    }

    pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_EDITMODE);
    if ( pc != 0 )
    {
        pc->SetState(STATE_CHECK, m_engine->RetEditIndentMode());
    }

    pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_EDITVALUE);
    if ( pc != 0 )
    {
        pc->SetState(STATE_CHECK, m_engine->RetEditIndentValue()>2);
    }

    pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_SOLUCE4);
    if ( pc != 0 )
    {
        pc->SetState(STATE_CHECK, m_bSoluce4);
    }

    pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_MOVIES);
    if ( pc != 0 )
    {
        pc->SetState(STATE_CHECK, m_bMovies);
    }

    pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_NICERST);
    if ( pc != 0 )
    {
        pc->SetState(STATE_CHECK, m_bNiceReset);
    }

    pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_HIMSELF);
    if ( pc != 0 )
    {
        pc->SetState(STATE_CHECK, m_bHimselfDamage);
    }

    pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_SCROLL);
    if ( pc != 0 )
    {
        pc->SetState(STATE_CHECK, m_bCameraScroll);
    }

    pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_INVERTX);
    if ( pc != 0 )
    {
        pc->SetState(STATE_CHECK, m_bCameraInvertX);
    }

    pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_INVERTY);
    if ( pc != 0 )
    {
        pc->SetState(STATE_CHECK, m_bCameraInvertY);
    }

    pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_EFFECT);
    if ( pc != 0 )
    {
        pc->SetState(STATE_CHECK, m_bEffect);
    }

    pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_SHADOW);
    if ( pc != 0 )
    {
        pc->SetState(STATE_CHECK, m_engine->RetShadow());
    }

    pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_GROUND);
    if ( pc != 0 )
    {
        pc->SetState(STATE_CHECK, m_engine->RetGroundSpot());
    }

    pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_DIRTY);
    if ( pc != 0 )
    {
        pc->SetState(STATE_CHECK, m_engine->RetDirty());
    }

    pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_FOG);
    if ( pc != 0 )
    {
        pc->SetState(STATE_CHECK, m_engine->RetFog());
    }

    pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_LENS);
    if ( pc != 0 )
    {
        pc->SetState(STATE_CHECK, m_engine->RetLensMode());
    }

    pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_SKY);
    if ( pc != 0 )
    {
        pc->SetState(STATE_CHECK, m_engine->RetSkyMode());
    }

    pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_PLANET);
    if ( pc != 0 )
    {
        pc->SetState(STATE_CHECK, m_engine->RetPlanetMode());
    }

    pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_LIGHT);
    if ( pc != 0 )
    {
        pc->SetState(STATE_CHECK, m_engine->RetLightMode());
    }

    pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_JOYSTICK);
    if ( pc != 0 )
    {
        pc->SetState(STATE_CHECK, m_engine->RetJoystick());
    }

    pv = (CEditValue*)pw->SearchControl(EVENT_INTERFACE_PARTI);
    if ( pv != 0 )
    {
        value = m_engine->RetParticuleDensity();
        pv->SetValue(value);
    }

    pv = (CEditValue*)pw->SearchControl(EVENT_INTERFACE_CLIP);
    if ( pv != 0 )
    {
        value = m_engine->RetClippingDistance();
        pv->SetValue(value);
    }

    pv = (CEditValue*)pw->SearchControl(EVENT_INTERFACE_DETAIL);
    if ( pv != 0 )
    {
        value = m_engine->RetObjectDetail();
        pv->SetValue(value);
    }

    pv = (CEditValue*)pw->SearchControl(EVENT_INTERFACE_GADGET);
    if ( pv != 0 )
    {
        value = m_engine->RetGadgetQuantity();
        pv->SetValue(value);
    }

    pv = (CEditValue*)pw->SearchControl(EVENT_INTERFACE_TEXTURE);
    if ( pv != 0 )
    {
        value = (float)m_engine->RetTextureQuality();
        pv->SetValue(value);
    }

    ps = (CSlider*)pw->SearchControl(EVENT_INTERFACE_VOLSOUND);
    if ( ps != 0 )
    {
        value = (float)m_sound->RetAudioVolume();
        ps->SetVisibleValue(value);
    }

    ps = (CSlider*)pw->SearchControl(EVENT_INTERFACE_VOLMUSIC);
    if ( ps != 0 )
    {
        value = (float)m_sound->RetMidiVolume();
        ps->SetVisibleValue(value);
    }

    pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_SOUND3D);
    if ( pc != 0 )
    {
        pc->SetState(STATE_CHECK, m_sound->RetSound3D());
        pc->SetState(STATE_ENABLE, m_sound->RetSound3DCap());
    }
}

// Updates the engine function of the buttons after the setup phase.

void CMainDialog::ChangeSetupButtons()
{
    CWindow*    pw;
    CEditValue* pv;
    CSlider*    ps;
    float       value;

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW5);
    if ( pw == 0 )  return;

    pv = (CEditValue*)pw->SearchControl(EVENT_INTERFACE_PARTI);
    if ( pv != 0 )
    {
        value = pv->RetValue();
        m_engine->SetParticuleDensity(value);
    }

    pv = (CEditValue*)pw->SearchControl(EVENT_INTERFACE_CLIP);
    if ( pv != 0 )
    {
        value = pv->RetValue();
        m_engine->SetClippingDistance(value);
    }

    pv = (CEditValue*)pw->SearchControl(EVENT_INTERFACE_DETAIL);
    if ( pv != 0 )
    {
        value = pv->RetValue();
        m_engine->SetObjectDetail(value);
    }

    pv = (CEditValue*)pw->SearchControl(EVENT_INTERFACE_GADGET);
    if ( pv != 0 )
    {
        value = pv->RetValue();
        m_engine->SetGadgetQuantity(value);
    }

    pv = (CEditValue*)pw->SearchControl(EVENT_INTERFACE_TEXTURE);
    if ( pv != 0 )
    {
        value = pv->RetValue();
        m_engine->SetTextureQuality((int)value);
    }

    ps = (CSlider*)pw->SearchControl(EVENT_INTERFACE_VOLSOUND);
    if ( ps != 0 )
    {
        value = ps->RetVisibleValue();
        m_sound->SetAudioVolume((int)value);
    }

    ps = (CSlider*)pw->SearchControl(EVENT_INTERFACE_VOLMUSIC);
    if ( ps != 0 )
    {
        value = ps->RetVisibleValue();
        m_sound->SetMidiVolume((int)value);
    }
}


// Memorizes all the settings.

void CMainDialog::SetupMemorize()
{
    float   fValue;
    int     iValue, i, j;
    char    key[500];
    char    num[10];

    SetLocalProfileString("Directory", "scene",    m_sceneDir);
    SetLocalProfileString("Directory", "savegame", m_savegameDir);
    SetLocalProfileString("Directory", "public",   m_publicDir);
    SetLocalProfileString("Directory", "user",     m_userDir);
    SetLocalProfileString("Directory", "files",    m_filesDir);

    iValue = m_engine->RetTotoMode();
    SetLocalProfileInt("Setup", "TotoMode", iValue);

    iValue = m_bTooltip;
    SetLocalProfileInt("Setup", "Tooltips", iValue);

    iValue = m_bGlint;
    SetLocalProfileInt("Setup", "InterfaceGlint", iValue);

    iValue = m_bRain;
    SetLocalProfileInt("Setup", "InterfaceGlint", iValue);

    iValue = m_engine->RetNiceMouse();
    SetLocalProfileInt("Setup", "NiceMouse", iValue);

    iValue = m_bSoluce4;
    SetLocalProfileInt("Setup", "Soluce4", iValue);

    iValue = m_bMovies;
    SetLocalProfileInt("Setup", "Movies", iValue);

    iValue = m_bNiceReset;
    SetLocalProfileInt("Setup", "NiceReset", iValue);

    iValue = m_bHimselfDamage;
    SetLocalProfileInt("Setup", "HimselfDamage", iValue);

    iValue = m_bCameraScroll;
    SetLocalProfileInt("Setup", "CameraScroll", iValue);

    iValue = m_bCameraInvertX;
    SetLocalProfileInt("Setup", "CameraInvertX", iValue);

    iValue = m_bEffect;
    SetLocalProfileInt("Setup", "InterfaceEffect", iValue);

    iValue = m_engine->RetShadow();
    SetLocalProfileInt("Setup", "GroundShadow", iValue);

    iValue = m_engine->RetGroundSpot();
    SetLocalProfileInt("Setup", "GroundSpot", iValue);

    iValue = m_engine->RetDirty();
    SetLocalProfileInt("Setup", "ObjectDirty", iValue);

    iValue = m_engine->RetFog();
    SetLocalProfileInt("Setup", "FogMode", iValue);

    iValue = m_engine->RetLensMode();
    SetLocalProfileInt("Setup", "LensMode", iValue);

    iValue = m_engine->RetSkyMode();
    SetLocalProfileInt("Setup", "SkyMode", iValue);

    iValue = m_engine->RetPlanetMode();
    SetLocalProfileInt("Setup", "PlanetMode", iValue);

    iValue = m_engine->RetLightMode();
    SetLocalProfileInt("Setup", "LightMode", iValue);

    iValue = m_engine->RetJoystick();
    SetLocalProfileInt("Setup", "UseJoystick", iValue);

    fValue = m_engine->RetParticuleDensity();
    SetLocalProfileFloat("Setup", "ParticuleDensity", fValue);

    fValue = m_engine->RetClippingDistance();
    SetLocalProfileFloat("Setup", "ClippingDistance", fValue);

    fValue = m_engine->RetObjectDetail();
    SetLocalProfileFloat("Setup", "ObjectDetail", fValue);

    fValue = m_engine->RetGadgetQuantity();
    SetLocalProfileFloat("Setup", "GadgetQuantity", fValue);

    iValue = m_engine->RetTextureQuality();
    SetLocalProfileInt("Setup", "TextureQuality", iValue);

    iValue = m_sound->RetAudioVolume();
    SetLocalProfileInt("Setup", "AudioVolume", iValue);

    iValue = m_sound->RetMidiVolume();
    SetLocalProfileInt("Setup", "MidiVolume", iValue);

    iValue = m_sound->RetSound3D();
    SetLocalProfileInt("Setup", "Sound3D", iValue);

    iValue = m_engine->RetEditIndentMode();
    SetLocalProfileInt("Setup", "EditIndentMode", iValue);

    iValue = m_engine->RetEditIndentValue();
    SetLocalProfileInt("Setup", "EditIndentValue", iValue);

    key[0] = 0;
    for ( i=0 ; i<100 ; i++ )
    {
        if ( m_engine->RetKey(i, 0) == 0 )  break;

        for ( j=0 ; j<2 ; j++ )
        {
            iValue = m_engine->RetKey(i, j);
            sprintf(num, "%d%c", iValue, j==0?'+':' ');
            strcat(key, num);
        }
    }
    SetLocalProfileString("Setup", "KeyMap", key);

#if _NET
    if ( m_accessEnable )
    {
        iValue = m_accessMission;
        SetLocalProfileInt("Setup", "AccessMission", iValue);

        iValue = m_accessUser;
        SetLocalProfileInt("Setup", "AccessUser", iValue);
    }
#endif

    iValue = m_bDeleteGamer;
    SetLocalProfileInt("Setup", "DeleteGamer", iValue);

    m_engine->WriteProfile();
}

// Remember all the settings.

void CMainDialog::SetupRecall()
{
    float   fValue;
    int     iValue, i, j;
    char    key[500];
    char*   p;

    if ( GetLocalProfileString("Directory", "scene", key, _MAX_FNAME) )
    {
        strcpy(m_sceneDir, key);
    }

    if ( GetLocalProfileString("Directory", "savegame", key, _MAX_FNAME) )
    {
        strcpy(m_savegameDir, key);
    }

    if ( GetLocalProfileString("Directory", "public", key, _MAX_FNAME) )
    {
        strcpy(m_publicDir, key);
    }

    if ( GetLocalProfileString("Directory", "user", key, _MAX_FNAME) )
    {
        strcpy(m_userDir, key);
    }

    if ( GetLocalProfileString("Directory", "files", key, _MAX_FNAME) )
    {
        strcpy(m_filesDir, key);
    }


    if ( GetLocalProfileInt("Setup", "TotoMode", iValue) )
    {
        m_engine->SetTotoMode(iValue);
    }

    if ( GetLocalProfileInt("Setup", "Tooltips", iValue) )
    {
        m_bTooltip = iValue;
    }

    if ( GetLocalProfileInt("Setup", "InterfaceGlint", iValue) )
    {
        m_bGlint = iValue;
    }

    if ( GetLocalProfileInt("Setup", "InterfaceGlint", iValue) )
    {
        m_bRain = iValue;
    }

    if ( GetLocalProfileInt("Setup", "NiceMouse", iValue) )
    {
        m_engine->SetNiceMouse(iValue);
    }

    if ( GetLocalProfileInt("Setup", "Soluce4", iValue) )
    {
        m_bSoluce4 = iValue;
    }

    if ( GetLocalProfileInt("Setup", "Movies", iValue) )
    {
        m_bMovies = iValue;
    }

    if ( GetLocalProfileInt("Setup", "NiceReset", iValue) )
    {
        m_bNiceReset = iValue;
    }

    if ( GetLocalProfileInt("Setup", "HimselfDamage", iValue) )
    {
        m_bHimselfDamage = iValue;
    }

    if ( GetLocalProfileInt("Setup", "CameraScroll", iValue) )
    {
        m_bCameraScroll = iValue;
        m_camera->SetCameraScroll(m_bCameraScroll);
    }

    if ( GetLocalProfileInt("Setup", "CameraInvertX", iValue) )
    {
        m_bCameraInvertX = iValue;
        m_camera->SetCameraInvertX(m_bCameraInvertX);
    }

    if ( GetLocalProfileInt("Setup", "CameraInvertY", iValue) )
    {
        m_bCameraInvertY = iValue;
        m_camera->SetCameraInvertY(m_bCameraInvertY);
    }

    if ( GetLocalProfileInt("Setup", "InterfaceEffect", iValue) )
    {
        m_bEffect = iValue;
    }

    if ( GetLocalProfileInt("Setup", "GroundShadow", iValue) )
    {
        m_engine->SetShadow(iValue);
    }

    if ( GetLocalProfileInt("Setup", "GroundSpot", iValue) )
    {
        m_engine->SetGroundSpot(iValue);
    }

    if ( GetLocalProfileInt("Setup", "ObjectDirty", iValue) )
    {
        m_engine->SetDirty(iValue);
    }

    if ( GetLocalProfileInt("Setup", "FogMode", iValue) )
    {
        m_engine->SetFog(iValue);
        m_camera->SetOverBaseColor(RetColor(RetColor(0.0f)));
    }

    if ( GetLocalProfileInt("Setup", "LensMode", iValue) )
    {
        m_engine->SetLensMode(iValue);
    }

    if ( GetLocalProfileInt("Setup", "SkyMode", iValue) )
    {
        m_engine->SetSkyMode(iValue);
    }

    if ( GetLocalProfileInt("Setup", "PlanetMode", iValue) )
    {
        m_engine->SetPlanetMode(iValue);
    }

    if ( GetLocalProfileInt("Setup", "LightMode", iValue) )
    {
        m_engine->SetLightMode(iValue);
    }

    if ( GetLocalProfileInt("Setup", "UseJoystick", iValue) )
    {
        m_engine->SetJoystick(iValue);
    }

    if ( GetLocalProfileFloat("Setup", "ParticuleDensity", fValue) )
    {
        m_engine->SetParticuleDensity(fValue);
    }

    if ( GetLocalProfileFloat("Setup", "ClippingDistance", fValue) )
    {
        m_engine->SetClippingDistance(fValue);
    }

    if ( GetLocalProfileFloat("Setup", "ObjectDetail", fValue) )
    {
        m_engine->SetObjectDetail(fValue);
    }

    if ( GetLocalProfileFloat("Setup", "GadgetQuantity", fValue) )
    {
        m_engine->SetGadgetQuantity(fValue);
    }

    if ( GetLocalProfileInt("Setup", "TextureQuality", iValue) )
    {
        m_engine->SetTextureQuality(iValue);
    }

    if ( GetLocalProfileInt("Setup", "AudioVolume", iValue) )
    {
        m_sound->SetAudioVolume(iValue);
    }

    if ( GetLocalProfileInt("Setup", "MidiVolume", iValue) )
    {
        m_sound->SetMidiVolume(iValue);
    }

    if ( GetLocalProfileInt("Setup", "EditIndentMode", iValue) )
    {
        m_engine->SetEditIndentMode(iValue);
    }

    if ( GetLocalProfileInt("Setup", "EditIndentValue", iValue) )
    {
        m_engine->SetEditIndentValue(iValue);
    }

    if ( GetLocalProfileString("Setup", "KeyMap", key, 500) )
    {
        p = key;
        for ( i=0 ; i<100 ; i++ )
        {
            if ( p[0] == 0 )  break;

            for ( j=0 ; j<2 ; j++ )
            {
                sscanf(p, "%d", &iValue);
                m_engine->SetKey(i, j, iValue);
                while ( *p >= '0' && *p <= '9' )  p++;
                while ( *p == ' ' || *p == '+' )  p++;
            }
        }
    }

#if _NET
    if ( m_accessEnable )
    {
        if ( GetLocalProfileInt("Setup", "AccessMission", iValue) )
        {
            m_accessMission = iValue;
        }

        if ( GetLocalProfileInt("Setup", "AccessUser", iValue) )
        {
            m_accessUser = iValue;
        }
    }
#endif

    if ( GetLocalProfileInt("Setup", "DeleteGamer", iValue) )
    {
        m_bDeleteGamer = iValue;
    }
}


// Changes the general level of quality.

void CMainDialog::ChangeSetupQuality(int quality)
{
    BOOL    bEnable;
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
    m_camera->SetOverBaseColor(RetColor(RetColor(0.0f)));

    if ( quality <  0 )  value = 0.0f;
    if ( quality == 0 )  value = 1.0f;
    if ( quality >  0 )  value = 2.0f;
    m_engine->SetParticuleDensity(value);

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

    m_engine->FirstExecuteAdapt(FALSE);
}


// Redefinable keys:

static int key_table[KEY_TOTAL] =
{
#if _SCHOOL & _TEEN
    KEYRANK_LEFT,
    KEYRANK_RIGHT,
    KEYRANK_UP,
    KEYRANK_DOWN,
    KEYRANK_CAMERA,
    KEYRANK_NEAR,
    KEYRANK_AWAY,
    KEYRANK_HELP,
    KEYRANK_PROG,
    KEYRANK_SPEED10,
    KEYRANK_SPEED15,
    KEYRANK_SPEED20,
    KEYRANK_QUIT,
#else
    KEYRANK_LEFT,
    KEYRANK_RIGHT,
    KEYRANK_UP,
    KEYRANK_DOWN,
    KEYRANK_GUP,
    KEYRANK_GDOWN,
    KEYRANK_ACTION,
    KEYRANK_CAMERA,
    KEYRANK_VISIT,
    KEYRANK_NEXT,
    KEYRANK_HUMAN,
    KEYRANK_DESEL,
    KEYRANK_NEAR,
    KEYRANK_AWAY,
    KEYRANK_HELP,
    KEYRANK_PROG,
    KEYRANK_CBOT,
    KEYRANK_SPEED10,
    KEYRANK_SPEED15,
    KEYRANK_SPEED20,
    KEYRANK_QUIT,
#endif
};

static EventMsg key_event[KEY_TOTAL] =
{
#if _SCHOOL & _TEEN
    EVENT_INTERFACE_KLEFT,
    EVENT_INTERFACE_KRIGHT,
    EVENT_INTERFACE_KUP,
    EVENT_INTERFACE_KDOWN,
    EVENT_INTERFACE_KCAMERA,
    EVENT_INTERFACE_KNEAR,
    EVENT_INTERFACE_KAWAY,
    EVENT_INTERFACE_KHELP,
    EVENT_INTERFACE_KPROG,
    EVENT_INTERFACE_KSPEED10,
    EVENT_INTERFACE_KSPEED15,
    EVENT_INTERFACE_KSPEED20,
    EVENT_INTERFACE_KQUIT,
#else
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
#endif
};

// Updates the list of keys.

void CMainDialog::UpdateKey()
{
    CWindow*    pw;
    CScroll*    ps;
    CKey*       pk;
    FPOINT      pos, dim;
    int         first, i;

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW5);
    if ( pw == 0 )  return;

    ps = (CScroll*)pw->SearchControl(EVENT_INTERFACE_KSCROLL);
    if ( ps == 0 )  return;

    first = (int)(ps->RetVisibleValue()*(KEY_TOTAL-KEY_VISIBLE));

    for ( i=0 ; i<KEY_TOTAL ; i++ )
    {
        pw->DeleteControl(key_event[i]);
    }

    dim.x = 400.0f/640.0f;
    dim.y =  20.0f/480.0f;
    pos.x = 110.0f/640.0f;
    pos.y = 168.0f/480.0f + dim.y*(KEY_VISIBLE-1);
    for ( i=0 ; i<KEY_VISIBLE ; i++ )
    {
        pw->CreateKey(pos, dim, -1, key_event[first+i]);
        pk = (CKey*)pw->SearchControl(key_event[first+i]);
        if ( pk == 0 )  break;
        pk->SetKey(0, m_engine->RetKey(key_table[first+i], 0));
        pk->SetKey(1, m_engine->RetKey(key_table[first+i], 1));
        pos.y -= dim.y;
    }
}

// Change a key.

void CMainDialog::ChangeKey(EventMsg event)
{
    CWindow*    pw;
    CScroll*    ps;
    CKey*       pk;
    int         i;

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW5);
    if ( pw == 0 )  return;

    ps = (CScroll*)pw->SearchControl(EVENT_INTERFACE_KSCROLL);
    if ( ps == 0 )  return;

    for ( i=0 ; i<KEY_TOTAL ; i++ )
    {
        if ( key_event[i] == event )
        {
            pk = (CKey*)pw->SearchControl(key_event[i]);
            if ( pk == 0 )  break;
            m_engine->SetKey(key_table[i], 0, pk->RetKey(0));
            m_engine->SetKey(key_table[i], 1, pk->RetKey(1));
        }
    }
}



// Do you want to quit the current mission?

void CMainDialog::StartAbort()
{
    CWindow*    pw;
    CButton*    pb;
    FPOINT      pos, dim;
    char        name[100];

    StartDialog(FPOINT(0.3f, 0.8f), TRUE, FALSE, FALSE);
    m_bDialogDelete = FALSE;

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW9);
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
#if _POLISH
    pos.x -=  7.0f/640.0f;
    dim.x += 14.0f/640.0f;
#endif
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

    if ( m_engine->RetSetupMode() )
    {
        pos.y = 0.39f;
        pb = pw->CreateButton(pos, dim, -1, EVENT_INTERFACE_SETUP);
        pb->SetState(STATE_SHADOW);
    }

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
    FPOINT      pos, dim;
    char        name[100];

    StartDialog(FPOINT(0.7f, 0.3f), FALSE, TRUE, TRUE);
    m_bDialogDelete = TRUE;

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW9);
    if ( pw == 0 )  return;

    pos.x = 0.00f;
    pos.y = 0.50f;
    dim.x = 1.00f;
    dim.y = 0.05f;
    GetResource(RES_TEXT, RT_DIALOG_DELOBJ, name);
    pw->CreateLabel(pos, dim, -1, EVENT_DIALOG_LABEL, name);

    pb = (CButton*)pw->SearchControl(EVENT_DIALOG_OK);
    if ( pb == 0 )  return;
    GetResource(RES_TEXT, RT_DIALOG_YESDEL, name);
    pb->SetName(name);
    pb->SetState(STATE_WARNING);

    pb = (CButton*)pw->SearchControl(EVENT_DIALOG_CANCEL);
    if ( pb == 0 )  return;
    GetResource(RES_TEXT, RT_DIALOG_NODEL, name);
    pb->SetName(name);
}

// Do you want to delete the player?

void CMainDialog::StartDeleteGame(char *gamer)
{
    CWindow*    pw;
    CButton*    pb;
    FPOINT      pos, dim;
    char        name[100];
    char        text[100];

    StartDialog(FPOINT(0.7f, 0.3f), FALSE, TRUE, TRUE);
    m_bDialogDelete = TRUE;

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW9);
    if ( pw == 0 )  return;

    pos.x = 0.00f;
    pos.y = 0.50f;
    dim.x = 1.00f;
    dim.y = 0.05f;
    GetResource(RES_TEXT, RT_DIALOG_DELGAME, name);
    sprintf(text, name, gamer);
    pw->CreateLabel(pos, dim, -1, EVENT_DIALOG_LABEL, text);

    pb = (CButton*)pw->SearchControl(EVENT_DIALOG_OK);
    if ( pb == 0 )  return;
    GetResource(RES_TEXT, RT_DIALOG_YESDEL, name);
    pb->SetName(name);
    pb->SetState(STATE_WARNING);

    pb = (CButton*)pw->SearchControl(EVENT_DIALOG_CANCEL);
    if ( pb == 0 )  return;
    GetResource(RES_TEXT, RT_DIALOG_NODEL, name);
    pb->SetName(name);
}

// Would you quit the game?

void CMainDialog::StartQuit()
{
    CWindow*    pw;
    CButton*    pb;
    FPOINT      pos, dim;
    char        name[100];

    StartDialog(FPOINT(0.6f, 0.3f), FALSE, TRUE, TRUE);

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW9);
    if ( pw == 0 )  return;

    pos.x = 0.00f;
    pos.y = 0.50f;
    dim.x = 1.00f;
    dim.y = 0.05f;
    GetResource(RES_TEXT, RT_DIALOG_QUIT, name);
    pw->CreateLabel(pos, dim, -1, EVENT_DIALOG_LABEL, name);

    pb = (CButton*)pw->SearchControl(EVENT_DIALOG_OK);
    if ( pb == 0 )  return;
    GetResource(RES_TEXT, RT_DIALOG_YESQUIT, name);
    pb->SetName(name);
    pb->SetState(STATE_WARNING);

    pb = (CButton*)pw->SearchControl(EVENT_DIALOG_CANCEL);
    if ( pb == 0 )  return;
    GetResource(RES_TEXT, RT_DIALOG_NOQUIT, name);
    pb->SetName(name);
}

// Beginning of displaying a dialog.

void CMainDialog::StartDialog(FPOINT dim, BOOL bFire, BOOL bOK, BOOL bCancel)
{
    CWindow*    pw;
    CButton*    pb;
    FPOINT      pos, ddim;
    char        name[100];

    StartSuspend();

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW0);
    if ( pw != 0 )  pw->ClearState(STATE_ENABLE);

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW1);
    if ( pw != 0 )  pw->ClearState(STATE_ENABLE);

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW2);
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

    pb = (CButton*)m_interface->SearchControl(EVENT_BUTTON_QUIT);
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
    m_bDialog = TRUE;
}

// Animation of a dialog.

void CMainDialog::FrameDialog(float rTime)
{
    CWindow*    pw;
    D3DVECTOR   pos, speed;
    FPOINT      dim, dpos, ddim;
    float       zoom;
    int         i;

    dpos = m_dialogPos;
    ddim = m_dialogDim;

    m_dialogTime += rTime;
    if ( m_dialogTime < 1.0f )
    {
        pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW9);
        if ( pw != 0 )
        {
            if ( m_dialogTime < 0.50f )
            {
                zoom = Bounce(m_dialogTime/0.50f);
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
    if ( m_dialogParti < m_engine->ParticuleAdapt(0.05f) )  return;
    m_dialogParti = 0.0f;

    if ( !m_bDialogFire )  return;

    dpos = m_dialogPos;
    ddim = m_dialogDim;

    pos.z = 0.0f;
    speed = D3DVECTOR(0.0f, 0.0f, 0.0f);

    for ( i=0 ; i<2 ; i++ )
    {
        // Bottom.
        pos.x = dpos.x + ddim.x*Rand();
        pos.y = dpos.y;
        pos.x += (Rand()-0.5f)*(6.0f/640.0f);
        pos.y += Rand()*(16.0f/480.0f)-(10.0f/480.0f);
        dim.x = 0.01f+Rand()*0.01f;
        dim.y = dim.x/0.75f;
        m_particule->CreateParticule(pos, speed, dim,
                                     (ParticuleType)(PARTILENS1+rand()%3),
                                     1.0f, 0.0f, 0.0f, SH_INTERFACE);

        // Top.
        pos.x = dpos.x + ddim.x*Rand();
        pos.y = dpos.y + ddim.y;
        pos.x += (Rand()-0.5f)*(6.0f/640.0f);
        pos.y -= Rand()*(16.0f/480.0f)-(10.0f/480.0f);
        dim.x = 0.01f+Rand()*0.01f;
        dim.y = dim.x/0.75f;
        m_particule->CreateParticule(pos, speed, dim,
                                     (ParticuleType)(PARTILENS1+rand()%3),
                                     1.0f, 0.0f, 0.0f, SH_INTERFACE);

        // Left.
        pos.y = dpos.y + ddim.y*Rand();
        pos.x = dpos.x;
        pos.x += Rand()*(16.0f/640.0f)-(10.0f/640.0f);
        pos.y += (Rand()-0.5f)*(6.0f/480.0f);
        dim.x = 0.01f+Rand()*0.01f;
        dim.y = dim.x/0.75f;
        m_particule->CreateParticule(pos, speed, dim,
                                     (ParticuleType)(PARTILENS1+rand()%3),
                                     1.0f, 0.0f, 0.0f, SH_INTERFACE);

        // Right.
        pos.y = dpos.y + ddim.y*Rand();
        pos.x = dpos.x + ddim.x;
        pos.x -= Rand()*(16.0f/640.0f)-(10.0f/640.0f);
        pos.y += (Rand()-0.5f)*(6.0f/480.0f);
        dim.x = 0.01f+Rand()*0.01f;
        dim.y = dim.x/0.75f;
        m_particule->CreateParticule(pos, speed, dim,
                                     (ParticuleType)(PARTILENS1+rand()%3),
                                     1.0f, 0.0f, 0.0f, SH_INTERFACE);
    }
}

// End of the display of a dialogue.

void CMainDialog::StopDialog()
{
    CWindow*    pw;
    CButton*    pb;

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW0);
    if ( pw != 0 )  pw->SetState(STATE_ENABLE);

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW1);
    if ( pw != 0 )  pw->SetState(STATE_ENABLE);

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW2);
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

    pb = (CButton*)m_interface->SearchControl(EVENT_BUTTON_QUIT);
    if ( pb != 0 )
    {
        pb->SetState(STATE_VISIBLE);
    }

    StopSuspend();
    m_interface->DeleteControl(EVENT_WINDOW9);
    m_bDialog = FALSE;
}

// Suspends the simulation for a dialog phase.

void CMainDialog::StartSuspend()
{
    m_sound->MuteAll(TRUE);
    m_main->ClearInterface();
    m_bInitPause = m_engine->RetPause();
    m_engine->SetPause(TRUE);
    m_engine->SetOverFront(FALSE);  // over flat behind
    m_main->CreateShortcuts();
    m_main->StartSuspend();
    m_initCamera = m_camera->RetType();
    m_camera->SetType(CAMERA_DIALOG);
}

// Resume the simulation after a period of dialog.

void CMainDialog::StopSuspend()
{
    m_sound->MuteAll(FALSE);
    m_main->ClearInterface();
    if ( !m_bInitPause )  m_engine->SetPause(FALSE);
    m_engine->SetOverFront(TRUE);  // over flat front
    m_main->CreateShortcuts();
    m_main->StopSuspend();
    m_camera->SetType(m_initCamera);
}


// Whether to use tooltips.

BOOL CMainDialog::RetTooltip()
{
    return m_bTooltip;
}

// Specifies whether a dialog is displayed.

BOOL CMainDialog::IsDialog()
{
    return m_bDialog;
}




// Specifies the name of the scene to read.

void CMainDialog::SetSceneRead(char* name)
{
    strcpy(m_sceneRead, name);
}

// Returns the name of the scene to read.

char* CMainDialog::RetSceneRead()
{
    return m_sceneRead;
}

// Specifies the name of the scene to read.

void CMainDialog::SetStackRead(char* name)
{
    strcpy(m_stackRead, name);
}

// Returns the name of the scene to read.

char* CMainDialog::RetStackRead()
{
    return m_stackRead;
}

// Specifies the name of the chosen to play scene.

void CMainDialog::SetSceneName(char* name)
{
    strcpy(m_sceneName, name);
}

// Returns the name of the chosen to play scene.

char* CMainDialog::RetSceneName()
{
    return m_sceneName;
}

// Specifies the rank of the chosen to play scene.

void CMainDialog::SetSceneRank(int rank)
{
    m_sceneRank = rank;
}

// Returns the rank of the chosen to play scene.

int CMainDialog::RetSceneRank()
{
    return m_sceneRank;
}

// Returns folder name of the scene that user selected to play.

char* CMainDialog::RetSceneDir()
{
    int     i;

    i = (m_sceneRank/100)-1;

    if ( i < 0 || i >= m_userTotal )  return 0;
    return m_userList[i];
}

// Whether to show the solution.

BOOL CMainDialog::RetSceneSoluce()
{
    return m_bSceneSoluce;
}

// Returns the name of the folder to save.

char* CMainDialog::RetSavegameDir()
{
    return m_savegameDir;
}

// Returns the name of public folder.

char* CMainDialog::RetPublicDir()
{
    return m_publicDir;
}


// Indicates if there are reflections on the buttons.

BOOL CMainDialog::RetGlint()
{
    return m_bGlint;
}

// Whether to show 4:solutions.

BOOL CMainDialog::RetSoluce4()
{
    return m_bSoluce4;
}

// Whether to show the cinematics.

BOOL CMainDialog::RetMovies()
{
    return m_bMovies;
}

// IWhether to make an animation in CTaskReset.

BOOL CMainDialog::RetNiceReset()
{
    return m_bNiceReset;
}

// Indicates whether the fire causes damage to its own units.

BOOL CMainDialog::RetHimselfDamage()
{
    return m_bHimselfDamage;
}



// Saves the personalized player.

void CMainDialog::WriteGamerPerso(char *gamer)
{
    FILE*   file;
    char    filename[100];
    char    line[100];

    sprintf(filename, "%s\\%s\\face.gam", m_savegameDir, gamer);
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
    D3DCOLORVALUE   color;

    m_perso.face = 0;
    DefPerso();

    sprintf(filename, "%s\\%s\\face.gam", m_savegameDir, gamer);
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
            m_perso.colorHair = OpColorValue(line, "hair", color);
        }

        if ( Cmd(line, "Body") )
        {
            color.r = 0.0f;
            color.g = 0.0f;
            color.b = 0.0f;
            color.a = 0.0f;
            m_perso.colorCombi = OpColorValue(line, "combi", color);

            color.r = 0.0f;
            color.g = 0.0f;
            color.b = 0.0f;
            color.a = 0.0f;
            m_perso.colorBand = OpColorValue(line, "band", color);
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

int CMainDialog::RetGamerFace(char *gamer)
{
    ReadGamerPerso(gamer);
    return m_perso.face;
}

// Gives the face of the player.

int CMainDialog::RetGamerFace()
{
    return m_perso.face;
}

int CMainDialog::RetGamerGlasses()
{
    return m_perso.glasses;
}

BOOL CMainDialog::RetGamerOnlyHead()
{
    return (m_phase == PHASE_PERSO && m_persoTab == 0);
}

float CMainDialog::RetPersoAngle()
{
    return m_persoAngle;
}

D3DCOLORVALUE CMainDialog::RetGamerColorHair()
{
    return m_perso.colorHair;
}

D3DCOLORVALUE CMainDialog::RetGamerColorCombi()
{
    return m_perso.colorCombi;
}

D3DCOLORVALUE CMainDialog::RetGamerColorBand()
{
    return m_perso.colorBand;
}


// Reads the file of the player.

BOOL CMainDialog::ReadGamerInfo()
{
    FILE*   file;
    char    line[100];
    int     chap, i, numTry, passed;

    for ( i=0 ; i<MAXSCENE ; i++ )
    {
        m_sceneInfo[i].numTry = 0;
        m_sceneInfo[i].bPassed = FALSE;
    }

    sprintf(line, "%s\\%s\\%s.gam", m_savegameDir, m_main->RetGamerName(), m_sceneName);
    file = fopen(line, "r");
    if ( file == NULL )  return FALSE;

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
    return TRUE;
}

// Writes the file of the player.

BOOL CMainDialog::WriteGamerInfo()
{
    FILE*   file;
    char    line[100];
    int     i;

    sprintf(line, "%s\\%s\\%s.gam", m_savegameDir, m_main->RetGamerName(), m_sceneName);
    file = fopen(line, "w");
    if ( file == NULL )  return FALSE;

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
    return TRUE;
}

void CMainDialog::SetGamerInfoTry(int rank, int numTry)
{
    if ( rank < 0 || rank >= MAXSCENE )  return;
    if ( numTry > 100 )  numTry = 100;
    m_sceneInfo[rank].numTry = numTry;
}

int CMainDialog::RetGamerInfoTry(int rank)
{
    if ( rank < 0 || rank >= MAXSCENE )  return 0;
    return m_sceneInfo[rank].numTry;
}

void CMainDialog::SetGamerInfoPassed(int rank, BOOL bPassed)
{
    int     chap, i;
    BOOL    bAll;

    if ( rank < 0 || rank >= MAXSCENE )  return;
    m_sceneInfo[rank].bPassed = bPassed;

    if ( bPassed )
    {
        bAll = TRUE;
        chap = rank/100;
        for ( i=0 ; i<m_maxList ; i++ )
        {
            bAll &= m_sceneInfo[chap*100+i+1].bPassed;
        }
        m_sceneInfo[chap*100].numTry ++;
        m_sceneInfo[chap*100].bPassed = bAll;
    }
}

BOOL CMainDialog::RetGamerInfoPassed(int rank)
{
    if ( rank < 0 || rank >= MAXSCENE )  return FALSE;
    return m_sceneInfo[rank].bPassed;
}


// Passes to the next mission, and possibly in the next chapter.

BOOL CMainDialog::NextMission()
{
    m_sel[m_index] ++;  // next mission

    if ( m_sel[m_index] >= m_maxList )  // last mission of the chapter?
    {
        m_chap[m_index] ++;  // next chapter
        m_sel[m_index] = 0;  // first mission
    }

    return TRUE;
}


