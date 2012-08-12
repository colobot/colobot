// * This file is part of the COLOBOT source code
// * Copyright (C) 2001-2008, Daniel ROUX & EPSITEC SA, www.epsitec.ch
// *
// * This program is free software: you can redistribute it and/or modify
// * it under the terms of the GNU General Public License as published by
// * the Free Software Foundation, either version 3 of the License, or
// * (at your option) any later version.
// *
// * This program is distributed in the hope that it will be useful,
// * but WITHOUT ANY WARRANTY; without even the implied warranty of
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// * GNU General Public License for more details.
// *
// * You should have received a copy of the GNU General Public License
// * along with this program. If not, see  http://www.gnu.org/licenses/.

// profile.cpp


#include <common/profile.h>


template<> CProfile* CSingleton<CProfile>::mInstance = nullptr;


CProfile::CProfile()
{
    m_ini = new CSimpleIniA();
    m_ini->SetUnicode();
    m_ini->SetMultiKey();
}


CProfile::~CProfile()
{
    m_ini->Reset();
    delete m_ini;
}


bool CProfile::InitCurrentDirectory()
{
    bool result = m_ini->LoadFile("colobot.ini") == SI_OK;
    return result;
}


bool CProfile::SetLocalProfileString(std::string section, std::string key, std::string value)
{
    return (m_ini->SetValue(section.c_str(), key.c_str(), value.c_str()) == SI_OK);
}


bool CProfile::GetLocalProfileString(std::string section, std::string key, std::string &buffer)
{
    const char* value = m_ini->GetValue(section.c_str(), key.c_str(), nullptr);
    if (strlen(value) > 0) {
        buffer = std::string(value);
        return true;
    }

    return false;
}


bool CProfile::SetLocalProfileInt(std::string section, std::string key, int value)
{
    return (m_ini->SetLongValue(section.c_str(), key.c_str(), value) == SI_OK);
}


bool CProfile::GetLocalProfileInt(std::string section, std::string key, int &value)
{
    value = m_ini->GetLongValue(section.c_str(), key.c_str(), 0L);
    return true;
}


bool CProfile::SetLocalProfileFloat(std::string section, std::string key, float value)
{
    return (m_ini->SetDoubleValue(section.c_str(), key.c_str(), value) == SI_OK);
}


bool CProfile::GetLocalProfileFloat(std::string section, std::string key, float &value)
{
    value = m_ini->GetDoubleValue(section.c_str(), key.c_str(), 0.0d);
    return true;
}


std::vector< std::string > CProfile::GetLocalProfileSection(std::string section, std::string key)
{
    std::vector< std::string > ret_list;

    CSimpleIniA::TNamesDepend values;
    m_ini->GetAllValues(section.c_str(), key.c_str(), values);
    values.sort(CSimpleIniA::Entry::LoadOrder());

    for (auto item : values) {
        ret_list.push_back(item.pItem);
    }

    return ret_list;
}
