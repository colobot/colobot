/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2016, Daniel Roux, EPSITEC SA & TerranovaTeam
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
 * \file common/font_config.h
 * \brief Class for loading fonts from /data/fonts/fonts.ini
 */
 
#pragma once

#include "common/singleton.h"

#include "graphics/engine/text.h"

#include <boost/property_tree/ptree.hpp>

#include <string>

/**
* \class CFontConfig
*
* \brief Class for loading config file
*
*/

const std::map<Gfx::FontType, std::string> defaultFont = 
{
    { Gfx::FONT_COMMON, "dvu_sans.ttf" },
    { Gfx::FONT_COMMON_BOLD, "dvu_sans_bold.ttf" },
    { Gfx::FONT_COMMON_ITALIC, "dvu_sans_italic.ttf" },
    { Gfx::FONT_STUDIO, "dvu_sans_mono.ttf" },
    { Gfx::FONT_STUDIO_BOLD, "dvu_sans_mono_bold.ttf" },
    { Gfx::FONT_STUDIO_ITALIC, "dvu_sans_mono_italic.ttf" },
    { Gfx::FONT_SATCOM, "dvu_sans.ttf" },
    { Gfx::FONT_SATCOM_BOLD, "dvu_sans_bold.ttf" },
    { Gfx::FONT_SATCOM_ITALIC, "dvu_sans_italic.ttf" },
};

const std::map<Gfx::FontType, std::string> fontType = 
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

class CFontConfig
{
public:
    CFontConfig();
    virtual ~CFontConfig();
    
    /** Loads fonts.ini
    * \return return true on success
    */
    bool Init();

    /** Reads given font from file
    * \return return path to font file
    */
    std::string GetFont(Gfx::FontType type);
    
    /** Const type method to read filenames of fonts from defaultFont map
    * used as a fallback if it wasn't possible to read font from fonts.ini
    * \return return filename of default path
    */
    std::string GetDefaultFont(Gfx::FontType type) const;
    
    /** Const type method converting Gfx::FontType to string
    * \return return id of font used in fonts.ini file
    */
    
    std::string GetFontType(Gfx::FontType type) const;
    
private:
    boost::property_tree::ptree m_propertyTree;
    bool m_needsSave;
    bool m_loaded;
};
