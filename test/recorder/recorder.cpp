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

#include "test/recorder/recorder.h"

#include "app/signal_handlers.h"

#include "common/key.h"
#include "common/logger.h"
#include "common/make_unique.h"
#include "common/profiler.h"
#include "common/resources/resourcemanager.h"
#include "common/restext.h"
#include "common/system/system.h"

#include "math/point.h"
#include "math/vector.h"

#include "test/recorder/app_mock.h"
#include "test/recorder/logger_mock.h"
#include "test/recorder/object_maker_mock.h"

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>


const std::string RECORDS_DIR_NAME = "records";

const std::string JSON_EVENT_TYPE = "type";
const std::string JSON_EVENT_TIME = "time";
const std::string JSON_EVENT_KEY_MOD = "key_mod";
const std::string JSON_EVENT_MOUSE_POS = "mouse_pos";
const std::string JSON_EVENT_MOUSE_BUTTION_STATE = "mouse_button_state";
const std::string JSON_EVENT_PARAM = "param";
const std::string JSON_EVENT_DATA = "data";
const std::string JSON_EVENT_ABS_TIME = "abs_time";

enum EventDataType
{
    JoyAxis         = 0,
    JoyButton       = 1,
    Key             = 2,
    MouseButton     = 3,
    MouseWheel      = 4,
    TextInput       = 5
};


char* CRecorder::m_path = nullptr;


void CRecord::WriteEvent(RecordedEvent& event)
{
    //if (IsInputEvent(event)) return;
    
    m_events.push_back(std::move(event));
}

Event CRecord::ReadEvent()
{
    return std::move(m_events[m_nextEvent++].event);
}

bool CRecord::TryReadRecordEvent(RecordedEvent& event)
{
    if (m_nextEvent < m_events.size())
    {
        event = std::move(m_events[m_nextEvent++]);
        
        return true;
    }
    else 
    {
        return false;
    }
}

bool CRecord::CanReadEvent(float time)
{
    return m_nextEvent < m_events.size() && time >= m_events[m_nextEvent].time;
}

void CRecorder::Write()
{
    nlohmann::json json;
    
    RecordedEvent event;
    
    if (m_record.m_initialGameMode)
    {
        json["start_game"] = true;
        json["category"] = static_cast<int>(m_record.m_initialCategory);
        json["chapter"] = m_record.m_initialChapter;
        json["level"] = m_record.m_initialLevel;
    }
    else
    {
        json["start_game"] = false;
    }
    
    while (m_record.TryReadRecordEvent(event))
    {
        json["events"].push_back(ConvertEventToJson(event));
    }
    
    boost::filesystem::path recordsDir = boost::filesystem::path(RECORDS_DIR_NAME);
    
    if (! boost::filesystem::exists(recordsDir))
    {
        boost::filesystem::create_directory(recordsDir);
    }
    
    std::string path = GetRecordPath(m_recordName);
    
    std::ofstream file(path, std::fstream::out | std::fstream::trunc);
    
    if (file.good())
    {
        file << json.dump(4);
        
        file.close();
    }
    else
    {
        std::cerr << "Writing to file " << path << " failed" << std::endl;
        exit(1);
    }
}

void CRecorder::Read()
{
    std::string path = GetRecordPath(m_recordName);
    
    std::ifstream file(path, std::fstream::in);
    
    if (file.good())
    {
        nlohmann::json json;
        
        json << file;
        
        if (json["start_game"])
        {
            m_record.m_initialGameMode = true;
            m_record.m_initialCategory = static_cast<LevelCategory>(static_cast<int>(json["category"]));
            m_record.m_initialChapter = json["chapter"];
            m_record.m_initialLevel = json["level"];
        }
    
        for (nlohmann::json& jsonEvent : json["events"])
        {
            RecordedEvent event = ConvertJsonToEvent(jsonEvent);
            
            m_record.WriteEvent(event);
        }
    }
    else
    {
        std::cerr << "Reading from file " << path << " failed" << std::endl;
        exit(1);
    }
}

std::string CRecorder::GetRecordPath(std::string name)
{
    return RECORDS_DIR_NAME + "/" + name + ".json";
}

void CRecord::SetInitialLevel(LevelCategory cat, int chap, int level)
{
    m_initialGameMode = true;
    m_initialCategory = cat;
    m_initialChapter = chap;
    m_initialLevel = level;
}

nlohmann::json CRecorder::ConvertEventToJson(RecordedEvent& event)
{
    nlohmann::json json = {
        {JSON_EVENT_TYPE, event.event.type},
        {JSON_EVENT_TIME, event.event.rTime},
        {JSON_EVENT_KEY_MOD, event.event.kmodState},
        {JSON_EVENT_MOUSE_POS, ConvertPointToJson(event.event.mousePos)},
        {JSON_EVENT_MOUSE_BUTTION_STATE, event.event.mouseButtonsState},
        {JSON_EVENT_PARAM, event.event.customParam},
        {JSON_EVENT_DATA, ConvertEventDataToJson(std::move(event.event.data))},
        {JSON_EVENT_ABS_TIME, event.time}
    };
    
    return json;
}

nlohmann::json CRecorder::ConvertVectorToJson(Math::Vector& vec)
{
    nlohmann::json json = {
        {"x", vec.x},
        {"y", vec.y},
        {"z", vec.z},
    };
    
    return json;
}

nlohmann::json CRecorder::ConvertPointToJson(Math::Point& point)
{
    nlohmann::json json = {
        {"x", point.x},
        {"y", point.y},
    };
    
    return json;
}

nlohmann::json CRecorder::ConvertEventDataToJson(std::unique_ptr<EventData> data)
{
    nlohmann::json json;
    
    EventData* dataPtr = data.get();
    
    if (dataPtr == nullptr)
    {
        return json;
    }
    else if (dynamic_cast<JoyAxisEventData*>(dataPtr) != nullptr)
    {
        auto e = dynamic_cast<JoyAxisEventData*>(dataPtr);
        json["data_type"] = EventDataType::JoyAxis;
        json["axis"] = e->axis;
        json["value"] = e->value;
    }
    else if (dynamic_cast<JoyButtonEventData*>(dataPtr) != nullptr)
    {
        auto e = dynamic_cast<JoyButtonEventData*>(dataPtr);
        json["data_type"] = EventDataType::JoyButton;
        json["button"] = e->button;
    }
    else if (dynamic_cast<KeyEventData*>(dataPtr) != nullptr)
    {
        auto e = dynamic_cast<KeyEventData*>(dataPtr);
        json["data_type"] = EventDataType::Key;
        json["virtual"] = e->virt;
        json["key"] = e->key;
        json["slot"] = e->slot;
    }
    else if (dynamic_cast<MouseButtonEventData*>(dataPtr) != nullptr)
    {
        auto e = dynamic_cast<MouseButtonEventData*>(dataPtr);
        json["data_type"] = EventDataType::MouseButton;
        json["button"] = e->button;
    }
    else if (dynamic_cast<MouseWheelEventData*>(dataPtr) != nullptr)
    {
        auto e = dynamic_cast<MouseWheelEventData*>(dataPtr);
        json["data_type"] = EventDataType::MouseWheel;
        json["x"] = e->x;
        json["y"] = e->y;
    }
    else if (dynamic_cast<TextInputData*>(dataPtr) != nullptr)
    {
        auto e = dynamic_cast<TextInputData*>(dataPtr);
        json["data_type"] = EventDataType::TextInput;
        json["text"] = e->text;
    }
    else
    {
        throw "This EventData is not implemented";
    }
    
    return json;
}

RecordedEvent CRecorder::ConvertJsonToEvent(nlohmann::json& event)
{
    RecordedEvent e;
    e.event.type = static_cast<EventType>(static_cast<int>(event[JSON_EVENT_TYPE]));
    e.event.rTime = event[JSON_EVENT_TIME];
    e.event.kmodState = event[JSON_EVENT_KEY_MOD];
    e.event.mousePos = ConvertJsonToPoint(event[JSON_EVENT_MOUSE_POS]);
    e.event.mouseButtonsState = event[JSON_EVENT_MOUSE_BUTTION_STATE];
    e.event.customParam = event[JSON_EVENT_PARAM];
    e.event.data = ConvertJsonToEventData(event[JSON_EVENT_DATA]);
    e.time = event[JSON_EVENT_ABS_TIME];
    
    return e;
}

Math::Vector CRecorder::ConvertJsonToVector(nlohmann::json& vec)
{
    return Math::Vector(vec["x"], vec["y"], vec["z"]);
}

Math::Point CRecorder::ConvertJsonToPoint(nlohmann::json& point)
{
    return Math::Point(point["x"], point["y"]);
}

std::unique_ptr<EventData> CRecorder::ConvertJsonToEventData(nlohmann::json& json)
{
    if (json.empty())
    {
        return std::unique_ptr<EventData>(nullptr);
    }
    else if (static_cast<int>(json["data_type"]) == static_cast<int>(EventDataType::JoyAxis))
    {
        auto e = MakeUnique<JoyAxisEventData>();
        e->axis = json["axis"];
        e->value = json["value"];
        return std::move(e);
    }
    else if (static_cast<int>(json["data_type"]) == static_cast<int>(EventDataType::JoyButton))
    {
        auto e = MakeUnique<JoyButtonEventData>();
        e->button = json["button"];
        return std::move(e);
    }
    else if (static_cast<int>(json["data_type"]) == static_cast<int>(EventDataType::Key))
    {
        auto e = MakeUnique<KeyEventData>();
        e->virt = json["virtual"];
        e->key = json["key"];
        e->slot = static_cast<InputSlot>(static_cast<int>(json["slot"]));
        return std::move(e);
    }
    else if (static_cast<int>(json["data_type"]) == static_cast<int>(EventDataType::MouseButton))
    {
        auto e = MakeUnique<MouseButtonEventData>();
        e->button = static_cast<enum MouseButton>(static_cast<int>(json["button"]));
        return std::move(e);
    }
    else if (static_cast<int>(json["data_type"]) == static_cast<int>(EventDataType::MouseWheel))
    {
        auto e = MakeUnique<MouseWheelEventData>();
        e->x = json["x"];
        e->y = json["y"];
        return std::move(e);
    }
    else if (static_cast<int>(json["data_type"]) == static_cast<int>(EventDataType::TextInput))
    {
        auto e = MakeUnique<TextInputData>();
        e->text = json["text"];
        return std::move(e);
    }
    else
    {
        throw "This data_type is not implemented";
    }
}

void CRecorder::Record()
{
    CObjectMakerRecord objectMaker(&m_record);

    CLogger logger; // single instance of logger
    logger.AddOutput(stderr);

    auto systemUtils = CSystemUtils::Create(); // platform-specific utils
    systemUtils->Init();

    CProfiler::SetSystemUtils(systemUtils.get());

    CSignalHandlers::Init(systemUtils.get());

    CResourceManager manager(m_path);

    // Initialize static string arrays
    InitializeRestext();
    InitializeEventTypeTexts();

    CApplicationRecord app(systemUtils.get()); // single instance of the application

    if (m_record.IsInitialGameMode())
    {
        app.SetInitialLevel(m_record.GetInitialCategory(), m_record.GetInitialChapter(), m_record.GetInitialRank());
    }

    app.Create();

    app.Run();

    Write();
}

void CRecorder::Replay()
{
    CProfiler::Reset(); // profiler have to be reseted after previous test

    Read();

    CObjectMakerReplay objectMaker(&m_record);

    CLoggerMock logger; // single instance of logger
    logger.AddOutput(stderr);

    auto systemUtils = CSystemUtils::Create(); // platform-specific utils
    systemUtils->Init();

    CProfiler::SetSystemUtils(systemUtils.get());

    CSignalHandlers::Init(systemUtils.get());

    CResourceManager manager(m_path);

    // Initialize static string arrays
    InitializeRestext();
    InitializeEventTypeTexts();

    CApplicationMock app(systemUtils.get()); // single instance of the application

    if (m_record.IsInitialGameMode())
    {
        app.SetInitialLevel(m_record.GetInitialCategory(), m_record.GetInitialChapter(), m_record.GetInitialRank());
    }

    app.Create();

    app.SetAssertion(m_assertion);

    app.Run();
}

void CRecorder::SetInitialLevel(LevelCategory category, int chapter, int level)
{
    m_record.SetInitialLevel(category, chapter, level);
}

void CRecorder::SetRecordName(std::string name)
{
    m_recordName = name;
}

void CRecorder::SetAssertion(AssertionFunc assertion)
{
    m_assertion = assertion;
}
