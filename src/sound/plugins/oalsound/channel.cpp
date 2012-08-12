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
}


Channel::~Channel() {
    if (mReady) {
        alSourcei(mSource, AL_BUFFER, 0);
        alDeleteSources(1, &mSource);
        if (alCheck())
            GetLogger()->Warn("Failed to delete sound source. Code: %s\n", alGetCode());
    }
}


bool Channel::Play() {
    if (!mReady)
        return false;
    
    alSourcePlay(mSource);
    if (alCheck())
        GetLogger()->Warn("Could not play audio sound source. Code: %s\n", alGetCode());
    return true;
}


bool Channel::SetPosition(Math::Vector pos) {
    if (!mReady)
        return false;
    
    alSource3f(mSource, AL_POSITION, pos.x, pos.y, pos.z);
    if (alCheck()) {
        GetLogger()->Warn("Could not set sound position. Code: %s\n", alGetCode());
        return false;
    }
    return true;
}


bool Channel::SetFrequency(float freq)
{
    if (!mReady)
        return false;

    alSourcef(mSource, AL_PITCH, freq);
    if (alCheck()) {
        GetLogger()->Warn("Could not set sound pitch. Code: %s\n", alGetCode());
        return false;
    }
    return true;
}


float Channel::GetFrequency()
{
    ALfloat freq;
    if (!mReady)
        return 0;
    
    alGetSourcef(mSource, AL_PITCH, &freq);
    if (alCheck()) {
        GetLogger()->Warn("Could not get sound pitch. Code: %s\n", alGetCode());
        return 0;
    }

    return freq;
}


bool Channel::SetVolume(float vol)
{
    if (!mReady || vol < 0)
        return false;
    
    alSourcef(mSource, AL_GAIN, vol / MAXVOLUME);
    if (alCheck()) {
        GetLogger()->Warn("Could not set sound volume. Code: %s\n", alGetCode());
        return false;
    }
    return true;
}


float Channel::GetVolume()
{
    ALfloat vol;
    if (!mReady)
        return 0;
    
    alGetSourcef(mSource, AL_GAIN, &vol);
    if (alCheck()) {
        GetLogger()->Warn("Could not get sound volume. Code: %s\n", alGetCode());
        return 0;
    }

    return vol * MAXVOLUME;
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
}


void Channel::SetStartFrequency(float freq)
{
    mStartFrequency = freq;
}


void Channel::SetChangeFrequency(float freq)
{
    mChangeFrequency = freq;
}


void Channel::SetInitFrequency(float freq)
{
    mInitFrequency = freq;
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
    return mBuffer->GetSoundType();
}


bool Channel::SetBuffer(Buffer *buffer) {
    if (!mReady)
        return false;
    
    assert(buffer);
    mBuffer = buffer;
    alSourcei(mSource, AL_BUFFER, buffer->GetBuffer());
    if (alCheck()) {
        GetLogger()->Warn("Could not set sound buffer. Code: %s\n", alGetCode());
        return false;
    }
    mInitFrequency = GetFrequency();
    return true;
}


void Channel::AdjustFrequency(float freq) {
    SetFrequency(freq * mInitFrequency);
}


void Channel::AdjustVolume(float volume) {
    SetVolume(mStartAmplitude * (float) volume);
}


bool Channel::IsPlaying() {
    ALint status;
    if (!mReady) return false;
    
    alGetSourcei(mSource, AL_SOURCE_STATE, &status);
    if (alCheck()) {
        GetLogger()->Warn("Could not get sound status. Code: %s\n", alGetCode());
        return false;
    }

    return status == AL_PLAYING;
}


bool Channel::IsReady() {
    return mReady;
}


bool Channel::Stop() {
    alSourceStop(mSource);
    if (alCheck()) {
        GetLogger()->Warn("Could not stop sound. Code: %s\n", alGetCode());
        return false;
    }
    return true;
}


float Channel::GetCurrentTime()
{
    ALfloat current;
    alGetSourcef(mSource, AL_SEC_OFFSET, &current);
    if (alCheck()) {
        GetLogger()->Warn("Could not get source current play time. Code: %s\n", alGetCode());
        return 0.0f;
    }
    return current;
}


void Channel::SetCurrentTime(float current)
{
    alSourcef(mSource, AL_SEC_OFFSET, current);
    if (alCheck())
        GetLogger()->Warn("Could not get source current play time. Code: %s\n", alGetCode());
}


float Channel::GetDuration()
{
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
