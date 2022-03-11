/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2021, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include "graphics/model/model_mod.h"
#include "graphics/model/model_txt.h"

#include "graphics/model/model_io_exception.h"

namespace Gfx
{

CModel ModelInput::Read(std::istream &stream, ModelFormat format)
{
    stream.exceptions(std::ios_base::failbit | std::ios_base::badbit);

    CModel model;

    try
    {
        switch (format)
        {
            case ModelFormat::Text:
                ModelIO::ReadTextModel(model, stream);
                break;

            case ModelFormat::Old:
                ModelIO::ReadOldModel(model, stream);
                break;
        }
    }
    catch (const CModelIOException& e)
    {
        throw;
    }
    catch (const std::exception& e)
    {
        throw CModelIOException(std::string("Other error while reading model data: ") + e.what());
    }

    return model;
}



} // namespace Gfx
