/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2016, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#pragma once

#include "common/event.h"

#include "level/level_category.h"

#include "test/recorder/app_mock.h"
#include "test/recorder/json.hpp"

#include <algorithm>
#include <boost/function.hpp>
#include <memory>
#include <string>
#include <vector>

namespace Math {
struct Point;
struct Vector;
} /* namespace Math */


class CRecorder;


struct RecordedEvent
{
    RecordedEvent()
    {
        
    }
    
    RecordedEvent(Event event, float time)
        : event(std::move(event)),
          time(time)
    {
              
    }
    
    Event event;
    float time;		//!< Time of occurrence event (from CApplication::GetAbsTime())
};

/**
 * \brief This class contains all events needed to replay gameplay
 */
class CRecord
{
friend CRecorder;
    
public:
    void                        WriteEvent(RecordedEvent& event);

    //! Check if next event exists and there is proper time to read it
    bool                        CanReadEvent(float time);
    //! Read next event
    Event                       ReadEvent();

    bool                        TryReadRecordEvent(RecordedEvent& event);

    //! Level will start immediately at the beginning of game instead of main menu
    void                        SetInitialLevel(LevelCategory cat, int chap, int level);
    bool                        IsInitialGameMode() {return m_initialGameMode;}
    LevelCategory               GetInitialCategory() {return m_initialCategory;}
    int                         GetInitialChapter() {return m_initialChapter;}
    int                         GetInitialRank() {return m_initialLevel;}
    
protected:
    std::vector<RecordedEvent>  m_events = {};
    unsigned int                m_nextEvent = 0;	//!< Index of next event to read

    bool                        m_initialGameMode = false;
    LevelCategory               m_initialCategory = LevelCategory::Max;	//!< If different from LevelCategory::Max, specified level will start instead of main menu
    int                         m_initialChapter = 0;
    int                         m_initialLevel = 0;
};

class CRecorder
{
public:
    CRecorder(std::string recordName = "")
        : m_recordName(recordName)
    { }

    //! Set path to program (argv[0]), it should be invoked before call Record() or Replay()
    static void                 SetPath(char* const path) { m_path = path; }

    void                        SetInitialLevel(LevelCategory category, int chapter, int level);

    void                        SetRecordName(std::string name);
    //! Set function that tests correctness of code, invoked while every EVENT_FRAME
    void                        SetAssertion(AssertionFunc assertion);
    //! Start game and save events to file
    void                        Record();
    //! Start game and read events from file
    void                        Replay();

protected:
    //! Save record to file
    void                        Write();
    //! Read record from file
    void                        Read();
    
    std::string                 GetRecordPath(std::string name);
    
    static nlohmann::json       ConvertEventToJson(RecordedEvent& event);
    static nlohmann::json       ConvertVectorToJson(Math::Vector& vec);
    static nlohmann::json       ConvertPointToJson(Math::Point& point);
    static nlohmann::json       ConvertEventDataToJson(std::unique_ptr<EventData> data);
    
    static RecordedEvent        ConvertJsonToEvent(nlohmann::json& event);
    static Math::Vector         ConvertJsonToVector(nlohmann::json& vec);
    static Math::Point          ConvertJsonToPoint(nlohmann::json& point);
    static std::unique_ptr<EventData> ConvertJsonToEventData(nlohmann::json& json);

protected:
    static char*                m_path;			//!< path to program (argv[0])

    std::string                 m_recordName;	//!< unique name of record, necessary to write or read from file
    CRecord                     m_record;

    AssertionFunc 				m_assertion = 0;	//!<

};
    
