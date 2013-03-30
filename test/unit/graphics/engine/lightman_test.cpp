#include "graphics/engine/lightman.h"

#include "app/system_mock.h"

#include "graphics/core/device_mock.h"
#include "graphics/engine/engine_mock.h"

#include <gtest/gtest.h>

using namespace Gfx;

using testing::_;
using testing::Invoke;
using testing::Return;

class LightManagerUT : public testing::Test
{
protected:
    LightManagerUT()
      : systemUtils(true)
      , lightManager(&engine)
    {}

    void PrepareLightTesting(int maxLights, Math::Vector eyePos);
    void CheckLightSorting(EngineObjectType objectType, const std::vector<int>& expectedLights);
    void CheckLight(int index, const Light& light);
    void AddLight(int type, LightPriority priority, bool used, bool enabled,
                  Math::Vector pos, EngineObjectType includeType, EngineObjectType excludeType);


    CSystemUtilsMock systemUtils;
    CLightManager lightManager;
    CEngineMock engine;
    CDeviceMock device;

private:
    std::vector<DynamicLight> dynamicLights;
    std::vector<int> expectedLightTypes;
    int maxLightsCount;
};

void LightManagerUT::PrepareLightTesting(int maxLights, Math::Vector eyePos)
{
    maxLightsCount = maxLights;

    EXPECT_CALL(device, GetMaxLightCount()).WillOnce(Return(maxLights));
    lightManager.SetDevice(&device);

    ON_CALL(device, SetLight(_, _)).WillByDefault(Invoke(this, &LightManagerUT::CheckLight));

    EXPECT_CALL(engine, GetEyePt()).WillRepeatedly(Return(eyePos));
}

void LightManagerUT::CheckLightSorting(EngineObjectType objectType, const std::vector<int>& expectedLights)
{
    expectedLightTypes = expectedLights;

    EXPECT_CALL(device, SetLight(_, _)).Times(expectedLights.size());

    for (int i = 0; i < static_cast<int>( expectedLights.size() ); ++i)
        EXPECT_CALL(device, SetLightEnabled(i, true));

    for (int i = expectedLights.size(); i < maxLightsCount; ++i)
        EXPECT_CALL(device, SetLightEnabled(i, false));

    lightManager.UpdateDeviceLights(objectType);
}

void LightManagerUT::CheckLight(int index, const Light& light)
{
    ASSERT_TRUE(index >= 0 && index < static_cast<int>( expectedLightTypes.size() ));
    ASSERT_EQ(expectedLightTypes[index], light.type);
}

void LightManagerUT::AddLight(int type, LightPriority priority, bool used, bool enabled,
                              Math::Vector pos, EngineObjectType includeType, EngineObjectType excludeType)
{
    int rank = lightManager.CreateLight(priority);

    Light light;
    light.type = static_cast<LightType>(type);
    light.position = pos;
    lightManager.SetLight(rank, light);

    lightManager.SetLightEnabled(rank, enabled);
    lightManager.SetLightIncludeType(rank, includeType);
    lightManager.SetLightExcludeType(rank, excludeType);

    if (!used)
        lightManager.DeleteLight(rank);
}

TEST_F(LightManagerUT, LightSorting_UnusedOrDisabledAreSkipped)
{
    const int lightCount = 10;
    const Math::Vector eyePos(0.0f, 0.0f, 0.0f);
    PrepareLightTesting(lightCount, eyePos);

    AddLight(1, LIGHT_PRI_LOW,  false,  true, Math::Vector(0.0f, 0.0f, 0.0f), ENG_OBJTYPE_NULL, ENG_OBJTYPE_NULL);
    AddLight(2, LIGHT_PRI_LOW,  true,  false, Math::Vector(0.0f, 0.0f, 0.0f), ENG_OBJTYPE_NULL, ENG_OBJTYPE_NULL);
    AddLight(3, LIGHT_PRI_LOW,  false, false, Math::Vector(0.0f, 0.0f, 0.0f), ENG_OBJTYPE_NULL, ENG_OBJTYPE_NULL);

    std::vector<int> expectedLights;
    CheckLightSorting(ENG_OBJTYPE_TERRAIN, expectedLights);
}

TEST_F(LightManagerUT, LightSorting_IncludeTypesAreIncluded)
{
    const int lightCount = 10;
    const Math::Vector eyePos(0.0f, 0.0f, 0.0f);
    PrepareLightTesting(lightCount, eyePos);

    AddLight(1, LIGHT_PRI_LOW, true, true, Math::Vector(0.0f, 0.0f, 0.0f), ENG_OBJTYPE_NULL,    ENG_OBJTYPE_NULL);
    AddLight(2, LIGHT_PRI_LOW, true, true, Math::Vector(0.0f, 0.0f, 0.0f), ENG_OBJTYPE_TERRAIN, ENG_OBJTYPE_NULL);
    AddLight(3, LIGHT_PRI_LOW, true, true, Math::Vector(0.0f, 0.0f, 0.0f), ENG_OBJTYPE_QUARTZ,  ENG_OBJTYPE_NULL);

    std::vector<int> expectedLights = { 1, 2 };
    CheckLightSorting(ENG_OBJTYPE_TERRAIN, expectedLights);
}

TEST_F(LightManagerUT, LightSorting_ExcludeTypesAreExcluded)
{
    const int lightCount = 10;
    const Math::Vector eyePos(0.0f, 0.0f, 0.0f);
    PrepareLightTesting(lightCount, eyePos);

    AddLight(1, LIGHT_PRI_LOW, true, true, Math::Vector(0.0f, 0.0f, 0.0f), ENG_OBJTYPE_NULL, ENG_OBJTYPE_NULL);
    AddLight(2, LIGHT_PRI_LOW, true, true, Math::Vector(0.0f, 0.0f, 0.0f), ENG_OBJTYPE_NULL, ENG_OBJTYPE_TERRAIN);
    AddLight(3, LIGHT_PRI_LOW, true, true, Math::Vector(0.0f, 0.0f, 0.0f), ENG_OBJTYPE_NULL, ENG_OBJTYPE_QUARTZ);

    std::vector<int> expectedLights = { 1, 3 };
    CheckLightSorting(ENG_OBJTYPE_TERRAIN, expectedLights);
}

TEST_F(LightManagerUT, LightSorting_SortingAccordingToDistance)
{
    const int lightCount = 3;
    const Math::Vector eyePos(0.0f, 0.0f, 0.0f);
    PrepareLightTesting(lightCount, eyePos);

    AddLight(1, LIGHT_PRI_HIGH, true, true, Math::Vector(10.0f, 0.0f, 0.0f),  ENG_OBJTYPE_NULL, ENG_OBJTYPE_NULL);
    AddLight(2, LIGHT_PRI_LOW,  true, true, Math::Vector(4.0f, 0.0f, 0.0f),   ENG_OBJTYPE_NULL, ENG_OBJTYPE_NULL);
    AddLight(3, LIGHT_PRI_HIGH, true, true, Math::Vector(20.0f, 0.0f, 0.0f),  ENG_OBJTYPE_NULL, ENG_OBJTYPE_NULL);
    AddLight(4, LIGHT_PRI_LOW,  true, true, Math::Vector(11.0f, 0.0f, 0.0f),  ENG_OBJTYPE_NULL, ENG_OBJTYPE_NULL);
    AddLight(5, LIGHT_PRI_LOW,  true, true, Math::Vector(100.0f, 0.0f, 0.0f), ENG_OBJTYPE_NULL, ENG_OBJTYPE_NULL);
    AddLight(6, LIGHT_PRI_HIGH, true, true, Math::Vector(21.0f, 0.0f, 0.0f),  ENG_OBJTYPE_NULL, ENG_OBJTYPE_NULL);

    std::vector<int> expectedLights = { 2, 1, 3 };
    CheckLightSorting(ENG_OBJTYPE_TERRAIN, expectedLights);
}
