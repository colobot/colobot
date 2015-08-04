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
#include "ui/screen/screen_player_select.h"

#include "app/app.h"

#include "common/logger.h"

#include "object/player_profile.h"

#include "ui/button.h"
#include "ui/edit.h"
#include "ui/interface.h"
#include "ui/label.h"
#include "ui/list.h"
#include "ui/maindialog.h"
#include "ui/window.h"

namespace Ui {

CScreenPlayerSelect::CScreenPlayerSelect()
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

    pos.x = 380.0f/640.0f;
    pos.y = 100.0f/480.0f;
    ddim.x =100.0f/640.0f;
    ddim.y = 32.0f/480.0f;
    pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_NCANCEL);
    pb->SetState(STATE_SHADOW);

    ReadNameList();
    UpdateNameList();
    UpdateNameControl();

    SetBackground("textures/interface/interface.png");
}

bool CScreenPlayerSelect::EventProcess(const Event &event)
{
    CWindow* pw;
    CButton* pb;
    CList*   pl;

    switch( event.type )
    {
        case EVENT_KEY_DOWN:
            if ( event.key.key == KEY(RETURN) )
            {
                NameSelect();
            }
            if ( event.key.key == KEY(ESCAPE) )
            {
                pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
                if ( pw == 0 )  break;
                pb = static_cast<CButton*>(pw->SearchControl(EVENT_INTERFACE_NCANCEL));
                if ( pb == 0 )  break;
                if ( pb->TestState(STATE_ENABLE) )
                {
                    m_main->ChangePhase(PHASE_MAIN_MENU);
                }
            }
            break;

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
            m_main->ChangePhase(PHASE_APPERANCE);
            break;

        case EVENT_INTERFACE_NCANCEL:
            m_main->ChangePhase(PHASE_MAIN_MENU);
            break;

        case EVENT_INTERFACE_NDELETE:
            pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
            if ( pw == 0 )  break;
            pl = static_cast<CList*>(pw->SearchControl(EVENT_INTERFACE_NLIST));
            if ( pl == 0 )  break;
            CMainDialog::GetInstancePointer()->StartDeleteGame(pl->GetItemName(pl->GetSelect()));
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
        pl->SetItemName(i, players.at(i).c_str());
    }
}

// Updates the controls of the players.

void CScreenPlayerSelect::UpdateNameControl()
{
    CWindow*    pw;
    CList*      pl;
    CButton*    pb;
    CEdit*      pe;
    char        name[100];
    int         total, sel;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == 0 )  return;
    pl = static_cast<CList*>(pw->SearchControl(EVENT_INTERFACE_NLIST));
    if ( pl == 0 )  return;
    pe = static_cast<CEdit*>(pw->SearchControl(EVENT_INTERFACE_NEDIT));
    if ( pe == 0 )  return;

    std::string gamer = m_main->GetPlayerProfile()->GetName();
    total = pl->GetTotal();
    sel   = pl->GetSelect();
    pe->GetText(name, 100);

    pb = static_cast<CButton*>(pw->SearchControl(EVENT_INTERFACE_NCANCEL));
    if ( pb != 0 )
    {
        pb->SetState(STATE_ENABLE, !gamer.empty());
    }

    pb = static_cast<CButton*>(pw->SearchControl(EVENT_INTERFACE_NDELETE));
    if ( pb != 0 )
    {
        pb->SetState(STATE_ENABLE, total>0 && sel!=-1);
    }

    pb = static_cast<CButton*>(pw->SearchControl(EVENT_INTERFACE_NOK));
    if ( pb != 0 )
    {
        pb->SetState(STATE_ENABLE, name[0]!=0 || sel!=-1);
    }

    pb = static_cast<CButton*>(pw->SearchControl(EVENT_INTERFACE_PERSO));
    if ( pb != 0 )
    {
        pb->SetState(STATE_ENABLE, name[0]!=0 || sel!=-1);
    }
}

// Updates the list of players by name frape.

void CScreenPlayerSelect::UpdateNameList()
{
    CWindow*    pw;
    CList*      pl;
    CEdit*      pe;
    char        name[100];
    int         total, i;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == 0 )  return;
    pl = static_cast<CList*>(pw->SearchControl(EVENT_INTERFACE_NLIST));
    if ( pl == 0 )  return;
    pe = static_cast<CEdit*>(pw->SearchControl(EVENT_INTERFACE_NEDIT));
    if ( pe == 0 )  return;

    pe->GetText(name, 100);
    total = pl->GetTotal();

    for ( i=0 ; i<total ; i++ )
    {
        // TODO: stricmp?
        if ( strcmp(name, pl->GetItemName(i)) == 0 )
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
    char*       name;
    int         sel;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == 0 )  return;
    pl = static_cast<CList*>(pw->SearchControl(EVENT_INTERFACE_NLIST));
    if ( pl == 0 )  return;
    pe = static_cast<CEdit*>(pw->SearchControl(EVENT_INTERFACE_NEDIT));
    if ( pe == 0 )  return;

    sel = pl->GetSelect();
    if ( sel == -1 )
    {
        pe->SetText("");
        pe->SetCursor(0, 0);
    }
    else
    {
        name = pl->GetItemName(sel);
        pe->SetText(name);
        pe->SetCursor(strlen(name), 0);
    }

    UpdateNameControl();
}

// Selects a player.

void CScreenPlayerSelect::NameSelect()
{
    CWindow*    pw;
    CList*      pl;
    CEdit*      pe;
    char        name[100];
    int         sel;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == 0 )  return;
    pl = static_cast<CList*>(pw->SearchControl(EVENT_INTERFACE_NLIST));
    if ( pl == 0 )  return;
    pe = static_cast<CEdit*>(pw->SearchControl(EVENT_INTERFACE_NEDIT));
    if ( pe == 0 )  return;

    pe->GetText(name, 100);
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
    char        name[100];
    char        c;
    int         len, i, j;

    GetLogger()->Info("Creating new player\n");
    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == 0 )  return false;
    pe = static_cast<CEdit*>(pw->SearchControl(EVENT_INTERFACE_NEDIT));
    if ( pe == 0 )  return false;

    pe->GetText(name, 100);
    if ( name[0] == 0 )
    {
        m_sound->Play(SOUND_TZOING);
        return false;
    }

    len = strlen(name);
    j = 0;
    for ( i=0 ; i<len ; i++ )
    {
        c = GetNoAccent(GetToLower(name[i]));
        if ( (c >= '0' && c <= '9') ||
             (c >= 'a' && c <= 'z') ||
             c == ' ' ||
             c == '-' ||
             c == '_' ||
             c == '.' ||
             c == ',' ||
             c == '\'' )
        {
            name[j++] = name[i];
        }
    }
    name[j] = 0;
    if ( j == 0 )
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

    char* gamer = pl->GetItemName(sel);

    m_main->SelectPlayer(gamer);
    if (!m_main->GetPlayerProfile()->Delete())
    {
        m_sound->Play(SOUND_TZOING);
        return;
    }

    pl->SetSelect(-1);

    ReadNameList();
    UpdateNameList();
    UpdateNameControl();
}

} // namespace Ui
