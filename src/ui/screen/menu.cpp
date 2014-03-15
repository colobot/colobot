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

#include "ui/interface.h"
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
    CWindow* pw;
    Math::Point pos, ddim;
    std::string name;
    
    if(m_page == MENUPAGE_INIT)
    {
        LoadLayout("mainmenu");
        InitButton(BUTTON_STARTGAME, RUIT_STARTGAME);
        InitButton(BUTTON_MULTIPLAYER, RUIT_MULTIPLAYER);
        InitButton(BUTTON_SETUP, RUIT_SETUP);
        InitButton(BUTTON_NAME, RUIT_NAME);
        InitButton(BUTTON_QUIT, RUIT_QUIT);
        
        // TODO: Move this to CEGUI
        Math::Point pos, ddim;
        std::string     name;
        pos.x  = 0.35f;
        pos.y  = 0.10f;
        ddim.x = 0.30f;
        ddim.y = 0.80f;
        pw = CRobotMain::GetInstancePointer()->GetInterface()->CreateWindows(pos, ddim, 10, EVENT_WINDOW5);
        GetResource(RES_UI, RUIT_TITLE_INIT, name);
        pw->SetName(name);
    }
    
    if(m_page == MENUPAGE_STARTGAME)
    {
        LoadLayout("startgame");
        InitButton(BUTTON_MISSIONS, RUIT_MISSIONS);
        InitButton(BUTTON_FREEGAME, RUIT_FREEGAME);
        InitButton(BUTTON_EXERCISES, RUIT_EXERCISES);
        InitButton(BUTTON_CHALLENGES, RUIT_CHALLENGES);
        InitButton(BUTTON_USERLVL, RUIT_USERLVL);
        InitButton(BUTTON_BACK, RUIT_BACK);
        
        // TODO: Move this to CEGUI
        pos.x  = 0.35f;
        pos.y  = 0.10f;
        ddim.x = 0.30f;
        ddim.y = 0.80f;
        pw = CRobotMain::GetInstancePointer()->GetInterface()->CreateWindows(pos, ddim, 10, EVENT_WINDOW5);
        GetResource(RES_UI, RUIT_TITLE_STARTGAME, name);
        pw->SetName(name);
    }
    
    if(m_page == MENUPAGE_MULTIPLAYER)
    {
        LoadLayout("multiplayer");
        InitButton(BUTTON_BACK, RUIT_BACK);
        
        // TODO: Move this to CEGUI
        pos.x  = 0.35f;
        pos.y  = 0.10f;
        ddim.x = 0.30f;
        ddim.y = 0.80f;
        pw = CRobotMain::GetInstancePointer()->GetInterface()->CreateWindows(pos, ddim, 10, EVENT_WINDOW5);
        GetResource(RES_UI, RUIT_TITLE_MULTIPLAYER, name);
        pw->SetName(name);
        
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
    
    // TODO: CEGUI
    pos.x  = 540.0f/640.0f;
    pos.y  =   9.0f/480.0f;
    ddim.x =  90.0f/640.0f;
    ddim.y =  10.0f/480.0f;
    CLabel* pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL1, COLOBOT_VERSION_DISPLAY);
    pl->SetFontType(Gfx::FONT_COURIER);
    pl->SetFontSize(9.0f);
    
    Gfx::CEngine* engine = Gfx::CEngine::GetInstancePointer();
    engine->SetBackground("interface.png",
            Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
            Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
            Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
            Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
            true);
    engine->SetBackForce(true);
}

void CScreenMenu::InitButton(std::string button, ResUiTextType textId)
{
    m_window->getChildRecursive(button)->subscribeEvent(CEGUI::Window::EventMouseClick, CEGUI::Event::Subscriber(&CScreenMenu::OnClick, this));
    
    std::string translated;
    GetResource(RES_UI, textId, translated);
    std::vector<std::string> strings;
    boost::split(strings, translated, boost::is_any_of("\\"));
    
    m_window->getChildRecursive(button)->setText(reinterpret_cast<const CEGUI::utf8*>(strings[0].c_str()));
    m_window->getChildRecursive(button)->setTooltipText(reinterpret_cast<const CEGUI::utf8*>(strings[1].c_str())); //TODO: For some reason tooltips don't work
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
    m_window = CEGUI::WindowManager::getSingleton().loadLayoutFromFile(name+".layout");
    CEGUI::System::getSingleton().getDefaultGUIContext().getRootWindow()->addChild(m_window);
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
                CApplication::GetInstancePointer()->GetSound()->Play(SOUND_TZOING);
                CRobotMain::GetInstancePointer()->ChangePhase(PHASE_GENERIC);
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

