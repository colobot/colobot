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


#include "sound/oalsound/channel.h"

Channel::Channel()
{
    alGenSources(1, &m_source);

    if (alCheck())
    {
        GetLogger()->Warn("Failed to create sound source. Code: %d\n", alGetCode());
        m_ready = false;
    }
    else
    {
        m_ready = true;
    }

    m_priority = 0;
    m_buffer = nullptr;
    m_loop = false;
    m_mute = false;
    m_initFrequency = 0.0f;
    m_startAmplitude = 0.0f;
    m_startFrequency = 0.0f;
    m_changeFrequency = 0.0f;
    m_volume = 0.0f;
}


Channel::~Channel()
{
    if (m_ready)
    {
        alSourceStop(m_source);
        alSourcei(m_source, AL_BUFFER, 0);
        alDeleteSources(1, &m_source);
        if (alCheck())
            GetLogger()->Warn("Failed to delete sound source. Code: %d\n", alGetCode());
    }
}


bool Channel::Play()
{
    if (!m_ready || m_buffer == nullptr)
    {
        return false;
    }

    alSourcei(m_source, AL_LOOPING, static_cast<ALint>(m_loop));
    alSourcei(m_source, AL_REFERENCE_DISTANCE, 10.0f);
    alSourcei(m_source, AL_MAX_DISTANCE, 110.0f);
    alSourcePlay(m_source);
    if (alCheck())
    {
        GetLogger()->Warn("Could not play audio sound source. Code: %d\n", alGetCode());
    }
    return true;
}


bool Channel::SetPan(Math::Vector pos)
{
    if (!m_ready || m_buffer == nullptr)
    {
        return false;
    }

    alSource3f(m_source, AL_POSITION, pos.x, pos.y, pos.z);
    if (alCheck())
    {
        GetLogger()->Warn("Could not set sound position. Code: %d\n", alGetCode());
        return false;
    }
    return true;
}


void Channel::SetPosition(Math::Vector pos)
{
    m_position = pos;
}


Math::Vector Channel::GetPosition()
{
    return m_position;
}


bool Channel::SetFrequency(float freq)
{
    if (!m_ready || m_buffer == nullptr)
    {
        return false;
    }

    alSourcef(m_source, AL_PITCH, freq);
    if (alCheck())
    {
        GetLogger()->Warn("Could not set sound pitch to '%f'. Code: %d\n", freq, alGetCode());
        return false;
    }
    return true;
}


float Channel::GetFrequency()
{
    ALfloat freq;
    if (!m_ready || m_buffer == nullptr)
    {
        return 0;
    }

    alGetSourcef(m_source, AL_PITCH, &freq);
    if (alCheck())
    {
        GetLogger()->Warn("Could not get sound pitch. Code: %d\n", alGetCode());
        return 0;
    }

    return freq;
}


bool Channel::SetVolume(float vol)
{
    if (!m_ready || vol < 0 || m_buffer == nullptr)
    {
        return false;
    }

    alSourcef(m_source, AL_GAIN, vol);
    if (alCheck())
    {
        GetLogger()->Warn("Could not set sound volume to '%f'. Code: %d\n", vol, alGetCode());
        return false;
    }
    return true;
}


float Channel::GetVolume()
{
    ALfloat vol;
    if (!m_ready || m_buffer == nullptr)
    {
        return 0;
    }

    alGetSourcef(m_source, AL_GAIN, &vol);
    if (alCheck())
    {
        GetLogger()->Warn("Could not get sound volume. Code: %d\n", alGetCode());
        return 0;
    }

    return vol;
}


void Channel::SetVolumeAtrib(float volume)
{
    m_volume = volume;
}


float Channel::GetVolumeAtrib()
{
    return m_volume;
}



int Channel::GetPriority()
{
    return m_priority;
}


void Channel::SetPriority(int pri)
{
    m_priority = pri;
}


void Channel::SetStartAmplitude(float gain)
{
    m_startAmplitude = gain;
}


void Channel::SetStartFrequency(float freq)
{
    m_startFrequency = freq;
}


void Channel::SetChangeFrequency(float freq)
{
    m_changeFrequency = freq;
}


float Channel::GetStartAmplitude()
{
    return m_startAmplitude;
}


float Channel::GetStartFrequency()
{
    return m_startFrequency;
}


float Channel::GetChangeFrequency()
{
    return m_changeFrequency;
}


float Channel::GetInitFrequency()
{
    return m_initFrequency;
}


void Channel::AddOper(SoundOper oper)
{
    m_oper.push_back(oper);
}


void Channel::ResetOper()
{
    m_oper.clear();
}


Sound Channel::GetSoundType()
{
    if (!m_ready || m_buffer == nullptr)
    {
        return SOUND_NONE;
    }

    return m_buffer->GetSoundType();
}


bool Channel::SetBuffer(Buffer *buffer)
{
    if (!m_ready)
        return false;

    Stop();
    m_buffer = buffer;
    if (buffer == nullptr)
    {
        alSourcei(m_source, AL_BUFFER, 0);
        return true;
    }

    alSourcei(m_source, AL_BUFFER, buffer->GetBuffer());
    if (alCheck())
    {
        GetLogger()->Warn("Could not set sound buffer. Code: %d\n", alGetCode());
        return false;
    }
    m_initFrequency = GetFrequency();
    return true;
}


bool Channel::FreeBuffer()
{
    if (!m_ready || !m_buffer)
    {
        return false;
    }

    alSourceStop(m_source);
    alSourcei(m_source, AL_BUFFER, 0);
    delete m_buffer;
    m_buffer = nullptr;
    return true;
}


bool Channel::IsPlaying()
{
    ALint status;
    if (!m_ready || m_buffer == nullptr)
    {
        return false;
    }

    alGetSourcei(m_source, AL_SOURCE_STATE, &status);
    if (alCheck())
    {
        GetLogger()->Warn("Could not get sound status. Code: %d\n", alGetCode());
        return false;
    }

    return status == AL_PLAYING;
}


bool Channel::IsReady()
{
    return m_ready;
}

bool Channel::IsLoaded()
{
    return m_buffer != nullptr;
}


bool Channel::Stop()
{
    if (!m_ready || m_buffer == nullptr)
    {
        return false;
    }

    alSourceStop(m_source);
    if (alCheck())
    {
        GetLogger()->Warn("Could not stop sound. Code: %d\n", alGetCode());
        return false;
    }
    return true;
}


float Channel::GetCurrentTime()
{
    if (!m_ready || m_buffer == nullptr)
    {
        return 0.0f;
    }

    ALfloat current;
    alGetSourcef(m_source, AL_SEC_OFFSET, &current);
    if (alCheck())
    {
        GetLogger()->Warn("Could not get source current play time. Code: %d\n", alGetCode());
        return 0.0f;
    }
    return current;
}


void Channel::SetCurrentTime(float current)
{
    if (!m_ready || m_buffer == nullptr)
    {
        return;
    }

    alSourcef(m_source, AL_SEC_OFFSET, current);
    if (alCheck())
    {
        GetLogger()->Warn("Could not get source current play time. Code: %d\n", alGetCode());
    }
}


float Channel::GetDuration()
{
    if (!m_ready || m_buffer == nullptr)
    {
        return 0.0f;
    }

    return m_buffer->GetDuration();
}


bool Channel::HasEnvelope()
{
    return m_oper.size() > 0;
}


SoundOper& Channel::GetEnvelope()
{
    return m_oper.front();
}


void Channel::PopEnvelope()
{
    m_oper.pop_front();
}


void Channel::SetLoop(bool loop)
{
    m_loop = loop;
}


void Channel::Mute(bool mute)
{
    m_mute = mute;
}


bool Channel::IsMuted()
{
    return m_mute;
}

