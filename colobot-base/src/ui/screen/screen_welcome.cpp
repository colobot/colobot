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

#include "ui/screen/screen_welcome.h"

#include "app/app.h"

#include "common/stringutils.h"

#include "graphics/core/material.h"

#include "graphics/engine/engine.h"

#include "level/robotmain.h"

#include "ui/controls/interface.h"
#include "ui/controls/window.h"

namespace Ui
{

const float WELCOME_LENGTH = 3.0f;

CScreenWelcome::CScreenWelcome()
    : m_imageIndex(0),
      m_time(0.0f)
{
}

void CScreenWelcome::SetImageIndex(int imageIndex)
{
    m_imageIndex = imageIndex;
    m_time = 0.0f;
}

void CScreenWelcome::CreateInterface()
{
    glm::vec2     pos, ddim;

    pos.x  = 0.0f;
    pos.y  = 0.0f;
    ddim.x = 0.0f;
    ddim.y = 0.0f;
    m_interface->CreateWindows(pos, ddim, -1, EVENT_WINDOW5);

    if ( m_imageIndex == 0 )
        m_engine->SetOverColor(Gfx::Color(1.0f, 1.0f, 1.0f, 1.0f), Gfx::TransparencyMode::BLACK);
    else
        m_engine->SetOverColor(Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f), Gfx::TransparencyMode::WHITE);
    m_engine->SetOverFront(true);

    SetBackground(StrUtils::ToPath("textures/interface/intro"+StrUtils::ToString<int>(m_imageIndex+1)+".png"), true);
}

bool CScreenWelcome::EventProcess(const Event &event)
{
    if (event.type == EVENT_FRAME)
    {
        m_time += event.rTime;

        // 1/4 of display time is animating
        float animatingTime = WELCOME_LENGTH / 4.0f;

        float intensity;
        if (m_time < animatingTime)
        {
            // appearing
            intensity = m_time / animatingTime;
        }
        else if (m_time < WELCOME_LENGTH - animatingTime)
        {
            // showing
            intensity = 1.0f;
        }
        else
        {
            // hiding
            intensity = (WELCOME_LENGTH - m_time) / animatingTime;
        }

        if ( intensity < 0.0f )  intensity = 0.0f;
        if ( intensity > 1.0f )  intensity = 1.0f;

        // white first, others -> black fading
        Gfx::TransparencyMode mode = Gfx::TransparencyMode::WHITE;
        if ((m_imageIndex == 0) && (m_time < WELCOME_LENGTH/2.0f))
        {
            intensity = 1.0f - intensity;
            mode = Gfx::TransparencyMode::BLACK;
        }

        m_engine->SetOverColor(Gfx::Color(intensity, intensity, intensity, intensity), mode);
    }

    if (m_time >= WELCOME_LENGTH             ||
        event.type == EVENT_KEY_DOWN         ||
        event.type == EVENT_MOUSE_BUTTON_DOWN )
    {
        m_main->ChangePhase(static_cast<Phase>(PHASE_WELCOME1+m_imageIndex+1));
        return false;
    }

    return true;
}

} // namespace Ui
