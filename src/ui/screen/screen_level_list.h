/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2015, Daniel Roux, EPSITEC SA & TerranovaTeam
 * http://epsiteс.ch; http://colobot.info; http://github.com/colobot
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

#include "object/level_category.h"

#include <map>
#include <vector>

namespace Ui {

class CScreenLevelList : public CScreen
{
public:
    CScreenLevelList(LevelCategory category);
    void CreateInterface();
    bool EventProcess(const Event &event);

    static void SetSelection(LevelCategory category, int chap, int rank);

    static bool GetSceneSoluce();

    void AllMissionUpdate();
    void ShowSoluceUpdate();

    static void UpdateChapterPassed();
    static void NextMission();

    static void UpdateCustomLevelList();
    static std::string GetCustomLevelName(int id);
    static const std::vector<std::string>& GetCustomLevelList();

protected:
    void UpdateSceneChap(int &chap);
    void UpdateSceneList(int chap, int &sel);
    void UpdateSceneResume(int chap, int rank);

protected:
    static LevelCategory m_category;
    static LevelCategory m_listCategory;

    static bool m_bSceneSoluce;

    static std::map<LevelCategory, int> m_chap;     // selected chapter (0..8)
    static std::map<LevelCategory, int> m_sel;      // chosen mission (0..98)
    static int m_maxList;

    static std::vector<std::string> m_customLevelList;

    int m_accessChap;
};

} // namespace Ui
