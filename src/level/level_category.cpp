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

#include "level/level_category.h"

#include <map>

// NOTE: Because of how save filenames are built, the first letter of category directories MUST be unique!!
// TODO: I'm not sure about "challenges" + "custom". It may be messing things up already right now.
const std::map<LevelCategory, std::string> CATEGORY_DIR_MAP = {
    { LevelCategory::Missions,     "missions"     },
    { LevelCategory::FreeGame,     "freemissions" },
    { LevelCategory::Exercises,    "exercises"    },
    { LevelCategory::Challenges,   "challenges"   },
    { LevelCategory::CodeBattles,  "battles"      },
    { LevelCategory::CustomLevels, "custom"       },
};

std::string GetLevelCategoryDir(LevelCategory category)
{
    return CATEGORY_DIR_MAP.at(category);
}

LevelCategory GetLevelCategoryFromDir(std::string dir)
{
    for(auto it = CATEGORY_DIR_MAP.begin(); it != CATEGORY_DIR_MAP.end(); ++it)
    {
        if(it->second == dir)
        {
            return it->first;
        }
    }
    return LevelCategory::Max;
}
