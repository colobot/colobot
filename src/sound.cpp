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

// sound.cpp

#define STRICT
#define D3D_OVERLOADS

#include <stdlib.h>
#include <ctype.h>
#include <d3dtypes.h>
#include <dsound.h>
#include <stdio.h>
#include "language.h"
#include "struct.h"
#include "iman.h"
#include "math3d.h"
#include "sound.h"


/////////////////////////////////////////////////////////////////////////////


#define LXIMAGE     640
#define LYIMAGE     480



// Header .WAV file.

struct WaveHeader
{
    BYTE        RIFF[4];        // "RIFF"
    DWORD       dwSize;         // size of data to follow
    BYTE        WAVE[4];        // "WAVE"
    BYTE        fmt_[4];        // "fmt "
    DWORD       dw16;           // 16
    WORD        wOne_0;         // 1
    WORD        wChnls;         // number of Channels
    DWORD       dwSRate;        // sample Rate
    DWORD       BytesPerSec;        // sample Rate
    WORD        wBlkAlign;      // 1
    WORD        BitsPerSample;      // sample size
    BYTE        DATA[4];        // "DATA"
    DWORD       dwDSize;        // number of Samples
};




// Displays an error DirectSound.

void DisplayError(char *name, Sound sound, HRESULT err)
{
    char    s[100];
    unsigned int i = err;
    if ( err == DS_OK )  return;
    sprintf(s, "SoundError in %s, sound=%d err=%d\n", name, sound, i);
    OutputDebugString(s);

    if ( err == DSERR_ALLOCATED )  OutputDebugString("DSERR_ALLOCATED\n");
    if ( err == DSERR_CONTROLUNAVAIL )  OutputDebugString("DSERR_CONTROLUNAVAIL\n");
    if ( err == DSERR_INVALIDPARAM )  OutputDebugString("DSERR_INVALIDPARAM\n");
    if ( err == DSERR_INVALIDCALL )  OutputDebugString("DSERR_INVALIDCALL\n");
    if ( err == DSERR_GENERIC )  OutputDebugString("DSERR_GENERIC\n");
    if ( err == DSERR_PRIOLEVELNEEDED )  OutputDebugString("DSERR_PRIOLEVELNEEDED\n");
    if ( err == DSERR_OUTOFMEMORY )  OutputDebugString("DSERR_OUTOFMEMORY\n");
    if ( err == DSERR_BADFORMAT )  OutputDebugString("DSERR_BADFORMAT\n");
    if ( err == DSERR_UNSUPPORTED )  OutputDebugString("DSERR_UNSUPPORTED\n");
    if ( err == DSERR_NODRIVER )  OutputDebugString("DSERR_NODRIVER\n");
    if ( err == DSERR_ALREADYINITIALIZED )  OutputDebugString("DSERR_ALREADYINITIALIZED\n");
    if ( err == DSERR_NOAGGREGATION )  OutputDebugString("DSERR_NOAGGREGATION\n");
    if ( err == DSERR_BUFFERLOST )  OutputDebugString("DSERR_BUFFERLOST\n");
    if ( err == DSERR_OTHERAPPHASPRIO )  OutputDebugString("DSERR_OTHERAPPHASPRIO\n");
    if ( err == DSERR_UNINITIALIZED )  OutputDebugString("DSERR_UNINITIALIZED\n");
    if ( err == DSERR_NOINTERFACE )  OutputDebugString("DSERR_NOINTERFACE\n");
    if ( err == DSERR_ACCESSDENIED )  OutputDebugString("DSERR_ACCESSDENIED\n");
}

// Returns the name of the current folder.

void GetCurrentDir(char *pName, int lg)
{
    int     i;

    strncpy(pName, _pgmptr, lg-1);
    pName[lg-1] = 0;

    lg = strlen(pName);
    if ( lg == 0 )  return;

    for ( i=0 ; i<lg ; i++ )
    {
        pName[i] = tolower(pName[i]);
    }

    while ( lg > 0 )
    {
        lg --;
        if ( pName[lg] == '\\' )
        {
            pName[lg+1] = 0;
            break;
        }
    }

    if ( lg > 6 && strcmp(pName+lg-6, "\\debug\\") == 0 )
    {
        pName[lg-5] = 0;  // ignores the folder \debug!
    }

    if ( lg > 6 && strcmp(pName+lg-6, "\\release\\") == 0 )
    {
        pName[lg-7] = 0;  // ignores the folder \release !
    }
}




/////////////////////////////////////////////////////////////////////////////


// Changes the volume of midi.
// The volume is between 0 and 20!

void InitMidiVolume(int volume)
{
    int         nb, i, n;
    MMRESULT        result;
    HMIDIOUT        hmo = 0;

    static int table[21] =
    {
        0x00000000,
        0x11111111,
        0x22222222,
        0x33333333,
        0x44444444,
        0x55555555,
        0x66666666,
        0x77777777,
        0x88888888,
        0x99999999,
        0xAAAAAAAA,
        0xBBBBBBBB,
        0xCCCCCCCC,
        0xDDDDDDDD,
        0xEEEEEEEE,
        0xF222F222,
        0xF555F555,
        0xF777F777,
        0xFAAAFAAA,
        0xFDDDFDDD,
        0xFFFFFFFF,
    };

    if ( volume < 0         )  volume = 0;
    if ( volume > MAXVOLUME )  volume = MAXVOLUME;

    nb = midiOutGetNumDevs();
    for ( i=0 ; i<nb ; i++ )
    {
        result = midiOutOpen((LPHMIDIOUT)&hmo, i, 0L, 0L, 0L);
        if ( result != MMSYSERR_NOERROR )
        {
            continue;
        }

        result = midiOutSetVolume(hmo, table[volume]);
        if ( result != MMSYSERR_NOERROR )
        {
            n = 1;
        }
        midiOutClose(hmo);
        hmo = 0;
    }
}

// Changes the volume of audio CD.
// The volume is between 0 and 20!
// Crashing in Vista. The current craft (if _SOUNDTRACKS = TRUE) no longer crashes,
// but this is not the correct volume which is modified!

BOOL InitAudioTrackVolume(int volume)
{
#if _SOUNDTRACKS
    MMRESULT rc;              // Return code.
    HMIXER hMixer;            // Mixer handle used in mixer API calls.
    MIXERCONTROL mxc;         // Holds the mixer control data.
    MIXERLINE mxl;            // Holds the mixer line data.
    MIXERLINECONTROLS mxlc;   // Obtains the mixer control.

    if ( volume < 0         )  volume = 0;
    if ( volume > MAXVOLUME )  volume = MAXVOLUME;

    // Open the mixer. This opens the mixer with a deviceID of 0. If you
    // have a single sound card/mixer, then this will open it. If you have
    // multiple sound cards/mixers, the deviceIDs will be 0, 1, 2, and
    // so on.
    rc = mixerOpen(&hMixer, 0,0,0,0);
    if ( rc != MMSYSERR_NOERROR )
    {
        return FALSE;  // Couldn't open the mixer.
    }

    // Initialize MIXERLINE structure.
    ZeroMemory(&mxl,sizeof(mxl));
    mxl.cbStruct = sizeof(mxl);

    // Specify the line you want to get. You are getting the input line
    // here. If you want to get the output line, you need to use
    // MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT.
    mxl.dwComponentType = MIXERLINE_COMPONENTTYPE_SRC_COMPACTDISC;

    rc = mixerGetLineInfo((HMIXEROBJ)hMixer, &mxl,
                           MIXER_GETLINEINFOF_COMPONENTTYPE);
    if ( rc != MMSYSERR_NOERROR )
    {
        return FALSE;  // Couldn't get the mixer line.
    }

    // Get the control.
    ZeroMemory(&mxlc, sizeof(mxlc));
    mxlc.cbStruct = sizeof(mxlc);
    mxlc.dwLineID = mxl.dwLineID;
//? mxlc.dwControlType = MIXERCONTROL_CONTROLTYPE_PEAKMETER;
//? mxlc.dwControlType = MIXERCONTROL_CONTROLF_UNIFORM;
    mxlc.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;
    mxlc.cControls = 1;
    mxlc.cbmxctrl = sizeof(mxc);
    mxlc.pamxctrl = &mxc;
    ZeroMemory(&mxc, sizeof(mxc));
    mxc.cbStruct = sizeof(mxc);
    rc = mixerGetLineControls((HMIXEROBJ)hMixer,&mxlc,
                               MIXER_GETLINECONTROLSF_ONEBYTYPE);
//?                            MIXER_GETLINECONTROLSF_ALL);
    if ( rc != MMSYSERR_NOERROR )
    {
        return FALSE;  // Couldn't get the control.
    }

    // After successfully getting the peakmeter control, the volume range
    // will be specified by mxc.Bounds.lMinimum to mxc.Bounds.lMaximum.

    MIXERCONTROLDETAILS mxcd;             // Gets the control values.
    MIXERCONTROLDETAILS_SIGNED volStruct; // Gets the control values.

    volStruct.lValue = volume*(mxc.Bounds.lMaximum-mxc.Bounds.lMinimum);
    volStruct.lValue /= MAXVOLUME;
    volStruct.lValue += mxc.Bounds.lMinimum;

    // Initialize the MIXERCONTROLDETAILS structure
    ZeroMemory(&mxcd, sizeof(mxcd));
    mxcd.cbStruct = sizeof(mxcd);
    mxcd.cbDetails = sizeof(volStruct);
    mxcd.dwControlID = mxc.dwControlID;
    mxcd.paDetails = &volStruct;
    mxcd.cChannels = 1;

    // Get the current value of the peakmeter control. Typically, you
    // would set a timer in your program to query the volume every 10th
    // of a second or so.
    rc = mixerSetControlDetails((HMIXEROBJ)hMixer, &mxcd,
                                 MIXER_SETCONTROLDETAILSF_VALUE);
    if ( rc != MMSYSERR_NOERROR )
    {
        return FALSE;  // Couldn't get the current volume.
    }
#endif

    return TRUE;
}


/////////////////////////////////////////////////////////////////////////////


// Constructor.

CSound::CSound(CInstanceManager* iMan)
{
    int     i;

    m_iMan = iMan;
    m_iMan->AddInstance(CLASS_SOUND, this);

    m_bEnable        = FALSE;
    m_bState         = FALSE;
    m_bAudioTrack    = TRUE;
    m_ctrl3D         = TRUE;
    m_bDebugMode     = FALSE;
    m_MidiDeviceID   = 0;
    m_MIDIMusic      = 0;
    m_audioVolume    = 20;
    m_midiVolume     = 15;
    m_lastMidiVolume = 0;
    m_listener       = 0;
    m_lastTime       = 0.0f;
    m_playTime       = 0.0f;
    m_uniqueStamp    = 0;
    m_maxSound       = MAXSOUND;
    m_eye            = D3DVECTOR(0.0f, 0.0f, 0.0f);
    m_hWnd           = 0;

    m_lpDS = NULL;

    ZeroMemory(m_channel, sizeof(SoundChannel)*MAXSOUND);
    for ( i=0 ; i<MAXSOUND ; i++ )
    {
        m_channel[i].bUsed = FALSE;
    }

    for ( i=0 ; i<MAXFILES ; i++ )
    {
        m_files[i] = 0;
    }
}

// Destructor.

CSound::~CSound()
{
    int     i;

    if ( m_bEnable )
    {
        InitMidiVolume(15);  // gives an average volume!
        InitAudioTrackVolume(15);  // gives an average volume!
    }

    for ( i=0 ; i<MAXSOUND ; i++ )
    {
        if ( m_channel[i].bUsed )
        {
            m_channel[i].soundBuffer->Stop();
            m_channel[i].soundBuffer->Release();
            m_channel[i].soundBuffer = 0;
            m_channel[i].bUsed = FALSE;
        }
    }

    if ( m_listener != NULL )
    {
        m_listener->Release();
        m_listener = NULL;
    }

    if ( m_lpDS != NULL )
    {
        m_lpDS->Release();
        m_lpDS = NULL;
    }
}


// Specifies whether you are in debug mode.

void CSound::SetDebugMode(BOOL bMode)
{
    m_bDebugMode = bMode;
}


// Initializes DirectSound.

BOOL CSound::Create(HWND hWnd, BOOL b3D)
{
    LPDIRECTSOUNDBUFFER primary;
    DSBUFFERDESC        dsbdesc;
    DSCAPS              dscaps;
    WAVEFORMATEX        wfx;
    HRESULT             hr;

    if ( !DirectSoundCreate(NULL, &m_lpDS, NULL) == DS_OK )
    {
        OutputDebugString("Fatal error: DirectSoundCreate\n");
        m_bEnable = FALSE;
        return FALSE;
    }

//? m_lpDS->SetCooperativeLevel(hWnd, DSSCL_NORMAL);
    m_lpDS->SetCooperativeLevel(hWnd, DSSCL_PRIORITY);

    if ( !RetSound3DCap() )  b3D = FALSE;

    m_ctrl3D = FALSE;
    if ( b3D )
    {
        // Obtain primary buffer, asking it for 3D control.
        ZeroMemory( &dsbdesc, sizeof(DSBUFFERDESC) );
        dsbdesc.dwSize = sizeof(DSBUFFERDESC);
        dsbdesc.dwFlags = DSBCAPS_PRIMARYBUFFER|DSBCAPS_CTRL3D;
        hr = m_lpDS->CreateSoundBuffer( &dsbdesc, &primary, NULL );
        if ( hr == S_OK )
        {
            m_ctrl3D = TRUE;
        }
    }

    if ( !m_ctrl3D )
    {
        // Obtain primary buffer, without 3D control.
        ZeroMemory( &dsbdesc, sizeof(DSBUFFERDESC) );
        dsbdesc.dwSize = sizeof(DSBUFFERDESC);
        dsbdesc.dwFlags = DSBCAPS_PRIMARYBUFFER;
        hr = m_lpDS->CreateSoundBuffer( &dsbdesc, &primary, NULL );
        if ( hr != S_OK )
        {
            return FALSE;
        }
        m_ctrl3D = FALSE;
    }

    if ( m_ctrl3D )
    {
        hr = primary->QueryInterface( IID_IDirectSound3DListener,
                                      (VOID**)&m_listener );
        if ( hr != S_OK )
        {
            primary->Release();
            return FALSE;
        }
    }

    // Set primary buffer format to 44kHz and 16-bit output.
    ZeroMemory( &wfx, sizeof(WAVEFORMATEX) );
    wfx.wFormatTag      = WAVE_FORMAT_PCM;
    wfx.nChannels       = 2;
    wfx.nSamplesPerSec  = 22050;
//? wfx.nSamplesPerSec  = 44100;
    wfx.wBitsPerSample  = 16;
    wfx.nBlockAlign     = wfx.wBitsPerSample / 8 * wfx.nChannels;
    wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
    hr = primary->SetFormat(&wfx);
    if ( hr != S_OK )
    {
        DisplayError("SetFormat", SOUND_CLICK, hr);
    }

    // Release the primary buffer, since it is not need anymore.
    primary->Release();

    // Search the maximum possible voices.
    if ( m_ctrl3D )
    {
        ZeroMemory( &dscaps, sizeof(DSCAPS) );
        dscaps.dwSize = sizeof(DSCAPS);
        hr = m_lpDS->GetCaps(&dscaps);
        if ( hr == DS_OK )
        {
            m_maxSound = dscaps.dwMaxHwMixingAllBuffers;
            if ( dscaps.dwMaxHw3DAllBuffers > 0 &&
                 m_maxSound > (int)dscaps.dwMaxHw3DAllBuffers )
            {
                m_maxSound = dscaps.dwMaxHw3DAllBuffers;
            }
            if ( m_maxSound > MAXSOUND )  m_maxSound = MAXSOUND;
        }
    }

    m_bEnable = TRUE;
    m_hWnd    = hWnd;
    return TRUE;
}


// Indicates whether to play sounds in 3D or not.

void CSound::SetSound3D(BOOL bMode)
{
    StopAll();

    if ( m_listener != NULL )
    {
        m_listener->Release();
        m_listener = NULL;
    }

    if ( m_lpDS != NULL )
    {
        m_lpDS->Release();
        m_lpDS = NULL;
    }

    Create(m_hWnd, bMode);
}

BOOL CSound::RetSound3D()
{
    return m_ctrl3D;
}

// Indicates whether it is possible to play sounds in 3D.

BOOL CSound::RetSound3DCap()
{
    DSCAPS      dscaps;
    HRESULT     hr;

    ZeroMemory( &dscaps, sizeof(DSCAPS) );
    dscaps.dwSize = sizeof(DSCAPS);
    hr = m_lpDS->GetCaps(&dscaps);
    if ( hr != DS_OK )  return FALSE;

    return ( dscaps.dwMaxHw3DAllBuffers > 0 );
}



// Returns the state of DirectSound.

BOOL CSound::RetEnable()
{
    return m_bEnable;
}


// Switches on or off the sound.

void CSound::SetState(BOOL bState)
{
    m_bState = bState;
}

// Specifies the pathname to the CD.

void CSound::SetCDpath(char *path)
{
    strcpy(m_CDpath, path);
}

// Switches on or off the CD-audio music.

void CSound::SetAudioTrack(BOOL bAudio)
{
    m_bAudioTrack = bAudio;
}


// Manages volumes of audio (. Wav) and midi (. Mid).

void CSound::SetAudioVolume(int volume)
{
    m_audioVolume = volume;
}

int CSound::RetAudioVolume()
{
    if ( !m_bEnable )  return 0;
    return m_audioVolume;
}

void CSound::SetMidiVolume(int volume)
{
    m_midiVolume = volume;

    if ( m_bAudioTrack )
    {
        InitAudioTrackVolume(m_midiVolume);
    }
}

int CSound::RetMidiVolume()
{
    if ( !m_bEnable )  return 0;
    return m_midiVolume;
}


// Reads a file.

BOOL CSound::ReadFile(Sound sound, char *metaname, char *filename)
{
    WaveHeader  wavHdr;
    DWORD       size;
    int         err;

    // Open the wave file.
    err = g_metafile.Open(metaname, filename);
    if ( err != 0 ) return FALSE;

    // Read in the wave header.
    g_metafile.Read(&wavHdr, sizeof(wavHdr));

    // Figure out the size of the data region.
    size = wavHdr.dwDSize;

    if ( m_files[sound] != 0 )
    {
        free(m_files[sound]);
    }
    m_files[sound] = (char*)malloc(sizeof(WaveHeader)+size);

    memcpy(m_files[sound], &wavHdr, sizeof(WaveHeader));
    g_metafile.Read(m_files[sound]+sizeof(WaveHeader), size);

    // Close out the wave file.
    g_metafile.Close();
    return TRUE;
}

// Hides all sound files (. Wav).

void CSound::CacheAll()
{
    int         i;
    char        meta[50];
    char        name[50];

    if ( !m_bEnable )  return;

    if ( m_bDebugMode )
    {
        strcpy(meta, "");
    }
    else
    {
#if _SCHOOL
        strcpy(meta, "ceebot3.dat");
#else
        strcpy(meta, "colobot3.dat");
#endif
    }

    for ( i=0 ; i<MAXFILES ; i++ )
    {
        if ( m_bDebugMode )
        {
            sprintf(name, "sound\\sound%.3d.wav", i);
        }
        else
        {
            sprintf(name, "sound%.3d.wav", i);
        }
        if ( !ReadFile((Sound)i, meta, name) )  break;
    }
}


// Return the priority of a sound.
// The higher the value, the greater the sound is important.

int CSound::RetPriority(Sound sound)
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

// Seeks a free buffer.

BOOL CSound::SearchFreeBuffer(Sound sound, int &channel, BOOL &bAlreadyLoaded)
{
    DWORD   status;
    int     i, priority;

    priority = RetPriority(sound);

#if 1
    // Seeks a channel used which sound is stopped.
    for ( i=0 ; i<m_maxSound ; i++ )
    {
        if ( !m_channel[i].bUsed )  continue;
        if ( m_channel[i].type != sound )  continue;

        m_channel[i].soundBuffer->GetStatus(&status);
        if ( (status&DSBSTATUS_PLAYING) == 0 )
        {
            m_channel[i].priority = priority;
            m_channel[i].uniqueStamp = m_uniqueStamp++;
            channel = i;
            bAlreadyLoaded = TRUE;
            return TRUE;
        }
    }
#endif

    // Seeks a channel completely free.
    for ( i=0 ; i<m_maxSound ; i++ )
    {
        if ( !m_channel[i].bUsed )
        {
            m_channel[i].priority = priority;
            m_channel[i].uniqueStamp = m_uniqueStamp++;
            channel = i;
            bAlreadyLoaded = FALSE;
            return TRUE;
        }
    }

    // Seeks a channel used which sound is stopped.
    for ( i=0 ; i<m_maxSound ; i++ )
    {
        if ( !m_channel[i].bUsed )  continue;

        m_channel[i].soundBuffer->GetStatus(&status);
        if ( (status&DSBSTATUS_PLAYING) == 0 )
        {
            m_channel[i].soundBuffer->Release();
            m_channel[i].soundBuffer = 0;
            m_channel[i].priority = priority;
            m_channel[i].uniqueStamp = m_uniqueStamp++;

            channel = i;
            bAlreadyLoaded = FALSE;
            return TRUE;
        }
    }

    // Seeks a lower priority channel used.
    for ( i=0 ; i<m_maxSound ; i++ )
    {
        if ( !m_channel[i].bUsed )  continue;
        if ( m_channel[i].priority >= priority )  continue;

        m_channel[i].soundBuffer->Stop();
        m_channel[i].soundBuffer->Release();
        m_channel[i].soundBuffer = 0;
        m_channel[i].priority = priority;
        m_channel[i].uniqueStamp = m_uniqueStamp++;

        channel = i;
        bAlreadyLoaded = FALSE;
        return TRUE;
    }

    // Seeks a channel used the same or lower priority.
    for ( i=0 ; i<m_maxSound ; i++ )
    {
        if ( !m_channel[i].bUsed )  continue;
        if ( m_channel[i].priority > priority )  continue;

        m_channel[i].soundBuffer->Stop();
        m_channel[i].soundBuffer->Release();
        m_channel[i].soundBuffer = 0;
        m_channel[i].priority = priority;
        m_channel[i].uniqueStamp = m_uniqueStamp++;

        channel = i;
        bAlreadyLoaded = FALSE;
        return TRUE;
    }

    char s[100];
    sprintf(s, "Sound %d forget (priority=%d)\n", sound, priority);
    OutputDebugString(s);

    return FALSE;
}

// Reads in data from a wave file.

BOOL CSound::ReadData(LPDIRECTSOUNDBUFFER lpDSB, Sound sound, DWORD size)
{
    LPVOID  pData1;
    DWORD   dwData1Size;
    LPVOID  pData2;
    DWORD   dwData2Size;
    HRESULT hr;

    // Lock data in buffer for writing.
    hr = lpDSB->Lock(0, size, &pData1, &dwData1Size, &pData2, &dwData2Size, DSBLOCK_FROMWRITECURSOR);
    if ( hr != DS_OK )
    {
        return FALSE;
    }

    // Read in first chunk of data.
    if ( dwData1Size > 0 )
    {
        memcpy(pData1, m_files[sound]+sizeof(WaveHeader), dwData1Size);
    }

    // Read in second chunk if necessary.
    if ( dwData2Size > 0 )
    {
        memcpy(pData2, m_files[sound]+sizeof(WaveHeader)+dwData1Size, dwData2Size);
    }

    // Unlock data in buffer.
    hr = lpDSB->Unlock(pData1, dwData1Size, pData2, dwData2Size);
    if ( hr != DS_OK )
    {
        return FALSE;
    }

    return TRUE;
}

// Creates a DirectSound buffer.

BOOL CSound::CreateSoundBuffer(int channel, DWORD size, DWORD freq,
                               DWORD bitsPerSample, DWORD blkAlign,
                               BOOL bStereo)
{
    PCMWAVEFORMAT   pcmwf;
    DSBUFFERDESC    dsbdesc;
    DS3DBUFFER      bufferParams;  // 3D buffer properties
    HRESULT         hr;

    // Set up wave format structure.
    memset( &pcmwf, 0, sizeof(PCMWAVEFORMAT) );
    pcmwf.wf.wFormatTag      = WAVE_FORMAT_PCM;
    pcmwf.wf.nChannels       = bStereo ? 2 : 1;
    pcmwf.wf.nSamplesPerSec  = freq;
    pcmwf.wf.nBlockAlign     = (WORD)blkAlign;
    pcmwf.wf.nAvgBytesPerSec = pcmwf.wf.nSamplesPerSec * pcmwf.wf.nBlockAlign;
    pcmwf.wBitsPerSample     = (WORD)bitsPerSample;

    // Set up DSBUFFERDESC structure.
    memset(&dsbdesc, 0, sizeof(DSBUFFERDESC));  // Zero it out.
    dsbdesc.dwSize = sizeof(DSBUFFERDESC);
    if ( m_ctrl3D )
    {
        dsbdesc.dwFlags = DSBCAPS_CTRL3D|DSBCAPS_MUTE3DATMAXDISTANCE|
                          DSBCAPS_LOCDEFER|
                          DSBCAPS_CTRLVOLUME|DSBCAPS_CTRLFREQUENCY;
    }
    else
    {
        dsbdesc.dwFlags = DSBCAPS_CTRLVOLUME|DSBCAPS_CTRLPAN|DSBCAPS_CTRLFREQUENCY;
    }
    dsbdesc.dwBufferBytes = size;
    dsbdesc.lpwfxFormat   = (LPWAVEFORMATEX)&pcmwf;

    hr = m_lpDS->CreateSoundBuffer(&dsbdesc, &m_channel[channel].soundBuffer, NULL);
    if ( hr != DS_OK )  return FALSE;

    if ( m_ctrl3D )
    {
        hr = m_channel[channel].soundBuffer->QueryInterface
                            (
                                IID_IDirectSound3DBuffer,
                                (VOID**)&m_channel[channel].soundBuffer3D
                            );
        if ( hr != DS_OK )  return FALSE;
    }

    m_channel[channel].bUsed = TRUE;
    m_channel[channel].bMute = FALSE;
    return TRUE;
}

// Creates a DirectSound buffer from a wave file.

BOOL CSound::CreateBuffer(int channel, Sound sound)
{
    WaveHeader* wavHdr;
    DWORD       size;
    BOOL        bStereo;

    if ( m_files[sound] == 0 )  return FALSE;

    wavHdr = (WaveHeader*)m_files[sound];
    size = wavHdr->dwDSize;
    bStereo = wavHdr->wChnls > 1 ? TRUE : FALSE;

    // Create the sound buffer for the wave file.
    if ( !CreateSoundBuffer(channel, size, wavHdr->dwSRate,
                            wavHdr->BitsPerSample, wavHdr->wBlkAlign, bStereo) )
    {
        return FALSE;
    }

    // Read the data for the wave file into the sound buffer.
    if ( !ReadData(m_channel[channel].soundBuffer, sound, size) )
    {
        return FALSE;
    }

    m_channel[channel].type = sound;

    // Close out the wave file.
    return TRUE;
}

// Calculates the volume and pan of a sound, non-3D mode.

void CSound::ComputeVolumePan2D(int channel, const D3DVECTOR &pos)
{
    float   dist, a, g;

    if ( pos.x == m_eye.x &&
         pos.y == m_eye.y &&
         pos.z == m_eye.z )
    {
        m_channel[channel].volume = 1.0f;  // maximum volume
        m_channel[channel].pan    = 0.0f;  // at the center
        return;
    }

#if _TEEN
    dist = Length(pos, m_eye);
    if ( dist >= 210.0f )  // very far?
    {
        m_channel[channel].volume = 0.0f;  // silence
        m_channel[channel].pan    = 0.0f;  // at the center
        return;
    }
    if ( dist <= 10.0f )  // very close?
    {
        m_channel[channel].volume = 1.0f;  // maximum volume
        m_channel[channel].pan    = 0.0f;  // at the center
        return;
    }
    m_channel[channel].volume = 1.0f-((dist-10.0f)/200.0f);
#else
    dist = Length(pos, m_eye);
    if ( dist >= 110.0f )  // very far?
    {
        m_channel[channel].volume = 0.0f;  // silence
        m_channel[channel].pan    = 0.0f;  // at the center
        return;
    }
    if ( dist <= 10.0f )  // very close?
    {
        m_channel[channel].volume = 1.0f;  // maximum volume
        m_channel[channel].pan    = 0.0f;  // at the center
        return;
    }
    m_channel[channel].volume = 1.0f-((dist-10.0f)/100.0f);
#endif

    a = RotateAngle(m_lookat.x-m_eye.x, m_eye.z-m_lookat.z);
    g = RotateAngle(pos.x-m_eye.x, m_eye.z-pos.z);
    m_channel[channel].pan = sinf(Direction(a, g));
}

// Sounds in the middle.
// Returns the associated channel or -1.

int CSound::Play(Sound sound, float amplitude, float frequency, BOOL bLoop)
{
    return Play(sound, m_lookat, amplitude, frequency, bLoop);
}

// Sounds at a given position.
// Returns the associated channel or -1.

int CSound::Play(Sound sound, D3DVECTOR pos,
                 float amplitude, float frequency, BOOL bLoop)
{
    DS3DBUFFER  sb;
    int         channel, iVolume, iPan, iFreq, uniqueStamp;
    BOOL        bAlreadyLoaded;
    DWORD       flag, freq;
    HRESULT     err;

    if ( !m_bEnable )  return -1;
    if ( !m_bState || m_audioVolume == 0 )  return -1;

//? if ( Length(pos, m_eye) > 100.0f )  return -1;

    if ( !SearchFreeBuffer(sound, channel, bAlreadyLoaded) )  return -1;

    if ( !bAlreadyLoaded )
    {
        if ( !CreateBuffer(channel, sound) )
        {
            if ( m_channel[channel].bUsed            &&
                 m_channel[channel].soundBuffer != 0 )
            {
                m_channel[channel].soundBuffer->Release();
                m_channel[channel].soundBuffer = 0;
            }
            m_channel[channel].bUsed = FALSE;
            return -1;
        }
    }

    m_channel[channel].pos = pos;

    if ( m_ctrl3D )
    {
        m_channel[channel].volume = 1.0f;
        m_channel[channel].pan    = 0.0f;
    }
    else
    {
        ComputeVolumePan2D(channel, pos);
    }

#if 0
    DWORD status;
    m_channel[channel].soundBuffer->GetStatus(&status);
    char s[100];
    sprintf(s, "Play sound=%d status=%d channel=%d flag=%d\n", sound, status, channel, bAlreadyLoaded);
    OutputDebugString(s);
#endif

    m_channel[channel].oper[0].bUsed = FALSE;
    m_channel[channel].startAmplitude = amplitude;
    m_channel[channel].startFrequency = frequency;
    m_channel[channel].changeFrequency = 1.0f;

    if ( m_ctrl3D )
    {
        sb.dwSize = sizeof(DS3DBUFFER);
        err = m_channel[channel].soundBuffer3D->GetAllParameters(&sb);
        DisplayError("GetAllParameters", sound, err);

        sb.vPosition = pos;
//?     sb.dwInsideConeAngle = 90;
//?     sb.dwOutsideConeAngle = 180;
//?     sb.vConeOrientation = D3DVECTOR(0.0f, 1.0f, 0.0f);
        sb.lConeOutsideVolume = DSBVOLUME_MIN;
#if _TEEN
        sb.flMinDistance = 50.0f;
#else
        sb.flMinDistance = 20.0f;
#endif
        sb.flMaxDistance = DS3D_DEFAULTMAXDISTANCE;

        err = m_channel[channel].soundBuffer3D->SetAllParameters(&sb, DS3D_IMMEDIATE);
        DisplayError("SetAllParameters", sound, err);
    }

    amplitude *= m_channel[channel].volume;
    amplitude *= (float)m_audioVolume/MAXVOLUME;
    iVolume = (int)((powf(amplitude, 0.2f)-1.0f)*10000.0f);
    if ( iVolume > 0 )  iVolume = 0;
    err = m_channel[channel].soundBuffer->SetVolume(iVolume);
    DisplayError("SetVolume", sound, err);

    if ( !m_ctrl3D )
    {
        iPan = (int)(m_channel[channel].pan*10000.0f);
        err = m_channel[channel].soundBuffer->SetPan(iPan);
        DisplayError("SetPan", sound, err);
    }

    if ( !bAlreadyLoaded )
    {
        err = m_channel[channel].soundBuffer->GetFrequency(&freq);
        DisplayError("GetFrequency", sound, err);
        m_channel[channel].initFrequency = freq;
    }
    iFreq = (int)(frequency*m_channel[channel].initFrequency);
    err = m_channel[channel].soundBuffer->SetFrequency(iFreq);
    DisplayError("SetFrequency", sound, err);

    err = m_channel[channel].soundBuffer->SetCurrentPosition(0);
    DisplayError("SetCurrentPosition", sound, err);

    flag = bLoop?DSBPLAY_LOOPING:0;
//? flag |= DSBPLAY_LOCHARDWARE|DSBPLAY_TERMINATEBY_DISTANCE;
//? flag |= DSBPLAY_TERMINATEBY_DISTANCE;
    err = m_channel[channel].soundBuffer->Play(0, 0, flag);
    DisplayError("Play", sound, err);
    if ( err == DSERR_BADFORMAT )
    {
        iFreq = m_channel[channel].initFrequency;
        err = m_channel[channel].soundBuffer->SetFrequency(iFreq);
        DisplayError("SetFrequency (repeat)", sound, err);

        err = m_channel[channel].soundBuffer->Play(0, 0, flag);
        DisplayError("Play (repeat)", sound, err);
    }

    uniqueStamp = m_channel[channel].uniqueStamp;
    return channel | ((uniqueStamp&0xffff)<<16);
}

// Check a channel number.
// Adapts the channel, so it can be used as an offset in m_channel.

BOOL CSound::CheckChannel(int &channel)
{
    int     uniqueStamp;

    uniqueStamp = (channel>>16)&0xffff;
    channel &= 0xffff;

    if ( !m_bEnable )  return FALSE;
    if ( !m_bState || m_audioVolume == 0 )  return FALSE;

    if ( channel < 0 || channel >= m_maxSound )  return FALSE;
    if ( !m_channel[channel].bUsed )  return FALSE;

    if ( m_channel[channel].uniqueStamp != uniqueStamp )  return FALSE;

    return TRUE;
}

// Removes all envelopes.

BOOL CSound::FlushEnvelope(int channel)
{
    if ( !CheckChannel(channel) )  return FALSE;

    m_channel[channel].oper[0].bUsed = FALSE;
    return TRUE;
}

// Adds an operation envelope.

BOOL CSound::AddEnvelope(int channel, float amplitude, float frequency,
                         float time, SoundNext oper)
{
    int     i;

    if ( !CheckChannel(channel) )  return FALSE;

    for ( i=0 ; i<MAXOPER ; i++ )
    {
        if ( m_channel[channel].oper[i].bUsed )  continue;

        m_channel[channel].oper[i].bUsed = TRUE;
        m_channel[channel].oper[i].finalAmplitude = amplitude;
        m_channel[channel].oper[i].finalFrequency = frequency;
        m_channel[channel].oper[i].totalTime = time;
        m_channel[channel].oper[i].currentTime = 0;
        m_channel[channel].oper[i].nextOper = oper;

        if ( i < MAXOPER-1 )
        {
            m_channel[channel].oper[i+1].bUsed = FALSE;
        }
        return TRUE;
    }
    return FALSE;
}

// Changes the position of a sound.

BOOL CSound::Position(int channel, D3DVECTOR pos)
{
    float       amplitude, pan;
    int         iVolume, iPan;
    HRESULT     err;

    if ( !CheckChannel(channel) )  return FALSE;

    m_channel[channel].pos = pos;

    if ( m_ctrl3D )
    {
        m_channel[channel].soundBuffer3D->SetPosition(pos.x, pos.y, pos.z, DS3D_DEFERRED);
    }
    else
    {
        ComputeVolumePan2D(channel, pos);

        if ( !m_channel[channel].oper[0].bUsed )
        {
            amplitude = m_channel[channel].startAmplitude;
            amplitude *= m_channel[channel].volume;
            amplitude *= (float)m_audioVolume/MAXVOLUME;
            iVolume = (int)((powf(amplitude, 0.2f)-1.0f)*10000.0f);
            if ( iVolume > 0 )  iVolume = 0;
            err = m_channel[channel].soundBuffer->SetVolume(iVolume);
            DisplayError("SetVolume", m_channel[channel].type, err);
        }

        pan = m_channel[channel].pan;
        iPan = (int)(pan*10000.0f);
        err = m_channel[channel].soundBuffer->SetPan(iPan);
        DisplayError("SetPan", m_channel[channel].type, err);
    }
    return TRUE;
}

// Changes the frequency of a sound.
// 0.5 down of an octave and 2.0 up of an octave.

BOOL CSound::Frequency(int channel, float frequency)
{
    HRESULT err;
    int     iFreq;

    if ( !CheckChannel(channel) )  return FALSE;

    m_channel[channel].changeFrequency = frequency;

    if ( !m_channel[channel].oper[0].bUsed )
    {
        iFreq = (int)(frequency*m_channel[channel].initFrequency);
        err = m_channel[channel].soundBuffer->SetFrequency(iFreq);
        DisplayError("Frequency", m_channel[channel].type, err);
    }

    return TRUE;
}

// Stops sound.

BOOL CSound::Stop(int channel)
{
    if ( !CheckChannel(channel) )  return FALSE;

    m_channel[channel].soundBuffer->Stop();
    return TRUE;
}

// Stops all sounds.

BOOL CSound::StopAll()
{
    DWORD   status;
    int     i;

    for ( i=0 ; i<MAXSOUND ; i++ )
    {
        if ( !m_channel[i].bUsed )  continue;

        m_channel[i].soundBuffer->GetStatus(&status);
        if ( (status&DSBSTATUS_PLAYING) == DSBSTATUS_PLAYING )
        {
            m_channel[i].soundBuffer->Stop();
        }
        m_channel[i].soundBuffer->Stop();
        m_channel[i].soundBuffer->Release();
        m_channel[i].soundBuffer = 0;

        m_channel[i].bUsed = FALSE;
    }
    return TRUE;
}

// Silent all sounds.

BOOL CSound::MuteAll(BOOL bMute)
{
    int     i;

    for ( i=0 ; i<MAXSOUND ; i++ )
    {
        if ( !m_channel[i].bUsed )  continue;

        m_channel[i].bMute = bMute;
    }
    return TRUE;
}


// Passes the following operation for a channel.

void CSound::OperNext(int channel)
{
    int     i;

    m_channel[channel].startAmplitude = m_channel[channel].oper[0].finalAmplitude;
    m_channel[channel].startFrequency = m_channel[channel].oper[0].finalFrequency;

    for ( i=0 ; i<MAXOPER-1 ; i++ )
    {
        if ( !m_channel[channel].oper[i+1].bUsed )  break;

        m_channel[channel].oper[i] = m_channel[channel].oper[i+1];
    }

    m_channel[channel].oper[i].bUsed = FALSE;
}

// Updates the sound buffers.

void CSound::FrameMove(float rTime)
{
    HRESULT     err;
    SoundNext   next;
    float       progress, volume, freq;
    int         i, iVolume, iFreq;

    m_playTime += rTime;

    for ( i=0 ; i<m_maxSound ; i++ )
    {
        if ( !m_channel[i].bUsed )  continue;
        if ( !m_channel[i].oper[0].bUsed )  continue;

        if ( m_channel[i].bMute )
        {
            m_channel[i].soundBuffer->SetVolume(-10000);  // silence
            continue;
        }

        m_channel[i].oper[0].currentTime += rTime;

        progress = m_channel[i].oper[0].currentTime / m_channel[i].oper[0].totalTime;
        if ( progress > 1.0f )  progress = 1.0f;

        volume = progress;
        volume *= m_channel[i].oper[0].finalAmplitude-m_channel[i].startAmplitude;
        volume += m_channel[i].startAmplitude;
        volume *= m_channel[i].volume;
        volume *= (float)m_audioVolume/MAXVOLUME;
        iVolume = (int)((powf(volume, 0.2f)-1.0f)*10000.0f);
        if ( iVolume > 0 )  iVolume = 0;
        m_channel[i].soundBuffer->SetVolume(iVolume);

        freq = progress;
        freq *= m_channel[i].oper[0].finalFrequency-m_channel[i].startFrequency;
        freq += m_channel[i].startFrequency;
        freq *= m_channel[i].changeFrequency;
        iFreq = (int)(freq*m_channel[i].initFrequency);
        err = m_channel[i].soundBuffer->SetFrequency(iFreq);
        DisplayError("FrameMove::Frequency", m_channel[i].type, err);

        if ( m_channel[i].oper[0].currentTime >=
             m_channel[i].oper[0].totalTime )
        {
            next = m_channel[i].oper[0].nextOper;

            if ( next == SOPER_LOOP )
            {
                m_channel[i].oper[0].currentTime = 0.0f;
            }
            else
            {
                OperNext(i);

                if ( next == SOPER_STOP )
                {
                    m_channel[i].soundBuffer->Stop();
                }
            }
        }
    }

    m_lastTime += rTime;
    if ( m_lastTime >= 0.05f && m_listener != 0 )
    {
        m_lastTime = 0.0f;
        m_listener->CommitDeferredSettings();
    }
}

// Specifies the position of the listener.
// Must be called whenever the camera moves.

void CSound::SetListener(D3DVECTOR eye, D3DVECTOR lookat)
{
    DS3DLISTENER    listenerParams;
    HRESULT         err;
    float           amplitude, pan;
    int             i, iVolume, iPan;

    m_eye = eye;
    m_lookat = lookat;

    if ( m_listener == 0 )
    {
        if ( m_ctrl3D )  return;

        for ( i=0 ; i<m_maxSound ; i++ )
        {
            if ( !m_channel[i].bUsed )  continue;

            ComputeVolumePan2D(i, m_channel[i].pos);

            if ( !m_channel[i].oper[0].bUsed )
            {
                amplitude = m_channel[i].startAmplitude;
                amplitude *= m_channel[i].volume;
                amplitude *= (float)m_audioVolume/MAXVOLUME;
                iVolume = (int)((powf(amplitude, 0.2f)-1.0f)*10000.0f);
                if ( iVolume > 0 )  iVolume = 0;
                err = m_channel[i].soundBuffer->SetVolume(iVolume);
                DisplayError("SetVolume", m_channel[i].type, err);
            }

            pan = m_channel[i].pan;
            iPan = (int)(pan*10000.0f);
            err = m_channel[i].soundBuffer->SetPan(iPan);
            DisplayError("SetPan", m_channel[i].type, err);
        }
        return;
    }

    // Get listener parameters.
    listenerParams.dwSize = sizeof(DS3DLISTENER);
    m_listener->GetAllParameters(&listenerParams);

    listenerParams.vPosition = eye;
    listenerParams.vOrientFront = lookat-eye;
    listenerParams.vOrientTop = D3DVECTOR(0.0f, 1.0f, 0.0f);
    listenerParams.flDistanceFactor = 10.0f;
    listenerParams.flRolloffFactor = 1.0f;

    m_listener->SetAllParameters(&listenerParams, DS3D_DEFERRED);
}




// Uses MCI to play a MIDI file. The window procedure
// is notified when playback is complete.

BOOL CSound::PlayMusic(int rank, BOOL bRepeat)
{
    MCI_OPEN_PARMS  mciOpenParms;
    MCI_PLAY_PARMS  mciPlayParms;
    DWORD           dwReturn;
    char            filename[MAX_PATH];

    m_bRepeatMusic = bRepeat;
    m_playTime = 0.0f;

    if ( m_midiVolume == 0 )  return TRUE;

    if ( m_bAudioTrack )
    {
        return PlayAudioTrack(rank);
    }

    if ( !m_bEnable )  return TRUE;
    InitMidiVolume(m_midiVolume);
    m_lastMidiVolume = m_midiVolume;

    GetCurrentDir(filename, MAX_PATH-30);
    sprintf(filename+strlen(filename), "sound\\music%.3d.blp", rank-1);

    // Open the device by specifying the device and filename.
    // MCI will attempt to choose the MIDI mapper as the output port.
    mciOpenParms.lpstrDeviceType = "sequencer";
    mciOpenParms.lpstrElementName = filename;
    dwReturn = mciSendCommand(NULL,
                              MCI_OPEN,
                              MCI_OPEN_TYPE|MCI_OPEN_ELEMENT,
                              (DWORD)(LPVOID)&mciOpenParms);
    if ( dwReturn != 0 )
    {
        mciGetErrorString(dwReturn, filename, 128);
        // Failed to open device. Don't close it; just return error.
        return FALSE;
    }

    // The device opened successfully; get the device ID.
    m_MidiDeviceID = mciOpenParms.wDeviceID;

    // Begin playback.
    mciPlayParms.dwCallback = (DWORD)m_hWnd;
    dwReturn = mciSendCommand(m_MidiDeviceID,
                              MCI_PLAY,
                              MCI_NOTIFY,
                              (DWORD)(LPVOID)&mciPlayParms);
    if ( dwReturn != 0 )
    {
        mciGetErrorString(dwReturn, filename, 128);
        StopMusic();
        return FALSE;
    }

    m_MIDIMusic = rank;
    return TRUE;
}

// Uses MCI to play a CD-audio track. The window procedure
// is notified when playback is complete.
// The rank parameter is in space [1..n] !
// For CD mix (data/audio), it will be [2..n] !

BOOL CSound::PlayAudioTrack(int rank)
{
#if _SOUNDTRACKS
    MCI_OPEN_PARMS  mciOpenParms;
    MCI_PLAY_PARMS  mciPlayParms;
    MCI_SET_PARMS   mciSetParms;
    DWORD           dwReturn;
    char            filename[MAX_PATH];
    char            device[10];

    if ( !m_bEnable )  return TRUE;
//? if ( m_midiVolume == 0 )  return TRUE;
    InitAudioTrackVolume(m_midiVolume);
    m_lastMidiVolume = m_midiVolume;

    // Open the device by specifying the device and filename.
    // MCI will attempt to choose the MIDI mapper as the output port.
    memset(&mciOpenParms, 0, sizeof(MCI_OPEN_PARMS));
//? mciOpenParms.lpstrDeviceType = (LPCTSTR)MCI_DEVTYPE_CD_AUDIO;
//? dwReturn = mciSendCommand(NULL,
//?                           MCI_OPEN,
//?                           MCI_OPEN_TYPE|MCI_OPEN_TYPE_ID,
//?                           (DWORD)(LPVOID)&mciOpenParms);
    mciOpenParms.lpstrDeviceType = (LPCTSTR)MCI_DEVTYPE_CD_AUDIO;
    if ( m_CDpath[0] == 0 )
    {
        dwReturn = mciSendCommand(NULL,
                                  MCI_OPEN,
                                  MCI_OPEN_TYPE|MCI_OPEN_TYPE_ID,
                                  (DWORD)(LPVOID)&mciOpenParms);
    }
    else
    {
        device[0] = m_CDpath[0];
        device[1] = ':';
        device[2] = 0;
        mciOpenParms.lpstrElementName = device;
        dwReturn = mciSendCommand(NULL,
                                  MCI_OPEN,
                                  MCI_OPEN_TYPE|MCI_OPEN_TYPE_ID|MCI_OPEN_ELEMENT,
                                  (DWORD)(LPVOID)&mciOpenParms);
    }
    if ( dwReturn != 0 )
    {
        mciGetErrorString(dwReturn, filename, 128);
        // Failed to open device. Don't close it; just return error.
        return FALSE;
    }

    // The device opened successfully; get the device ID.
    m_MidiDeviceID = mciOpenParms.wDeviceID;

    // Set the time format to track/minute/second/frame (TMSF).
    memset(&mciSetParms, 0, sizeof(MCI_SET_PARMS));
    mciSetParms.dwTimeFormat = MCI_FORMAT_TMSF;
    dwReturn = mciSendCommand(m_MidiDeviceID,
                              MCI_SET,
                              MCI_SET_TIME_FORMAT,
                              (DWORD)&mciSetParms);
    if ( dwReturn != 0 )
    {
        mciGetErrorString(dwReturn, filename, 128);
        StopMusic();
        return FALSE;
    }

    // Begin playback.
    memset(&mciPlayParms, 0, sizeof(MCI_PLAY_PARMS));
    mciPlayParms.dwCallback = (DWORD)m_hWnd;
    mciPlayParms.dwFrom = MCI_MAKE_TMSF(rank+0, 0, 0, 0);
    mciPlayParms.dwTo   = MCI_MAKE_TMSF(rank+1, 0, 0, 0);
    dwReturn = mciSendCommand(m_MidiDeviceID,
                              MCI_PLAY,
                              MCI_NOTIFY|MCI_FROM|MCI_TO,
                              (DWORD)(LPVOID)&mciPlayParms);
    if ( dwReturn != 0 )
    {
        mciGetErrorString(dwReturn, filename, 128);
        StopMusic();
        return FALSE;
    }

    m_MIDIMusic = rank;
#endif
    return TRUE;
}

// Restart the MIDI player.

BOOL CSound::RestartMusic()
{
    if ( !m_bRepeatMusic )  return FALSE;

    OutputDebugString("RestartMusic\n");
    if ( !m_bEnable )  return TRUE;
//? if ( m_midiVolume == 0 )  return TRUE;
    if ( m_MIDIMusic == 0 )  return FALSE;
    if ( m_playTime < 5.0f )  return FALSE;

    return PlayMusic(m_MIDIMusic, TRUE);
}

// Shuts down the MIDI player.

void CSound::SuspendMusic()
{
    if ( !m_bEnable )  return;

//? if ( m_MidiDeviceID && m_midiVolume != 0 )
    if ( m_MidiDeviceID )
    {
        if ( m_bAudioTrack )  mciSendCommand(m_MidiDeviceID, MCI_STOP, 0, NULL);
        mciSendCommand(m_MidiDeviceID, MCI_CLOSE, 0, NULL);
    }
    m_MidiDeviceID = 0;
}

// Shuts down the MIDI player.

void CSound::StopMusic()
{
    SuspendMusic();
    m_MIDIMusic = 0;
}

// Returns TRUE if the music is in progress.

BOOL CSound::IsPlayingMusic()
{
    return (m_MIDIMusic != 0);
}

// Adjusts the volume of currently music, if necessary.

void CSound::AdaptVolumeMusic()
{
    if ( m_midiVolume != m_lastMidiVolume )
    {
        if ( m_bAudioTrack )
        {
            InitAudioTrackVolume(m_midiVolume);
        }
        else
        {
            InitMidiVolume(m_midiVolume);
        }
        m_lastMidiVolume = m_midiVolume;
        RestartMusic();
    }
}

