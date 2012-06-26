// * This file is part of the COLOBOT source code
// * Copyright (C) 2001-2008, Daniel ROUX & EPSITEC SA, www.epsitec.ch
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

// sound.h

#ifndef SOUND_H
#define SOUND_H


#include <dsound.h>


#define MAXFILES    200
#define MAXSOUND    32
#define MAXVOLUME   20
#define MAXOPER     4

class CInstanceManager;


enum Sound
{
    SOUND_CLICK     = 0,
    SOUND_BOUM      = 1,
    SOUND_EXPLO     = 2,
    SOUND_FLYh      = 3,    // human
    SOUND_FLY       = 4,
    SOUND_STEPs     = 5,    // smooth
    SOUND_MOTORw        = 6,    // wheel
    SOUND_MOTORt        = 7,    // tank
    SOUND_MOTORr        = 8,    // roller
    SOUND_ERROR     = 9,
    SOUND_CONVERT       = 10,
    SOUND_ENERGY        = 11,
    SOUND_PLOUF     = 12,
    SOUND_BLUP      = 13,
    SOUND_WARNING       = 14,
    SOUND_DERRICK       = 15,
    SOUND_LABO      = 16,
    SOUND_STATION       = 17,
    SOUND_REPAIR        = 18,
    SOUND_RESEARCH      = 19,
    SOUND_INSECTs       = 20,   // spider
    SOUND_BURN      = 21,
    SOUND_TZOING        = 22,
    SOUND_GGG       = 23,
    SOUND_MANIP     = 24,
    SOUND_FIRE      = 25,   // shooting with fireball
    SOUND_HUMAN1        = 26,   // breathing
    SOUND_STEPw     = 27,   // water
    SOUND_SWIM      = 28,
    SOUND_RADAR     = 29,
    SOUND_BUILD     = 30,
    SOUND_ALARM     = 31,   // energy alarm
    SOUND_SLIDE     = 32,
    SOUND_EXPLOi        = 33,   // insect
    SOUND_INSECTa       = 34,   // ant
    SOUND_INSECTb       = 35,   // bee
    SOUND_INSECTw       = 36,   // worm
    SOUND_INSECTm       = 37,   // mother
    SOUND_TREMBLE       = 38,
    SOUND_PSHHH     = 39,
    SOUND_NUCLEAR       = 40,
    SOUND_INFO      = 41,
    SOUND_OPEN      = 42,
    SOUND_CLOSE     = 43,
    SOUND_FACTORY       = 44,
    SOUND_EGG       = 45,
    SOUND_MOTORs        = 46,   // submarine
    SOUND_MOTORi        = 47,   // insect (legs)
    SOUND_SHIELD        = 48,
    SOUND_FIREi     = 49,   // shooting with orgaball (insect)
    SOUND_GUNDEL        = 50,
    SOUND_PSHHH2        = 51,   // shield
    SOUND_MESSAGE       = 52,
    SOUND_BOUMm     = 53,   // metal
    SOUND_BOUMv     = 54,   // plant
    SOUND_BOUMs     = 55,   // smooth
    SOUND_EXPLOl        = 56,   // little
    SOUND_EXPLOlp       = 57,   // little power
    SOUND_EXPLOp        = 58,   // power
    SOUND_STEPh     = 59,   // hard
    SOUND_STEPm     = 60,   // metal
    SOUND_POWERON       = 61,
    SOUND_POWEROFF      = 62,
    SOUND_AIE       = 63,
    SOUND_WAYPOINT      = 64,
    SOUND_RECOVER       = 65,
    SOUND_DEADi     = 66,
    SOUND_JOSTLE        = 67,
    SOUND_GFLAT     = 68,
    SOUND_DEADg     = 69,   // shooting death
    SOUND_DEADw     = 70,   // drowning
    SOUND_FLYf      = 71,   // reactor fail
    SOUND_ALARMt        = 72,   // temperature alarm
    SOUND_FINDING       = 73,   // finds a cache object
    SOUND_THUMP     = 74,
    SOUND_TOUCH     = 75,
    SOUND_BLITZ     = 76,
    SOUND_MUSHROOM      = 77,
    SOUND_FIREp     = 78,   // shooting with phazer
    SOUND_EXPLOg1       = 79,   // impact gun 1
    SOUND_EXPLOg2       = 80,   // impact gun 2
    SOUND_MOTORd        = 81,   // engine friction
};

enum SoundNext
{
    SOPER_CONTINUE  = 1,
    SOPER_STOP      = 2,
    SOPER_LOOP      = 3,
};

typedef struct
{
    char        bUsed;
    float       finalAmplitude;
    float       finalFrequency;
    float       totalTime;
    float       currentTime;
    SoundNext   nextOper;
}
SoundOper;

typedef struct
{
    char                    bUsed;          // buffer used?
    char                    bMute;          // silence?
    Sound                   type;           // SOUND_*
    int                 priority;       // so great -> important
    D3DVECTOR               pos;            // position in space
    unsigned short              uniqueStamp;        // unique marker
    LPDIRECTSOUNDBUFFER         soundBuffer;
    LPDIRECTSOUND3DBUFFER           soundBuffer3D;
    float                   startAmplitude;
    float                   startFrequency;
    float                   changeFrequency;
    int                 initFrequency;
    float                   volume;         // 2D: volume 1..0 depending on position
    float                   pan;            // 2D: pan -1..+1 depending on position
    SoundOper               oper[MAXOPER];
}
SoundChannel;



class CSound
{
public:
    CSound(CInstanceManager* iMan);
    ~CSound();

    void    SetDebugMode(BOOL bMode);
    BOOL    Create(HWND hWnd, BOOL b3D);
    void    CacheAll();

    void    SetState(BOOL bState);
    BOOL    RetEnable();

    void    SetCDpath(char *path);
    void    SetAudioTrack(BOOL bAudio);

    void    SetSound3D(BOOL bMode);
    BOOL    RetSound3D();
    BOOL    RetSound3DCap();

    void    SetAudioVolume(int volume);
    int     RetAudioVolume();
    void    SetMidiVolume(int volume);
    int     RetMidiVolume();

    void    SetListener(D3DVECTOR eye, D3DVECTOR lookat);
    void    FrameMove(float rTime);

    int     Play(Sound sound, float amplitude=1.0f, float frequency=1.0f, BOOL bLoop=FALSE);
    int     Play(Sound sound, D3DVECTOR pos, float amplitude=1.0f, float frequency=1.0f, BOOL bLoop=FALSE);
    BOOL    FlushEnvelope(int channel);
    BOOL    AddEnvelope(int channel, float amplitude, float frequency, float time, SoundNext oper);
    BOOL    Position(int channel, D3DVECTOR pos);
    BOOL    Frequency(int channel, float frequency);
    BOOL    Stop(int channel);
    BOOL    StopAll();
    BOOL    MuteAll(BOOL bMute);

    BOOL    PlayMusic(int rank, BOOL bRepeat);
    BOOL    RestartMusic();
    void    SuspendMusic();
    void    StopMusic();
    BOOL    IsPlayingMusic();
    void    AdaptVolumeMusic();

protected:
    BOOL    CheckChannel(int &channel);
    BOOL    CreateSoundBuffer(int channel, DWORD size, DWORD freq, DWORD bitsPerSample, DWORD blkAlign, BOOL bStereo);
    BOOL    ReadData(LPDIRECTSOUNDBUFFER lpDSB, Sound sound, DWORD size);
    BOOL    CreateBuffer(int channel, Sound sound);
    void    ComputeVolumePan2D(int channel, const D3DVECTOR &pos);
    BOOL    ReadFile(Sound sound, char *metaname, char *filename);
    int     RetPriority(Sound sound);
    BOOL    SearchFreeBuffer(Sound sound, int &channel, BOOL &bAlreadyLoaded);
    void    OperNext(int channel);
    BOOL    PlayAudioTrack(int rank);

protected:
    CInstanceManager*       m_iMan;

    HWND                    m_hWnd;
    BOOL                    m_bEnable;
    BOOL                    m_bState;
    BOOL                    m_bAudioTrack;
    BOOL                    m_ctrl3D;
    BOOL                    m_bDebugMode;
    LPDIRECTSOUND           m_lpDS;
    LPDIRECTSOUND3DLISTENER m_listener;
    SoundChannel            m_channel[MAXSOUND];
    char*                   m_files[MAXFILES];
    UINT                    m_MidiDeviceID;
    int                     m_MIDIMusic;
    BOOL                    m_bRepeatMusic;
    int                     m_audioVolume;
    int                     m_midiVolume;
    int                     m_lastMidiVolume;
    D3DVECTOR               m_eye;
    D3DVECTOR               m_lookat;
    float                   m_lastTime;
    float                   m_playTime;
    int                     m_uniqueStamp;
    int                     m_maxSound;
    char                    m_CDpath[100];
};

#endif // SOUND_H
