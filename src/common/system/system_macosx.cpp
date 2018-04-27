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

#include "common/system/system_macosx.h"

#include "common/logger.h"

#include <stdlib.h>
#include <unistd.h>

// MacOS-specific headers
#include <CoreFoundation/CFBundle.h>
#include <CoreServices/CoreServices.h>

#include <boost/filesystem.hpp>

inline std::string CFStringRefToStdString(CFStringRef str) {

    std::string stdstr;

    char *fullPath;
    CFStringEncoding encodingMethod = CFStringGetSystemEncoding();

    // 1st try for English system
    fullPath = const_cast<char*>(CFStringGetCStringPtr(str, encodingMethod));
    if( fullPath == nullptr )
    {
        // 2nd try for Japanese system
        encodingMethod = kCFStringEncodingUTF8;
        fullPath = const_cast<char*>(CFStringGetCStringPtr(str, encodingMethod));
    }

    // for safer operation.
    if( fullPath == nullptr )
    {
        CFIndex length = CFStringGetLength(str);
        fullPath = static_cast<char *>(malloc( length + 1 ));

        // TODO: Check boolean result of that conversion
        CFStringGetCString(str, fullPath, length, kCFStringEncodingUTF8 );

        stdstr = fullPath;

        free( fullPath );
    }
    else
        stdstr = fullPath;

    return stdstr;
}

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

    // Get the Resources bundle URL
    CFBundleRef mainBundle = CFBundleGetMainBundle();
    CFURLRef resourcesURL = CFBundleCopyBundleURL(mainBundle);
    CFStringRef str = CFURLCopyFileSystemPath( resourcesURL, kCFURLPOSIXPathStyle );
    CFRelease(resourcesURL);

    m_dataPath = CFStringRefToStdString(str);
    m_dataPath += "/Contents/Resources";
}

std::string CSystemUtilsMacOSX::GetDataPath()
{
    return m_dataPath;
}

std::string CSystemUtilsMacOSX::GetLangPath()
{
    return m_dataPath + "/i18n";
}

std::string CSystemUtilsMacOSX::GetSaveDir()
{
    std::string savegameDir = m_ASPath;
    GetLogger()->Trace("Saved game files are going to %s\n", savegameDir.c_str());

    return savegameDir;
}

void CSystemUtilsMacOSX::Usleep(int usec)
{
    usleep(usec);
}
