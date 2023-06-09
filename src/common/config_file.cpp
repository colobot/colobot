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

#include "common/config_file.h"
#include "common/simpleini_wrapper.h"

#include "common/logger.h"
#include "common/make_unique.h"

#include "common/resources/inputstream.h"
#include "common/resources/outputstream.h"

#include "common/system/system.h"

#include <utility>
#include <cstring>
#include <fstream>

CConfigFile::CConfigFile()
   : m_reader(std::make_unique<si::SimpleIni>())
   , m_needsSave(false)
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
    std::unique_ptr<std::istream> stream;
    bool good;

    if (m_useCurrentDirectory)
    {
        stream = std::make_unique<std::ifstream>("./colobot.ini");
        good = stream->good();
    }
    else
    {
        auto inputStream = std::make_unique<CInputStream>("colobot.ini");
        good = inputStream->is_open();
        stream = std::move(inputStream);
    }

    if (!good)
    {
        GetLogger()->Error("Error on parsing config file: failed to open file\n");
        return false;
    }

    m_reader->SetUnicode();
    m_reader->SetSpaces();

    auto err = m_reader->LoadData(*stream);

    if (err < 0) {
        GetLogger()->Error("Error on parsing config file: %d\n", err);
        return false;
    }

    m_loaded = true;

    return true;
}

bool CConfigFile::Save()
{
    if (!m_needsSave) {
        return true;
    }

    std::unique_ptr<std::ostream> stream;
    bool good;

    if (m_useCurrentDirectory)
    {
        auto stream = std::make_unique<std::ofstream>("./colobot.ini");
        good = stream->good();
    }
    else
    {
        auto outputStream = std::make_unique<COutputStream>("colobot.ini");
        good = outputStream->is_open();
        stream = std::move(outputStream);
    }

    if (!good)
    {
        GetLogger()->Error("Error on storing config file: failed to open file\n");
        return false;
    }

    auto err = m_reader->Save(*stream);

    if (err < 0) {
        GetLogger()->Error("Error on storing config file: %d\n", err);
        return false;
    }

    m_needsSave = false;

    return true;
}

bool CConfigFile::SetStringProperty(std::string section, std::string key, std::string value)
{
    auto err = m_reader->SetValue(section.c_str(), key.c_str(), value.c_str());

    if (err < 0) {
        GetLogger()->Error("Error on editing config file: %d\n", err);
        return false;
    }

    m_needsSave = true;

    return true;
}

bool CConfigFile::GetStringProperty(std::string section, std::string key, std::string &value)
{
    auto readValue = m_reader->GetValue(section.c_str(), key.c_str(), nullptr);

    if (!readValue) {
        GetLogger()->Log(m_loaded ? LOG_INFO : LOG_TRACE, "Error on parsing config file: %s - No value found\n", key.c_str());
        return false;
    }

    value = readValue;

    return true;
}

bool CConfigFile::SetIntProperty(std::string section, std::string key, int value)
{
    auto err = m_reader->SetLongValue(section.c_str(), key.c_str(), value);

    if (err < 0) {
        GetLogger()->Error("Error on editing config file: %d\n", err);
        return false;
    }

    m_needsSave = true;

    return true;
}

bool CConfigFile::GetIntProperty(std::string section, std::string key, int &value)
{
    if (!m_reader->KeyExists(section.c_str(), key.c_str())) {
        GetLogger()->Log(m_loaded ? LOG_INFO : LOG_TRACE, "Error on parsing config file: %s - No value found\n", key.c_str());
        return false;
    }

    value = m_reader->GetLongValue(section.c_str(), key.c_str());

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
    auto err = m_reader->SetDoubleValue(section.c_str(), key.c_str(), value);

    if (err < 0) {
        GetLogger()->Error("Error on editing config file: %d\n", err);
        return false;
    }

    m_needsSave = true;

    return true;
}

bool CConfigFile::GetFloatProperty(std::string section, std::string key, float &value)
{
    if (!m_reader->KeyExists(section.c_str(), key.c_str())) {
        GetLogger()->Log(m_loaded ? LOG_INFO : LOG_TRACE, "Error on parsing config file: %s - No value found\n", key.c_str());
        return false;
    }

    value = m_reader->GetDoubleValue(section.c_str(), key.c_str());

    return true;
}
