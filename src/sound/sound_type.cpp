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

#include "sound/sound_type.h"

#include "common/logger.h"

#include <unordered_map>

namespace
{

const std::unordered_map<std::string, SoundType> SOUND_STRINGS =
{
    {"none", SOUND_NONE},
    {"click", SOUND_CLICK},
    {"boum", SOUND_BOUM},
    {"explo", SOUND_EXPLO},
    {"flyh", SOUND_FLYh},
    {"fly", SOUND_FLY},
    {"steps", SOUND_STEPs},
    {"motorw", SOUND_MOTORw},
    {"motort", SOUND_MOTORt},
    {"motorr", SOUND_MOTORr},
    {"error", SOUND_ERROR},
    {"convert", SOUND_CONVERT},
    {"energy", SOUND_ENERGY},
    {"plouf", SOUND_PLOUF},
    {"blup", SOUND_BLUP},
    {"warning", SOUND_WARNING},
    {"derrick", SOUND_DERRICK},
    {"labo", SOUND_LABO},
    {"station", SOUND_STATION},
    {"repair", SOUND_REPAIR},
    {"research", SOUND_RESEARCH},
    {"insects", SOUND_INSECTs},
    {"burn", SOUND_BURN},
    {"tzoing", SOUND_TZOING},
    {"ggg", SOUND_GGG},
    {"manip", SOUND_MANIP},
    {"fire", SOUND_FIRE},
    {"human1", SOUND_HUMAN1},
    {"stepw", SOUND_STEPw},
    {"swim", SOUND_SWIM},
    {"radar", SOUND_RADAR},
    {"build", SOUND_BUILD},
    {"alarm", SOUND_ALARM},
    {"slide", SOUND_SLIDE},
    {"exploi", SOUND_EXPLOi},
    {"insecta", SOUND_INSECTa},
    {"insectb", SOUND_INSECTb},
    {"insectw", SOUND_INSECTw},
    {"insectm", SOUND_INSECTm},
    {"tremble", SOUND_TREMBLE},
    {"pshhh", SOUND_PSHHH},
    {"nuclear", SOUND_NUCLEAR},
    {"info", SOUND_INFO},
    {"open", SOUND_OPEN},
    {"close", SOUND_CLOSE},
    {"factory", SOUND_FACTORY},
    {"egg", SOUND_EGG},
    {"motors", SOUND_MOTORs},
    {"motori", SOUND_MOTORi},
    {"shield", SOUND_SHIELD},
    {"firei", SOUND_FIREi},
    {"gundel", SOUND_GUNDEL},
    {"pshhh2", SOUND_PSHHH2},
    {"message", SOUND_MESSAGE},
    {"boumm", SOUND_BOUMm},
    {"boumv", SOUND_BOUMv},
    {"boums", SOUND_BOUMs},
    {"explol", SOUND_EXPLOl},
    {"explolp", SOUND_EXPLOlp},
    {"explop", SOUND_EXPLOp},
    {"steph", SOUND_STEPh},
    {"stepm", SOUND_STEPm},
    {"poweron", SOUND_POWERON},
    {"poweroff", SOUND_POWEROFF},
    {"aie", SOUND_AIE},
    {"waypoint", SOUND_WAYPOINT},
    {"recover", SOUND_RECOVER},
    {"deadi", SOUND_DEADi},
    {"jostle", SOUND_JOSTLE},
    {"gflat", SOUND_GFLAT},
    {"deadg", SOUND_DEADg},
    {"deadw", SOUND_DEADw},
    {"flyf", SOUND_FLYf},
    {"alarmt", SOUND_ALARMt},
    {"finding", SOUND_FINDING},
    {"thump", SOUND_THUMP},
    {"touch", SOUND_TOUCH},
    {"blitz", SOUND_BLITZ},
    {"mushroom", SOUND_MUSHROOM},
    {"firep", SOUND_FIREp},
    {"explog1", SOUND_EXPLOg1},
    {"explog2", SOUND_EXPLOg2},
    {"alarms", SOUND_ALARMs}
};

} // anonymous namespace

SoundType ParseSoundType(const std::string& str)
{
    auto it = SOUND_STRINGS.find(str);
    if (it == SOUND_STRINGS.end())
    {
        GetLogger()->Error("Could not parse sound string: %s\n", str.c_str());
        return SOUND_NONE;
    }

    return it->second;
}
