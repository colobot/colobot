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
 * \file common/font_file.h
 * \brief Class for loading fonts from /data/fonts/fonts.ini
 */
 
#pragma once

#include "common/singleton.h"

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
    
    /** Saves fonts.ini
    * \return return true on success
    */
    bool Save();
    
    /** Reads common font from file
    * \return return path to font file
    */
    
    std::string GetCommonFont();
    
    /** Writes common font to file
    * \return return true on success
    */
    
    bool SetCommonFont(std::string filename);
    
    /** Reads common bold font from file
    * \return return path to font file
    */
    
    std::string GetCommonBoldFont();
    
    /** Writes common bold font to file
    * \return return true on success
    */
    
    bool SetCommonBoldFont(std::string filename);
    
    /** Reads common italic font from file
    * \return return path to font file
    */
    
    std::string GetCommonItalicFont();
    
    /** Writes common italic font to file
    * \return return true on success
    */
    
    bool SetCommonItalicFont(std::string filename);
    
    /** Reads studio font from file
    * \return return path to font file
    */
    
    std::string GetStudioFont();
    
    /** Writes studio font to file
    * \return return true on success
    */
    
    bool SetStudioFont(std::string filename);
    
    /** Reads studio bold font from file
    * \return returns path to font file
    */
    
    std::string GetStudioBoldFont();
    
    /** Writes studio bold font to file
    * \return return true on success
    */
    
    bool SetStudioBoldFont(std::string filename);
    
private:
    boost::property_tree::ptree m_propertyTree;
    bool m_needsSave;
    bool m_useCurrentDirectory;
    bool m_loaded;
    
    /*std::string m_colobotFont;
    std::string m_colobotFontb;
    std::string m_colobotFonti;
    
    std::string m_courierFont;
    std::string m_courierFontb;*/
};

/**
 * \enum Fonts
 * \brief enum of types of fonts used in game
 *//*
enum Fonts
{
    FONT_COLOBOT = 0,
    FONT_COLOBOT_BOLD = 1,
    FONT_COLOBOT_ITALIC = 2,
    FONT_COURIER = 3,
    FONT_COURIER_BOLD = 4
};*/

//! Global function to get config file instance
inline CFontConfigFile & GetFontConfigFile()
{
    return CFontConfigFile::GetInstance();
}