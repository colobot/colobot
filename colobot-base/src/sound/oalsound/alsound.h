/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2023, Daniel Roux, EPSITEC SA & TerranovaTeam
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
 * \file alsound.h
 * \brief OpenAL implementation of sound system
 */

#pragma once

#include "sound/sound.h"

#include "common/thread/worker_thread.h"

#include "sound/oalsound/buffer.h"
#include "sound/oalsound/channel.h"
#include "sound/oalsound/check.h"

#include <map>
#include <memory>
#include <string>
#include <list>
#include <filesystem>

#include <al.h>


struct OldMusic
{
    OldMusic() = default;

    OldMusic(const OldMusic&) = delete;
    OldMusic& operator=(const OldMusic&) = delete;

    // Workaround for MSVC2013
    OldMusic(OldMusic&& other)
      : music(std::move(other.music)),
        fadeTime(std::move(other.fadeTime)),
        currentTime(std::move(other.currentTime))
    {}

    OldMusic& operator=(OldMusic&& other)
    {
        music = std::move(other.music);
        fadeTime = std::move(other.fadeTime);
        currentTime = std::move(other.currentTime);
        return *this;
    }

    std::unique_ptr<CChannel> music;
    float fadeTime = 0.0f;
    float currentTime = 0.0f;

    inline friend bool operator<(const OldMusic & l, const OldMusic & r)
    {
        return l.currentTime < r.currentTime;
    }

    inline friend bool operator==(const OldMusic & l, const OldMusic & r)
    {
        return l.currentTime == r.currentTime;
    }
};

class CALSound : public CSoundInterface
{
public:
    CALSound();
    ~CALSound();

    bool Create() override;
    void Reset() override;
    bool Cache(SoundType, const std::filesystem::path &) override;
    void CacheMusic(const std::filesystem::path &) override;
    bool IsCached(SoundType) override;
    bool IsCachedMusic(const std::filesystem::path &) override;

    bool GetEnable() override;
    void SetAudioVolume(int volume) override;
    int GetAudioVolume() override;
    void SetMusicVolume(int volume) override;
    int GetMusicVolume() override;

    void SetListener(const glm::vec3 &eye, const glm::vec3 &lookat) override;
    void FrameMove(float rTime) override;

    int Play(SoundType sound, float amplitude=1.0f, float frequency=1.0f, bool loop = false) override;
    int Play(SoundType sound, const glm::vec3 &pos, float amplitude=1.0f, float frequency=1.0f, bool loop = false) override;
    bool FlushEnvelope(int channel) override;
    bool AddEnvelope(int channel, float amplitude, float frequency, float time, SoundNext oper) override;
    bool Position(int channel, const glm::vec3 &pos) override;
    bool Frequency(int channel, float frequency) override;
    bool Stop(int channel) override;
    bool StopAll() override;
    bool MuteAll(bool mute) override;

    void PlayMusic(const std::filesystem::path& filename, bool repeat, float fadeTime = 2.0f) override;
    void StopMusic(float fadeTime=2.0f) override;
    bool IsPlayingMusic() override;
    void PlayPauseMusic(const std::filesystem::path& filename, bool repeat) override;
    void StopPauseMusic() override;

private:
    void CleanUp();
    int Play(SoundType sound, const glm::vec3 &pos, bool relativeToListener, float amplitude, float frequency, bool loop);
    int GetPriority(SoundType);
    bool SearchFreeBuffer(SoundType sound, int &channel, bool &alreadyLoaded);
    bool CheckChannel(int &channel);

    bool m_enabled;
    float m_audioVolume;
    float m_musicVolume;
    unsigned int m_channelsLimit;
    ALCdevice* m_device;
    ALCcontext* m_context;
    std::map<SoundType, std::unique_ptr<CBuffer>> m_sounds;
    std::map<std::filesystem::path, std::unique_ptr<CBuffer>> m_music;
    std::map<int, std::unique_ptr<CChannel>> m_channels;
    std::unique_ptr<CChannel> m_currentMusic;
    std::list<OldMusic> m_oldMusic;
    OldMusic m_previousMusic;
    glm::vec3 m_eye;
    glm::vec3 m_lookat;
    CWorkerThread m_thread;
};
