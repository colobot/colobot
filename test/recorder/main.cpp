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

#include "common/config.h"

#include "app/app.h"
#include "app/signal_handlers.h"

#include "common/logger.h"
#include "common/make_unique.h"
#include "common/profiler.h"
#include "common/restext.h"
#include "common/version.h"

#include "common/resources/resourcemanager.h"
#include "common/system/system.h"
#if PLATFORM_WINDOWS
    #include "common/system/system_windows.h"
#endif

#include "object/object.h"
#include "object/interface/carrier_object.h"

#include "test/recorder/app_mock.h"
#include "test/recorder/event_queue_mock.h"
#include "test/recorder/controller_mock.h"
#include "test/recorder/input_mock.h"
#include "test/recorder/object_maker_mock.h"
#include "test/recorder/recorder.h"

#if PLATFORM_WINDOWS
    #include <windows.h>
#endif

#include <memory>
#include <vector>
#include <boost/filesystem.hpp>
#include <string>
#include <gtest/gtest.h>


/**
 * \file test/recorder/main.cpp
 * \brief A tool for recording, replaying and testing
 *
 * To record gameplay as list of event :
 * \code{.sh}
 * ./colobot_recorder record NAME_OF_RECORD
 * \endcode
 * This command starts game and begins recording all events.
 * After exit game, file ./records/NAME_OF_RECORD.json will be created.
 *
 * If you want to run level immediately after starting game, add optional arguments :
 * \code{.sh}
 * ./colobot_recorder record NAME_OF_RECORD CATEGORY CHAPTER LEVEL
 * \endcode
 * For example :
 * \code{.sh}
 * ./colobot_recorder record NAME_OF_RECORD 2 1 3
 * \endcode
 * will run third mission on Earth
 *
 * To replay recorded game :
 * \code{.sh}
 * ./colobot_recorder replay NAME_OF_RECORD
 * \endcode
 *
 * To run tests :
 * \code{.sh}
 * ./colobot_recorder test
 * \endcode
 *
 * To add test :
 * 1) Record proper gameplay and give it suitable name
 * 2) Write new test in the end of this file. Test should have form :
 * \code
 * 	TEST_F(CRecorderTest, NAME_OF_TEST)
 *	{
 *		auto assertion = [](CApplicationMock* app)
 *		{
 * 			// Write test of code that will be executed while every frame
 * 			// NOTE : ASSERT_* from google test framework won't stop test, it doesn't work in nested functions
 *		};
 *
 *		StartTest("NAME_OF_RECORD", assertion);
 *	}
 * \endcode
 * 3) If test works properly, copy record file from build directory to source directory (test/recorder/records).
 *    When other team's member builds project, records will copied from source directory to build directory.
 */

class CRecorderTest : public testing::Test
{
public:
    void StartTest(std::string recordName, AssertionFunc assertion);

protected:
    CRecorder   m_recorder;
};

void CRecorderTest::StartTest(std::string recordName, AssertionFunc assertion)
{
    m_recorder.SetRecordName(recordName);
    m_recorder.SetAssertion(assertion);

    m_recorder.Replay();
}

    
int main(int argc, char *argv[])
{
    CRecorder::SetPath(argv[0]);

    if (strcmp(argv[1], "record") == 0)
    {
        CRecorder recorder = CRecorder(std::string(argv[2]));

        if (argc == 6)
        {
            recorder.SetInitialLevel(static_cast<LevelCategory>(std::stoi(argv[3])), std::stoi(argv[4]), std::stoi(argv[5]));
        }

        recorder.Record();
    }
    else if (strcmp(argv[1], "replay") == 0)
    {
        CRecorder recorder = CRecorder(std::string(argv[2]));

        recorder.Replay();
    }
    else if (strcmp(argv[1], "test") == 0)
    {
        ::testing::InitGoogleTest(&argc, argv);

        return RUN_ALL_TESTS();
    }

    return 0;
}

// Example of test that checks if robot properly grabed titan
TEST_F(CRecorderTest, GrabTitan)
{
	bool assertionDone = false;

    auto assertion = [&](CApplicationMock* app)
    {
    	if (!assertionDone && app->GetAbsTime() >= 8.0f) // check only one time after 8 seconds
    	{
    		assertionDone = true;

    		CObject* obj = app->GetObjectManager()->GetObjectById(1);

    		CCarrierObject* carrier = dynamic_cast<CCarrierObject*>(obj);

    		ASSERT_TRUE (carrier->GetCargo() != nullptr); // check if robot is grabbing cargo
    	}
    };

    StartTest("grab_titan", assertion); // replay record named "grab_titan"

    ASSERT_TRUE (assertionDone); // if assertion never once executed (e.g. when game is earlier terminated), fail test
}


