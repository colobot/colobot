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

#include "sound/sound.h"

#include "common/logger.h"

#include "math/vector.h"

#include <iostream>
#include <iomanip>
#include <sstream>


CSoundInterface::CSoundInterface()
{
}

CSoundInterface::~CSoundInterface()
{
}

bool CSoundInterface::Create()
{
    return true;
}

void CSoundInterface::CacheAll()
{
    for ( int i = 0; i < SOUND_MAX; i++ )
    {
        std::stringstream filename;
        filename << "sounds/sound" << std::setfill('0') << std::setw(3) << i << ".wav";
        if ( !Cache(static_cast<SoundType>(i), filename.str()) )
            GetLogger()->Warn("Unable to load audio: %s\n", filename.str().c_str());
    }
}

bool CSoundInterface::Cache(SoundType sound, const std::string &file)
{
    return true;
}

void CSoundInterface::CacheMusic(const std::string &file)
{
}

bool CSoundInterface::IsCached(SoundType sound)
{
    return false;
}

bool CSoundInterface::IsCachedMusic(const std::string& file)
{
    return false;
}

bool CSoundInterface::GetEnable()
{
    return true;
}

void CSoundInterface::SetAudioVolume(int volume)
{
}

int CSoundInterface::GetAudioVolume()
{
    return 0;
}

void CSoundInterface::SetMusicVolume(int volume)
{
}

int CSoundInterface::GetMusicVolume()
{
    return 0;
}

void CSoundInterface::SetListener(const Math::Vector &eye, const Math::Vector &lookat)
{
}

void CSoundInterface::FrameMove(float rTime)
{
}

int CSoundInterface::Play(SoundType sound, float amplitude, float frequency, bool loop)
{
    return 0;
}

int CSoundInterface::Play(SoundType sound, const Math::Vector &pos, float amplitude, float frequency, bool loop)
{
    return 0;
}

bool CSoundInterface::FlushEnvelope(int channel)
{
    return true;
}

bool CSoundInterface::AddEnvelope(int channel, float amplitude, float frequency, float time, SoundNext oper)
{
    return true;
}

bool CSoundInterface::Position(int channel, const Math::Vector &pos)
{
    return true;
}

bool CSoundInterface::Frequency(int channel, float frequency)
{
    return true;
}

bool CSoundInterface::Stop(int channel)
{
    return true;
}

bool CSoundInterface::StopAll()
{
    return true;
}

bool CSoundInterface::MuteAll(bool mute)
{
    return true;
}

void CSoundInterface::PlayMusic(const std::string &filename, bool repeat, float fadeTime)
{
}

void CSoundInterface::StopMusic(float fadeTime)
{
}

bool CSoundInterface::IsPlayingMusic()
{
    return false;
}

void CSoundInterface::PlayPauseMusic(const std::string &filename, bool repeat)
{
}

void CSoundInterface::StopPauseMusic()
{
}
