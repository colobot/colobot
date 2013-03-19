#include "app/system.h"
#include "app/system_linux.h"

#include <gtest/gtest.h>

TEST(SystemLinuxTest, TimeStampDiff)
{
    const long long SEC = 1000000000;

    SystemTimeStamp before, after;

    before.clockTime.tv_sec = 1;
    before.clockTime.tv_nsec = 100;

    after.clockTime.tv_sec = 1;
    after.clockTime.tv_nsec = 900;

    long long tDiff = TimeStampExactDiff_Linux(&before, &after);
    EXPECT_EQ( 800, tDiff);

    tDiff = TimeStampExactDiff_Linux(&after, &before);
    EXPECT_EQ(-800, tDiff);

    // -------

    before.clockTime.tv_sec = 2;
    before.clockTime.tv_nsec = 200;

    after.clockTime.tv_sec = 3;
    after.clockTime.tv_nsec = 500;

    tDiff = TimeStampExactDiff_Linux(&before, &after);
    EXPECT_EQ( SEC + 300, tDiff);

    tDiff = TimeStampExactDiff_Linux(&after, &before);
    EXPECT_EQ(-SEC - 300, tDiff);

    // -------

    before.clockTime.tv_sec = 3;
    before.clockTime.tv_nsec = 200;

    after.clockTime.tv_sec = 4;
    after.clockTime.tv_nsec = 100;

    tDiff = TimeStampExactDiff_Linux(&before, &after);
    EXPECT_EQ( SEC - 100, tDiff);

    tDiff = TimeStampExactDiff_Linux(&after, &before);
    EXPECT_EQ(-SEC + 100, tDiff);
}
