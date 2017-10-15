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
* \class CFontConfigFile
*
* \brief Class for loading config file
*
*/

class CFontConfigFile : public CSingleton<CFontConfigFile>
{
public:
    CFontConfigFile();
    virtual ~CFontConfigFile();
    
    /** Loads fonts.ini
    * \return return true on success
    */
    bool Init();

    /** Reads given font from file
    * \return return path to font file
    */
    std::string GetFont(Gfx::FontType type);
    
private:
    boost::property_tree::ptree m_propertyTree;
    bool m_needsSave;
    bool m_loaded;
};

//! Global function to get config file instance
inline CFontConfigFile & GetFontConfigFile()
{
    return CFontConfigFile::GetInstance();
}