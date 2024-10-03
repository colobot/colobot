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


#include "ui/filedialog.h"

#include "app/app.h"

#include "common/logger.h"
#include "common/restext.h"
#include "common/stringutils.h"

#include "common/resources/resourcemanager.h"

#include "level/robotmain.h"

#include "ui/controls/check.h"
#include "ui/controls/control.h"
#include "ui/controls/edit.h"
#include "ui/controls/group.h"
#include "ui/controls/interface.h"
#include "ui/controls/label.h"
#include "ui/controls/list.h"
#include "ui/controls/window.h"

#include <algorithm>
#include <stdio.h>
#include <ctime>


namespace Ui
{


// time limit for double-click on directory
const float DELAY_DBCLICK_DIR = 0.75f;


// Object's constructor.

CFileDialog::CFileDialog()
{
    m_eventQueue = CApplication::GetInstancePointer()->GetEventQueue();
    m_interface = CRobotMain::GetInstancePointer()->GetInterface();
    m_time = 0.0f;
    m_lastTimeClickDir = 0.0f;
}

// Object's destructor.

CFileDialog::~CFileDialog()
{
}

// Beginning of the display of a dialogue.

void CFileDialog::StartDialog()
{
    CWindow* pw = static_cast< CWindow* >(m_interface->SearchControl(EVENT_WINDOW0));
    if ( pw != nullptr )  pw->ClearState(STATE_ENABLE);

    pw = static_cast< CWindow* >(m_interface->SearchControl(EVENT_WINDOW1));
    if ( pw != nullptr )  pw->ClearState(STATE_ENABLE);

    pw = static_cast< CWindow* >(m_interface->SearchControl(EVENT_WINDOW2));
    if ( pw != nullptr )  pw->ClearState(STATE_ENABLE);

    pw = static_cast< CWindow* >(m_interface->SearchControl(EVENT_WINDOW3));
    if ( pw != nullptr )  pw->ClearState(STATE_ENABLE);

    pw = static_cast< CWindow* >(m_interface->SearchControl(EVENT_WINDOW4));
    if ( pw != nullptr )  pw->ClearState(STATE_ENABLE);

    pw = static_cast< CWindow* >(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw != nullptr )  pw->ClearState(STATE_ENABLE);

    // the code battle interface, EVENT_WINDOW6, must be managed seperately

    pw = static_cast< CWindow* >(m_interface->SearchControl(EVENT_WINDOW7));
    if ( pw != nullptr )  pw->ClearState(STATE_ENABLE);

    pw = static_cast< CWindow* >(m_interface->SearchControl(EVENT_WINDOW8));
    if ( pw != nullptr )  pw->ClearState(STATE_ENABLE);

    if ( m_dialogtype != CFileDialog::Type::None )
    {
        StartFileDialog();
    }
    else
    {
        m_eventQueue->AddEvent(Event(EVENT_DIALOG_STOP));
    }
}

void CFileDialog::StopDialog()
{
    CWindow* pw = static_cast< CWindow* >(m_interface->SearchControl(EVENT_WINDOW0));
    if ( pw != nullptr )  pw->SetState(STATE_ENABLE);

    pw = static_cast< CWindow* >(m_interface->SearchControl(EVENT_WINDOW1));
    if ( pw != nullptr )  pw->SetState(STATE_ENABLE);

    pw = static_cast< CWindow* >(m_interface->SearchControl(EVENT_WINDOW2));
    if ( pw != nullptr )  pw->SetState(STATE_ENABLE);

    pw = static_cast< CWindow* >(m_interface->SearchControl(EVENT_WINDOW3));
    if ( pw != nullptr )  pw->SetState(STATE_ENABLE);

    pw = static_cast< CWindow* >(m_interface->SearchControl(EVENT_WINDOW4));
    if ( pw != nullptr )  pw->SetState(STATE_ENABLE);

    pw = static_cast< CWindow* >(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw != nullptr )  pw->SetState(STATE_ENABLE);

    // the code battle interface, EVENT_WINDOW6, must be managed seperately

    pw = static_cast< CWindow* >(m_interface->SearchControl(EVENT_WINDOW7));
    if ( pw != nullptr )  pw->SetState(STATE_ENABLE);

    pw = static_cast< CWindow* >(m_interface->SearchControl(EVENT_WINDOW8));
    if ( pw != nullptr )  pw->SetState(STATE_ENABLE);

    if ( m_windowEvent != EVENT_NULL ) // the window was created?
    {
        m_interface->DeleteControl(m_windowEvent); // delete it
    }
}

void CFileDialog::SetUsePublicPrivate(bool usePublic)
{
    m_usePublicPrivate = usePublic;
}

void CFileDialog::SetPublic(bool bPublic)
{
    m_public = bPublic;
}

bool CFileDialog::GetPublic()
{
    return m_public;
}

void CFileDialog::SetPublicFolder(const std::filesystem::path& dir)
{
    m_pathPublic = dir;
}

void CFileDialog::SetPrivateFolder(const std::filesystem::path& dir)
{
    m_pathPrivate = dir;
}

void CFileDialog::SetBasePath(const std::filesystem::path& dir)
{
    m_basePath = dir;
}

const std::filesystem::path& CFileDialog::GetBasePath() const
{
    return m_basePath;
}

void CFileDialog::SetSubFolderPath(const std::filesystem::path& dir)
{
    m_subDirPath = dir;
}

const std::filesystem::path& CFileDialog::GetSubFolderPath() const
{
    return m_subDirPath;
}

void CFileDialog::SetFilename(const std::filesystem::path& filename)
{
    m_filename = filename;
}

const std::filesystem::path& CFileDialog::GetFilename() const
{
    return m_filename;
}

// Start display of Open or Save dialogue

void CFileDialog::StartFileDialog()
{
    m_captureClick = false;
    glm::vec2 pos = m_windowPos;
    glm::vec2 dim = m_windowDim;
    int icon = (m_dialogtype == CFileDialog::Type::Open) ? 14 : 13 ;

    CWindow* pw = m_interface->CreateWindows(pos, dim, icon, m_windowEvent);
    if ( pw == nullptr ) return;

    if ( m_windowEvent == EVENT_NULL )      // event type was not set?
    {
        m_windowEvent = pw->GetEventType(); // get the unique event id
    }

    pw->SetState(STATE_SHADOW);
    pw->SetMovable(true);
    pw->SetClosable(true);
    pw->SetMinDim({ 320.0f / 640.0f, (121.0f + 18.0f * 4) / 480.0f });

    if ( m_title.empty() )
    {
        if (m_dialogtype == CFileDialog::Type::Open)
            GetResource(RES_TEXT, RT_IO_OPEN, m_title);

        if (m_dialogtype == CFileDialog::Type::Save)
            GetResource(RES_TEXT, RT_IO_SAVE, m_title);

        if (m_dialogtype == CFileDialog::Type::Folder)
            GetResource(RES_TEXT, RT_IO_SELECT_DIR, m_title);
    }
    pw->SetName(m_title);

    std::string name;
    pos = { 0.0f, 0.0f };
    dim = { 0.0f, 0.0f };

    GetResource(RES_TEXT, RT_IO_LIST, name);
    CLabel* pla = pw->CreateLabel(pos, dim, 0, EVENT_DIALOG_LABEL1, name); // path label
    pla->SetTextAlign(Gfx::TEXT_ALIGN_LEFT);

    CButton* pb = pw->CreateButton(pos, dim, -1, EVENT_DIALOG_NEWDIR);     // New Folder button
    pb->SetState(STATE_SHADOW);

    CList* pli = pw->CreateList(pos, dim, 0, EVENT_DIALOG_LIST);           // file list
    pli->SetState(STATE_SHADOW);

    CEdit* pe = pw->CreateEdit(pos, dim, 0, EVENT_DIALOG_EDIT2);           // new folder edit box
    pe->ClearState(STATE_VISIBLE | STATE_ENABLE);

    if ( m_usePublicPrivate )
    {
        GetResource(RES_TEXT, RT_IO_DIR, name);
        pla = pw->CreateLabel(pos, dim, 0, EVENT_DIALOG_LABEL3, name);     // "Folder:" label
        pla->SetTextAlign(Gfx::TEXT_ALIGN_LEFT);

        CCheck* pc = pw->CreateCheck(pos, dim, 0, EVENT_DIALOG_CHECK1);    // private check box
        GetResource(RES_TEXT, RT_IO_PRIVATE, name);
        pc->SetName(name);
        pc->SetState(STATE_SHADOW);

        pc = pw->CreateCheck(pos, dim, 0, EVENT_DIALOG_CHECK2);            // public check box
        GetResource(RES_TEXT, RT_IO_PUBLIC, name);
        pc->SetName(name);
        pc->SetState(STATE_SHADOW);
    }

    pb = pw->CreateButton(pos, dim, -1, EVENT_DIALOG_CANCEL);              // Cancel button
    pb->SetState(STATE_SHADOW);

    pb = pw->CreateButton(pos, dim, -1, EVENT_DIALOG_OK);                  // open/save button
    pb->SetState(STATE_SHADOW);

    if (m_dialogtype == CFileDialog::Type::Folder)
    {
        m_selectFolderMode = true;
        AdjustDialog();
        UpdatePublic(m_public);
        PopulateList();
        UpdatePathLabel();
        UpdateSelectFolder();
        return;
    }

    GetResource(RES_TEXT, RT_IO_OPEN, name);
    if ( m_dialogtype == CFileDialog::Type::Save ) GetResource(RES_TEXT, RT_IO_SAVE, name);
    pb->SetName(name);

    GetResource(RES_TEXT, RT_IO_NAME, name);
    pla = pw->CreateLabel(pos, dim, 0, EVENT_DIALOG_LABEL2, name);         // "Name:" label
    pla->SetTextAlign(Gfx::TEXT_ALIGN_LEFT);

    pe = pw->CreateEdit(pos, dim, 0, EVENT_DIALOG_EDIT);                   // file name edit box
    pe->SetState(STATE_SHADOW);

    // create "Overwrite existing file?" controls
    CGroup* pg = pw->CreateGroup(pos, dim, 0, EVENT_DIALOG_GROUP1);   // box "Overwrite ?"
    pg->ClearState(STATE_VISIBLE | STATE_ENABLE);
    pg->SetState(STATE_SHADOW);

    pla = pw->CreateLabel(pos, dim, 0, EVENT_LABEL0, name); // label "Overwrite ?"
    pla->SetTextAlign(Gfx::TEXT_ALIGN_CENTER);
    pla->ClearState(STATE_VISIBLE | STATE_ENABLE);
    GetResource(RES_TEXT, RT_IO_REPLACE, name);
    pla->SetName(name);
    pla->SetState(STATE_SHADOW);

    pla = pw->CreateLabel(pos, dim, 0, EVENT_LABEL1, name); // filename label
    pla->SetTextAlign(Gfx::TEXT_ALIGN_CENTER);
    pla->ClearState(STATE_VISIBLE | STATE_ENABLE);

    pb = pw->CreateButton(pos, dim, -1, EVENT_BUTTON_CANCEL); // cancel
    pb->ClearState(STATE_VISIBLE | STATE_ENABLE);
    pb->SetState(STATE_SHADOW);

    pb = pw->CreateButton(pos, dim, -1, EVENT_BUTTON_OK); // ok
    pb->ClearState(STATE_VISIBLE | STATE_ENABLE);
    pb->SetState(STATE_SHADOW);

    AdjustDialog();

    SetFilenameField(pe, m_filename);
    SetFilename("");

    UpdatePublic(m_public);
    PopulateList();
    SearchList(pe->GetText(999));
    UpdateAction();
    UpdatePathLabel();

    pe->SetCursor(999, 0);
    m_interface->SetFocus(pe);
}

void CFileDialog::SetFilenameField(CEdit* edit, const std::filesystem::path& filename)
{
    std::string name = StrUtils::ToString(filename);
    if (name.length() > static_cast<unsigned int>(edit->GetMaxChar()))
    {
        if (name.substr(name.length()-5) == ".cbot")
            name = name.substr(0, name.length()-5);
        else if (name.substr(name.length()-4) == ".txt")
            name = name.substr(0, name.length()-4);
        if (name.length() > static_cast<unsigned int>(edit->GetMaxChar()))
        {
            GetLogger()->Warn("Tried to load too long filename!");
            name = name.substr(0, edit->GetMaxChar());  // truncates according to max length
        }
    }
    edit->SetText(name);
}

void CFileDialog::AdjustDialog()
{
    CWindow* pw = static_cast< CWindow* >(m_interface->SearchControl(m_windowEvent));
    if ( pw == nullptr )  return;

    glm::vec2 wpos = m_windowPos = pw->GetPos();
    glm::vec2 wdim = m_windowDim = pw->GetDim();

    pw->SetPos(wpos);  // to move the buttons on the titlebar
    pw->SetDim(wdim);

    glm::vec2 ppos(wpos.x+10.0f/640.0f, wpos.y+wdim.y-55.0f/480.0f);
    glm::vec2 ddim(wdim.x-150.0f/640.0f, 20.0f/480.0f);

    CLabel* pla = static_cast< CLabel* >(pw->SearchControl(EVENT_DIALOG_LABEL1)); // path label
    if ( pla != nullptr )
    {
        pla->SetPos(ppos);
        pla->SetDim(ddim);
        UpdatePathLabel();
    }

    ddim.x = wdim.x-20.0f/640.0f;
    int nli = static_cast<int>((wdim.y-120.0f/480.0f)/(17.0f/480.0f));
    ddim.y = nli*17.0f/480.0f+9.0f/480.0f;
    ppos.y = wpos.y+wdim.y-48.0f/480.0f-ddim.y;
    if ( m_newFolderMode ) ddim.y -= 17.5f/480.0f;

    CList* pli = static_cast< CList* >(pw->SearchControl(EVENT_DIALOG_LIST));     // file list
    if ( pli != nullptr )
    {
        pli->SetPos(ppos);
        pli->SetDim(ddim);
        pli->SetTabs(0, ddim.x-(50.0f+140.0f+16.0f)/640.0f);
        pli->SetTabs(1,  50.0f/640.0f, Gfx::TEXT_ALIGN_RIGHT);
        pli->SetTabs(2, 140.0f/640.0f);
        if (pli->GetSelect() != -1) pli->ShowSelect(false);
    }

    if ( m_newFolderMode )
    {
        ppos.y += ddim.y-3.0f/480.0f;
        ddim.y = 20.0f/480.0f;
        CEdit* pe = static_cast< CEdit* >(pw->SearchControl(EVENT_DIALOG_EDIT2)); // new folder edit box
        if ( pe != nullptr )
        {
            pe->SetPos(ppos);
            pe->SetDim(ddim);
            int nch = static_cast< int >((ddim.x*640.0f-22.0f)/5.75f);
            std::string name = pe->GetText(nch);
            pe->SetMaxChar(nch);
            pe->SetText(name);
        }
    }

    if (!m_selectFolderMode)
    {
        ppos.y = wpos.y+30.0f/480.0f;
        ddim.x = 50.0f/640.0f;
        ddim.y = 20.0f/480.0f;
        pla = static_cast< CLabel* >(pw->SearchControl(EVENT_DIALOG_LABEL2)); // "Name:" label
        if ( pla != nullptr )
        {
            pla->SetPos(ppos);
            pla->SetDim(ddim);
        }

        ppos.x += 50.0f/640.0f;
        ppos.y = wpos.y+36.0f/480.0f;
        ddim.x = wdim.x-170.0f/640.0f;
        CEdit* pe = static_cast< CEdit* >(pw->SearchControl(EVENT_DIALOG_EDIT)); // file name edit box
        if ( pe != nullptr )
        {
            pe->SetPos(ppos);
            pe->SetDim(ddim);

            int nch = static_cast< int >((ddim.x*640.0f-22.0f)/5.75f);
            std::string name = pe->GetText(nch);
            pe->SetMaxChar(nch);
            pe->SetText(name);
        }
    }

    if ( m_usePublicPrivate )
    {
        ppos.x = wpos.x+10.0f/640.0f;
        ppos.y = wpos.y+5.0f/480.0f;
        ddim.x = 50.0f/640.0f;
        ddim.y = 16.0f/480.0f;
        pla = static_cast< CLabel* >(pw->SearchControl(EVENT_DIALOG_LABEL3)); // "Folder:" label
        if ( pla != nullptr )
        {
            pla->SetPos(ppos);
            pla->SetDim(ddim);
        }

        ppos.x += 50.0f/640.0f;
        ppos.y = wpos.y+12.0f/480.0f;
        ddim.x = 70.0f/640.0f;
        CCheck* pc = static_cast< CCheck* >(pw->SearchControl(EVENT_DIALOG_CHECK1)); // private check box
        if ( pc != nullptr )
        {
            pc->SetPos(ppos);
            pc->SetDim(ddim);
        }

        ppos.x += 80.0f/640.0f;
        pc = static_cast< CCheck* >(pw->SearchControl(EVENT_DIALOG_CHECK2)); // public check box
        if ( pc != nullptr )
        {
            pc->SetPos(ppos);
            pc->SetDim(ddim);
        }
    }

    ppos.x = wpos.x+wdim.x-100.0f/640.0f;
    ppos.y = wpos.y+34.0f/480.0f;
    ddim.x = 90.0f/640.0f;
    ddim.y = 23.0f/480.0f;
    CButton* pb = static_cast< CButton* >(pw->SearchControl(EVENT_DIALOG_OK)); // open/save button
    if ( pb != nullptr )
    {
        pb->SetPos(ppos);
        pb->SetDim(ddim);
    }

    ppos.y -= 26.0f/480.0f;
    pb = static_cast< CButton* >(pw->SearchControl(EVENT_DIALOG_CANCEL)); // Cancel button
    if ( pb != nullptr )
    {
        pb->SetPos(ppos);
        pb->SetDim(ddim);
    }

    ppos.y = wpos.y+wdim.y-49.0f/480.0f;
    pb = static_cast< CButton* >(pw->SearchControl(EVENT_DIALOG_NEWDIR)); // New Folder button
    if ( pb != nullptr )
    {
        pb->SetPos(ppos);
        pb->SetDim(ddim);
    }

    if ( m_askOverwriteMode )
    {
        ppos = wpos;
        ppos.x += (wdim.x/2.0f) - (97.5f/640.0f);
        ppos.y += (wdim.y/2.0f) - (50.0f/480.0f);

        ddim.x = 195.0f/640.0f;
        ddim.y = 100.0f/480.0f;
        CGroup* pg = static_cast< CGroup* >(pw->SearchControl(EVENT_DIALOG_GROUP1)); // "Overwrite ?" box
        if ( pg != nullptr )
        {
            pg->SetPos(ppos);
            pg->SetDim(ddim);
        }

        ddim.x = 82.5f/640.0f;
        ddim.y = 23.0f/480.0f;
        ppos.x += 10.0f/640.0f;
        ppos.y += 10.0f/480.0f;
        pb = static_cast< CButton* >(pw->SearchControl(EVENT_BUTTON_CANCEL)); // Cancel button
        if ( pb != nullptr )
        {
            pb->SetPos(ppos);
            pb->SetDim(ddim);
        }

        ppos.x += 92.5f/640.0f;
        pb = static_cast< CButton* >(pw->SearchControl(EVENT_BUTTON_OK)); // Ok button
        if ( pb != nullptr )
        {
            pb->SetPos(ppos);
            pb->SetDim(ddim);
        }

        ddim.x = 185.0f/640.0f;
        ppos.x -= 97.5f/640.0f;
        ppos.y += 28.0f/480.0f;
        pla = static_cast< CLabel* >(pw->SearchControl(EVENT_LABEL1));  // filename label
        if ( pla != nullptr )
        {
            pla->SetPos(ppos);
            pla->SetDim(ddim);
        }

        ppos.y += 28.0f/480.0f;
        pla = static_cast< CLabel* >(pw->SearchControl(EVENT_LABEL0));  // "Overwrite ?" label
        if ( pla != nullptr )
        {
            pla->SetPos(ppos);
            pla->SetDim(ddim);
        }
    }
}

// Management of a dialogue event.

bool CFileDialog::EventProcess(const Event &event)
{
    if ( event.type == m_windowEvent )  // window is moved ?
    {
        AdjustDialog();
        return true;
    }

    if ( m_askOverwriteMode ) return EventAskOverwrite(event);

    m_time += event.rTime;
    if ( event.type == EVENT_DIALOG_LIST ) // a list item was clicked ?
    {
        if ( ListItemIsFolder() )
        {
            if ( m_captureClick )
            {
                if (m_time - m_lastTimeClickDir <= DELAY_DBCLICK_DIR) // double click open folder
                {
                    OpenFolder();
                }
                m_captureClick = false;
            }
            else
            {
                m_captureClick = true;
                m_lastTimeClickDir = m_time;
            }
            if (m_selectFolderMode) UpdateSelectFolder();
            return true;
        }

        // a file name was clicked
        m_captureClick = false;
        if (m_selectFolderMode)
        {
            UpdateSelectFolder();
            return true;
        }

        if (!m_newFolderMode) GetListChoice();
        return true;
    }

    CWindow* pw = static_cast< CWindow* >(m_interface->SearchControl(m_windowEvent));
    if ( pw == nullptr ) return false;

    if ( event.type == EVENT_DIALOG_CANCEL || event.type == pw->GetEventTypeClose() )
    {
        m_eventQueue->AddEvent(Event(EVENT_DIALOG_STOP));
        return true;
    }

    EventType etype;
    etype = ( m_newFolderMode ) ? EVENT_DIALOG_EDIT2 : EVENT_DIALOG_EDIT;
    CEdit* pe = static_cast< CEdit* >(pw->SearchControl(etype));

    if ( event.type == etype )
    {
        if (pe != nullptr) SearchList(pe->GetText(999), m_newFolderMode);
        return true;
    }

    if ( m_usePublicPrivate )
    {
        if ( event.type == EVENT_DIALOG_CHECK1 )  // private?
        {
            m_subDirPath.clear();
            UpdatePublic(false);
            PopulateList();
            UpdatePathLabel();
            if (pe != nullptr) SearchList(pe->GetText(999), m_newFolderMode);
            if (m_selectFolderMode) UpdateSelectFolder();
        }
        else if ( event.type == EVENT_DIALOG_CHECK2 )  // public?
        {
            m_subDirPath.clear();
            UpdatePublic(true);
            PopulateList();
            UpdatePathLabel();
            if (pe != nullptr) SearchList(pe->GetText(999), m_newFolderMode);
            if (m_selectFolderMode) UpdateSelectFolder();
        }
    }

    if (m_captureClick && m_time - m_lastTimeClickDir > DELAY_DBCLICK_DIR)
    {
        m_captureClick = false;
    }

    if ( m_newFolderMode ) return EventNewFolder(event); // process 'new folder' events

    if ( event.type == EVENT_DIALOG_NEWDIR )
    {
        if ( !StartNewFolderMode() ) StopNewFolderMode(true);
        return true;
    }

    if (m_selectFolderMode) return EventSelectFolder(event);

    if ( event.type == EVENT_DIALOG_OK ||
         (event.type == EVENT_KEY_DOWN && event.GetData<KeyEventData>()->key == KEY(RETURN)) )
    {
        if ( m_dialogtype == CFileDialog::Type::Open )
        {
            if ( !ActionOpen() ) return true;
        }
        if ( m_dialogtype == CFileDialog::Type::Save )
        {
            if ( !ActionSave(m_confirmOverwrite) ) return true;
        }

        m_eventQueue->AddEvent(Event(EVENT_DIALOG_ACTION));
    }

    if ( event.type == EVENT_KEY_DOWN && event.GetData<KeyEventData>()->key == KEY(ESCAPE) )
    {
        m_eventQueue->AddEvent(Event(EVENT_DIALOG_STOP));
    }

    return true;
}

bool CFileDialog::StartNewFolderMode()
{
    CWindow* pw = static_cast< CWindow* >(m_interface->SearchControl(m_windowEvent)); // dialog window
    if ( pw == nullptr ) return false;

    if (!m_selectFolderMode)
    {
        CEdit* pe = static_cast< CEdit* >(pw->SearchControl(EVENT_DIALOG_EDIT));   // filename edit box
        if ( pe == nullptr ) return false;
        pe->SetState(STATE_ENABLE, false);
    }

    CButton* pb = static_cast< CButton* >(pw->SearchControl(EVENT_DIALOG_NEWDIR)); // new folder button
    if ( pb == nullptr ) return false;
    std::string name;
    GetResource(RES_EVENT, EVENT_DIALOG_CANCEL, name);
    pb->SetName(name);


    CList* pli = static_cast< CList* >(pw->SearchControl(EVENT_DIALOG_LIST));    // file list
    if ( pli == nullptr ) return false;
    glm::vec2 dim = pli->GetDim();
    dim.y -= 17.5f/480.0f;
    pli->SetDim(dim);
    pli->SetSelect(-1);
    pli->ShowSelect(false);

    glm::vec2 pos = pli->GetPos();

    pos.y += dim.y-3.0f/480.0f;
    dim.y = 20.0f/480.0f;

    CEdit* pe = static_cast< CEdit* >(pw->SearchControl(EVENT_DIALOG_EDIT2));  // new folder edit box
    if ( pe == nullptr ) return false;
    pe->SetState(STATE_VISIBLE, true);
    pe->SetState(STATE_ENABLE, true);
    pe->SetPos(pos);
    pe->SetDim(dim);
    pw->SetFocus(pe);

    pb = static_cast< CButton* >(pw->SearchControl(EVENT_DIALOG_OK));   // open/save button
    if ( pb == nullptr ) return false;
    pb->SetState(STATE_ENABLE, false);

    m_newFolderMode = true;

    return true;
}

bool CFileDialog::StopNewFolderMode(bool bCancel)
{
    m_newFolderMode = false;

    CWindow* pw = static_cast< CWindow* >(m_interface->SearchControl(m_windowEvent)); // dialog window
    if ( pw == nullptr ) return false;
    CEdit* pe = static_cast< CEdit* >(pw->SearchControl(EVENT_DIALOG_EDIT2)); // new folder edit box
    if ( pe != nullptr )
    {
        pe->SetText("");
        pe->ClearState(STATE_VISIBLE | STATE_ENABLE);
    }

    if (!m_selectFolderMode)
    {
        pe = static_cast< CEdit* >(pw->SearchControl(EVENT_DIALOG_EDIT));      // filename edit box
        if ( pe != nullptr )
        {
            pe->SetState(STATE_ENABLE, true);
            pw->SetFocus(pe);
        }
    }

    CButton* pb = static_cast< CButton* >(pw->SearchControl(EVENT_DIALOG_NEWDIR)); // new folder button
    if ( pb != nullptr )
    {
        std::string name;
        GetResource(RES_EVENT, EVENT_DIALOG_NEWDIR, name);
        pb->SetName(name);
    }

    AdjustDialog();

    if (m_selectFolderMode)
    {
        UpdateSelectFolder();
        return true;
    }

    if ( pe != nullptr )
    {
        pe->SetCursor(999, 0);
        if ( bCancel ) SearchList(pe->GetText(999));
    }

    return true;
}

bool CFileDialog::EventNewFolder(const Event &event)
{
    CWindow* pw = static_cast< CWindow* >(m_interface->SearchControl(m_windowEvent));
    if ( pw == nullptr ) return false;

    CButton* pb = static_cast< CButton* >(pw->SearchControl(EVENT_DIALOG_NEWDIR)); // new folder button
    if ( pb == nullptr ) return false;

    if ( event.type == EVENT_DIALOG_NEWDIR )
    {
        std::string text;
        GetResource(RES_EVENT, EVENT_DIALOG_CANCEL, text);
        if (pb->GetName() == text) return StopNewFolderMode(true); // cancel
        CreateNewFolder();
        return StopNewFolderMode();
    }

    if (event.type == EVENT_KEY_DOWN)
    {
        if (event.GetData<KeyEventData>()->key == KEY(ESCAPE))
            return StopNewFolderMode(true);

        if (event.GetData<KeyEventData>()->key == KEY(RETURN))
        {
            std::string text;
            GetResource(RES_EVENT, EVENT_DIALOG_CANCEL, text);
            if (pb->GetName() == text) return true;
            CreateNewFolder();
            return StopNewFolderMode();
        }
    }

    return true;
}

bool CFileDialog::EventSelectFolder(const Event &event)
{
    CWindow* pw = static_cast< CWindow* >(m_interface->SearchControl(m_windowEvent));
    if ( pw == nullptr ) return false;

    if ( event.type == EVENT_DIALOG_OK )
    {
        CList* pl = static_cast< CList* >(pw->SearchControl(EVENT_DIALOG_LIST));
        if ( pl == nullptr ) return false;
        std::string name = pl->GetItemName(pl->GetSelect());
        name = name.substr(0, name.find_first_of("\t"));
        if (m_subDirPath.empty()) {
            m_subDirPath = TempToPath(name);
        } else {
            m_subDirPath /= TempToPath(name);
        }
        m_eventQueue->AddEvent(Event(EVENT_DIALOG_ACTION));
    }

    if (event.type == EVENT_KEY_DOWN)
    {
        if (event.GetData<KeyEventData>()->key == KEY(ESCAPE))
        {
            m_eventQueue->AddEvent(Event(EVENT_DIALOG_STOP));
        }

        if (event.GetData<KeyEventData>()->key == KEY(RETURN))
        {
            if ( ListItemIsFolder() )
            {
                CList* pl = static_cast< CList* >(pw->SearchControl(EVENT_DIALOG_LIST));
                if ( pl == nullptr ) return false;
                std::string name = pl->GetItemName(pl->GetSelect());
                name = name.substr(0, name.find_first_of("\t"));
                if ( name != ".." )
                {
                    if (m_subDirPath.empty()) {
                        m_subDirPath = TempToPath(name);
                    } else {
                        m_subDirPath /= TempToPath(name);
                    }
                    m_eventQueue->AddEvent(Event(EVENT_DIALOG_ACTION));
                }
            }
        }
    }

    return true;
}

// Updates the file name edit box after a click in the list.

void CFileDialog::GetListChoice()
{
    CWindow* pw = static_cast< CWindow* >(m_interface->SearchControl(m_windowEvent));
    if ( pw == nullptr )  return;
    CList* pl = static_cast< CList* >(pw->SearchControl(EVENT_DIALOG_LIST));
    if ( pl == nullptr )  return;
    CEdit* pe = static_cast< CEdit* >(pw->SearchControl(EVENT_DIALOG_EDIT));
    if ( pe == nullptr )  return;

    std::string name = pl->GetItemName(pl->GetSelect());
    name = name.substr(0, name.find_first_of("\t"));
    SetFilenameField(pe, TempToPath(name));
    pe->SetCursor(999, 0);  // select all
    m_interface->SetFocus(pe);

    UpdateAction();
}

// Test if the selected list item is a directory that exists.

bool CFileDialog::ListItemIsFolder()
{
    CWindow* pw = static_cast< CWindow* >(m_interface->SearchControl(m_windowEvent));
    if ( pw == nullptr )  return false;

    CList* pl = static_cast< CList* >(pw->SearchControl(EVENT_DIALOG_LIST));
    if ( pl == nullptr )  return false;

    int n = pl->GetTotal();
    int i = pl->GetSelect();
    if (i < 0 || i >= n) return false;

    std::string name = pl->GetItemName(i);
    name = name.substr(0, name.find_first_of("\t"));

    if (name == "..") return !m_subDirPath.empty();

    if (name.find_first_of(".*?:<>\"|/\\") != std::string::npos) return false;

    return DirectoryExists(TempToPath(name));
}

// Updates the list after a change in name.

void CFileDialog::SearchList(const std::string &text, bool dirOnly)
{
    CWindow* pw = static_cast< CWindow* >(m_interface->SearchControl(m_windowEvent));
    if ( pw == nullptr )  return;
    CList* pl = static_cast< CList* >(pw->SearchControl(EVENT_DIALOG_LIST));
    if ( pl == nullptr )  return;

    pl->SetSelect(-1);

    // highlight the list item matching what is typed in the edit box
    if (!text.empty())
    {
        for (int i = 0; i < pl->GetTotal(); i++)
        {
            std::string item = pl->GetItemName(i);
            if (dirOnly && item.find("\t** DIR **  \t") == std::string::npos) break;
            item = item.substr(0, item.find_first_of("\t"));
            if (item.substr(0, text.length()) != text) continue;
            pl->SetSelect(i); // select item
            pl->ShowSelect(false);  // scroll list
            break;
        }
    }

    if ( m_newFolderMode ) UpdateNewFolder(); else UpdateAction();
}

// Updates the action button.

void CFileDialog::UpdateAction()
{
    CWindow* pw = static_cast< CWindow* >(m_interface->SearchControl(m_windowEvent));
    if ( pw == nullptr )  return;
    CEdit* pe = static_cast< CEdit* >(pw->SearchControl(EVENT_DIALOG_EDIT));
    if ( pe == nullptr )  return;
    CButton* pb = static_cast< CButton* >(pw->SearchControl(EVENT_DIALOG_OK));
    if ( pb == nullptr )  return;

    std::string text = pe->GetText(999);
    if (
        text.empty()
        || text.find_first_of("*?:<>\"|/\\") != std::string::npos  // *?:<>"|/\ are banned on Windows
    )
    {
        pb->SetState(STATE_ENABLE, false);
        return;
    }
    std::filesystem::path filename = StrUtils::ToPath(text);
    if (DirectoryExists(filename))
    {
        pb->SetState(STATE_ENABLE, false);
        return;
    }
    if (CheckFilename(filename))
    {
        pb->SetState(STATE_ENABLE, true);
        return;
    }
    filename += m_extension;
    pb->SetState(STATE_ENABLE, CheckFilename(filename));
}

void CFileDialog::UpdateSelectFolder()
{
    CWindow* pw = static_cast< CWindow* >(m_interface->SearchControl(m_windowEvent));
    if ( pw == nullptr ) return;
    CButton* pb = static_cast< CButton* >(pw->SearchControl(EVENT_DIALOG_OK));
    if ( pb == nullptr ) return;

    bool bError = true;
    if (!m_newFolderMode && ListItemIsFolder())
    {
        CList* pl = static_cast< CList* >(pw->SearchControl(EVENT_DIALOG_LIST));
        if ( pl != nullptr )
        {
            std::string name = pl->GetItemName(pl->GetSelect());
            name = name.substr(0, name.find_first_of("\t"));
            if (name != "..") bError = false;
        }
    }

    pb->SetState(STATE_ENABLE, !bError);
}

// Updates the New Folder button.

void CFileDialog::UpdateNewFolder()
{
    CWindow* pw = static_cast< CWindow* >(m_interface->SearchControl(m_windowEvent));
    if ( pw == nullptr )  return;
    CEdit* pe = static_cast< CEdit* >(pw->SearchControl(EVENT_DIALOG_EDIT2));
    if ( pe == nullptr )  return;
    CButton* pb = static_cast< CButton* >(pw->SearchControl(EVENT_DIALOG_NEWDIR));
    if ( pb == nullptr )  return;

    bool bError = true;
    std::string text = pe->GetText(999);
    if ( !text.empty() )
    {
        if (text.find_first_of(".*?:<>\"|/\\") == std::string::npos)
            bError = DirectoryExists(StrUtils::ToPath(text));
    }

    std::string res;
    if (bError) GetResource(RES_EVENT, EVENT_DIALOG_CANCEL, res);
    else GetResource(RES_EVENT, EVENT_DIALOG_OK, res);
    pb->SetName(res);
}

// Updates the private/public buttons.

void CFileDialog::UpdatePublic(bool bPublic)
{
    if ( !m_usePublicPrivate ) return;

    m_public = bPublic;

    SetBasePath(bPublic ? m_pathPublic : m_pathPrivate);

    CWindow* pw = static_cast< CWindow* >(m_interface->SearchControl(m_windowEvent));
    if ( pw == nullptr ) return;

    CCheck* pc = static_cast< CCheck* >(pw->SearchControl(EVENT_DIALOG_CHECK1));
    if ( pc != nullptr )
    {
        pc->SetState(STATE_CHECK, !bPublic);
    }

    pc = static_cast< CCheck* >(pw->SearchControl(EVENT_DIALOG_CHECK2));
    if ( pc != nullptr )
    {
        pc->SetState(STATE_CHECK, bPublic);
    }
}

// Updates the path label

void CFileDialog::UpdatePathLabel()
{
    CWindow* pw = static_cast< CWindow* >(m_interface->SearchControl(m_windowEvent));
    if ( pw == nullptr )  return;

    CLabel* pl = static_cast< CLabel* >(pw->SearchControl(EVENT_DIALOG_LABEL1));
    if ( pl != nullptr )
    {
        glm::vec2 dim = pl->GetDim();
        size_t nch = static_cast< size_t >((dim.x*640.0f)/5.75f);
        std::string text = StrUtils::ToString(SearchDirectory(false));
        if (text.length() > nch)
        {
            text = "..." + text.substr(text.length()-nch, nch);
        }
        pl->SetName(text, false);
    }
}

// Fills the list with files and folders.

void CFileDialog::PopulateList()
{
    CWindow* pw = static_cast< CWindow* >(m_interface->SearchControl(m_windowEvent));
    if ( pw == nullptr )  return;
    CList* pl = static_cast< CList* >(pw->SearchControl(EVENT_DIALOG_LIST));
    if ( pl == nullptr )  return;
    pl->Flush();

    if (!CResourceManager::DirectoryExists(SearchDirectory(false)))
        return;

    int  i = 0;
    char timestr[100];

    // list all folders
    std::vector<std::filesystem::path> folders = CResourceManager::ListDirectories(SearchDirectory(false));
    if (!m_subDirPath.empty()) folders.insert(folders.begin(), "..");
    for (auto& dir : folders)
    {
        time_t now = CResourceManager::GetLastModificationTime(SearchDirectory(false) / dir);
        strftime(timestr, 99, "%x %X", localtime(&now));
        std::ostringstream temp;
        temp << dir << '\t' << "** DIR **" << "  \t" << timestr;
        pl->SetItemName(i++, temp.str().c_str());
    }

    // list all files
    std::vector<std::filesystem::path> files = CResourceManager::ListFiles(SearchDirectory(false), true);

    auto it = std::remove_if(files.begin(), files.end(), [this](const std::filesystem::path& name)
    {
        return !CheckFilename(name);
    });
    files.erase(it, files.end()); // remove invalid file names

    for (const auto& filename : files)
    {
        auto path = SearchDirectory(false) / filename;
        time_t now = CResourceManager::GetLastModificationTime(path);
        strftime(timestr, 99, "%x %X", localtime(&now));
        std::ostringstream temp;
        temp << StrUtils::ToString(filename) << '\t' << CResourceManager::GetFileSize(path) << "  \t" << timestr;
        pl->SetItemName(i++, temp.str().c_str());
    }
}

// Constructs the name of the folder for open/save.
// If the folder does not exist, it can be created.
std::filesystem::path CFileDialog::SearchDirectory(bool bCreate)
{
    std::filesystem::path dir = m_basePath;

    if (bCreate && !CResourceManager::DirectoryExists(dir))
    {
        CResourceManager::CreateNewDirectory(dir);
    }

    if (!m_subDirPath.empty())
    {
        dir /= m_subDirPath;

        if (bCreate && !CResourceManager::DirectoryExists(dir))
        {
            CResourceManager::CreateNewDirectory(dir);
        }
    }

    return dir;
}

bool CFileDialog::DirectoryExists(const std::filesystem::path& name)
{
    if ( name.empty() ) return false;

    if ( name == ".." ) return !m_subDirPath.empty();

    return CResourceManager::DirectoryExists(SearchDirectory(false) / name);
}

// Make folder

void CFileDialog::CreateNewFolder()
{
    CWindow* pw = static_cast< CWindow* >(m_interface->SearchControl(m_windowEvent));
    if ( pw == nullptr ) return;

    CEdit* pe = static_cast< CEdit* >(pw->SearchControl(EVENT_DIALOG_EDIT2));
    if ( pe == nullptr ) return;
    std::filesystem::path name = StrUtils::ToPath(pe->GetText(999));
    if ( name.empty() ) return;

    m_subDirPath /= name;   // add to current path

    SearchDirectory(true);  // make the new folder

    m_subDirPath = m_subDirPath.parent_path();

    PopulateList();         // redraw the list
    SearchList(StrUtils::ToString(name), true); // highlight the new folder in list box
}

// Open folder

void CFileDialog::OpenFolder()
{
    CWindow* pw = static_cast< CWindow* >(m_interface->SearchControl(m_windowEvent));
    if ( pw == nullptr ) return;

    CList* pl = static_cast< CList* >(pw->SearchControl(EVENT_DIALOG_LIST));
    if ( pl == nullptr ) return;

    std::string name = pl->GetItemName(pl->GetSelect());
    name = name.substr(0, name.find_first_of("\t"));

    if ( name.empty() ) return;

    if ( name == ".." ) // parent folder
    {
        m_subDirPath = m_subDirPath.parent_path();
    }
    else if ( DirectoryExists(TempToPath(name)) )
    {
        m_subDirPath /= TempToPath(name);
    }

    PopulateList();
    UpdatePathLabel();
    EventType type;
    type = m_newFolderMode ? EVENT_DIALOG_EDIT2 : EVENT_DIALOG_EDIT;

    CEdit* pe = static_cast< CEdit* >(pw->SearchControl(type));
    if ( pe != nullptr ) SearchList(pe->GetText(999), m_newFolderMode);
}

bool CFileDialog::CheckFilename(const std::filesystem::path& name)
{
    if ( name.empty() || StrUtils::ToString(name)[0] == '.' ) return false;

    if ( m_extension.empty() && m_extlist.empty() ) return true;      // no required extension?

    for ( const std::filesystem::path& ext : m_extlist ) // allowed extensions?
    {
        if ( ext == name.extension() ) return true;
    }

    if ( !m_extension.empty() ) // default extension?
    {
        if ( m_extension == name.extension() ) return true;
    }
    return false;
}

bool CFileDialog::ActionOpen()
{
    CWindow* pw = static_cast< CWindow* >(m_interface->SearchControl(m_windowEvent));
    if ( pw == nullptr ) return false;

    CEdit* pe = static_cast< CEdit* >(pw->SearchControl(EVENT_DIALOG_EDIT));
    if ( pe == nullptr ) return false;
    std::filesystem::path filename = StrUtils::ToPath(pe->GetText(100));
    if ( filename.empty() ) return false;

    if ( !CheckFilename(filename) ) // add default extension ?
    {
        if ( !m_extension.empty() ) filename += m_extension;
        if ( !CheckFilename(filename) ) return false; // file name is ok ?
    }

    SearchDirectory(true);
    SetFilename(filename);
    SetFilenameField(pe, filename);
    pe->SetCursor(999, 0);  // select all
    pw->SetFocus(pe);

    return true;
}

bool CFileDialog::ActionSave(bool checkFileExist)
{
    CWindow* pw = static_cast< CWindow* >(m_interface->SearchControl(m_windowEvent));
    if ( pw == nullptr ) return false;

    CEdit* pe = static_cast< CEdit* >(pw->SearchControl(EVENT_DIALOG_EDIT));
    if ( pe == nullptr ) return false;
    std::filesystem::path filename = StrUtils::ToPath(pe->GetText(100));
    if ( filename.empty() ) return false;

    if ( !CheckFilename(filename) ) // add default extension ?
    {
        if ( !m_extension.empty() ) filename += m_extension;
        if ( !CheckFilename(filename) ) return false; // file name is ok ?
    }

    SearchDirectory(true);

    if ( checkFileExist )
    {
        if (CResourceManager::Exists(SearchDirectory(false) / filename))
        {
            if ( !StartAskOverwrite(filename) ) StopAskOverwrite();
            return false;
        }
    }

    SetFilename(filename);
    SetFilenameField(pe, filename);
    pe->SetCursor(999, 0);  // select all
    pw->SetFocus(pe);

    return true;
}

bool CFileDialog::StartAskOverwrite(const std::filesystem::path& name)
{
    CWindow* pw = static_cast< CWindow* >(m_interface->SearchControl(m_windowEvent)); // dialog window
    if ( pw == nullptr ) return false;

    // disable controls
    CEdit* pe = static_cast< CEdit* >(pw->SearchControl(EVENT_DIALOG_EDIT)); // filename edit box
    if ( pe == nullptr ) return false;
    pe->SetState(STATE_ENABLE, false);

    CButton* pb = static_cast< CButton* >(pw->SearchControl(EVENT_DIALOG_NEWDIR)); // new folder button
    if ( pb == nullptr ) return false;
    pb->SetState(STATE_ENABLE, false);

    CList* pli = static_cast< CList* >(pw->SearchControl(EVENT_DIALOG_LIST)); // file list
    if ( pli == nullptr ) return false;
    pli->SetState(STATE_ENABLE, false);

    if ( m_usePublicPrivate )
    {
        CCheck* pc = static_cast< CCheck* >(pw->SearchControl(EVENT_DIALOG_CHECK1)); // private check box
        if ( pc == nullptr ) return false;
        pc->SetState(STATE_ENABLE, false);

        pc = static_cast< CCheck* >(pw->SearchControl(EVENT_DIALOG_CHECK2)); // public check box
        if ( pc == nullptr ) return false;
        pc->SetState(STATE_ENABLE, false);
    }

    pb = static_cast< CButton* >(pw->SearchControl(EVENT_DIALOG_OK));    // open/save button
    if ( pb == nullptr ) return false;
    pb->SetState(STATE_ENABLE, false);

    // show overwrite controls
    CGroup* pg = static_cast< CGroup* >(pw->SearchControl(EVENT_DIALOG_GROUP1)); // "Overwrite ?" box
    if ( pg == nullptr ) return false;
    pg->SetState(STATE_VISIBLE | STATE_ENABLE);

    CLabel* pla = static_cast< CLabel* >(pw->SearchControl(EVENT_LABEL0)); // "Overwrite ?" label
    if ( pla == nullptr ) return false;
    pla->SetState(STATE_VISIBLE | STATE_ENABLE);

    pla = static_cast< CLabel* >(pw->SearchControl(EVENT_LABEL1));       // filename label
    if ( pla == nullptr ) return false;
    pla->SetState(STATE_VISIBLE | STATE_ENABLE);
    pla->SetName(StrUtils::ToString(name));

    pb = static_cast< CButton* >(pw->SearchControl(EVENT_BUTTON_CANCEL)); // Cancel button
    if ( pb == nullptr ) return false;
    pb->SetState(STATE_VISIBLE | STATE_ENABLE);

    pb = static_cast< CButton* >(pw->SearchControl(EVENT_BUTTON_OK));    // Ok button
    if ( pb == nullptr ) return false;
    pb->SetState(STATE_VISIBLE | STATE_ENABLE);

    m_askOverwriteMode = true;

    AdjustDialog();

    return true;
}

bool CFileDialog::StopAskOverwrite()
{
    m_askOverwriteMode = false;

    CWindow* pw = static_cast< CWindow* >(m_interface->SearchControl(m_windowEvent)); // dialog window
    if ( pw == nullptr ) return false;

    // hide overwrite controls
    CGroup* pg = static_cast< CGroup* >(pw->SearchControl(EVENT_DIALOG_GROUP1)); // "Overwrite ?" box
    if ( pg != nullptr ) pg->ClearState(STATE_VISIBLE | STATE_ENABLE);

    CLabel* pla = static_cast< CLabel* >(pw->SearchControl(EVENT_LABEL0)); // "Overwrite ?" label
    if ( pla != nullptr ) pla->ClearState(STATE_VISIBLE | STATE_ENABLE);

    pla = static_cast< CLabel* >(pw->SearchControl(EVENT_LABEL1));       // filename label
    if ( pla != nullptr ) pla->ClearState(STATE_VISIBLE | STATE_ENABLE);

    CButton* pb = static_cast< CButton* >(pw->SearchControl(EVENT_BUTTON_CANCEL)); // Cancel button
    if ( pb != nullptr ) pb->ClearState(STATE_VISIBLE | STATE_ENABLE);

    pb = static_cast< CButton* >(pw->SearchControl(EVENT_BUTTON_OK));    // Ok button
    if ( pb != nullptr ) pb->ClearState(STATE_VISIBLE | STATE_ENABLE);

    // enable other controls
    CEdit* pe = static_cast< CEdit* >(pw->SearchControl(EVENT_DIALOG_EDIT)); // filename edit box
    if ( pe != nullptr )
    {
        pe->SetState(STATE_ENABLE, true);
        pe->SetCursor(999, 0);
        pw->SetFocus(pe);
    }

    pb = static_cast< CButton* >(pw->SearchControl(EVENT_DIALOG_NEWDIR)); // new folder button
    if ( pb != nullptr ) pb->SetState(STATE_ENABLE, true);

    CList* pli = static_cast< CList* >(pw->SearchControl(EVENT_DIALOG_LIST)); // file list
    if ( pli != nullptr ) pli->SetState(STATE_ENABLE, true);

    if ( m_usePublicPrivate )
    {
        CCheck* pc = static_cast< CCheck* >(pw->SearchControl(EVENT_DIALOG_CHECK1)); // private check box
        if ( pc != nullptr ) pc->SetState(STATE_ENABLE, true);

        pc = static_cast< CCheck* >(pw->SearchControl(EVENT_DIALOG_CHECK2)); // public check box
        if ( pc != nullptr ) pc->SetState(STATE_ENABLE, true);
    }

    pb = static_cast< CButton* >(pw->SearchControl(EVENT_DIALOG_OK));    // open/save button
    if ( pb != nullptr ) pb->SetState(STATE_ENABLE, true);

    return true;
}

bool CFileDialog::EventAskOverwrite(const Event &event)
{
    if ( event.type == EVENT_KEY_DOWN && event.GetData<KeyEventData>()->key == KEY(ESCAPE) )
    {
        return StopAskOverwrite();
    }

    if ( event.type == EVENT_BUTTON_CANCEL )
    {
        return StopAskOverwrite();
    }

    if ( event.type == EVENT_BUTTON_OK ||
         (event.type == EVENT_KEY_DOWN && event.GetData<KeyEventData>()->key == KEY(RETURN)) )
    {
        if ( ActionSave() )
        {
            m_eventQueue->AddEvent(Event(EVENT_DIALOG_ACTION));
        }

        return StopAskOverwrite();
    }

    CWindow* pw = static_cast< CWindow* >(m_interface->SearchControl(m_windowEvent)); // dialog window
    if ( pw == nullptr ) return false;

    if ( event.type == EVENT_DIALOG_CANCEL || event.type == pw->GetEventTypeClose() )
    {
        m_eventQueue->AddEvent(Event(EVENT_DIALOG_STOP));
        return true;
    }

    return true;
}


} // namespace Ui
