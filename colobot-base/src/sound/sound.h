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
 * \file sound/sound.h
 * \brief Sound plugin interface
 */

#pragma once

#include "sound/sound_type.h"

#include <glm/glm.hpp>

#include <string>
#include <filesystem>

/*!
 * Maximum possible audio volume
 */
const float MAXVOLUME = 100.0f;


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

    /** Stop all sounds and music and clean cache.
     */
    virtual void Reset();

    /** Function called to cache sound effect file.
     *  This function is called by plugin interface for each file.
     * \param sound - id of a file, will be used to identify sound files
     * \param file - file to load
     * \return return true on success
     */
    virtual bool Cache(SoundType sound, const std::string &file);

    /** Function called to cache music file.
     *  This function is called by CRobotMain for each file used in the mission.
     *  This function is executed asynchronously
     * \param file - file to load
     */
    virtual void CacheMusic(const std::filesystem::path &file);

    /** Function to check if sound effect file was cached.
     * \param sound - id of a sound effect file
     * \return return true if the file was cached
     */
    virtual bool IsCached(SoundType sound);

    /** Function called to check if music file was cached.
     * \param file - file to check
     * \return return true if the file was cached
     */
    virtual bool IsCachedMusic(const std::string &file);

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
    virtual void SetListener(const glm::vec3 &eye, const glm::vec3 &lookat);

    /** Update data each frame
     * \param rTime - time since last update
     */
    virtual void FrameMove(float rTime);

    /** Play specific sound
     * \param sound - sound to play
     * \param amplitude - change amplitude of sound before playing
     * \param frequency - change sound frequency before playing (0.5 octave down, 2.0 octave up)
     * \param loop - loop sound
     * \return identifier of channel that sound will be played on
     */
    virtual int Play(SoundType sound, float amplitude=1.0f, float frequency=1.0f, bool loop = false);

    /** Play specific sound
     * \param sound - sound to play
     * \param pos - position of sound in space
     * \param amplitude - change amplitude of sound before playing
     * \param frequency - change sound frequency before playing (0.5 octave down, 2.0 octave up)
     * \param loop - loop sound
     * \return identifier of channel that sound will be played on
     */
    virtual int Play(SoundType sound, const glm::vec3 &pos, float amplitude=1.0f, float frequency=1.0f, bool loop = false);

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
    virtual bool Position(int channel, const glm::vec3 &pos);

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
     * \param mute
     * \return return true on success
     */
    virtual bool MuteAll(bool mute);

    /** Start playing music
     * This function is executed asynchronously
     * \param filename - name of file to play
     * \param repeat - repeat playing
     * \param fadeTime - time of transition between music, 0 to disable
     */
    virtual void PlayMusic(const std::filesystem::path& filename, bool repeat, float fadeTime = 2.0f);

    /** Stop playing music
     * \return nothing
     */
    virtual void StopMusic(float fadeTime=2.0f);

    /** Check if music if playing
     * \return return true if music is playing
     */
    virtual bool IsPlayingMusic();

    /** Start playing pause music
     * This function is executed asynchronously
     * \param filename - name of file to play
     * \param repeat - repeat playing
     * \return return true on success
     */
     virtual void PlayPauseMusic(const std::string &filename, bool repeat);

     /** Stop playing pause music and return to the mission music
      * \return nothing
      */
     virtual void StopPauseMusic();
};
