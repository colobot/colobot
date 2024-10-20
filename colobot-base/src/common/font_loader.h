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

 /**
 * \file common/font_loader.h
 * \brief Class for loading fonts from /data/fonts/fonts.ini
 */

#pragma once

#include "common/singleton.h"

#include "graphics/engine/text.h"

#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>

/**
* \class CFontLoader
*
* \brief Class for loading config file
*
*/

class CFontLoader
{
public:
    CFontLoader();
    virtual ~CFontLoader();

    /** Loads fonts.ini
    * \return return true on success
    */
    bool Init();

    /** Reads given font path from file
    * \return return path to font file if font type is configured
    */
    std::optional<std::filesystem::path> GetFont(Gfx::FontType type) const;

private:
    std::unordered_map<std::string, std::filesystem::path> m_fonts;
};
