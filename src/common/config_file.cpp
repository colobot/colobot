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


#include "common/config_file.h"

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

CConfigFile::CConfigFile()
   : m_needsSave(false)
   , m_useCurrentDirectory(false)
   , m_loaded(false)
{
}


CConfigFile::~CConfigFile()
{
    if (m_needsSave)
    {
        GetLogger()->Warn("Config file was not properly saved! Saving now...\n");
        Save();
    }
}

void CConfigFile::SetUseCurrentDirectory(bool useCurrentDirectory)
{
    m_useCurrentDirectory = useCurrentDirectory;
}

bool CConfigFile::Init()
{
    try
    {
        std::unique_ptr<std::istream> stream;
        bool good;
        if (m_useCurrentDirectory)
        {
            auto inputStream = MakeUnique<std::ifstream>("./colobot.ini");
            good = inputStream->good();
            stream = std::move(inputStream);
        }
        else
        {
            auto inputStream = MakeUnique<CInputStream>("colobot.ini");
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
            GetLogger()->Error("Error on parsing config file: failed to open file\n");
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

bool CConfigFile::Save()
{
    if (m_needsSave)
    {
        try
        {
            std::unique_ptr<std::ostream> stream;
            bool good;
            if (m_useCurrentDirectory)
            {
                auto outputStream = MakeUnique<std::ofstream>("./colobot.ini");
                good = outputStream->good();
                stream = std::move(outputStream);
            }
            else
            {
                auto outputStream = MakeUnique<COutputStream>("colobot.ini");
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
                GetLogger()->Error("Error on storing config file: failed to open file\n");
                return false;
            }
        }
        catch (std::exception & e)
        {
            GetLogger()->Error("Error on storing config file: %s\n", e.what());
            return false;
        }
    }
    return true;
}

bool CConfigFile::SetStringProperty(std::string section, std::string key, std::string value)
{
    try
    {
        m_propertyTree.put(section + "." + key, value);
        m_needsSave = true;
    }
    catch (std::exception & e)
    {
        GetLogger()->Error("Error on editing config file: %s\n", e.what());
        return false;
    }
    return true;
}

bool CConfigFile::GetStringProperty(std::string section, std::string key, std::string &value)
{
    try
    {
        std::string readValue = m_propertyTree.get<std::string>(section + "." + key);
        value = std::move(readValue);
    }
    catch (std::exception & e)
    {
        GetLogger()->Log(m_loaded ? LOG_INFO : LOG_TRACE, "Error on parsing config file: %s\n", e.what());
        return false;
    }
    return true;
}

bool CConfigFile::SetIntProperty(std::string section, std::string key, int value)
{
    try
    {
        m_propertyTree.put(section + "." + key, value);
        m_needsSave = true;
    }
    catch (std::exception & e)
    {
        GetLogger()->Error("Error on editing config file: %s\n", e.what());
        return false;
    }
    return true;
}

bool CConfigFile::GetIntProperty(std::string section, std::string key, int &value)
{
    try
    {
        int readValue = m_propertyTree.get<int>(section + "." + key);
        value = readValue;
    }
    catch (std::exception & e)
    {
        GetLogger()->Log(m_loaded ? LOG_INFO : LOG_TRACE, "Error on parsing config file: %s\n", e.what());
        return false;
    }
    return true;
}

bool CConfigFile::SetBoolProperty(std::string section, std::string key, bool value)
{
    return SetIntProperty(section, key, value ? 1 : 0);
}

bool CConfigFile::GetBoolProperty(std::string section, std::string key, bool& value)
{
    int intValue = 0;
    bool result = GetIntProperty(section, key, intValue);
    if (result)
    {
        if (intValue == 0)
        {
            value = false;
        }
        else if (intValue == 1)
        {
            value = true;
        }
        else
        {
            GetLogger()->Log(m_loaded ? LOG_INFO : LOG_TRACE, "Error on parsing bool property %s.%s (expected 0 or 1, not %d)\n",
                             section.c_str(), key.c_str(), intValue);
            return false;
        }
    }
    return result;
}

bool CConfigFile::SetFloatProperty(std::string section, std::string key, float value)
{
    try
    {
        m_propertyTree.put(section + "." + key, value);
        m_needsSave = true;
    }
    catch (std::exception & e)
    {
        GetLogger()->Error("Error on editing config file: %s\n", e.what());
        return false;
    }
    return true;
}

bool CConfigFile::GetFloatProperty(std::string section, std::string key, float &value)
{
    try
    {
        float readValue = m_propertyTree.get<float>(section + "." + key);
        value = readValue;
    }
    catch (std::exception & e)
    {
        GetLogger()->Log(m_loaded ? LOG_INFO : LOG_TRACE, "Error on parsing config file: %s\n", e.what());
        return false;
    }
    return true;
}
