// * This file is part of the COLOBOT source code
// * Copyright (C) 2001-2008, Daniel ROUX & EPSITEC SA, www.epsitec.ch
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
 * \file alsound.h
 * \brief OpenAL implementation of sound system
 */

#pragma once

#include "common/logger.h"
#include "sound/sound.h"

#include "sound/oalsound/buffer.h"
#include "sound/oalsound/channel.h"
#include "sound/oalsound/check.h"

#include <map>
#include <string>

#include <al.h>


class ALSound : public CSoundInterface
{
public:
    ALSound();
    ~ALSound();

    bool Create(bool b3D);
    bool Cache(Sound, std::string);
    bool CacheMusic(std::string);

    bool GetEnable();

    void SetSound3D(bool bMode);
    bool GetSound3D();
    bool GetSound3DCap();

    void SetAudioVolume(int volume);
    int GetAudioVolume();
    void SetMusicVolume(int volume);
    int GetMusicVolume();

    void SetListener(Math::Vector eye, Math::Vector lookat);
    void FrameMove(float rTime);

    int Play(Sound sound, float amplitude=1.0f, float frequency=1.0f, bool bLoop = false);
    int Play(Sound sound, Math::Vector pos, float amplitude=1.0f, float frequency=1.0f, bool bLoop = false);
    bool FlushEnvelope(int channel);
    bool AddEnvelope(int channel, float amplitude, float frequency, float time, SoundNext oper);
    bool Position(int channel, Math::Vector pos);
    bool Frequency(int channel, float frequency);
    bool Stop(int channel);
    bool StopAll();
    bool MuteAll(bool bMute);

    bool PlayMusic(int rank, bool bRepeat);
    bool PlayMusic(std::string filename, bool bRepeat);
    bool RestartMusic();
    void SuspendMusic();
    void StopMusic();
    bool IsPlayingMusic();

    // plugin interface
    std::string PluginName();
    int PluginVersion();
    void InstallPlugin();
    bool UninstallPlugin(std::string &);

private:
    void CleanUp();
    int GetPriority(Sound);
    bool SearchFreeBuffer(Sound sound, int &channel, bool &bAlreadyLoaded);
    void ComputeVolumePan2D(int channel, Math::Vector &pos);

    bool m_enabled;
    bool m_3D;
    float m_audioVolume;
    float m_musicVolume;
    ALCdevice* m_device;
    ALCcontext* m_context;
    std::map<Sound, Buffer*> m_sounds;
    std::map<std::string, Buffer*> m_music;
    std::map<int, Channel*> m_channels;
    Channel *m_currentMusic;
    Math::Vector m_eye;
    Math::Vector m_lookat;
};

