/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2021, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include "ui/screen/screen_player_select.h"

#include "app/app.h"

#include "common/logger.h"
#include "core/stringutils.h"

#include "level/player_profile.h"
#include "level/robotmain.h"

#include "sound/sound.h"

#include "ui/maindialog.h"

#include "ui/controls/button.h"
#include "ui/controls/edit.h"
#include "ui/controls/gauge.h"
#include "ui/controls/group.h"
#include "ui/controls/interface.h"
#include "ui/controls/label.h"
#include "ui/controls/list.h"
#include "ui/controls/window.h"

namespace Ui
{

CScreenPlayerSelect::CScreenPlayerSelect(CMainDialog* mainDialog)
    : m_dialog(mainDialog)
{
}

void CScreenPlayerSelect::CreateInterface()
{
    CWindow*        pw;
    CEdit*          pe;
    CLabel*         pl;
    CButton*        pb;
    CList*          pli;
    CGroup*         pg;
    Math::Point     pos, ddim;
    std::string     name;

    pos.x = 0.10f;
    pos.y = 0.10f;
    ddim.x = 0.80f;
    ddim.y = 0.80f;
    pw = m_interface->CreateWindows(pos, ddim, 12, EVENT_WINDOW5);
    GetResource(RES_TEXT, RT_TITLE_NAME, name);
    pw->SetName(name);

    pos.x  = 0.10f;
    pos.y  = 0.40f;
    ddim.x = 0.50f;
    ddim.y = 0.50f;
    pw->CreateGroup(pos, ddim, 5, EVENT_INTERFACE_GLINTl);  // orange corner
    pos.x  = 0.40f;
    pos.y  = 0.10f;
    ddim.x = 0.50f;
    ddim.y = 0.50f;
    pw->CreateGroup(pos, ddim, 4, EVENT_INTERFACE_GLINTr);  // blue corner

    pos.x =  60.0f/640.0f;
    pos.y = 313.0f/480.0f;
    ddim.x = 120.0f/640.0f;
    ddim.y =  32.0f/480.0f;
    GetResource(RES_EVENT, EVENT_INTERFACE_NLABEL, name);
    pl = pw->CreateLabel(pos, ddim, -1, EVENT_INTERFACE_NLABEL, name);
    pl->SetTextAlign(Gfx::TEXT_ALIGN_RIGHT);

    pos.x = 200.0f/640.0f;
    pos.y = 320.0f/480.0f;
    ddim.x = 160.0f/640.0f;
    ddim.y =  32.0f/480.0f;
    pg = pw->CreateGroup(pos, ddim, 7, EVENT_LABEL1);
    pg->SetState(STATE_SHADOW);

    pos.x = 207.0f/640.0f;
    pos.y = 328.0f/480.0f;
    ddim.x = 144.0f/640.0f;
    ddim.y =  18.0f/480.0f;
    pe = pw->CreateEdit(pos, ddim, 0, EVENT_INTERFACE_NEDIT);
    pe->SetMaxChar(15);
    if(m_main->GetPlayerProfile() != nullptr)
    {
        name = m_main->GetPlayerProfile()->GetName();
    }
    else
    {
        name = CPlayerProfile::GetLastName();
    }
    pe->SetText(name.c_str());
    pe->SetCursor(name.length(), 0);
    m_interface->SetFocus(pe);

    pos.x = 380.0f/640.0f;
    pos.y = 320.0f/480.0f;
    ddim.x =100.0f/640.0f;
    ddim.y = 32.0f/480.0f;
    pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_NOK);
    pb->SetState(STATE_SHADOW);

    pos.x = 380.0f/640.0f;
    pos.y = 250.0f/480.0f;
    ddim.x =100.0f/640.0f;
    ddim.y = 52.0f/480.0f;
    pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_PERSO);
    pb->SetState(STATE_SHADOW);

    pos.x = 200.0f/640.0f;
    pos.y = 150.0f/480.0f;
    ddim.x = 160.0f/640.0f;
    ddim.y = 160.0f/480.0f;
    pli = pw->CreateList(pos, ddim, 0, EVENT_INTERFACE_NLIST);
    pli->SetState(STATE_SHADOW);

    pos.x = 200.0f/640.0f;
    pos.y = 100.0f/480.0f;
    ddim.x = 160.0f/640.0f;
    ddim.y =  32.0f/480.0f;
    pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_NDELETE);
    pb->SetState(STATE_SHADOW);

    SetBackground("textures/interface/interface.png");
    CreateVersionDisplay();

    ReadNameList();
    UpdateNameList();
    UpdateNameControl();
}

bool CScreenPlayerSelect::EventProcess(const Event &event)
{
    CWindow* pw;
    CList*   pl;
    std::string name;
    std::string gamer;

    switch( event.type )
    {
        case EVENT_KEY_DOWN:
        {
            auto data = event.GetData<KeyEventData>();
            if (data->key == KEY(RETURN) || data->key == KEY(ESCAPE))
            {
                NameSelect();
            }
            break;
        }

        case EVENT_INTERFACE_NEDIT:
            UpdateNameList();
            UpdateNameControl();
            break;

        case EVENT_INTERFACE_NLIST:
            UpdateNameEdit();
            break;

        case EVENT_INTERFACE_NOK:
            NameSelect();
            break;

        case EVENT_INTERFACE_PERSO:
            NameSelect();
            m_main->ChangePhase(PHASE_APPEARANCE);
            break;

        case EVENT_INTERFACE_NDELETE:
            pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
            if ( pw == nullptr )  break;
            pl = static_cast<CList*>(pw->SearchControl(EVENT_INTERFACE_NLIST));
            if ( pl == nullptr )  break;

            GetResource(RES_TEXT, RT_DIALOG_DELGAME, name);
            gamer = pl->GetItemName(pl->GetSelect());
            m_dialog->StartQuestion(
                StrUtils::Format(name.c_str(), gamer.c_str()), true, false, false,
                [&]()
                {
                    NameDelete();
                }
            );
            break;

        default:
            return true;
    }
    return false;
}

// Updates the list of players after checking the files on disk.

void CScreenPlayerSelect::ReadNameList()
{
    CWindow* pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if (pw == nullptr) return;
    CList* pl = static_cast<CList*>(pw->SearchControl(EVENT_INTERFACE_NLIST));
    if (pl == nullptr) return;
    pl->Flush();

    auto players = CPlayerProfile::GetPlayerList();
    for (int i = 0; i < static_cast<int>(players.size()); ++i)
    {
        pl->SetItemName(i, players.at(i));
    }
}

// Updates the controls of the players.

void CScreenPlayerSelect::UpdateNameControl()
{
    CWindow*    pw;
    CList*      pl;
    CButton*    pb;
    CEdit*      pe;
    std::string name;
    int         total, sel;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == nullptr )  return;
    pl = static_cast<CList*>(pw->SearchControl(EVENT_INTERFACE_NLIST));
    if ( pl == nullptr )  return;
    pe = static_cast<CEdit*>(pw->SearchControl(EVENT_INTERFACE_NEDIT));
    if ( pe == nullptr )  return;

    total = pl->GetTotal();
    sel   = pl->GetSelect();
    name = pe->GetText(100);
    StrUtils::Trim(name);

    pb = static_cast<CButton*>(pw->SearchControl(EVENT_INTERFACE_NDELETE));
    if ( pb != nullptr )
    {
        pb->SetState(STATE_ENABLE, total>0 && sel!=-1);
    }

    pb = static_cast<CButton*>(pw->SearchControl(EVENT_INTERFACE_NOK));
    if ( pb != nullptr )
    {
        pb->SetState(STATE_ENABLE, !name.empty() || sel!=-1);
    }

    pb = static_cast<CButton*>(pw->SearchControl(EVENT_INTERFACE_PERSO));
    if ( pb != nullptr )
    {
        pb->SetState(STATE_ENABLE, !name.empty() || sel!=-1);
    }
}

// Updates the list of players by name frape.

void CScreenPlayerSelect::UpdateNameList()
{
    CWindow*    pw;
    CList*      pl;
    CEdit*      pe;
    std::string name;
    int         total, i;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == nullptr )  return;
    pl = static_cast<CList*>(pw->SearchControl(EVENT_INTERFACE_NLIST));
    if ( pl == nullptr )  return;
    pe = static_cast<CEdit*>(pw->SearchControl(EVENT_INTERFACE_NEDIT));
    if ( pe == nullptr )  return;

    name = pe->GetText(100);
    total = pl->GetTotal();

    for ( i=0 ; i<total ; i++ )
    {
        // TODO: stricmp?
        if ( name == pl->GetItemName(i) )
        {
            pl->SetSelect(i);
            pl->ShowSelect(false);
            return;
        }
    }

    pl->SetSelect(-1);
}

// Updates the player's name and function of the selected list.

void CScreenPlayerSelect::UpdateNameEdit()
{
    CWindow*    pw;
    CList*      pl;
    CEdit*      pe;
    std::string name;
    int         sel;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == nullptr )  return;
    pl = static_cast<CList*>(pw->SearchControl(EVENT_INTERFACE_NLIST));
    if ( pl == nullptr )  return;
    pe = static_cast<CEdit*>(pw->SearchControl(EVENT_INTERFACE_NEDIT));
    if ( pe == nullptr )  return;

    sel = pl->GetSelect();
    if ( sel == -1 )
    {
        pe->SetText("");
        pe->SetCursor(0, 0);
    }
    else
    {
        name = pl->GetItemName(sel);
        pe->SetText(name.c_str());
        pe->SetCursor(name.length(), 0);
    }

    UpdateNameControl();
}

// Selects a player.

void CScreenPlayerSelect::NameSelect()
{
    CWindow*    pw;
    CList*      pl;
    CEdit*      pe;
    std::string name;
    int         sel;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == nullptr )  return;
    pl = static_cast<CList*>(pw->SearchControl(EVENT_INTERFACE_NLIST));
    if ( pl == nullptr )  return;
    pe = static_cast<CEdit*>(pw->SearchControl(EVENT_INTERFACE_NEDIT));
    if ( pe == nullptr )  return;

    name = pe->GetText(100);
    sel  = pl->GetSelect();

    if ( sel == -1 )
    {
        NameCreate();
    }
    else
    {
        m_main->SelectPlayer(pl->GetItemName(sel));
    }

    m_main->ChangePhase(PHASE_MAIN_MENU);
}

// Creates a new player.

bool CScreenPlayerSelect::NameCreate()
{
    CWindow*    pw;
    CEdit*      pe;

    GetLogger()->Info("Creating new player\n");
    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == nullptr )  return false;
    pe = static_cast<CEdit*>(pw->SearchControl(EVENT_INTERFACE_NEDIT));
    if ( pe == nullptr )  return false;

    std::string name;
    name = pe->GetText(100);
    StrUtils::Trim(name);
    if ( name.empty() )
    {
        m_sound->Play(SOUND_TZOING);
        return false;
    }

    m_main->SelectPlayer(name);
    m_main->GetPlayerProfile()->Create();

    return true;
}

// Removes a player.

void CScreenPlayerSelect::NameDelete()
{
    CWindow* pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if (pw == nullptr) return;
    CList* pl = static_cast<CList*>(pw->SearchControl(EVENT_INTERFACE_NLIST));
    if (pl == nullptr) return;

    int sel = pl->GetSelect();
    if (sel == -1)
    {
        m_sound->Play(SOUND_TZOING);
        return;
    }

    std::string gamer = pl->GetItemName(sel);

    m_main->SelectPlayer(gamer);
    if (!m_main->GetPlayerProfile()->Delete())
    {
        m_sound->Play(SOUND_TZOING);
        return;
    }

    pl->SetSelect(-1);
    CEdit* pe = static_cast<CEdit*>(pw->SearchControl(EVENT_INTERFACE_NEDIT));
    if (pe != nullptr)
    {
        pe->SetText("");
    }

    ReadNameList();
    UpdateNameList();
    UpdateNameControl();
}

} // namespace Ui
