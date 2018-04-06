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

#include "ui/screen/screen.h"

#include <vector>

class CPathManager;

namespace Ui
{

class CScreenModManager : public CScreen
{
public:
    CScreenModManager();

    void CreateInterface() override;
    bool EventProcess(const Event &event) override;

    void UpdateUnloadedModList();
    void UpdateLoadedModList();

protected:
    CPathManager* m_pathManager;

    bool m_global = false;

    std::vector<std::string> m_unloadedModList;
    std::vector<std::string> m_loadedModList;
};

} // namespace Ui
