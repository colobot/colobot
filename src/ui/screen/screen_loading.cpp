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

#include "ui/screen/screen_loading.h"

#include "app/app.h"

#include "ui/controls/button.h"
#include "ui/controls/edit.h"
#include "ui/controls/gauge.h"
#include "ui/controls/group.h"
#include "ui/controls/interface.h"
#include "ui/controls/label.h"
#include "ui/controls/window.h"

namespace Ui
{

CScreenLoading::CScreenLoading()
    : CScreen(EVENT_WINDOW5)
    , m_visible(false)
    , m_lastProgress(0.0f)
{
}

void CScreenLoading::CreateInterface()
{
    CWindow*        pw;
    CLabel*         pl;
    CGroup*         pg;
    Math::Point     pos, ddim;
    std::string     name;

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

    pos.x  = 257.5f/640.0f;
    pos.y  = 210.0f/480.0f;
    ddim.x = 125.0f/640.0f;
    ddim.y =  20.0f/480.0f;
    GetResource(RES_TEXT, RT_DIALOG_LOADING, name);
    pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL1, name);
    pl->SetTextAlign(Gfx::TEXT_ALIGN_CENTER);

    pos.y -= ddim.y*1.75f;
    pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL2, "");
    pl->SetTextAlign(Gfx::TEXT_ALIGN_CENTER);

    pos.y -= ddim.y*1.0f;
    pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL3, "");
    pl->SetTextAlign(Gfx::TEXT_ALIGN_CENTER);
    pl->SetFontSize(Gfx::FONT_SIZE_SMALL*0.75f);

    pos.y -= ddim.y*2.0f;
    pw->CreateGauge(pos, ddim, 0, EVENT_LOADING);

    SetBackground("textures/interface/interface.png");
    CreateVersionDisplay();

    m_engine->SetDrawWorld(false);
    m_app->Render();

    m_visible = true;
}

void CScreenLoading::DestroyInterface()
{
    if (!m_visible) return;
    m_visible = false;

    m_engine->SetBackForce(false);
    m_engine->SetDrawWorld(true);

    m_interface->DeleteControl(EVENT_WINDOW5);
}

bool CScreenLoading::EventProcess(const Event &event)
{
    return false; // Stop all events!
}

bool CScreenLoading::IsVisible()
{
    return m_visible;
}

void CScreenLoading::SetProgress(float progress, const std::string& text, const std::string& details)
{
    if (!m_visible) return;

    CWindow* pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if (pw == nullptr) return;

    CLabel* pl = static_cast<CLabel*>(pw->SearchControl(EVENT_LABEL2));
    if (pl != nullptr)
    {
        pl->SetName(text);
    }

    pl = static_cast<CLabel*>(pw->SearchControl(EVENT_LABEL3));
    if (pl != nullptr)
    {
        pl->SetName(details);
    }

    CGauge* pg = static_cast<CGauge*>(pw->SearchControl(EVENT_LOADING));
    if (pg != nullptr)
    {
        pg->SetLevel(progress);
    }

    if (progress != m_lastProgress ||
        text     != m_lastText     ||
        details  != m_lastDetails   )
    {
        SetBackground("textures/interface/interface.png");
        m_engine->SetBackForce(true);
        m_app->RenderIfNeeded(60);
    }

    m_lastProgress = progress;
    m_lastText = text;
    m_lastDetails = details;
}

void CScreenLoading::SetProgress(float progress, ResTextType text, const std::string& details)
{
    std::string name;
    GetResource(RES_TEXT, text, name);
    SetProgress(progress, name, details);
}

void CScreenLoading::SetProgress(float progress, ResTextType text, ResTextType details)
{
    std::string name;
    GetResource(RES_TEXT, details, name);
    SetProgress(progress, text, name);
}

} // namespace Ui
