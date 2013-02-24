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


#include "alsound.h"

#define MIN(a, b) (a > b ? b : a)

ALSound::ALSound()
{
    mEnabled = false;
    m3D = false;
    mAudioVolume = 1.0f;
    mMusicVolume = 1.0f;
    mMute = false;
    mCurrentMusic = nullptr;
}


ALSound::~ALSound()
{
    CleanUp();
}


void ALSound::CleanUp()
{
    if (mEnabled) {
        GetLogger()->Info("Unloading files and closing device...\n");
        StopAll();

        for (auto channel : mChannels) {
            delete channel.second;
        }

        for (auto item : mSounds) {
            delete item.second;
        }

        mEnabled = false;

        mCurrentMusic->FreeBuffer();
        delete mCurrentMusic;
        alcDestroyContext(mContext);
        alcCloseDevice(mDevice);
    }
}


bool ALSound::Create(bool b3D)
{
    CleanUp();

    if (mEnabled)
        return true;

    GetLogger()->Info("Opening audio device...\n");
    mDevice = alcOpenDevice(NULL);
    if (!mDevice) {
        GetLogger()->Error("Could not open audio device!\n");
        return false;
    }

    mContext = alcCreateContext(mDevice, NULL);
    if (!mContext) {
        GetLogger()->Error("Could not create audio context!\n");
        return false;
    }
    alcMakeContextCurrent(mContext);

    mCurrentMusic = new Channel();
    GetLogger()->Info("Done.\n");
    mEnabled = true;
    return true;
}


void ALSound::SetSound3D(bool bMode)
{
    // TODO stub! need to be implemented
    m3D = bMode;
}


bool ALSound::GetSound3D()
{
    // TODO stub! need to be implemented
    return true;
}


bool ALSound::GetSound3DCap()
{
    // TODO stub! need to be implemented
    return true;
}


bool ALSound::GetEnable()
{
    return mEnabled;
}


void ALSound::SetAudioVolume(int volume)
{
    mAudioVolume = MIN(static_cast<float>(volume) / MAXVOLUME, 1.0f);
    alListenerf(AL_GAIN, mAudioVolume);
}


int ALSound::GetAudioVolume()
{
    if ( !mEnabled )
        return 0;

    return mAudioVolume * MAXVOLUME;
}


void ALSound::SetMusicVolume(int volume)
{
    mMusicVolume = MIN(static_cast<float>(volume) / MAXVOLUME, 1.0f);
    if (mCurrentMusic) {
        mCurrentMusic->SetVolume(mMusicVolume);
    }
}


int ALSound::GetMusicVolume()
{
    if ( !mEnabled )
        return 0.0f;

    return mMusicVolume * MAXVOLUME;
}


bool ALSound::Cache(Sound sound, std::string filename)
{
    Buffer *buffer = new Buffer();
    if (buffer->LoadFromFile(filename, sound)) {
        mSounds[sound] = buffer;
        return true;
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
    for (auto it : mChannels) {
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
    if (mChannels.size() == 0) {
        Channel *chn = new Channel();
        // check if we channel ready to play music, if not report error
        if (chn->IsReady()) {
            chn->SetPriority(priority);
            mChannels[1] = chn;
            channel = 1;
            bAlreadyLoaded = false;
            return true;
        }
        delete chn;
        GetLogger()->Error("Could not open channel to play sound!");
        return false;
    }

    // Seeks a channel completely free.
    if (mChannels.size() < 64) {
        auto it = mChannels.end();
        it--;
        int i = (*it).first;
        while (++i) {
            if (mChannels.find(i) == mChannels.end()) {
                Channel *chn = new Channel();
                // check if channel is ready to play music, if not destroy it and seek free one
                if (chn->IsReady()) {
                    chn->SetPriority(priority);
                    mChannels[++i] = chn;
                    channel = i;
                    bAlreadyLoaded = false;
                    return true;
                }
                delete chn;
                GetLogger()->Warn("Could not open additional channel to play sound!");
            }
        }
    }

    int lowerOrEqual = -1;
    for (auto it : mChannels) {
        if (it.second->GetPriority() < priority) {
            GetLogger()->Debug("Sound channel with lower priority will be reused.");
            channel = it.first;
            return true;
        }
        if (it.second->GetPriority() <= priority)
            lowerOrEqual = it.first;
    }

    if (lowerOrEqual != -1) {
        channel = lowerOrEqual;
        GetLogger()->Debug("Sound channel with lower or equal priority will be reused.");
        return true;
    }

    GetLogger()->Warn("Could not find free buffer to use.\n");
    return false;
}


int ALSound::Play(Sound sound, float amplitude, float frequency, bool bLoop)
{
    return Play(sound, Math::Vector(), amplitude, frequency, bLoop);
}


int ALSound::Play(Sound sound, Math::Vector pos, float amplitude, float frequency, bool bLoop)
{
    if (!mEnabled) {
        return -1;
    }

    if (mSounds.find(sound) == mSounds.end()) {
        GetLogger()->Warn("Sound %d was not loaded!\n", sound);
        return -1;
    }

    int channel;
    bool bAlreadyLoaded = false;
    if (!SearchFreeBuffer(sound, channel, bAlreadyLoaded))
        return -1;

    if (!bAlreadyLoaded) {
        if (!mChannels[channel]->SetBuffer(mSounds[sound])) {
            mChannels[channel]->SetBuffer(nullptr);
            return -1;
        }
    }
    Position(channel, pos);

    // setting initial values
    mChannels[channel]->SetStartAmplitude(amplitude * mAudioVolume);
    mChannels[channel]->SetStartFrequency(frequency);
    mChannels[channel]->SetChangeFrequency(1.0f);
    mChannels[channel]->ResetOper();
    mChannels[channel]->SetFrequency(frequency);
    mChannels[channel]->SetVolume(amplitude * mAudioVolume);
    mChannels[channel]->SetLoop(bLoop);
    mChannels[channel]->Play();

    return channel;
}


bool ALSound::FlushEnvelope(int channel)
{
    if (mChannels.find(channel) == mChannels.end()) {
        return false;
    }

    mChannels[channel]->ResetOper();
    return true;
}


bool ALSound::AddEnvelope(int channel, float amplitude, float frequency, float time, SoundNext oper)
{
    if (!mEnabled)
        return false;

    if (mChannels.find(channel) == mChannels.end()) {
        return false;
    }
    
    SoundOper op;
    op.finalAmplitude = amplitude;
    op.finalFrequency = frequency;
    op.totalTime = time;
    op.nextOper = oper;
    op.currentTime = 0.0f;
    mChannels[channel]->AddOper(op);

    return true;
}


bool ALSound::Position(int channel, Math::Vector pos)
{
    if (!mEnabled)
        return false;

    if (mChannels.find(channel) == mChannels.end()) {
        return false;
    }

    mChannels[channel]->SetPosition(pos);
    return true;
}


bool ALSound::Frequency(int channel, float frequency)
{
    if (!mEnabled)
        return false;

    if (mChannels.find(channel) == mChannels.end()) {
        return false;
    }

    mChannels[channel]->SetFrequency(frequency * mChannels[channel]->GetInitFrequency());
    mChannels[channel]->SetChangeFrequency(frequency);
    return true;
}

bool ALSound::Stop(int channel)
{
    if (!mEnabled)
        return false;

    if (mChannels.find(channel) == mChannels.end()) {
        return false;
    }

    mChannels[channel]->Stop();
    mChannels[channel]->ResetOper();

    return true;
}


bool ALSound::StopAll()
{
    if (!mEnabled)
        return false;

    for (auto channel : mChannels) {
        channel.second->Stop();
        channel.second->ResetOper();
    }

    return true;
}


bool ALSound::MuteAll(bool bMute)
{
    if (!mEnabled)
        return false;

    float volume;
    mMute = bMute;
    if (mMute)
        volume = 0;
    else
        volume = mAudioVolume;

    for (auto channel : mChannels) {
        channel.second->SetVolume(volume);
    }

    return true;
}

void ALSound::FrameMove(float delta)
{
    if (!mEnabled)
        return;

    float progress;
    float volume, frequency;
    for (auto it : mChannels) {
        if (!it.second->IsPlaying()) {
            continue;
        }

        if (!it.second->HasEnvelope())
            continue;

        SoundOper &oper = it.second->GetEnvelope();
        oper.currentTime += delta;
        progress = oper.currentTime / oper.totalTime;
        progress = MIN(progress, 1.0f);
       
        // setting volume
        volume = progress * (oper.finalAmplitude - it.second->GetStartAmplitude());
        volume = (volume + it.second->GetStartAmplitude()) * mAudioVolume;
        it.second->SetVolume(volume);

        // setting frequency
        frequency = progress * (oper.finalFrequency - it.second->GetStartFrequency()) * it.second->GetStartFrequency() * it.second->GetChangeFrequency() * it.second->GetInitFrequency();
        it.second->AdjustFrequency(frequency);
        GetLogger()->Error("%f\n", frequency);

        if (oper.totalTime <= oper.currentTime) {
            if (oper.nextOper == SOPER_LOOP) {
                oper.currentTime = 0.0f;
                it.second->Play();
            } else {
                it.second->SetStartAmplitude(oper.finalAmplitude);
                it.second->SetStartFrequency(oper.finalFrequency);
                if (oper.nextOper == SOPER_STOP) {
                    it.second->Stop();
                }

                it.second->PopEnvelope();
            }
        }
    }
}


void ALSound::SetListener(Math::Vector eye, Math::Vector lookat)
{
    float orientation[] = {lookat.x, lookat.y, lookat.z, 0.f, 1.f, 0.f};
    alListener3f(AL_POSITION, eye.x, eye.y, eye.z);
    alListenerfv(AL_ORIENTATION, orientation);
}


bool ALSound::PlayMusic(int rank, bool bRepeat)
{
    if (!mEnabled) {
        return false;
    }
    
    if (static_cast<int>(mCurrentMusic->GetSoundType()) != rank) {
        // check if we have music in cache
        for (auto music : mMusicCache) {
            if (static_cast<int>(music->GetSoundType()) == rank) {
                GetLogger()->Debug("Music loaded from cache\n");
                mCurrentMusic->SetBuffer(music);

                mCurrentMusic->SetVolume(mMusicVolume);
                mCurrentMusic->SetLoop(bRepeat);
                mCurrentMusic->Play();
                return true;
            }
        }
     
        // we cache only 3 music files
        if (mMusicCache.size() == 3) {
            mCurrentMusic->FreeBuffer();
            mMusicCache.pop_back();
        }

        if (mMusic.find(rank) == mMusic.end()) {
            GetLogger()->Info("Requested music %d was not found.\n", rank);
            return false;
        }

        Buffer *buffer = new Buffer();
        mMusicCache.push_front(buffer);
        buffer->LoadFromFile(mMusic.at(rank), static_cast<Sound>(rank));
        mCurrentMusic->SetBuffer(buffer);
        mMusicCache[rank] = buffer;
    }
    
    mCurrentMusic->SetVolume(mMusicVolume);
    mCurrentMusic->SetLoop(bRepeat);
    mCurrentMusic->Play();
    
    return true;
}


bool ALSound::RestartMusic()
{
    if (!mEnabled || !mCurrentMusic) {
        return false;
    }

    mCurrentMusic->Stop();
    mCurrentMusic->Play();
    return true;
}

void ALSound::StopMusic()
{
    if (!mEnabled || !mCurrentMusic) {
        return;
    }
    
    SuspendMusic();
}


bool ALSound::IsPlayingMusic()
{
    if (!mEnabled || !mCurrentMusic) {
        return false;
    }
    
    return mCurrentMusic->IsPlaying();
}


void ALSound::SuspendMusic()
{
    if (!mEnabled || !mCurrentMusic) {
        return;
    }
    
    mCurrentMusic->Stop();
}
