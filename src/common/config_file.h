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

/**
 * \file common/config_file.h
 * \brief Class for loading profile (currently for loading ini config file)
 */

#pragma once

#include "common/singleton.h"

#include <boost/property_tree/ptree.hpp>

#include <string>


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
