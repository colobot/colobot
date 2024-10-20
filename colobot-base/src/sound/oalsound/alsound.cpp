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


#include "sound/oalsound/alsound.h"
#include "common/stringutils.h"

#include <algorithm>
#include <iomanip>


CALSound::CALSound()
    : m_enabled(false),
      m_audioVolume(1.0f),
      m_musicVolume(1.0f),
      m_channelsLimit(2048),
      m_device{},
      m_context{}
{
}

CALSound::~CALSound()
{
    CleanUp();
}

void CALSound::CleanUp()
{
    if (m_enabled)
    {
        GetLogger()->Info("Unloading files and closing device...");
        Reset();

        alcDestroyContext(m_context);
        alcCloseDevice(m_device);
    }
}

bool CALSound::Create()
{
    CleanUp();

    if (m_enabled)
        return true;

    GetLogger()->Info("Opening audio device...");
    m_device = alcOpenDevice(nullptr);
    if (!m_device)
    {
        GetLogger()->Error("Could not open audio device!");
        return false;
    }

    m_context = alcCreateContext(m_device, nullptr);
    if (!m_context)
    {
        GetLogger()->Error("Could not create audio context!");
        return false;
    }
    alcMakeContextCurrent(m_context);
    alListenerf(AL_GAIN, m_audioVolume);
    alDistanceModel(AL_LINEAR_DISTANCE_CLAMPED);

    GetLogger()->Info("Done.");
    m_enabled = true;
    return true;
}

void CALSound::Reset()
{
    StopAll();
    StopMusic();

    m_channels.clear();

    m_currentMusic.reset();

    m_oldMusic.clear();

    m_previousMusic.music.reset();

    m_sounds.clear();

    m_music.clear();
}

bool CALSound::GetEnable()
{
    return m_enabled;
}

void CALSound::SetAudioVolume(int volume)
{
    m_audioVolume = static_cast<float>(volume) / MAXVOLUME;
}

int CALSound::GetAudioVolume()
{
    if ( !m_enabled )
        return 0;

    return m_audioVolume * MAXVOLUME;
}

void CALSound::SetMusicVolume(int volume)
{
    m_musicVolume = static_cast<float>(volume) / MAXVOLUME;
    if (m_currentMusic)
    {
        m_currentMusic->SetVolume(m_musicVolume);
    }
}

int CALSound::GetMusicVolume()
{
    if ( !m_enabled )
        return 0.0f;

    return m_musicVolume * MAXVOLUME;
}

bool CALSound::Cache(SoundType sound, const std::filesystem::path &filename)
{
    auto buffer = std::make_unique<CBuffer>();
    if (buffer->LoadFromFile(filename, sound))
    {
        m_sounds[sound] = std::move(buffer);
        return true;
    }
    return false;
}

void CALSound::CacheMusic(const std::filesystem::path &filename)
{
    m_thread.Start([this, filename]()
    {
        if (m_music.find(filename) == m_music.end())
        {
            auto buffer = std::make_unique<CBuffer>();
            if (buffer->LoadFromFile(filename, static_cast<SoundType>(-1)))
            {
                m_music[filename] = std::move(buffer);
            }
        }
    });
}

bool CALSound::IsCached(SoundType sound)
{
    return m_sounds.find(sound) != m_sounds.end();
}

bool CALSound::IsCachedMusic(const std::filesystem::path &filename)
{
    return m_music.find(filename) != m_music.end();
}

int CALSound::GetPriority(SoundType sound)
{
    if ( sound == SOUND_FLYh   ||
        sound == SOUND_FLY    ||
        sound == SOUND_MOTORw ||
        sound == SOUND_MOTORt ||
        sound == SOUND_MOTORr ||
        sound == SOUND_MOTORs ||
        sound == SOUND_SLIDE  ||
        sound == SOUND_ERROR  )
    {
        return 30;
    }

    if ( sound == SOUND_CONVERT  ||
        sound == SOUND_ENERGY   ||
        sound == SOUND_DERRICK  ||
        sound == SOUND_STATION  ||
        sound == SOUND_REPAIR   ||
        sound == SOUND_RESEARCH ||
        sound == SOUND_BURN     ||
        sound == SOUND_BUILD    ||
        sound == SOUND_TREMBLE  ||
        sound == SOUND_NUCLEAR  ||
        sound == SOUND_EXPLO    ||
        sound == SOUND_EXPLOl   ||
        sound == SOUND_EXPLOlp  ||
        sound == SOUND_EXPLOp   ||
        sound == SOUND_EXPLOi   )
    {
        return 20;
    }

    if ( sound == SOUND_BLUP    ||
        sound == SOUND_INSECTs ||
        sound == SOUND_INSECTa ||
        sound == SOUND_INSECTb ||
        sound == SOUND_INSECTw ||
        sound == SOUND_INSECTm ||
        sound == SOUND_PSHHH   ||
        sound == SOUND_EGG     )
    {
        return 0;
    }

    return 10;
}

bool CALSound::SearchFreeBuffer(SoundType sound, int &channel, bool &alreadyLoaded)
{
    int priority = GetPriority(sound);

    // Seeks a channel used which sound is stopped.
    for (auto& it : m_channels)
    {
        if (it.second->IsPlaying())
        {
            continue;
        }
        if (it.second->GetSoundType() != sound)
        {
            continue;
        }

        it.second->SetPriority(priority);
        it.second->Reset();
        channel = it.first;
        alreadyLoaded = it.second->IsLoaded();
        return true;
    }

    // just add a new channel if we dont have any
    if (m_channels.size() == 0)
    {
        auto chn = std::make_unique<CChannel>();
        // check if we channel ready to play music, if not report error
        if (chn->IsReady())
        {
            chn->SetPriority(priority);
            chn->Reset();
            channel = 1;
            m_channels[channel] = std::move(chn);
            alreadyLoaded = false;
            return true;
        }
        GetLogger()->Error("Could not open channel to play sound!");
        return false;
    }

    // Assigns new channel within limit
    if (m_channels.size() < m_channelsLimit)
    {
        auto it = m_channels.end();
        it--;
        int i = (*it).first;
        while (++i)
        {
            if (m_channels.find(i) == m_channels.end())
            {
                auto chn = std::make_unique<CChannel>();
                // check if channel is ready to play music, if not destroy it and seek free one
                if (chn->IsReady())
                {
                    chn->SetPriority(priority);
                    chn->Reset();
                    m_channels[i] = std::move(chn);
                    channel = i;
                    alreadyLoaded = false;
                    return true;
                }
                GetLogger()->Debug("Could not open additional channel to play sound!");
                break;
            }
        }
    }

    int lowerOrEqual = -1;
    for (auto& it : m_channels)
    {
        if (it.second->GetPriority() < priority)
        {
            GetLogger()->Debug("Sound channel with lower priority will be reused.");
            channel = it.first;
            it.second->Reset();
            return true;
        }
        if (it.second->GetPriority() <= priority)
            lowerOrEqual = it.first;
    }

    if (lowerOrEqual != -1)
    {
        channel = lowerOrEqual;
        m_channels[channel]->Reset();
        GetLogger()->Debug("Sound channel with lower or equal priority will be reused.");
        return true;
    }

    GetLogger()->Debug("Could not find free buffer to use.");
    return false;
}

int CALSound::Play(SoundType sound, float amplitude, float frequency, bool loop)
{
    return Play(sound, glm::vec3{0, 0, 0}, true, amplitude, frequency, loop);
}

int CALSound::Play(SoundType sound, const glm::vec3 &pos, float amplitude, float frequency, bool loop)
{
    return Play(sound, pos, false, amplitude, frequency, loop);
}

int CALSound::Play(SoundType sound, const glm::vec3 &pos, bool relativeToListener, float amplitude, float frequency, bool loop)
{
    if (!m_enabled)
    {
        return -1;
    }
    if (m_sounds.find(sound) == m_sounds.end())
    {
        GetLogger()->Debug("Sound %% was not loaded!", sound);
        return -1;
    }

    int channel;
    bool alreadyLoaded = false;
    if (!SearchFreeBuffer(sound, channel, alreadyLoaded))
    {
        return -1;
    }

    if (!alreadyLoaded)
    {
        if (!m_channels[channel]->SetBuffer(m_sounds[sound].get()))
        {
            m_channels[channel]->SetBuffer(nullptr);
            return -1;
        }
    }

    CChannel* chn = m_channels[channel].get();

    chn->SetPosition(pos, relativeToListener);
    chn->SetVolumeAtrib(1.0f);

    // setting initial values
    chn->SetStartAmplitude(amplitude);
    chn->SetStartFrequency(frequency);
    chn->SetChangeFrequency(1.0f);
    chn->ResetOper();
    chn->SetFrequency(frequency);
    chn->SetVolume(powf(amplitude * chn->GetVolumeAtrib(), 0.2f) * m_audioVolume);
    chn->SetLoop(loop);
    chn->Mute(false);

    if (!chn->Play())
    {
        m_channelsLimit = m_channels.size() - 1;
        GetLogger()->Debug("Changing channel limit to %%.", m_channelsLimit);
        m_channels.erase(channel);

        return -1;
    }

    return channel | ((chn->GetId() & 0xffff) << 16);
}

bool CALSound::FlushEnvelope(int channel)
{
    if (!CheckChannel(channel))
    {
        return false;
    }

    m_channels[channel]->ResetOper();
    return true;
}

bool CALSound::AddEnvelope(int channel, float amplitude, float frequency, float time, SoundNext oper)
{
    if (!CheckChannel(channel))
    {
        return false;
    }

    SoundOper op;
    op.finalAmplitude = amplitude;
    op.finalFrequency = frequency;
    op.totalTime = time;
    op.nextOper = oper;
    op.currentTime = 0.0f;
    m_channels[channel]->AddOper(op);

    return true;
}

bool CALSound::Position(int channel, const glm::vec3 &pos)
{
    if (!CheckChannel(channel))
    {
        return false;
    }

    m_channels[channel]->SetPosition(pos);
    return true;
}

bool CALSound::Frequency(int channel, float frequency)
{
    if (!CheckChannel(channel))
    {
        return false;
    }

    m_channels[channel]->SetFrequency(frequency * m_channels[channel]->GetInitFrequency());
    m_channels[channel]->SetChangeFrequency(frequency);
    return true;
}

bool CALSound::Stop(int channel)
{
    if (!CheckChannel(channel))
    {
        return false;
    }

    m_channels[channel]->Stop();
    m_channels[channel]->ResetOper();

    return true;
}

bool CALSound::StopAll()
{
    if (!m_enabled)
    {
        return false;
    }

    for (auto& channel : m_channels)
    {
        channel.second->Stop();
        channel.second->ResetOper();
    }

    return true;
}

bool CALSound::MuteAll(bool mute)
{
    if (!m_enabled)
    {
        return false;
    }

    for (auto& it : m_channels)
    {
        if (it.second->IsPlaying())
        {
            it.second->Mute(mute);
        }
    }

    return true;
}

void CALSound::FrameMove(float rTime)
{
    if (!m_enabled)
    {
        return;
    }

    float progress;
    float volume, frequency;
    for (auto& it : m_channels)
    {
        if (!it.second->IsPlaying())
        {
            continue;
        }
        if (it.second->IsMuted())
        {
            it.second->SetVolume(0.0f);
            continue;
        }

        if (!it.second->HasEnvelope())
            continue;

        SoundOper &oper = it.second->GetEnvelope();
        oper.currentTime += rTime;
        progress = oper.currentTime / oper.totalTime;
        progress = std::min(progress, 1.0f);

        // setting volume
        volume = progress * (oper.finalAmplitude - it.second->GetStartAmplitude());
        volume = volume + it.second->GetStartAmplitude();
        it.second->SetVolume(powf(volume * it.second->GetVolumeAtrib(), 0.2f) * m_audioVolume);

        // setting frequency
        frequency = progress;
        frequency *= oper.finalFrequency - it.second->GetStartFrequency();
        frequency += it.second->GetStartFrequency();
        frequency *= it.second->GetChangeFrequency();
        frequency = (frequency * it.second->GetInitFrequency());
        it.second->SetFrequency(frequency);

        if (oper.totalTime <= oper.currentTime)
        {
            if (oper.nextOper == SOPER_LOOP)
            {
                oper.currentTime = 0.0f;
                it.second->Play();
            }
            else
            {
                it.second->SetStartAmplitude(oper.finalAmplitude);
                it.second->SetStartFrequency(oper.finalFrequency);
                if (oper.nextOper == SOPER_STOP)
                {
                    it.second->Stop();
                }

                it.second->PopEnvelope();
            }
        }
    }

    auto it = m_oldMusic.begin();
    while (it != m_oldMusic.end())
    {
        if (it->currentTime >= it->fadeTime)
        {
            it = m_oldMusic.erase(it);
        }
        else
        {
            it->currentTime += rTime;
            it->music->SetVolume(((it->fadeTime-it->currentTime) / it->fadeTime) * m_musicVolume);
            ++it;
        }
    }

    if (m_previousMusic.fadeTime > 0.0f)
    {
        if (m_previousMusic.currentTime >= m_previousMusic.fadeTime)
        {
            m_previousMusic.music->Pause();
        }
        else
        {
            m_previousMusic.currentTime += rTime;
            m_previousMusic.music->SetVolume(((m_previousMusic.fadeTime-m_previousMusic.currentTime) / m_previousMusic.fadeTime) * m_musicVolume);
        }
    }
}

void CALSound::SetListener(const glm::vec3 &eye, const glm::vec3 &lookat)
{
    m_eye = eye;
    m_lookat = lookat;
    glm::vec3 forward = glm::normalize(lookat - eye);
    float orientation[] = {forward.x, forward.y, forward.z, 0.f, -1.0f, 0.0f};

    alListener3f(AL_POSITION, eye.x, eye.y, eye.z);
    alListenerfv(AL_ORIENTATION, orientation);
}

void CALSound::PlayMusic(const std::filesystem::path& filename, bool repeat, float fadeTime)
{
    if (!m_enabled)
    {
        return;
    }

    m_thread.Start([this, filename, repeat, fadeTime]()
    {
        CBuffer* buffer = nullptr;

        // check if we have music in cache
        if (m_music.find(filename) == m_music.end())
        {
            GetLogger()->Debug("Music %% was not cached!", filename);

            auto newBuffer = std::make_unique<CBuffer>();
            buffer = newBuffer.get();
            if (!newBuffer->LoadFromFile(filename, static_cast<SoundType>(-1)))
            {
                return;
            }
            m_music[filename] = std::move(newBuffer);
        }
        else
        {
            GetLogger()->Debug("Music loaded from cache");
            buffer = m_music[filename].get();
        }

        if (m_currentMusic)
        {
            OldMusic old;
            old.music = std::move(m_currentMusic);
            old.fadeTime = fadeTime;
            old.currentTime = 0.0f;
            m_oldMusic.push_back(std::move(old));
        }

        m_currentMusic = std::make_unique<CChannel>();
        m_currentMusic->SetBuffer(buffer);
        m_currentMusic->SetVolume(m_musicVolume);
        m_currentMusic->SetLoop(repeat);
        m_currentMusic->Play();
    });
}

void CALSound::PlayPauseMusic(const std::filesystem::path& filename, bool repeat)
{
    if (m_previousMusic.fadeTime > 0.0f)
    {
        if (m_currentMusic != nullptr)
        {
            OldMusic old;
            old.music = std::move(m_currentMusic);
            old.fadeTime = 2.0f;
            old.currentTime = 0.0f;
            m_oldMusic.push_back(std::move(old));
        }
    }
    else
    {
        if (m_currentMusic != nullptr)
        {
            m_previousMusic.music = std::move(m_currentMusic);
            m_previousMusic.fadeTime = 2.0f;
            m_previousMusic.currentTime = 0.0f;
        }
    }
    PlayMusic(filename, repeat);
}

void CALSound::StopPauseMusic()
{
    if (m_previousMusic.fadeTime > 0.0f)
    {
        StopMusic();

        m_currentMusic = std::move(m_previousMusic.music);
        if (m_currentMusic != nullptr)
        {
            m_currentMusic->SetVolume(m_musicVolume);
            if (m_previousMusic.currentTime >= m_previousMusic.fadeTime)
            {
                m_currentMusic->Play();
            }
        }
        m_previousMusic.fadeTime = 0.0f;
    }
}

void CALSound::StopMusic(float fadeTime)
{
    if (!m_enabled || m_currentMusic == nullptr)
    {
        return;
    }

    OldMusic old;
    old.music = std::move(m_currentMusic);
    old.fadeTime = fadeTime;
    old.currentTime = 0.0f;
    m_oldMusic.push_back(std::move(old));
}

bool CALSound::IsPlayingMusic()
{
    if (!m_enabled || m_currentMusic == nullptr)
    {
        return false;
    }

    return m_currentMusic->IsPlaying();
}

bool CALSound::CheckChannel(int &channel)
{
    int id = (channel >> 16) & 0xffff;
    channel &= 0xffff;

    if (!m_enabled)
    {
        return false;
    }

    if (m_channels.find(channel) == m_channels.end())
    {
        return false;
    }

    if  (m_audioVolume == 0)
    {
        return false;
    }

    if (m_channels[channel]->GetId() != id)
    {
        return false;
    }

    return true;
}
