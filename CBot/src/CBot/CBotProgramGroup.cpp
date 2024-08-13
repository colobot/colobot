/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2024 TerranovaTeam
 * http://colobot.info; http://github.com/colobot
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

#include "CBot/CBotProgramGroup.h"
#include "CBot/CBotProgram.h"

namespace CBot
{

std::unique_ptr<CBotProgram> CBotProgramGroup::AddProgram(CBotVar* thisVar)
{
    return std::unique_ptr<CBotProgram>(new CBotProgram(thisVar, *this));
}

const std::set<CBotFunction*>& CBotProgramGroup::GetPublicFunctions() const
{
    return m_publicFunctions;
}

void CBotProgramGroup::AddPublic(CBotFunction* func)
{
    m_publicFunctions.insert(func);
}

void CBotProgramGroup::RemovePublic(CBotFunction* func)
{
    m_publicFunctions.erase(func);
}

} // namespace CBot

