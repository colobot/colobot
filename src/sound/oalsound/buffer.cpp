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
    mBuffer = alutCreateBufferFromFile(filename.c_str());

    ALenum error = alutGetError();
    if (error) {
        GetLogger()->Warn("Failed to load file. Reason: %s\n", alutGetErrorString(error));
        mLoaded = false;
        return false;
    }

    ALint size, bits, channels, freq;

    alGetBufferi(mBuffer, AL_SIZE, &size);
    alGetBufferi(mBuffer, AL_BITS, &bits);
    alGetBufferi(mBuffer, AL_CHANNELS, &channels);
    alGetBufferi(mBuffer, AL_FREQUENCY, &freq);

    mDuration =  static_cast<ALfloat>(size) * 8 / channels / bits / static_cast<ALfloat>(freq);

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
