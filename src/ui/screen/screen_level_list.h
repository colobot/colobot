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

#include "level/level_category.h"

#include <map>
#include <vector>

namespace Ui
{
class CMainDialog;

class CScreenLevelList : public CScreen
{
public:
    CScreenLevelList(Ui::CMainDialog* mainDialog);

    void SetLevelCategory(LevelCategory category);

    void CreateInterface() override;
    bool EventProcess(const Event &event) override;

    void SetSelection(LevelCategory category, int chap, int rank);

    bool GetSceneSoluce();

    void AllMissionUpdate();
    void ShowSoluceUpdate();

    void UpdateChapterPassed();
    void NextMission();

    void UpdateCustomLevelList();
    std::string GetCustomLevelName(int id);
    const std::vector<std::string>& GetCustomLevelList();

protected:
    void UpdateSceneChap(int &chap);
    void UpdateSceneList(int chap, int &sel);
    void UpdateSceneResume(int chap, int rank);
private:
    void DisplayActive(const short slide, const bool bUnselectEdit =false) override;

protected:
    Ui::CMainDialog* m_dialog;

    LevelCategory m_category;

    bool m_sceneSoluce;

    std::map<LevelCategory, int> m_chap;     // selected chapter (0..8)
    std::map<LevelCategory, int> m_sel;      // chosen mission (0..98)
    int m_maxList;

    std::vector<std::string> m_customLevelList;

    int m_accessChap;
};

} // namespace Ui
