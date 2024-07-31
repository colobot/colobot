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


#include "graphics/engine/lightman.h"

#include "common/logger.h"

#include "graphics/core/device.h"

#include "graphics/engine/engine.h"

#include "math/geometry.h"


#include <cmath>
#include <algorithm>


// Graphics module namespace
namespace Gfx
{


void LightProgression::Init(float value)
{
    starting = value;
    ending   = value;
    current  = value;
    progress = 0.0f;
    speed    = 100.0f;
}

void LightProgression::Update(float rTime)
{
    if (speed < 100.0f)
    {
        if (progress < 1.0f)
        {
            progress += speed * rTime;
            if (progress > 1.0f)
                progress = 1.0f;
        }

        current = starting + progress * (ending - starting);
    }
    else
    {
        current = ending;
    }
}

void LightProgression::SetTarget(float value)
{
    starting = current;
    ending   = value;
    progress = 0.0f;
}


CLightManager::CLightManager(CEngine* engine)
{
    m_device = nullptr;
    m_engine = engine;

    m_time = 0.0f;
}

CLightManager::~CLightManager()
{
    m_device = nullptr;
    m_engine = nullptr;
}

void CLightManager::SetDevice(CDevice* device)
{
    m_device = device;
    m_lightMap = std::vector<int>(8, -1);
}

void CLightManager::DebugDumpLights()
{
    CLogger* l = GetLogger();

    l->Debug("Dynamic lights:");

    for (int i = 0; i < static_cast<int>( m_dynLights.size() ); ++i)
    {
        const DynamicLight& dynLight = m_dynLights[i];
        if (!dynLight.used)
            continue;

        int deviceLight = -1;
        for (int j = 0; j < static_cast<int>( m_lightMap.size() ); ++j)
        {
            if (m_lightMap[j] == i)
            {
                deviceLight = j;
                break;
            }
        }

        l->Debug(" light %%", i);
        l->Debug("  enabled = %%", dynLight.enabled);
        l->Debug("  priority = %%", dynLight.priority);
        l->Debug("  device light = %%", deviceLight);
        l->Debug("  light:");

        const Light& light = dynLight.light;
        std::string str;

        l->Debug("   type = %%", light.type);
        str = light.ambient.ToString();
        l->Debug("   ambient = %%", str);
        str = light.diffuse.ToString();
        l->Debug("   diffuse = %%", str);
        str = light.specular.ToString();
        l->Debug("   specular = %%", str);
        str = Math::ToString(light.position);
        l->Debug("   position = %%", str);
        str = Math::ToString(light.direction);
        l->Debug("   direction = %%", str);
        l->Debug("   attenuation0 = %%", light.attenuation0);
        l->Debug("   attenuation1 = %%", light.attenuation1);
        l->Debug("   attenuation2 = %%", light.attenuation2);
        l->Debug("   spotAngle = %%", light.spotAngle);
        l->Debug("   spotIntensity = %%", light.spotIntensity);

        l->Debug(" intensity: %%", dynLight.intensity.current);
        l->Debug(" color: %% %% %%", dynLight.colorRed.current, dynLight.colorGreen.current, dynLight.colorBlue.current);
        l->Debug(" includeType: %%", dynLight.includeType);
        l->Debug(" excludeType: %%", dynLight.excludeType);
    }
}

void CLightManager::FlushLights()
{
    m_dynLights.clear();
}

/** Returns the index of light created. */
int CLightManager::CreateLight(LightPriority priority)
{
    int index = 0;
    for (; index < static_cast<int>( m_dynLights.size() ); index++)
    {
        if (! m_dynLights[index].used)
            break;
    }

    if (index == static_cast<int>(m_dynLights.size()))
        m_dynLights.push_back(DynamicLight());

    m_dynLights[index] = DynamicLight();
    m_dynLights[index].rank     = index;
    m_dynLights[index].used     = true;
    m_dynLights[index].enabled  = true;
    m_dynLights[index].priority = priority;

    m_dynLights[index].includeType = ENG_OBJTYPE_NULL;
    m_dynLights[index].excludeType = ENG_OBJTYPE_NULL;

    m_dynLights[index].light.type      = LIGHT_DIRECTIONAL;
    m_dynLights[index].light.diffuse   = Color(0.5f, 0.5f, 0.5f);
    m_dynLights[index].light.ambient   = Color(0.0f, 0.0f, 0.0f);
    m_dynLights[index].light.position  = glm::vec3(-100.0f,  100.0f, -100.0f);
    m_dynLights[index].light.direction = glm::vec3( 1.0f, -1.0f,  1.0f);

    m_dynLights[index].intensity.Init(1.0f);  // maximum
    m_dynLights[index].colorRed.Init(0.5f);
    m_dynLights[index].colorGreen.Init(0.5f);
    m_dynLights[index].colorBlue.Init(0.5f);  // gray

    return index;
}

bool CLightManager::DeleteLight(int lightRank)
{
    if ( (lightRank < 0) || (lightRank >= static_cast<int>( m_dynLights.size() )) )
        return false;

    m_dynLights[lightRank].used = false;
    return true;
}

bool CLightManager::SetLight(int lightRank, const Light &light)
{
    if ( (lightRank < 0) || (lightRank >= static_cast<int>( m_dynLights.size() )) )
        return false;

    m_dynLights[lightRank].light = light;

    m_dynLights[lightRank].colorRed.Init(m_dynLights[lightRank].light.diffuse.r);
    m_dynLights[lightRank].colorGreen.Init(m_dynLights[lightRank].light.diffuse.g);
    m_dynLights[lightRank].colorBlue.Init(m_dynLights[lightRank].light.diffuse.b);

    return true;
}

bool CLightManager::GetLight(int lightRank, Light &light)
{
    if ( (lightRank < 0) || (lightRank >= static_cast<int>( m_dynLights.size() )) )
        return false;

    light = m_dynLights[lightRank].light;
    return true;
}

bool CLightManager::SetLightEnabled(int lightRank, bool enabled)
{
    if ( (lightRank < 0) || (lightRank >= static_cast<int>( m_dynLights.size() )) )
        return false;

    m_dynLights[lightRank].enabled = enabled;
    return true;
}

bool CLightManager::SetLightPriority(int lightRank, LightPriority priority)
{
    if ( (lightRank < 0) || (lightRank >= static_cast<int>( m_dynLights.size() )) )
        return false;

    m_dynLights[lightRank].priority = priority;
    return true;
}

bool CLightManager::SetLightIncludeType(int lightRank, EngineObjectType type)
{
    if ( (lightRank < 0) || (lightRank >= static_cast<int>( m_dynLights.size() )) )
        return false;

    m_dynLights[lightRank].includeType = type;
    return true;
}

bool CLightManager::SetLightExcludeType(int lightRank, EngineObjectType type)
{
    if ( (lightRank < 0) || (lightRank >= static_cast<int>( m_dynLights.size() )) )
        return false;

    m_dynLights[lightRank].excludeType = type;
    return true;
}

bool CLightManager::SetLightPos(int lightRank, const glm::vec3 &pos)
{
    if ( (lightRank < 0) || (lightRank >= static_cast<int>( m_dynLights.size() )) )
        return false;

    m_dynLights[lightRank].light.position = pos;
    return true;
}

glm::vec3 CLightManager::GetLightPos(int lightRank)
{
    if ( (lightRank < 0) || (lightRank >= static_cast<int>( m_dynLights.size() )) )
        return glm::vec3(0.0f, 0.0f, 0.0f);

    return m_dynLights[lightRank].light.position;
}

bool CLightManager::SetLightDir(int lightRank, const glm::vec3 &dir)
{
    if ( (lightRank < 0) || (lightRank >= static_cast<int>( m_dynLights.size() )) )
        return false;

    m_dynLights[lightRank].light.direction = dir;
    return true;
}

glm::vec3 CLightManager::GetLightDir(int lightRank)
{
    if ( (lightRank < 0) || (lightRank >= static_cast<int>( m_dynLights.size() )) )
        return glm::vec3(0.0f, 0.0f, 0.0f);

    return m_dynLights[lightRank].light.direction;
}

bool CLightManager::SetLightIntensitySpeed(int lightRank, float speed)
{
    if ( (lightRank < 0) || (lightRank >= static_cast<int>( m_dynLights.size() )) )
        return false;

    m_dynLights[lightRank].intensity.speed = speed;
    return true;
}

bool CLightManager::SetLightIntensity(int lightRank, float value)
{
    if ( (lightRank < 0) || (lightRank >= static_cast<int>( m_dynLights.size() )) )
        return false;

    m_dynLights[lightRank].intensity.SetTarget(value);
    return true;
}

float CLightManager::GetLightIntensity(int lightRank)
{
    if ( (lightRank < 0) || (lightRank >= static_cast<int>( m_dynLights.size() )) )
        return 0.0f;

    return m_dynLights[lightRank].intensity.current;
}


bool CLightManager::SetLightColorSpeed(int lightRank, float speed)
{
    if ( (lightRank < 0) || (lightRank >= static_cast<int>( m_dynLights.size() )) )
        return false;

    m_dynLights[lightRank].colorRed.speed   = speed;
    m_dynLights[lightRank].colorGreen.speed = speed;
    m_dynLights[lightRank].colorBlue.speed  = speed;
    return true;
}

bool CLightManager::SetLightColor(int lightRank, const Color &color)
{
    if ( (lightRank < 0) || (lightRank >= static_cast<int>( m_dynLights.size() )) )
        return false;

    m_dynLights[lightRank].colorRed.SetTarget(color.r);
    m_dynLights[lightRank].colorGreen.SetTarget(color.g);
    m_dynLights[lightRank].colorBlue.SetTarget(color.b);
    return true;
}

Color CLightManager::GetLightColor(int lightRank)
{
    if ( (lightRank < 0) || (lightRank >= static_cast<int>( m_dynLights.size() )) )
        return Color(0.5f, 0.5f, 0.5f, 0.5f);

    Color color;
    color.r = m_dynLights[lightRank].colorRed.current;
    color.g = m_dynLights[lightRank].colorGreen.current;
    color.b = m_dynLights[lightRank].colorBlue.current;
    return color;
}

void CLightManager::AdaptLightColor(const Color &color, float factor)
{
    for (int i = 0; i < static_cast<int>( m_dynLights.size() ); i++)
    {
        if (! m_dynLights[i].used)
            continue;

        Color value;
        value.r = m_dynLights[i].colorRed.current;
        value.g = m_dynLights[i].colorGreen.current;
        value.b = m_dynLights[i].colorBlue.current;

        value.r += color.r * factor;
        value.g += color.g * factor;
        value.b += color.b * factor;

        m_dynLights[i].colorRed.Init(value.r);
        m_dynLights[i].colorGreen.Init(value.g);
        m_dynLights[i].colorBlue.Init(value.b);
    }

    UpdateLights();
}

void CLightManager::UpdateProgression(float rTime)
{
    if (m_engine->GetPause())
        return;

    m_time += rTime;

    for (int i = 0; i < static_cast<int>( m_dynLights.size() ); i++)
    {
        if (! m_dynLights[i].used)
            continue;

        m_dynLights[i].intensity.Update(rTime);
        m_dynLights[i].colorRed.Update(rTime);
        m_dynLights[i].colorGreen.Update(rTime);
        m_dynLights[i].colorBlue.Update(rTime);

        if (m_dynLights[i].includeType == ENG_OBJTYPE_QUARTZ)
        {
            m_dynLights[i].light.direction.x = sinf(1.0f * (m_time + i*Math::PI*0.5f));
            m_dynLights[i].light.direction.z = cosf(1.1f * (m_time + i*Math::PI*0.5f));
            m_dynLights[i].light.direction.y = -1.0f + 0.5f * cosf((m_time + i*Math::PI*0.5f)*2.7f);
        }

        if (m_dynLights[i].includeType == ENG_OBJTYPE_METAL)
        {
            glm::vec3 dir = m_engine->GetEyePt() - m_engine->GetLookatPt();
            float angle = Math::RotateAngle(dir.x, dir.z);
            angle += Math::PI * 0.5f * i;
            m_dynLights[i].light.direction.x = sinf(2.0f * angle);
            m_dynLights[i].light.direction.z = cosf(2.0f * angle);
        }
    }
}

void CLightManager::UpdateLights()
{
    for (int i = 0; i < static_cast<int>( m_dynLights.size() ); i++)
    {
        if (! m_dynLights[i].used)
            continue;

        bool enabled = m_dynLights[i].enabled;

        if (Math::IsZero(m_dynLights[i].intensity.current))
            enabled = false;

        if (enabled)
        {
            float value = m_dynLights[i].colorRed.current * m_dynLights[i].intensity.current;
            m_dynLights[i].light.diffuse.r = value;

            value = m_dynLights[i].colorGreen.current * m_dynLights[i].intensity.current;
            m_dynLights[i].light.diffuse.g = value;

            value = m_dynLights[i].colorBlue.current * m_dynLights[i].intensity.current;
            m_dynLights[i].light.diffuse.b = value;
        }
        else
        {
            m_dynLights[i].light.diffuse.r = 0.0f;
            m_dynLights[i].light.diffuse.g = 0.0f;
            m_dynLights[i].light.diffuse.b = 0.0f;
        }
    }
}

void CLightManager::UpdateDeviceLights(EngineObjectType type)
{
    for (int i = 0; i < static_cast<int>( m_lightMap.size() ); ++i)
        m_lightMap[i] = -1;

    std::vector<DynamicLight> sortedLights = m_dynLights;
    CLightsComparator lightsComparator(m_engine->GetEyePt(), type);
    std::sort(sortedLights.begin(), sortedLights.end(), lightsComparator);

    int lightMapIndex = 0;
    for (int i = 0; i < static_cast<int>( sortedLights.size() ); i++)
    {
        if (! sortedLights[i].used)
            continue;
        if (! sortedLights[i].enabled)
            continue;
        if (sortedLights[i].intensity.current == 0.0f)
            continue;

        bool enabled = true;
        if (sortedLights[i].includeType != ENG_OBJTYPE_NULL)
            enabled = (sortedLights[i].includeType == type);

        if (sortedLights[i].excludeType != ENG_OBJTYPE_NULL)
            enabled = (sortedLights[i].excludeType != type);

        if (enabled)
        {
            m_lightMap[lightMapIndex] = sortedLights[i].rank;
            ++lightMapIndex;
        }

        if (lightMapIndex >= static_cast<int>( m_lightMap.size() ))
            break;
    }

    /*
    for (int i = 0; i < static_cast<int>( m_lightMap.size() ); ++i)
    {
        int rank = m_lightMap[i];
        if (rank != -1)
        {
            Light light = m_dynLights[rank].light;
            m_device->SetLight(i, light);
            m_device->SetLightEnabled(i, true);
        }
        else
        {
            m_device->SetLightEnabled(i, false);
        }
    }
    */
}

// -----------

CLightManager::CLightsComparator::CLightsComparator(glm::vec3 eyePos, EngineObjectType objectType)
{
    m_eyePos = eyePos;
    m_objectType = objectType;
}

float CLightManager::CLightsComparator::GetLightWeight(const DynamicLight& dynLight)
{
    if (dynLight.priority == LIGHT_PRI_HIGHEST)
        return -1.0f;

    bool enabled = true;
    if (!dynLight.used || !dynLight.enabled || dynLight.intensity.current == 0.0f)
        enabled = false;
    else if (dynLight.includeType != ENG_OBJTYPE_NULL)
        enabled = dynLight.includeType == m_objectType;
    else if (dynLight.excludeType != ENG_OBJTYPE_NULL)
        enabled = dynLight.excludeType != m_objectType;

    return enabled ? ( glm::length(dynLight.light.position - m_eyePos) * dynLight.priority ) : 10000.0f;
}

bool CLightManager::CLightsComparator::operator()(const DynamicLight& left, const DynamicLight& right)
{
    float leftWeight = GetLightWeight(left);
    float rightWeight = GetLightWeight(right);

    return leftWeight < rightWeight;
}

} // namespace Gfx

