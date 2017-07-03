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

#include "ui/screen/buzzingcars/screen_car.h"

#include "app/app.h"

#include "common/restext.h"
#include "common/stringutils.h"

#include "graphics/engine/camera.h"
#include "graphics/engine/pyro.h"
#include "graphics/engine/pyro_manager.h"

#include "level/player_profile.h"
#include "level/robotmain.h"

#include "math/geometry.h"

#include "object/object_manager.h"
#include "object/old_object.h"

#include "object/motion/motion.h"

#include "physics/physics.h"

#include "sound/sound.h"

#include "ui/controls/button.h"
#include "ui/controls/color.h"
#include "ui/controls/check.h"
#include "ui/controls/gauge.h"
#include "ui/controls/group.h"
#include "ui/controls/interface.h"
#include "ui/controls/label.h"
#include "ui/controls/slider.h"
#include "ui/controls/window.h"

// TODO (krzys_h): Clean this up. A lot.

namespace Ui
{

static int perso_color[3*15] =
{
    255, 209,  67,  // jaune-orange (car01)
     72, 153, 236,  // bleu (car03)
    196,  55,  61,  // rouge (car04)
     64,  64,  64,  // gris-noir (car05)
    143,  88, 224,  // violet (car02)
    230, 251, 252,  // blanc-cyan
     93,  13,  13,  // brun
    238, 132, 214,  // rose (car06)
    255, 136,  39,  // orange
     20, 192,  20,  // vert
    255, 255,   0,  // jaune (car08)
      0,  53, 226,  // bleu royal
     89,  34, 172,  // violet foncé (car07)
    126, 186, 244,  // cyan
    149, 149, 149,  // gris
};

CScreenCar::CScreenCar()
{
    FlushPerso();

    // TODO (krzys_h): Just unlock everything for now
    BuyAllPerso();
}

void CScreenCar::CreateInterface()
{
    Ui::CWindow*		pw;
    Ui::CEdit*			pe;
    Ui::CLabel*			pl;
    Ui::CList*			pli;
//    Ui::CArray*			pa;
    Ui::CCheck*			pc;
    Ui::CScroll*		ps;
    Ui::CSlider*		psl;
    Ui::CButton*		pb;
    Ui::CColor*			pco;
    Ui::CGauge*			pgg;
//    Ui::CPesetas*		pp;
    Ui::CGroup*			pg;
    Ui::CImage*			pi;
    Math::Point			pos, dim, ddim;
    float			ox, oy, sx, sy;
    std::string			name;
    int				res, i;

    pos.x  = 0.10f;
    pos.y  = 0.10f;
    ddim.x = 0.80f;
    ddim.y = 0.80f;
    pw = m_interface->CreateWindows(pos, ddim, 15, EVENT_WINDOW4);
    pw->SetClosable(true);
    GetResource(RES_TEXT, RT_TITLE_CAR, name);
    pw->SetName(name);

    pos.x  = 0.10f;
    pos.y  = 0.44f;
    ddim.x = 0.50f;
    ddim.y = 0.50f;
    pw->CreateGroup(pos, ddim, 5, EVENT_INTERFACE_GLINTl);  // coin orange
    pos.x  = 0.40f;
    pos.y  = 0.06f;
    ddim.x = 0.50f;
    ddim.y = 0.50f;
    pw->CreateGroup(pos, ddim, 4, EVENT_INTERFACE_GLINTr);  // coin bleu

    pos.x  =  74.0f/640.0f;
    pos.y  = 370.0f/480.0f;
    ddim.x = 108.0f/640.0f;
    ddim.y =  20.0f/480.0f;
    GetResource(RES_EVENT, EVENT_INTERFACE_PGSPEED, name);
    pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL1, name);
    pos.y -=  10.0f/480.0f;
    pgg = pw->CreateGauge(pos, ddim, 0, EVENT_INTERFACE_PGSPEED);
    pgg->SetState(STATE_SHADOW);
    pos.y -=  28.0f/480.0f;
    pl = pw->CreateLabel(pos, ddim, 0, EVENT_INTERFACE_PLSPEED, "");

    pos.x  = 202.0f/640.0f;
    pos.y  = 370.0f/480.0f;
    ddim.x = 108.0f/640.0f;
    ddim.y =  20.0f/480.0f;
    GetResource(RES_EVENT, EVENT_INTERFACE_PGACCEL, name);
    pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL2, name);
    pos.y -=  10.0f/480.0f;
    pgg = pw->CreateGauge(pos, ddim, 0, EVENT_INTERFACE_PGACCEL);
    pgg->SetState(STATE_SHADOW);
    pos.y -=  28.0f/480.0f;
    pl = pw->CreateLabel(pos, ddim, 0, EVENT_INTERFACE_PLACCEL, "");

    pos.x  = 330.0f/640.0f;
    pos.y  = 370.0f/480.0f;
    ddim.x = 108.0f/640.0f;
    ddim.y =  20.0f/480.0f;
    GetResource(RES_EVENT, EVENT_INTERFACE_PGGRIP, name);
    pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL3, name);
    pos.y -=  10.0f/480.0f;
    pgg = pw->CreateGauge(pos, ddim, 0, EVENT_INTERFACE_PGGRIP);
    pgg->SetState(STATE_SHADOW);
    pos.y -=  28.0f/480.0f;
    pl = pw->CreateLabel(pos, ddim, 0, EVENT_INTERFACE_PLGRIP, "");

    pos.x  = 458.0f/640.0f;
    pos.y  = 370.0f/480.0f;
    ddim.x = 108.0f/640.0f;
    ddim.y =  20.0f/480.0f;
    GetResource(RES_EVENT, EVENT_INTERFACE_PGSOLID, name);
    pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL4, name);
    pos.y -=  10.0f/480.0f;
    pgg = pw->CreateGauge(pos, ddim, 0, EVENT_INTERFACE_PGSOLID);
    pgg->SetState(STATE_SHADOW);
    pos.y -=  28.0f/480.0f;
    pl = pw->CreateLabel(pos, ddim, 0, EVENT_INTERFACE_PLSOLID, "");

    m_persoElevation = -0.1f;
    pos.x  =  74.0f/640.0f;
    pos.y  = 239.0f/480.0f;
    ddim.x =  16.0f/640.0f;
    ddim.y = 100.0f/480.0f;
    psl = pw->CreateSlider(pos, ddim, 0, EVENT_INTERFACE_PELEV);
    psl->SetState(STATE_SHADOW);
    psl->SetLimit(-0.7f, 0.3f);
    psl->SetArrowStep(0.1f);
    psl->SetVisibleValue(m_persoElevation);
//TODO (krzys_h):    psl->SetTabOrder(17);

    EventType table[] =
    {
        EVENT_INTERFACE_PCOLOR5,  // blanc
        EVENT_INTERFACE_PCOLOR10, // jaune
        EVENT_INTERFACE_PCOLOR0,  // jaune-orange
        EVENT_INTERFACE_PCOLOR8,  // orange
        EVENT_INTERFACE_PCOLOR2,  // rouge
        EVENT_INTERFACE_PCOLOR6,  // brun
        EVENT_INTERFACE_PCOLOR3,  // noir
        EVENT_INTERFACE_PCOLOR12, // violet foncé
        EVENT_INTERFACE_PCOLOR4,  // violet
        EVENT_INTERFACE_PCOLOR11, // bleu royal
        EVENT_INTERFACE_PCOLOR1,  // bleu
        EVENT_INTERFACE_PCOLOR13, // cyan
        EVENT_INTERFACE_PCOLOR14, // gris
        EVENT_INTERFACE_PCOLOR9,  // vert
        EVENT_INTERFACE_PCOLOR7,  // rose
    };

    pos.x  = 550.0f/640.0f;
    pos.y  = 324.0f/480.0f;
    ddim.x =  15.0f/640.0f;
    ddim.y =  15.0f/480.0f;
    for ( i=0 ; i<15 ; i++ )
    {
        pco = pw->CreateColor(pos, ddim, -1, table[i]);
        pco->SetState(STATE_SHADOW);
//TODO (krzys_h):        pco->SetTabOrder(2+i);
        pos.y -= 15.0f/480.0f;
    }

    pos.x  =  74.0f/640.0f;
    pos.y  = 170.0f/480.0f;
    ddim.x =  16.0f/640.0f;
    ddim.y =  16.0f/480.0f;
    pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_PSUBMOD0);
    pb->SetFontSize(9.0f);
    pb->SetState(STATE_SHADOW);
//TODO (krzys_h):    pb->SetTabOrder(18);
    pos.y -= 18.0f/480.0f;
    pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_PSUBMOD1);
    pb->SetFontSize(9.0f);
    pb->SetState(STATE_SHADOW);
//TODO (krzys_h):    pb->SetTabOrder(19);
    pos.y -= 18.0f/480.0f;
    pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_PSUBMOD2);
    pb->SetFontSize(9.0f);
    pb->SetState(STATE_SHADOW);
//TODO (krzys_h):    pb->SetTabOrder(20);
    pos.y -= 18.0f/480.0f;
    pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_PSUBMOD3);
    pb->SetFontSize(9.0f);
    pb->SetState(STATE_SHADOW);
//TODO (krzys_h):    pb->SetTabOrder(21);

#if _DEMO|_SE
    pos.x  =  94.0f/640.0f;
		pos.y  = 298.0f/480.0f;
		ddim.x = 452.0f/640.0f;
		ddim.y =  44.0f/480.0f;
		pg = pw->CreateGroup(pos, ddim, 29, EVENT_INTERFACE_PSPECB);
#endif

    pos.x  =  94.0f/640.0f;
    pos.y  = 308.0f/480.0f;
    ddim.x = 452.0f/640.0f;
    ddim.y =  20.0f/480.0f;
    pl = pw->CreateLabel(pos, ddim, 0, EVENT_INTERFACE_PPRICE, "");
    pl->SetFontSize(13.0f);

    pos.x  =  94.0f/640.0f;
    pos.y  = 294.0f/480.0f;
    ddim.x = 452.0f/640.0f;
    ddim.y =  20.0f/480.0f;
    pl = pw->CreateLabel(pos, ddim, 0, EVENT_INTERFACE_PPESETAS, "");
    pl->SetFontSize(10.0f);

#if _DEMO|_SE
    pos.x  =  94.0f/640.0f;
		pos.y  = 292.0f/480.0f;
		ddim.x = 452.0f/640.0f;
		ddim.y =  20.0f/480.0f;
		pl = pw->CreateLabel(pos, ddim, 0, EVENT_INTERFACE_PSPECT, "");
		pl->SetFontSize(10.0f);
#else
    pos.x  = 150.0f/640.0f;
    pos.y  = 230.0f/480.0f;
    ddim.x = 340.0f/640.0f;
    ddim.y =  30.0f/480.0f;
    pg = pw->CreateGroup(pos, ddim, 9, EVENT_INTERFACE_PSPECB);
    pg->SetState(STATE_SHADOW);
    pos.x  = 150.0f/640.0f;
    pos.y  = 222.0f/480.0f;
    ddim.x = 340.0f/640.0f;
    ddim.y =  20.0f/480.0f;
    pl = pw->CreateLabel(pos, ddim, 0, EVENT_INTERFACE_PSPECT, "");
    pl->SetFontSize(15.0f);
#endif

    pos.x  =  94.0f/640.0f;
    pos.y  =  74.0f/480.0f;
    ddim.x =  60.0f/640.0f;
    ddim.y =  32.0f/480.0f;
    pb = pw->CreateButton(pos, ddim, 55, EVENT_INTERFACE_PPREV);
    pb->SetState(STATE_SHADOW);
//TODO (krzys_h):    pb->SetTabOrder(22);

    pos.x  = 164.0f/640.0f;
    pos.y  =  74.0f/480.0f;
    ddim.x =  60.0f/640.0f;
    ddim.y =  32.0f/480.0f;
    pb = pw->CreateButton(pos, ddim, 48, EVENT_INTERFACE_PNEXT);
    pb->SetState(STATE_SHADOW);
//TODO (krzys_h):    pb->SetTabOrder(23);

    pos.x  = 336.0f/640.0f;
    pos.y  =  74.0f/480.0f;
    ddim.x = 210.0f/640.0f;
    ddim.y =  32.0f/480.0f;
    pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_PCAROK);
    pb->SetFontType(Gfx::FONT_BOLD);
    pb->SetState(STATE_SHADOW);
//TODO (krzys_h):    pb->SetTabOrder(0);
//TODO (krzys_h):    pb->SetFocus(true);

    /* TODO (krzys_h):
    pos.x  = 332.0f/640.0f;
    pos.y  =  38.0f/480.0f;
    ddim.x = 218.0f/640.0f;
    ddim.y =  26.0f/480.0f;
    pg = pw->CreateGroup(pos, ddim, 27, EVENT_INTERFACE_GHOSTg);
    pg->SetState(STATE_SHADOW);

    pos.x  = 336.0f/640.0f;
    pos.y  =  43.0f/480.0f;
    ddim.x = 210.0f/640.0f;
    ddim.y =  16.0f/480.0f;
    pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_GHOSTm);
    pc->SetFontSize(9.5f);
    pc->SetState(STATE_SHADOW);
//TODO (krzys_h):    pc->SetTabOrder(1);
//TODO (krzys_h):    UpdateSceneGhost(m_sel[m_index]);
     */

//TODO (krzys_h):    m_persoCopy = m_perso;  // copie si annulation
    m_persoAngle = -0.6f;
    m_persoTime = 0.0f;
    m_persoRun = false;
    BuyablePerso();
    m_main->ScenePerso("levels/other/car.txt");
    UpdatePerso();
    CameraPerso();

//TODO (krzys_h):    m_ghostName[0] = 0;  // utilise la voiture fantome standard
}

bool CScreenCar::EventProcess(const Event &event)
{
    COldObject*	vehicle;
    CPhysics*	physics;
    Math::Vector	pos;

    switch( event.type )
    {
        case EVENT_FRAME:
            vehicle = dynamic_cast<COldObject*>(CObjectManager::GetInstancePointer()->FindNearest(nullptr, OBJECT_CAR));
            if ( vehicle != 0 )
            {
                if ( m_persoTime == 0.0f )
                {
                    pos = vehicle->GetPosition();
                    pos.y += 10.0f;
                    vehicle->SetPosition(pos);
                }

                physics = vehicle->GetPhysics();
                if ( physics != 0 )
                {
                    m_persoTime += event.rTime;
                    m_persoAngle += event.rTime*0.5f;
                    CameraPerso();

                    if ( m_persoTime > 2.5f && !m_persoRun )
                    {
                        m_persoRun = true;
                        physics->SetForceSlow(true);  // moteur au ralenti
                    }
                    if ( m_persoTime > 4.0f )
                    {
                        physics->ForceMotorSpeedZ(sinf((m_persoTime-4.0f)*3.0f));
                    }
                }
            }
            return true;
            
        case EVENT_KEY_DOWN:
            if ( event.GetData<KeyEventData>()->key == KEY(ESCAPE)  ||
                 event.GetData<KeyEventData>()->key == VIRTUAL_JOY(9) )
            {
//TODO (krzys_h):                m_perso = m_persoCopy;
//TODO (krzys_h):                WriteGamerInfo();
//TODO (krzys_h):                m_main->ChangePhase(m_phasePerso);
                m_main->ChangePhase(PHASE_LEVEL_LIST);
                return false;
            }
            break;

        case EVENT_INTERFACE_PELEV:
            ElevationPerso();
            break;

        case EVENT_INTERFACE_PCOLOR0:
        case EVENT_INTERFACE_PCOLOR1:
        case EVENT_INTERFACE_PCOLOR2:
        case EVENT_INTERFACE_PCOLOR3:
        case EVENT_INTERFACE_PCOLOR4:
        case EVENT_INTERFACE_PCOLOR5:
        case EVENT_INTERFACE_PCOLOR6:
        case EVENT_INTERFACE_PCOLOR7:
        case EVENT_INTERFACE_PCOLOR8:
        case EVENT_INTERFACE_PCOLOR9:
        case EVENT_INTERFACE_PCOLOR10:
        case EVENT_INTERFACE_PCOLOR11:
        case EVENT_INTERFACE_PCOLOR12:
        case EVENT_INTERFACE_PCOLOR13:
        case EVENT_INTERFACE_PCOLOR14:
        case EVENT_INTERFACE_PCOLOR15:
        case EVENT_INTERFACE_PCOLOR16:
        case EVENT_INTERFACE_PCOLOR17:
        case EVENT_INTERFACE_PCOLOR18:
        case EVENT_INTERFACE_PCOLOR19:
            FixPerso(event.type-EVENT_INTERFACE_PCOLOR0);
//TODO (krzys_h):            WriteGamerInfo();
            m_main->ScenePerso("levels/other/car.txt");
            UpdatePerso();
            m_persoTime = 0.01f;
            m_persoRun = false;
            vehicle = dynamic_cast<COldObject*>(CObjectManager::GetInstancePointer()->FindNearest(nullptr, OBJECT_CAR));
            if ( vehicle != 0 )
            {
                m_sound->Play(SOUND_FINDING);
                m_engine->GetPyroManager()->Create(Gfx::PT_PAINTING, vehicle, 0.0f);
            }
            break;

        case EVENT_INTERFACE_PSUBMOD0:
        case EVENT_INTERFACE_PSUBMOD1:
        case EVENT_INTERFACE_PSUBMOD2:
        case EVENT_INTERFACE_PSUBMOD3:
            m_perso.subModel[m_perso.selectCar] = event.type-EVENT_INTERFACE_PSUBMOD0;
            DefPerso(m_perso.selectCar);  // met la couleur standard
//TODO (krzys_h):            WriteGamerInfo();
            m_main->ScenePerso("levels/other/car.txt");
            UpdatePerso();
            m_persoTime = 0.01f;
            m_persoRun = false;
            vehicle = dynamic_cast<COldObject*>(CObjectManager::GetInstancePointer()->FindNearest(nullptr, OBJECT_CAR));
            if ( vehicle != 0 )
            {
                m_sound->Play(SOUND_FINDING);
                m_engine->GetPyroManager()->Create(Gfx::PT_PAINTING, vehicle, 0.0f);
            }
            break;

        /* TODO (krzys_h)
        case EVENT_INTERFACE_GHOSTm:
            m_bGhostEnable = !m_bGhostEnable;
//TODO (krzys_h):            UpdateSceneGhost(m_sel[m_index]);
            break;
        */

        case EVENT_INTERFACE_PPREV:
            if ( m_perso.selectCar > 0 )
            {
                NextPerso(-1);
                m_main->ScenePerso("levels/other/car.txt");
                UpdatePerso();
//TODO (krzys_h):                UpdateSceneGhost(m_sel[m_index]);
                m_persoTime = 0.0f;
                m_persoRun = false;
            }
            break;
        case EVENT_INTERFACE_PNEXT:
#if _SE
            if ( m_perso.selectCar < m_perso.total-1 )
#else
            if ( m_perso.selectCar < m_perso.buyable-1 ||
                 m_perso.selectCar < m_perso.bonus-1   )
#endif
            {
                NextPerso(1);
                m_main->ScenePerso("levels/other/car.txt");
                UpdatePerso();
//TODO (krzys_h):                UpdateSceneGhost(m_sel[m_index]);
                m_persoTime = 0.0f;
                m_persoRun = false;
            }
            break;

        case EVENT_INTERFACE_PCAROK:
            SelectPerso();
//TODO (krzys_h):            m_bDuel = false;
            LaunchSimul();
            break;

        default:
            return true;
    }
    return false;
}

// Tests whether two colors are equal or nearly are.

namespace
{
bool EqColor(const Gfx::Color &c1, const Gfx::Color &c2)
{
    return (fabs(c1.r-c2.r) < 0.01f &&
            fabs(c1.g-c2.g) < 0.01f &&
            fabs(c1.b-c2.b) < 0.01f );
}
}


// Met à jour tous les boutons pour le personnage.

void CScreenCar::UpdatePerso()
{
    COldObject*		vehicle;
    CPhysics*		physics;
    CMotion*		motion;
    Character*		character;
    Ui::CWindow*		pw;
    Ui::CButton*		pb;
    Ui::CColor*			pco;
    Ui::CGauge*			pg;
    Ui::CGroup*			pgr;
    Ui::CLabel*			pl;
    Gfx::Color	color;
    CarSpec			missionSpec, carSpec;
    std::string text, res, car;
    int				i, icon = 0;
    float			value;
    bool			bOK;

    vehicle = dynamic_cast<COldObject*>(CObjectManager::GetInstancePointer()->FindNearest(nullptr, OBJECT_CAR));
    if ( vehicle == 0 )  return;
    physics = vehicle->GetPhysics();
    if ( physics == 0 )  return;
    motion = vehicle->GetMotion();
    if ( motion == 0 )  return;
    character = vehicle->GetCharacter();

    pw = (Ui::CWindow*)m_interface->SearchControl(EVENT_WINDOW4);
    if ( pw == 0 )  return;

    pg = (Ui::CGauge*)pw->SearchControl(EVENT_INTERFACE_PGSPEED);
    if ( pg == 0 )  return;
    value = physics->GetLinMotionX(MO_ADVSPEED);
    pg->SetLevel(value/180.0f);
    pl = (Ui::CLabel*)pw->SearchControl(EVENT_INTERFACE_PLSPEED);
    if ( pl == 0 )  return;
#if _MPH
    value *= 0.6f;  // approximatif: 100 KPH = 60 MPH !
#endif
    text = StrUtils::ToString<int>(value);
    pl->SetName(text);

    pg = (Ui::CGauge*)pw->SearchControl(EVENT_INTERFACE_PGACCEL);
    if ( pg == 0 )  return;
    value = physics->GetLinMotionX(MO_ADVACCEL);
    pg->SetLevel(value/100.0f);
    pl = (Ui::CLabel*)pw->SearchControl(EVENT_INTERFACE_PLACCEL);
    if ( pl == 0 )  return;
    text = StrUtils::ToString<int>(value);
    pl->SetName(text);

    pg = (Ui::CGauge*)pw->SearchControl(EVENT_INTERFACE_PGGRIP);
    if ( pg == 0 )  return;
    value = 100.0f-character->gripSlide;
    pg->SetLevel(value/50.0f);
    pl = (Ui::CLabel*)pw->SearchControl(EVENT_INTERFACE_PLGRIP);
    if ( pl == 0 )  return;
    text = StrUtils::ToString<int>(value);
    pl->SetName(text);

    pg = (Ui::CGauge*)pw->SearchControl(EVENT_INTERFACE_PGSOLID);
    if ( pg == 0 )  return;
    value = (float)motion->GetUsedPart();
    pg->SetLevel(value/30.0f);
    pl = (Ui::CLabel*)pw->SearchControl(EVENT_INTERFACE_PLSOLID);
    if ( pl == 0 )  return;
    text = StrUtils::ToString<int>(value);
    pl->SetName(text);

    pb = (Ui::CButton*)pw->SearchControl(EVENT_INTERFACE_PPREV);
    if ( pb == 0 )  return;
    pb->SetState(STATE_ENABLE, m_perso.selectCar > 0);

    pb = (Ui::CButton*)pw->SearchControl(EVENT_INTERFACE_PNEXT);
    if ( pb == 0 )  return;
#if !_SE
    pb->SetState(STATE_ENABLE, m_perso.selectCar < m_perso.buyable-1 ||
                               m_perso.selectCar < m_perso.bonus-1   );
#endif

    for ( i=0 ; i<15 ; i++ )
    {
        pco = (Ui::CColor*)pw->SearchControl((EventType)(EVENT_INTERFACE_PCOLOR0+i));
        if ( pco == 0 )  break;
        pco->SetState(STATE_VISIBLE);
        color.r = perso_color[3*i+0]/255.0f;
        color.g = perso_color[3*i+1]/255.0f;
        color.b = perso_color[3*i+2]/255.0f;
        color.a = 0.0f;
        pco->SetColor(color);
        pco->SetState(STATE_CHECK, EqColor(color, m_perso.colorBody[m_perso.selectCar]));
    }

    pb = (Ui::CButton*)pw->SearchControl(EVENT_INTERFACE_PSUBMOD0);
    if ( pb == 0 )  return;
    pb->SetState(STATE_CHECK, m_perso.subModel[m_perso.selectCar]==0);

    pb = (Ui::CButton*)pw->SearchControl(EVENT_INTERFACE_PSUBMOD1);
    if ( pb == 0 )  return;
    pb->SetState(STATE_CHECK, m_perso.subModel[m_perso.selectCar]==1);

    pb = (Ui::CButton*)pw->SearchControl(EVENT_INTERFACE_PSUBMOD2);
    if ( pb == 0 )  return;
    pb->SetState(STATE_CHECK, m_perso.subModel[m_perso.selectCar]==2);

    pb = (Ui::CButton*)pw->SearchControl(EVENT_INTERFACE_PSUBMOD3);
    if ( pb == 0 )  return;
    pb->SetState(STATE_CHECK, m_perso.subModel[m_perso.selectCar]==3);

#if _SE
    if ( m_perso.stateCars[m_perso.selectCar] != SC_FORSALE )
#else
    if ( m_perso.selectCar < m_perso.total ||
         m_perso.selectCar < m_perso.bonus )  // sélectionne ?
#endif
    {
        pl = (Ui::CLabel*)pw->SearchControl(EVENT_INTERFACE_PPRICE);
        if ( pl == 0 )  return;
        pl->SetFontType(Gfx::FONT_COLOBOT);
        pl->SetState(STATE_VISIBLE);
        car = NamePerso(m_perso.usedCars[m_perso.selectCar]);
        pl->SetName(car);  // nom du véhicule

        pl = (Ui::CLabel*)pw->SearchControl(EVENT_INTERFACE_PPESETAS);
        if ( pl == 0 )  return;
        pl->ClearState(STATE_VISIBLE);

        pb = (Ui::CButton*)pw->SearchControl(EVENT_INTERFACE_PCAROK);
        if ( pb == 0 )  return;
        GetResource(RES_EVENT, EVENT_INTERFACE_PCAROK, res);
        pb->SetName(res);
        pb->ClearState(STATE_CHECK);
    }
    else	// achète ?
    {
        pl = (Ui::CLabel*)pw->SearchControl(EVENT_INTERFACE_PPRICE);
        if ( pl == 0 )  return;
        pl->SetFontType(Gfx::FONT_BOLD);
        pl->SetState(STATE_VISIBLE);
        GetResource(RES_EVENT, EVENT_INTERFACE_PPRICE, res);
        car = NamePerso(m_perso.usedCars[m_perso.selectCar]);
        text = StrUtils::Format(res.c_str(), car.c_str(), PricePerso(m_perso.usedCars[m_perso.selectCar]));
        pl->SetName(text);  // prix du véhicule

        pl = (Ui::CLabel*)pw->SearchControl(EVENT_INTERFACE_PPESETAS);
        if ( pl == 0 )  return;
        pl->SetState(STATE_VISIBLE);
        GetResource(RES_EVENT, EVENT_INTERFACE_PPESETAS, res);
        text = StrUtils::Format(res.c_str(), m_perso.pesetas);
        pl->SetName(text);  // somme disponible

        pb = (Ui::CButton*)pw->SearchControl(EVENT_INTERFACE_PCAROK);
        if ( pb == 0 )  return;
        GetResource(RES_TEXT, RT_PERSO_BUY, res);
        pb->SetName(res);
        pb->SetState(STATE_CHECK);
    }

    bOK = true;
    /* TODO (krzys_h):
    // Teste si la voiture choisie a des spécifications compatibles
    // avec la mission à effectuer.
    bOK = true;
    res[0] = 0;
    ReadCarSpec(m_sel[m_index], missionSpec);
    SpecPerso(carSpec, m_perso.usedCars[m_perso.selectCar]);
//?	if ( m_perso.level < 3 && carSpec.minSpeed != NAN )
#if _DEMO
    if ( m_perso.selectCar != 2 )  // pas tijuana taxi ?
	{
		bOK = false;
		GetResource(RES_TEXT, RT_SPEC_DEMO, res);
	}
#else
#if _SE
    icon = 29;  // rouge
	if ( m_perso.stateCars[m_perso.selectCar] == SC_SHOWCASE )  // en vitrine ?
	{
		if ( m_perso.usedCars[m_perso.selectCar] == 4 ||  // firecraker ?
			 m_perso.usedCars[m_perso.selectCar] == 5 ||  // hooligan ?
			 m_perso.usedCars[m_perso.selectCar] == 1 )   // tijuana taxi ?
		{
			GetResource(RES_TEXT, RT_SPEC_DISPO, res);
			icon = 30;  // bleu
		}
		else
		{
			GetResource(RES_TEXT, RT_SPEC_DEMO, res);
		}
		bOK = false;
	}
#else
    if ( carSpec.minSpeed != NAN )
    {
        if ( missionSpec.minSpeed != NAN &&
             carSpec.minSpeed < missionSpec.minSpeed )
        {
            bOK = false;
            GetResource(RES_TEXT, RT_SPEC_MINSPEED, res);
        }
        if ( missionSpec.maxSpeed != NAN &&
             carSpec.minSpeed > missionSpec.maxSpeed )
        {
            bOK = false;
            GetResource(RES_TEXT, RT_SPEC_MAXSPEED, res);
        }
    }
#endif
#endif
    */

    pb = (Ui::CButton*)pw->SearchControl(EVENT_INTERFACE_PCAROK);
    if ( pb == 0 )  return;
    pb->SetState(STATE_ENABLE, bOK);

    pgr = (Ui::CGroup*)pw->SearchControl(EVENT_INTERFACE_PSPECB);
    if ( pgr == 0 )  return;
    pgr->SetState(STATE_VISIBLE, !bOK);
#if _SE
    pgr->SetIcon(icon);
#endif
    pl = (Ui::CLabel*)pw->SearchControl(EVENT_INTERFACE_PSPECT);
    if ( pl == 0 )  return;
    pl->SetState(STATE_VISIBLE, !bOK);
    pl->SetName(res);
}

// Met à jour l'élévation de la caméra selon le slider correspondant.

void CScreenCar::ElevationPerso()
{
    Ui::CWindow*	pw;
    Ui::CSlider*	ps;

    pw = (Ui::CWindow*)m_interface->SearchControl(EVENT_WINDOW4);
    if ( pw == 0 )  return;

    ps = (Ui::CSlider*)pw->SearchControl(EVENT_INTERFACE_PELEV);
    if ( ps == 0 )  return;

    m_persoElevation = ps->GetVisibleValue();
}

// TODO (krzys_h): Copied from CScreenApperance, perhaps should be moved somewhere common?
void CScreenCar::SetCamera(float x, float y, float cameraDistance)
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

// Met à jour la caméra pour le personnage.

void CScreenCar::CameraPerso()
{
    Gfx::CCamera* camera = m_main->GetCamera();
    Math::Vector	eye, look;

    eye = RotateView(Math::Vector(0.0f, 0.0f, 0.0f), m_persoAngle, m_persoElevation, 20.0f);
//?	look.y -= powf(eye.y, 5.0f)/10000.0f;  // empirique !
    look.y -= powf(eye.y, 7.0f)/4000000.0f;  // empirique !
    look.x = 0.0f;
    look.z = 0.0f;
    eye.y  += 2.0f+(0.7f+m_persoElevation)*2.0f;
    look.y += 2.0f+(0.7f+m_persoElevation)*2.0f;

    camera->SetType(Gfx::CAM_TYPE_SCRIPT);
    camera->SetScriptCamera(eye, look);
}


// Indique s'il est possible d'acheter une ou plusieurs voitures.

bool CScreenCar::IsBuyablePerso()
{
    int		i, model;

    if ( !m_bPesetas )  return false;  // course simple ?

    for ( i=0 ; i<50 ; i++ )
    {
        model = GetPersoModel(i);
        if ( model != 0 &&
             !UsedPerso(model) &&
             PricePerso(model) <= m_perso.pesetas )
        {
            return true;
        }
    }
    return false;
}

// Met à jour toutes les voitures que le joueur peut acheter.

void CScreenCar::BuyablePerso()
{
    int		i, index, model, max;

    m_perso.bonus = m_perso.total;

    if ( !m_bPesetas )  // course simple ?
    {
        m_perso.buyable = m_perso.total;

        if ( !UsedPerso(8) )  // torpedo ?
        {
            index = m_perso.total;
            m_perso.usedCars[index] = 8;
            m_perso.subModel[index] = 0;
            DefPerso(index);  // met la couleur standard
            m_perso.bonus = index+1;
        }
        return;
    }

#if _SE
    i = 0;
	m_perso.buyable = m_perso.total;
	for ( index=0 ; index<m_perso.total ; index++ )
	{
		model = m_perso.usedCars[index];
		if ( model != 0 &&
			 (model == 4 ||    // firecraker ?
			  model == 5 ||    // hooligan ?
			  model == 1 ) &&  // tijuana taxi ?
			 !UsedPerso(model) &&
			 PricePerso(model) <= m_perso.pesetas )
		{
			m_perso.subModel[index] = 0;
			DefPerso(index);  // met la couleur standard
			m_perso.stateCars[index] = SC_FORSALE;  // voiture à vendre
			m_perso.buyable = index;
			m_perso.selectCar = index;
		}
	}
#else
    index = m_perso.total;
    for ( i=0 ; i<50 ; i++ )
    {
        model = GetPersoModel(i);
        if ( model != 0 &&
             !UsedPerso(model) &&
             PricePerso(model) <= m_perso.pesetas )
        {
            m_perso.usedCars[index] = model;
            m_perso.subModel[index] = 0;
            DefPerso(index);  // met la couleur standard
            index ++;
        }
    }
    m_perso.buyable = index;

    if ( index > m_perso.total )
    {
        m_perso.selectCar = index-1;  // sélectionne la voiture à acheter
    }

    max = m_perso.total;
    if ( max < m_perso.bonus   )  max = m_perso.bonus;
    if ( max < m_perso.buyable )  max = m_perso.buyable;
    if ( m_perso.selectCar >= max && max > 0 )
    {
        m_perso.selectCar = max-1;
    }
#endif
}

// Sélectionne la voiture suivante ou précédente.

void CScreenCar::NextPerso(int dir)
{
    int		i;

    if ( dir > 0 )  // voiture suivante ?
    {
        i = m_perso.selectCar+1;
#if _SE
        if ( i >= m_perso.total )  i = 0;
#else
        if ( i >= m_perso.buyable &&
             i >= m_perso.bonus   )  i = 0;
#endif
        m_perso.selectCar = i;
    }
    else	// voiture précédente ?
    {
        i = m_perso.selectCar-1;
#if _SE
        if ( i < 0 )  i = m_perso.total-1;
#else
        if ( i < 0 )
        {
            if ( m_perso.buyable > m_perso.bonus )	i = m_perso.buyable-1;
            else									i = m_perso.bonus-1;
        }
#endif
        m_perso.selectCar = i;
    }
}

// Sélectionne ou achète la voiture courante.

void CScreenCar::SelectPerso()
{
#if _SE
    if ( m_perso.stateCars[m_perso.selectCar] != SC_FORSALE )
#else
    if ( m_perso.selectCar < m_perso.total ||
         m_perso.selectCar < m_perso.bonus )  // sélectionne ?
#endif
    {
//TODO (krzys_h):        WriteGamerInfo();
    }
    else	// achète ?
    {
        BuyPerso();
    }
}

// Achète une voiture.

void CScreenCar::BuyPerso()
{
    int		model, sel;

    if ( m_perso.total >= 49 )  return;

    sel = m_perso.selectCar;
    model = m_perso.usedCars[m_perso.selectCar];

#if _SE
    m_perso.stateCars[m_perso.selectCar] = SC_OWNER;
#else
    m_perso.selectCar = m_perso.total;
    m_perso.total ++;
#endif

    m_perso.usedCars[m_perso.selectCar] = model;
    m_perso.subModel[m_perso.selectCar] = m_perso.subModel[sel];
    m_perso.colorBody[m_perso.selectCar] = m_perso.colorBody[sel];

    m_perso.pesetas -= PricePerso(model);  // débite le compte du joueur

//TODO (krzys_h):    WriteGamerInfo();
}

// Achète toutes les voitures.

void CScreenCar::BuyAllPerso()
{
    int		i, rank;

    m_perso.total = 0;

    for ( i=0 ; i<50 ; i++ )
    {
        rank = GetPersoModel(i);
        if ( rank == 0 )  break;

        m_perso.usedCars[i] = rank;
        m_perso.subModel[i] = 0;
        m_perso.stateCars[i] = i==0?SC_OWNER:SC_SHOWCASE;
        DefPerso(i);
        m_perso.total ++;
    }

    m_perso.buyable = m_perso.total;
    m_perso.bonus   = m_perso.total;
    m_perso.selectCar = 0;

#if !_SE
//TODO (krzys_h):    WriteGamerInfo();
#endif
}

// Getourne le modèle d'une voiture dans l'ordre des performances.

int CScreenCar::GetPersoModel(int rank)
{
    if ( rank == 0 )  return 4;  // firecraker
    if ( rank == 1 )  return 5;  // hooligan
    if ( rank == 2 )  return 1;  // tijuana taxi
    if ( rank == 3 )  return 3;  // pickup
    if ( rank == 4 )  return 7;  // reo
    if ( rank == 5 )  return 2;  // ford 32
    if ( rank == 6 )  return 6;  // chevy
    if ( rank == 7 )  return 8;  // torpedo
    return 0;  // terminateur
}

// Getourne l'index d'un modèle de voiture.

int CScreenCar::IndexPerso(int model)
{
    int		i, max;

    if ( m_perso.total > m_perso.bonus )  max = m_perso.total;
    else                                  max = m_perso.bonus;

    for ( i=0 ; i<max ; i++ )
    {
        if ( model == m_perso.usedCars[i] )  return i;
    }
    return -1;
}

// Indique si un modèle de voiture est acheté par le joueur.

bool CScreenCar::UsedPerso(int model)
{
    int		i, max;

    if ( m_perso.total > m_perso.bonus )  max = m_perso.total;
    else                                  max = m_perso.bonus;

    for ( i=0 ; i<max ; i++ )
    {
#if _SE
        if ( model == m_perso.usedCars[i] &&
			 m_perso.stateCars[i] == SC_OWNER )  return true;
#else
        if ( model == m_perso.usedCars[i] )  return true;
#endif
    }
    return false;
}

// Getourne le prix d'un modèle de voiture.

int CScreenCar::PricePerso(int model)
{
    if ( model == 4 )  return     2;  // firecraker
    if ( model == 5 )  return    10;  // hooligan
    if ( model == 1 )  return    40;  // tijuana taxi
    if ( model == 3 )  return   150;  // pickup
    if ( model == 7 )  return   600;  // reo
    if ( model == 2 )  return  2500;  // ford 32
    if ( model == 6 )  return 10000;  // chevy
    if ( model == 8 )  return 12000;  // torpedo
    return 0;
}

// Getourne le nom d'un modèle de voiture.

const std::string CScreenCar::NamePerso(int model)
{
    if ( model == 4 )  return "Basic Buzzing Buggy";  // firecraker
    if ( model == 5 )  return "Hooly Wheely";         // hooligan
    if ( model == 1 )  return "Mellow Cab";           // tijuana taxi
    if ( model == 3 )  return "Daddy's Pickup";       // pickup
    if ( model == 7 )  return "Don Carleone";         // reo
    if ( model == 2 )  return "Drag Star";            // ford 32
    if ( model == 6 )  return "Big Buzzing Bee";      // chevy
    if ( model == 8 )  return "Speedy Torpedo";       // torpedo
    return "?";
}

// Getourne les spécifications d'un modèle de voiture.

void CScreenCar::SpecPerso(CarSpec &spec, int model)
{
    spec.minSpeed = NAN;
    spec.maxSpeed = NAN;

    if ( model == 4 )  // firecraker
    {
        spec.minSpeed = 120.0f;
        spec.maxSpeed = 120.0f;
    }
    if ( model == 5 )  // hooligan
    {
        spec.minSpeed = 120.0f;
        spec.maxSpeed = 120.0f;
    }
    if ( model == 1 )  // tijuana taxi
    {
        spec.minSpeed = 120.0f;
        spec.maxSpeed = 120.0f;
    }
    if ( model == 3 )  // pickup
    {
        spec.minSpeed = 130.0f;
        spec.maxSpeed = 130.0f;
    }
    if ( model == 7 )  // reo
    {
        spec.minSpeed = 140.0f;
        spec.maxSpeed = 140.0f;
    }
    if ( model == 2 )  // ford 32
    {
        spec.minSpeed = 150.0f;
        spec.maxSpeed = 150.0f;
    }
    if ( model == 6 )  // chevy
    {
        spec.minSpeed = 160.0f;
        spec.maxSpeed = 160.0f;
    }
    if ( model == 8 )  // torpedo
    {
        spec.minSpeed = 170.0f;
        spec.maxSpeed = 170.0f;
    }
}

// Met une couleur fixe.

void CScreenCar::FixPerso(int rank)
{
    m_perso.colorBody[m_perso.selectCar].r = perso_color[rank*3+0]/255.0f;
    m_perso.colorBody[m_perso.selectCar].g = perso_color[rank*3+1]/255.0f;
    m_perso.colorBody[m_perso.selectCar].b = perso_color[rank*3+2]/255.0f;
}

// Initialise le personnage de base lors de la création d'un joueur.

void CScreenCar::FlushPerso()
{
    memset(&m_perso, 0, sizeof(GamerPerso));
    m_perso.usedCars[0] = 4;  // firecraker
    m_perso.subModel[0] = 0;  // peinture de base
    m_perso.total = 1;
    m_perso.bonus = 1;
    m_perso.selectCar = 0;
    m_perso.pesetas = 0;
    m_perso.level = 3;  // niveau intermédiaire (costaud)
    DefPerso(0);  // met la couleur standard
}

// Met la couleur standard à une voiture.

void CScreenCar::DefPerso(int rank)
{
    int		model, color;

    model = m_perso.usedCars[rank];
    color = 0;  // couleur standard selon model
    if ( m_perso.subModel[rank] == 0 )
    {
        if ( model == 1 )  color =  0;  // jaune  - tijuana taxi
        if ( model == 2 )  color =  4;  // violet - ford 32
        if ( model == 3 )  color =  1;  // bleu   - pickup
        if ( model == 4 )  color =  2;  // rouge  - firecraker
        if ( model == 5 )  color =  3;  // noir   - hooligan
        if ( model == 6 )  color =  7;  // rose   - chevy
        if ( model == 7 )  color = 12;  // violet - reo
        if ( model == 8 )  color =  2;  // rouge  - torpedo
    }
    if ( m_perso.subModel[rank] == 1 )
    {
        if ( model == 1 )  color = 13;  // cyan   - tijuana taxi
        if ( model == 2 )  color = 13;  // cyan   - ford 32
        if ( model == 3 )  color =  6;  // brun   - pickup
        if ( model == 4 )  color = 12;  // violet - firecraker
        if ( model == 5 )  color =  4;  // violet - hooligan
        if ( model == 6 )  color = 11;  // bleu   - chevy
        if ( model == 7 )  color =  6;  // brun   - reo
        if ( model == 8 )  color = 12;  // violet - torpedo
    }
    if ( m_perso.subModel[rank] == 2 )
    {
        if ( model == 1 )  color = 12;  // violet - tijuana taxi
        if ( model == 2 )  color = 12;  // violet - ford 32
        if ( model == 3 )  color = 12;  // violet - pickup
        if ( model == 4 )  color =  4;  // violet - firecraker
        if ( model == 5 )  color = 12;  // violet - hooligan
        if ( model == 6 )  color =  4;  // violet - chevy
        if ( model == 7 )  color =  7;  // rose   - reo
        if ( model == 8 )  color =  6;  // brun   - torpedo
    }
    if ( m_perso.subModel[rank] == 3 )
    {
        if ( model == 1 )  color =  1;  // bleu   - tijuana taxi
        if ( model == 2 )  color =  0;  // jaune  - ford 32
        if ( model == 3 )  color =  3;  // noir   - pickup
        if ( model == 4 )  color =  5;  // blanc  - firecraker
        if ( model == 5 )  color =  1;  // bleu   - hooligan
        if ( model == 6 )  color = 12;  // violet - chevy
        if ( model == 7 )  color =  3;  // noir   - reo
        if ( model == 8 )  color = 10;  // jaune  - torpedo
    }

    m_perso.colorBody[rank].r = perso_color[color*3+0]/255.0f;
    m_perso.colorBody[rank].g = perso_color[color*3+1]/255.0f;
    m_perso.colorBody[rank].b = perso_color[color*3+2]/255.0f;
    m_perso.colorBody[rank].a = 0.0f;
}

// Getourne le modèle de la voiture choisie.

int CScreenCar::GetModel()
{
//TODO (krzys_h):    if ( m_bDuel )
    {
//TODO (krzys_h):        return m_duelModel;
    }
//TODO (krzys_h):    else
    {
        return m_perso.usedCars[m_perso.selectCar];
    }
}

int CScreenCar::GetSubModel()
{
//TODO (krzys_h):    if ( m_bDuel )
    {
//TODO (krzys_h):        return m_duelSubModel;
    }
//TODO (krzys_h):    else
    {
        return m_perso.subModel[m_perso.selectCar];
    }
}

// Débute une mission.

void CScreenCar::LaunchSimul()
{
    /* TODO (krzys_h):
    if ( m_bDuel )
    {
        if ( m_duelType == 's' )  strcpy(m_sceneName, "scene");
        if ( m_duelType == 'f' )  strcpy(m_sceneName, "free");
        if ( m_duelType == 'u' )  strcpy(m_sceneName, "user");
        if ( m_duelType == 'p' )  strcpy(m_sceneName, "proto");
        m_sceneRank = m_duelMission;
    }
    else
    {
        if ( m_phaseTerm == PHASE_MISSION )  strcpy(m_sceneName, "scene");
        if ( m_phaseTerm == PHASE_FREE    )  strcpy(m_sceneName, "free");
        if ( m_phaseTerm == PHASE_USER    )  strcpy(m_sceneName, "user");
        if ( m_phaseTerm == PHASE_PROTO   )  strcpy(m_sceneName, "proto");
        m_sceneRank = m_sel[m_index];
    }
    m_main->ChangePhase(PHASE_LOADING);*/
    m_main->ChangePhase(PHASE_SIMUL);
}

// Retourne la couleur de carrosserie du véhicule sélectionné.

Gfx::Color CScreenCar::GetGamerColorCar()
{
//TODO (krzys_h):    if ( m_bDuel )
    {
//TODO (krzys_h):        return m_duelColor;
    }
//TODO (krzys_h):    else
    {
        return m_perso.colorBody[m_perso.selectCar];
    }
}

} // namespace Ui
