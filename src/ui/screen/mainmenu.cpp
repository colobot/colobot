// * This file is part of the COLOBOT source code
// * Copyright (C) 2014 Polish Portal of Colobot (PPC)
// *
// * This program is free software: you can redistribute it and/or modify
// * it under the terms of the GNU General Public License as published by
// * the Free Software Foundation, either version 3 of the License, or
// * (at your option) any later version.
// *
// * This program is distributed in the hope that it will be useful,
// * but WITHOUT ANY WARRANTY; without even the implied warranty of
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// * GNU General Public License for more details.
// *
// * You should have received a copy of the GNU General Public License
// * along with this program. If not, see  http://www.gnu.org/licenses/.


#include "ui/screen/mainmenu.h"


#include "common/logger.h"


namespace Ui {

CScreenMainMenu::CScreenMainMenu()
{
    m_window = nullptr;
    
    m_enabledPhases.insert(PHASE_INIT);
}

CScreenMainMenu::~CScreenMainMenu()
{
    Stop();
}

void CScreenMainMenu::Start()
{
    m_window = CEGUI::WindowManager::getSingleton().loadLayoutFromFile("mainmenu.layout");
    CEGUI::System::getSingleton().getDefaultGUIContext().getRootWindow()->addChild(m_window);
    
    m_window->getChildRecursive("missions_button")->setText("Missions");
}

void CScreenMainMenu::Stop()
{
    if(m_window != nullptr) {
        CEGUI::WindowManager::getSingleton().destroyWindow(m_window);
        m_window = nullptr;
    }
}

bool CScreenMainMenu::EventProcess(const Event &event)
{
    return true;
}

void CScreenMainMenu::ChangePhase(Phase phase)
{
}

}

