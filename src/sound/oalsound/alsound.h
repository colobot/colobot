/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2014, Daniel Roux, EPSITEC SA & TerranovaTeam
 * http://epsiteс.ch; http://colobot.info; http://github.com/colobot
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
#include <list>

#include <al.h>


struct OldMusic {
    Channel* music;
    float fadeTime;
    float currentTime;
};

class ALSound : public CSoundInterface
{
public:
    ALSound();
    ~ALSound();

    bool Create() OVERRIDE;
    bool Cache(Sound, const std::string &) OVERRIDE;
    bool CacheMusic(const std::string &) OVERRIDE;
    bool IsCached(Sound) OVERRIDE;
    bool IsCachedMusic(const std::string &) OVERRIDE;

    bool GetEnable() OVERRIDE;

    void SetAudioVolume(int volume) OVERRIDE;
    int GetAudioVolume() OVERRIDE;
    void SetMusicVolume(int volume) OVERRIDE;
    int GetMusicVolume() OVERRIDE;

    void SetListener(const Math::Vector &eye, const Math::Vector &lookat) OVERRIDE;
    void FrameMove(float rTime) OVERRIDE;

    int Play(Sound sound, float amplitude=1.0f, float frequency=1.0f, bool bLoop = false) OVERRIDE;
    int Play(Sound sound, const Math::Vector &pos, float amplitude=1.0f, float frequency=1.0f, bool bLoop = false) OVERRIDE;
    bool FlushEnvelope(int channel) OVERRIDE;
    bool AddEnvelope(int channel, float amplitude, float frequency, float time, SoundNext oper) OVERRIDE;
    bool Position(int channel, const Math::Vector &pos) OVERRIDE;
    bool Frequency(int channel, float frequency) OVERRIDE;
    bool Stop(int channel) OVERRIDE;
    bool StopAll() OVERRIDE;
    bool MuteAll(bool bMute) OVERRIDE;

    bool PlayMusic(int rank, bool bRepeat, float fadeTime=2.0f) OVERRIDE;
    bool PlayMusic(const std::string &filename, bool bRepeat, float fadeTime=2.0f) OVERRIDE;
    bool RestartMusic() OVERRIDE;
    void SuspendMusic() OVERRIDE;
    void StopMusic(float fadeTime=2.0f) OVERRIDE;
    bool IsPlayingMusic() OVERRIDE;
    bool PlayPauseMusic(const std::string &filename, bool repeat) OVERRIDE;
    void StopPauseMusic() OVERRIDE;

private:
    void CleanUp();
    int GetPriority(Sound);
    bool SearchFreeBuffer(Sound sound, int &channel, bool &bAlreadyLoaded);
    bool CheckChannel(int &channel);

    bool m_enabled;
    float m_audioVolume;
    float m_musicVolume;
    unsigned int m_channels_limit;
    ALCdevice* m_device;
    ALCcontext* m_context;
    std::map<Sound, Buffer*> m_sounds;
    std::map<std::string, Buffer*> m_music;
    std::map<int, Channel*> m_channels;
    Channel *m_currentMusic;
    std::list<OldMusic> m_oldMusic;
    OldMusic m_previousMusic;
    Math::Vector m_eye;
    Math::Vector m_lookat;
};

