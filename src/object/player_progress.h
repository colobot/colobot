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

#include "graphics/core/color.h"

#include "object/level_category.h"

#include <string>
#include <map>

struct LevelInfo
{
    int     numTry;
    bool    bPassed;
};

struct PlayerApperance
{
    int     face;           // face
    int     glasses;        // glasses
    Gfx::Color   colorHair;      // hair color
    Gfx::Color   colorCombi;     // spacesuit volor
    Gfx::Color   colorBand;      // strips color

    void DefPerso();
};

class CPlayerProgress
{
public:
    CPlayerProgress(std::string playerName);
    static std::string GetLastName();
    ~CPlayerProgress();

    bool Delete();

    std::string GetName();

    std::string GetSaveDir();
    std::string GetSaveFile(std::string filename);

    void IncrementLevelTryCount(LevelCategory cat, int chap, int rank);
    int GetLevelTryCount(LevelCategory cat, int chap, int rank);
    void SetLevelPassed(LevelCategory cat, int chap, int rank, bool bPassed);
    bool GetLevelPassed(LevelCategory cat, int chap, int rank);
    int GetChapPassed(LevelCategory cat);

    void SetSelectedChap(LevelCategory category, int chap);
    int GetSelectedChap(LevelCategory category);
    void SetSelectedRank(LevelCategory category, int rank);
    int GetSelectedRank(LevelCategory category);

    int GetFreeGameBuildUnlock();
    void SetFreeGameBuildUnlock(int freeBuild);
    int GetFreeGameResearchUnlock();
    void SetFreeGameResearchUnlock(int freeResearch);

    PlayerApperance& GetApperance();
    void LoadApperance();
    void SaveApperance();

    void SaveScene(std::string dir, std::string info);

protected:
    void LoadFinishedLevels(LevelCategory category);
    void SaveFinishedLevels(LevelCategory category);

    void LoadFreeGameUnlock();
    void SaveFreeGameUnlock();

protected:
    //! Player name
    std::string m_playerName;

    //! Is finished levels file loaded already?
    std::map<LevelCategory, bool> m_levelInfoLoaded;
    //! Level completion info
    std::map<LevelCategory, std::map<int, std::map<int, LevelInfo>>> m_levelInfo;
    //! Selected level chapter
    std::map<LevelCategory, int> m_selectChap;
    //! Selected level rank
    std::map<LevelCategory, int> m_selectRank;

    //! Is freegame save file loaded already?
    bool m_freegameLoaded;
    //! Buildings unlocked for free game
    int m_freegameBuild;
    //! Researches unlocked for free game
    int m_freegameResearch;

    //! Player apperance
    PlayerApperance m_apperance;
};
