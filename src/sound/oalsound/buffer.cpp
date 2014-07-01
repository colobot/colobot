// * This file is part of the COLOBOT source code
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


#include "sound/oalsound/buffer.h"

#include <cstring>

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

    SF_INFO fileInfo;
    memset(&fileInfo, 0, sizeof(SF_INFO));
    SNDFILE *file = sf_open(filename.c_str(), SFM_READ, &fileInfo);

    GetLogger()->Trace("  channels %d\n", fileInfo.channels);
    GetLogger()->Trace("  format %d\n", fileInfo.format);
    GetLogger()->Trace("  frames %d\n", fileInfo.frames);
    GetLogger()->Trace("  samplerate %d\n", fileInfo.samplerate);
    GetLogger()->Trace("  sections %d\n", fileInfo.sections);

    if (!file)
    {
        GetLogger()->Warn("Could not load file. Reason: %s\n", sf_strerror(file));
        m_loaded = false;
        return false;
    }

    alGenBuffers(1, &m_buffer);
    if (!m_buffer)
    {
        GetLogger()->Warn("Could not create audio buffer\n");
        m_loaded = false;
        sf_close(file);
        return false;
    }

    // read chunks of 4096 samples
    std::vector<uint16_t> data;
    std::array<int16_t, 4096> buffer;
    data.reserve(fileInfo.frames);
    size_t read = 0;
    while ((read = sf_read_short(file, buffer.data(), buffer.size())) != 0)
    {
        data.insert(data.end(), buffer.begin(), buffer.begin() + read);
    }
    sf_close(file);

    alBufferData(m_buffer, fileInfo.channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16, &data.front(), data.size() * sizeof(uint16_t), fileInfo.samplerate);
    m_duration = static_cast<float>(fileInfo.frames) / fileInfo.samplerate;
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

