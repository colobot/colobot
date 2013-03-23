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


#include "app/system.h"

#include "common/config.h"


#if defined(PLATFORM_WINDOWS)
    #include "app/system_windows.h"
#elif defined(PLATFORM_LINUX)
    #include "app/system_linux.h"
#else
    #include "app/system_other.h"
#endif

#include <cassert>
#include <iostream>


template<>
CSystemUtils* CSingleton<CSystemUtils>::m_instance = nullptr;


CSystemUtils::CSystemUtils()
{
}

CSystemUtils* CSystemUtils::Create()
{
    assert(m_instance == nullptr);
#if defined(PLATFORM_WINDOWS)
    m_instance = new CSystemUtilsWindows();
#elif defined(PLATFORM_LINUX)
    m_instance = new CSystemUtilsLinux();
#else
    m_instance = new CSystemUtilsOther();
#endif
    return m_instance;
}

SystemDialogResult CSystemUtils::ConsoleSystemDialog(SystemDialogType type, const std::string& title, const std::string& message)
{
    switch (type)
    {
        case SDT_INFO:
            std::cout << "INFO: ";
            break;
        case SDT_WARNING:
            std::cout << "WARNING:";
            break;
        case SDT_ERROR:
            std::cout << "ERROR: ";
            break;
        case SDT_YES_NO:
        case SDT_OK_CANCEL:
            std::cout << "QUESTION: ";
            break;
    }

    std::cout << message << std::endl;

    std::string line;

    SystemDialogResult result = SDR_OK;

    bool done = false;
    while (!done)
    {
        switch (type)
        {
            case SDT_INFO:
            case SDT_WARNING:
            case SDT_ERROR:
                std::cout << "Press ENTER to continue";
                break;

            case SDT_YES_NO:
                std::cout << "Type 'Y' for Yes or 'N' for No";
                break;

            case SDT_OK_CANCEL:
                std::cout << "Type 'O' for OK or 'C' for Cancel";
                break;
        }

        std::getline(std::cin, line);

        switch (type)
        {
            case SDT_INFO:
            case SDT_WARNING:
            case SDT_ERROR:
                done = true;
                break;

            case SDT_YES_NO:
                if (line == "Y" || line == "y")
                {
                    result = SDR_YES;
                    done = true;
                }
                else if (line == "N" || line == "n")
                {
                    result = SDR_NO;
                    done = true;
                }
                break;

            case SDT_OK_CANCEL:
                if (line == "O" || line == "o")
                {
                    done = true;
                    result = SDR_OK;
                }
                else if (line == "C" || line == "c")
                {
                    done = true;
                    result = SDR_CANCEL;
                }
                break;
        }
    }

    return result;
}

SystemTimeStamp* CSystemUtils::CreateTimeStamp()
{
    return new SystemTimeStamp();
}

void CSystemUtils::DestroyTimeStamp(SystemTimeStamp *stamp)
{
    delete stamp;
}

void CSystemUtils::CopyTimeStamp(SystemTimeStamp *dst, SystemTimeStamp *src)
{
    *dst = *src;
}

float CSystemUtils::GetTimeStampResolution(SystemTimeUnit unit)
{
    unsigned long long exact = GetTimeStampExactResolution();
    float result = 0.0f;
    if (unit == STU_SEC)
        result = exact * 1e-9;
    else if (unit == STU_MSEC)
        result = exact * 1e-6;
    else if (unit == STU_USEC)
        result = exact * 1e-3;
    else
        assert(false);

    return result;
}

float CSystemUtils::TimeStampDiff(SystemTimeStamp *before, SystemTimeStamp *after, SystemTimeUnit unit)
{
    long long exact = TimeStampExactDiff(before, after);

    float result = 0.0f;
    if (unit == STU_SEC)
        result = exact * 1e-9;
    else if (unit == STU_MSEC)
        result = exact * 1e-6;
    else if (unit == STU_USEC)
        result = exact * 1e-3;
    else
        assert(false);

    return result;
}
