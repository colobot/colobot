/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2016, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#pragma once

#include "ui/screen/screen_setup.h"

namespace Ui
{

class CScreenSetupGraphics : public CScreenSetup
{
public:
    CScreenSetupGraphics();
    void SetActive() override;

    void CreateInterface() override;
    bool EventProcess(const Event &event) override;

protected:
    void UpdateSetupButtons();
    void ChangeSetupButtons();
    void ChangeSetupQuality(int quality);
    void UpdateDisplayMode();
    void ChangeDisplay();
    void UpdateApply();

protected:
    int m_setupSelMode;
    bool m_setupFull;
    bool m_change;
};

} // namespace Ui
