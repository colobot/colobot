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

#include "ui/screen/screen_io.h"

#include "common/logger.h"
#include "common/restext.h"
#include "common/stringutils.h"

#include "graphics/engine/engine.h"

#include "level/player_profile.h"
#include "level/robotmain.h"

#include "level/parser/parser.h"

#include "sound/sound.h"

#include "ui/controls/button.h"
#include "ui/controls/edit.h"
#include "ui/controls/image.h"
#include "ui/controls/interface.h"
#include "ui/controls/list.h"
#include "ui/controls/window.h"

#include "ui/screen/screen_level_list.h"

#include <ctime>
#include <cstring>

namespace Ui
{

CScreenIO::CScreenIO(CScreenLevelList* screenLevelList)
    : m_screenLevelList(screenLevelList)
{}


// Builds the file name by default.

void CScreenIO::IOReadName()
{
    CWindow*    pw;
    CEdit*      pe;
    std::string resume;
    char        line[100];
    std::string name;
    time_t      now;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == nullptr )  return;
    pe = static_cast<CEdit*>(pw->SearchControl(EVENT_INTERFACE_IONAME));
    if ( pe == nullptr )  return;

    resume = GetLevelCategoryDir(m_main->GetLevelCategory()) + " " + StrUtils::ToString<int>(m_main->GetLevelChap());

    CLevelParser levelParser(m_main->GetLevelCategory(), m_main->GetLevelChap(), 0);
    try
    {
        levelParser.Load();
        resume = levelParser.Get("Title")->GetParam("resume")->AsString();
    }
    catch (CLevelParserException& e)
    {
        GetLogger()->Warn("%%", e.what());
    }

    time(&now);
    strftime(line, 99, "%y.%m.%d %H:%M", localtime(&now));
    name = StrUtils::Format("%s - %s %d", line, resume.c_str(), m_main->GetLevelRank());

    pe->SetText(name);
    pe->SetCursor(name.length(), 0);
    m_interface->SetFocus(pe);
}

// Updates the list of games recorded on disk.

void CScreenIO::IOReadList(bool isWrite)
{
    CWindow* pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if (pw == nullptr) return;
    CList* pl = static_cast<CList*>(pw->SearchControl(EVENT_INTERFACE_IOLIST));
    if (pl == nullptr) return;

    pl->Flush();

    m_saveList.clear();
    for(const SavedScene& save : m_main->GetPlayerProfile()->GetSavedSceneList())
    {
        pl->SetItemName(m_saveList.size(), save.name);
        m_saveList.push_back(save.path);
    }

    // invalid index
    if ( isWrite )
    {
        std::string nameStr;
        GetResource(RES_TEXT, RT_IO_NEW, nameStr);
        pl->SetItemName(m_saveList.size(), nameStr);
    }

    pl->SetSelect(m_saveList.size());
    pl->ShowSelect(false);  // shows the selected columns

    for (unsigned int i = 0; i < m_saveList.size(); i++)
    {
        m_engine->DeleteTexture(m_saveList.at(i) + "/screen.png");
    }
}

// Updates the buttons according to the selected part in the list.

void CScreenIO::IOUpdateList(bool isWrite)
{
    CWindow*    pw;
    CList*      pl;
    CButton*    pb;
    CImage*     pi;
    int         sel, max;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == nullptr )  return;
    pl = static_cast<CList*>(pw->SearchControl(EVENT_INTERFACE_IOLIST));
    if ( pl == nullptr )  return;
    pi = static_cast<CImage*>(pw->SearchControl(EVENT_INTERFACE_IOIMAGE));
    if ( pi == nullptr )  return;

    sel = pl->GetSelect();
    max = pl->GetTotal();

    // enable/disable buttons if we have selected a game
    pb = static_cast<CButton*>(pw->SearchControl(EVENT_INTERFACE_IODELETE));
    if ( pb != nullptr )
    {
        pb->SetState(STATE_ENABLE, (!isWrite && sel < max) || (isWrite && sel < max - 1));
    }

    pb = static_cast<CButton*>(pw->SearchControl(EVENT_INTERFACE_IOREAD));
    if ( pb != nullptr )
    {
        pb->SetState(STATE_ENABLE, sel < max);
    }

    if (m_saveList.size() <= static_cast<unsigned int>(sel))
    {
        pi->SetFilename(""); // clear screenshot, nothing selected or New save selected
        return;
    }

    std::filesystem::path filename = StrUtils::ToPath(m_saveList.at(sel)) / "screen.png";
    if ( isWrite )
    {
        if ( sel < max-1 )
        {
            pi->SetFilename(filename);
        }
    }
    else
    {
        pi->SetFilename(filename);
    }
}

// Deletes the selected scene.

void CScreenIO::IODeleteScene()
{
    CWindow* pw;
    CList*   pl;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == nullptr )  return;
    pl = static_cast<CList*>(pw->SearchControl(EVENT_INTERFACE_IOLIST));
    if ( pl == nullptr )  return;

    int sel = pl->GetSelect();
    if (sel < 0 || sel >= static_cast<int>(m_saveList.size())) return;

    if (!m_main->GetPlayerProfile()->DeleteScene(m_saveList.at(sel)))
    {
        m_sound->Play(SOUND_TZOING);
        return;
    }
}

// clears filename only to leave letter or numbers
static std::string clearName(std::string name)
{
    std::string ret;
    for (int i = 0; i < static_cast<int>(name.size()); i++)
    {
        if (isalnum(name[i]))
        {
            ret += name[i];
        }
    }
    return ret;
}


// Writes the scene.
void CScreenIO::IOWriteScene()
{
    CWindow*    pw;
    CList*      pl;
    CEdit*      pe;
    std::string info;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == nullptr )  return;
    pl = static_cast<CList*>(pw->SearchControl(EVENT_INTERFACE_IOLIST));
    if ( pl == nullptr )  return;
    pe = static_cast<CEdit*>(pw->SearchControl(EVENT_INTERFACE_IONAME));
    if ( pe == nullptr )  return;

    int sel = pl->GetSelect();
    if ( sel == -1 ) return;

    info = pe->GetText(100);

    m_interface->DeleteControl(EVENT_WINDOW5);

    std::filesystem::path dir;
    if (static_cast<unsigned int>(sel) >= m_saveList.size())
    {
        dir = m_main->GetPlayerProfile()->GetSaveFile("save"+clearName(info));
    }
    else
    {
        dir = m_saveList.at(sel);
    }

    m_main->GetPlayerProfile()->SaveScene(StrUtils::ToString(dir), info);
}

// Reads the scene.

bool CScreenIO::IOReadScene()
{
    CWindow*    pw;
    CList*      pl;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == nullptr )  return false;
    pl = static_cast<CList*>(pw->SearchControl(EVENT_INTERFACE_IOLIST));
    if ( pl == nullptr )  return false;

    int sel = pl->GetSelect();
    if (sel < 0 || sel >= static_cast<int>(m_saveList.size())) return false;

    m_main->GetPlayerProfile()->LoadScene(m_saveList.at(sel));

    m_screenLevelList->SetSelection(m_main->GetLevelCategory(), m_main->GetLevelChap()-1, m_main->GetLevelRank()-1);

    return true;
}

} // namespace Ui
