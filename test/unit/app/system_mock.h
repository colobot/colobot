#pragma once

#include "app/system.h"

#include <gmock/gmock.h>

class CSystemUtilsMock : public CSystemUtils
{
public:
    CSystemUtilsMock(bool defaultExpects = false)
    {
        if (defaultExpects)
            SetDefaultExpects();
    }

    virtual ~CSystemUtilsMock() {}

    void SetDefaultExpects()
    {
        using testing::_;
        using testing::Return;
        using testing::AnyNumber;

        EXPECT_CALL(*this, CreateTimeStamp()).Times(AnyNumber()).WillRepeatedly(Return(nullptr));
        EXPECT_CALL(*this, DestroyTimeStamp(_)).Times(AnyNumber());
        EXPECT_CALL(*this, CopyTimeStamp(_, _)).Times(AnyNumber());
        EXPECT_CALL(*this, GetCurrentTimeStamp(_)).Times(AnyNumber());

        EXPECT_CALL(*this, GetTimeStampResolution(_)).Times(AnyNumber()).WillRepeatedly(Return(0.0f));
        EXPECT_CALL(*this, GetTimeStampExactResolution()).Times(AnyNumber()).WillRepeatedly(Return(0ll));
        EXPECT_CALL(*this, TimeStampDiff(_, _, _)).Times(AnyNumber()).WillRepeatedly(Return(0.0f));
        EXPECT_CALL(*this, TimeStampExactDiff(_, _)).Times(AnyNumber()).WillRepeatedly(Return(0ll));
    }

    MOCK_METHOD0(Init, void());

    MOCK_METHOD3(SystemDialog, SystemDialogResult(SystemDialogType, const std::string &title, const std::string &message));
    MOCK_METHOD3(ConsoleSystemDialog, SystemDialogResult(SystemDialogType type, const std::string& title, const std::string& message));

    MOCK_METHOD0(CreateTimeStamp, SystemTimeStamp*());
    MOCK_METHOD1(DestroyTimeStamp, void (SystemTimeStamp *stamp));
    MOCK_METHOD2(CopyTimeStamp, void (SystemTimeStamp *dst, SystemTimeStamp *src));
    MOCK_METHOD1(GetCurrentTimeStamp, void (SystemTimeStamp *stamp));
    MOCK_METHOD1(GetTimeStampResolution, float (SystemTimeUnit unit));
    MOCK_METHOD0(GetTimeStampExactResolution, long long());
    MOCK_METHOD3(TimeStampDiff, float(SystemTimeStamp *before, SystemTimeStamp *after, SystemTimeUnit unit));
    MOCK_METHOD2(TimeStampExactDiff, long long(SystemTimeStamp *before, SystemTimeStamp *after));
};
