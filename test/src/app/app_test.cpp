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

#include "app/app.h"

#include "common/system/system_other.h"

#include <functional>
#include <memory>

#include <gtest/gtest.h>
#include <hippomocks.h>

using namespace HippoMocks;
namespace ph = std::placeholders;
using TimeUtils::TimeStamp;

class CApplicationWrapper : public CApplication
{
public:
    CApplicationWrapper(CSystemUtils* systemUtils)
        : CApplication(systemUtils)
    {
        SDL_Init(0);
        m_eventQueue = std::make_unique<CEventQueue>();
    }

    ~CApplicationWrapper()
    {
        SDL_Quit();
    }

    Event CreateUpdateEvent(TimeStamp timestamp) override
    {
        return CApplication::CreateUpdateEvent(timestamp);
    }
};

class CApplicationUT : public testing::Test
{
protected:
    CApplicationUT() :
        m_systemUtils(nullptr),
        m_currentTime(0)
    {}

    ~CApplicationUT() noexcept
    {}

    void SetUp() override;
    void TearDown() override;

    void NextInstant(long long diff);

    void TestCreateUpdateEvent(long long relTimeExact, long long absTimeExact,
                               float relTime, float absTime,
                               long long relTimeReal, long long absTimeReal);

protected:
    std::unique_ptr<CApplicationWrapper> m_app;
    MockRepository m_mocks;
    CSystemUtils* m_systemUtils;

private:
    long long m_currentTime;
};

void CApplicationUT::SetUp()
{
    m_systemUtils = m_mocks.Mock<CSystemUtils>();

    m_mocks.OnCall(m_systemUtils, CSystemUtils::GetDataPath).Return("");
    m_mocks.OnCall(m_systemUtils, CSystemUtils::GetLangPath).Return("");
    m_mocks.OnCall(m_systemUtils, CSystemUtils::GetSaveDir).Return("");

    m_app = std::make_unique<CApplicationWrapper>(m_systemUtils);
}

void CApplicationUT::TearDown()
{
    m_app.reset();
}

void CApplicationUT::NextInstant(long long diff)
{
    m_currentTime += diff;
}

void CApplicationUT::TestCreateUpdateEvent(long long relTimeExact, long long absTimeExact,
                                           float relTime, float absTime,
                                           long long relTimeReal, long long absTimeReal)
{
    TimeStamp now = TimeUtils::GetCurrentTimeStamp();
    Event event = m_app->CreateUpdateEvent(now);
    EXPECT_EQ(EVENT_FRAME, event.type);
    EXPECT_FLOAT_EQ(relTime, event.rTime);
    EXPECT_FLOAT_EQ(relTime, m_app->GetRelTime());
    EXPECT_FLOAT_EQ(absTime, m_app->GetAbsTime());
    EXPECT_EQ(relTimeExact, m_app->GetExactRelTime());
    EXPECT_EQ(absTimeExact, m_app->GetExactAbsTime());
    EXPECT_EQ(relTimeReal, m_app->GetRealRelTime());
    EXPECT_EQ(absTimeReal, m_app->GetRealAbsTime());
}


TEST_F(CApplicationUT, UpdateEventTimeCalculation_SimulationSuspended)
{
    m_app->SuspendSimulation();

    Event event = m_app->CreateUpdateEvent(TimeUtils::GetCurrentTimeStamp());

    EXPECT_EQ(EVENT_NULL, event.type);
}

TEST_F(CApplicationUT, UpdateEventTimeCalculation_NormalOperation)
{
    // 1st update

    long long relTimeExact = 1111;
    long long absTimeExact = relTimeExact;
    float relTime = relTimeExact / 1e9f;
    float absTime = absTimeExact / 1e9f;
    long long relTimeReal = relTimeExact;
    long long absTimeReal = absTimeExact;

    NextInstant(relTimeReal);

    TestCreateUpdateEvent(relTimeExact, absTimeExact, relTime, absTime, relTimeReal, absTimeReal);

    // 2nd update

    relTimeExact = 2222;
    absTimeExact += relTimeExact;
    relTime = relTimeExact / 1e9f;
    absTime = absTimeExact / 1e9f;
    relTimeReal = relTimeExact;
    absTimeReal = absTimeExact;

    NextInstant(relTimeReal);

    TestCreateUpdateEvent(relTimeExact, absTimeExact, relTime, absTime, relTimeReal, absTimeReal);
}

TEST_F(CApplicationUT, UpdateEventTimeCalculation_NegativeTimeOperation)
{
    // 1st update

    long long relTimeExact = 2222;
    long long absTimeExact = relTimeExact;
    float relTime = relTimeExact / 1e9f;
    float absTime = absTimeExact / 1e9f;
    long long relTimeReal = relTimeExact;
    long long absTimeReal = absTimeExact;

    NextInstant(relTimeReal);

    TestCreateUpdateEvent(relTimeExact, absTimeExact, relTime, absTime, relTimeReal, absTimeReal);

    // 2nd update

    NextInstant(-1111);

    Event event = m_app->CreateUpdateEvent(TimeUtils::GetCurrentTimeStamp());
    EXPECT_EQ(EVENT_NULL, event.type);
}

TEST_F(CApplicationUT, UpdateEventTimeCalculation_ChangingSimulationSpeed)
{
    m_app->SetSimulationSpeed(2.0f);

    // 1st update -- speed 2x

    long long relTimeReal = 100;
    long long absTimeReal = relTimeReal;
    long long relTimeExact = relTimeReal*2;
    long long absTimeExact = absTimeReal*2;
    float relTime = relTimeExact / 1e9f;
    float absTime = absTimeExact / 1e9f;

    NextInstant(relTimeReal);

    TestCreateUpdateEvent(relTimeExact, absTimeExact, relTime, absTime, relTimeReal, absTimeReal);

    // 2nd update -- speed 2x

    relTimeReal = 200;
    absTimeReal += relTimeReal;
    relTimeExact = relTimeReal*2;
    absTimeExact += relTimeReal*2;
    relTime = relTimeExact / 1e9f;
    absTime = absTimeExact / 1e9f;

    NextInstant(relTimeReal);

    TestCreateUpdateEvent(relTimeExact, absTimeExact, relTime, absTime, relTimeReal, absTimeReal);

    // 3rd update -- speed 4x
    m_app->SetSimulationSpeed(4.0f);

    relTimeReal = 300;
    absTimeReal += relTimeReal;
    relTimeExact = relTimeReal*4;
    absTimeExact += relTimeReal*4;
    relTime = relTimeExact / 1e9f;
    absTime = absTimeExact / 1e9f;

    NextInstant(relTimeReal);

    TestCreateUpdateEvent(relTimeExact, absTimeExact, relTime, absTime, relTimeReal, absTimeReal);

    // 4th update -- speed 1x
    m_app->SetSimulationSpeed(1.0f);

    relTimeReal = 400;
    absTimeReal += relTimeReal;
    relTimeExact = relTimeReal;
    absTimeExact += relTimeReal;
    relTime = relTimeExact / 1e9f;
    absTime = absTimeExact / 1e9f;

    NextInstant(relTimeReal);

    TestCreateUpdateEvent(relTimeExact, absTimeExact, relTime, absTime, relTimeReal, absTimeReal);
}

TEST_F(CApplicationUT, UpdateEventTimeCalculation_SuspendingAndResumingSimulation)
{
    // 1st update -- simulation enabled

    long long relTimeReal = 1000;
    long long absTimeReal = relTimeReal;
    long long relTimeExact = relTimeReal;
    long long absTimeExact = absTimeReal;
    float relTime = relTimeExact / 1e9f;
    float absTime = absTimeExact / 1e9f;

    NextInstant(relTimeReal);

    TestCreateUpdateEvent(relTimeExact, absTimeExact, relTime, absTime, relTimeReal, absTimeReal);

    // 2nd update -- simulation suspended

    m_app->SuspendSimulation();

    long long suspensionTime = 5000;

    NextInstant(suspensionTime);

    // 3rd update -- simulation resumed

    m_app->ResumeSimulation();

    relTimeReal = 200;
    absTimeReal += relTimeReal;
    relTimeExact = relTimeReal;
    absTimeExact += relTimeReal;
    relTime = relTimeExact / 1e9f;
    absTime = absTimeExact / 1e9f;

    NextInstant(relTimeReal);

    TestCreateUpdateEvent(relTimeExact, absTimeExact, relTime, absTime, relTimeReal, absTimeReal);
}
