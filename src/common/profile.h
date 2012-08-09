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

// profile.h

#pragma once

#include <cstdlib>

#include <lib/simpleini/SimpleIni.h>

#include <common/singleton.h>


class CProfile : public CSingleton<CProfile>
{
    public:
        CProfile();
        ~CProfile();

        bool InitCurrentDirectory();
        bool SetLocalProfileString(std::string section, std::string key, std::string value);
        bool GetLocalProfileString(std::string section, std::string key, std::string& buffer);

        bool SetLocalProfileInt(std::string section, std::string key, int value);
        bool GetLocalProfileInt(std::string section, std::string key, int &value);

        bool SetLocalProfileFloat(std::string section, std::string key, float value);
        bool GetLocalProfileFloat(std::string section, std::string key, float &value);

        static CProfile& GetInstance();
        static CProfile* GetInstancePointer();

    private:
        CSimpleIniA *m_ini;
};
