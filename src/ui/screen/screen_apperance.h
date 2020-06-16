/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2018, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include "ui/screen/screen.h"

namespace Ui
{

class CScreenApperance : public CScreen
{
public:
    CScreenApperance();
    void CreateInterface() override;
    bool EventProcess(const Event &event) override;

    bool GetGamerOnlyHead();
    float GetPersoAngle();

protected:
    void UpdatePerso();
    void CameraPerso();
    void FixPerso(int rank, int index);
    void ColorPerso();

    //! Move camera in that way that astronaut's origin is in (x, y) point on window
    void SetCamera(float x, float y, float cameraDistance);

protected:
    int             m_apperanceTab;         // perso: tab selected
    float           m_apperanceAngle;           // perso: angle of presentation
};

} // namespace Ui
