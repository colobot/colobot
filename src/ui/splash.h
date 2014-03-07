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

#include <CEGUI/CEGUI.h>
#include <set>

namespace Ui {

class CSplash
{
public:
    CSplash();
    ~CSplash();
    
    void UpdateSplashList();
    void Start();
    void Next();
    bool OnAnimationEnded(const CEGUI::EventArgs& e);
    bool IsFinished();
    
protected:
    const std::string SPLASH_IMAGESET = "ColobotSplash";
    const std::string SPLASH_ANIMATION = "Colobot/Splash";
    
    CEGUI::Window* m_splashWindow;
    CEGUI::AnimationInstance* m_splashWindowAnim;
    
    std::set<CEGUI::String> m_splashes;
};

}

