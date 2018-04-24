/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2018, Daniel Roux, EPSITEC SA & TerranovaTeam
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


#include "sound/oalsound/channel.h"

#include "sound/oalsound/buffer.h"

CChannel::CChannel()
    : m_buffer(nullptr),
      m_source(0),
      m_priority(0),
      m_id(0),
      m_startAmplitude(0.0f),
      m_startFrequency(0.0f),
      m_changeFrequency(0.0f),
      m_initFrequency(0.0f),
      m_volume(0.0f),
      m_ready(false),
      m_loop(false),
      m_mute(false)
{
    alGenSources(1, &m_source);

    if (CheckOpenALError())
    {
        GetLogger()->Debug("Failed to create sound source. Code: %d\n", GetOpenALErrorCode());
        m_ready = false;
    }
    else
    {
        m_ready = true;
    }
}

CChannel::~CChannel()
{
    if (m_ready)
    {
        alSourceStop(m_source);
        alSourcei(m_source, AL_BUFFER, 0);
        alDeleteSources(1, &m_source);
        if (CheckOpenALError())
            GetLogger()->Debug("Failed to delete sound source. Code: %d\n", GetOpenALErrorCode());
    }
}

bool CChannel::Play()
{
    if (!m_ready || m_buffer == nullptr)
    {
        return false;
    }

    alSourcei(m_source, AL_LOOPING, static_cast<ALint>(m_loop));
    alSourcei(m_source, AL_REFERENCE_DISTANCE, 10.0f);
    alSourcei(m_source, AL_MAX_DISTANCE, 110.0f);
    alSourcePlay(m_source);
    if (CheckOpenALError())
    {
        GetLogger()->Debug("Could not play audio sound source. Code: %d\n", GetOpenALErrorCode());
    }
    return true;
}

bool CChannel::Pause()
{
    if (!m_ready || !IsPlaying())
    {
        return false;
    }

    alSourcePause(m_source);
    if (CheckOpenALError())
    {
        GetLogger()->Debug("Could not pause audio sound source. Code: %d\n", GetOpenALErrorCode());
    }
    return true;
}

bool CChannel::SetPosition(const Math::Vector &pos)
{
    if (!m_ready || m_buffer == nullptr)
    {
        return false;
    }

    alSource3f(m_source, AL_POSITION, pos.x, pos.y, pos.z);
    if (CheckOpenALError())
    {
        GetLogger()->Debug("Could not set sound position. Code: %d\n", GetOpenALErrorCode());
        return false;
    }
    return true;
}

bool CChannel::SetFrequency(float freq)
{
    if (!m_ready || m_buffer == nullptr)
    {
        return false;
    }

    alSourcef(m_source, AL_PITCH, freq);
    if (CheckOpenALError())
    {
        GetLogger()->Debug("Could not set sound pitch to '%f'. Code: %d\n", freq, GetOpenALErrorCode());
        return false;
    }
    return true;
}

float CChannel::GetFrequency()
{
    ALfloat freq;
    if (!m_ready || m_buffer == nullptr)
    {
        return 0;
    }

    alGetSourcef(m_source, AL_PITCH, &freq);
    if (CheckOpenALError())
    {
        GetLogger()->Debug("Could not get sound pitch. Code: %d\n", GetOpenALErrorCode());
        return 0;
    }

    return freq;
}

bool CChannel::SetVolume(float vol)
{
    if (!m_ready || vol < 0 || m_buffer == nullptr)
    {
        return false;
    }

    alSourcef(m_source, AL_GAIN, vol);
    if (CheckOpenALError())
    {
        GetLogger()->Debug("Could not set sound volume to '%f'. Code: %d\n", vol, GetOpenALErrorCode());
        return false;
    }
    return true;
}

float CChannel::GetVolume()
{
    ALfloat vol;
    if (!m_ready || m_buffer == nullptr)
    {
        return 0;
    }

    alGetSourcef(m_source, AL_GAIN, &vol);
    if (CheckOpenALError())
    {
        GetLogger()->Debug("Could not get sound volume. Code: %d\n", GetOpenALErrorCode());
        return 0;
    }

    return vol;
}

void CChannel::SetVolumeAtrib(float volume)
{
    m_volume = volume;
}

float CChannel::GetVolumeAtrib()
{
    return m_volume;
}

int CChannel::GetPriority()
{
    return m_priority;
}

void CChannel::SetPriority(int pri)
{
    m_priority = pri;
}

void CChannel::SetStartAmplitude(float gain)
{
    m_startAmplitude = gain;
}

void CChannel::SetStartFrequency(float freq)
{
    m_startFrequency = freq;
}

void CChannel::SetChangeFrequency(float freq)
{
    m_changeFrequency = freq;
}

float CChannel::GetStartAmplitude()
{
    return m_startAmplitude;
}

float CChannel::GetStartFrequency()
{
    return m_startFrequency;
}

float CChannel::GetChangeFrequency()
{
    return m_changeFrequency;
}

float CChannel::GetInitFrequency()
{
    return m_initFrequency;
}

void CChannel::AddOper(SoundOper oper)
{
    m_oper.push_back(oper);
}

void CChannel::ResetOper()
{
    m_oper.clear();
}

SoundType CChannel::GetSoundType()
{
    if (!m_ready || m_buffer == nullptr)
    {
        return SOUND_NONE;
    }

    return m_buffer->GetSoundType();
}

bool CChannel::SetBuffer(CBuffer *buffer)
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
    if (CheckOpenALError())
    {
        GetLogger()->Warn("Could not set sound buffer. Code: %d\n", GetOpenALErrorCode());
        return false;
    }
    m_initFrequency = GetFrequency();
    return true;
}

bool CChannel::IsPlaying()
{
    ALint status;
    if (!m_ready || m_buffer == nullptr)
    {
        return false;
    }

    alGetSourcei(m_source, AL_SOURCE_STATE, &status);
    if (CheckOpenALError())
    {
        GetLogger()->Warn("Could not get sound status. Code: %d\n", GetOpenALErrorCode());
        return false;
    }

    return status == AL_PLAYING;
}

bool CChannel::IsReady()
{
    return m_ready;
}

bool CChannel::IsLoaded()
{
    return m_buffer != nullptr;
}

bool CChannel::Stop()
{
    if (!m_ready || m_buffer == nullptr)
    {
        return false;
    }

    alSourceStop(m_source);
    if (CheckOpenALError())
    {
        GetLogger()->Warn("Could not stop sound. Code: %d\n", GetOpenALErrorCode());
        return false;
    }
    return true;
}

float CChannel::GetCurrentTime()
{
    if (!m_ready || m_buffer == nullptr)
    {
        return 0.0f;
    }

    ALfloat current;
    alGetSourcef(m_source, AL_SEC_OFFSET, &current);
    if (CheckOpenALError())
    {
        GetLogger()->Warn("Could not get source current play time. Code: %d\n", GetOpenALErrorCode());
        return 0.0f;
    }
    return current;
}

void CChannel::SetCurrentTime(float current)
{
    if (!m_ready || m_buffer == nullptr)
    {
        return;
    }

    alSourcef(m_source, AL_SEC_OFFSET, current);
    if (CheckOpenALError())
    {
        GetLogger()->Warn("Could not get source current play time. Code: %d\n", GetOpenALErrorCode());
    }
}

float CChannel::GetDuration()
{
    if (!m_ready || m_buffer == nullptr)
    {
        return 0.0f;
    }

    return m_buffer->GetDuration();
}

bool CChannel::HasEnvelope()
{
    return m_oper.size() > 0;
}

SoundOper& CChannel::GetEnvelope()
{
    return m_oper.front();
}

void CChannel::PopEnvelope()
{
    m_oper.pop_front();
}

void CChannel::SetLoop(bool loop)
{
    m_loop = loop;
}

void CChannel::Mute(bool mute)
{
    m_mute = mute;
}

bool CChannel::IsMuted()
{
    return m_mute;
}

void CChannel::Reset()
{
    m_id++;
}

int CChannel::GetId()
{
    return m_id;
}

