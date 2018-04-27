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

#include "common/restext.h"

namespace Ui
{

class CScreenLoading : public CScreen
{
public:
    CScreenLoading();

    void CreateInterface() override;
    void DestroyInterface();
    bool EventProcess(const Event &event) override;

    bool IsVisible();

    void SetProgress(float progress, const std::string& text, const std::string& details = "");
    void SetProgress(float progress, ResTextType text, const std::string& details = "");
    void SetProgress(float progress, ResTextType text, ResTextType details);

protected:
    bool m_visible;

    float       m_lastProgress;
    std::string m_lastText;
    std::string m_lastDetails;
};

} // namespace Ui
