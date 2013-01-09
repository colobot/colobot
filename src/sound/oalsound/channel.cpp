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

// channel.cpp

#include "channel.h"

#define MIN(a, b) (a > b ? b : a)

Channel::Channel() {
    alGenSources(1, &mSource);

    if (alCheck()) {
        GetLogger()->Warn("Failed to create sound source. Code: %d\n", alGetCode());
        mReady = false;
    } else {
        mReady = true;
    }
    
    mPriority = 0;
    mBuffer = nullptr;
    mLoop = false;
    mInitFrequency = 0.0f;
}


Channel::~Channel() {
    if (mReady) {
	alSourceStop(mSource);
        alSourcei(mSource, AL_BUFFER, 0);
        alDeleteSources(1, &mSource);
        if (alCheck())
            GetLogger()->Warn("Failed to delete sound source. Code: %d\n", alGetCode());
    }
}


bool Channel::Play() {
    if (!mReady || mBuffer == nullptr)
        return false;
    
    alSourcei(mSource, AL_LOOPING, static_cast<ALint>(mLoop));
    alSourcePlay(mSource);
    if (alCheck())
        GetLogger()->Warn("Could not play audio sound source. Code: %d\n", alGetCode());
    return true;
}


bool Channel::SetPosition(Math::Vector pos) {
    if (!mReady || mBuffer == nullptr)
        return false;
    
    alSource3f(mSource, AL_POSITION, pos.x, pos.y, pos.z);
    if (alCheck()) {
        GetLogger()->Warn("Could not set sound position. Code: %d\n", alGetCode());
        return false;
    }
    return true;
}


bool Channel::SetFrequency(float freq)
{
    if (!mReady || mBuffer == nullptr)
        return false;

    alSourcef(mSource, AL_PITCH, freq);
    if (alCheck()) {
        GetLogger()->Warn("Could not set sound pitch to '%f'. Code: %d\n", freq, alGetCode());
        return false;
    }
    return true;
}


bool Channel::AdjustFrequency(float freq)
{
    if (!mReady || mBuffer == nullptr)
        return false;

    return SetFrequency(mInitFrequency - freq);
}


float Channel::GetFrequency()
{
    ALfloat freq;
    if (!mReady || mBuffer == nullptr)
        return 0;
    
    alGetSourcef(mSource, AL_PITCH, &freq);
    if (alCheck()) {
        GetLogger()->Warn("Could not get sound pitch. Code: %d\n", alGetCode());
        return 0;
    }

    return freq;
}


bool Channel::SetVolume(float vol)
{
    if (!mReady || vol < 0 || mBuffer == nullptr)
        return false;
    
    alSourcef(mSource, AL_GAIN, MIN(vol, 1.0f));
    if (alCheck()) {
        GetLogger()->Warn("Could not set sound volume to '%f'. Code: %d\n", vol, alGetCode());
        return false;
    }
    return true;
}


float Channel::GetVolume()
{
    ALfloat vol;
    if (!mReady || mBuffer == nullptr)
        return 0;
    
    alGetSourcef(mSource, AL_GAIN, &vol);
    if (alCheck()) {
        GetLogger()->Warn("Could not get sound volume. Code: %d\n", alGetCode());
        return 0;
    }

    return vol;
}


int Channel::GetPriority()
{
    return mPriority;
}


void Channel::SetPriority(int pri)
{
    mPriority = pri;
}


void Channel::SetStartAmplitude(float gain)
{
    mStartAmplitude = gain;
    SetVolume(mStartAmplitude);
}


void Channel::SetStartFrequency(float freq)
{
    mStartFrequency = freq;
}


void Channel::SetChangeFrequency(float freq)
{
    mChangeFrequency = freq;
}


float Channel::GetStartAmplitude()
{
    return mStartAmplitude;
}


float Channel::GetStartFrequency()
{
    return mStartFrequency;
}


float Channel::GetChangeFrequency()
{
    return mChangeFrequency;
}


float Channel::GetInitFrequency()
{
    return mInitFrequency;
}


void Channel::AddOper(SoundOper oper)
{
    mOper.push_back(oper);
}


void Channel::ResetOper()
{
    mOper.clear();
}


Sound Channel::GetSoundType() {
    if (!mReady || mBuffer == nullptr)
        return SOUND_NONE;
    
    return mBuffer->GetSoundType();
}


bool Channel::SetBuffer(Buffer *buffer) {
    if (!mReady)
        return false;
    
    mBuffer = buffer;
    if (buffer == nullptr) {
	alSourcei(mSource, AL_BUFFER, 0);
	return true;
    }    
    
    alSourcei(mSource, AL_BUFFER, buffer->GetBuffer());
    if (alCheck()) {
        GetLogger()->Warn("Could not set sound buffer. Code: %d\n", alGetCode());
        return false;
    }
    mInitFrequency = GetFrequency();
    return true;
}


bool Channel::IsPlaying() {
    ALint status;
    if (!mReady || mBuffer == nullptr)
	return false;
    
    alGetSourcei(mSource, AL_SOURCE_STATE, &status);
    if (alCheck()) {
        GetLogger()->Warn("Could not get sound status. Code: %d\n", alGetCode());
        return false;
    }

    return status == AL_PLAYING;
}


bool Channel::IsReady() {
    return mReady;
}

bool Channel::IsLoaded() {
    return mBuffer == nullptr;
}


bool Channel::Stop() {
    if (!mReady || mBuffer == nullptr)
	return false;
    
    alSourceStop(mSource);
    if (alCheck()) {
        GetLogger()->Warn("Could not stop sound. Code: %d\n", alGetCode());
        return false;
    }
    return true;
}


float Channel::GetCurrentTime()
{
    if (!mReady || mBuffer == nullptr)
	return 0.0f;
    
    ALfloat current;
    alGetSourcef(mSource, AL_SEC_OFFSET, &current);
    if (alCheck()) {
        GetLogger()->Warn("Could not get source current play time. Code: %d\n", alGetCode());
        return 0.0f;
    }
    return current;
}


void Channel::SetCurrentTime(float current)
{
    if (!mReady || mBuffer == nullptr)
	return;
    
    alSourcef(mSource, AL_SEC_OFFSET, current);
    if (alCheck())
        GetLogger()->Warn("Could not get source current play time. Code: %d\n", alGetCode());
}


float Channel::GetDuration()
{
    if (!mReady || mBuffer == nullptr)
	return 0.0f;
    
    return mBuffer->GetDuration();
}


bool Channel::HasEnvelope()
{
    return mOper.size() > 0;
}


SoundOper& Channel::GetEnvelope()
{
    return mOper.front();
}


void Channel::PopEnvelope()
{
    mOper.pop_front();
}


void Channel::SetLoop(bool loop) {
    mLoop = loop;
}