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


#include "graphics/engine/lightning.h"

#include "app/app.h"

#include "common/logger.h"
#include "common/iman.h"

#include "graphics/core/device.h"
#include "graphics/engine/camera.h"
#include "graphics/engine/terrain.h"

#include "math/geometry.h"

#include "object/object.h"
#include "object/robotmain.h"

#include "object/auto/autopara.h"


// Graphics module namespace
namespace Gfx {


CLightning::CLightning(CEngine* engine)
{
    m_engine = engine;
    m_terrain = nullptr;
    m_camera = nullptr;
    m_sound = nullptr;

    Flush();
}

CLightning::~CLightning()
{
}

void CLightning::Flush()
{
    m_lightningExists = false;
    m_phase = LP_WAIT;
    m_speed = 0.0f;
    m_progress = 0.0f;

    for (int i = 0; i < FLASH_SEGMENTS; i++)
    {
        m_shift[i] = Math::Point(0.0f, 0.0f);
        m_width[i] = 1.0f;
    }
}

bool CLightning::EventProcess(const Event &event)
{
    if (event.type == EVENT_FRAME)
        return EventFrame(event);

    return true;
}

bool CLightning::EventFrame(const Event &event)
{
    if (m_engine->GetPause()) return true;
    if (m_engine->GetMovieLock()) return true;

    m_progress += event.rTime*m_speed;

    if (m_phase == LP_WAIT)
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
                m_pos = obj->GetPosition(0);
                m_terrain->AdjustToFloor(m_pos, true);

                ObjectType type = obj->GetType();
                if (type == OBJECT_BASE)
                {
                    m_pos.y += 120.0f;  // top of the rocket
                }
                else if (type == OBJECT_PARA)
                {
                    CAutoPara* automat = static_cast<CAutoPara*>(obj->GetAuto());
                    if (automat != nullptr)
                        automat->StartLightning();

                    m_pos.y += 67.0f;  // top of lightning rod
                }
                else
                {
                    obj->ExploObject(EXPLO_BOUM, 1.0f);
                }
            }

            Math::Vector eye = m_engine->GetEyePt();
            float dist = Math::Distance(m_pos, eye);
            float deep = m_engine->GetDeepView();

            if (dist < deep)
            {
                Math::Vector pos = eye+((m_pos-eye)*0.2f);  // like so close!
                m_sound->Play(SOUND_BLITZ, pos);

                m_camera->StartOver(CAM_OVER_EFFECT_LIGHTNING, m_pos, 1.0f);

                m_phase    = LP_FLASH;
                m_progress = 0.0f;
                m_speed    = 1.0f;
            }
        }
    }

    if (m_phase == LP_FLASH)
    {
        if (m_progress < 1.0f)
        {
            float max = 5.0f;
            for (int i = 0; i < FLASH_SEGMENTS; i++)
            {
                max += 0.4f;

                m_shift[i].x += (Math::Rand()-0.5f)*max*2.0f;
                if ( m_shift[i].x < -max )  m_shift[i].x = -max;
                if ( m_shift[i].x >  max )  m_shift[i].x =  max;

                m_shift[i].y += (Math::Rand()-0.5f)*max*2.0f;
                if ( m_shift[i].y < -max )  m_shift[i].y = -max;
                if ( m_shift[i].y >  max )  m_shift[i].y =  max;

                m_width[i] += (Math::Rand()-0.5f)*2.0f;
                if ( m_width[i] < 1.0f )  m_width[i] = 1.0f;
                if ( m_width[i] > 6.0f )  m_width[i] = 6.0f;
            }
            m_shift[0].x = 0.0f;
            m_shift[0].y = 0.0f;
            m_width[0]   = 0.0f;
        }
        else
        {
            m_phase    = LP_WAIT;
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

    m_phase    = LP_WAIT;
    m_progress = 0.0f;
    m_speed    = 1.0f / m_sleep;

    if (m_terrain == nullptr)
        m_terrain = CRobotMain::GetInstancePointer()->GetTerrain();

    if (m_camera == nullptr)
        m_camera = CRobotMain::GetInstancePointer()->GetCamera();

    if (m_sound == nullptr)
        m_sound = CApplication::GetInstancePointer()->GetSound();

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
    m_phase = LP_WAIT;
    m_speed = 1.0f/m_sleep;

    return true;
}

void CLightning::Draw()
{
    if (!m_lightningExists) return;
    if (m_phase != LP_FLASH) return;

    CDevice* device = m_engine->GetDevice();

    Math::Matrix mat;
    mat.LoadIdentity();
    device->SetTransform(TRANSFORM_WORLD, mat);

    m_engine->SetTexture("effect00.png");
    m_engine->SetState(ENG_RSTATE_TTEXTURE_BLACK);

    Math::Point texInf;
    texInf.x = 64.5f/256.0f;
    texInf.y = 33.0f/256.0f;
    Math::Point texSup;
    texSup.x = 95.5f/256.0f;
    texSup.y = 34.0f/256.0f;  // blank

    Math::Vector p1 = m_pos;
    Math::Vector eye = m_engine->GetEyePt();
    float a = Math::RotateAngle(eye.x-p1.x, eye.z-p1.z);
    Math::Vector n = Math::Normalize(p1-eye);

    Math::Vector corner[4];
    Vertex vertex[4];

    for (int i = 0; i < FLASH_SEGMENTS-1; i++)
    {
        Math::Vector p2 = p1;
        p2.y += 8.0f+0.2f*i;

        Math::Point rot;

        Math::Vector p = p1;
        p.x += m_width[i];
        rot = Math::RotatePoint(Math::Point(p1.x, p1.z), a+Math::PI/2.0f, Math::Point(p.x, p.z));
        corner[0].x = rot.x+m_shift[i].x;
        corner[0].y = p1.y;
        corner[0].z = rot.y+m_shift[i].y;
        rot = Math::RotatePoint(Math::Point(p1.x, p1.z), a-Math::PI/2.0f, Math::Point(p.x, p.z));
        corner[1].x = rot.x+m_shift[i].x;
        corner[1].y = p1.y;
        corner[1].z = rot.y+m_shift[i].y;

        p = p2;
        p.x += m_width[i+1];
        rot = Math::RotatePoint(Math::Point(p2.x, p2.z), a+Math::PI/2.0f, Math::Point(p.x, p.z));
        corner[2].x = rot.x+m_shift[i+1].x;
        corner[2].y = p2.y;
        corner[2].z = rot.y+m_shift[i+1].y;
        rot = Math::RotatePoint(Math::Point(p2.x, p2.z), a-Math::PI/2.0f, Math::Point(p.x, p.z));
        corner[3].x = rot.x+m_shift[i+1].x;
        corner[3].y = p2.y;
        corner[3].z = rot.y+m_shift[i+1].y;

        if (p2.y < p1.y)
        {
            vertex[0] = Vertex(corner[1], n, Math::Point(texSup.x, texSup.y));
            vertex[1] = Vertex(corner[0], n, Math::Point(texInf.x, texSup.y));
            vertex[2] = Vertex(corner[3], n, Math::Point(texSup.x, texInf.y));
            vertex[3] = Vertex(corner[2], n, Math::Point(texInf.x, texInf.y));
        }
        else
        {
            vertex[0] = Vertex(corner[0], n, Math::Point(texSup.x, texSup.y));
            vertex[1] = Vertex(corner[1], n, Math::Point(texInf.x, texSup.y));
            vertex[2] = Vertex(corner[2], n, Math::Point(texSup.x, texInf.y));
            vertex[3] = Vertex(corner[3], n, Math::Point(texInf.x, texInf.y));
        }

        device->DrawPrimitive(PRIMITIVE_TRIANGLE_STRIP, vertex, 4);
        m_engine->AddStatisticTriangle(2);

        p1 = p2;
    }
}

CObject* CLightning::SearchObject(Math::Vector pos)
{
    // Lightning conductors
    std::vector<CObject*> paraObj;
    paraObj.reserve(100);
    std::vector<Math::Vector> paraObjPos;
    paraObjPos.reserve(100);

    CInstanceManager* iMan = CInstanceManager::GetInstancePointer();

    // Seeking the object closest to the point of impact of lightning.
    CObject* bestObj = 0;
    float min = 100000.0f;
    for (int i = 0; i < 1000000; i++)
    {
        CObject* obj = static_cast<CObject*>( iMan->SearchInstance(CLASS_OBJECT, i) );
        if (obj == nullptr) break;

        if (!obj->GetActif()) continue;  // inactive object?
        if (obj->GetTruck() != nullptr) continue;  // object transported?

        ObjectType type = obj->GetType();
        if ( type == OBJECT_BASE ||
             type == OBJECT_PARA )  // building a lightning effect?
        {
            paraObj.push_back(obj);
            paraObjPos.push_back(obj->GetPosition(0));
        }

        float detect = 0.0f;
        if ( type == OBJECT_BASE     ||
             type == OBJECT_DERRICK  ||
             type == OBJECT_FACTORY  ||
             type == OBJECT_REPAIR   ||
             type == OBJECT_DESTROYER||
             type == OBJECT_STATION  ||
             type == OBJECT_CONVERT  ||
             type == OBJECT_TOWER    ||
             type == OBJECT_RESEARCH ||
             type == OBJECT_RADAR    ||
             type == OBJECT_INFO     ||
             type == OBJECT_ENERGY   ||
             type == OBJECT_LABO     ||
             type == OBJECT_NUCLEAR  ||
             type == OBJECT_PARA     ||
             type == OBJECT_SAFE     ||
             type == OBJECT_HUSTON   )
        {
            detect = m_magnetic;
        }
        if ( type == OBJECT_METAL    ||
             type == OBJECT_POWER    ||
             type == OBJECT_ATOMIC   )
        {
            detect = m_magnetic*0.3f;
        }
        if ( type == OBJECT_MOBILEfa ||
             type == OBJECT_MOBILEta ||
             type == OBJECT_MOBILEwa ||
             type == OBJECT_MOBILEia ||
             type == OBJECT_MOBILEfc ||
             type == OBJECT_MOBILEtc ||
             type == OBJECT_MOBILEwc ||
             type == OBJECT_MOBILEic ||
             type == OBJECT_MOBILEfi ||
             type == OBJECT_MOBILEti ||
             type == OBJECT_MOBILEwi ||
             type == OBJECT_MOBILEii ||
             type == OBJECT_MOBILEfs ||
             type == OBJECT_MOBILEts ||
             type == OBJECT_MOBILEws ||
             type == OBJECT_MOBILEis ||
             type == OBJECT_MOBILErt ||
             type == OBJECT_MOBILErc ||
             type == OBJECT_MOBILErr ||
             type == OBJECT_MOBILErs ||
             type == OBJECT_MOBILEsa ||
             type == OBJECT_MOBILEft ||
             type == OBJECT_MOBILEtt ||
             type == OBJECT_MOBILEwt ||
             type == OBJECT_MOBILEit ||
             type == OBJECT_MOBILEdr )
        {
            detect = m_magnetic*0.5f;
        }
        if (detect == 0.0f) continue;

        Math::Vector oPos = obj->GetPosition(0);
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
    Math::Vector oPos = bestObj->GetPosition(0);
    for (int i = paraObj.size()-1; i >= 0; i--)
    {
        float dist = Math::DistanceProjected(oPos, paraObjPos[i]);
        if (dist <= LTNG_PROTECTION_RADIUS)
            return paraObj[i];
    }

    return bestObj;
}


} // namespace Gfx

