/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2018, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include "common/make_unique.h"

#include "graphics/core/device.h"

#include "graphics/engine/lightman.h"

#include <gtest/gtest.h>
#include <hippomocks.h>

#include <memory>
#include <functional>

using namespace Gfx;
using namespace HippoMocks;
namespace ph = std::placeholders;

class CLightManagerUT : public testing::Test
{
protected:
    CLightManagerUT() :
        m_engine(nullptr),
        m_device(nullptr),
        m_maxLightsCount(0)
    {}
    ~CLightManagerUT() NOEXCEPT
    {}

    void SetUp() override;

    void PrepareLightTesting(int maxLights, Math::Vector eyePos);
    void CheckLightSorting(EngineObjectType objectType, const std::vector<int>& expectedLights);
    void CheckLight(int index, const Light& light);
    void AddLight(int type, LightPriority priority, bool used, bool enabled,
                  Math::Vector pos, EngineObjectType includeType, EngineObjectType excludeType);


    std::unique_ptr<CLightManager> m_lightManager;
    MockRepository m_mocks;
    CEngine* m_engine;
    CDevice* m_device;

private:
    std::vector<DynamicLight> m_dynamicLights;
    std::vector<int> m_expectedLightTypes;
    int m_maxLightsCount;
};

void CLightManagerUT::SetUp()
{
    m_engine = m_mocks.Mock<CEngine>();
    m_device = m_mocks.Mock<CDevice>();

    m_lightManager = MakeUnique<CLightManager>(m_engine);
}

void CLightManagerUT::PrepareLightTesting(int maxLights, Math::Vector eyePos)
{
    m_maxLightsCount = maxLights;

    m_mocks.OnCall(m_device, CDevice::GetMaxLightCount).Return(maxLights);

    m_lightManager->SetDevice(m_device);

    m_mocks.OnCall(m_device, CDevice::SetLight).Do(std::bind(&CLightManagerUT::CheckLight, this, ph::_1, ph::_2));

    m_mocks.OnCall(m_engine, CEngine::GetEyePt).Return(eyePos);
}

void CLightManagerUT::CheckLightSorting(EngineObjectType objectType, const std::vector<int>& expectedLights)
{
    m_expectedLightTypes = expectedLights;

    for (int i = 0; i < m_maxLightsCount; ++i)
    {
        if (i < static_cast<int>( expectedLights.size() ))
        {
            m_mocks.ExpectCall(m_device, CDevice::SetLight).With(i, _);
            m_mocks.ExpectCall(m_device, CDevice::SetLightEnabled).With(i, true);
        }
        else
        {
            m_mocks.ExpectCall(m_device, CDevice::SetLightEnabled).With(i, false);
        }
    }

    m_lightManager->UpdateDeviceLights(objectType);
}

void CLightManagerUT::CheckLight(int index, const Light& light)
{
    ASSERT_TRUE(index >= 0 && index < static_cast<int>( m_expectedLightTypes.size() ));
    ASSERT_EQ(m_expectedLightTypes[index], light.type);
}

void CLightManagerUT::AddLight(int type, LightPriority priority, bool used, bool enabled,
                               Math::Vector pos, EngineObjectType includeType, EngineObjectType excludeType)
{
    int rank = m_lightManager->CreateLight(priority);

    Light light;
    light.type = static_cast<LightType>(type);
    light.position = pos;
    m_lightManager->SetLight(rank, light);

    m_lightManager->SetLightEnabled(rank, enabled);
    m_lightManager->SetLightIncludeType(rank, includeType);
    m_lightManager->SetLightExcludeType(rank, excludeType);

    if (!used)
        m_lightManager->DeleteLight(rank);
}

TEST_F(CLightManagerUT, LightSorting_UnusedOrDisabledAreSkipped)
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

TEST_F(CLightManagerUT, LightSorting_IncludeTypesAreIncluded)
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

TEST_F(CLightManagerUT, LightSorting_ExcludeTypesAreExcluded)
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

TEST_F(CLightManagerUT, LightSorting_SortingAccordingToDistance)
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
