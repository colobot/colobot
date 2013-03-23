#include "app/app.h"

#if defined(PLATFORM_WINDOWS)
    #include "app/system_windows.h"
#elif defined(PLATFORM_LINUX)
    #include "app/system_linux.h"
#else
    #include "app/system_other.h"
#endif

#include "app/system_mock.h"

#include "common/logger.h"

#include <gtest/gtest.h>

using testing::_;
using testing::InSequence;
using testing::Return;

struct FakeSystemTimeStamp : public SystemTimeStamp
{
    FakeSystemTimeStamp(int uid) : uid(uid), time(0) {}

    int uid;
    long long time;
};


class CApplicationWrapper : public CApplication
{
public:
    virtual Event CreateUpdateEvent() override
    {
        return CApplication::CreateUpdateEvent();
    }
};

class ApplicationUT : public testing::Test
{
protected:
    ApplicationUT();

    virtual void SetUp() override;
    virtual void TearDown() override;

    void NextInstant(long long diff);

    SystemTimeStamp* CreateTimeStamp();
    void DestroyTimeStamp(SystemTimeStamp *stamp);
    void CopyTimeStamp(SystemTimeStamp *dst, SystemTimeStamp *src);
    void GetCurrentTimeStamp(SystemTimeStamp *stamp);
    long long TimeStampExactDiff(SystemTimeStamp *before, SystemTimeStamp *after);

    void TestCreateUpdateEvent(long long relTimeExact, long long absTimeExact,
                               float relTime, float absTime,
                               long long relTimeReal, long long absTimeReal);

protected:
    CLogger logger;
    CApplicationWrapper* app;
    CSystemUtilsMock* systemUtils;

private:
    int m_stampUid;
    long long m_currentTime;
};

ApplicationUT::ApplicationUT()
 : m_stampUid(0)
 , m_currentTime(0)
{}

void ApplicationUT::SetUp()
{
    systemUtils = new CSystemUtilsMock();

    ON_CALL(*systemUtils, CreateTimeStamp()).WillByDefault(Invoke(this, &ApplicationUT::CreateTimeStamp));
    ON_CALL(*systemUtils, DestroyTimeStamp(_)).WillByDefault(Invoke(this, &ApplicationUT::DestroyTimeStamp));
    ON_CALL(*systemUtils, CopyTimeStamp(_, _)).WillByDefault(Invoke(this, &ApplicationUT::CopyTimeStamp));
    ON_CALL(*systemUtils, GetCurrentTimeStamp(_)).WillByDefault(Invoke(this, &ApplicationUT::GetCurrentTimeStamp));
    ON_CALL(*systemUtils, TimeStampExactDiff(_, _)).WillByDefault(Invoke(this, &ApplicationUT::TimeStampExactDiff));

    EXPECT_CALL(*systemUtils, CreateTimeStamp()).Times(3 + PCNT_MAX*2);
    app = new CApplicationWrapper();
}

void ApplicationUT::TearDown()
{
    EXPECT_CALL(*systemUtils, DestroyTimeStamp(_)).Times(3 + PCNT_MAX*2);
    delete app;
    app = nullptr;

    delete systemUtils;
    systemUtils = nullptr;
}

SystemTimeStamp* ApplicationUT::CreateTimeStamp()
{
    return new FakeSystemTimeStamp(++m_stampUid);
}

void ApplicationUT::DestroyTimeStamp(SystemTimeStamp *stamp)
{
    delete static_cast<FakeSystemTimeStamp*>(stamp);
}

void ApplicationUT::CopyTimeStamp(SystemTimeStamp *dst, SystemTimeStamp *src)
{
    *static_cast<FakeSystemTimeStamp*>(dst) = *static_cast<FakeSystemTimeStamp*>(src);
}

void ApplicationUT::GetCurrentTimeStamp(SystemTimeStamp *stamp)
{
    static_cast<FakeSystemTimeStamp*>(stamp)->time = m_currentTime;
}

long long ApplicationUT::TimeStampExactDiff(SystemTimeStamp *before, SystemTimeStamp *after)
{
    return static_cast<FakeSystemTimeStamp*>(after)->time - static_cast<FakeSystemTimeStamp*>(before)->time;
}

void ApplicationUT::NextInstant(long long diff)
{
    m_currentTime += diff;
}

void ApplicationUT::TestCreateUpdateEvent(long long relTimeExact, long long absTimeExact,
                                          float relTime, float absTime,
                                          long long relTimeReal, long long absTimeReal)
{
    {
        InSequence seq;
        EXPECT_CALL(*systemUtils, CopyTimeStamp(_, _));
        EXPECT_CALL(*systemUtils, GetCurrentTimeStamp(_));
        EXPECT_CALL(*systemUtils, TimeStampExactDiff(_, _)).Times(2);
    }

    Event event = app->CreateUpdateEvent();
    EXPECT_EQ(EVENT_FRAME, event.type);
    EXPECT_FLOAT_EQ(relTime, event.rTime);
    EXPECT_FLOAT_EQ(relTime, app->GetRelTime());
    EXPECT_FLOAT_EQ(absTime, app->GetAbsTime());
    EXPECT_EQ(relTimeExact, app->GetExactRelTime());
    EXPECT_EQ(absTimeExact, app->GetExactAbsTime());
    EXPECT_EQ(relTimeReal, app->GetRealRelTime());
    EXPECT_EQ(absTimeReal, app->GetRealAbsTime());
}


TEST_F(ApplicationUT, UpdateEventTimeCalculation_SimulationSuspended)
{
    app->SuspendSimulation();
    Event event = app->CreateUpdateEvent();
    EXPECT_EQ(EVENT_NULL, event.type);
}

TEST_F(ApplicationUT, UpdateEventTimeCalculation_NormalOperation)
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

TEST_F(ApplicationUT, UpdateEventTimeCalculation_NegativeTimeOperation)
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

    {
        InSequence seq;
        EXPECT_CALL(*systemUtils, CopyTimeStamp(_, _));
        EXPECT_CALL(*systemUtils, GetCurrentTimeStamp(_));
        EXPECT_CALL(*systemUtils, TimeStampExactDiff(_, _)).Times(2);
    }
    Event event = app->CreateUpdateEvent();
    EXPECT_EQ(EVENT_NULL, event.type);
}

TEST_F(ApplicationUT, UpdateEventTimeCalculation_ChangingSimulationSpeed)
{
    EXPECT_CALL(*systemUtils, GetCurrentTimeStamp(_));
    app->SetSimulationSpeed(2.0f);

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
    EXPECT_CALL(*systemUtils, GetCurrentTimeStamp(_));
    app->SetSimulationSpeed(4.0f);

    relTimeReal = 300;
    absTimeReal += relTimeReal;
    relTimeExact = relTimeReal*4;
    absTimeExact += relTimeReal*4;
    relTime = relTimeExact / 1e9f;
    absTime = absTimeExact / 1e9f;

    NextInstant(relTimeReal);

    TestCreateUpdateEvent(relTimeExact, absTimeExact, relTime, absTime, relTimeReal, absTimeReal);

    // 4th update -- speed 1x
    EXPECT_CALL(*systemUtils, GetCurrentTimeStamp(_));
    app->SetSimulationSpeed(1.0f);

    relTimeReal = 400;
    absTimeReal += relTimeReal;
    relTimeExact = relTimeReal;
    absTimeExact += relTimeReal;
    relTime = relTimeExact / 1e9f;
    absTime = absTimeExact / 1e9f;

    NextInstant(relTimeReal);

    TestCreateUpdateEvent(relTimeExact, absTimeExact, relTime, absTime, relTimeReal, absTimeReal);
}

TEST_F(ApplicationUT, UpdateEventTimeCalculation_SuspendingAndResumingSimulation)
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

    app->SuspendSimulation();

    long long suspensionTime = 5000;

    NextInstant(suspensionTime);

    // 3rd update -- simulation resumed

    {
        InSequence seq;
        EXPECT_CALL(*systemUtils, GetCurrentTimeStamp(_));
        EXPECT_CALL(*systemUtils, CopyTimeStamp(_, _));
    }
    app->ResumeSimulation();

    relTimeReal = 200;
    absTimeReal += relTimeReal;
    relTimeExact = relTimeReal;
    absTimeExact += relTimeReal;
    relTime = relTimeExact / 1e9f;
    absTime = absTimeExact / 1e9f;

    NextInstant(relTimeReal);

    TestCreateUpdateEvent(relTimeExact, absTimeExact, relTime, absTime, relTimeReal, absTimeReal);
}
