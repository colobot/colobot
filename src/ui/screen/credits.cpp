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


#include "ui/screen/credits.h"

#include "app/app.h"

#include "common/logger.h"


namespace Ui {

CScreenCredits::CScreenCredits()
{
    m_window = nullptr;
    m_currentWindow = nullptr;
    m_currentWindowAnim = nullptr;
    m_currentWindowIdx = 0;
    
    CEGUI::AnimationManager::getSingleton().loadAnimationsFromXML("credits.animations");
    
    m_enabledPhases.insert(PHASE_CREDITS);
}

CScreenCredits::~CScreenCredits()
{
    Stop();
}

void CScreenCredits::Start()
{
    m_window = CEGUI::WindowManager::getSingleton().loadLayoutFromFile("credits.layout");
    CEGUI::System::getSingleton().getDefaultGUIContext().getRootWindow()->addChild(m_window);
    CApplication::GetInstancePointer()->GetSound()->PlayMusic("Prototype.ogg", false, 1.5f);
    
    InitText("credits0_title", RUIT_GAMENAME);
    InitText("credits1_title", RUIT_DIRECTOR);
    InitText("credits2_title", RUIT_PROGRAMMERS);
    InitText("credits3_title", RUIT_2DGRAPHICS);
    InitText("credits4_title", RUIT_3DGRAPHICS);
    InitText("credits5_title", RUIT_GOLD_BY);
    InitText("credits6_title", RUIT_ORIGINAL_BY);
    InitText("credits7_title", RUIT_ORIGINAL_CREATORS);
    
    Next();
}

void CScreenCredits::Stop()
{
    if(m_currentWindowAnim != nullptr) {
        CEGUI::AnimationManager::getSingleton().destroyAnimationInstance(m_currentWindowAnim);
        m_currentWindowAnim = nullptr;
    }
    
    if(m_window != nullptr) {
        CEGUI::WindowManager::getSingleton().destroyWindow(m_window);
        m_window = nullptr;
    }
    
    m_currentWindow = nullptr;
    m_currentWindowIdx = 0;
}

void CScreenCredits::InitText(std::string name, ResUiTextType text)
{
    std::string translated;
    GetResource(RES_UI, text, translated);
    
    m_window->getChildRecursive(name)->setText(reinterpret_cast<const CEGUI::utf8*>(translated.c_str()));
}

void CScreenCredits::Next()
{
    if(m_currentWindowAnim != nullptr && m_currentWindowAnim->isRunning())
    {
        CEGUI::AnimationManager::getSingleton().destroyAnimationInstance(m_currentWindowAnim);
        m_currentWindowAnim = nullptr;
    }
    
    if(m_currentWindowIdx == m_window->getChildCount()-1) {
        m_currentWindow = nullptr;
        return;
    }
    
    m_currentWindow = m_window->getChildAtIdx(++m_currentWindowIdx);

    m_currentWindowAnim = CEGUI::AnimationManager::getSingleton().instantiateAnimation(m_currentWindow->getName()+ANIMATION_SUFFIX);
    m_currentWindowAnim->setTargetWindow(m_currentWindow);
    m_currentWindow->subscribeEvent(CEGUI::AnimationInstance::EventAnimationEnded, CEGUI::Event::Subscriber(&CScreenCredits::OnAnimationEnded, this));
    m_currentWindowAnim->start();
}

bool CScreenCredits::OnAnimationEnded(const CEGUI::EventArgs& e)
{
    Next();
    return true;
}

bool CScreenCredits::EventProcess(const Event &event)
{
    if ( event.type == EVENT_KEY_DOWN )
    {
        if ( event.key.key == KEY(ESCAPE) )
        {
            CApplication::GetInstancePointer()->GetSound()->PlayMusic("Intro1.ogg", false, 1.5f);
            CRobotMain::GetInstancePointer()->ChangePhase(PHASE_MENU);
        }
        return true;
    }

    if ( event.type == EVENT_FRAME )
    {
        if( m_currentWindow == nullptr )
        {
            CApplication::GetInstancePointer()->GetSound()->PlayMusic("Intro1.ogg", false, 1.5f);
            CRobotMain::GetInstancePointer()->ChangePhase(PHASE_MENU);
        }
    }
    
    return true;
}

void CScreenCredits::ChangePhase(Phase phase)
{
}

}

