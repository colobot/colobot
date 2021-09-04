/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2020, Daniel Roux, EPSITEC SA & TerranovaTeam
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
 * \file common/config_file.h
 * \brief Class for loading profile (currently for loading ini config file)
 */

#pragma once

#include "common/singleton.h"

#include "common/logger.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/lexical_cast.hpp>

#include <string>
#include <sstream>
#include <vector>
#include <stdexcept>


/**
* \class CConfigFile
*
* \brief Class for loading config file
*
*/
class CConfigFile : public CSingleton<CConfigFile>
{
public:
    CConfigFile();
    virtual ~CConfigFile();

    /** Set flag to force using ini file from current directory */
    void SetUseCurrentDirectory(bool useCurrentDirectory);

    /** Loads colobot.ini
     * \return return true on success
     */
    bool Init();

    /** Saves colobot.ini
     * \return return true on success
     */
    bool Save();

    /** Sets string value in section under specified key
     * \return return true on success
     */
    bool SetStringProperty(std::string section, std::string key, std::string value);

    /** Gets string value in section under specified key
     * \return return true on success
     */
    bool GetStringProperty(std::string section, std::string key, std::string& value);

    /** Sets int value in section under specified key
     * \return return true on success
     */
    bool SetIntProperty(std::string section, std::string key, int value);

    /** Sets bool value in section under specified key
     * \return return true on success
     */
    bool SetBoolProperty(std::string section, std::string key, bool value);

    /** Gets int value in section under specified key
     * \a value will only be changed if key exists
     * \return return true on success
     */
    bool GetIntProperty(std::string section, std::string key, int &value);

    /** Sets float value in section under specified key
     * \a value will only be changed if key exists
     * \return return true on success
     */
    bool SetFloatProperty(std::string section, std::string key, float value);

    /** Gets float value in section under specified key
     * \a value will only be changed if key exists
     * \return return true on success
     */
    bool GetFloatProperty(std::string section, std::string key, float &value);

    /** Gets bool value in section under specified key
     * \a value will only be changed if key exists
     * \return return true on success
     */
    bool GetBoolProperty(std::string section, std::string key, bool &value);

    /** Gets an array of values of type T in section under specified key
     * The value separator is ','.
     * \a array will only be changed if key exists
     * \return return true on success
     */
    template<typename T>
    bool SetArrayProperty(std::string section, std::string key, const std::vector<T>& array)
    {
        try
        {
            std::string convertedValue = ArrayToString(array);
            m_propertyTree.put(section + "." + key, convertedValue);
            m_needsSave = true;
        }
        catch (std::exception & e)
        {
            GetLogger()->Error("Error on editing config file: %s\n", e.what());
            return false;
        }
        return true;
    }

    /** Sets an array of values of type T in section under specified key.
     * The value separator is ','.
     * \a array will only be changed if key exists
     * \return return true on success
     */
    template<typename T>
    bool GetArrayProperty(std::string section, std::string key, std::vector<T>& array)
    {
        try
        {
            std::string readValue = m_propertyTree.get<std::string>(section + "." + key);
            std::vector<T> convertedValue = StringToArray<T>(readValue);
            array = std::move(convertedValue);
        }
        catch (std::exception & e)
        {
            GetLogger()->Log(m_loaded ? LOG_INFO : LOG_TRACE, "Error on parsing config file: %s\n", e.what());
            return false;
        }
        return true;
    }

private:
    template<typename T>
    std::vector<T> StringToArray(const std::string& s)
    {
        std::vector<T> result;
        std::stringstream ss(s);
        std::string item;
        while (std::getline(ss, item, ','))
        {
            result.push_back(boost::lexical_cast<T>(item));
        }
        return result;
    }

    template<typename T>
    std::string ArrayToString(const std::vector<T> &array)
    {
        std::ostringstream oss;
        if (!array.empty())
        {
            std::copy(array.begin(), array.end() - 1, std::ostream_iterator<T>(oss, ","));
            oss << array.back();
        }
        return oss.str();
    }

private:
    boost::property_tree::ptree m_propertyTree;
    bool m_needsSave;
    bool m_useCurrentDirectory;
    bool m_loaded;
};

//! Global function to get config file instance
inline CConfigFile & GetConfigFile()
{
    return CConfigFile::GetInstance();
}
