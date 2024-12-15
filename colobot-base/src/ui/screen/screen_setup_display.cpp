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

#include "ui/screen/screen_setup_display.h"

#include "common/config.h"

#include "app/app.h"

#include "common/restext.h"
#include "common/settings.h"
#include "common/stringutils.h"

#include "graphics/core/device.h"
#include "graphics/engine/camera.h"
#include "graphics/engine/engine.h"

#include "ui/controls/button.h"
#include "ui/controls/check.h"
#include "ui/controls/interface.h"
#include "ui/controls/label.h"
#include "ui/controls/list.h"
#include "ui/controls/window.h"

namespace Ui
{

CScreenSetupDisplay::CScreenSetupDisplay()
    : m_setupSelMode{0},
      m_setupFull{false}
{
}

void CScreenSetupDisplay::SetActive()
{
    m_tab = PHASE_SETUPd;
}

void CScreenSetupDisplay::CreateInterface()
{
    CWindow*        pw;
    CLabel*         pl;
    CList*          pli;
    CCheck*         pc;
    CButton*        pb;
    glm::vec2       pos, ddim;
    std::string     name;

    CScreenSetup::CreateInterface();
    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == nullptr )  return;

    auto modes = m_app->GetVideoResolutionList();

    for (auto it = modes.begin(); it != modes.end(); ++it)
    {
        if (it->x == m_app->GetVideoConfig().size.x && it->y == m_app->GetVideoConfig().size.y)
        {
            m_setupSelMode = it - modes.begin();
            break;
        }
    }
    m_setupFull = m_app->GetVideoConfig().fullScreen;

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

    pos.x = ox+sx*10;
    pos.y = oy+sy*6.75f;
    ddim.x = dim.x*6;
    ddim.y = dim.y*1;
    GetResource(RES_EVENT, EVENT_INTERFACE_VSYNC, name);
    pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL2, name);
    pl->SetTextAlign(Gfx::TEXT_ALIGN_LEFT);

    pos.x = ox+sx*10;
    pos.y = oy+sy*5.2f;
    ddim.x = dim.x*6;
    ddim.y = dim.y*2;
    pli = pw->CreateList(pos, ddim, 0, EVENT_INTERFACE_VSYNC);
    pli->SetState(STATE_SHADOW);
    pli->SetItemName(0, "Off");
    pli->SetItemName(1, "Adaptive");
    pli->SetItemName(2, "On");
    switch(m_engine->GetVSync())
    {
        case -1: //Adaptive?
            pli->SetSelect(1);
            break;
        case 0: //Off?
            pli->SetSelect(0);
            break;
        case 1: //On?
            pli->SetSelect(2);
            break;
    }

    ddim.x = dim.x*6;
    ddim.y = dim.y*1;
    pos.x = ox+sx*10;
    pos.y = oy+sy*2;
    pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_APPLY);
    pb->SetState(STATE_SHADOW);
    UpdateApply();
}

bool CScreenSetupDisplay::EventProcess(const Event &event)
{
    if (!CScreenSetup::EventProcess(event)) return false;

    CWindow* pw;
    CCheck* pc;
    CButton* pb;
    CList* pl;

    switch( event.type )
    {
        case EVENT_LIST2:
            UpdateApply();
            break;

        case EVENT_INTERFACE_FULL:
            pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
            if ( pw == nullptr )  break;
            pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_FULL));
            if ( pc == nullptr )  break;

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
            if ( pw == nullptr )  break;
            pb = static_cast<CButton*>(pw->SearchControl(EVENT_INTERFACE_APPLY));
            if ( pb == nullptr )  break;
            pb->ClearState(STATE_PRESS);
            pb->ClearState(STATE_HILIGHT);
            ChangeDisplay();
            UpdateApply();
            break;

        case EVENT_INTERFACE_VSYNC:
            pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
            if ( pw == nullptr ) break;
            pl = static_cast<CList*>(pw->SearchControl(EVENT_INTERFACE_VSYNC));
            if (pl == nullptr ) break;
            switch(pl->GetSelect())
            {
                case 0: //Off?
                    m_engine->SetVSync(0);
                    break;
                case 1: //Adaptive?
                    m_engine->SetVSync(-1);
                    break;
                case 2: //On?
                    m_engine->SetVSync(1);
                    break;
            }
            ChangeDisplay();
            UpdateApply();
            break;

        default:
            return true;
    }
    return false;
}

// Updates the list of modes.

static int GCD(int a, int b)
{
    return (b == 0) ? a : GCD(b, a%b);
}

static glm::ivec2 AspectRatio(const glm::ivec2& resolution)
{
    return resolution / GCD(resolution.x, resolution.y);
}

void CScreenSetupDisplay::UpdateDisplayMode()
{
    CWindow*    pw;
    CList*      pl;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == nullptr )  return;
    pl = static_cast<CList*>(pw->SearchControl(EVENT_LIST2));
    if ( pl == nullptr )  return;
    pl->Flush();

    auto modes = m_app->GetVideoResolutionList();

    int i = 0;
    std::stringstream mode_text;
    for (const auto& mode : modes)
    {
        mode_text.str("");
        glm::ivec2 aspect = AspectRatio(mode);
        mode_text << mode.x << "x" << mode.y << " [" << aspect.x << ":" << aspect.y << "]";
        pl->SetItemName(i++, mode_text.str());
    }

    pl->SetSelect(m_setupSelMode);
    pl->ShowSelect(false);
}

// Change the graphics mode.

void CScreenSetupDisplay::ChangeDisplay()
{
    CWindow*    pw;
    CList*      pl;
    CCheck*     pc;
    bool        bFull;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == nullptr )  return;

    pl = static_cast<CList*>(pw->SearchControl(EVENT_LIST2));
    if ( pl == nullptr )  return;
    m_setupSelMode = pl->GetSelect();

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_FULL));
    if ( pc == nullptr )  return;
    bFull = pc->TestState(STATE_CHECK);
    m_setupFull = bFull;

    auto modes = m_app->GetVideoResolutionList();

    Gfx::DeviceConfig config = m_app->GetVideoConfig();
    config.size = modes[m_setupSelMode];
    config.fullScreen = bFull;

    m_settings->SaveResolutionSettings(config);

    m_app->ChangeVideoConfig(config, /* isSetSize */ true);
}



// Updates the "apply" button.

void CScreenSetupDisplay::UpdateApply()
{
    CWindow*    pw;
    CButton*    pb;
    CList*      pl;
    CList*      pvl;
    CCheck*     pc;
    int         sel2;
    bool        bFull;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == nullptr )  return;

    pb = static_cast<CButton*>(pw->SearchControl(EVENT_INTERFACE_APPLY));

    pl = static_cast<CList*>(pw->SearchControl(EVENT_LIST2));
    if ( pl == nullptr )  return;
    sel2 = pl->GetSelect();

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_FULL));
    bFull = pc->TestState(STATE_CHECK);

    pvl = static_cast<CList*>(pw->SearchControl(EVENT_INTERFACE_VSYNC));
    if (pvl == nullptr)  return;

    switch (m_engine->GetVSync())
    {
    case -1: //Adaptive?
        pvl->SetSelect(1);
        break;
    case 0: //Off?
        pvl->SetSelect(0);
        break;
    case 1: //On?
        pvl->SetSelect(2);
        break;
    }

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

} // namespace Ui
