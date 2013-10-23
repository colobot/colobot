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
 * \file app/system_other.h
 * \brief Fallback code for other systems
 */

#include "app/system.h"

#include <SDL.h>

#include <iostream>


struct SystemTimeStamp
{
    Uint32 sdlTicks;

    SystemTimeStamp()
    {
        sdlTicks = 0;
    }
};

class CSystemUtilsOther : public CSystemUtils
{
public:
    virtual void Init() {};
    virtual SystemDialogResult SystemDialog(SystemDialogType type, const std::string& title, const std::string& message) override;

    virtual void GetCurrentTimeStamp(SystemTimeStamp *stamp) override;
    virtual long long int GetTimeStampExactResolution() override;
    virtual long long TimeStampExactDiff(SystemTimeStamp *before, SystemTimeStamp *after) override;
};

