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

#include "common/iman.h"
#include "common/logger.h"

#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>


/*!
 * Maximum possible audio volume
 */
#define MAXVOLUME 100.0f


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
  SOUND_MOTORd  = 81, /*!< engine friction */
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
    inline CSoundInterface() {
      CInstanceManager::GetInstance().AddInstance(CLASS_SOUND, this);
      //m_iMan->AddInstance(CLASS_SOUND, this);
    };
    inline virtual ~CSoundInterface() {};

    /** Function to initialize sound device
     *  \param b3D - enable support for 3D sound
     */
    inline virtual bool Create(bool b3D) { return true; };

    /** Function called to cache all sound effect files.
     *  Function calls \link CSoundInterface::Cache() \endlink for each file
     */
    inline void CacheAll(std::string path) {
        for ( int i = 1; i <= 81; i++ ) {
            std::stringstream filename;
            filename << path << "/sound" << std::setfill('0') << std::setw(3) << i << ".wav";
            if ( !Cache(static_cast<Sound>(i), filename.str()) )
                GetLogger()->Warn("Unable to load audio: %s\n", filename.str().c_str());
        }
    };

    /** Function called to cache sound effect file.
     *  This function is called by plugin interface for each file.
     * \param bSound - id of a file, will be used to identify sound files
     * \param bFile - file to load
     * \return return true on success
     */
    inline virtual bool Cache(Sound bSound, std::string bFile) { return true; };

    /** Return if plugin is enabled
     *  \return return true if plugin is enabled
     */
    inline virtual bool GetEnable() {return true;};

    /** Change sound mode to 2D/3D
     * \param bMode - true to enable 3D sound
     */
    inline virtual void SetSound3D(bool bMode) {};

    /** Return if we use 3D sound
     * \return true if we have 3D sound enabled
     */
    inline virtual bool GetSound3D() {return true;};

    /** Return if we have 3D sound capable card
     * \return true for 3D sound support
     */
    inline virtual bool GetSound3DCap() {return true;};

    /** Change global sound volume
     * \param volume - range from 0 to MAXVOLUME
     */
    inline virtual void SetAudioVolume(int volume) {};

    /** Return global sound volume
     * \return global volume as int in range from 0 to MAXVOLUME
     */
    inline virtual int GetAudioVolume() {return 0;};

    /** Set music volume
     * \param volume - range from 0 to MAXVOLUME
     */
    inline virtual void SetMusicVolume(int volume) {};

    /** Return music volume
     * \return music volume as int in range from 0 to MAXVOLUME
     */
    inline virtual int GetMusicVolume() {return 0;};

    /** Set listener position
     * \param eye - position of listener
     * \param lookat - direction listener is looking at
     */
    inline virtual void SetListener(Math::Vector eye, Math::Vector lookat) {};

    /** Update data each frame
     * \param rTime - time since last update
     */
    inline virtual void FrameMove(float rTime) {};

    /** Play specific sound
     * \param sound - sound to play
     * \param amplitude - change amplitude of sound before playing
     * \param frequency - change sound frequency before playing (0.5 octave down, 2.0 octave up)
     * \param bLoop - loop sound
     * \return identifier of channel that sound will be played on
     */
    inline virtual int Play(Sound sound, float amplitude=1.0f, float frequency=1.0f, bool bLoop = false) {return 0;};

    /** Play specific sound
     * \param sound - sound to play
     * \param pos - position of sound in space
     * \param amplitude - change amplitude of sound before playing
     * \param frequency - change sound frequency before playing (0.5 octave down, 2.0 octave up)
     * \param bLoop - loop sound
     * \return identifier of channel that sound will be played on
     */
    inline virtual int Play(Sound sound, Math::Vector pos, float amplitude=1.0f, float frequency=1.0f, bool bLoop = false) {return 0;};

    /** Remove all operations that would be made on sound in channel.
     * \param channel - channel to work on
     * \return return true on success
     */
    inline virtual bool FlushEnvelope(int channel) {return true;};

    /** Add envelope to sound. Envelope is a operatino that will be performend on sound in future like changing frequency
     * \param channel - channel to work on
     * \param amplitude - change amplitude
     * \param frequency - change frequency
     * \param time - when to change (sample time)
     * \param oper - operation to perform
     * \return return true on success
     */
    inline virtual bool AddEnvelope(int channel, float amplitude, float frequency, float time, SoundNext oper) {return true;};

    /** Set sound position in space
     * \param channel - channel to work on
     * \param pos - new positino of a sound
     * \return return true on success
     */
    inline virtual bool Position(int channel, Math::Vector pos) {return true;};

    /** Set sound frequency
     * \param channel - channel to work on
     * \param frequency - change sound frequency
     * \return return true on success
     */
    inline virtual bool Frequency(int channel, float frequency) {return true;};

    /** Stop playing sound
     * \param channel - channel to work on
     * \return return true on success
     */
    inline virtual bool Stop(int channel) {return true;};

    /** Stop playing all sounds
     * \return return true on success
     */
    inline virtual bool StopAll() {return true;};

    /** Mute/unmute all sounds
     * \param bMute
     * \return return true on success
     */
    inline virtual bool MuteAll(bool bMute) {return true;};

    /** Start playing music
     * \param rank - track number
     * \param bRepeat - repeat playing
     * \return return true on success
     */
    inline virtual bool PlayMusic(int rank, bool bRepeat) {return true;};

    /** Restart music
     * @return return true on success
     */
    inline virtual bool RestartMusic() {return true;};

    /** Susspend paying music
     * \return return true on success
     */
    inline virtual void SuspendMusic() {};

    /** Stop playing music
     * \return return true on success
     */
    inline virtual void StopMusic() {};

    /** Check if music if playing
     * \return return true if music is playing
     */
    inline virtual bool IsPlayingMusic() {return true;};
};

