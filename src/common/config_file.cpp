/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2014, Daniel Roux, EPSITEC SA & TerranovaTeam
 * http://epsiteс.ch; http://colobot.info; http://github.com/colobot
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

#include "app/system.h"

#include "common/logger.h"

#include "common/resources/inputstream.h"
#include "common/resources/outputstream.h"

#include <memory>
#include <utility>
#include <cstring>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/regex.hpp>


template<> CConfigFile* CSingleton<CConfigFile>::m_instance = nullptr;

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
            std::ifstream* inputStream = new std::ifstream("./colobot.ini");
            stream = std::unique_ptr<std::istream>(inputStream);
            good = inputStream->good();
        }
        else
        {
            CInputStream* inputStream = new CInputStream("colobot.ini");
            stream = std::unique_ptr<std::istream>(inputStream);
            good = inputStream->is_open();
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
                std::ofstream* outputStream = new std::ofstream("./colobot.ini");
                stream = std::unique_ptr<std::ostream>(outputStream);
                good = outputStream->good();
            }
            else
            {
                COutputStream* outputStream = new COutputStream("colobot.ini");
                stream = std::unique_ptr<std::ostream>(outputStream);
                good = outputStream->is_open();
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


bool CConfigFile::GetStringProperty(std::string section, std::string key, std::string &buffer)
{
    try
    {
        buffer = m_propertyTree.get<std::string>(section + "." + key);
    }
    catch (std::exception & e)
    {
        if (m_loaded)
        {
            GetLogger()->Info("Error on parsing config file: %s\n", e.what());
        }
        else
        {
            GetLogger()->Trace("Error on parsing config file: %s\n", e.what());
        }
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
        value = m_propertyTree.get<int>(section + "." + key);
    }
    catch (std::exception & e)
    {
        if (m_loaded)
        {
            GetLogger()->Info("Error on parsing config file: %s\n", e.what());
        }
        else
        {
            GetLogger()->Trace("Error on parsing config file: %s\n", e.what());
        }
        return false;
    }
    return true;
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
        value = m_propertyTree.get<float>(section + "." + key);
    }
    catch (std::exception & e)
    {
        if (m_loaded)
        {
            GetLogger()->Info("Error on parsing config file: %s\n", e.what());
        }
        else
        {
            GetLogger()->Trace("Error on parsing config file: %s\n", e.what());
        }
        return false;
    }
    return true;
}
