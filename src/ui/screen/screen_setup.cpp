/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2015, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include "ui/screen/screen_setup.h"

#include "common/config.h"

#include "app/app.h"

#include "common/settings.h"
#include "common/stringutils.h"

#include "graphics/engine/camera.h"

#include "ui/button.h"
#include "ui/check.h"
#include "ui/color.h"
#include "ui/editvalue.h"
#include "ui/group.h"
#include "ui/interface.h"
#include "ui/key.h"
#include "ui/label.h"
#include "ui/list.h"
#include "ui/scroll.h"
#include "ui/slider.h"
#include "ui/window.h"

namespace Ui
{

const int KEY_VISIBLE = 6;      // number of visible keys redefinable

CScreenSetup::CScreenSetup()
    : m_tab{PHASE_SETUPg},
      m_simulationSetup{false},
      m_setupSelMode{0},
      m_setupFull{false}
{
    m_settings = CSettings::GetInstancePointer();
    m_camera = m_main->GetCamera();
}

void CScreenSetup::SetTab(Phase tab, bool simulationSetup)
{
    m_tab = tab;
    m_simulationSetup = simulationSetup;

    //TODO: Move to CreateInterface?
    std::vector<Math::IntPoint> modes;
    m_app->GetVideoResolutionList(modes, true, true);
    for (auto it = modes.begin(); it != modes.end(); ++it)
    {
        if (it->x == m_app->GetVideoConfig().size.x && it->y == m_app->GetVideoConfig().size.y)
        {
            m_setupSelMode = it - modes.begin();
            break;
        }
    }
    m_setupFull = m_app->GetVideoConfig().fullScreen;
}

//TODO: Split this futher to separate class for each tab
void CScreenSetup::CreateInterface()
{
    CWindow*        pw;
    CEditValue*     pv;
    CLabel*         pl;
    CList*          pli;
    CCheck*         pc;
    CScroll*        ps;
    CSlider*        psl;
    CEnumSlider*    pes;
    CButton*        pb;
    CGroup*         pg;
    Math::Point     pos, ddim;
    std::string     name;

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
    pb->SetState(STATE_CHECK, (m_tab == PHASE_SETUPd));
    #if PLATFORM_WINDOWS
    pb->SetState(STATE_ENABLE, !m_simulationSetup);
    #endif

    pos.x += ddim.x+0.01f;
    pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_SETUPg);
    pb->SetState(STATE_SHADOW);
    pb->SetState(STATE_CARD);
    pb->SetState(STATE_CHECK, (m_tab == PHASE_SETUPg));

    pos.x += ddim.x+0.01f;
    pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_SETUPp);
    pb->SetState(STATE_SHADOW);
    pb->SetState(STATE_CARD);
    pb->SetState(STATE_CHECK, (m_tab == PHASE_SETUPp));

    pos.x += ddim.x+0.01f;
    pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_SETUPc);
    pb->SetState(STATE_SHADOW);
    pb->SetState(STATE_CARD);
    pb->SetState(STATE_CHECK, (m_tab == PHASE_SETUPc));

    pos.x += ddim.x+0.01f;
    pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_SETUPs);
    pb->SetState(STATE_SHADOW);
    pb->SetState(STATE_CARD);
    pb->SetState(STATE_CHECK, (m_tab == PHASE_SETUPs));

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

    if ( !m_simulationSetup )
    {
        SetBackground("textures/interface/interface.png");
    }

    if ( m_tab == PHASE_SETUPd ) // setup/display ?
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

    if ( m_tab == PHASE_SETUPg )  // setup/graphic ?
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
        if ( m_simulationSetup )
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
        if ( m_simulationSetup )
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
        if ( m_simulationSetup )
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

    if ( m_tab == PHASE_SETUPp )  // setup/game ?
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
        if ( m_simulationSetup )
        {
            pc->SetState(STATE_DEAD);
        }
        pos.y -= 0.048f;
        pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_EDITVALUE);
        pc->SetState(STATE_SHADOW);

        UpdateSetupButtons();
    }

    if ( m_tab == PHASE_SETUPc )  // setup/commands ?
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

    if ( m_tab == PHASE_SETUPs )  // setup/sound ?
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
}

bool CScreenSetup::EventProcess(const Event &event)
{
    if ( !m_simulationSetup )
    {
        CWindow* pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
        if ( pw == 0 )  return false;

        if ( event.type == pw->GetEventTypeClose() ||
                event.type == EVENT_INTERFACE_BACK   ||
                (event.type == EVENT_KEY_DOWN && event.key.key == KEY(ESCAPE)) )
        {
            m_settings->SaveSettings();
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
    else
    {
        CWindow* pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
        if ( pw == 0 )  return false;

        if ( event.type == pw->GetEventTypeClose() ||
                event.type == EVENT_INTERFACE_BACK   ||
                (event.type == EVENT_KEY_DOWN && event.key.key == KEY(ESCAPE)) )
        {
            m_settings->SaveSettings();
            m_engine->ApplyChange();
            m_interface->DeleteControl(EVENT_WINDOW5);
            m_main->ChangePhase(PHASE_SIMUL);
            m_main->StopSuspend();
            return false;
        }

        switch( event.type )
        {
            case EVENT_INTERFACE_SETUPd:
                m_main->ChangePhase(PHASE_SETUPds);
                return false;

            case EVENT_INTERFACE_SETUPg:
                m_main->ChangePhase(PHASE_SETUPgs);
                return false;

            case EVENT_INTERFACE_SETUPp:
                m_main->ChangePhase(PHASE_SETUPps);
                return false;

            case EVENT_INTERFACE_SETUPc:
                m_main->ChangePhase(PHASE_SETUPcs);
                return false;

            case EVENT_INTERFACE_SETUPs:
                m_main->ChangePhase(PHASE_SETUPss);
                return false;

            default:
                break;
        }
    }

    if ( m_tab == PHASE_SETUPd )  // setup/display ?
    {
        CWindow* pw;
        CCheck* pc;
        CButton* pb;

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

    if ( m_tab == PHASE_SETUPg )  // setup/graphic ?
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

    if ( m_tab == PHASE_SETUPp )  // setup/game ?
    {
        switch( event.type )
        {
            case EVENT_INTERFACE_TOTO:
                m_engine->SetTotoMode(!m_engine->GetTotoMode());
                ChangeSetupButtons();
                UpdateSetupButtons();
                break;

            case EVENT_INTERFACE_TOOLTIP:
                m_settings->SetTooltips(!m_settings->GetTooltips());
                ChangeSetupButtons();
                UpdateSetupButtons();
                break;

            case EVENT_INTERFACE_GLINT:
                m_settings->SetInterfaceGlint(!m_settings->GetInterfaceGlint());
                ChangeSetupButtons();
                UpdateSetupButtons();
                break;

            case EVENT_INTERFACE_RAIN:
                m_settings->SetInterfaceRain(!m_settings->GetInterfaceRain());
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
                m_settings->SetSoluce4(!m_settings->GetSoluce4());
                ChangeSetupButtons();
                UpdateSetupButtons();
                break;

            case EVENT_INTERFACE_MOVIES:
                m_settings->SetMovies(!m_settings->GetMovies());
                ChangeSetupButtons();
                UpdateSetupButtons();
                break;

            case EVENT_INTERFACE_NICERST:
                m_settings->SetNiceReset(!m_settings->GetNiceReset());
                ChangeSetupButtons();
                UpdateSetupButtons();
                break;

            case EVENT_INTERFACE_HIMSELF:
                m_settings->SetHimselfDamage(!m_settings->GetHimselfDamage());
                ChangeSetupButtons();
                UpdateSetupButtons();
                break;

            case EVENT_INTERFACE_SCROLL:
                m_camera->SetCameraScroll(!m_camera->GetCameraScroll());
                ChangeSetupButtons();
                UpdateSetupButtons();
                break;

            case EVENT_INTERFACE_INVERTX:
                m_camera->SetCameraInvertX(!m_camera->GetCameraInvertX());
                ChangeSetupButtons();
                UpdateSetupButtons();
                break;

            case EVENT_INTERFACE_INVERTY:
                m_camera->SetCameraInvertY(!m_camera->GetCameraInvertY());
                ChangeSetupButtons();
                UpdateSetupButtons();
                break;

            case EVENT_INTERFACE_EFFECT:
                m_camera->SetEffect(!m_camera->GetEffect());
                ChangeSetupButtons();
                UpdateSetupButtons();
                break;

            case EVENT_INTERFACE_BLOOD:
                m_camera->SetBlood(!m_camera->GetBlood());
                ChangeSetupButtons();
                UpdateSetupButtons();
                break;

            case EVENT_INTERFACE_AUTOSAVE_ENABLE:
                m_main->SetAutosave(!m_main->GetAutosave());
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

    if ( m_tab == PHASE_SETUPc )  // setup/commands ?
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

    if ( m_tab == PHASE_SETUPs )  // setup/sound ?
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

    return true;
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

void CScreenSetup::UpdateDisplayMode()
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

void CScreenSetup::ChangeDisplay()
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

    std::vector<Math::IntPoint> modes;
    m_app->GetVideoResolutionList(modes, true, true);

    Gfx::DeviceConfig config = m_app->GetVideoConfig();
    config.size = modes[m_setupSelMode];
    config.fullScreen = bFull;

    m_settings->SaveResolutionSettings(config);

    #if !PLATFORM_LINUX
    // Windows causes problems, so we'll restart the game
    // Mac OS was not tested so let's restart just to be sure
    m_app->Restart();
    #else
    m_app->ChangeVideoConfig(config);
    #endif
}



// Updates the "apply" button.

void CScreenSetup::UpdateApply()
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

void CScreenSetup::UpdateSetupButtons()
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
        pc->SetState(STATE_CHECK, m_settings->GetTooltips());
    }

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_GLINT));
    if ( pc != 0 )
    {
        pc->SetState(STATE_CHECK, m_settings->GetInterfaceGlint());
    }

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_RAIN));
    if ( pc != 0 )
    {
        pc->SetState(STATE_CHECK, m_settings->GetInterfaceRain());
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
        pc->SetState(STATE_CHECK, m_settings->GetSoluce4());
    }

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_MOVIES));
    if ( pc != 0 )
    {
        pc->SetState(STATE_CHECK, m_settings->GetMovies());
    }

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_NICERST));
    if ( pc != 0 )
    {
        pc->SetState(STATE_CHECK, m_settings->GetNiceReset());
    }

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_HIMSELF));
    if ( pc != 0 )
    {
        pc->SetState(STATE_CHECK, m_settings->GetHimselfDamage());
    }

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_SCROLL));
    if ( pc != 0 )
    {
        pc->SetState(STATE_CHECK, m_camera->GetCameraScroll());
    }

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_INVERTX));
    if ( pc != 0 )
    {
        pc->SetState(STATE_CHECK, m_camera->GetCameraInvertX());
    }

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_INVERTY));
    if ( pc != 0 )
    {
        pc->SetState(STATE_CHECK, m_camera->GetCameraInvertY());
    }

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_EFFECT));
    if ( pc != 0 )
    {
        pc->SetState(STATE_CHECK, m_camera->GetEffect());
    }

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_BLOOD));
    if ( pc != 0 )
    {
        pc->SetState(STATE_CHECK, m_camera->GetBlood());
    }

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_AUTOSAVE_ENABLE));
    if ( pc != 0 )
    {
        pc->SetState(STATE_CHECK, m_main->GetAutosave());
    }

    ps = static_cast<CSlider*>(pw->SearchControl(EVENT_INTERFACE_AUTOSAVE_INTERVAL));
    if ( ps != 0 )
    {
        ps->SetState(STATE_ENABLE, m_main->GetAutosave());
        ps->SetVisibleValue(m_main->GetAutosaveInterval());

    }

    ps = static_cast<CSlider*>(pw->SearchControl(EVENT_INTERFACE_AUTOSAVE_SLOTS));
    if ( ps != 0 )
    {
        ps->SetState(STATE_ENABLE, m_main->GetAutosave());
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

void CScreenSetup::ChangeSetupButtons()
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


// Changes the general level of quality.

void CScreenSetup::ChangeSetupQuality(int quality)
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

void CScreenSetup::UpdateKey()
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

void CScreenSetup::ChangeKey(EventType event)
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

Phase CScreenSetup::GetSetupTab()
{
    return m_tab;
}

} // namespace Ui
