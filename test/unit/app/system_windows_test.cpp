#include "app/system.h"
#include "app/system_windows.h"

#include <gtest/gtest.h>

class CSystemUtilsWindowsWrapper : public CSystemUtilsWindows
{
public:
    CSystemUtilsWindowsWrapper() {}

    void SetFrequency(long long counterFrequency)
    {
        m_counterFrequency = counterFrequency;
    }
};

class SystemUtilsWindowsUT : public testing::Test
{
protected:
    static const long long SEC = 1000000000;

    CSystemUtilsWindowsWrapper systemUtils;
};


TEST_F(SystemUtilsWindowsUT, TimerResolution)
{
    systemUtils.SetFrequency(SEC);
    EXPECT_EQ(1u, systemUtils.GetTimeStampExactResolution());

    systemUtils.SetFrequency(SEC/3);
    EXPECT_EQ(3u, systemUtils.GetTimeStampExactResolution());
}

TEST_F(SystemUtilsWindowsUT, TimeStampDiff)
{
    systemUtils.SetFrequency(SEC);

    SystemTimeStamp before, after;

    before.counterValue = 100;
    after.counterValue = 200;

    long long tDiff = systemUtils.TimeStampExactDiff(&before, &after);
    EXPECT_EQ( 100, tDiff);

    tDiff = systemUtils.TimeStampExactDiff(&after, &before);
    EXPECT_EQ(-100, tDiff);

    // -------

    systemUtils.SetFrequency(SEC/3);

    before.counterValue = 200;
    after.counterValue = 400;

    tDiff = systemUtils.TimeStampExactDiff(&before, &after);
    EXPECT_EQ( 200*3, tDiff);

    tDiff = systemUtils.TimeStampExactDiff(&after, &before);
    EXPECT_EQ(-200*3, tDiff);
}
