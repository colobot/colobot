// * This file is part of the COLOBOT source code
// * Copyright (C) 2001-2008, Daniel ROUX & EPSITEC SA, www.epsitec.ch
// * Copyright (C) 2012-2013, Polish Portal of Colobot (PPC)
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

#include "sound/sound.h"

#include "math/vector.h"

#include "common/logger.h"

#include <iostream>
#include <iomanip>
#include <sstream>

#include <boost/filesystem.hpp>


CSoundInterface::CSoundInterface()
{
}

CSoundInterface::~CSoundInterface()
{
}

bool CSoundInterface::Create(bool b3D)
{
    return true;
}

void CSoundInterface::CacheAll(std::string path)
{
    for ( int i = 1; i < SOUND_MAX; i++ )
    {
        std::stringstream filename;
        filename << path << "/sound" << std::setfill('0') << std::setw(3) << i << ".wav";
        if ( !Cache(static_cast<Sound>(i), filename.str()) )
            GetLogger()->Warn("Unable to load audio: %s\n", filename.str().c_str());
    }
}

void CSoundInterface::AddMusicFiles(std::string path)
{
    m_soundPath = path;
    CacheMusic("Intro1.ogg");
    CacheMusic("Intro2.ogg");
    CacheMusic("music010.ogg");
    CacheMusic("music011.ogg");
}

bool CSoundInterface::Cache(Sound bSound, std::string bFile)
{
    return true;
}

bool CSoundInterface::CacheMusic(std::string bFile)
{
    return true;
}

bool CSoundInterface::GetEnable()
{
    return true;
}

void CSoundInterface::SetSound3D(bool bMode)
{
}

bool CSoundInterface::GetSound3D()
{
    return true;
}

bool CSoundInterface::GetSound3DCap()
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

void CSoundInterface::SetListener(Math::Vector eye, Math::Vector lookat)
{
}

void CSoundInterface::FrameMove(float rTime)
{
}

int CSoundInterface::Play(Sound sound, float amplitude, float frequency, bool bLoop)
{
    return 0;
}

int CSoundInterface::Play(Sound sound, Math::Vector pos, float amplitude, float frequency, bool bLoop)
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

bool CSoundInterface::Position(int channel, Math::Vector pos)
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

bool CSoundInterface::MuteAll(bool bMute)
{
    return true;
}

bool CSoundInterface::PlayMusic(int rank, bool bRepeat)
{
    return true;
}

bool CSoundInterface::PlayMusic(std::string filename, bool bRepeat)
{
    return true;
}

bool CSoundInterface::RestartMusic()
{
    return true;
}

void CSoundInterface::SuspendMusic()
{
}

void CSoundInterface::StopMusic()
{
}

bool CSoundInterface::IsPlayingMusic()
{
    return true;
}

