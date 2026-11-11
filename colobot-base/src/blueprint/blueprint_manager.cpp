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

#include "blueprint/blueprint_manager.h"

#include "blueprint/blueprint.h"
#include "blueprint/blueprint_all_parts.h"
#include "blueprint/blueprint_storage.h"

CBlueprintManager::CBlueprintManager()
    : m_storage(std::make_unique<CBlueprintStorage>())
{
    LoadStorageFromHardcode();
}

CBlueprintManager::~CBlueprintManager() = default;

Blueprint* CBlueprintManager::GetBlueprint(ObjectType type)
{
    return m_storage->GetBlueprint(type);
}

void CBlueprintManager::ClearStorage()
{
    m_storage->Clear();
}

void CBlueprintManager::LoadStorageFromHardcode()
{
    m_storage->LoadFromHardcode();
}
