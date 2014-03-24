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


#include "ui/screen/menu.h"


#include "app/app.h"

#include "common/config.h"
#include "common/logger.h"

#include "graphics/engine/engine.h"

#include "ui/maindialog.h"

#include <boost/algorithm/string/split.hpp>


namespace Ui {

CScreenMenu::CScreenMenu()
{
    m_page = MENUPAGE_INIT;
    m_oldMusicVol = m_oldAudioVol = MAXVOLUME;
    
    m_window = nullptr;
    
    m_enabledPhases.insert(PHASE_MENU);
}

CScreenMenu::~CScreenMenu()
{
    Stop();
}

void CScreenMenu::Start()
{
    m_window = CEGUI::WindowManager::getSingleton().loadLayoutFromFile("menu.layout");
    CEGUI::System::getSingleton().getDefaultGUIContext().getRootWindow()->addChild(m_window);
    
    if(m_page == MENUPAGE_INIT)
    {
        LoadLayout("mainmenu");
        InitWindow(WINDOW_INIT, RUIT_TITLE_INIT);
        InitButton(BUTTON_STARTGAME, RUIT_STARTGAME);
        InitButton(BUTTON_MULTIPLAYER, RUIT_MULTIPLAYER);
        InitButton(BUTTON_CREDITS, RUIT_CREDITS);
        InitButton(BUTTON_SETUP, RUIT_SETUP);
        InitButton(BUTTON_NAME, RUIT_NAME);
        InitButton(BUTTON_QUIT, RUIT_QUIT);
    }
    
    if(m_page == MENUPAGE_STARTGAME)
    {
        LoadLayout("startgame");
        InitWindow(WINDOW_STARTGAME, RUIT_TITLE_STARTGAME);
        InitButton(BUTTON_MISSIONS, RUIT_MISSIONS);
        InitButton(BUTTON_FREEGAME, RUIT_FREEGAME);
        InitButton(BUTTON_EXERCISES, RUIT_EXERCISES);
        InitButton(BUTTON_CHALLENGES, RUIT_CHALLENGES);
        InitButton(BUTTON_USERLVL, RUIT_USERLVL);
        InitButton(BUTTON_BACK, RUIT_BACK);
        
        #if !DEV_BUILD
        m_window->getChildRecursive(BUTTON_USERLVL)->setEnabled(false);
        #endif
    }
    
    if(m_page == MENUPAGE_MULTIPLAYER)
    {
        LoadLayout("multiplayer");
        InitWindow(WINDOW_MULTIPLAYER, RUIT_TITLE_MULTIPLAYER);
        InitButton(BUTTON_BACK, RUIT_BACK);
        
        m_oldAudioVol = CApplication::GetInstancePointer()->GetSound()->GetAudioVolume();
        m_oldMusicVol = CApplication::GetInstancePointer()->GetSound()->GetMusicVolume();
        CApplication::GetInstancePointer()->GetSound()->SetAudioVolume(0);
        CApplication::GetInstancePointer()->GetSound()->SetMusicVolume(MAXVOLUME);
        CApplication::GetInstancePointer()->GetSound()->PlayMusic("music012.ogg", true, 1.f); // Easter egg? Why not! :D
    }
    
    /* TODO: What does this do? Is it needed?
    pos.x  = 0.35f;
    pos.y  = 0.60f;
    ddim.x = 0.30f;
    ddim.y = 0.30f;
    pw->CreateGroup(pos, ddim, 5, EVENT_INTERFACE_GLINTl);  // orange corner
    pos.x  = 0.35f;
    pos.y  = 0.10f;
    ddim.x = 0.30f;
    ddim.y = 0.30f;
    pw->CreateGroup(pos, ddim, 4, EVENT_INTERFACE_GLINTr);  // blue corner
    */
    
    m_window->getChildRecursive(LABEL_VERSION)->setText(COLOBOT_VERSION_DISPLAY+CEGUI::String("\n")+COLOBOT_VERSION_DATE);
    
    Gfx::CEngine* engine = Gfx::CEngine::GetInstancePointer();
    engine->SetBackground("interface.png",
            Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
            Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
            Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
            Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
            true);
    engine->SetBackForce(true);
}

void CScreenMenu::InitButton(std::string name, ResUiTextType textId)
{
    m_window->getChildRecursive(name)->subscribeEvent(CEGUI::Window::EventMouseClick, CEGUI::Event::Subscriber(&CScreenMenu::OnClick, this));
    
    std::string translated;
    GetResource(RES_UI, textId, translated);
    std::vector<std::string> strings;
    boost::split(strings, translated, boost::is_any_of("\\"));
    
    m_window->getChildRecursive(name)->setText(reinterpret_cast<const CEGUI::utf8*>(strings[0].c_str()));
    m_window->getChildRecursive(name)->setTooltipText(reinterpret_cast<const CEGUI::utf8*>(strings[1].c_str())); //TODO: For some reason tooltips don't work
}

void CScreenMenu::InitWindow(std::string name, ResUiTextType textId)
{
    std::string translated;
    GetResource(RES_UI, textId, translated);
    
    m_window->getChildRecursive(name)->setProperty("LabelText", reinterpret_cast<const CEGUI::utf8*>(translated.c_str()));
}

void CScreenMenu::Stop()
{
    if(m_window != nullptr) {
        CEGUI::WindowManager::getSingleton().destroyWindow(m_window);
        m_window = nullptr;
    }
}

void CScreenMenu::ChangePage(MenuPage page)
{
    Stop();
    m_page = page;
    Start();
}

void CScreenMenu::LoadLayout(CEGUI::String name)
{
    CEGUI::Window* menuContent = CEGUI::WindowManager::getSingleton().loadLayoutFromFile(name+".layout");
    m_window->addChild(menuContent);
}

bool CScreenMenu::OnClick(const CEGUI::EventArgs& e)
{
    const CEGUI::WindowEventArgs* args = static_cast<const CEGUI::WindowEventArgs*>(&e);
    CEGUI::Window* target = static_cast<CEGUI::Window*>(args->window);
    CEGUI::String targetName = target->getName();
    
    if(m_page == MENUPAGE_INIT)
    {
        if(targetName == BUTTON_STARTGAME)   ChangePage(MENUPAGE_STARTGAME);
        if(targetName == BUTTON_MULTIPLAYER) ChangePage(MENUPAGE_MULTIPLAYER);
        if(targetName == BUTTON_CREDITS)     CRobotMain::GetInstancePointer()->ChangePhase(PHASE_CREDITS);
        if(targetName == BUTTON_SETUP)       CRobotMain::GetInstancePointer()->ChangePhase(PHASE_SETUPg);
        if(targetName == BUTTON_NAME)        CRobotMain::GetInstancePointer()->ChangePhase(PHASE_NAME);
        if(targetName == BUTTON_QUIT)        CRobotMain::GetInstancePointer()->ChangePhase(PHASE_GENERIC);
    }
    
    if(m_page == MENUPAGE_STARTGAME)
    {
        if(targetName == BUTTON_MISSIONS)   CRobotMain::GetInstancePointer()->ChangePhase(PHASE_MISSION);
        if(targetName == BUTTON_FREEGAME)   CRobotMain::GetInstancePointer()->ChangePhase(PHASE_FREE);
        if(targetName == BUTTON_EXERCISES)  CRobotMain::GetInstancePointer()->ChangePhase(PHASE_TRAINER);
        if(targetName == BUTTON_CHALLENGES) CRobotMain::GetInstancePointer()->ChangePhase(PHASE_DEFI);
        if(targetName == BUTTON_USERLVL)    CRobotMain::GetInstancePointer()->ChangePhase(PHASE_USER);
        if(targetName == BUTTON_BACK)       ChangePage(MENUPAGE_INIT);
    }
    
    if(m_page == MENUPAGE_MULTIPLAYER)
    {
        if(targetName == BUTTON_BACK)
        {
            ChangePage(MENUPAGE_INIT);
            CApplication::GetInstancePointer()->GetSound()->SetAudioVolume(m_oldAudioVol);
            CApplication::GetInstancePointer()->GetSound()->SetMusicVolume(m_oldMusicVol);
            CApplication::GetInstancePointer()->GetSound()->PlayMusic("Intro1.ogg", false, 1.f);
        }
    }
    
    return true;
}

bool CScreenMenu::EventProcess(const Event &event)
{
    switch( event.type )
    {
        case EVENT_KEY_DOWN:
            if ( event.key.key == KEY(ESCAPE) )
            {
                if(m_page == MENUPAGE_INIT) {
                    CApplication::GetInstancePointer()->GetSound()->Play(SOUND_TZOING);
                    CRobotMain::GetInstancePointer()->ChangePhase(PHASE_GENERIC);
                } else {
                    if(m_page == MENUPAGE_MULTIPLAYER)
                    {
                        CApplication::GetInstancePointer()->GetSound()->SetAudioVolume(m_oldAudioVol);
                        CApplication::GetInstancePointer()->GetSound()->SetMusicVolume(m_oldMusicVol);
                        CApplication::GetInstancePointer()->GetSound()->PlayMusic("Intro1.ogg", false, 1.f);
                    }
                    ChangePage(MENUPAGE_INIT);
                }
            }
            return false;
        
        default:
            return true;
    }
}

void CScreenMenu::ChangePhase(Phase phase)
{
}

}

