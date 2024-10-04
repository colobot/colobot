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
 * \file buffer.h
 * \brief OpenAL buffer
 */

#pragma once

#include "sound/sound.h"

#include <string>
#include <vector>
#include <array>
#include <filesystem>

#include <al.h>

class CBuffer
{
public:
    CBuffer();
    ~CBuffer();

    bool LoadFromFile(const std::filesystem::path&, SoundType);
    bool IsLoaded();

    SoundType GetSoundType();
    ALuint GetBuffer();
    float GetDuration();

private:
    ALuint m_buffer;
    SoundType m_sound;
    bool m_loaded;
    float m_duration;
};

