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


#include "common/profile.h"
#include "common/logger.h"

#include <utility>
#include <cstring>
#include <boost/property_tree/ini_parser.hpp>


template<> CProfile* CSingleton<CProfile>::mInstance = nullptr;

namespace bp = boost::property_tree;

CProfile::CProfile()
{
    // m_ini = new CSimpleIniA();
    // m_ini->SetUnicode();
    // m_ini->SetMultiKey();
}


CProfile::~CProfile()
{
    // m_ini->Reset();
    // delete m_ini;
}


bool CProfile::InitCurrentDirectory()
{
    try
    {
        bp::ini_parser::read_ini("colobot.ini", m_propertyTree);
    }
    catch (std::exception & e)
    {
        GetLogger()->Info("Error on parsing profile: %s\n", e.what());
    }
    // return result;
    return true;
}


bool CProfile::SetLocalProfileString(std::string section, std::string key, std::string value)
{
    try
    {
        m_propertyTree.put(section + "." + key, value);
    }
    catch (std::exception & e)
    {
        GetLogger()->Info("Error on parsing profile: %s\n", e.what());
    }
    // return (m_ini->SetValue(section.c_str(), key.c_str(), value.c_str()) == SI_OK);
    return true;
}


bool CProfile::GetLocalProfileString(std::string section, std::string key, std::string &buffer)
{
    try
    {
        buffer = m_propertyTree.get<std::string>(section + "." + key);
    }
    catch (std::exception & e)
    {
        GetLogger()->Info("Error on parsing profile: %s\n", e.what());
    }
    return true;
}


bool CProfile::SetLocalProfileInt(std::string section, std::string key, int value)
{
    try
    {
        m_propertyTree.put(section + "." + key, value);
    }
    catch (std::exception & e)
    {
        GetLogger()->Info("Error on parsing profile: %s\n", e.what());
    }
    // return (m_ini->SetLongValue(section.c_str(), key.c_str(), value) == SI_OK);
    return true;
}


bool CProfile::GetLocalProfileInt(std::string section, std::string key, int &value)
{
    try
    {
        value = m_propertyTree.get<int>(section + "." + key);
    }
    catch (std::exception & e)
    {
        GetLogger()->Info("Error on parsing profile: %s\n", e.what());
    }
    // value = m_ini->GetLongValue(section.c_str(), key.c_str(), 0L);
    return true;
}


bool CProfile::SetLocalProfileFloat(std::string section, std::string key, float value)
{
    try
    {
        m_propertyTree.put(section + "." + key, value);
    }
    catch (std::exception & e)
    {
        GetLogger()->Info("Error on parsing profile: %s\n", e.what());
    }
    // return (m_ini->SetDoubleValue(section.c_str(), key.c_str(), value) == SI_OK);
    return true;
}


bool CProfile::GetLocalProfileFloat(std::string section, std::string key, float &value)
{
    try
    {
        value = m_propertyTree.get<float>(section + "." + key);
    }
    catch (std::exception & e)
    {
        GetLogger()->Info("Error on parsing profile: %s\n", e.what());
    }
    // value = m_ini->GetDoubleValue(section.c_str(), key.c_str(), 0.0d);
    return true;
}


std::vector< std::string > CProfile::GetLocalProfileSection(std::string section, std::string key)
{
    std::vector< std::string > ret_list;

    try
    {
        for(bp::ptree::value_type const & v : m_propertyTree.get_child(section))
        {
            if (v.first == key)
            {
                ret_list.push_back(v.second.get_value<std::string>());
            }
        }
    }
    catch (std::exception & e)
    {
        GetLogger()->Info("Error on parsing profile: %s\n", e.what());
    }
    // CSimpleIniA::TNamesDepend values;
    // m_ini->GetAllValues(section.c_str(), key.c_str(), values);
    // values.sort(CSimpleIniA::Entry::LoadOrder());

    // for (auto item : values) {
    //     ret_list.push_back(item.pItem);
    // }

    return ret_list;
}
