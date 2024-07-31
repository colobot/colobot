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

#include "graphics/engine/lightning.h"

#include "app/app.h"

#include "common/logger.h"

#include "graphics/core/device.h"
#include "graphics/core/transparency.h"

#include "graphics/engine/camera.h"
#include "graphics/engine/engine.h"
#include "graphics/engine/terrain.h"

#include "level/robotmain.h"

#include "math/geometry.h"

#include "object/object.h"
#include "object/object_manager.h"

#include "object/auto/autopowercaptor.h"

#include "object/interface/destroyable_object.h"
#include "object/interface/transportable_object.h"

#include "sound/sound.h"


// Graphics module namespace
namespace Gfx
{

namespace
{
const int LIGHTNING_SEGMENTS_COUNT = 50;
} // anonymous namespace


CLightning::CLightning(CEngine* engine)
    : m_engine(engine),
      m_segments(LIGHTNING_SEGMENTS_COUNT, LightningSegment())
{}

CLightning::~CLightning()
{
}

void CLightning::Flush()
{
    m_lightningExists = false;
    m_phase = LightningPhase::Wait;
    m_speed = 0.0f;
    m_progress = 0.0f;

    std::fill(m_segments.begin(), m_segments.end(), LightningSegment());
}

bool CLightning::EventProcess(const Event &event)
{
    if (event.type == EVENT_FRAME)
        return EventFrame(event);

    return true;
}

bool CLightning::EventFrame(const Event &event)
{
    if (m_terrain == nullptr)
        m_terrain = CRobotMain::GetInstancePointer()->GetTerrain();

    if (m_camera == nullptr)
        m_camera = CRobotMain::GetInstancePointer()->GetCamera();

    if (m_sound == nullptr)
        m_sound = CApplication::GetInstancePointer()->GetSound();

    if (m_engine->GetPause()) return true;
    if (CRobotMain::GetInstancePointer()->GetMovieLock()) return true;

    m_progress += event.rTime*m_speed;

    if (m_phase == LightningPhase::Wait && m_lightningExists)
    {
        if (m_progress >= 1.0f)
        {
            m_pos.x = (Math::Rand()-0.5f)*(3200.0f-200.0f);
            m_pos.z = (Math::Rand()-0.5f)*(3200.0f-200.0f);
            m_pos.y = 0.0f;

            CObject* obj = SearchObject(m_pos);
            if (obj == nullptr)
            {
                m_terrain->AdjustToFloor(m_pos, true);
            }
            else
            {
                m_pos = obj->GetPosition();
                m_terrain->AdjustToFloor(m_pos, true);

                // TODO: CLightningConductorObject
                ObjectType type = obj->GetType();
                if (type == OBJECT_BASE)
                {
                    m_pos.y += 120.0f;  // top of the rocket
                }
                else if (type == OBJECT_PARA)
                {
                    CAutoPowerCaptor* automat = static_cast<CAutoPowerCaptor*>(obj->GetAuto());
                    if (automat != nullptr)
                        automat->StartLightning();

                    m_pos.y += 67.0f;  // top of lightning rod
                }
                else
                {
                    assert(obj->Implements(ObjectInterfaceType::Destroyable));
                    dynamic_cast<CDestroyableObject*>(obj)->DamageObject(DamageType::Lightning, std::numeric_limits<float>::infinity());
                }
            }

            StrikeAtPos(m_pos);
        }
    }

    if (m_phase == LightningPhase::Flash)
    {
        if (m_progress < 1.0f)
        {
            float max = 5.0f;
            for (std::size_t i = 0; i < m_segments.size(); i++)
            {
                max += 0.4f;

                m_segments[i].shift.x += (Math::Rand()-0.5f)*max*2.0f;
                if ( m_segments[i].shift.x < -max )  m_segments[i].shift.x = -max;
                if ( m_segments[i].shift.x >  max )  m_segments[i].shift.x =  max;

                m_segments[i].shift.y += (Math::Rand()-0.5f)*max*2.0f;
                if ( m_segments[i].shift.y < -max )  m_segments[i].shift.y = -max;
                if ( m_segments[i].shift.y >  max )  m_segments[i].shift.y =  max;

                m_segments[i].width += (Math::Rand()-0.5f)*2.0f;
                if ( m_segments[i].width < 1.0f )  m_segments[i].width = 1.0f;
                if ( m_segments[i].width > 6.0f )  m_segments[i].width = 6.0f;
            }
            m_segments[0].shift.x = 0.0f;
            m_segments[0].shift.y = 0.0f;
            m_segments[0].width   = 0.0f;
        }
        else
        {
            m_phase    = LightningPhase::Wait;
            m_progress = 0.0f;
            m_speed    = 1.0f / (1.0f+Math::Rand()*m_delay);
        }
    }

    return true;
}

bool CLightning::Create(float sleep, float delay, float magnetic)
{
    m_lightningExists = true;
    if (sleep < 1.0f) sleep = 1.0f;
    m_sleep = sleep;
    m_delay = delay;
    m_magnetic = magnetic;

    m_phase    = LightningPhase::Wait;
    m_progress = 0.0f;
    m_speed    = 1.0f / m_sleep;

    return false;
}

bool CLightning::GetStatus(float &sleep, float &delay, float &magnetic, float &progress)
{
    if (! m_lightningExists) return false;

    sleep = m_sleep;
    delay = m_delay;
    magnetic = m_magnetic;
    progress = m_progress;

    return true;
}

bool CLightning::SetStatus(float sleep, float delay, float magnetic, float progress)
{
    m_lightningExists = true;

    m_sleep = sleep;
    m_delay = delay;
    m_magnetic = magnetic;
    m_progress = progress;
    m_phase = LightningPhase::Wait;
    m_speed = 1.0f/m_sleep;

    return true;
}

void CLightning::Draw()
{
    if (m_phase != LightningPhase::Flash) return;

    CDevice* device = m_engine->GetDevice();
    auto renderer = device->GetParticleRenderer();

    auto texture = m_engine->LoadTexture("textures/effect00.png");

    renderer->SetModelMatrix(glm::mat4(1.0f));
    renderer->SetTexture(texture);
    renderer->SetTransparency(TransparencyMode::BLACK);

    glm::vec2 texInf;
    texInf.x = 64.5f/256.0f;
    texInf.y = 33.0f/256.0f;
    glm::vec2 texSup;
    texSup.x = 95.5f/256.0f;
    texSup.y = 34.0f/256.0f;  // blank

    glm::vec3 p1 = m_pos;
    glm::vec3 eye = m_engine->GetEyePt();
    float a = Math::RotateAngle(eye.x-p1.x, eye.z-p1.z);

    glm::vec3 corner[4];
    VertexParticle vertex[4];

    for (std::size_t i = 0; i < m_segments.size() - 1; i++)
    {
        glm::vec3 p2 = p1;
        p2.y += 8.0f+0.2f*i;

        glm::vec2 rot;

        glm::vec3 p = p1;
        p.x += m_segments[i].width;
        rot = Math::RotatePoint({ p1.x, p1.z }, a + Math::PI / 2.0f, { p.x, p.z });
        corner[0].x = rot.x+m_segments[i].shift.x;
        corner[0].y = p1.y;
        corner[0].z = rot.y+m_segments[i].shift.y;
        rot = Math::RotatePoint({ p1.x, p1.z }, a - Math::PI / 2.0f, { p.x, p.z });
        corner[1].x = rot.x+m_segments[i].shift.x;
        corner[1].y = p1.y;
        corner[1].z = rot.y+m_segments[i].shift.y;

        p = p2;
        p.x += m_segments[i+1].width;
        rot = Math::RotatePoint({ p2.x, p2.z }, a + Math::PI / 2.0f, { p.x, p.z });
        corner[2].x = rot.x+m_segments[i+1].shift.x;
        corner[2].y = p2.y;
        corner[2].z = rot.y+m_segments[i+1].shift.y;
        rot = Math::RotatePoint({ p2.x, p2.z }, a - Math::PI / 2.0f, { p.x, p.z });
        corner[3].x = rot.x+m_segments[i+1].shift.x;
        corner[3].y = p2.y;
        corner[3].z = rot.y+m_segments[i+1].shift.y;

        IntColor white = IntColor(255, 255, 255, 255);

        if (p2.y < p1.y)
        {
            vertex[0] = { corner[1], white, { texSup.x, texSup.y } };
            vertex[1] = { corner[0], white, { texInf.x, texSup.y } };
            vertex[2] = { corner[3], white, { texSup.x, texInf.y } };
            vertex[3] = { corner[2], white, { texInf.x, texInf.y } };
        }
        else
        {
            vertex[0] = { corner[0], white, { texSup.x, texSup.y } };
            vertex[1] = { corner[1], white, { texInf.x, texSup.y } };
            vertex[2] = { corner[2], white, { texSup.x, texInf.y } };
            vertex[3] = { corner[3], white, { texInf.x, texInf.y } };
        }

        renderer->DrawParticle(PrimitiveType::TRIANGLE_STRIP, 4, vertex);
        m_engine->AddStatisticTriangle(2);

        p1 = p2;
    }
}

CObject* CLightning::SearchObject(glm::vec3 pos)
{
    // Lightning conductors
    std::vector<CObject*> paraObj;
    paraObj.reserve(100);
    std::vector<glm::vec3> paraObjPos;
    paraObjPos.reserve(100);

    // Seeking the object closest to the point of impact of lightning.
    CObject* bestObj = nullptr;
    float min = 100000.0f;
    for (CObject* obj : CObjectManager::GetInstancePointer()->GetAllObjects())
    {
        if (!obj->GetDetectable()) continue;  // inactive object?

        if (IsObjectBeingTransported(obj)) continue;

        ObjectType type = obj->GetType();
        if ( type == OBJECT_BASE ||
             type == OBJECT_PARA )  // building a lightning effect?
        {
            paraObj.push_back(obj);
            paraObjPos.push_back(obj->GetPosition());
            continue;
        }

        if (!obj->Implements(ObjectInterfaceType::Destroyable)) continue;

        float detect = m_magnetic * dynamic_cast<CDestroyableObject&>(*obj).GetLightningHitProbability();
        if (detect == 0.0f) continue;

        glm::vec3 oPos = obj->GetPosition();
        float dist = Math::DistanceProjected(oPos, pos);
        if (dist > detect) continue;
        if (dist < min)
        {
            min = dist;
            bestObj = obj;
        }
    }

    if (bestObj == nullptr)
        return nullptr;  // nothing found

    // Under the protection of a lightning conductor?
    glm::vec3 oPos = bestObj->GetPosition();
    for (int i = paraObj.size()-1; i >= 0; i--)
    {
        float dist = Math::DistanceProjected(oPos, paraObjPos[i]);
        if (dist <= LTNG_PROTECTION_RADIUS)
            return paraObj[i];
    }

    return bestObj;
}


void CLightning::StrikeAtPos(glm::vec3 pos)
{
    m_pos = pos;

    glm::vec3 eye = m_engine->GetEyePt();
    float dist = glm::distance(m_pos, eye);
    float deep = m_engine->GetDeepView();

    if (dist < deep)
    {
        glm::vec3 position = eye+((m_pos-eye)*0.2f);  // like so close!
        m_sound->Play(SOUND_BLITZ, position);

        m_camera->StartOver(CAM_OVER_EFFECT_LIGHTNING, m_pos, 1.0f);

        m_phase    = LightningPhase::Flash;
        m_progress = 0.0f;
        m_speed    = 1.0f;
    }
}

} // namespace Gfx
