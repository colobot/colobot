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


#include "sound/oalsound/buffer.h"

#include "common/resources/resourcemanager.h"

#include "common/stringutils.h"

#include "sound/oalsound/check.h"

#include <cstddef>
#include <memory>


CBuffer::CBuffer()
    : m_buffer(),
      m_sound(),
      m_loaded(false),
      m_duration(0.0f)
{}

CBuffer::~CBuffer()
{
    if (m_loaded)
    {
        alDeleteBuffers(1, &m_buffer);
        if (CheckOpenALError())
            GetLogger()->Debug("Failed to unload buffer. Code %%", GetOpenALErrorCode());
    }
}

bool CBuffer::LoadFromFile(const std::filesystem::path& filename, SoundType sound)
{
    m_sound = sound;
    GetLogger()->Debug("Loading audio file: %%", filename);

    auto file = CResourceManager::GetSNDFileHandler(filename);

    GetLogger()->Trace("  channels %%", file->GetFileInfo().channels);
    GetLogger()->Trace("  format %%", file->GetFileInfo().format);
    GetLogger()->Trace("  frames %%", file->GetFileInfo().frames);
    GetLogger()->Trace("  samplerate %%", file->GetFileInfo().samplerate);
    GetLogger()->Trace("  sections %%", file->GetFileInfo().sections);

    if (!file->IsOpen())
    {
        GetLogger()->Warn("Could not load file %%. Reason: %%", filename, file->GetLastError());
        m_loaded = false;
        return false;
    }

    alGenBuffers(1, &m_buffer);
    if (CheckOpenALError())
    {
        GetLogger()->Warn("Could not create audio buffer. Code: %%", GetOpenALErrorCode());
        m_loaded = false;
        return false;
    }

    // read chunks of 4096 samples
    std::vector<uint16_t> data;
    std::array<int16_t, 4096> buffer;
    data.reserve(file->GetFileInfo().frames);
    std::size_t read = 0;
    while ((read = file->Read(buffer.data(), buffer.size())) != 0)
    {
        data.insert(data.end(), buffer.begin(), buffer.begin() + read);
    }

    ALenum format = file->GetFileInfo().channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
    alBufferData(m_buffer, format, &data.front(), data.size() * sizeof(uint16_t), file->GetFileInfo().samplerate);
    m_duration = static_cast<float>(file->GetFileInfo().frames) / file->GetFileInfo().samplerate;
    m_loaded = true;
    return true;
}

SoundType CBuffer::GetSoundType()
{
    return m_sound;
}

ALuint CBuffer::GetBuffer()
{
    return m_buffer;
}

bool CBuffer::IsLoaded()
{
    return m_loaded;
}

float CBuffer::GetDuration()
{
    return m_duration;
}
