/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2023, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include "common/system/system_windows.h"

#include "common/logger.h"
#include "common/stringutils.h"
#include "common/version.h"

#include <filesystem>

#include <Windows.h>

std::unique_ptr<CSystemUtils> CSystemUtils::Create()
{
    return std::make_unique<CSystemUtilsWindows>();
}

void CSystemUtilsWindows::Init([[maybe_unused]] const std::vector<std::string>& args)
{
    m_arguments.clear();

    int wargc = 0;
    wchar_t** wargv = CommandLineToArgvW(GetCommandLineW(), &wargc);
    if (wargv == nullptr)
    {
        GetLogger()->Error("CommandLineToArgvW failed\n");
        return;
    }

    for (int i = 0; i < wargc; i++)
    {
        std::wstring warg = wargv[i];
        m_arguments.push_back(CSystemUtilsWindows::UTF8_Encode(warg));
    }

    LocalFree(wargv);
}

SystemDialogResult CSystemUtilsWindows::SystemDialog(SystemDialogType type, const std::string& title, const std::string& message)
{
    unsigned int windowsType = 0;
    std::wstring windowsMessage = UTF8_Decode(message);
    std::wstring windowsTitle = UTF8_Decode(title);

    switch (type)
    {
        case SystemDialogType::INFO:
        default:
            windowsType = MB_ICONINFORMATION|MB_OK;
            break;
        case SystemDialogType::WARNING:
            windowsType = MB_ICONWARNING|MB_OK;
            break;
        case SystemDialogType::ERROR_MSG:
            windowsType = MB_ICONERROR|MB_OK;
            break;
        case SystemDialogType::YES_NO:
            windowsType = MB_ICONQUESTION|MB_YESNO;
            break;
        case SystemDialogType::OK_CANCEL:
            windowsType = MB_ICONWARNING|MB_OKCANCEL;
            break;
    }

    switch (MessageBoxW(nullptr, windowsMessage.c_str(), windowsTitle.c_str(), windowsType))
    {
        case IDOK:
            return SystemDialogResult::OK;
        case IDCANCEL:
            return SystemDialogResult::CANCEL;
        case IDYES:
            return SystemDialogResult::YES;
        case IDNO:
            return SystemDialogResult::NO;
        default:
            break;
    }

    return SystemDialogResult::OK;
}

//! Converts a wide Unicode string to an UTF8 string
std::string CSystemUtilsWindows::UTF8_Encode(const std::wstring& wstr)
{
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], static_cast<int>(wstr.size()), nullptr, 0, nullptr, nullptr);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], static_cast<int>(wstr.size()), &strTo[0], size_needed, nullptr, nullptr);
    return strTo;
}

//! Converts an UTF8 string to a wide Unicode String
std::wstring CSystemUtilsWindows::UTF8_Decode(const std::string& str)
{
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], static_cast<int>(str.size()), nullptr, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], static_cast<int>(str.size()), &wstrTo[0], size_needed);
    return wstrTo;

}

std::filesystem::path CSystemUtilsWindows::GetSaveDir() const
{
    if constexpr (Version::PORTABLE_SAVES || Version::DEVELOPMENT_BUILD)
    {
        return CSystemUtils::GetSaveDir();
    }
    else
    {
        std::filesystem::path savegameDir;

        auto envUSERPROFILE = GetEnvVar("USERPROFILE");
        if (envUSERPROFILE.empty())
        {
            GetLogger()->Warn("Unable to find directory for saves - using default directory");
            savegameDir = CSystemUtils::GetSaveDir();
        }
        else
        {
            savegameDir = StrUtils::ToPath(envUSERPROFILE) / "colobot";
        }
        GetLogger()->Trace("Saved game files are going to %%", savegameDir);

        return savegameDir;
    }
}

std::string CSystemUtilsWindows::GetEnvVar(const std::string& name) const
{
    std::wstring wname(name.begin(), name.end());
    wchar_t* envVar = _wgetenv(wname.c_str());
    if (envVar == nullptr)
    {
        return "";
    }
    else
    {
        std::string var = UTF8_Encode(std::wstring(envVar));
        GetLogger()->Trace("Detected environment variable %% = %%", name, var);
        return var;
    }
}

bool CSystemUtilsWindows::OpenPath(const std::filesystem::path& path)
{
    int result = system(("start explorer \"" + std::filesystem::path{path}.make_preferred().string() + "\"").c_str());
    if (result != 0)
    {
        GetLogger()->Error("Failed to open path: %%, error code: %%", path, result);
        return false;
    }
    return true;
}

bool CSystemUtilsWindows::OpenWebsite(const std::string& url)
{
    int result = system(("rundll32 url.dll,FileProtocolHandler \"" + url + "\"").c_str());
    if (result != 0)
    {
        GetLogger()->Error("Failed to open website: %%, error code: %%", url, result);
        return false;
    }
    return true;
}

bool CSystemUtilsWindows::IsDebuggerPresent() const
{
    return ::IsDebuggerPresent();
}
