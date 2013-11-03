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

/**
 * \file channel.h
 * \brief OpenAL channel
 */

#pragma once

#include "sound/sound.h"

#include "sound/oalsound/buffer.h"
#include "sound/oalsound/check.h"

#include <string>
#include <deque>
#include <cassert>

#include <al.h>
#include <alc.h>

struct SoundOper
{
    float finalAmplitude;
    float finalFrequency;
    float totalTime;
    float currentTime;
    SoundNext nextOper;
};


class Channel
{
public:
    Channel();
    ~Channel();

    bool Play();
    bool Stop();

    bool SetPan(Math::Vector);
    void SetPosition(Math::Vector);
    Math::Vector GetPosition();

    bool SetFrequency(float);
    float GetFrequency();

    float GetCurrentTime();
    void SetCurrentTime(float);
    float GetDuration();

    bool SetVolume(float);
    float GetVolume();
    void SetVolumeAtrib(float);
    float GetVolumeAtrib();

    bool IsPlaying();
    bool IsReady();
    bool IsLoaded();

    bool SetBuffer(Buffer *);
    bool FreeBuffer();

    bool HasEnvelope();
    SoundOper& GetEnvelope();
    void PopEnvelope();

    int GetPriority();
    void SetPriority(int);

    void SetStartAmplitude(float);
    void SetStartFrequency(float);
    void SetChangeFrequency(float);

    float GetStartAmplitude();
    float GetStartFrequency();
    float GetChangeFrequency();
    float GetInitFrequency();

    void AddOper(SoundOper);
    void ResetOper();
    Sound GetSoundType();
    void SetLoop(bool);
    void Mute(bool);
    bool IsMuted();

private:
    Buffer *m_buffer;
    ALuint m_source;

    int m_priority;
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

