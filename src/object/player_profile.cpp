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
#include "object/player_profile.h"

#include "common/logger.h"
#include "common/make_unique.h"
#include "common/config_file.h"
#include "common/resources/resourcemanager.h"
#include "common/resources/inputstream.h"
#include "common/resources/outputstream.h"
#include "common/restext.h"

#include "object/level/parser.h"
#include "object/robotmain.h"

void PlayerApperance::DefPerso()
{
    this->colorCombi.r = 206.0f/256.0f;
    this->colorCombi.g = 206.0f/256.0f;
    this->colorCombi.b = 204.0f/256.0f;  // ~white
    this->colorBand.r  = 255.0f/256.0f;
    this->colorBand.g  = 132.0f/256.0f;
    this->colorBand.b  =   1.0f/256.0f;  // orange

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
        this->colorHair.r =  83.0f/256.0f;
        this->colorHair.g =  64.0f/256.0f;
        this->colorHair.b =  51.0f/256.0f;  // brown
    }
    if ( this->face == 2 )  // carlos ?
    {
        this->glasses = 1;
        this->colorHair.r =  85.0f/256.0f;
        this->colorHair.g =  48.0f/256.0f;
        this->colorHair.b =   9.0f/256.0f;  // brown
    }
    if ( this->face == 3 )  // blond ?
    {
        this->glasses = 4;
        this->colorHair.r = 255.0f/256.0f;
        this->colorHair.g = 255.0f/256.0f;
        this->colorHair.b = 181.0f/256.0f;  // yellow
    }

    this->colorHair.a  = 0.0f;
    this->colorCombi.a = 0.0f;
    this->colorBand.a  = 0.0f;
}

CPlayerProfile::CPlayerProfile(std::string playerName)
{
    m_playerName = playerName;
    GetConfigFile().SetStringProperty("Gamer", "LastName", m_playerName);

    if (!CResourceManager::DirectoryExists(GetSaveDir()))
    {
        CResourceManager::CreateDirectory(GetSaveDir());
    }

    m_freegameLoaded = false;

    for(int i = 0; i < static_cast<int>(LevelCategory::Max); i++)
    {
        m_levelInfoLoaded[static_cast<LevelCategory>(i)] = false;
    }

    LoadApperance();
}

CPlayerProfile::~CPlayerProfile()
{
}

std::string CPlayerProfile::GetLastName()
{
    std::string name;

    if(!GetConfigFile().GetStringProperty("Gamer", "LastName", name))
        GetResource(RES_TEXT, RT_NAME_DEFAULT, name);

    return name;
}

std::vector<std::string> CPlayerProfile::GetPlayerList()
{
    return CResourceManager::ListDirectories("savegame");
}

bool CPlayerProfile::Delete()
{
    return CResourceManager::RemoveDirectory(GetSaveDir());
}

std::string CPlayerProfile::GetName()
{
    return m_playerName;
}

std::string CPlayerProfile::GetSaveDir()
{
    return "savegame/" + m_playerName;
}

std::string CPlayerProfile::GetSaveFile(std::string filename)
{
    return GetSaveDir() + "/" + filename;
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
            return j;
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
    std::string filename = GetSaveFile(GetLevelCategoryDir(category)+".gam");

    if (!CResourceManager::Exists(filename))
        return;

    CInputStream file;
    file.open(filename);
    if (!file.is_open())
    {
        GetLogger()->Error("Unable to read list of finished levels from '%s'\n", filename.c_str());
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
    std::string filename = GetSaveFile(GetLevelCategoryDir(category)+".gam");
    COutputStream file;
    file.open(filename);
    if (!file.is_open())
    {
        GetLogger()->Error("Unable to read list of finished missions from '%s'\n", filename.c_str());
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

    std::string filename = GetSaveFile("research.gam");

    if (!CResourceManager::Exists(filename))
        return;

    CInputStream file;
    file.open(filename);
    if (!file.is_open())
    {
        GetLogger()->Error("Unable to read free game unlock state from '%s'\n", filename.c_str());
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
    std::string filename = GetSaveFile("research.gam");
    COutputStream file;
    file.open(filename);
    if (!file.is_open())
    {
        GetLogger()->Error("Unable to write free game unlock state to '%s'\n", filename.c_str());
        return;
    }

    file << "research=" << m_freegameResearch << " build=" << m_freegameBuild << "\n";

    file.close();
}

// APPERANCE

PlayerApperance& CPlayerProfile::GetApperance()
{
    return m_apperance;
}

void CPlayerProfile::LoadApperance()
{
    m_apperance.face = 0;
    m_apperance.DefPerso();

    std::string filename = GetSaveFile("face.gam");
    if (!CResourceManager::Exists(filename))
        return;

    try
    {
        CLevelParser apperanceParser(filename);
        apperanceParser.Load();
        CLevelParserLine* line;

        line = apperanceParser.Get("Head");
        m_apperance.face = line->GetParam("face")->AsInt();
        m_apperance.glasses = line->GetParam("glasses")->AsInt();
        m_apperance.colorHair = line->GetParam("hair")->AsColor();

        line = apperanceParser.Get("Body");
        m_apperance.colorCombi = line->GetParam("combi")->AsColor();
        m_apperance.colorBand = line->GetParam("band")->AsColor();
    }
    catch (CLevelParserException& e)
    {
        GetLogger()->Error("Unable to read personalized player apperance: %s\n", e.what());
    }
}

void CPlayerProfile::SaveApperance()
{
    try
    {
        CLevelParser apperanceParser(GetSaveFile("face.gam"));
        CLevelParserLineUPtr line;

        line = MakeUnique<CLevelParserLine>("Head");
        line->AddParam("face", MakeUnique<CLevelParserParam>(m_apperance.face));
        line->AddParam("glasses", MakeUnique<CLevelParserParam>(m_apperance.glasses));
        line->AddParam("hair", MakeUnique<CLevelParserParam>(m_apperance.colorHair));
        apperanceParser.AddLine(std::move(line));

        line = MakeUnique<CLevelParserLine>("Body");
        line->AddParam("combi", MakeUnique<CLevelParserParam>(m_apperance.colorCombi));
        line->AddParam("band", MakeUnique<CLevelParserParam>(m_apperance.colorBand));
        apperanceParser.AddLine(std::move(line));

        apperanceParser.Save();
    }
    catch (CLevelParserException& e)
    {
        GetLogger()->Error("Unable to write personalized player apperance: %s\n", e.what());
    }
}

// SAVE / LOAD SCENE

void CPlayerProfile::SaveScene(std::string dir, std::string info)
{
    if (!CResourceManager::DirectoryExists(dir))
    {
        CResourceManager::CreateDirectory(dir);
    }

    std::string savegameFileName = dir + "/data.sav";
    std::string fileCBot = CResourceManager::GetSaveLocation() + "/" + dir + "/cbot.run";
    CRobotMain::GetInstancePointer()->IOWriteScene(savegameFileName.c_str(), fileCBot.c_str(), const_cast<char*>(info.c_str()));
    CRobotMain::GetInstancePointer()->MakeSaveScreenshot(dir + "/screen.png");
}
