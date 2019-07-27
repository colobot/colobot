/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2019, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include "ui/screen/screen_setup_mods.h"

#include "app/app.h"
#include "app/pathman.h"

#include "common/restext.h"
#include "common/config.h"
#include "common/logger.h"
#include "common/settings.h"

#include "common/resources/resourcemanager.h"
#include "level/parser/parser.h"

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

CScreenSetupMods::CScreenSetupMods()
{
}

void CScreenSetupMods::SetActive()
{
    m_tab = PHASE_SETUPm;
}

void CScreenSetupMods::CreateInterface()
{
    CWindow*        pw;
    CLabel*         pl;
    CButton*        pb;
    CList*          pli;
    Math::Point     pos, ddim;
    std::string     name;

    CScreenSetup::CreateInterface();
    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == nullptr )  return;

    // Displays a list of unloaded mods:
    pos.x = ox+sx*3;
    pos.y = oy+sy*9;
    ddim.x = dim.x*6;
    ddim.y = dim.y*1;
    GetResource(RES_TEXT, RT_MODS_UNLOADED, name);
    pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL11, name);
    pl->SetTextAlign(Gfx::TEXT_ALIGN_LEFT);

    pos.y = oy+sy*3.75f;
    ddim.x = dim.x*6.5f;
    ddim.y = dim.y*6.05f;
    pli = pw->CreateList(pos, ddim, 0, EVENT_INTERFACE_MODS_UNLOADED);
    pli->SetState(STATE_SHADOW);
    UpdateUnloadedModList();

    // Displays a list of loaded mods:
    pos.x = ox+sx*9.5f;
    pos.y = oy+sy*9;
    ddim.x = dim.x*6;
    ddim.y = dim.y*1;
    GetResource(RES_TEXT, RT_MODS_LOADED, name);
    pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL12, name);
    pl->SetTextAlign(Gfx::TEXT_ALIGN_LEFT);

    pos.y = oy+sy*3.75f;
    ddim.x = dim.x*6.5f;
    ddim.y = dim.y*6.05f;
    pli = pw->CreateList(pos, ddim, 0, EVENT_INTERFACE_MODS_LOADED);
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
}
bool CScreenSetupMods::EventProcess(const Event &event)
{
    CWindow*               pw;
    CButton*               pb;
    CList*                 pl;
    int                    result;
    std::string            modName, modPath;

    if (!CScreenSetup::EventProcess(event)) return false;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == nullptr )  return false;

    if (event.type == EVENT_INTERFACE_LOAD)
    {
        pl = static_cast<CList*>(pw->SearchControl(EVENT_INTERFACE_MODS_UNLOADED));
        if ( pl == nullptr )  return false;
        modName = pl->GetItemName(pl->GetSelect());
        LoadMod(modName);

        m_app->Reload();
        m_main->ChangePhase(PHASE_SETUPm);
    }
    if (event.type == EVENT_INTERFACE_UNLOAD)
    {
        pl = static_cast<CList*>(pw->SearchControl(EVENT_INTERFACE_MODS_LOADED));
        if ( pl == nullptr )  return false;
        modName = pl->GetItemName(pl->GetSelect());
        UnloadMod(modName);

        m_app->Reload();
        m_main->ChangePhase(PHASE_SETUPm);
    }
    if (event.type == EVENT_INTERFACE_MODS_DIR)
    {
        modPath = CResourceManager::GetSaveLocation() + "/" + "mods";
        #if defined(PLATFORM_WINDOWS)
            std::replace(modPath.begin(), modPath.end(), '/', '\\');
            result = system(("explorer \""+modPath+"\"").c_str());
        #elif defined(PLATFORM_LINUX)
            result = system(("xdg-open \""+modPath+"\"").c_str());
        #elif defined(PLATFORM_MACOSX)
            result = system(("open \""+modPath+"\"").c_str()); //TODO: Test on macOS
        #endif
        if (result == -1)
        {
            GetLogger()->Error("Failed to open Mods directory! Does directory exists?\n");
        }
    }
    switch (event.type)
    {
        case EVENT_INTERFACE_MODS_UNLOADED:
            pl = static_cast<CList*>(pw->SearchControl(EVENT_INTERFACE_MODS_LOADED));
            if ( pl == nullptr )  break;

            pb = static_cast<CButton*>(pw->SearchControl(EVENT_INTERFACE_UNLOAD));
            if ( pb == nullptr )  break;
            pl->SetSelect(-1);
            pb->ClearState(STATE_ENABLE);

            pb = static_cast<CButton*>(pw->SearchControl(EVENT_INTERFACE_LOAD));
            if ( pb == nullptr )  break;
            pb->SetState(STATE_ENABLE);
            break;

        case EVENT_INTERFACE_MODS_LOADED:
            pl = static_cast<CList*>(pw->SearchControl(EVENT_INTERFACE_MODS_UNLOADED));
            if ( pl == nullptr )  break;

            pb = static_cast<CButton*>(pw->SearchControl(EVENT_INTERFACE_LOAD));
            if ( pb == nullptr )  break;
            pl->SetSelect(-1);
            pb->ClearState(STATE_ENABLE);

            pb = static_cast<CButton*>(pw->SearchControl(EVENT_INTERFACE_UNLOAD));
            if ( pb == nullptr )  break;
            pb->SetState(STATE_ENABLE);
            break;

        case EVENT_INTERFACE_WORKSHOP:
            #if defined(PLATFORM_WINDOWS)
                result = system("rundll32 url.dll,FileProtocolHandler \"https://www.moddb.com/games/colobot-gold-edition\"");
            #elif defined(PLATFORM_LINUX)
                result = system("xdg-open \"https://www.moddb.com/games/colobot-gold-edition\"");
            #elif defined(PLATFORM_MACOSX)
                result = system("open \"https://www.moddb.com/games/colobot-gold-edition\""); //TODO: Test on macOS
            #endif
            if (result == -1)
            {
                GetLogger()->Error("Failed to open Workshop page! Is any Web Broswer installed?\n");
            }
            break;
        default:
            return true;
    }
    return false;
}

void CScreenSetupMods::UnloadMod(std::string modName)
{
    std::string            modPath, modPathRaw, disabled = "~";

    modPathRaw = CResourceManager::GetSaveLocation() + "/" + "mods" + "/";
    modPath = modPathRaw.c_str();

    m_pathManager->RemoveMod(modPath+modName);
    boost::filesystem::rename(modPath+modName, modPath+disabled+modName);
}

void CScreenSetupMods::LoadMod(std::string modName)
{
    std::string            modPath, modPathRaw, disabled = "~";

    modPathRaw = CResourceManager::GetSaveLocation() + "/" + "mods" + "/";
    modPath = modPathRaw.c_str();

    boost::filesystem::rename(modPath+disabled+modName, modPath+modName);
    m_pathManager->AddMod(modPath+modName);
}

void CScreenSetupMods::UpdateUnloadedModList()
{
    CWindow*    pw;
    CList*      pl;
    int         i = 0;
    std::string modName;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == nullptr )  return;

    pl = static_cast<CList*>(pw->SearchControl(EVENT_INTERFACE_MODS_UNLOADED));
    if ( pl == nullptr )  return;
    pl->Flush();

    auto modsDir = CResourceManager::ListDirectories("mods/");
    std::sort(modsDir.begin(), modsDir.end());

    for(auto const& modNameRaw : modsDir)
    {
        modName = modNameRaw;
        std::string::size_type enabled;
        enabled = modName.find('~');
        if (enabled != std::string::npos)
        {
            modName.erase(0,1);
            pl->SetItemName(i++, modName);
        }
    }
    pl->ShowSelect(false);  // shows the selected columns
}
void CScreenSetupMods::UpdateLoadedModList()
{
    CWindow*    pw;
    CList*      pl;
    int         i = 0;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == nullptr )  return;

    pl = static_cast<CList*>(pw->SearchControl(EVENT_INTERFACE_MODS_LOADED));
    if ( pl == nullptr )  return;
    pl->Flush();

    auto modsDir = CResourceManager::ListDirectories("mods/");
    std::sort(modsDir.begin(), modsDir.end());

    for(auto const &modName : modsDir)
    {
        std::string::size_type enabled;
        enabled = modName.find('~');
        if (enabled == std::string::npos)
            pl->SetItemName(i++, modName);
    }
    pl->ShowSelect(false);  // shows the selected columns
}
} // namespace Ui
