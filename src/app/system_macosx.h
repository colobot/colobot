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
 * \file app/system_macosx.h
 * \brief MacOSX-specific implementation of system functions
 */

#include "app/system.h"
#include "app/system_other.h"

class CSystemUtilsMacOSX : public CSystemUtilsOther
{
public:
    virtual void Init() override;

    virtual std::string GetDataPath() override;
    virtual std::string GetLangPath() override;
    virtual std::string GetProfileFileLocation() override;
    virtual std::string GetSavegameDirectoryLocation() override;
private:
    std::string m_ASPath;
    std::string m_dataPath;
};

