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


#include "app/app.h"

#include "common/logger.h"

#include "ui/maindialog.h"

#include <boost/algorithm/string/split.hpp>


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
    
    InitButton(BUTTON_MISSIONS, RUIT_MISSIONS);
    InitButton(BUTTON_FREEGAME, RUIT_FREEGAME);
    InitButton(BUTTON_EXERCISES, RUIT_EXERCISES);
    InitButton(BUTTON_CHALLENGES, RUIT_CHALLENGES);
    InitButton(BUTTON_USERLVL, RUIT_USERLVL);
    InitButton(BUTTON_SETUP, RUIT_SETUP);
    InitButton(BUTTON_NAME, RUIT_NAME);
    InitButton(BUTTON_QUIT, RUIT_QUIT);
}

void CScreenMainMenu::InitButton(std::string button, ResUiTextType textId)
{
    m_window->getChildRecursive(button)->subscribeEvent(CEGUI::Window::EventMouseClick, CEGUI::Event::Subscriber(&CScreenMainMenu::OnClick, this));
    
    std::string translated;
    GetResource(RES_UI, textId, translated);
    std::vector<std::string> strings;
    boost::split(strings, translated, boost::is_any_of("\\"));
    
    m_window->getChildRecursive(button)->setText(reinterpret_cast<const CEGUI::utf8*>(strings[0].c_str()));
    m_window->getChildRecursive(button)->setTooltipText(reinterpret_cast<const CEGUI::utf8*>(strings[1].c_str())); //TODO: For some reason tooltips don't work
}

void CScreenMainMenu::Stop()
{
    if(m_window != nullptr) {
        CEGUI::WindowManager::getSingleton().destroyWindow(m_window);
        m_window = nullptr;
    }
}

bool CScreenMainMenu::OnClick(const CEGUI::EventArgs& e)
{
    const CEGUI::WindowEventArgs* args = static_cast<const CEGUI::WindowEventArgs*>(&e);
    CEGUI::Window* target = static_cast<CEGUI::Window*>(args->window);
    CEGUI::String targetName = target->getName();
    
    if(targetName == BUTTON_MISSIONS) CRobotMain::GetInstancePointer()->ChangePhase(PHASE_MISSION);
    if(targetName == BUTTON_FREEGAME) CRobotMain::GetInstancePointer()->ChangePhase(PHASE_FREE);
    if(targetName == BUTTON_EXERCISES) CRobotMain::GetInstancePointer()->ChangePhase(PHASE_TRAINER);
    if(targetName == BUTTON_CHALLENGES) CRobotMain::GetInstancePointer()->ChangePhase(PHASE_DEFI);
    if(targetName == BUTTON_SETUP) CRobotMain::GetInstancePointer()->ChangePhase(PHASE_SETUPg);
    if(targetName == BUTTON_NAME) CRobotMain::GetInstancePointer()->ChangePhase(PHASE_NAME);
    if(targetName == BUTTON_QUIT) CRobotMain::GetInstancePointer()->ChangePhase(PHASE_GENERIC);
    
    return true;
}

bool CScreenMainMenu::EventProcess(const Event &event)
{
    switch( event.type )
    {
        case EVENT_KEY_DOWN:
            if ( event.key.key == KEY(ESCAPE) )
            {
                CApplication::GetInstancePointer()->GetSound()->Play(SOUND_TZOING);
                CRobotMain::GetInstancePointer()->ChangePhase(PHASE_GENERIC);
            }
            return false;
        
        default:
            return true;
    }
}

void CScreenMainMenu::ChangePhase(Phase phase)
{
}

}

