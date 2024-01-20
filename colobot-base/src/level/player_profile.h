/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2023, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include "graphics/core/color.h"

#include "level/level_category.h"

#include <filesystem>
#include <string>
#include <map>
#include <vector>

struct LevelInfo
{
    int     numTry = 0;
    bool    bPassed = false;
};

struct PlayerAppearance
{
    int     face = 0;           // face
    int     glasses = 0;        // glasses
    Gfx::Color   colorHair;      // hair color
    Gfx::Color   colorCombi;     // spacesuit volor
    Gfx::Color   colorBand;      // strips color

    void DefPerso();
    void DefHairColor();
};

struct SavedScene
{
    std::string path;
    std::string name;

    SavedScene(std::string path = "", std::string name = "")
        : path(path), name(name)
    {}
};

class CPlayerProfile
{
public:
    // Creates CPlayerProfile instance and loads player save data
    CPlayerProfile(std::string playerName);
    ~CPlayerProfile();
    // Returns last used player name, or "Player" if none found
    static std::string GetLastName();
    // Returnslist of possible player names
    static std::vector<std::string> GetPlayerList();

    // Creates player save directory (if needed)
    bool Create();
    // Removes player save directory
    bool Delete();

    //! Returns player name
    std::string GetName();

    //! Returns player's save directory path
    std::filesystem::path GetSaveDir();
    //! Returns path to file inside player's save directory
    std::filesystem::path GetSaveFile(const std::filesystem::path& filename);

    //! Increments level try count
    void IncrementLevelTryCount(LevelCategory cat, int chap, int rank);
    //! Returns try count for given level
    int GetLevelTryCount(LevelCategory cat, int chap, int rank);
    //! Changes level passed status (also updates chapter passed status)
    void SetLevelPassed(LevelCategory cat, int chap, int rank, bool bPassed);
    //! Chenks if the level (or chapter, if rank=0) is passed
    bool GetLevelPassed(LevelCategory cat, int chap, int rank);
    //! Returns the highest chapter accessible in the given category
    int GetChapPassed(LevelCategory cat);

    //! Sets selected chapter for save file
    void SetSelectedChap(LevelCategory category, int chap);
    //! Returns selected chapter from save file
    int GetSelectedChap(LevelCategory category);
    //! Sets selected level for save file
    void SetSelectedRank(LevelCategory category, int rank);
    //! Returns selected level from save file
    int GetSelectedRank(LevelCategory category);

    //! Returns unlocked EnableBuild for free game
    int GetFreeGameBuildUnlock();
    //! Saves unlocked EnableBuild for free game
    void SetFreeGameBuildUnlock(int freeBuild);
    //! Returns unlocked DoneResearch for free game
    int GetFreeGameResearchUnlock();
    //! Saves unlocked DoneResearch for free game
    void SetFreeGameResearchUnlock(int freeResearch);

    //! Returns a reference to PlayerAppearance structure
    PlayerAppearance& GetAppearance();
    //! Loads PlayerAppearance structure
    void LoadAppearance();
    //! Saves PlayerAppearance structure
    void SaveAppearance();

    //! Returns true if player has at least one saved scene
    bool HasAnySavedScene();
    //! Returns list of all saved scenes
    std::vector<SavedScene> GetSavedSceneList();
    //! Saves current scene status
    void SaveScene(const std::filesystem::path& dir, std::string info);
    //! Loads scene
    void LoadScene(const std::filesystem::path& dir);
    //! Delete saved scene
    bool DeleteScene(const std::filesystem::path& dir);

protected:
    //! Loads LevelInfo data for given category
    void LoadFinishedLevels(LevelCategory category);
    //! Saves LevelInfo data for given category
    void SaveFinishedLevels(LevelCategory category);

    //! Loads free game unlock state
    void LoadFreeGameUnlock();
    //! Saves free game unlock state
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

    //! Player appearance
    PlayerAppearance m_appearance;
};
