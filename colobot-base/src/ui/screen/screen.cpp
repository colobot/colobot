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

#include "ui/screen/screen.h"

#include "app/app.h"

#include "common/version.h"
#include "common/stringutils.h"

#include "graphics/engine/engine.h"

#include "level/robotmain.h"

#include "ui/controls/interface.h"
#include "ui/controls/label.h"
#include "ui/controls/window.h"

namespace Ui
{

CScreen::CScreen()
{
    m_main       = CRobotMain::GetInstancePointer();
    m_interface  = m_main->GetInterface();
    m_app        = CApplication::GetInstancePointer();
    m_eventQueue = m_app->GetEventQueue();
    m_engine     = Gfx::CEngine::GetInstancePointer();
    m_sound      = m_app->GetSound();
}

CScreen::~CScreen()
{
}

void CScreen::SetBackground(const std::string& filename, bool scaled)
{
    m_engine->SetBackground(TempToPath(filename),
            Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
            Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
            Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
            Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
            true, scaled);
    m_engine->SetBackForce(true);
}

void CScreen::CreateVersionDisplay()
{
    CWindow* pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if (pw != nullptr)
    {
        glm::vec2 pos, ddim;

        pos.x  = 540.0f/640.0f;
        pos.y  =   9.0f/480.0f;
        ddim.x =  90.0f/640.0f;
        ddim.y =  10.0f/480.0f;
        CLabel* pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL1, std::string(Version::VERSION_DISPLAY));
        pl->SetFontType(Gfx::FONT_STUDIO);
        pl->SetFontSize(9.0f);
    }
}

} // namespace Ui
