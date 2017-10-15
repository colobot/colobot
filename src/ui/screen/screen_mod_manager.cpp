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

#include "ui/screen/screen_mod_manager.h"

#include "app/app.h"

#include "common/logger.h"

#include "common/settings.h"

#include "common/resources/resourcemanager.h"

#include "level/parser/parser.h"

#include "ui/maindialog.h"

#include "ui/controls/button.h"
#include "ui/controls/edit.h"
#include "ui/controls/interface.h"
#include "ui/controls/label.h"
#include "ui/controls/list.h"
#include "ui/controls/window.h"

#include <algorithm>
#include <boost/filesystem.hpp>

namespace Ui
{

CScreenModManager::CScreenModManager(CMainDialog* mainDialog)
    : m_dialog(mainDialog)
{
}

void CScreenModManager::CreateInterface()
{
    CWindow*        pw;
    CLabel*         pl;
    CButton*        pb;
    CList*          pli;
    Math::Point     pos, ddim;
    std::string     name;

    pos.x = 0.10f;
    pos.y = 0.10f;
    ddim.x = 0.80f;
    ddim.y = 0.80f;
    pw = m_interface->CreateWindows(pos, ddim, 12, EVENT_WINDOW5);
    pw->SetClosable(false);
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

    ddim.x = 0.975f/5-0.01f;
    ddim.y = 0.06f;
    pos.x = 0.115f;
    pos.y = 0.76f;
    pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_MODLIST_LOCAL);
    pb->SetState(STATE_SHADOW);
    pb->SetState(STATE_CARD);
    pb->SetState(STATE_CHECK, (m_global == false));

    pos.x += ddim.x+0.01f;
    pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_MODLIST_GLOBAL);
    pb->SetState(STATE_SHADOW);
    pb->SetState(STATE_CARD);
    pb->SetState(STATE_CHECK, (m_global == true));
    pb->ClearState(STATE_ENABLE);

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

    // Displays a list of unloaded mods:
    pos.x = ox+sx*3;
    pos.y = oy+sy*9;
    ddim.x = dim.x*7.5f;
    ddim.y = dim.y*0.6f;
    GetResource(RES_TEXT, RT_MODLIST_UNLOADED, name);
    pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL11, name);
    pl->SetTextAlign(Gfx::TEXT_ALIGN_LEFT);

    pos.y = oy+sy*3.7f;
    ddim.x = dim.x*6.5f;
    ddim.y = dim.y*6;
    pli = pw->CreateList(pos, ddim, 0, EVENT_INTERFACE_MODLIST_UNLOADED);
    pli->SetState(STATE_SHADOW);
    UpdateUnloadedModList();

    // Displays a list of loaded mods:
    pos.x = ox+sx*9.5f;
    pos.y = oy+sy*9;
    ddim.x = dim.x*7.5f;
    ddim.y = dim.y*0.6f;
    GetResource(RES_TEXT, RT_MODLIST_LOADED, name);
    pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL12, name);
    pl->SetTextAlign(Gfx::TEXT_ALIGN_LEFT);

    pos.y = oy+sy*3.7f;
    ddim.x = dim.x*6.5f;
    ddim.y = dim.y*6;
    pli = pw->CreateList(pos, ddim, 0, EVENT_INTERFACE_MODLIST_LOADED);
    pli->SetState(STATE_SHADOW);
    UpdateLoadedModList();
    pos = pli->GetPos();
    ddim = pli->GetDim();

    pos.x  = ox+sx*8.2f;
    pos.y  = oy+sy*2;
    ddim.x = dim.x*1;
    ddim.y = dim.y*1;
    pb = pw->CreateButton(pos, ddim, 40, EVENT_INTERFACE_WORKSHOP);
    pb->SetState(STATE_SHADOW);

    pos.x += dim.x*1.3f;
    pb = pw->CreateButton(pos, ddim, 57, EVENT_INTERFACE_MODS_DIR);
    pb->SetState(STATE_SHADOW);

    pos.x += dim.x*1.3f;
    ddim.x = dim.x*2.5f;
    pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_LOAD);
    pb->SetState(STATE_SHADOW);
    pb->ClearState(STATE_ENABLE);

    pos.x += dim.x*2.8f;
    pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_UNLOAD);
    pb->SetState(STATE_SHADOW);
    pb->ClearState(STATE_ENABLE);

    pos.x  = ox+sx*3;
    ddim.x = dim.x*4;
    pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_BACK);
    pb->SetState(STATE_SHADOW);

    SetBackground("textures/interface/interface.png");
    CreateVersionDisplay();
}

bool CScreenModManager::EventProcess(const Event &event)
{
    CWindow*               pw;
    CButton*               pb;
    CList*                 pl;
    int                    result;
    std::string            OFF = "~";
    std::string            modName;
    std::string            modPath;
    std::string            modPathRaw = CResourceManager::GetSaveLocation() + "/" + "mods" + "/";

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == nullptr )  return false;

    if ( event.type == pw->GetEventTypeClose() ||
         event.type == EVENT_INTERFACE_BACK    ||
         (event.type == EVENT_KEY_DOWN && event.GetData<KeyEventData>()->key == KEY(ESCAPE)) )
    {
        std::string title, htext, text;
        GetResource(RES_TEXT, RT_TITLE_APPNAME, title);
        GetResource(RES_TEXT, RT_DIALOG_MODSCHANGE_TITLE, htext);
        GetResource(RES_TEXT, RT_DIALOG_MODSCHANGE_TEXT, text);
        m_dialog->StartInformation(title, htext, text);
        m_main->ChangePhase(PHASE_MAIN_MENU);
        return false;
    }

    if (event.type == EVENT_INTERFACE_LOAD)
    {
        pl = static_cast<CList*>(pw->SearchControl(EVENT_INTERFACE_MODLIST_UNLOADED));
        if ( pl == nullptr )  return false;
        modName = pl->GetItemName(pl->GetSelect());
        modPath = modPathRaw.c_str();
        boost::filesystem::rename(modPath+OFF+modName, modPath+modName);
        m_main->ChangePhase(PHASE_MOD_MANAGER);
    }

    if (event.type == EVENT_INTERFACE_UNLOAD)
    {
        pl = static_cast<CList*>(pw->SearchControl(EVENT_INTERFACE_MODLIST_LOADED));
        if ( pl == nullptr )  return false;
        modName = pl->GetItemName(pl->GetSelect());
        modPath = modPathRaw.c_str();
        boost::filesystem::rename(modPath+modName, modPath+OFF+modName);
        m_main->ChangePhase(PHASE_MOD_MANAGER);
    }

    switch (event.type)
    {
        case EVENT_INTERFACE_MODLIST_LOCAL:
            m_global = false;
            m_main->ChangePhase(PHASE_MOD_MANAGER);
            break;

        case EVENT_INTERFACE_MODLIST_GLOBAL:
            m_global = true;
            m_main->ChangePhase(PHASE_MOD_MANAGER);
            break;

        case EVENT_INTERFACE_MODLIST_UNLOADED:
            pl = static_cast<CList*>(pw->SearchControl(EVENT_INTERFACE_MODLIST_LOADED));
            if ( pl == nullptr )  break;
            pb = static_cast<CButton*>(pw->SearchControl(EVENT_INTERFACE_UNLOAD));
            if ( pb == nullptr )  break;
            pl->SetSelect(-1);
            pb->ClearState(STATE_ENABLE);
            pb = static_cast<CButton*>(pw->SearchControl(EVENT_INTERFACE_LOAD));
            if ( pb == nullptr )  break;
            pb->SetState(STATE_ENABLE);
            break;

        case EVENT_INTERFACE_MODLIST_LOADED:
            pl = static_cast<CList*>(pw->SearchControl(EVENT_INTERFACE_MODLIST_UNLOADED));
            if ( pl == nullptr )  break;
            pb = static_cast<CButton*>(pw->SearchControl(EVENT_INTERFACE_LOAD));
            if ( pb == nullptr )  break;
            pl->SetSelect(-1);
            pb->ClearState(STATE_ENABLE);
            pb = static_cast<CButton*>(pw->SearchControl(EVENT_INTERFACE_UNLOAD));
            if ( pb == nullptr )  break;
            pb->SetState(STATE_ENABLE);
            break;

        case EVENT_INTERFACE_MODS_DIR:
            modPathRaw = CResourceManager::GetSaveLocation() + "/" + "mods";
            #ifdef _WIN32
                result = system(("start \""+modPathRaw+"\"").c_str());
            #endif
            #ifdef __linux__
                result = system(("xdg-open \""+modPathRaw+"\"").c_str());
            #endif
            #ifdef __APPLE__
                result = system(("open \""+modPathRaw+"\"").c_str());
            #endif
            if (result == -1)
            {
                GetLogger()->Error("Failed to open Mods Directory! Is directory Exists?\n");
            }
            break;

        case EVENT_INTERFACE_WORKSHOP:
            #ifdef _WIN32
                result = system("start \"https://colobot.info/forum/forumdisplay.php?fid=60\"");
            #endif
            #ifdef __linux__
                result = system("xdg-open \"https://colobot.info/forum/forumdisplay.php?fid=60\"");
            #endif
            #ifdef __APPLE__
                result = system("open \"https://colobot.info/forum/forumdisplay.php?fid=60\"");
            #endif
            if (result == -1)
            {
                GetLogger()->Error("Failed to open Workshop! Is any Web Broswer Installed?\n");
            }
            break;

        default:
            return true;
    }
    return false;
}

void CScreenModManager::UpdateUnloadedModList()
{
    CWindow*    pw;
    CList*      pl;
    int         i = 0;
    std::string modName;
    auto        modsDir = CResourceManager::ListDirectories("mods/");

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == nullptr )  return;
    pl = static_cast<CList*>(pw->SearchControl(EVENT_INTERFACE_MODLIST_UNLOADED));
    if ( pl == nullptr )  return;

    std::sort(modsDir.begin(), modsDir.end());
    pl->Flush();

    for(auto const& modNameRaw : modsDir)
    {
        modName = modNameRaw;
        std::string::size_type ON;
        ON = modName.find('~');
        if (ON != std::string::npos)
        {
            modName.erase(0,1);
            pl->SetItemName(i++, modName);
        }
    }
    pl->ShowSelect(false);  // shows the selected columns
}

void CScreenModManager::UpdateLoadedModList()
{
    CWindow*    pw;
    CList*      pl;
    int         i = 0;
    auto        modsDir = CResourceManager::ListDirectories("mods/");

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == nullptr )  return;
    pl = static_cast<CList*>(pw->SearchControl(EVENT_INTERFACE_MODLIST_LOADED));
    if ( pl == nullptr )  return;

    std::sort(modsDir.begin(), modsDir.end());
    pl->Flush();

    for(auto const &modName : modsDir)
    {
        std::string::size_type ON;
        ON = modName.find('~');
        if (ON == std::string::npos)
            pl->SetItemName(i++, modName);
    }
    pl->ShowSelect(false);  // shows the selected columns
}

} // namespace Ui
