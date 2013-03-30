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

// channel.h

#pragma once

#include <string>
#include <deque>
#include <cassert>

#include <AL/al.h>
#include <AL/alc.h>

#include "sound/sound.h"

#include "buffer.h"
#include "check.h"

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
        bool SetPosition(Math::Vector);

        bool SetFrequency(float);
        float GetFrequency();
        bool AdjustFrequency(float);

        float GetCurrentTime();
        void SetCurrentTime(float);
        float GetDuration();

        bool SetVolume(float);
        float GetVolume();
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
        
    private:
        Buffer *mBuffer;
        ALuint mSource;

        int mPriority;
        float mStartAmplitude;
        float mStartFrequency;
        float mChangeFrequency;
        float mInitFrequency;
        std::deque<SoundOper> mOper;
        bool mReady;
        bool mLoop;
};
