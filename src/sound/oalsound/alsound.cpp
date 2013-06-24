// * This file is part of the COLOBOT source code
// * Copyright (C) 2001-2008, Daniel ROUX & EPSITEC SA, www.epsitec.ch
// * Copyright (C) 2012 Polish Portal of Colobot (PPC)
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


#include "sound/oalsound/alsound.h"

#include <algorithm>
#include <iomanip>

#include <boost/filesystem.hpp>

ALSound::ALSound()
{
    m_enabled = false;
    m_3D = false;
    m_audioVolume = 1.0f;
    m_musicVolume = 1.0f;
    m_currentMusic = nullptr;
    m_eye.LoadZero();
    m_lookat.LoadZero();
}


ALSound::~ALSound()
{
    CleanUp();
}


void ALSound::CleanUp()
{
    if (m_enabled)
    {
        GetLogger()->Info("Unloading files and closing device...\n");
        StopAll();
        StopMusic();

        for (auto channel : m_channels)
        {
            delete channel.second;
        }

        if (m_currentMusic)
        {
            delete m_currentMusic;
        }

        for (auto item : m_sounds)
        {
            delete item.second;
        }

        for (auto item : m_music)
        {
            delete item.second;
        }

        m_enabled = false;

        alcDestroyContext(m_context);
        alcCloseDevice(m_device);
    }
}


bool ALSound::Create(bool b3D)
{
    CleanUp();

    if (m_enabled)
        return true;

    GetLogger()->Info("Opening audio device...\n");
    m_device = alcOpenDevice(NULL);
    if (!m_device)
    {
        GetLogger()->Error("Could not open audio device!\n");
        return false;
    }

    m_context = alcCreateContext(m_device, NULL);
    if (!m_context)
    {
        GetLogger()->Error("Could not create audio context!\n");
        return false;
    }
    alcMakeContextCurrent(m_context);
    alListenerf(AL_GAIN, m_audioVolume);
    alDistanceModel(AL_LINEAR_DISTANCE_CLAMPED);

    m_currentMusic = new Channel();
    GetLogger()->Info("Done.\n");
    m_enabled = true;
    return true;
}


void ALSound::SetSound3D(bool bMode)
{
    m_3D = bMode;
}


bool ALSound::GetSound3D()
{
    return m_3D;
}


bool ALSound::GetSound3DCap()
{
    // TODO stub! need to be implemented
    return true;
}


bool ALSound::GetEnable()
{
    return m_enabled;
}


void ALSound::SetAudioVolume(int volume)
{
    m_audioVolume = static_cast<float>(volume) / MAXVOLUME;
}


int ALSound::GetAudioVolume()
{
    if ( !m_enabled )
        return 0;

    return m_audioVolume * MAXVOLUME;
}


void ALSound::SetMusicVolume(int volume)
{
    m_musicVolume = static_cast<float>(volume) / MAXVOLUME;
    if (m_currentMusic)
    {
        m_currentMusic->SetVolume(m_musicVolume);
    }
}


int ALSound::GetMusicVolume()
{
    if ( !m_enabled )
        return 0.0f;

    return m_musicVolume * MAXVOLUME;
}


bool ALSound::Cache(Sound sound, std::string filename)
{
    Buffer *buffer = new Buffer();
    if (buffer->LoadFromFile(filename, sound))
    {
        m_sounds[sound] = buffer;
        return true;
    }
    return false;
}

bool ALSound::CacheMusic(std::string filename)
{
    if (m_music.find(filename) == m_music.end())
    {
        Buffer *buffer = new Buffer();
        std::stringstream file;
        file << m_soundPath << "/" << filename;
        if (buffer->LoadFromFile(file.str(), static_cast<Sound>(-1)))
        {
            m_music[filename] = buffer;
            return true;
        }
    }
    return false;
}

int ALSound::GetPriority(Sound sound)
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


bool ALSound::SearchFreeBuffer(Sound sound, int &channel, bool &bAlreadyLoaded)
{
    int priority = GetPriority(sound);

    // Seeks a channel used which sound is stopped.
    for (auto it : m_channels)
    {
        if (it.second->IsPlaying())
            continue;
        if (it.second->GetSoundType() != sound)
            continue;

        it.second->SetPriority(priority);
        channel = it.first;
        bAlreadyLoaded = it.second->IsLoaded();
        return true;
    }

    // just add a new channel if we dont have any
    if (m_channels.size() == 0)
    {
        Channel *chn = new Channel();
        // check if we channel ready to play music, if not report error
        if (chn->IsReady())
        {
            chn->SetPriority(priority);
            m_channels[1] = chn;
            channel = 1;
            bAlreadyLoaded = false;
            return true;
        }
        delete chn;
        GetLogger()->Error("Could not open channel to play sound!\n");
        return false;
    }

    // Seeks a channel completely free.
    if (m_channels.size() < 64)
    {
        auto it = m_channels.end();
        it--;
        int i = (*it).first;
        while (++i)
        {
            if (m_channels.find(i) == m_channels.end())
            {
                Channel *chn = new Channel();
                // check if channel is ready to play music, if not destroy it and seek free one
                if (chn->IsReady())
                {
                    chn->SetPriority(priority);
                    m_channels[++i] = chn;
                    channel = i;
                    bAlreadyLoaded = false;
                    return true;
                }
                delete chn;
                GetLogger()->Warn("Could not open additional channel to play sound!\n");
            }
        }
    }

    int lowerOrEqual = -1;
    for (auto it : m_channels)
    {
        if (it.second->GetPriority() < priority)
        {
            GetLogger()->Debug("Sound channel with lower priority will be reused.\n");
            channel = it.first;
            return true;
        }
        if (it.second->GetPriority() <= priority)
            lowerOrEqual = it.first;
    }

    if (lowerOrEqual != -1)
    {
        channel = lowerOrEqual;
        GetLogger()->Debug("Sound channel with lower or equal priority will be reused.\n");
        return true;
    }

    GetLogger()->Warn("Could not find free buffer to use.\n");
    return false;
}


int ALSound::Play(Sound sound, float amplitude, float frequency, bool bLoop)
{
    return Play(sound, m_eye, amplitude, frequency, bLoop);
}


int ALSound::Play(Sound sound, Math::Vector pos, float amplitude, float frequency, bool bLoop)
{
    if (!m_enabled)
    {
        return -1;
    }
    if (m_sounds.find(sound) == m_sounds.end())
    {
        GetLogger()->Warn("Sound %d was not loaded!\n", sound);
        return -1;
    }

    int channel;
    bool bAlreadyLoaded = false;
    if (!SearchFreeBuffer(sound, channel, bAlreadyLoaded))
        return -1;

    if (!bAlreadyLoaded)
    {
        if (!m_channels[channel]->SetBuffer(m_sounds[sound]))
        {
            m_channels[channel]->SetBuffer(nullptr);
            return -1;
        }
    }

    Position(channel, pos);
    if (!m_3D)
    {
        ComputeVolumePan2D(channel, pos);
    }
    else
    {
        m_channels[channel]->SetVolumeAtrib(1.0f);
    }

    // setting initial values
    m_channels[channel]->SetStartAmplitude(amplitude);
    m_channels[channel]->SetStartFrequency(frequency);
    m_channels[channel]->SetChangeFrequency(1.0f);
    m_channels[channel]->ResetOper();
    m_channels[channel]->SetFrequency(frequency);
    m_channels[channel]->SetVolume(powf(amplitude * m_channels[channel]->GetVolumeAtrib(), 0.2f) * m_audioVolume);
    m_channels[channel]->SetLoop(bLoop);
    m_channels[channel]->Play();

    return channel;
}


bool ALSound::FlushEnvelope(int channel)
{
    if (m_channels.find(channel) == m_channels.end())
    {
        return false;
    }

    m_channels[channel]->ResetOper();
    return true;
}


bool ALSound::AddEnvelope(int channel, float amplitude, float frequency, float time, SoundNext oper)
{
    if (!m_enabled)
        return false;

    if (m_channels.find(channel) == m_channels.end())
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


bool ALSound::Position(int channel, Math::Vector pos)
{
    if (!m_enabled)
        return false;

    if (m_channels.find(channel) == m_channels.end())
    {
        return false;
    }

    if (m_3D)
    {
        m_channels[channel]->SetPan(pos);
    }
    else
    {
        ComputeVolumePan2D(channel, pos);

        if (!m_channels[channel]->HasEnvelope())
        {
            float volume = m_channels[channel]->GetStartAmplitude();
            m_channels[channel]->SetVolume(powf(volume * m_channels[channel]->GetVolumeAtrib(), 0.2f) * m_audioVolume);
        }
    }
    return true;
}


bool ALSound::Frequency(int channel, float frequency)
{
    if (!m_enabled)
        return false;

    if (m_channels.find(channel) == m_channels.end())
    {
        return false;
    }

    m_channels[channel]->SetFrequency(frequency * m_channels[channel]->GetInitFrequency());
    m_channels[channel]->SetChangeFrequency(frequency);
    return true;
}

bool ALSound::Stop(int channel)
{
    if (!m_enabled)
        return false;

    if (m_channels.find(channel) == m_channels.end())
    {
        return false;
    }

    m_channels[channel]->Stop();
    m_channels[channel]->ResetOper();

    return true;
}


bool ALSound::StopAll()
{
    if (!m_enabled)
        return false;

    for (auto channel : m_channels)
    {
        channel.second->Stop();
        channel.second->ResetOper();
    }

    return true;
}


bool ALSound::MuteAll(bool bMute)
{
    if (!m_enabled)
        return false;

    for (auto it : m_channels)
    {
        if (it.second->IsPlaying())
        {
            it.second->Mute(bMute);
        }
    }

    if (bMute)
    {
        m_currentMusic->SetVolume(0.0f);
    }
    else
    {
        m_currentMusic->SetVolume(m_musicVolume);
    }
    return true;
}


void ALSound::FrameMove(float delta)
{
    if (!m_enabled)
        return;

    float progress;
    float volume, frequency;
    for (auto it : m_channels)
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
        oper.currentTime += delta;
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
}


void ALSound::SetListener(Math::Vector eye, Math::Vector lookat)
{
    m_eye = eye;
    m_lookat = lookat;
    if (m_3D)
    {
        float orientation[] = {lookat.x, lookat.y, lookat.z, 0.f, 1.f, 0.f};
        alListener3f(AL_POSITION, eye.x, eye.y, eye.z);
        alListenerfv(AL_ORIENTATION, orientation);
    }
    else
    {
        float orientation[] = {0.0f, 0.0f, 0.0f, 0.f, 1.f, 0.f};
        alListener3f(AL_POSITION, 0.0f, 0.0f, 0.0f);
        alListenerfv(AL_ORIENTATION, orientation);

        // recalculate sound position
        for (auto it : m_channels)
        {
            if (it.second->IsPlaying())
            {
                Math::Vector pos = it.second->GetPosition();
                ComputeVolumePan2D(it.first, pos);

                if (!it.second->HasEnvelope())
                {
                    float volume = it.second->GetStartAmplitude();
                    it.second->SetVolume(powf(volume * it.second->GetVolumeAtrib(), 0.2f) * m_audioVolume);
                }
            }
        }
    }
}

bool ALSound::PlayMusic(int rank, bool bRepeat)
{
    std::stringstream filename;
    filename << "music" << std::setfill('0') << std::setw(3) << rank << ".ogg";
    return PlayMusic(filename.str(), bRepeat);
}

bool ALSound::PlayMusic(std::string filename, bool bRepeat)
{
    if (!m_enabled)
    {
        return false;
    }

    std::stringstream file;
    file << m_soundPath << "/" << filename;

    // check if we have music in cache
    if (m_music.find(filename) == m_music.end())
    {
        GetLogger()->Warn("Music %s was not cached!\n", filename.c_str());
        if (!boost::filesystem::exists(file.str()))
        {
            GetLogger()->Warn("Requested music %s was not found.\n", filename.c_str());
            return false;
        }
        Buffer *buffer = new Buffer();
        buffer->LoadFromFile(file.str(), static_cast<Sound>(-1));
        m_currentMusic->SetBuffer(buffer);
    }
    else
    {
        GetLogger()->Debug("Music loaded from cache\n");
        m_currentMusic->SetBuffer(m_music[filename]);
    }

    m_currentMusic->SetVolume(m_musicVolume);
    m_currentMusic->SetLoop(bRepeat);
    m_currentMusic->Play();

    return true;
}


bool ALSound::RestartMusic()
{
    if (!m_enabled || !m_currentMusic)
    {
        return false;
    }

    m_currentMusic->Stop();
    m_currentMusic->Play();
    return true;
}

void ALSound::StopMusic()
{
    if (!m_enabled || !m_currentMusic)
    {
        return;
    }

    SuspendMusic();
}


bool ALSound::IsPlayingMusic()
{
    if (!m_enabled || !m_currentMusic)
    {
        return false;
    }

    return m_currentMusic->IsPlaying();
}


void ALSound::SuspendMusic()
{
    if (!m_enabled || !m_currentMusic)
    {
        return;
    }

    m_currentMusic->Stop();
}


void ALSound::ComputeVolumePan2D(int channel, Math::Vector &pos)
{
    float dist, a, g;
    m_channels[channel]->SetPosition(pos);

    if (VectorsEqual(pos, m_eye))
    {
        m_channels[channel]->SetVolumeAtrib(1.0f);  // maximum volume
        m_channels[channel]->SetPan(Math::Vector());  // at the center
        return;
    }

    dist = Distance(pos, m_eye);
    if ( dist >= 110.0f ) // very far?
    {
        m_channels[channel]->SetVolumeAtrib(0.0f);  // silence
        m_channels[channel]->SetPan(Math::Vector());  // at the center
        return;
    }
    else if ( dist <= 10.0f ) // very close?
    {
        m_channels[channel]->SetVolumeAtrib(1.0f);   // maximum volume
        m_channels[channel]->SetPan(Math::Vector());  // at the center
        return;
    }
    m_channels[channel]->SetVolumeAtrib(1.0f - ((dist - 10.0f) / 100.0f));

    Math::Vector one = Math::Vector(1.0f, 0.0f, 0.0f);
    float angle_a = Angle(Math::Vector(m_lookat.x - m_eye.x, m_lookat.z - m_eye.z, 0.0f), one);
    float angle_g = Angle(Math::Vector(pos.x - m_eye.x, pos.z - m_eye.z, 0.0f), one);

    a = fmodf(angle_a, Math::PI * 2.0f);
    g = fmodf(angle_g, Math::PI * 2.0f);

    if ( a < 0.0f )
    {
        a += Math::PI * 2.0f;
    }
    if ( g < 0.0f )
    {
        g += Math::PI * 2.0f;
    }

    if ( a < g )
    {
        if (a + Math::PI * 2.0f - g < g - a )
        {
            a += Math::PI * 2.0f;
        }
    }
    else
    {
        if ( g + Math::PI * 2.0f - a < a - g )
        {
            g += Math::PI * 2.0f;
        }
    }

    m_channels[channel]->SetPan( Math::Vector(0.0f, 0.0f, sinf(g - a)) );
}

