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

// sound.h

/**
 *  @file sound/sound.h
 *  @brief Sound plugin interface
 */

#pragma once

#include <math/vector.h>

#include <plugins/plugininterface.h>

#include <string>


/*!
 * Maximum possible audio volume
 */
#define MAXVOLUME 100


/**
 * \public
 * \enum    Sound   sound/sound.h
 * \brief   Sound enum representing sound file
**/
enum Sound
{
  SOUND_CLICK  = 0,
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
  SOUND_MOTORd  = 81, /*!< engine friction */
};


/**
 * \public
 * \enum    SoundNext   sound/sound.h
 * \brief   Enum representing operation that will be performend on a sound at given time
**/
enum SoundNext
{
  SOPER_CONTINUE = 1, /*!< continue playing */
  SOPER_STOP = 2, /*!< stop playing */
  SOPER_LOOP = 3, /*!< start over */
};


/**
* @class CSound
*
* @brief Sound plugin interface
*
*/
class CSound : public CPluginInterface
{
  public:
    CSound() {
      //CInstanceManager::getInstance().AddInstance(CLASS_SOUND, this);
      //m_iMan->AddInstance(CLASS_SOUND, this);
    };
    virtual ~CSound() = 0;

    /** Function to initialize sound device
     *  @param bool b3D - enable support for 3D sound
     */
    virtual bool Create(bool b3D) = 0;

    /** Function called to cache all sound effect files.
     *  Function calls \link CSound::Cache() \endlink for each file
     */
    virtual void CacheAll() = 0;

    /** Function called to cache sound effect file.
     *  This function is called by plugin interface for each file.
     * @param Sound bSound - id of a file, will be used to identify sound files
     * @param std::string bFile - file to load
     * @return return true on success
     */
    virtual bool Cache(Sound bSound, std::string bFile) = 0;

    /** Return if plugin is enabled
     *  @return return true if plugin is enabled
     */
    virtual bool RetEnable() = 0;

    /** Change sound mode to 2D/3D
     * @param bool bMode - true to enable 3D sound
     */
    virtual void SetSound3D(bool bMode) = 0;

    /** Return if we use 3D sound
     * @return true if we have 3D sound enabled
     */
    virtual bool RetSound3D() = 0;

    /** Return if we have 3D sound capable card
     * @return true for 3D sound support
     */
    virtual bool RetSound3DCap() = 0;

    /** Change global sound volume
     * @param int volume - range from 0 to MAXVOLUME
     */
    virtual void SetAudioVolume(int volume) = 0;

    /** Return global sound volume
     * @return global volume as int in range from 0 to MAXVOLUME
     */
    virtual int RetAudioVolume() = 0;

    /** Set music volume
     * @param int volume - range from 0 to MAXVOLUME
     */
    virtual void SetMusicVolume(int volume) = 0;

    /** Return music volume
     * @return music volume as int in range from 0 to MAXVOLUME
     */
    virtual int RetMusicVolume() = 0;

    /** Set listener position
     * @param Math::Vector eye - position of listener
     * @param Math::Vector lookat - direction listener is looking at
     */
    virtual void SetListener(Math::Vector eye, Math::Vector lookat) = 0;

    /** Update data each frame
     * @param float rTime - time since last update
     */
    virtual void FrameMove(float rTime) = 0;

    /** Play specific sound
     * @param Sound sound - sound to play
     * @param float amplitude - change amplitude of sound before playing
     * @param float frequency - change sound frequency before playing (0.5 octave down, 2.0 octave up)
     * @param bool bLoop - loop sound
     * @return identifier of channel that sound will be played on
     */
    virtual int Play(Sound sound, float amplitude=1.0f, float frequency=1.0f, bool bLoop = false) = 0;

    /** Play specific sound
     * @param Sound sound - sound to play
     * @param Math:Vector pos - position of sound in space
     * @param float amplitude - change amplitude of sound before playing
     * @param float frequency - change sound frequency before playing (0.5 octave down, 2.0 octave up)
     * @param bool bLoop - loop sound
     * @return identifier of channel that sound will be played on
     */
    virtual int Play(Sound sound, Math::Vector pos, float amplitude=1.0f, float frequency=1.0f, bool bLoop = false) = 0;

    /** Remove all operations that would be made on sound in channel.
     * @param int channel - channel to work on
     * @return return true on success
     */
    virtual bool FlushEnvelope(int channel) = 0;

    /** Add envelope to sound. Envelope is a operatino that will be performend on sound in future like changing frequency
     * @param int channel - channel to work on
     * @param float amplitude - change amplitude
     * @param float frequency - change frequency
     * @param float time - when to change (sample time)
     * @param SoundNext oper - operation to perform
     * @return return true on success
     */
    virtual bool AddEnvelope(int channel, float amplitude, float frequency, float time, SoundNext oper) = 0;

    /** Set sound position in space
     * @param int channel - channel to work on
     * @param Math::Vector pos - new positino of a sound
     * @return return true on success
     */
    virtual bool Position(int channel, Math::Vector pos) = 0;

    /** Set sound frequency
     * @param int channel - channel to work on
     * @param float frequency - change sound frequency
     * @return return true on success
     */
    virtual bool Frequency(int channel, float frequency) = 0;

    /** Stop playing sound
     * @param int channel - channel to work on
     * @return return true on success
     */
    virtual bool Stop(int channel) = 0;

    /** Stop playing all sounds
     * @return return true on success
     */
    virtual bool StopAll() = 0;

    /** Mute/unmute all sounds
     * @param bool bMute
     * @return return true on success
     */
    virtual bool MuteAll(bool bMute) = 0;

    /** Start playing music
     * @param int rank - track number
     * @param bool bRepeat - repeat playing
     * @return return true on success
     */
    virtual bool PlayMusic(int rank, bool bRepeat) = 0;

    /** Restart music
     * @return return true on success
     */
    virtual bool RestartMusic() = 0;

    /** Susspend paying music
     * @return return true on success
     */
    virtual void SuspendMusic() = 0;

    /** Stop playing music
     * @return return true on success
     */
    virtual void StopMusic() = 0;

    /** Check if music if playing
     * @return return true if music is playing
     */
    virtual bool IsPlayingMusic() = 0;
};

