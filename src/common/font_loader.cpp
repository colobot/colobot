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

#include "common/font_loader.h"

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
#include <boost/property_tree/ini_parser.hpp>
#include <boost/regex.hpp>

namespace bp = boost::property_tree;

const std::map<Gfx::FontType, std::string> DEFAULT_FONT =
{
    { Gfx::FONT_COMMON, "dvu_sans.ttf" },
    { Gfx::FONT_COMMON_BOLD, "dvu_sans_bold.ttf" },
    { Gfx::FONT_COMMON_ITALIC, "dvu_sans_italic.ttf" },
    { Gfx::FONT_STUDIO, "dvu_sans_mono.ttf" },
    { Gfx::FONT_STUDIO_BOLD, "dvu_sans_mono_bold.ttf" },
    { Gfx::FONT_STUDIO_ITALIC, "dvu_sans_mono.ttf" }, //placeholder for future use, DejaVu Sans Mono doesn't have italic variant
    { Gfx::FONT_SATCOM, "dvu_sans.ttf" },
    { Gfx::FONT_SATCOM_BOLD, "dvu_sans_bold.ttf" },
    { Gfx::FONT_SATCOM_ITALIC, "dvu_sans_italic.ttf" },
};

const std::map<Gfx::FontType, std::string> FONT_TYPE =
{
    { Gfx::FONT_COMMON, "FontCommon" },
    { Gfx::FONT_COMMON_BOLD, "FontCommonBold" },
    { Gfx::FONT_COMMON_ITALIC, "FontCommonItalic" },
    { Gfx::FONT_STUDIO, "FontStudio" },
    { Gfx::FONT_STUDIO_BOLD, "FontStudioBold" },
    { Gfx::FONT_STUDIO_ITALIC, "FontStudioItalic" },
    { Gfx::FONT_SATCOM, "FontSatCom" },
    { Gfx::FONT_SATCOM_BOLD, "FontSatComBold" },
    { Gfx::FONT_SATCOM_ITALIC, "FontSatComItalic" },
};

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
        auto inputStream = MakeUnique<CInputStream>("/fonts/fonts.ini");
        bool good = inputStream->is_open();
        stream = std::move(inputStream);

        if (good)
        {
            bp::ini_parser::read_ini(*stream, m_propertyTree);
            GetLogger()->Debug("Fonts config file loaded correctly. \n");
        }
        else
        {
            return false;
        }
    }
    catch (std::exception & e)
    {
        GetLogger()->Error("Error on parsing config file: %s\n", e.what());
        return false;
    }
    return true;
}

std::string CFontLoader::GetFont(Gfx::FontType type)
{
    return std::string("/fonts/") + m_propertyTree.get<std::string>(GetFontType(type), GetDefaultFont(type));
}

std::string CFontLoader::GetDefaultFont(Gfx::FontType type) const
{
    return DEFAULT_FONT.at(type);
}

std::string CFontLoader::GetFontType(Gfx::FontType type) const
{
    return FONT_TYPE.at(type);
}
