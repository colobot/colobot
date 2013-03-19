// * This file is part of the COLOBOT source code
// * Copyright (C) 2001-2008, Daniel ROUX & EPSITEC SA, www.epsitec.ch
// * Copyright (C) 2012, Polish Portal of Colobot (PPC)
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

#include "app/system_windows.h"


// Convert a wide Unicode string to an UTF8 string
std::string UTF8_Encode_Windows(const std::wstring &wstr)
{
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], static_cast<int>(wstr.size()), NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], static_cast<int>(wstr.size()), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

// Convert an UTF8 string to a wide Unicode String
std::wstring UTF8_Decode_Windows(const std::string &str)
{
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], static_cast<int>(str.size()), NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], static_cast<int>(str.size()), &wstrTo[0], size_needed);
    return wstrTo;
}

SystemDialogResult SystemDialog_Windows(SystemDialogType type, const std::string& title, const std::string& message)
{
    unsigned int windowsType = 0;
    std::wstring windowsMessage = UTF8_Decode_Windows(message);
    std::wstring windowsTitle = UTF8_Decode_Windows(title);

    switch (type)
    {
        case SDT_INFO:
        default:
            windowsType = MB_ICONINFORMATION|MB_OK;
            break;
        case SDT_WARNING:
            windowsType = MB_ICONWARNING|MB_OK;
            break;
        case SDT_ERROR:
            windowsType = MB_ICONERROR|MB_OK;
            break;
        case SDT_YES_NO:
            windowsType = MB_ICONQUESTION|MB_YESNO;
            break;
        case SDT_OK_CANCEL:
            windowsType = MB_ICONWARNING|MB_OKCANCEL;
            break;
    }

    switch (MessageBoxW(NULL, windowsMessage.c_str(), windowsTitle.c_str(), windowsType))
    {
        case IDOK:
            return SDR_OK;
        case IDCANCEL:
            return SDR_CANCEL;
        case IDYES:
            return SDR_YES;
        case IDNO:
            return SDR_NO;
        default:
            break;
    }

    return SDR_OK;
}


void GetCurrentTimeStamp_Windows(SystemTimeStamp *stamp)
{
    GetSystemTimeAsFileTime(&stamp->fileTime);
}

long long GetTimeStampExactResolution_Windows()
{
    return 100ll;
}

long long TimeStampExactDiff_Windows(SystemTimeStamp *before, SystemTimeStamp *after)
{
    long long tH = (1ll << 32) * (after->fileTime.dwHighDateTime - before->fileTime.dwHighDateTime);
    long long tL = after->fileTime.dwLowDateTime - before->fileTime.dwLowDateTime;
    return (tH + tL) * 100ll;
}
