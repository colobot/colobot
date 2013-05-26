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


#include "common/profile.h"

#include "common/logger.h"

#include "app/system.h"

#include <utility>
#include <cstring>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/regex.hpp>


template<> CProfile* CSingleton<CProfile>::m_instance = nullptr;

namespace bp = boost::property_tree;

CProfile::CProfile() :
    m_profileNeedSave(false)
{
}


CProfile::~CProfile()
{
    SaveCurrentDirectory();
}


bool CProfile::InitCurrentDirectory()
{
    try
    {
        // TODO: NDEBUG should be replaced with something like BUILD_TYPE == "DEBUG"/"RELEASE"
        #ifdef NDEBUG
        bp::ini_parser::read_ini(GetSystemUtils()->GetProfileFileLocation(), m_propertyTree);
        #else
        bp::ini_parser::read_ini("colobot.ini", m_propertyTree);
        #endif
    }
    catch (std::exception & e)
    {
        GetLogger()->Error("Error on parsing profile: %s\n", e.what());
        return false;
    }
    return true;
}

bool CProfile::SaveCurrentDirectory()
{
    if (m_profileNeedSave)
    {
        try
        {
            #ifdef NDEBUG
            bp::ini_parser::write_ini(GetSystemUtils()->GetProfileFileLocation(), m_propertyTree);
            #else
            bp::ini_parser::write_ini("colobot.ini", m_propertyTree);
            #endif
        }
        catch (std::exception & e)
        {
            GetLogger()->Error("Error on storing profile: %s\n", e.what());
            return false;
        }
    }
    return true;
}

bool CProfile::SetLocalProfileString(std::string section, std::string key, std::string value)
{
    try
    {
        m_propertyTree.put(section + "." + key, value);
        m_profileNeedSave = true;
    }
    catch (std::exception & e)
    {
        GetLogger()->Error("Error on parsing profile: %s\n", e.what());
        return false;
    }
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
        GetLogger()->Error("Error on parsing profile: %s\n", e.what());
        return false;
    }
    return true;
}


bool CProfile::SetLocalProfileInt(std::string section, std::string key, int value)
{
    try
    {
        m_propertyTree.put(section + "." + key, value);
        m_profileNeedSave = true;
    }
    catch (std::exception & e)
    {
        GetLogger()->Error("Error on parsing profile: %s\n", e.what());
        return false;
    }
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
        GetLogger()->Error("Error on parsing profile: %s\n", e.what());
        return false;
    }
    return true;
}


bool CProfile::SetLocalProfileFloat(std::string section, std::string key, float value)
{
    try
    {
        m_propertyTree.put(section + "." + key, value);
        m_profileNeedSave = true;
    }
    catch (std::exception & e)
    {
        GetLogger()->Error("Error on parsing profile: %s\n", e.what());
        return false;
    }
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
        GetLogger()->Error("Error on parsing profile: %s\n", e.what());
        return false;
    }
    return true;
}


std::vector< std::string > CProfile::GetLocalProfileSection(std::string section, std::string key)
{
    std::vector< std::string > ret_list;
    boost::regex re(key + "[0-9]*"); //we want to match all key followed my any number

    try
    {
        for(bp::ptree::value_type const & v : m_propertyTree.get_child(section))
        {
            if (boost::regex_search(v.first, re))
            {
                ret_list.push_back(v.second.get_value<std::string>());
            }
        }
    }
    catch (std::exception & e)
    {
        GetLogger()->Error("Error on parsing profile: %s\n", e.what());
    }

    return ret_list;
}


void CProfile::SetUserDir(std::string dir)
{
    m_userDirectory = dir;
}


std::string CProfile::GetUserBasedPath(std::string dir, std::string default_dir)
{
    std::string path = dir;
    boost::replace_all(path, "\\", "/");
    if (dir.find("/") == std::string::npos)
    {
        path = default_dir + "/" + dir;
    }

    if (m_userDirectory.length() > 0)
    {
        boost::replace_all(path, "%user%", m_userDirectory);
    }
    else
    {
        boost::replace_all(path, "%user%", default_dir);
    }

    return fs::path(path).make_preferred().string();
}


bool CProfile::CopyFileToTemp(std::string filename)
{
    std::string src, dst;
    std::string tmp_user_dir = m_userDirectory;

    src = GetUserBasedPath(filename, "textures");
    SetUserDir("temp");
    dst = GetUserBasedPath(filename, "textures");
    SetUserDir(tmp_user_dir);

    fs::create_directory(fs::path(dst).parent_path().make_preferred().string());
    fs::copy_file(src, dst, fs::copy_option::overwrite_if_exists);
    if (fs::exists(dst))
    {
        return true;
    }

    return false;
}

