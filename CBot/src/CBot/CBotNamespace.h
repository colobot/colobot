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

#pragma once

#include <memory>
#include <string>
#include <set>

namespace CBot
{

class CBotProgram;
class CBotVar;
class CBotFunction;
class CBotClass;

class CBotNamespace
{
public:
    std::unique_ptr<CBotProgram> AddProgram(CBotVar* thisVar);

private:
    const std::set<CBotFunction*>& GetPublicFunctions() const;
    void AddPublic(CBotFunction* func);
    void RemovePublic(CBotFunction* func);

    CBotClass* FindPublicClass(const std::string& name);

    friend class CBotProgram;

    std::set<CBotFunction*> m_publicFunctions;
    std::set<CBotClass*> m_publicClasses;
};

} // namespace CBot
