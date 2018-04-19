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

/**
 * \file channel.h
 * \brief OpenAL channel
 */

#pragma once

#include "math/vector.h"

#include "sound/sound.h"

#include "sound/oalsound/check.h"

#include <string>
#include <deque>
#include <cassert>

#include <al.h>
#include <alc.h>

class CBuffer;

struct SoundOper
{
    float finalAmplitude = 0.0f;
    float finalFrequency = 0.0f;
    float totalTime = 0.0f;
    float currentTime = 0.0f;
    SoundNext nextOper = SOPER_CONTINUE;
};


class CChannel
{
public:
    CChannel();
    ~CChannel();

    bool Play();
    bool Pause();
    bool Stop();

    bool SetPosition(const Math::Vector &pos);

    bool SetFrequency(float freq);
    float GetFrequency();

    float GetCurrentTime();
    void SetCurrentTime(float current);
    float GetDuration();

    bool SetVolume(float vol);
    float GetVolume();
    void SetVolumeAtrib(float volume);
    float GetVolumeAtrib();

    bool IsPlaying();
    bool IsReady();
    bool IsLoaded();

    bool SetBuffer(CBuffer *buffer);

    bool HasEnvelope();
    SoundOper& GetEnvelope();
    void PopEnvelope();

    int GetPriority();
    void SetPriority(int pri);

    void SetStartAmplitude(float gain);
    void SetStartFrequency(float freq);
    void SetChangeFrequency(float freq);

    float GetStartAmplitude();
    float GetStartFrequency();
    float GetChangeFrequency();
    float GetInitFrequency();

    void AddOper(SoundOper oper);
    void ResetOper();
    SoundType GetSoundType();
    void SetLoop(bool loop);
    void Mute(bool mute);
    bool IsMuted();

    void Reset();
    int GetId();

private:
    CBuffer *m_buffer;
    ALuint m_source;

    int m_priority;
    int m_id;
    float m_startAmplitude;
    float m_startFrequency;
    float m_changeFrequency;
    float m_initFrequency;
    float m_volume;
    std::deque<SoundOper> m_oper;
    bool m_ready;
    bool m_loop;
    bool m_mute;
    Math::Vector m_position;
};

