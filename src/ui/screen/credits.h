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

#pragma once

#include "common/restext.h"

#include "ui/screen/screen.h"

#include <CEGUI/CEGUI.h>

namespace Ui {

class CScreenCredits : public CScreen
{
public:
    CScreenCredits();
    ~CScreenCredits();
    void Start();
    void Stop();
    bool EventProcess(const Event &event);
    void ChangePhase(Phase phase);
    
protected:
    void Next();
    void InitText(std::string name, ResUiTextType text);
    bool OnAnimationEnded(const CEGUI::EventArgs& e);
    
protected:
    const std::string ANIMATION_SUFFIX = "_anim";
    
    CEGUI::Window* m_window;
    CEGUI::Window* m_currentWindow;
    unsigned int m_currentWindowIdx;
    CEGUI::AnimationInstance* m_currentWindowAnim;
};

}

