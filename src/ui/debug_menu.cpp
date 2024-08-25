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

#include "ui/debug_menu.h"

#include "app/app.h"

#include "common/event.h"
#include "common/stringutils.h"
#include "common/global.h"

#include "graphics/engine/lightning.h"
#include "graphics/engine/terrain.h"

#include "level/robotmain.h"

#include "object/object.h"
#include "object/object_manager.h"

#include "sound/sound.h"

#include "ui/controls/button.h"
#include "ui/controls/check.h"
#include "ui/controls/interface.h"
#include "ui/controls/label.h"
#include "ui/controls/window.h"

#include <SDL_clipboard.h>

namespace Ui
{

CDebugMenu::CDebugMenu(CRobotMain* main, Gfx::CEngine* engine, CObjectManager* objMan, CSoundInterface* sound)
{
    m_main = main;
    m_interface = m_main->GetInterface();
    m_engine = engine;
    m_objMan = objMan;
    m_sound = sound;
}

CDebugMenu::~CDebugMenu()
{

}

void CDebugMenu::ToggleInterface()
{
    if (!IsActive())
    {
        CreateInterface();
        CLabel* pl = m_interface->CreateLabel(Math::Point(0.0f, 0.9f), Math::Point(1.0f, 0.1f), -1, EVENT_LABEL19, "??");
        pl->SetFontType(Gfx::FONT_STUDIO);
    }
    else
    {
        m_interface->DeleteControl(EVENT_LABEL19);
        DestroyInterface();
    }
}

const Math::Point dim = Math::Point(33.0f/640.0f, 33.0f/480.0f);
const float ox = 3.0f/640.0f, oy = 3.0f/480.0f;
const float /*sx = 33.0f/640.0f,*/ sy = 33.0f/480.0f;

void CDebugMenu::CreateInterface()
{
    CWindow* pw = m_interface->CreateWindows(Math::Point(), Math::Point(), 0, EVENT_WINDOW7);
    Math::Point pos, ddim;
    CCheck* pc;
    CButton* pb;

    ddim.x = 4*dim.x+4*ox;
    ddim.y = 222.0f/480.0f;
    pos.x = 1.0f-ddim.x;
    pos.y = oy+sy*3.0f;
    pw->CreateGroup(pos, ddim, 6, EVENT_WINDOW7);

    ddim.x = ddim.x - 4*ox;
    ddim.y = dim.y*0.5f;
    pos.x += 2*ox;
    pos.y = oy+sy*9.0f;
    pb = pw->CreateButton(pos, ddim, -1, EVENT_DBG_SPAWN_OBJ);
    pb->SetName("Spawn object");
    pos.y -= ddim.y;
    pb = pw->CreateButton(pos, ddim, -1, EVENT_DBG_TELEPORT);
    pb->SetName("Teleport");
    pos.y -= ddim.y;
    pb = pw->CreateButton(pos, ddim, -1, EVENT_DBG_LIGHTNING);
    pb->SetName("Lightning");
    pos.y -= 0.048f;
    pc = pw->CreateCheck(pos, ddim, -1, EVENT_DBG_STATS);
    pc->SetName("Display stats");
    pos.y -= 0.048f;
    pos.y -= 0.048f;
    pc = pw->CreateCheck(pos, ddim, -1, EVENT_DBG_RESOURCES);
    pc->SetName("Underground resources");
    pos.y -= 0.048f;
    pc = pw->CreateCheck(pos, ddim, -1, EVENT_DBG_GOTO);
    pc->SetName("Render goto() path");
    pos.y -= 0.048f;
    pc = pw->CreateCheck(pos, ddim, -1, EVENT_DBG_CRASHSPHERES);
    pc->SetName("Render crash spheres");
    pos.y -= 0.048f;
    pc = pw->CreateCheck(pos, ddim, -1, EVENT_DBG_LIGHTS);
    pc->SetName("Render dynamic lights");
    pos.y -= 0.048f;
    pb = pw->CreateButton(pos, ddim, -1, EVENT_DBG_LIGHTS_DUMP);
    pb->SetName("Dump lights to log");

    UpdateInterface();
}

void CDebugMenu::CreateSpawnInterface()
{
    CWindow* pw = m_interface->CreateWindows(Math::Point(), Math::Point(), 0, EVENT_WINDOW7);
    Math::Point pos, ddim;
    CButton* pb;

    ddim.x = 4*dim.x+4*ox;
    ddim.y = 222.0f/480.0f;
    pos.x = 1.0f-ddim.x;
    pos.y = oy+sy*3.0f;
    pw->CreateGroup(pos, ddim, 6, EVENT_WINDOW7);

    ddim.x = ddim.x - 4*ox;
    ddim.y = dim.y*0.5f;
    pos.x += 2*ox;
    pos.y = oy+sy*9.0f;
    pb = pw->CreateButton(pos, ddim, -1, EVENT_SPAWN_CANCEL);
    pb->SetName("Cancel");
    pos.y -= ddim.y;

    pos.y -= dim.y;
    pw->CreateButton(pos, dim, 128+8, EVENT_SPAWN_ME);
    pos.x += dim.x;
    pw->CreateButton(pos, dim, 128+9, EVENT_SPAWN_WHEELEDGRABBER);
    pos.x += dim.x;
    pw->CreateButton(pos, dim, 128+15, EVENT_SPAWN_WHEELEDSHOOTER);
    pos.x += dim.x;
    pw->CreateButton(pos, dim, 128+19, EVENT_SPAWN_PHAZERSHOOTER);
    pos.x -= 3*dim.x;
    pos.y -= dim.y;
    pw->CreateButton(pos, dim, 128+32, EVENT_SPAWN_BOTFACTORY);
    pos.x += dim.x;
    pw->CreateButton(pos, dim, 128+34, EVENT_SPAWN_CONVERTER);
    pos.x += dim.x;
    pw->CreateButton(pos, dim, 128+33, EVENT_SPAWN_DERRICK);
    pos.x += dim.x;
    pw->CreateButton(pos, dim, 128+36, EVENT_SPAWN_POWERSTATION);
    pos.x -= 3*dim.x;
    pos.y -= ddim.y;
    pb = pw->CreateButton(pos, ddim, -1, EVENT_SPAWN_TITANIUM);
    pb->SetName("Titanium");
    pos.y -= ddim.y;
    pb = pw->CreateButton(pos, ddim, -1, EVENT_SPAWN_TITANIUMORE);
    pb->SetName("TitaniumOre");
    pos.y -= ddim.y;
    pb = pw->CreateButton(pos, ddim, -1, EVENT_SPAWN_URANIUMORE);
    pb->SetName("UraniumOre");
    pos.y -= ddim.y;
    pb = pw->CreateButton(pos, ddim, -1, EVENT_SPAWN_POWERCELL);
    pb->SetName("PowerCell");
    pos.y -= ddim.y;
    pb = pw->CreateButton(pos, ddim, -1, EVENT_SPAWN_NUCLEARCELL);
    pb->SetName("NuclearCell");
}

const std::map<EventType, ObjectType> SPAWN_TYPES = {
    {EVENT_SPAWN_ME,             OBJECT_HUMAN},
    {EVENT_SPAWN_WHEELEDGRABBER, OBJECT_MOBILEwa},
    {EVENT_SPAWN_WHEELEDSHOOTER, OBJECT_MOBILEwc},
    {EVENT_SPAWN_PHAZERSHOOTER,  OBJECT_MOBILErc},
    {EVENT_SPAWN_BOTFACTORY,     OBJECT_FACTORY},
    {EVENT_SPAWN_CONVERTER,      OBJECT_CONVERT},
    {EVENT_SPAWN_DERRICK,        OBJECT_DERRICK},
    {EVENT_SPAWN_POWERSTATION,   OBJECT_STATION},
    {EVENT_SPAWN_TITANIUM,       OBJECT_METAL},
    {EVENT_SPAWN_TITANIUMORE,    OBJECT_STONE},
    {EVENT_SPAWN_URANIUMORE,     OBJECT_URANIUM},
    {EVENT_SPAWN_POWERCELL,      OBJECT_POWER},
    {EVENT_SPAWN_NUCLEARCELL,    OBJECT_ATOMIC},
};

void CDebugMenu::UpdateInterface()
{
    CCheck* pc;
    CButton* pb;

    CWindow* pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW7));
    if (pw == nullptr) return;


    pb = static_cast<CButton*>(pw->SearchControl(EVENT_DBG_LIGHTNING));
    if (pb != nullptr)
    {
        pb->SetName(m_lightningActive ? "Disable lightning" : "Lightning");
    }

    pb = static_cast<CButton*>(pw->SearchControl(EVENT_DBG_TELEPORT));
    if (pb != nullptr)
    {
        pb->SetName(m_teleportActive ? "Abort teleport" : "Teleport");
    }

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_DBG_STATS));
    if (pc != nullptr)
    {
        pc->SetState(STATE_CHECK, m_engine->GetShowStats());
    }

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_DBG_RESOURCES));
    if (pc != nullptr)
    {
        pc->SetState(STATE_CHECK, m_engine->GetDebugResources());
    }

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_DBG_GOTO));
    if (pc != nullptr)
    {
        pc->SetState(STATE_CHECK, m_engine->GetDebugGoto());
    }

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_DBG_CRASHSPHERES));
    if (pc != nullptr)
    {
        pc->SetState(STATE_CHECK, m_main->GetDebugCrashSpheres());
    }

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_DBG_LIGHTS));
    if (pc != nullptr)
    {
        pc->SetState(STATE_CHECK, m_engine->GetDebugLights());
    }

    for (const auto& it : SPAWN_TYPES)
    {
        pb = static_cast<CButton*>(pw->SearchControl(it.first));
        if (pb != nullptr)
        {
            pb->SetState(STATE_ENABLE, it.second != m_spawningType);
        }
    }
}

void CDebugMenu::DestroyInterface()
{
    m_interface->DeleteControl(EVENT_WINDOW7);
    m_spawningType = OBJECT_NULL;
}

bool CDebugMenu::EventProcess(const Event &event)
{
    switch (event.type)
    {
        case EVENT_DBG_STATS:
            m_engine->SetShowStats(!m_engine->GetShowStats());
            UpdateInterface();
            break;

        case EVENT_DBG_SPAWN_OBJ:
            DestroyInterface();
            CreateSpawnInterface();
            break;

        case EVENT_DBG_TELEPORT:
            if (!m_teleportActive)
            {
                if (m_main->GetSelect() != nullptr)
                    m_teleportActive = true;
                else
                    m_sound->Play(SOUND_CLICK);
            }
            else
            {
                m_teleportActive = false;
            }
            UpdateInterface();
            break;

        case EVENT_DBG_LIGHTNING:
            m_lightningActive = !m_lightningActive;
            UpdateInterface();
            break;

        case EVENT_DBG_RESOURCES:
            m_engine->SetDebugResources(!m_engine->GetDebugResources());
            UpdateInterface();
            break;

        case EVENT_DBG_GOTO:
            m_engine->SetDebugGoto(!m_engine->GetDebugGoto());
            UpdateInterface();
            break;

        case EVENT_DBG_CRASHSPHERES:
            m_main->SetDebugCrashSpheres(!m_main->GetDebugCrashSpheres());
            UpdateInterface();
            break;

        case EVENT_DBG_LIGHTS:
            m_engine->SetDebugLights(!m_engine->GetDebugLights());
            UpdateInterface();
            break;

        case EVENT_DBG_LIGHTS_DUMP:
            m_engine->DebugDumpLights();
            break;


        case EVENT_SPAWN_CANCEL:
            DestroyInterface();
            CreateInterface();
            break;

        case EVENT_SPAWN_ME:
        case EVENT_SPAWN_WHEELEDGRABBER:
        case EVENT_SPAWN_WHEELEDSHOOTER:
        case EVENT_SPAWN_PHAZERSHOOTER:
        case EVENT_SPAWN_BOTFACTORY:
        case EVENT_SPAWN_CONVERTER:
        case EVENT_SPAWN_DERRICK:
        case EVENT_SPAWN_POWERSTATION:
        case EVENT_SPAWN_TITANIUM:
        case EVENT_SPAWN_TITANIUMORE:
        case EVENT_SPAWN_URANIUMORE:
        case EVENT_SPAWN_POWERCELL:
        case EVENT_SPAWN_NUCLEARCELL:
            m_spawningType = SPAWN_TYPES.at(event.type);
            UpdateInterface();
            break;

        case EVENT_MOUSE_BUTTON_DOWN:
            if (event.GetData<MouseButtonEventData>()->button == MOUSE_BUTTON_LEFT)
            {
                if (m_lightningActive)
                {
                    return !HandleLightning(event.mousePos);
                }

                if (m_teleportActive)
                {
                    return !HandleTeleport(event.mousePos);
                }

                if (m_spawningType != OBJECT_NULL)
                {
                    return !HandleSpawnObject(m_spawningType, event.mousePos);
                }
            }
            break;

        case EVENT_MOUSE_MOVE:
            if (m_spawningType != OBJECT_NULL || m_teleportActive || m_lightningActive)
            {
                return false;
            }
            break;

        case EVENT_FRAME:
            HandleFrameUpdate(event);
            break;

        case EVENT_KEY_DOWN:
            if (event.GetData<KeyEventData>()->key == KEY(c) && (event.kmodState & KMOD_CTRL) != 0)
            {
                if (IsActive())
                {
                    return !HandleCopy(event.mousePos);
                }
            }

        default:
            break;
    }
    return true;
}

bool CDebugMenu::HandleSpawnObject(ObjectType type, Math::Point mousePos)
{
    Math::Vector pos;
    if (m_engine->DetectObject(mousePos, pos, true) == -1)
    {
        m_sound->Play(SOUND_CLICK, 1.0f, 0.5f);
        return false;
    }

    ObjectCreateParams params;
    params.pos = pos;
    params.type = type;
    params.power = 100.0f;
    m_objMan->CreateObject(params);

    // Update shortcuts in the upper-left corner
    m_main->CreateShortcuts();

    m_sound->Play(SOUND_RADAR, 2.0f, 1.5f);

    return true;
}

bool CDebugMenu::HandleLightning(Math::Point mousePos)
{
    Math::Vector pos;
    if (m_engine->DetectObject(mousePos, pos, true) == -1)
    {
        m_sound->Play(SOUND_CLICK, 1.0f, 0.5f);
        return false;
    }

    m_engine->GetLightning()->StrikeAtPos(pos);

    return true;
}

bool CDebugMenu::HandleTeleport(Math::Point mousePos)
{
    CObject* select = m_main->GetSelect();

    Math::Vector pos;
    if (m_engine->DetectObject(mousePos, pos, true) == -1 || !m_engine->GetTerrain()->AdjustToFloor(pos) || select == nullptr)
    {
        m_sound->Play(SOUND_CLICK, 1.0f, 0.5f);
        m_teleportActive = false;
        UpdateInterface();
        return false;
    }

    select->SetPosition(pos);

    m_sound->Play(SOUND_BUILD, 4.0f, 0.75f);
    m_sound->Play(SOUND_BUILD, pos, 4.0f, 0.75f);

    m_teleportActive = false;
    UpdateInterface();

    return true;
}

void CDebugMenu::HandleFrameUpdate(const Event &event)
{
    std::string str = "-";
    Math::Vector pos;
    int obj;
    if ((obj = m_engine->DetectObject(event.mousePos, pos, true)) != -1)
        str = StrUtils::Format("pos=% 3.2f; % 3.2f    height=% 3.2f    objId=% 4d", pos.x / g_unit, pos.z / g_unit, pos.y / g_unit, obj);

    CLabel* pl = static_cast<CLabel*>(m_interface->SearchControl(EVENT_LABEL19));
    if (pl == nullptr) return;
    pl->SetName(str.c_str());
}

bool CDebugMenu::HandleCopy(Math::Point mousePos)
{
    Math::Vector pos;
    if (m_engine->DetectObject(mousePos, pos, true) == -1)
    {
        m_sound->Play(SOUND_CLICK, 1.0f, 0.5f);
        return false;
    }

    std::string str = StrUtils::Format("pos=%.2f;%.2f", pos.x, pos.z);

    GetLogger()->Debug("%s\n", str.c_str());
    SDL_SetClipboardText(str.c_str());
    return true;
}

bool CDebugMenu::IsActive()
{
    return m_interface->SearchControl(EVENT_WINDOW7) != nullptr;
}

}
