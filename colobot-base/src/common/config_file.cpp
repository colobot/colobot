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


#include "common/config_file.h"

#include "common/logger.h"

#include "common/resources/inputstream.h"
#include "common/resources/outputstream.h"

#include "common/system/system.h"

#include <memory>
#include <utility>
#include <cstring>
#include <fstream>

CConfigFile::CConfigFile() = default;

CConfigFile::~CConfigFile()
{
    if (m_needsSave)
    {
        GetLogger()->Warn("Config file was not properly saved! Saving now...");
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
            auto inputStream = std::make_unique<std::ifstream>("./colobot.json");
            good = inputStream->good();
            stream = std::move(inputStream);
        }
        else
        {
            auto inputStream = std::make_unique<CInputStream>("colobot.json");
            good = inputStream->is_open();
            stream = std::move(inputStream);
        }

        if (good)
        {
            m_properties = nlohmann::json::parse(*stream);
            m_loaded = true;
        }
        else
        {
            GetLogger()->Error("Error on parsing config file: failed to open file");
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
                auto outputStream = std::make_unique<std::ofstream>("./colobot.json");
                good = outputStream->good();
                stream = std::move(outputStream);
            }
            else
            {
                auto outputStream = std::make_unique<COutputStream>("colobot.json");
                good = outputStream->is_open();
                stream = std::move(outputStream);
            }

            if (good)
            {
                *stream << m_properties.dump(4);
                m_needsSave = false;
            }
            else
            {
                GetLogger()->Error("Error on storing config file: failed to open file");
                return false;
            }
        }
        catch (std::exception & e)
        {
            GetLogger()->Error("Error on storing config file: %%", e.what());
            return false;
        }
    }
    return true;
}

bool CConfigFile::SetStringProperty(std::string section, std::string key, std::string value)
{
    m_properties[section][key] = value;
    m_needsSave = true;
    return true;
}

bool CConfigFile::GetStringProperty(std::string section, std::string key, std::string &value)
{
    auto element = m_properties[section][key];

    if (!element.is_string()) return false;

    value = element.get<std::string>();
    return true;
}

bool CConfigFile::SetIntProperty(std::string section, std::string key, int value)
{
    m_properties[section][key] = value;
    m_needsSave = true;
    return true;
}

bool CConfigFile::GetIntProperty(std::string section, std::string key, int &value)
{
    auto element = m_properties[section][key];

    if (!element.is_number()) return false;

    value = element.get<int>();
    return true;
}

bool CConfigFile::SetBoolProperty(std::string section, std::string key, bool value)
{
    m_properties[section][key] = value;
    m_needsSave = true;
    return true;
}

bool CConfigFile::GetBoolProperty(std::string section, std::string key, bool& value)
{
    auto element = m_properties[section][key];

    if (!element.is_boolean()) return false;

    value = element.get<bool>();
    return true;
}

bool CConfigFile::SetFloatProperty(std::string section, std::string key, float value)
{
    m_properties[section][key] = value;
    m_needsSave = true;
    return true;
}

bool CConfigFile::GetFloatProperty(std::string section, std::string key, float &value)
{
    auto element = m_properties[section][key];

    if (!element.is_number()) return false;

    value = element.get<float>();
    return true;
}
