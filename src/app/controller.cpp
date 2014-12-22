/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2014, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include "app/controller.h"


#include "object/robotmain.h"

#include "ui/maindialog.h"


template<> CController* CSingleton<CController>::m_instance = nullptr;


CController::CController(CApplication* app, bool loadProfile)
{
    m_app = app;
    m_main = new CRobotMain(this);
    m_dialog = new Ui::CMainDialog();
    
    m_main->Create(loadProfile);
    m_dialog->Create();
    if (!loadProfile)
        m_main->CreateIni();
    else
        m_main->LoadIni();
}

CController::~CController()
{
    delete m_dialog;
    m_dialog = nullptr;
    
    delete m_main;
    m_main = nullptr;
    
    m_app = nullptr;
}

CApplication* CController::GetApplication()
{
    return m_app;
}

CRobotMain* CController::GetRobotMain()
{
    return m_main;
}

Ui::CMainDialog* CController::GetMainDialog()
{
    return m_dialog;
}

void CController::StartApp()
{
    m_main->ChangePhase(PHASE_WELCOME1);
}

void CController::StartGame(std::string cat, int chap, int lvl)
{
    m_dialog->SetSceneName(cat.c_str());
    m_dialog->SetSceneRank(chap*100+lvl);
    m_main->ChangePhase(PHASE_LOADING);
}

void CController::ProcessEvent(Event& event)
{
    bool passOn = m_dialog->EventProcess(event);
    if(passOn)
        m_main->ProcessEvent(event);
}