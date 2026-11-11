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

#include "blueprint/blueprint_storage.h"

#include "blueprint/blueprint.h"
#include "blueprint/blueprint_all_parts.h"
#include "blueprint/blueprint_hardcode.h"
#include "blueprint/blueprint_parser.h"

#include "common/stringutils.h"

#include "level/parser/parser.h"

#define BLUEPRINT_LOAD_HARDCODE(FIELD, TYPE) MakeUniqueBlueprintHardcode<TYPE>(type, m_objects[type].FIELD);
#define BLUEPRINT_PARSE_LINE(FIELD, TYPE) if (ParseBlueprintFromLine(line.get(), m_objects[type].FIELD.get())) { continue; }
#define BLUEPRINT_WRITE_LINES(FIELD, TYPE) WriteBlueprintToLines(&parser, type, m_objects[type].FIELD.get());

void CBlueprintStorage::Clear()
{
    m_objects.clear();
}

template<typename T>
void MakeUniqueBlueprintHardcode(ObjectType type, std::unique_ptr<T>& target)
{
    target = std::make_unique<T>(GetBlueprintHardcode<T>(type));
}

Blueprint* CBlueprintStorage::GetBlueprint(ObjectType type)
{
    return &m_objects[type];
}

void CBlueprintStorage::LoadFromHardcode()
{
    for ( int i = 1 ; i < OBJECT_MAX ; i++ )
    {
        ObjectType type = static_cast<ObjectType>(i);

        FOR_EACH_PART_OF_BLUEPRINT(BLUEPRINT_LOAD_HARDCODE)
    }
}

void CBlueprintStorage::LoadFromFile(const char* path)
{
    CLevelParser parser(path);
    if (!parser.Exists())
    {
        throw CLevelParserException("Failed to open file: " + StrUtils::ToString(path));
    }

    parser.Load();
    for (auto& line : parser.GetLines())
    {
        if ( line->GetParam("type")->IsDefined() )
        {
            ObjectType type = line->GetParam("type")->AsObjectType();
            FOR_EACH_PART_OF_BLUEPRINT(BLUEPRINT_PARSE_LINE)
        }
        throw UnusedCmdException(line.get());
    }
}

void CBlueprintStorage::SaveToFile(const char* path)
{
    CLevelParser parser(path);
    for ( int i = 0 ; i < OBJECT_MAX ; i++ )
    {
        ObjectType type = static_cast<ObjectType>(i);
        FOR_EACH_PART_OF_BLUEPRINT(BLUEPRINT_WRITE_LINES)
    }
    parser.Save();
}
