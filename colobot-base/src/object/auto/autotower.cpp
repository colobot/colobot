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


#include "object/auto/autotower.h"

#include "graphics/engine/engine.h"

#include "level/robotmain.h"

#include "level/parser/parserline.h"
#include "level/parser/parserparam.h"

#include "math/geometry.h"

#include "object/object_manager.h"
#include "object/old_object.h"

#include "object/interface/slotted_object.h"

#include "physics/physics.h"

#include "sound/sound.h"

#include "ui/controls/gauge.h"
#include "ui/controls/interface.h"
#include "ui/controls/window.h"


const float TOWER_SCOPE     = 200.0f;   // range of beam
const float ENERGY_FIRE     = 0.125f;   // energy consumed by fire


// Object's constructor.

CAutoTower::CAutoTower(COldObject* object) : CAuto(object)
{
    for (int i = 0; i < 4; i++)
    {
        m_partiStop[i] = -1;
    }

    Init();
    m_phase = ATP_WAIT;  // paused until the first Init ()
    m_time = 0.0f;
    m_lastUpdateTime = 0.0f;

    assert(m_object->GetNumSlots() == 1);
}

// Object's destructor.

CAutoTower::~CAutoTower()
{
}


// Destroys the object.

void CAutoTower::DeleteObject(bool bAll)
{
    FireStopUpdate(0.0f, false);
    CAuto::DeleteObject(bAll);
}


// Initialize the object.

void CAutoTower::Init()
{
    m_phase    = ATP_ZERO;
    m_progress = 0.0f;
    m_speed    = 1.0f/1.0f;

    m_time = 0.0f;
    m_timeVirus = 0.0f;
    m_lastUpdateTime = 0.0f;
    m_lastParticle = 0.0f;
}


// Management of an event.

bool CAutoTower::EventProcess(const Event &event)
{
    CObject*    target;
    glm::vec3    pos;
    float       angle, quick;

    CAuto::EventProcess(event);

    if ( m_engine->GetPause() )  return true;
    if ( event.type != EVENT_FRAME )  return true;

    m_timeVirus -= event.rTime;

    if ( m_object->GetVirusMode() )  // contaminated by a virus?
    {
        if ( m_timeVirus <= 0.0f )
        {
            m_timeVirus = 0.1f+Math::Rand()*0.3f;

            angle = m_object->GetPartRotationY(1);
            angle += Math::Rand()*0.5f;
            m_object->SetPartRotationY(1, angle);

            m_object->SetPartRotationZ(2, Math::Rand()*0.5f);
        }
        return true;
    }

    CPowerContainerObject* power = GetObjectPowerCell(m_object);
    float energy = power == nullptr ? 0.0f : power->GetEnergy();

    UpdateInterface(event.rTime);

    if ( m_phase == ATP_WAIT )  return true;

    m_progress += event.rTime*m_speed;

    if ( m_phase == ATP_ZERO )
    {
        FireStopUpdate(m_progress, true);  // blinks
        if ( m_progress < 1.0f )
        {
            if ( energy >= ENERGY_FIRE )
            {
                m_phase    = ATP_SEARCH;
                m_progress = 0.0f;
                m_speed    = 1.0f/3.0f;
            }
        }
        else
        {
            m_phase    = ATP_ZERO;
            m_progress = 0.0f;
            m_speed    = 1.0f/1.0f;
        }
    }

    if ( m_phase == ATP_SEARCH )
    {
        FireStopUpdate(m_progress, false);  // extinguished
        if ( m_progress < 1.0f )
        {
            quick = 1.0f;
//?         if ( g_researchDone & RESEARCH_QUICK )  quick = 3.0f;

            angle = m_object->GetPartRotationY(1);
            angle -= event.rTime*quick*2.0f;
            m_object->SetPartRotationY(1, angle);

            angle = m_object->GetPartRotationZ(2);
            angle += event.rTime*quick*0.5f;
            if ( angle > 0.0f )  angle = 0.0f;
            m_object->SetPartRotationZ(2, angle);
        }
        else
        {
            target = SearchTarget(m_targetPos);
            if ( energy < ENERGY_FIRE )
            {
                m_main->DisplayError(ERR_TOWER_ENERGY, m_object);
            }
            if ( target == nullptr || energy < ENERGY_FIRE )
            {
                m_phase    = ATP_ZERO;
                m_progress = 0.0f;
                m_speed    = 1.0f/1.0f;
            }
            else
            {
                pos = m_object->GetPosition();
                pos.y += 24.5f;
                m_angleYfinal = Math::RotateAngle(m_targetPos.x-pos.x, pos.z-m_targetPos.z);  // CW !
                m_angleYfinal += Math::PI*2.0f;
                m_angleYfinal -= m_object->GetRotationY();
                m_angleYactual = Math::NormAngle(m_object->GetPartRotationY(1));

                m_angleZfinal = -Math::PI/2.0f;
                m_angleZfinal -= Math::RotateAngle(Math::DistanceProjected(m_targetPos, pos), pos.y-m_targetPos.y);  // CW !
                m_angleZactual = m_object->GetPartRotationZ(2);

                m_phase    = ATP_TURN;
                m_progress = 0.0f;
                m_speed    = 1.0f/1.0f;
//?             if ( g_researchDone & RESEARCH_QUICK )  m_speed = 1.0f/0.2f;
            }
        }
    }

    if ( m_phase == ATP_TURN )
    {
        if ( m_progress < 1.0f )
        {
            angle = m_angleYactual+(m_angleYfinal-m_angleYactual)*m_progress;
            m_object->SetPartRotationY(1, angle);

            angle = m_angleZactual+(m_angleZfinal-m_angleZactual)*m_progress;
            m_object->SetPartRotationZ(2, angle);
        }
        else
        {
            if ( energy < ENERGY_FIRE )
            {
                m_phase    = ATP_ZERO;
                m_progress = 0.0f;
                m_speed    = 1.0f/1.0f;
            }
            else
            {
                m_object->SetPartRotationY(1, m_angleYfinal);
                m_object->SetPartRotationZ(2, m_angleZfinal);
                energy -= ENERGY_FIRE;
                power->SetEnergy(energy);
                m_sound->Play(SOUND_GGG, m_object->GetPosition());
                m_phase    = ATP_FIRE;
                m_progress = 0.0f;
                m_speed    = 1.0f/1.5f;
            }
        }
    }

    if ( m_phase == ATP_FIRE )
    {
        if ( m_progress == 0.0f )
        {
            pos = m_object->GetPosition();
            pos.y += 24.5f;
            m_particle->CreateRay(pos, m_targetPos, Gfx::PARTIRAY1,
                { 5.0f, 5.0f }, 1.5f);
        }
        if ( m_progress >= 1.0f )
        {
            m_phase    = ATP_ZERO;
            m_progress = 0.0f;
            m_speed    = 1.0f/1.0f;
        }
    }

    return true;
}


// Seeks the nearest target object.

CObject* CAutoTower::SearchTarget(glm::vec3 &impact)
{
    glm::vec3 iPos = m_object->GetPosition();
    float min = 1000000.0f;

    CObject* best = nullptr;
    for (CObject* obj : CObjectManager::GetInstancePointer()->GetAllObjects())
    {
        int oTeam=obj->GetTeam();
        int myTeam=m_object->GetTeam();
        ObjectType oType = obj->GetType();
        if ( oType != OBJECT_MOTHER &&
             oType != OBJECT_ANT    &&
             oType != OBJECT_SPIDER &&
             oType != OBJECT_BEE    &&
             oType != OBJECT_WORM   &&
             (oTeam == myTeam        ||
             oTeam == 0)             )  continue;

        if ( !obj->GetDetectable() )  continue;  // inactive?

//?     if ( g_researchDone & RESEARCH_QUICK )
        if ( false )
        {
            if ( obj->Implements(ObjectInterfaceType::Movable) )
            {
                CPhysics* physics = dynamic_cast<CMovableObject&>(*obj).GetPhysics();
                float speed = fabs(physics->GetLinMotionX(MO_REASPEED));
                if ( speed > 20.0f )  continue;  // moving too fast?
            }
        }

        if (obj->GetCrashSphereCount() == 0) continue;

        glm::vec3 oPos = obj->GetFirstCrashSphere().sphere.pos;
        float distance = glm::distance(oPos, iPos);
        if ( distance > TOWER_SCOPE )  continue;  // too far
        if ( distance < min )
        {
            min = distance; best = obj;
        }
    }
    if ( best == nullptr )  return nullptr;

    impact = best->GetPosition();
    return best;
}


// Returns an error due the state of the automation.

Error CAutoTower::GetError()
{
    if ( m_object->GetVirusMode() )
    {
        return ERR_BAT_VIRUS;
    }

    CPowerContainerObject *power = GetObjectPowerCell(m_object);
    if ( power == nullptr )
    {
        return ERR_TOWER_POWER;  // no battery
    }

    if ( power->GetEnergy() < ENERGY_FIRE )
    {
        return ERR_TOWER_ENERGY;  // not enough energy
    }

    return ERR_OK;
}


// Updates the stop lights.

void CAutoTower::FireStopUpdate(float progress, bool bLightOn)
{
    glm::vec3    pos, speed;
    glm::vec2     dim;
    int         i;

    static float listpos[8] =
    {
         4.5f,   0.0f,
         0.0f,   4.5f,
        -4.5f,   0.0f,
         0.0f,  -4.5f,
    };

    if ( !bLightOn )  // extinguished?
    {
        for ( i=0 ; i<4 ; i++ )
        {
            if ( m_partiStop[i] != -1 )
            {
                m_particle->DeleteParticle(m_partiStop[i]);
                m_partiStop[i] = -1;
            }
        }
        return;
    }

    glm::mat4 mat = m_object->GetWorldMatrix(0);

    speed = glm::vec3(0.0f, 0.0f, 0.0f);
    dim.x = 2.0f;
    dim.y = dim.x;

    for ( i=0 ; i<4 ; i++ )
    {
        if ( Math::Mod(progress+i*0.125f, 0.5f) < 0.2f )
        {
            if ( m_partiStop[i] != -1 )
            {
                m_particle->DeleteParticle(m_partiStop[i]);
                m_partiStop[i] = -1;
            }
        }
        else
        {
            if ( m_partiStop[i] == -1 )
            {
                pos.x = listpos[i*2+0];
                pos.y = 18.0f;
                pos.z = listpos[i*2+1];
                pos = Math::Transform(mat, pos);

                m_partiStop[i] = m_particle->CreateParticle(pos, speed,
                                                              dim, Gfx::PARTISELR,
                                                              1.0f, 0.0f, 0.0f);
            }
        }
    }
}


// Creates all the interface when the object is selected.

bool CAutoTower::CreateInterface(bool bSelect)
{
    Ui::CWindow*    pw;
    glm::vec2     pos, ddim;
    float       ox, oy, sx, sy;

    CAuto::CreateInterface(bSelect);

    if ( !bSelect )  return true;

    pw = static_cast< Ui::CWindow* >(m_interface->SearchControl(EVENT_WINDOW0));
    if ( pw == nullptr )  return false;

    ox = 3.0f/640.0f;
    oy = 3.0f/480.0f;
    sx = 33.0f/640.0f;
    sy = 33.0f/480.0f;

    pos.x = ox+sx*14.5f;
    pos.y = oy+sy*0;
    ddim.x = 14.0f/640.0f;
    ddim.y = 66.0f/480.0f;
    pw->CreateGauge(pos, ddim, 0, EVENT_OBJECT_GENERGY);

    pos.x = ox+sx*0.0f;
    pos.y = oy+sy*0;
    ddim.x = 66.0f/640.0f;
    ddim.y = 66.0f/480.0f;
    pw->CreateGroup(pos, ddim, 107, EVENT_OBJECT_TYPE);

    pos.x = ox+sx*10.2f;
    pos.y = oy+sy*0.5f;
    ddim.x = 33.0f/640.0f;
    ddim.y = 33.0f/480.0f;
    pw->CreateButton(pos, ddim, 41, EVENT_OBJECT_LIMIT);

    return true;
}

// Updates the state of all buttons on the interface,
// following the time that elapses ...

void CAutoTower::UpdateInterface(float rTime)
{
    CAuto::UpdateInterface(rTime);

    if ( m_time < m_lastUpdateTime+0.1f )  return;
    m_lastUpdateTime = m_time;

    if ( !m_object->GetSelect() )  return;

    Ui::CWindow* pw = static_cast< Ui::CWindow* >(m_interface->SearchControl(EVENT_WINDOW0));
    if ( pw == nullptr )  return;

    Ui::CGauge* pg = static_cast< Ui::CGauge* >(pw->SearchControl(EVENT_OBJECT_GENERGY));
    if ( pg != nullptr )
    {
        float energy = GetObjectEnergyLevel(m_object);
        pg->SetLevel(energy);
    }
}


// Saves all parameters of the controller.

bool CAutoTower::Write(CLevelParserLine* line)
{
    if ( m_phase == ATP_WAIT )  return false;

    line->AddParam("aExist", std::make_unique<CLevelParserParam>(true));
    CAuto::Write(line);
    line->AddParam("aPhase", std::make_unique<CLevelParserParam>(static_cast<int>(m_phase)));
    line->AddParam("aProgress", std::make_unique<CLevelParserParam>(m_progress));
    line->AddParam("aSpeed", std::make_unique<CLevelParserParam>(m_speed));
    line->AddParam("aTargetPos", std::make_unique<CLevelParserParam>(m_targetPos));
    line->AddParam("aAngleYactual", std::make_unique<CLevelParserParam>(m_angleYactual));
    line->AddParam("aAngleZactual", std::make_unique<CLevelParserParam>(m_angleZactual));
    line->AddParam("aAngleYfinal", std::make_unique<CLevelParserParam>(m_angleYfinal));
    line->AddParam("aAngleZfinal", std::make_unique<CLevelParserParam>(m_angleZfinal));

    return true;
}

// Restores all parameters of the controller.

bool CAutoTower::Read(CLevelParserLine* line)
{
    if ( !line->GetParam("aExist")->AsBool(false) )  return false;

    CAuto::Read(line);
    m_phase = static_cast< AutoTowerPhase >(line->GetParam("aPhase")->AsInt(ATP_WAIT));
    m_progress = line->GetParam("aProgress")->AsFloat(0.0f);
    m_speed = line->GetParam("aSpeed")->AsFloat(1.0f);
    m_targetPos = line->GetParam("aTargetPos")->AsPoint(glm::vec3(0.0f, 0.0f, 0.0f));
    m_angleYactual = line->GetParam("aAngleYactual")->AsFloat(0.0f);
    m_angleZactual = line->GetParam("aAngleZactual")->AsFloat(0.0f);
    m_angleYfinal = line->GetParam("aAngleYfinal")->AsFloat(0.0f);
    m_angleZfinal = line->GetParam("aAngleZfinal")->AsFloat(0.0f);

    m_lastUpdateTime = 0.0f;

    return true;
}
