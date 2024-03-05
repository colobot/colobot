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

/**
 * \file common/system/system_macosx.h
 * \brief MacOSX-specific implementation of system functions
 */

#include "common/system/system.h"

//@colobot-lint-exclude UndefinedFunctionRule

class CSystemUtilsMacOSX : public CSystemUtils
{
public:
    void Init(const std::vector<std::string>& args) override;

    SystemDialogResult SystemDialog(SystemDialogType type, const std::string& title, const std::string& message) override;

    std::filesystem::path GetDataPath() override;
    std::filesystem::path GetLangPath() override;
    std::filesystem::path GetSaveDir() override;

    std::string GetEnvVar(const std::string& name) override;

    bool OpenPath(const std::filesystem::path& path) override;
    bool OpenWebsite(const std::string& url) override;

private:
    std::filesystem::path m_ASPath;
    std::filesystem::path m_dataPath;
};

//@end-colobot-lint-exclude
