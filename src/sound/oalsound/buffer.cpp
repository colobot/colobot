/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2014, Daniel Roux, EPSITEC SA & TerranovaTeam
 * http://epsiteс.ch; http://colobot.info; http://github.com/colobot
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

#include <memory>

#include "common/resources/resourcemanager.h"


Buffer::Buffer()
{
    m_loaded = false;
    m_duration = 0.0f;
}


Buffer::~Buffer()
{
    if (m_loaded)
    {
        alDeleteBuffers(1, &m_buffer);
        if (alCheck())
            GetLogger()->Debug("Failed to unload buffer. Code %d\n", alGetCode());
    }
}


bool Buffer::LoadFromFile(std::string filename, Sound sound)
{
    m_sound = sound;
    GetLogger()->Debug("Loading audio file: %s\n", filename.c_str());

    std::unique_ptr<CSNDFile> file = std::unique_ptr<CSNDFile>(CResourceManager::GetSNDFileHandler(filename));

    GetLogger()->Trace("  channels %d\n", file->GetFileInfo().channels);
    GetLogger()->Trace("  format %d\n", file->GetFileInfo().format);
    GetLogger()->Trace("  frames %d\n", file->GetFileInfo().frames);
    GetLogger()->Trace("  samplerate %d\n", file->GetFileInfo().samplerate);
    GetLogger()->Trace("  sections %d\n", file->GetFileInfo().sections);

    if (!file->IsOpen())
    {
        GetLogger()->Warn("Could not load file. Reason: %s\n", file->GetLastError().c_str());
        m_loaded = false;
        return false;
    }

    alGenBuffers(1, &m_buffer);
    if (!m_buffer)
    {
        GetLogger()->Warn("Could not create audio buffer\n");
        m_loaded = false;
        return false;
    }

    // read chunks of 4096 samples
    std::vector<uint16_t> data;
    std::array<int16_t, 4096> buffer;
    data.reserve(file->GetFileInfo().frames);
    size_t read = 0;
    while ((read = file->Read(buffer.data(), buffer.size())) != 0)
    {
        data.insert(data.end(), buffer.begin(), buffer.begin() + read);
    }

    alBufferData(m_buffer, file->GetFileInfo().channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16, &data.front(), data.size() * sizeof(uint16_t), file->GetFileInfo().samplerate);
    m_duration = static_cast<float>(file->GetFileInfo().frames) / file->GetFileInfo().samplerate;
    m_loaded = true;
    return true;
}


Sound Buffer::GetSoundType()
{
    return m_sound;
}


ALuint Buffer::GetBuffer()
{
    return m_buffer;
}


bool Buffer::IsLoaded()
{
    return m_loaded;
}


float Buffer::GetDuration()
{
    return m_duration;
}

