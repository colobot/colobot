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

/**
 * \file common/profile.h
 * \brief Class for loading profile (currently for loading ini config file)
 */

#pragma once

#include "lib/simpleini/SimpleIni.h"

#include "common/singleton.h"

#include <string>
#include <vector>


/**
* @class CProfile
*
* @brief Class for loading profile (currently for loading ini config file)
*
*/
class CProfile : public CSingleton<CProfile>
{
    public:
        CProfile();
        ~CProfile();

        /** Loads colobot.ini from current directory
         * @return return true on success
         */
        bool InitCurrentDirectory();

        /** Sets string value in section under specified key
         * @param std::string section
         * @param std::string key
         * @param std::string value
         * @return return true on success
         */
        bool SetLocalProfileString(std::string section, std::string key, std::string value);

        /** Gets string value in section under specified key
         * @param std::string section
         * @param std::string key
         * @param std::string& buffer
         * @return return true on success
         */
        bool GetLocalProfileString(std::string section, std::string key, std::string& buffer);

        /** Sets int value in section under specified key
         * @param std::string section
         * @param std::string key
         * @param int value
         * @return return true on success
         */
        bool SetLocalProfileInt(std::string section, std::string key, int value);

        /** Gets int value in section under specified key
         * @param std::string section
         * @param std::string key
         * @param int& value
         * @return return true on success
         */
        bool GetLocalProfileInt(std::string section, std::string key, int &value);

        /** Sets float value in section under specified key
         * @param std::string section
         * @param std::string key
         * @param float value
         * @return return true on success
         */
        bool SetLocalProfileFloat(std::string section, std::string key, float value);

        /** Gets float value in section under specified key
         * @param std::string section
         * @param std::string key
         * @param float& value
         * @return return true on success
         */
        bool GetLocalProfileFloat(std::string section, std::string key, float &value);

        /** Gets all values in section under specified key
         * @param std::string section
         * @param std::string key
         * @return vector of values
         */
        std::vector< std::string > GetLocalProfileSection(std::string section, std::string key);

    private:
        CSimpleIniA *m_ini;
};

//! Global function to get profile instance
inline CProfile* GetProfile() {
    return CProfile::GetInstancePointer();
}
