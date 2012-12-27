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

// alsound.cpp


#include "alsound.h"


#define MIN(a, b) (a > b ? b : a)

ALSound::ALSound()
{
    mEnabled = false;
    m3D = false;
    mAudioVolume = MAXVOLUME;
    mMute = false;
    auto pointer = CInstanceManager::GetInstancePointer();
    if (pointer != nullptr)
        CInstanceManager::GetInstancePointer()->AddInstance(CLASS_SOUND, this);
}


ALSound::~ALSound()
{
    auto pointer = CInstanceManager::GetInstancePointer();
    if (pointer != nullptr)
        CInstanceManager::GetInstancePointer()->DeleteInstance(CLASS_SOUND, this);
    CleanUp();
}


void ALSound::CleanUp()
{
    if (mEnabled) {
        GetLogger()->Info("Unloading files and closing device...\n");
        StopAll();

        for (auto item : mSounds)
            delete item.second;

        mEnabled = false;
        alutExit();
    }
}


bool ALSound::Create(bool b3D)
{
    CleanUp();

    if (mEnabled)
        return true;

    GetLogger()->Info("Opening audio device...\n");
    if (!alutInit(NULL, NULL)) {
        ALenum error = alutGetError();
        GetLogger()->Error("Could not open audio device! Reason: %s\n", alutGetErrorString(error));
        return false;
    }
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


bool ALSound::RetEnable()
{
    return mEnabled;
}


void ALSound::SetAudioVolume(int volume)
{
    alListenerf(AL_GAIN, MIN(volume, MAXVOLUME) * 0.01f);
    mAudioVolume = MIN(volume, MAXVOLUME);
}


int ALSound::GetAudioVolume()
{
    float volume;
    if ( !mEnabled )
        return 0;

    alGetListenerf(AL_GAIN, &volume);
    return volume * MAXVOLUME;
}


void ALSound::SetMusicVolume(int volume)
{
    // TODO stub! Add music support
}


int ALSound::GetMusicVolume()
{
    // TODO stub! Add music support
    if ( !mEnabled )
        return 0;

    return 0;
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
        bAlreadyLoaded = true;
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
        while (++i)
        {
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
    if (!mEnabled)
        return -1;

    if (mAudioVolume <= 0.0f)
        return -1;

    if (mSounds.find(sound) == mSounds.end()) {
        GetLogger()->Warn("Sound %d was not loaded!\n", sound);
        return -1;
    }

    int channel;
    bool bAlreadyLoaded;
    if (!SearchFreeBuffer(sound, channel, bAlreadyLoaded))
        return -1;
    if ( !bAlreadyLoaded ) {
        mChannels[channel]->SetBuffer(mSounds[sound]);
    }

    Position(channel, pos);

    // setting initial values
    mChannels[channel]->SetStartAmplitude(amplitude);
    mChannels[channel]->SetStartFrequency(frequency);
    mChannels[channel]->SetChangeFrequency(1.0f);
    mChannels[channel]->ResetOper();
    mChannels[channel]->AdjustFrequency(frequency);
    mChannels[channel]->AdjustVolume(mAudioVolume);
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
    mChannels[channel]->AddOper(op);

    return false;
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

    mChannels[channel]->SetFrequency(frequency);
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
        if (!it.second->IsPlaying())
            continue;

        if (!it.second->HasEnvelope())
            continue;

        //it.second->GetEnvelope().currentTime += delta;
        SoundOper oper = it.second->GetEnvelope();
        progress = it.second->GetCurrentTime() / oper.totalTime;
        progress = MIN(progress, 1.0f);

        // setting volume
        volume = progress * abs(oper.finalAmplitude - it.second->GetStartAmplitude());
        it.second->AdjustVolume(volume * mAudioVolume);

        // setting frequency
        frequency = progress * (oper.finalFrequency - it.second->GetStartFrequency()) * it.second->GetStartFrequency() * it.second->GetChangeFrequency();
        it.second->AdjustFrequency(frequency);

        if (it.second->GetEnvelope().totalTime <= it.second->GetCurrentTime()) {

            if (oper.nextOper == SOPER_LOOP) {
                GetLogger()->Trace("Sound oper: replay.\n");
                it.second->SetCurrentTime(0.0f);
                it.second->Play();
            } else {
                GetLogger()->Trace("Sound oper: next.\n");
                it.second->SetStartAmplitude(oper.finalAmplitude);
                it.second->SetStartFrequency(oper.finalFrequency);
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
    // TODO stub! Add music support
    return true;
}


bool ALSound::RestartMusic()
{
    // TODO stub! Add music support
    return true;
}

void ALSound::StopMusic()
{
    // TODO stub! Add music support
    SuspendMusic();
}


bool ALSound::IsPlayingMusic()
{
    // TODO stub! Add music support
    return true;
}


void ALSound::SuspendMusic()
{
    // TODO stub! Add music support
}
