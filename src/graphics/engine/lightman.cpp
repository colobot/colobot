// * This file is part of the COLOBOT source code
// * Copyright (C) 2001-2008, Daniel ROUX & EPSITEC SA, www.epsitec.ch
// * Copyright (C) 2012, Polish Portal of Colobot (PPC)
// *
// * This program is free software: you can redistribute it and/or modify
// * it under the terms of the GNU General Public License as published by
// * the Free Software Foundation, either version 3 of the License, or
// * (at your option) any later version.
// *
// * This program is distributed in the hope that it will be useful,
// * but WITHOUT ANY WARRANTY; without even the implied warranty of
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// * GNU General Public License for more details.
// *
// * You should have received a copy of the GNU General Public License
// * along with this program. If not, see  http://www.gnu.org/licenses/.


#include "graphics/engine/lightman.h"

#include "common/logger.h"

#include "graphics/core/device.h"

#include "math/geometry.h"


#include <cmath>


// Graphics module namespace
namespace Gfx {


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


DynamicLight::DynamicLight()
{
    used = enabled = false;
    priority = LIGHT_PRI_LOW;
    includeType = excludeType = ENG_OBJTYPE_NULL;
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
    m_lightMap = std::vector<int>(m_device->GetMaxLightCount(), -1);
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
    m_dynLights[index].used     = true;
    m_dynLights[index].enabled  = true;
    m_dynLights[index].priority = priority;

    m_dynLights[index].includeType = ENG_OBJTYPE_NULL;
    m_dynLights[index].excludeType = ENG_OBJTYPE_NULL;

    m_dynLights[index].light.type      = LIGHT_DIRECTIONAL;
    m_dynLights[index].light.diffuse   = Color(0.5f, 0.5f, 0.5f);
    m_dynLights[index].light.position  = Math::Vector(-100.0f,  100.0f, -100.0f);
    m_dynLights[index].light.direction = Math::Vector( 1.0f, -1.0f,  1.0f);

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

bool CLightManager::SetLightPos(int lightRank, const Math::Vector &pos)
{
    if ( (lightRank < 0) || (lightRank >= static_cast<int>( m_dynLights.size() )) )
        return false;

    m_dynLights[lightRank].light.position = pos;
    return true;
}

Math::Vector CLightManager::GetLightPos(int lightRank)
{
    if ( (lightRank < 0) || (lightRank >= static_cast<int>( m_dynLights.size() )) )
        return Math::Vector(0.0f, 0.0f, 0.0f);

    return m_dynLights[lightRank].light.position;
}

bool CLightManager::SetLightDir(int lightRank, const Math::Vector &dir)
{
    if ( (lightRank < 0) || (lightRank >= static_cast<int>( m_dynLights.size() )) )
        return false;

    m_dynLights[lightRank].light.direction = dir;
    return true;
}

Math::Vector CLightManager::GetLightDir(int lightRank)
{
    if ( (lightRank < 0) || (lightRank >= static_cast<int>( m_dynLights.size() )) )
        return Math::Vector(0.0f, 0.0f, 0.0f);

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
            Math::Vector dir = m_engine->GetEyePt() - m_engine->GetLookatPt();
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

    // High priority
    for (int i = 0; i < static_cast<int>( m_dynLights.size() ); i++)
    {
        if (! m_dynLights[i].used)
            continue;
        if (! m_dynLights[i].enabled)
            continue;
        if (Math::IsZero(m_dynLights[i].intensity.current))
            continue;
        if (m_dynLights[i].priority == LIGHT_PRI_LOW)
            continue;

        bool enabled = true;
        if (m_dynLights[i].includeType != ENG_OBJTYPE_NULL)
            enabled = (m_dynLights[i].includeType == type);

        if (m_dynLights[i].excludeType != ENG_OBJTYPE_NULL)
            enabled = (m_dynLights[i].excludeType != type);

        if (enabled)
        {
            for (int j = 0; j < static_cast<int>( m_lightMap.size() ); ++j)
            {
                if (m_lightMap[j] == -1)
                {
                    m_lightMap[j] = i;
                    break;
                }
            }
        }
    }

    // Low priority
    for (int i = 0; i < static_cast<int>( m_dynLights.size() ); i++)
    {
        if (! m_dynLights[i].used)
            continue;
        if (! m_dynLights[i].enabled)
            continue;
        if (m_dynLights[i].intensity.current == 0.0f)
            continue;
        if (m_dynLights[i].priority == LIGHT_PRI_HIGH)
            continue;

        bool enabled = true;
        if (m_dynLights[i].includeType != ENG_OBJTYPE_NULL)
            enabled = (m_dynLights[i].includeType == type);

        if (m_dynLights[i].excludeType != ENG_OBJTYPE_NULL)
            enabled = (m_dynLights[i].excludeType != type);

        if (enabled)
        {
            for (int j = 0; j < static_cast<int>( m_lightMap.size() ); ++j)
            {
                if (m_lightMap[j] == -1)
                {
                    m_lightMap[j] = i;
                    break;
                }
            }
        }
    }

    for (int i = 0; i < static_cast<int>( m_lightMap.size() ); ++i)
    {
        int rank = m_lightMap[i];
        if (rank != -1)
        {
            m_device->SetLight(i, m_dynLights[rank].light);
            m_device->SetLightEnabled(i, true);
        }
        else
        {
            m_device->SetLightEnabled(i, false);
        }
    }
}


} // namespace Gfx
