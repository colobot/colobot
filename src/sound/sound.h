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
 * \file sound/sound.h
 * \brief Sound plugin interface
 */

#pragma once

#include "math/vector.h"

#include "common/logger.h"

#include <string>

/*!
 * Maximum possible audio volume
 */
const float MAXVOLUME = 100.0f;


/**
 * \public
 * \enum    Sound   sound/sound.h
 * \brief   Sound enum representing sound file
**/
enum Sound
{
  SOUND_NONE = -1,
  SOUND_CLICK = 0,
  SOUND_BOUM  = 1,
  SOUND_EXPLO  = 2,
  SOUND_FLYh  = 3, /*!< human */
  SOUND_FLY  = 4,
  SOUND_STEPs  = 5, /*!< smooth */
  SOUND_MOTORw  = 6, /*!< wheel */
  SOUND_MOTORt  = 7, /*!< tank */
  SOUND_MOTORr  = 8, /*!< roller */
  SOUND_ERROR  = 9,
  SOUND_CONVERT  = 10,
  SOUND_ENERGY  = 11,
  SOUND_PLOUF  = 12,
  SOUND_BLUP  = 13,
  SOUND_WARNING  = 14,
  SOUND_DERRICK  = 15,
  SOUND_LABO  = 16,
  SOUND_STATION  = 17,
  SOUND_REPAIR  = 18,
  SOUND_RESEARCH  = 19,
  SOUND_INSECTs  = 20, /*!< spider */
  SOUND_BURN  = 21,
  SOUND_TZOING  = 22,
  SOUND_GGG  = 23,
  SOUND_MANIP  = 24,
  SOUND_FIRE  = 25, /*!< shooting with fireball */
  SOUND_HUMAN1  = 26, /*!< breathing */
  SOUND_STEPw  = 27, /*!< water */
  SOUND_SWIM  = 28,
  SOUND_RADAR  = 29,
  SOUND_BUILD  = 30,
  SOUND_ALARM  = 31, /*!< energy alarm */
  SOUND_SLIDE  = 32,
  SOUND_EXPLOi  = 33, /*!< insect */
  SOUND_INSECTa  = 34, /*!< ant */
  SOUND_INSECTb  = 35, /*!< bee */
  SOUND_INSECTw  = 36, /*!< worm */
  SOUND_INSECTm  = 37, /*!< mother */
  SOUND_TREMBLE  = 38,
  SOUND_PSHHH  = 39,
  SOUND_NUCLEAR  = 40,
  SOUND_INFO  = 41,
  SOUND_OPEN  = 42,
  SOUND_CLOSE  = 43,
  SOUND_FACTORY  = 44,
  SOUND_EGG  = 45,
  SOUND_MOTORs  = 46, /*!< submarine */
  SOUND_MOTORi  = 47, /*!< insect (legs) */
  SOUND_SHIELD  = 48,
  SOUND_FIREi  = 49, /*!< shooting with orgaball (insect) */
  SOUND_GUNDEL  = 50,
  SOUND_PSHHH2  = 51, /*!< shield */
  SOUND_MESSAGE  = 52,
  SOUND_BOUMm  = 53, /*!< metal */
  SOUND_BOUMv  = 54, /*!< plant */
  SOUND_BOUMs  = 55, /*!< smooth */
  SOUND_EXPLOl  = 56, /*!< little */
  SOUND_EXPLOlp  = 57, /*!< little power */
  SOUND_EXPLOp  = 58, /*!< power */
  SOUND_STEPh  = 59, /*!< hard */
  SOUND_STEPm  = 60, /*!< metal */
  SOUND_POWERON  = 61,
  SOUND_POWEROFF  = 62,
  SOUND_AIE  = 63,
  SOUND_WAYPOINT  = 64,
  SOUND_RECOVER  = 65,
  SOUND_DEADi  = 66,
  SOUND_JOSTLE  = 67,
  SOUND_GFLAT  = 68,
  SOUND_DEADg  = 69, /*!< shooting death */
  SOUND_DEADw  = 70, /*!< drowning */
  SOUND_FLYf  = 71, /*!< reactor fail */
  SOUND_ALARMt  = 72, /*!< temperature alarm */
  SOUND_FINDING  = 73, /*!< finds a cache object */
  SOUND_THUMP  = 74,
  SOUND_TOUCH  = 75,
  SOUND_BLITZ  = 76,
  SOUND_MUSHROOM  = 77,
  SOUND_FIREp  = 78, /*!< shooting with phazer */
  SOUND_EXPLOg1  = 79, /*!< impact gun 1 */
  SOUND_EXPLOg2  = 80, /*!< impact gun 2 */
//  SOUND_MOTORd  = 81, /*!< engine friction */
  SOUND_MAX /** number of items in enum */
};


/**
 * \enum    SoundNext
 * \brief   Enum representing operation that will be performend on a sound at given time
**/
enum SoundNext
{
  SOPER_CONTINUE = 1, /*!< continue playing */
  SOPER_STOP = 2, /*!< stop playing */
  SOPER_LOOP = 3, /*!< start over */
};


/**
* \class CSoundInterface
*
* \brief Sound plugin interface
*
*/
class CSoundInterface
{
public:
    CSoundInterface();
    virtual ~CSoundInterface();

    /** Function to initialize sound device
     */
    virtual bool Create();

    /** Function called to cache all sound effect files.
     *  Function calls \link CSoundInterface::Cache() \endlink for each file
     */
    void CacheAll();

    /** Function called to add all music files to list */
    void AddMusicFiles();

    /** Function called to cache sound effect file.
     *  This function is called by plugin interface for each file.
     * \param bSound - id of a file, will be used to identify sound files
     * \param bFile - file to load
     * \return return true on success
     */
    virtual bool Cache(Sound bSound, const std::string &bFile);

    /** Function called to cache music file.
     *  This function is called by CRobotMain for each file used in the mission.
     * \param bFile - file to load
     * \return return true on success
     */
    virtual bool CacheMusic(const std::string &bFile);

    /** Return if plugin is enabled
     *  \return return true if plugin is enabled
     */
    virtual bool GetEnable();

    /** Change global sound volume
     * \param volume - range from 0 to MAXVOLUME
     */
    virtual void SetAudioVolume(int volume);

    /** Return global sound volume
     * \return global volume as int in range from 0 to MAXVOLUME
     */
    virtual int GetAudioVolume();

    /** Set music volume
     * \param volume - range from 0 to MAXVOLUME
     */
    virtual void SetMusicVolume(int volume);

    /** Return music volume
     * \return music volume as int in range from 0 to MAXVOLUME
     */
    virtual int GetMusicVolume();

    /** Set listener position
     * \param eye - position of listener
     * \param lookat - direction listener is looking at
     */
    virtual void SetListener(const Math::Vector &eye, const Math::Vector &lookat);

    /** Update data each frame
     * \param rTime - time since last update
     */
    virtual void FrameMove(float rTime);

    /** Play specific sound
     * \param sound - sound to play
     * \param amplitude - change amplitude of sound before playing
     * \param frequency - change sound frequency before playing (0.5 octave down, 2.0 octave up)
     * \param bLoop - loop sound
     * \return identifier of channel that sound will be played on
     */
    virtual int Play(Sound sound, float amplitude=1.0f, float frequency=1.0f, bool bLoop = false);

    /** Play specific sound
     * \param sound - sound to play
     * \param pos - position of sound in space
     * \param amplitude - change amplitude of sound before playing
     * \param frequency - change sound frequency before playing (0.5 octave down, 2.0 octave up)
     * \param bLoop - loop sound
     * \return identifier of channel that sound will be played on
     */
    virtual int Play(Sound sound, const Math::Vector &pos, float amplitude=1.0f, float frequency=1.0f, bool bLoop = false);

    /** Remove all operations that would be made on sound in channel.
     * \param channel - channel to work on
     * \return return true on success
     */
    virtual bool FlushEnvelope(int channel);

    /** Add envelope to sound. Envelope is a operatino that will be performend on sound in future like changing frequency
     * \param channel - channel to work on
     * \param amplitude - change amplitude
     * \param frequency - change frequency
     * \param time - when to change (sample time)
     * \param oper - operation to perform
     * \return return true on success
     */
    virtual bool AddEnvelope(int channel, float amplitude, float frequency, float time, SoundNext oper);

    /** Set sound position in space
     * \param channel - channel to work on
     * \param pos - new positino of a sound
     * \return return true on success
     */
    virtual bool Position(int channel, const Math::Vector &pos);

    /** Set sound frequency
     * \param channel - channel to work on
     * \param frequency - change sound frequency
     * \return return true on success
     */
    virtual bool Frequency(int channel, float frequency);

    /** Stop playing sound
     * \param channel - channel to work on
     * \return return true on success
     */
    virtual bool Stop(int channel);

    /** Stop playing all sounds
     * \return return true on success
     */
    virtual bool StopAll();

    /** Mute/unmute all sounds
     * \param bMute
     * \return return true on success
     */
    virtual bool MuteAll(bool bMute);

    /** Start playing music
     * \param rank - track number
     * \param bRepeat - repeat playing
     * \param fadeTime - time of transition between music
     * \return return true on success
     */
    virtual bool PlayMusic(int rank, bool bRepeat, float fadeTime=2.0f);

    /** Start playing music
     * \param filename - name of file to play
     * \param bRepeat - repeat playing
     * \param fadeTime - time of transition between music
     * \return return true on success
     */
    virtual bool PlayMusic(const std::string &filename, bool bRepeat, float fadeTime=2.0f);

    /** Restart music
     * \return return true on success
     */
    virtual bool RestartMusic();

    /** Susspend playing music
     * \return nothing
     */
    virtual void SuspendMusic();

    /** Stop playing music
     * \return nothing
     */
    virtual void StopMusic(float fadeTime=2.0f);

    /** Check if music if playing
     * \return return true if music is playing
     */
    virtual bool IsPlayingMusic();
    
    /** Start playing pause music
     * \param filename - name of file to play
     * \return return true on success
     */
     virtual bool PlayPauseMusic(const std::string &filename, bool repeat);
     
     /** Stop playing pause music and return to the mission music
      * \return nothing
      */
     virtual void StopPauseMusic();
};

