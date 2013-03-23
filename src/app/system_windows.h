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

/**
 * \file app/system_windows.h
 * \brief Windows-specific implementation of system functions
 */

#include "app/system.h"


struct SystemTimeStamp
{
    long long counterValue;

    SystemTimeStamp()
    {
        counterValue = 0;
    }
};

class CSystemUtilsWindows : public CSystemUtils
{
public:
    virtual void Init() override;

    virtual SystemDialogResult SystemDialog(SystemDialogType type, const std::string& title, const std::string& message) override;

    virtual void GetCurrentTimeStamp(SystemTimeStamp *stamp) override;
    virtual long long GetTimeStampExactResolution() override;
    virtual long long TimeStampExactDiff(SystemTimeStamp *before, SystemTimeStamp *after) override;

private:
    std::string UTF8_Encode(const std::wstring &wstr);
    std::wstring UTF8_Decode(const std::string &str);

protected:
    long long m_counterFrequency;
};
