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

#include "level/player_profile.h"

#include "common/config_file.h"
#include "common/logger.h"
#include "common/restext.h"
#include "common/stringutils.h"

#include "common/resources/inputstream.h"
#include "common/resources/outputstream.h"
#include "common/resources/resourcemanager.h"
#include "common/version.h"

#include "level/robotmain.h"

#include "level/parser/parser.h"


void PlayerAppearance::DefPerso()
{
    this->colorCombi.r = 206.0f/256.0f;
    this->colorCombi.g = 206.0f/256.0f;
    this->colorCombi.b = 204.0f/256.0f;  // ~white
    this->colorBand.r = 255.0f / 256.0f;
    this->colorBand.g = 132.0f / 256.0f;
    this->colorBand.b = 1.0f / 256.0f;  // orange

    if ( this->face == 0 )  // normal ?
    {
        this->glasses = 0;
        this->colorHair.r =  90.0f/256.0f;
        this->colorHair.g =  95.0f/256.0f;
        this->colorHair.b =  85.0f/256.0f;  // black
    }
    if ( this->face == 1 )  // bald ?
    {
        this->glasses = 0;
        this->colorHair.r = 74.0f / 256.0f;
        this->colorHair.g = 58.0f / 256.0f;
        this->colorHair.b = 46.0f / 256.0f;  // brown
    }
    if ( this->face == 2 )  // carlos ?
    {
        this->glasses = 1;
        this->colorHair.r = 70.0f / 256.0f;
        this->colorHair.g = 40.0f / 256.0f;
        this->colorHair.b =   9.0f/256.0f;  // brown
    }
    if ( this->face == 3 )  // blond ? -> ginger ?
    {
        this->glasses = 4;
        this->colorHair.r = 74.0f / 256.0f;
        this->colorHair.g = 16.0f / 256.0f;
        this->colorHair.b = 0.0f / 256.0f;  // yellow, changed to ginger
    }

    this->colorHair.a  = 0.0f;
    this->colorCombi.a = 0.0f;
    this->colorBand.a  = 0.0f;
}

void PlayerAppearance::DefHairColor()
{
    if (this->face == 0)  // normal ?
    {
        this->colorHair.r = 90.0f / 256.0f;
        this->colorHair.g = 95.0f / 256.0f;
        this->colorHair.b = 85.0f / 256.0f;  // black
    }
    if (this->face == 1)  // bald ?
    {
        this->colorHair.r = 74.0f / 256.0f;
        this->colorHair.g = 58.0f / 256.0f;
        this->colorHair.b = 46.0f / 256.0f;  // brown
    }
    if (this->face == 2)  // carlos ?
    {
        this->colorHair.r = 70.0f / 256.0f;
        this->colorHair.g = 40.0f / 256.0f;
        this->colorHair.b = 9.0f / 256.0f;  // brown
    }
    if (this->face == 3)  // blond ? -> ginger ?
    {
        this->colorHair.r = 74.0f / 256.0f;
        this->colorHair.g = 16.0f / 256.0f;
        this->colorHair.b = 0.0f / 256.0f;  // yellow, changed to ginger
    }

    this->colorHair.a = 0.0f;
}


CPlayerProfile::CPlayerProfile(std::string playerName)
{
    m_playerName = playerName;
    GetConfigFile().SetStringProperty("Gamer", "LastName", m_playerName);
    GetConfigFile().Save();

    m_freegameLoaded = false;
    m_freegameBuild = 0;
    m_freegameResearch = 0;

    for(int i = 0; i < static_cast<int>(LevelCategory::Max); i++)
    {
        m_levelInfoLoaded[static_cast<LevelCategory>(i)] = false;
    }

    LoadAppearance();
}

CPlayerProfile::~CPlayerProfile()
{
}

std::string CPlayerProfile::GetLastName()
{
    std::string name;

    if(!GetConfigFile().GetStringProperty("Gamer", "LastName", name) || name.empty())
        GetResource(RES_TEXT, RT_NAME_DEFAULT, name);

    return name;
}

std::vector<std::string> CPlayerProfile::GetPlayerList()
{
    std::vector<std::string> players;

    for (const auto& path : CResourceManager::ListDirectories("savegame"))
        players.push_back(StrUtils::ToString(path));

    return players;
}

bool CPlayerProfile::Create()
{
    if (!CResourceManager::DirectoryExists(GetSaveDir()))
    {
        return CResourceManager::CreateNewDirectory(GetSaveDir());
    }
    return true;
}

bool CPlayerProfile::Delete()
{
    return CResourceManager::RemoveExistingDirectory(GetSaveDir());
}

std::string CPlayerProfile::GetName()
{
    return m_playerName;
}

std::filesystem::path CPlayerProfile::GetSaveDir()
{
    return std::filesystem::path("savegame") / StrUtils::ToPath(m_playerName);
}

std::filesystem::path CPlayerProfile::GetSaveFile(const std::filesystem::path& filename)
{
    return GetSaveDir() / filename;
}

// FINISHED LEVELS

void CPlayerProfile::IncrementLevelTryCount(LevelCategory cat, int chap, int rank)
{
    m_levelInfo[cat][chap][rank].numTry ++;
    SaveFinishedLevels(cat);
}

int CPlayerProfile::GetLevelTryCount(LevelCategory cat, int chap, int rank)
{
    if(!m_levelInfoLoaded[cat])
        LoadFinishedLevels(cat);
    return m_levelInfo[cat][chap][rank].numTry;
}

void CPlayerProfile::SetLevelPassed(LevelCategory cat, int chap, int rank, bool bPassed)
{
    m_levelInfo[cat][chap][rank].bPassed = bPassed;
    SaveFinishedLevels(cat);

    if (bPassed && rank != 0)
    {
        assert(cat == CRobotMain::GetInstancePointer()->GetLevelCategory() && chap == CRobotMain::GetInstancePointer()->GetLevelChap()); //TODO: Refactor UpdateChapterPassed
        CRobotMain::GetInstancePointer()->UpdateChapterPassed();
    }
}

bool CPlayerProfile::GetLevelPassed(LevelCategory cat, int chap, int rank)
{
    if(!m_levelInfoLoaded[cat])
        LoadFinishedLevels(cat);
    return m_levelInfo[cat][chap][rank].bPassed;
}

int CPlayerProfile::GetChapPassed(LevelCategory cat)
{
    if ( CRobotMain::GetInstancePointer()->GetShowAll() )  return MAXSCENE;

    for ( int j = 1; j <= MAXSCENE; j++ )
    {
        if ( !GetLevelPassed(cat, j, 0) )
        {
            return j-1;
        }
    }
    return MAXSCENE;
}

void CPlayerProfile::SetSelectedChap(LevelCategory category, int chap)
{
    m_selectChap[category] = chap;
    SaveFinishedLevels(category);
}

int CPlayerProfile::GetSelectedChap(LevelCategory category)
{
    if(!m_levelInfoLoaded[category])
        LoadFinishedLevels(category);
    if(m_selectChap[category] < 1) return 1;
    return m_selectChap[category];
}

void CPlayerProfile::SetSelectedRank(LevelCategory category, int rank)
{
    m_selectRank[category] = rank;
    SaveFinishedLevels(category);
}

int CPlayerProfile::GetSelectedRank(LevelCategory category)
{
    if(!m_levelInfoLoaded[category])
        LoadFinishedLevels(category);
    if(m_selectRank[category] < 1) return 1;
    return m_selectRank[category];
}

void CPlayerProfile::LoadFinishedLevels(LevelCategory category)
{
    m_levelInfo[category].clear();
    std::filesystem::path filename = GetSaveFile(StrUtils::ToPath(GetLevelCategoryDir(category) + ".gam"));

    if (!CResourceManager::Exists(filename))
        return;

    CInputStream file;
    file.open(filename);
    if (!file.is_open())
    {
        GetLogger()->Error("Unable to read list of finished levels from '%%'", filename);
        return;
    }

    std::string line;
    std::getline(file, line);
    sscanf(line.c_str(), "CurrentChapter=%d CurrentSel=%d\n", &m_selectChap[category], &m_selectRank[category]);

    while (!file.eof())
    {
        std::getline(file, line);
        if (line == "")
        {
            break;
        }

        int chap, rank, numTry, passed;
        sscanf(line.c_str(), "Chapter %d: Scene %d: numTry=%d passed=%d\n",
                &chap, &rank, &numTry, &passed);

        if ( chap < 0 || chap > MAXSCENE ) continue;
        if ( rank < 0 || rank > MAXSCENE ) continue;

        m_levelInfo[category][chap][rank].numTry  = numTry;
        m_levelInfo[category][chap][rank].bPassed = passed;
    }

    file.close();
    m_levelInfoLoaded[category] = true;
}

void CPlayerProfile::SaveFinishedLevels(LevelCategory category)
{
    std::filesystem::path filename = GetSaveFile(StrUtils::ToPath(GetLevelCategoryDir(category) + ".gam"));
    COutputStream file;
    file.open(filename);
    if (!file.is_open())
    {
        GetLogger()->Error("Unable to read list of finished missions from '%%'", filename);
        return;
    }

    file << "CurrentChapter=" << m_selectChap[category] << " CurrentSel=" << m_selectRank[category] << "\n";

    for (int chap = 0; chap <= MAXSCENE ; chap++)
    {
        if (m_levelInfo[category].find(chap) == m_levelInfo[category].end()) continue;
        for(int rank = 0; rank <= MAXSCENE; rank++)
        {
            if (m_levelInfo[category][chap].find(rank) == m_levelInfo[category][chap].end()) continue;
            if (m_levelInfo[category][chap][rank].numTry == 0 && !m_levelInfo[category][chap][rank].bPassed)  continue;

            file << "Chapter " << chap << ": Scene " << rank << ": numTry=" << m_levelInfo[category][chap][rank].numTry << " passed=" << (m_levelInfo[category][chap][rank].bPassed ? "1" : "0") << "\n";
        }
    }

    file.close();
}

// FREE GAME UNLOCK

int CPlayerProfile::GetFreeGameBuildUnlock()
{
    if(!m_freegameLoaded)
        LoadFreeGameUnlock();

    return m_freegameBuild;
}

void CPlayerProfile::SetFreeGameBuildUnlock(int freeBuild)
{
    m_freegameBuild = freeBuild;
    SaveFreeGameUnlock();
}

int CPlayerProfile::GetFreeGameResearchUnlock()
{
    if(!m_freegameLoaded)
        LoadFreeGameUnlock();
    return m_freegameResearch;
}

void CPlayerProfile::SetFreeGameResearchUnlock(int freeResearch)
{
    m_freegameResearch = freeResearch;
    SaveFreeGameUnlock();
}

void CPlayerProfile::LoadFreeGameUnlock()
{
    m_freegameResearch = 0;
    m_freegameBuild    = 0;

    std::filesystem::path filename = GetSaveFile("research.gam");

    if (!CResourceManager::Exists(filename))
        return;

    CInputStream file;
    file.open(filename);
    if (!file.is_open())
    {
        GetLogger()->Error("Unable to read free game unlock state from '%%'", filename);
        return;
    }

    std::string line;
    std::getline(file, line);
    sscanf(line.c_str(), "research=%d build=%d\n", &m_freegameResearch, &m_freegameBuild);

    file.close();

    m_freegameLoaded = false;
}

void CPlayerProfile::SaveFreeGameUnlock()
{
    std::filesystem::path filename = GetSaveFile("research.gam");
    COutputStream file;
    file.open(filename);
    if (!file.is_open())
    {
        GetLogger()->Error("Unable to write free game unlock state to '%%'", filename);
        return;
    }

    file << "research=" << m_freegameResearch << " build=" << m_freegameBuild << "\n";

    file.close();
}

// APPEARANCE

PlayerAppearance& CPlayerProfile::GetAppearance()
{
    return m_appearance;
}

void CPlayerProfile::LoadAppearance()
{
    m_appearance.face = 0;
    m_appearance.DefPerso();

    std::filesystem::path filename = GetSaveFile("face.gam");
    if (!CResourceManager::Exists(filename))
        return;

    try
    {
        CLevelParser appearanceParser(filename);
        appearanceParser.Load();
        CLevelParserLine* line;

        line = appearanceParser.Get("Head");
        m_appearance.face = line->GetParam("face")->AsInt();
        m_appearance.glasses = line->GetParam("glasses")->AsInt();
        m_appearance.colorHair = line->GetParam("hair")->AsColor();

        line = appearanceParser.Get("Body");
        m_appearance.colorCombi = line->GetParam("combi")->AsColor();
        m_appearance.colorBand = line->GetParam("band")->AsColor();
    }
    catch (CLevelParserException& e)
    {
        GetLogger()->Error("Unable to read personalized player appearance: %%", e.what());
    }
}

void CPlayerProfile::SaveAppearance()
{
    try
    {
        CLevelParser appearanceParser(GetSaveFile("face.gam"));
        CLevelParserLineUPtr line;

        line = std::make_unique<CLevelParserLine>("Head");
        line->AddParam("face", std::make_unique<CLevelParserParam>(m_appearance.face));
        line->AddParam("glasses", std::make_unique<CLevelParserParam>(m_appearance.glasses));
        line->AddParam("hair", std::make_unique<CLevelParserParam>(m_appearance.colorHair));
        appearanceParser.AddLine(std::move(line));

        line = std::make_unique<CLevelParserLine>("Body");
        line->AddParam("combi", std::make_unique<CLevelParserParam>(m_appearance.colorCombi));
        line->AddParam("band", std::make_unique<CLevelParserParam>(m_appearance.colorBand));
        appearanceParser.AddLine(std::move(line));

        appearanceParser.Save();
    }
    catch (CLevelParserException& e)
    {
        GetLogger()->Error("Unable to write personalized player appearance: %%", e.what());
    }
}

// SAVE / LOAD SCENE

bool CPlayerProfile::HasAnySavedScene()
{
    auto saveDirs = CResourceManager::ListDirectories(GetSaveDir());
    for (auto dir : saveDirs)
    {
        if (CResourceManager::Exists(GetSaveFile(dir / "data.sav")))
        {
            return true;
        }
    }
    return false;
}

std::vector<SavedScene> CPlayerProfile::GetSavedSceneList()
{
    auto saveDirs = CResourceManager::ListDirectories(GetSaveDir());
    std::map<int, SavedScene> sortedSaveDirs;

    for (auto dir : saveDirs)
    {
        std::filesystem::path savegameFile = GetSaveFile(dir / "data.sav");
        if (CResourceManager::Exists(savegameFile) && CResourceManager::GetFileSize(savegameFile) > 0)
        {
            CLevelParser levelParser(savegameFile);
            levelParser.Load();
            CLevelParserLine* line = levelParser.GetIfDefined("Created");
            int time = line != nullptr ? line->GetParam("date")->AsInt() : 0;
            try
            {
                sortedSaveDirs[time] = SavedScene{ GetSaveFile(dir),
                    levelParser.Get("Title")->GetParam("text")->AsString() };
            }
            catch (CLevelParserException &e)
            {
                GetLogger()->Error("Error trying to load savegame title: %%", e.what());
            }
        }
    }

    std::vector<SavedScene> result;
    for (auto dir : sortedSaveDirs)
    {
        result.push_back(dir.second);
    }
    return result;
}

void CPlayerProfile::SaveScene(const std::filesystem::path& dir, std::string info)
{
    if (!CResourceManager::DirectoryExists(dir))
    {
        CResourceManager::CreateNewDirectory(dir);
    }

    CRobotMain::GetInstancePointer()->IOWriteScene(
        dir / "data.sav", dir / "cbot.run", dir / "screen.png", info);
}

void CPlayerProfile::LoadScene(const std::filesystem::path& dir)
{
    CLevelParser levelParser(dir / "data.sav");
    levelParser.Load();

    LevelCategory cat;
    int chap;
    int rank;

    CLevelParserLine* line = levelParser.Get("Mission");
    cat = GetLevelCategoryFromDir(line->GetParam("base")->AsString());

    if(!m_levelInfoLoaded[cat])
        LoadFinishedLevels(cat);

    rank = line->GetParam("rank")->AsInt();
    if (cat == LevelCategory::CustomLevels)
    {
        chap = 0;
        // Read "dir" as a string to avoid interpreting %lvl% etc.
        std::filesystem::path dir = StrUtils::ToPath(line->GetParam("dir")->AsString());
        CRobotMain::GetInstancePointer()->UpdateCustomLevelList();
        auto customLevelList = CRobotMain::GetInstancePointer()->GetCustomLevelList();
        for (unsigned int i = 0; i < customLevelList.size(); i++)
        {
            if (customLevelList[i] == dir)
            {
                chap = i+1;
                break;
            }
        }
        if (chap == 0) return; //TODO: Exception
    }
    else
    {
        if(line->GetParam("chap")->IsDefined())
        {
            chap = line->GetParam("chap")->AsInt();
        }
        else
        {
            // Backwards compatibility
            chap = rank/100;
            rank = rank%100;
        }
    }

    CRobotMain::GetInstancePointer()->SetLevel(cat, chap, rank);
    CRobotMain::GetInstancePointer()->SetReadScene(dir);
    CRobotMain::GetInstancePointer()->ChangePhase(PHASE_SIMUL);
}

bool CPlayerProfile::DeleteScene(const std::filesystem::path& dir)
{
    if (CResourceManager::DirectoryExists(dir))
    {
        return CResourceManager::RemoveExistingDirectory(dir);
    }
    return false;
}

bool IsVersionSaveSupported(const std::filesystem::path &dir)
{
    CLevelParser levelParser(dir / "data.sav");
    levelParser.Load();
    CLevelParserLine* ver_line = levelParser.GetIfDefined("GameVersion");
    return (
        ver_line
        && ver_line->GetParam("major")->AsInt() == Version::MAJOR
        && ver_line->GetParam("minor")->AsInt() == Version::MINOR
        && ver_line->GetParam("patch")->AsInt() == Version::PATCH
        && ver_line->GetParam("tweak")->AsInt() <= Version::TWEAK
    );
}
