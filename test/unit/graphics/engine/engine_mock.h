#include "graphics/engine/engine.h"

#include <gmock/gmock.h>

class CEngineMock : public Gfx::CEngine
{
public:
    CEngineMock() : Gfx::CEngine(nullptr) {}

    MOCK_METHOD0(GetPause, bool());

    MOCK_METHOD0(GetEyePt, Math::Vector());
    MOCK_METHOD0(GetLookatPt, Math::Vector());
};
