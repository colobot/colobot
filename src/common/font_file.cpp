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
 
#include "common/font_file.h"

#include "common/logger.h"
#include "common/make_unique.h"

#include "common/resources/inputstream.h"
#include "common/resources/outputstream.h"

#include "common/system/system.h"

#include <memory>
#include <utility>
#include <cstring>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/regex.hpp>

namespace bp = boost::property_tree;

CFontConfigFile::CFontConfigFile()
   : m_needsSave(false)
   , m_useCurrentDirectory(false)
   , m_loaded(false)
{
}

CFontConfigFile::~CFontConfigFile()
{
    if (m_needsSave)
    {
        GetLogger()->Warn("Font config file was not properly saved! Saving now...\n");
        Save();
    }
}

bool CFontConfigFile::Init()
{
    try
    {
        std::unique_ptr<std::istream> stream;
        bool good;
        if (m_useCurrentDirectory)
        {
            auto inputStream = MakeUnique<std::ifstream>("./fonts.ini");
            good = inputStream->good();
            stream = std::move(inputStream);
        }
        else
        {
            auto inputStream = MakeUnique<CInputStream>("fonts.ini");
            good = inputStream->is_open();
            stream = std::move(inputStream);
        }
        
        if (good)
        {
            bp::ini_parser::read_ini(*stream, m_propertyTree);
            m_loaded = true;
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

bool CFontConfigFile::Save()
{
    if (m_needsSave)
    {
        try
        {
            std::unique_ptr<std::ostream> stream;
            bool good;
            if (m_useCurrentDirectory)
            {
                auto outputStream = MakeUnique<std::ofstream>("./fonts.ini");
                good = outputStream->good();
                stream = std::move(outputStream);
            }
            else
            {
                auto outputStream = MakeUnique<COutputStream>("fonts.ini");
                good = outputStream->is_open();
                stream = std::move(outputStream);
            }

            if (good)
            {
                bp::ini_parser::write_ini(*stream, m_propertyTree);
                m_needsSave = false;
            }
            else
            {
                GetLogger()->Error("Error on storing fonts config file: failed to open file\n");
                return false;
            }
        }
        catch (std::exception & e)
        {
            GetLogger()->Error("Error on storing fonts config file: %s\n", e.what());
            return false;
        }
    }
    return true;
}

std::string CFontConfigFile::GetCommonFont()
{
    try
    {
        std::string path = std::string("/fonts/") + m_propertyTree.get<std::string>("FONT_COMMON");
        return path;
    }
    catch (std::exception & e)
    {
        GetLogger()->Log(m_loaded ? LOG_INFO : LOG_TRACE, "Error on parsing config file: %s. Default font will be used instead.\n", e.what());
        SetCommonFont("dvu_sans.ttf");
        return "/fonts/dvu_sans.ttf";
    }
    return "";
}

bool CFontConfigFile::SetCommonFont(std::string filename)
{
    try
    {
        m_propertyTree.put("FONT_COMMON", filename);
        m_needsSave = true;
    }
    catch (std::exception & e)
    {
        GetLogger()->Error("Error on editing config file: %s\n", e.what());
        return false;
    }
    return true;
}

std::string CFontConfigFile::GetCommonBoldFont()
{
    try
    {
        std::string path = std::string("/fonts/") + m_propertyTree.get<std::string>("FONT_COMMON_BOLD");
        return path;
    }
    catch (std::exception & e)
    {
        GetLogger()->Log(m_loaded ? LOG_INFO : LOG_TRACE, "Error on parsing config file: %s. Default font will be used instead.\n", e.what());
        SetCommonBoldFont("dvu_sans_bold.ttf");
        return "/fonts/dvu_sans_bold.ttf";
    }
    return "";
}

bool CFontConfigFile::SetCommonBoldFont(std::string filename)
{
    try
    {
        m_propertyTree.put("FONT_COMMON_BOLD", filename);
        m_needsSave = true;
    }
    catch (std::exception & e)
    {
        GetLogger()->Error("Error on editing config file: %s\n", e.what());
        return false;
    }
    return true;
}

std::string CFontConfigFile::GetCommonItalicFont()
{
    try
    {
        std::string path = std::string("/fonts/") + m_propertyTree.get<std::string>("FONT_COMMON_ITALIC");
        return path;
    }
    catch (std::exception & e)
    {
        GetLogger()->Log(m_loaded ? LOG_INFO : LOG_TRACE, "Error on parsing config file: %s. Default font will be used instead.\n", e.what());
        SetCommonItalicFont("dvu_sans_italic.ttf");
        return "/fonts/dvu_sans_italic.ttf";
    }
    return "";
}

bool CFontConfigFile::SetCommonItalicFont(std::string filename)
{
    try
    {
        m_propertyTree.put("FONT_COMMON_ITALIC", filename);
        m_needsSave = true;
    }
    catch (std::exception & e)
    {
        GetLogger()->Error("Error on editing config file: %s\n", e.what());
        return false;
    }
    return true;
}

std::string CFontConfigFile::GetStudioFont()
{
    try
    {
        std::string path = std::string("/fonts/") + m_propertyTree.get<std::string>("FONT_STUDIO");
        return path;
    }
    catch (std::exception & e)
    {
        GetLogger()->Log(m_loaded ? LOG_INFO : LOG_TRACE, "Error on parsing config file: %s. Default font will be used instead.\n", e.what());
        SetStudioFont("dvu_sans_mono.ttf");
        return "/fonts/dvu_sans_mono.ttf";
    }
    return "";
}

bool CFontConfigFile::SetStudioFont(std::string filename)
{
    try
    {
        m_propertyTree.put("FONT_STUDIO", filename);
        m_needsSave = true;
    }
    catch (std::exception & e)
    {
        GetLogger()->Error("Error on editing config file: %s\n", e.what());
        return false;
    }
    return true;
}

std::string CFontConfigFile::GetStudioBoldFont()
{
    try
    {
        std::string path = std::string("/fonts/") + m_propertyTree.get<std::string>("FONT_STUDIO_BOLD");
        return path;
    }
    catch (std::exception & e)
    {
        GetLogger()->Log(m_loaded ? LOG_INFO : LOG_TRACE, "Error on parsing config file: %s. Default font will be used instead.\n", e.what());
        SetStudioBoldFont("dvu_sans_mono_bold.ttf");
        return "/fonts/dvu_sans_mono_bold.ttf";
    }
    return "";
}

bool CFontConfigFile::SetStudioBoldFont(std::string filename)
{
    try
    {
        m_propertyTree.put("FONT_STUDIO_BOLD", filename);
        m_needsSave = true;
    }
    catch (std::exception & e)
    {
        GetLogger()->Error("Error on editing config file: %s\n", e.what());
        return false;
    }
    return true;
}
