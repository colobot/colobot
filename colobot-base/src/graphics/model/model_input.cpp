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

#include "graphics/model/model_input.h"

#include "graphics/model/model_gltf.h"
#include "graphics/model/model_mod.h"
#include "graphics/model/model_txt.h"

#include "graphics/model/model_io_exception.h"

#include "common/stringutils.h"

namespace Gfx
{

std::unique_ptr<CModel> ModelInput::Read(const std::filesystem::path& path)
{
    auto extension = path.extension();

    if (extension == ".mod")
    {
        return ModelIO::ReadOldModel(path);
    }
    else if (extension == ".txt")
    {
        return ModelIO::ReadTextModel(path);
    }
    else if (extension == ".gltf")
    {
        return ModelIO::ReadGLTFModel(path);
    }
    else
    {
        throw CModelIOException(std::string("Unknown model format: ") + StrUtils::ToString(extension));
    }
}

} // namespace Gfx
