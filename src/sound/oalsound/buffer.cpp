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

// buffer.cpp

#include "buffer.h"

Buffer::Buffer() {
    mLoaded = false;
    mDuration = 0.0f;
}


Buffer::~Buffer() {
    if (mLoaded) {
        alDeleteBuffers(1, &mBuffer);
        if (alCheck())
            GetLogger()->Warn("Failed to unload buffer. Code %d\n", alGetCode());
    }
}


bool Buffer::LoadFromFile(std::string filename, Sound sound) {
    mSound = sound;
    GetLogger()->Debug("Loading audio file: %s\n", filename.c_str());

    SF_INFO fileInfo;
    SNDFILE *file = sf_open(filename.c_str(), SFM_READ, &fileInfo);

    GetLogger()->Trace("  channels %d\n", fileInfo.channels);
    GetLogger()->Trace("  format %d\n", fileInfo.format);
    GetLogger()->Trace("  frames %d\n", fileInfo.frames);
    GetLogger()->Trace("  samplerate %d\n", fileInfo.samplerate);
    GetLogger()->Trace("  sections %d\n", fileInfo.sections);

    if (!file) {
        GetLogger()->Warn("Could not load file. Reason: %s\n", sf_strerror(file));
        mLoaded = false;
        return false;
    }

    alGenBuffers(1, &mBuffer);
    if (!mBuffer) {
        GetLogger()->Warn("Could not create audio buffer\n");
        mLoaded = false;
        sf_close(file);
        return false;
    }

    // read chunks of 4096 samples
    std::vector<uint16_t> data;
    std::array<int16_t, 4096> buffer;
    data.reserve(fileInfo.frames);
    size_t read = 0;
    while ((read = sf_read_short(file, buffer.data(), buffer.size())) != 0) {
        data.insert(data.end(), buffer.begin(), buffer.begin() + read);
    }
    sf_close(file);   

    alBufferData(mBuffer, fileInfo.channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16, &data.front(), data.size() * sizeof(uint16_t), fileInfo.samplerate);
    mDuration = static_cast<float>(fileInfo.frames) / fileInfo.samplerate;
    mLoaded = true;
    return true;
}


Sound Buffer::GetSoundType() {
    return mSound;
}


ALuint Buffer::GetBuffer() {
    return mBuffer;
}


bool Buffer::IsLoaded() {
    return mLoaded;
}


float Buffer::GetDuration() {
    return mDuration;
}
