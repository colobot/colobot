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

#include "common/font_loader.h"

#include "common/logger.h"
#include "common/stringutils.h"

#include "common/resources/inputstream.h"
#include "common/resources/outputstream.h"

#include "common/system/system.h"

#include "graphics/engine/text.h"

#include <map>
#include <memory>
#include <utility>
#include <cstring>


CFontLoader::CFontLoader()
{
}

CFontLoader::~CFontLoader()
{
}

bool CFontLoader::Init()
{
    try
    {
        std::unique_ptr<std::istream> stream;
        auto inputStream = std::make_unique<CInputStream>("fonts/fonts.ini");
        bool good = inputStream->is_open();
        stream = std::move(inputStream);

        if (good)
        {
            std::string line;

            while (std::getline(*stream, line))
            {
                auto parts = StrUtils::Split(line, " =");

                m_fonts[parts[0]] = parts[1];
            }

            GetLogger()->Debug("Fonts config file loaded correctly.");
        }
        else
        {
            return false;
        }
    }
    catch (std::exception & e)
    {
        GetLogger()->Error("Error on parsing config file: %%", e.what());
        return false;
    }
    return true;
}

std::optional<std::filesystem::path> CFontLoader::GetFont(Gfx::FontType type) const
{
    auto iterator = m_fonts.find(ToString(type));

    if (iterator == m_fonts.end())
        return std::nullopt;
    else
        return "fonts" / TempToPath(iterator->second);
}
