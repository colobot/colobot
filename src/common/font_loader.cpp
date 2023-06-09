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

#include "common/font_loader.h"
#include "common/simpleini_wrapper.h"

#include "common/logger.h"
#include "common/make_unique.h"

#include "common/resources/inputstream.h"
#include "common/resources/outputstream.h"

#include "common/system/system.h"

#include "graphics/engine/text.h"

#include <map>
#include <memory>
#include <utility>
#include <cstring>

CFontLoader::CFontLoader()
   : m_reader(std::make_unique<si::SimpleIni>())
{
}

CFontLoader::~CFontLoader()
{
}

bool CFontLoader::Init()
{
    auto inputStream = std::make_unique<CInputStream>("/fonts/fonts.ini");

    if (!inputStream->is_open())
    {
        return false;
    }

    m_reader->SetUnicode();

    auto err = m_reader->LoadData(*inputStream);

    if (err < 0) {
        GetLogger()->Error("Error on parsing config file: %d\n", err);
        return false;
    }

    GetLogger()->Debug("Fonts config file loaded correctly. \n");

    return true;
}

std::optional<std::string> CFontLoader::GetFont(Gfx::FontType type) const
{
    auto strType = ToString(type);
    std::string font = m_reader->GetValue("", strType.c_str(), "");

    if (!font.empty())
        return std::string("/fonts/") + font;

    return std::nullopt;
}
