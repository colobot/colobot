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

#include "ui/screen/screen.h"


#include "common/restext.h"

#include <CEGUI/CEGUI.h>
#include <string>

namespace Ui {

class CScreenMainMenu : public CScreen
{
public:
    CScreenMainMenu();
    ~CScreenMainMenu();
    void Start();
    void Stop();
    bool EventProcess(const Event &event);
    void ChangePhase(Phase phase);

protected:
    const std::string BUTTON_MISSIONS   = "missions_button";
    const std::string BUTTON_FREEGAME   = "freegame_button";
    const std::string BUTTON_EXERCISES  = "exercises_button";
    const std::string BUTTON_CHALLENGES = "challenges_button";
    const std::string BUTTON_USERLVL    = "userlvl_button";
    const std::string BUTTON_SETUP      = "setup_button";
    const std::string BUTTON_NAME       = "name_button";
    const std::string BUTTON_QUIT       = "quit_button";

    void InitButton(std::string button, ResUiTextType textId);
    bool OnClick(const CEGUI::EventArgs& e);
    
    CEGUI::Window* m_window;
};

}

