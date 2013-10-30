// * This file is part of the COLOBOT source code
// * Copyright (C) 2001-2008, Daniel ROUX & EPSITEC SA, www.epsitec.ch
// * Copyright (C) 2013, Polish Portal of Colobot (PPC)
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

#include "app/system_macosx.h"

#include "common/logger.h"

#include <stdlib.h>

#include <CoreServices/CoreServices.h>

#include <boost/filesystem.hpp>

void CSystemUtilsMacOSX::Init()
{
    // These functions are a deprecated way to get the 'Application Support' folder, but they do work, in plain C++
    FSRef ref;
    OSType folderType = kApplicationSupportFolderType;
    char path[PATH_MAX];
    FSFindFolder( kUserDomain, folderType, kCreateFolder, &ref );
    FSRefMakePath( &ref, reinterpret_cast<UInt8*>(&path), PATH_MAX );

    m_ASPath = path;
    m_ASPath.append("/colobot/");

    // Make sure the directory exists
    boost::filesystem::create_directories(m_ASPath.c_str());
}

std::string CSystemUtilsMacOSX::GetProfileFileLocation()
{
    std::string profileFile = m_ASPath + "/colobot.ini";

    GetLogger()->Trace("Profile file is %s\n", profileFile.c_str());
    return profileFile;
}

std::string CSystemUtilsMacOSX::GetSavegameDirectoryLocation()
{
    std::string savegameDir = m_ASPath + "/savegame";
    boost::filesystem::create_directories(savegameDir.c_str());
    GetLogger()->Trace("Saved game files are going to %s\n", savegameDir.c_str());

    return savegameDir;
}

