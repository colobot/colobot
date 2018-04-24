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

#include "graphics/model/model_manager.h"

#include "common/logger.h"

#include "common/resources/inputstream.h"

#include "graphics/model/model_input.h"
#include "graphics/model/model_io_exception.h"

namespace Gfx
{

CModel& CModelManager::GetModel(const std::string& modelName)
{
    auto it = m_models.find(modelName);
    if (it != m_models.end())
        return it->second;

    std::string modelFile = "models-new/" + modelName + ".txt";

    GetLogger()->Debug("Loading new model: %s\n", modelFile.c_str());

    CInputStream stream;
    stream.open(modelFile.c_str());
    if (!stream.is_open())
        throw CModelIOException(std::string("Could not open file '") + modelName + "'");

    CModel model = ModelInput::Read(stream, ModelFormat::Text);
    m_models[modelName] = model;

    return m_models[modelName];
}

void CModelManager::ClearCache()
{
    m_models.clear();
}

} // namespace Gfx
