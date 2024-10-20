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

#include "graphics/model/model.h"

#include <memory>
#include <string>
#include <map>
#include <filesystem>

namespace Gfx
{

/**
 * \class CModelManager
 * \brief Manager for models read from model files
 */
class CModelManager
{
public:
    //! Returns a model named \a modelName
    /** @throws CModelIOException on read error */
    CModel* GetModel(const std::filesystem::path& modelName);

    //! Clears cached models
    void ClearCache();

private:
    std::map<std::filesystem::path, std::unique_ptr<CModel>> m_models;
};

} // namespace Gfx
