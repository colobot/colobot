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

#pragma once

#include <filesystem>
#include <string>

#include <physfs.h>
#include <sndfile.h>


class CSNDFileWrapper final
{
public:
    CSNDFileWrapper(const std::filesystem::path& filename);
    ~CSNDFileWrapper();

    CSNDFileWrapper(const CSNDFileWrapper&) = delete;
    CSNDFileWrapper& operator=(const CSNDFileWrapper&) = delete;

    CSNDFileWrapper(CSNDFileWrapper&&) = delete;
    CSNDFileWrapper& operator=(CSNDFileWrapper&&) = delete;

    const SF_INFO& GetFileInfo() const;
    bool IsOpen() const;
    const std::string& GetLastError() const;

    sf_count_t Read(short int *ptr, sf_count_t items);

private:
    SF_INFO m_file_info = {};
    SNDFILE *m_snd_file = nullptr;
    PHYSFS_File *m_file = nullptr;
    std::string m_last_error = {};
    SF_VIRTUAL_IO m_snd_callbacks = {};
};
