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


#include "object/task/taskgoto.h"

#include "common/event.h"
#include "common/global.h"
#include "common/image.h"

#include "graphics/engine/engine.h"
#include "graphics/engine/terrain.h"
#include "graphics/engine/water.h"

#include "math/geometry.h"

#include "object/object_manager.h"
#include "object/old_object.h"

#include "object/interface/slotted_object.h"
#include "object/interface/transportable_object.h"

#include "object/subclass/base_alien.h"

#include "physics/physics.h"

#include <string.h>


const float FLY_DIST_GROUND = 80.0f;    // minimum distance to remain on the ground
const float FLY_DEF_HEIGHT  = 50.0f;    // default flying height

// Settings that define goto() accuracy:
const float BM_DIM_STEP     = 5.0f;     // Size of one pixel on the bitmap. Setting 5 means that 5x5 square (in game units) will be represented by 1 px on the bitmap. Decreasing this value will make a bigger bitmap, and may increase accuracy. TODO: Check how it actually impacts goto() accuracy
const float SAFETY_MARGIN   = 1.5f;     // Smallest distance between two objects. Smaller = less "no route to destination", but higher probability of collisions between objects.
// Changing SAFETY_MARGIN (old value was 4.0f) seems to have fixed many issues with goto(). TODO: maybe we could make it even smaller? Did changing it introduce any new bugs?

const int NB_ITER = 200;  // Maximum number of iterations you have the right to make before temporarily interrupt in order not to lower the framerate.



// Object's constructor.

CTaskGoto::CTaskGoto(COldObject* object) : CForegroundTask(object)
{
    m_bmArray = nullptr;
}

// Object's destructor.

CTaskGoto::~CTaskGoto()
{
    BitmapClose();

    if (m_engine->GetDebugGoto() && m_object->GetSelect())
        m_engine->SetDebugGotoBitmap(std::move(nullptr));
}


// Management of an event.

bool CTaskGoto::EventProcess(const Event &event)
{
    glm::vec3    pos, goal;
    glm::vec2       rot, repulse;
    float           a, g, dist, linSpeed, cirSpeed, h, hh, factor, dir;
    Error           ret;

    if ( event.type != EVENT_FRAME )  return true;

    if (m_engine->GetDebugGoto())
    {
        auto AdjustPoint = [&](glm::vec3 p) -> glm::vec3
        {
            m_terrain->AdjustToFloor(p);
            p.y += 2.0f;
            return p;
        };

        std::vector<Gfx::Vertex3D> debugLine;
        if (m_bmTotal > 0)
        {
            Gfx::Color color = Gfx::Color(0.0f, 1.0f, 0.0f);
            for (int i = 0; i < m_bmTotal; i++)
            {
                if (i > m_bmIndex-1)
                    color = Gfx::Color(1.0f, 0.0f, 0.0f);

                auto intcolor = Gfx::ColorToIntColor(color);

                debugLine.push_back({ AdjustPoint(m_bmPoints[i]), {}, intcolor });
            }
            m_engine->AddDebugGotoLine(debugLine);
            debugLine.clear();
        }
        Gfx::Color color = Gfx::Color(0.0f, 0.0f, 1.0f);
        auto pos = AdjustPoint(m_bmTotal > 0 && m_bmIndex <= m_bmTotal && m_phase != TGP_BEAMSEARCH ? m_bmPoints[m_bmIndex] : m_goal);
        debugLine.push_back({ m_object->GetPosition(), {}, color });
        debugLine.push_back({ pos, {}, color });
        m_engine->AddDebugGotoLine(debugLine);

        if (m_object->GetSelect() && m_bmChanged)
        {
            if (m_bmArray != nullptr)
            {
                std::unique_ptr<CImage> debugImage = std::make_unique<CImage>(glm::ivec2(m_bmSize, m_bmSize));
                debugImage->Fill(Gfx::IntColor(255, 255, 255, 255));
                for (int x = 0; x < m_bmSize; x++)
                {
                    for (int y = 0; y < m_bmSize; y++)
                    {
                        bool a = BitmapTestDot(0, x, y);
                        bool b = BitmapTestDot(1, x, y);
                        if (a || b)
                        {
                            Gfx::Color c = Gfx::Color(0.0f, 0.0f, 0.0f, 1.0f);
                            if (b) c = Gfx::Color(0.0f, 1.0f, 1.0f, 1.0f);
                            debugImage->SetPixel({ x, y }, c);
                        }
                    }
                }
                m_engine->SetDebugGotoBitmap(std::move(debugImage));
            }
            m_bmChanged = false;
        }
    }

    if ( m_engine->GetPause() )  return true;

    // Momentarily stationary object (ant on the back)?
    CBaseAlien* alien = dynamic_cast<CBaseAlien*>(m_object);
    if ( alien != nullptr && alien->GetFixed() )
    {
        m_physics->SetMotorSpeedX(0.0f);  // stops the advance
        m_physics->SetMotorSpeedZ(0.0f);  // stops the rotation
        return true;
    }

    if ( m_error != ERR_OK )  return false;

    if ( m_bWorm )
    {
        WormFrame(event.rTime);
    }

    if ( m_phase == TGP_BEAMLEAK )  // leak?
    {
        m_leakTime += event.rTime;

        pos = m_object->GetPosition();

        rot.x = m_leakPos.x-pos.x;
        rot.y = m_leakPos.z-pos.z;
        dist = glm::length(glm::vec2(rot.x, rot.y));
        if (dist != 0)
        {
            rot.x /= dist;
            rot.y /= dist;
        }

        a = m_object->GetRotationY();
        g = Math::RotateAngle(rot.x, -rot.y);  // CW !
        a = Math::Direction(a, g)*1.0f;
        cirSpeed = a;
        if ( cirSpeed >  1.0f )  cirSpeed =  1.0f;
        if ( cirSpeed < -1.0f )  cirSpeed = -1.0f;

        a = Math::NormAngle(a);
        if ( a > Math::PI*0.5f && a < Math::PI*1.5f )
        {
            linSpeed = 1.0f;  // obstacle behind -> advance
            cirSpeed = -cirSpeed;
        }
        else
        {
            linSpeed = -1.0f;  // obstacle in front -> back
        }

        if ( m_bLeakRecede )
        {
            linSpeed = -1.0f;
            cirSpeed = 0.0f;
        }

        m_physics->SetMotorSpeedZ(cirSpeed);  // turns left / right
        m_physics->SetMotorSpeedX(linSpeed);  // advance
        return true;
    }

    if ( m_phase == TGP_BEAMSEARCH )  // search path?
    {
        if ( m_bmStep == 0 )
        {
            // Frees the area around the departure.
            BitmapClearCircle(m_object->GetPosition(), BM_DIM_STEP*1.8f);
        }

        pos = m_object->GetPosition();

        if ( m_bmCargoObject == nullptr )
        {
            goal = m_goal;
            dist = 0.0f;
        }
        else
        {
            goal = m_goalObject;
            dist = TAKE_DIST+2.0f;
            if ( m_bmCargoObject->GetType() == OBJECT_BASE )  dist = 12.0f;
        }

        ret = PathFindingSearch(pos, goal, dist);
        if ( ret == ERR_OK )
        {
            if ( m_physics->GetLand() )  m_phase = TGP_BEAMWCOLD;
            else                         m_phase = TGP_BEAMGOTO;
            m_bmIndex = 0;
            m_bmWatchDogPos = m_object->GetPosition();
            m_bmWatchDogTime = 0.0f;
        }
        if ( ret == ERR_GOTO_IMPOSSIBLE || ret == ERR_GOTO_ITER )
        {
            m_error = ret;
            return false;
        }
        return true;
    }

    if ( m_phase == TGP_BEAMWCOLD )  // expects cooled reactor?
    {
        return true;
    }

    if ( m_phase == TGP_BEAMUP )  // off?
    {
        m_physics->SetMotorSpeedY(1.0f);  // up
        return true;
    }

    if ( m_phase == TGP_BEAMGOTO )  // goto dot list? (?)
    {
        if ( m_physics->GetCollision() )  // collision?
        {
            m_physics->SetCollision(false);  // there's more
        }

        pos = m_object->GetPosition();

        if ( m_object->Implements(ObjectInterfaceType::Flying) && m_altitude == 0.0f )
        {
            if ( m_physics->GetLand() )
            {
                m_physics->SetMotorSpeedY(0.0f);
            }
            else
            {
                m_physics->SetMotorSpeedY(-1.0f);
            }
        }

        if ( m_object->Implements(ObjectInterfaceType::Flying) && m_altitude > 0.0f )
        {
            goal = m_bmPoints[m_bmIndex];
            goal.y = pos.y;
            h = m_terrain->GetHeightToFloor(goal, true, true);
            dist = Math::DistanceProjected(pos, goal);
            if ( dist != 0.0f )  // anticipates?
            {
                linSpeed = m_physics->GetLinMotionX(MO_REASPEED);
                linSpeed /= m_physics->GetLinMotionX(MO_ADVSPEED);
                goal.x = pos.x + (goal.x-pos.x)*linSpeed*20.0f/dist;
                goal.z = pos.z + (goal.z-pos.z)*linSpeed*20.0f/dist;
            }
            goal.y = pos.y;
            hh = m_terrain->GetHeightToFloor(goal, true, true);
            h = Math::Min(h, hh);
            linSpeed = 0.0f;
            if ( h < m_altitude-1.0f )
            {
                linSpeed = 0.2f+((m_altitude-1.0f)-h)*0.1f;  // up
                if ( linSpeed > 1.0f )  linSpeed = 1.0f;
            }
            if ( h > m_altitude+1.0f )
            {
                linSpeed = -0.2f;  // down
            }
            m_physics->SetMotorSpeedY(linSpeed);
        }

        rot.x = m_bmPoints[m_bmIndex].x-pos.x;
        rot.y = m_bmPoints[m_bmIndex].z-pos.z;
        dist = glm::length(glm::vec2(rot.x, rot.y));
        rot.x /= dist;
        rot.y /= dist;

        a = m_object->GetRotationY();
        g = Math::RotateAngle(rot.x, -rot.y);  // CW !
        cirSpeed = Math::Direction(a, g)*2.0f;
        if ( cirSpeed >  1.0f )  cirSpeed =  1.0f;
        if ( cirSpeed < -1.0f )  cirSpeed = -1.0f;
        if ( dist < 4.0f )  cirSpeed *= dist/4.0f;  // so close -> turns less

        if ( m_bmIndex == m_bmTotal )  // last point?
        {
            linSpeed = dist/(m_physics->GetLinStopLength()*1.5f);
            if ( linSpeed > 1.0f )  linSpeed = 1.0f;
        }
        else
        {
            linSpeed = 1.0f;  // dark without stopping
        }

        linSpeed *= 1.0f-(1.0f-0.3f)*fabs(cirSpeed);

        if ( fabs(cirSpeed) >= 0.2f )
        {
            linSpeed = 0.0f;  // turns first, then advance
        }

        dist = Math::DistanceProjected(pos, m_bmWatchDogPos);
        if ( dist < 1.0f && linSpeed != 0.0f )
        {
            m_bmWatchDogTime += event.rTime;
        }
        else
        {
            m_bmWatchDogTime = 0.0f;
            m_bmWatchDogPos = pos;
        }

        if ( m_bmWatchDogTime >= 1.0f )  // immobile for a long time?
        {
            m_physics->SetMotorSpeedX(0.0f);  // stops the advance
            m_physics->SetMotorSpeedZ(0.0f);  // stops the rotation
            PathFindingStart();  // we start all
            return true;
        }

        m_physics->SetMotorSpeedZ(cirSpeed);  // turns left / right
        m_physics->SetMotorSpeedX(linSpeed);  // advance
        return true;
    }

    if ( m_phase == TGP_BEAMDOWN )  // landed?
    {
        m_physics->SetMotorSpeedY(-0.5f);  // tomb
        return true;
    }

    if ( m_phase == TGP_LAND )  // landed?
    {
        m_physics->SetMotorSpeedY(-0.5f);  // tomb
        return true;
    }

    if ( m_goalMode == TGG_EXPRESS )
    {
        if ( m_crashMode == TGC_HALT )
        {
            if ( m_physics->GetCollision() )  // collision?
            {
                m_physics->SetCollision(false);  // there's more
                m_error = ERR_STOP;
                return true;
            }
        }

        pos = m_object->GetPosition();

        if ( m_altitude > 0.0f )
        {
            h = m_terrain->GetHeightToFloor(pos, true, true);
            linSpeed = 0.0f;
            if ( h < m_altitude )
            {
                linSpeed = 0.1f;  // up
            }
            if ( h > m_altitude )
            {
                linSpeed = -0.2f;  // down
            }
            m_physics->SetMotorSpeedY(linSpeed);
        }

        rot.x = m_goal.x-pos.x;
        rot.y = m_goal.z-pos.z;
        a = m_object->GetRotationY();
        g = Math::RotateAngle(rot.x, -rot.y);  // CW !
        cirSpeed = Math::Direction(a, g)*1.0f;
        if ( cirSpeed >  1.0f )  cirSpeed =  1.0f;
        if ( cirSpeed < -1.0f )  cirSpeed = -1.0f;

        m_physics->SetMotorSpeedZ(cirSpeed);  // turns left / right
        m_physics->SetMotorSpeedX(1.0f);  // advance
        return true;
    }

    if ( m_phase != TGP_TURN                 &&
         m_object->Implements(ObjectInterfaceType::Flying) &&
         m_altitude > 0.0f                   )
    {
        pos = m_object->GetPosition();
        dist = Math::DistanceProjected(m_goal, pos);
        factor = (dist-20.0f)/20.0f;
        if ( factor < 0.0f )  factor = 0.0f;
        if ( factor > 1.0f )  factor = 1.0f;

        h = m_terrain->GetHeightToFloor(m_object->GetPosition(), true, true);
        linSpeed = 0.0f;
        if ( h < (m_altitude-0.5f)*factor && factor == 1.0f )
        {
            linSpeed = 0.1f;  // up
        }
        if ( h > m_altitude*factor )
        {
            linSpeed = -0.2f;  // down
        }
        ComputeFlyingRepulse(dir);
        linSpeed += dir*0.2f;

        m_physics->SetMotorSpeedY(linSpeed);
    }

    if ( m_phase == TGP_ADVANCE )  // going towards the goal?
    {
        if ( m_physics->GetCollision() )  // collision?
        {
            m_physics->SetCollision(false);  // there's more
            m_time = 0.0f;
            m_phase = TGP_CRWAIT;
            return true;
        }

        pos = m_object->GetPosition();

        rot.x = m_goal.x-pos.x;
        rot.y = m_goal.z-pos.z;
        dist = glm::length(glm::vec2(rot.x, rot.y));
        rot.x /= dist;
        rot.y /= dist;

        ComputeRepulse(repulse);
        rot.x += repulse.x*2.0f;
        rot.y += repulse.y*2.0f;

        a = m_object->GetRotationY();
        g = Math::RotateAngle(rot.x, -rot.y);  // CW !
        cirSpeed = Math::Direction(a, g)*1.0f;
        if ( cirSpeed >  1.0f )  cirSpeed =  1.0f;
        if ( cirSpeed < -1.0f )  cirSpeed = -1.0f;

        dist = Math::DistanceProjected(m_goal, pos);
        linSpeed = dist/(m_physics->GetLinStopLength()*1.5f);
        if ( linSpeed > 1.0f )  linSpeed =  1.0f;

        linSpeed *= 1.0f-(1.0f-0.3f)*fabs(cirSpeed);

        if ( dist < 20.0f && fabs(cirSpeed) >= 0.5f )
        {
            linSpeed = 0.0f;  // turns first, then advance
        }

        m_physics->SetMotorSpeedZ(cirSpeed);  // turns left / right
        m_physics->SetMotorSpeedX(linSpeed);  // advance
    }

    if ( m_phase == TGP_TURN   ||  // turns to the object?
         m_phase == TGP_CRTURN ||  // turns after collision?
         m_phase == TGP_CLTURN )   // turns after collision?
    {
        a = m_object->GetRotationY();
        g = m_angle;
        cirSpeed = Math::Direction(a, g)*1.0f;
        if ( cirSpeed >  1.0f )  cirSpeed =  1.0f;
        if ( cirSpeed < -1.0f )  cirSpeed = -1.0f;

        m_physics->SetMotorSpeedZ(cirSpeed);  // turns left / right
    }

    if ( m_phase == TGP_CRWAIT ||  // waits after collision?
         m_phase == TGP_CLWAIT )   // waits after collision?
    {
        m_time += event.rTime;
        m_physics->SetMotorSpeedX(0.0f);  // stops the advance
        m_physics->SetMotorSpeedZ(0.0f);  // stops the rotation
    }

    if ( m_phase == TGP_CRADVANCE )  // advance after collision?
    {
        if ( m_physics->GetCollision() )  // collision?
        {
            m_physics->SetCollision(false);  // there's more
            m_time = 0.0f;
            m_phase = TGP_CLWAIT;
            return true;
        }
        m_physics->SetMotorSpeedX(0.5f);  // advance mollo
    }

    if ( m_phase == TGP_CLADVANCE )  // advance after collision?
    {
        if ( m_physics->GetCollision() )  // collision?
        {
            m_physics->SetCollision(false);  // there's more
            m_time = 0.0f;
            m_phase = TGP_CRWAIT;
            return true;
        }
        m_physics->SetMotorSpeedX(0.5f);  // advance mollo
    }

    if ( m_phase == TGP_MOVE )  // final advance?
    {
        m_bmTimeLimit -= event.rTime;
        m_physics->SetMotorSpeedX(1.0f);
    }

    return true;
}


// Sought a target for the worm.

CObject* CTaskGoto::WormSearch(glm::vec3 &impact)
{
    glm::vec3 iPos = m_object->GetPosition();
    float min = 1000000.0f;

    CObject* best = nullptr;
    for (CObject* obj : CObjectManager::GetInstancePointer()->GetAllObjects())
    {
        ObjectType oType = obj->GetType();
        if ( oType != OBJECT_MOBILEfa &&
             oType != OBJECT_MOBILEta &&
             oType != OBJECT_MOBILEwa &&
             oType != OBJECT_MOBILEia &&
             oType != OBJECT_MOBILEfb &&
             oType != OBJECT_MOBILEtb &&
             oType != OBJECT_MOBILEwb &&
             oType != OBJECT_MOBILEib &&
             oType != OBJECT_MOBILEfc &&
             oType != OBJECT_MOBILEtc &&
             oType != OBJECT_MOBILEwc &&
             oType != OBJECT_MOBILEic &&
             oType != OBJECT_MOBILEfi &&
             oType != OBJECT_MOBILEti &&
             oType != OBJECT_MOBILEwi &&
             oType != OBJECT_MOBILEii &&
             oType != OBJECT_MOBILEfs &&
             oType != OBJECT_MOBILEts &&
             oType != OBJECT_MOBILEws &&
             oType != OBJECT_MOBILEis &&
             oType != OBJECT_MOBILErt &&
             oType != OBJECT_MOBILErc &&
             oType != OBJECT_MOBILErr &&
             oType != OBJECT_MOBILErs &&
             oType != OBJECT_MOBILEsa &&
             oType != OBJECT_MOBILEtg &&
             oType != OBJECT_MOBILEft &&
             oType != OBJECT_MOBILEtt &&
             oType != OBJECT_MOBILEwt &&
             oType != OBJECT_MOBILEit &&
             oType != OBJECT_MOBILErp &&
             oType != OBJECT_MOBILEst &&
             oType != OBJECT_MOBILEdr &&
             oType != OBJECT_DERRICK  &&
             oType != OBJECT_STATION  &&
             oType != OBJECT_FACTORY  &&
             oType != OBJECT_REPAIR   &&
             oType != OBJECT_DESTROYER &&
             oType != OBJECT_CONVERT  &&
             oType != OBJECT_TOWER    &&
             oType != OBJECT_RESEARCH &&
             oType != OBJECT_RADAR    &&
             oType != OBJECT_INFO     &&
             oType != OBJECT_ENERGY   &&
             oType != OBJECT_LABO     &&
             oType != OBJECT_NUCLEAR  &&
             oType != OBJECT_PARA     &&
             oType != OBJECT_SAFE     &&
             oType != OBJECT_HUSTON   )  continue;

        if ( obj->GetVirusMode() )  continue;  // object infected?

        if (obj->GetCrashSphereCount() == 0) continue;

        glm::vec3 oPos = obj->GetFirstCrashSphere().sphere.pos;
        float distance = Math::DistanceProjected(oPos, iPos);
        if (distance < min)
        {
            min = distance;
            best = obj;
        }
    }
    if ( best == nullptr )  return nullptr;

    impact = best->GetPosition();
    return best;
}

// Contaminate objects near the worm.

void CTaskGoto::WormFrame(float rTime)
{
    CObject*    pObj;
    glm::vec3    impact, pos;
    float       dist;

    m_wormLastTime += rTime;

    if ( m_wormLastTime >= 0.5f )
    {
        m_wormLastTime = 0.0f;

        pObj = WormSearch(impact);
        if ( pObj != nullptr )
        {
            pos = m_object->GetPosition();
            dist = glm::distance(pos, impact);
            if ( dist <= 15.0f )
            {
                pObj->SetVirusMode(true);  // bam, infected!
            }
        }
    }
}



// Assigns the goal was achieved.
// "dist" is the distance that needs to go far to make a deposit or object.

Error CTaskGoto::Start(glm::vec3 goal, float altitude,
                       TaskGotoGoal goalMode, TaskGotoCrash crashMode)
{
    glm::vec3    pos;
    CObject*    target;
    ObjectType  type;
    float       dist;
    int         x, y;

    type = m_object->GetType();

    if ( goalMode == TGG_DEFAULT )
    {
        goalMode = TGG_STOP;
        if ( type == OBJECT_MOTHER ||
             type == OBJECT_ANT    ||
             type == OBJECT_SPIDER ||
             type == OBJECT_WORM   )
        {
            goalMode = TGG_EXPRESS;
        }
    }

    if ( crashMode == TGC_DEFAULT )
    {
//?     crashMode = TGC_RIGHTLEFT;
        crashMode = TGC_BEAM;
        if ( type == OBJECT_MOTHER ||
             type == OBJECT_ANT    ||
             type == OBJECT_SPIDER ||
             type == OBJECT_WORM   ||
             type == OBJECT_BEE    )
        {
            crashMode = TGC_HALT;
        }
    }

    m_altitude   = altitude;
    m_goalMode   = goalMode;
    m_crashMode  = crashMode;
    m_goalObject = goal;
    m_goal       = goal;

    m_bTake = false;
    m_phase = TGP_ADVANCE;
    m_error = ERR_OK;
    m_try = 0;
    m_bmCargoObject = nullptr;
    m_bmFinalMove = 0.0f;

    pos = m_object->GetPosition();
    dist = Math::DistanceProjected(pos, m_goal);
    if ( dist < 10.0f && m_crashMode == TGC_BEAM )
    {
        m_crashMode = TGC_RIGHTLEFT;
    }

    m_bWorm = false;
    if ( type == OBJECT_WORM )
    {
        m_bWorm = true;
        m_wormLastTime = 0.0f;
    }

    m_bApprox = false;
    if ( type == OBJECT_HUMAN    ||
         type == OBJECT_TECH     ||
         type == OBJECT_MOTHER   ||
         type == OBJECT_ANT      ||
         type == OBJECT_SPIDER   ||
         type == OBJECT_BEE      ||
         type == OBJECT_WORM     ||
         type == OBJECT_MOBILErt ||
         type == OBJECT_MOBILErc ||
         type == OBJECT_MOBILErr ||
         type == OBJECT_MOBILErs ||
         type == OBJECT_MOBILErp )
    {
        m_bApprox = true;
    }

    if ( !m_bApprox && m_crashMode != TGC_BEAM )
    {
        target = SearchTarget(goal, 1.0f);
        if ( target != nullptr )
        {
            m_goal = target->GetPosition();
            dist = 0.0f;
            if ( !AdjustBuilding(m_goal, 1.0f, dist) )
            {
                dist = 0.0f;
                AdjustTarget(target, m_goal, dist);
            }
            m_bTake = true;  // object was taken on arrival (final rotation)
        }
    }

    m_lastDistance = 1000.0f;
    m_physics->SetCollision(false);

    if ( m_crashMode == TGC_BEAM )  // with the algorithm of rays?
    {
        target = SearchTarget(goal, 1.0f);
        if ( target != nullptr )
        {
            m_goal = target->GetPosition();
            dist = 4.0f;
            if ( AdjustBuilding(m_goal, 1.0f, dist) )
            {
                m_bmFinalMove = dist;
            }
            else
            {
                dist = 4.0f;
                if ( AdjustTarget(target, m_goal, dist) )
                {
                    m_bmCargoObject = target;  // cargo on the ground
                }
                else
                {
                    m_bmFinalMove = dist;
                }
            }
            m_bTake = true;  // object was taken on arrival (final rotation)
        }

        if ( m_object->Implements(ObjectInterfaceType::Flying) && m_altitude == 0.0f )
        {
            pos = m_object->GetPosition();
            dist = Math::DistanceProjected(pos, m_goal);
            if ( dist > FLY_DIST_GROUND )  // over 20 meters?
            {
                m_altitude = FLY_DEF_HEIGHT;  // default altitude
            }
        }

        PathFindingStart();

        if ( m_bmCargoObject == nullptr )
        {
            x = static_cast<int>((m_goal.x+1600.0f)/BM_DIM_STEP);
            y = static_cast<int>((m_goal.z+1600.0f)/BM_DIM_STEP);
            if ( BitmapTestDot(0, x, y) )  // arrival occupied?
            {
                m_error = ERR_GOTO_BUSY;
                return m_error;
            }
        }
    }

    return ERR_OK;
}

// Indicates whether the action is finished.

Error CTaskGoto::IsEnded()
{
    glm::vec3    pos;
    float       limit, angle = 0.0f, h, level;
    volatile float dist; //fix for issue #844

    if ( m_engine->GetPause() )  return ERR_CONTINUE;
    if ( m_error != ERR_OK )  return m_error;

    pos = m_object->GetPosition();

    if ( m_phase == TGP_BEAMLEAK )  // leak?
    {
        if ( m_leakTime >= m_leakDelay )
        {
            m_physics->SetMotorSpeedX(0.0f);  // stops the advance
            m_physics->SetMotorSpeedZ(0.0f);  // stops the rotation
            PathFindingInit();
            m_phase = TGP_BEAMSEARCH;  // will seek the path
        }
        return ERR_CONTINUE;
    }

    if ( m_phase == TGP_BEAMSEARCH )  // search path?
    {
        return ERR_CONTINUE;
    }

    if ( m_phase == TGP_BEAMWCOLD )  // expects cool reactor?
    {
        if ( m_altitude != 0.0f &&
             (m_object->Implements(ObjectInterfaceType::JetFlying) && dynamic_cast<CJetFlyingObject*>(m_object)->GetReactorRange() < 1.0f) )  return ERR_CONTINUE;
        m_phase = TGP_BEAMUP;
    }

    if ( m_phase == TGP_BEAMUP )  // off?
    {
        if ( m_object->Implements(ObjectInterfaceType::Flying) && m_altitude > 0.0f )
        {
            level = m_terrain->GetFloorLevel(pos, true, true);
            h = level+m_altitude-20.0f;
            limit = m_terrain->GetFlyingMaxHeight();
            if ( h > limit )  h = limit;
            if ( pos.y < h-1.0f )  return ERR_CONTINUE;

            m_physics->SetMotorSpeedY(0.0f);  // stops the ascent
        }
        m_phase = TGP_BEAMGOTO;
    }

    if ( m_phase == TGP_BEAMGOTO )  // goto dot list ?
    {
        if ( m_altitude != 0.0f &&
             (m_object->Implements(ObjectInterfaceType::JetFlying) && dynamic_cast<CJetFlyingObject*>(m_object)->GetReactorRange() < 0.1f) )  // overheating?
        {
            m_physics->SetMotorSpeedX(0.0f);  // stops the advance
            m_physics->SetMotorSpeedZ(0.0f);  // stops the rotation
            m_physics->SetMotorSpeedY(-1.0f);  // tomb
            m_phase = TGP_BEAMWCOLD;
            return ERR_CONTINUE;
        }

        if ( m_physics->GetLand() )  // on the ground?
        {
            limit = 1.0f;
        }
        else    // in flight?
        {
            limit = 2.0f;
            if ( m_bmIndex < m_bmTotal )  limit *= 2.0f;  // intermediate point
        }
        if ( m_bApprox )  limit = 2.0f;

        if ( fabs(pos.x - m_bmPoints[m_bmIndex].x) < limit &&
             fabs(pos.z - m_bmPoints[m_bmIndex].z) < limit )
        {
            m_physics->SetMotorSpeedX(0.0f);  // stops the advance
            m_physics->SetMotorSpeedZ(0.0f);  // stops the rotation

            m_bmIndex = PathFindingShortcut();

            if ( m_bmIndex > m_bmTotal )
            {
                m_phase = TGP_BEAMDOWN;
            }
        }
    }

    if ( m_phase == TGP_BEAMDOWN )  // landed?
    {
        if ( m_object->Implements(ObjectInterfaceType::Flying) && m_altitude > 0.0f )
        {
            if ( !m_physics->GetLand() )  return ERR_CONTINUE;
            m_physics->SetMotorSpeedY(0.0f);  // stops the descent

            m_altitude = 0.0f;
            m_phase = TGP_BEAMGOTO;  // advance finely on the ground to finish
            m_bmIndex = m_bmTotal;
            return ERR_CONTINUE;
        }

        if ( m_bTake )
        {
            m_angle = Math::RotateAngle(m_goalObject.x-pos.x, pos.z-m_goalObject.z);
            m_phase = TGP_TURN;
        }
        else
        {
            m_physics->SetMotorSpeedX(0.0f);  // stops the advance
            m_physics->SetMotorSpeedZ(0.0f);  // stops the rotation
            return ERR_STOP;
        }
    }

    if ( m_goalMode == TGG_EXPRESS )
    {
        dist = Math::DistanceProjected(m_goal, pos);
        float margin = 10.0f;
        if ( m_object->Implements(ObjectInterfaceType::Flying) ) margin = 20.0f;
        if ( dist < margin && dist > m_lastDistance )
        {
            return ERR_STOP;
        }
        m_lastDistance = dist;
    }

    if ( m_phase == TGP_ADVANCE )  // going towards the goal?
    {
        if ( m_physics->GetLand() )  limit = 0.1f;  // on the ground
        else                         limit = 1.0f;  // flying
        if ( m_bApprox )  limit = 2.0f;

        if ( fabs(pos.x - m_goal.x) < limit &&
             fabs(pos.z - m_goal.z) < limit )
        {
            m_physics->SetMotorSpeedX(0.0f);  // stops the advance
            m_physics->SetMotorSpeedZ(0.0f);  // stops the rotation
            m_phase = TGP_LAND;
        }
    }

    if ( m_phase == TGP_LAND )  // landed?
    {
        if ( m_object->Implements(ObjectInterfaceType::Flying) && m_altitude > 0.0f )
        {
            if ( !m_physics->GetLand() )  return ERR_CONTINUE;
            m_physics->SetMotorSpeedY(0.0f);
        }

        if ( m_bTake )
        {
            m_angle = Math::RotateAngle(m_goalObject.x-pos.x, pos.z-m_goalObject.z);
            m_phase = TGP_TURN;
        }
        else
        {
            return ERR_STOP;
        }
    }

    if ( m_phase == TGP_TURN )  // turns to the object?
    {
        angle = Math::NormAngle(m_object->GetRotationY());
        limit = 0.02f;
        if ( m_bApprox )  limit = 0.10f;
        if ( fabs(angle-m_angle) < limit )
        {
            m_physics->SetMotorSpeedZ(0.0f);  // stops the rotation
            if ( m_bmFinalMove == 0.0f )  return ERR_STOP;

            m_bmFinalPos = m_object->GetPosition();
            m_bmFinalDist = m_physics->GetLinLength(m_bmFinalMove);
            m_bmTimeLimit = m_physics->GetLinTimeLength(fabs(m_bmFinalMove))*1.5f;
            if ( m_bmTimeLimit < 0.5f )  m_bmTimeLimit = 0.5f;
            m_phase = TGP_MOVE;
        }
    }

    if ( m_phase == TGP_CRWAIT )  // waits after collision?
    {
        if ( m_crashMode == TGC_HALT )
        {
            m_physics->SetMotorSpeedX(0.0f);  // stops the advance
            m_physics->SetMotorSpeedZ(0.0f);  // stops the rotation
            m_error = ERR_UNKNOWN;
            return m_error;
        }
        if ( m_time >= 1.0f )
        {
            if ( m_crashMode == TGC_RIGHTLEFT ||
                 m_crashMode == TGC_RIGHT     )  angle =  Math::PI/2.0f;  // 90 deegres to the right
            else                            angle = -Math::PI/2.0f;  // 90 deegres to the left
            m_angle = Math::NormAngle(m_object->GetRotationY()+angle);
            m_phase = TGP_CRTURN;
//?         m_phase = TGP_ADVANCE;
        }
    }

    if ( m_phase == TGP_CRTURN )  // turns after collision?
    {
        angle = Math::NormAngle(m_object->GetRotationY());
        limit = 0.1f;
        if ( fabs(angle-m_angle) < limit )
        {
            m_physics->SetMotorSpeedZ(0.0f);  // stops the rotation
            m_pos = pos;
            m_phase = TGP_CRADVANCE;
        }
    }

    if ( m_phase == TGP_CRADVANCE )  // advance after collision?
    {
        if ( glm::distance(pos, m_pos) >= 5.0f )
        {
            m_phase = TGP_ADVANCE;
        }
    }

    if ( m_phase == TGP_CLWAIT )  // waits after collision?
    {
        if ( m_time >= 1.0f )
        {
            if ( m_crashMode == TGC_RIGHTLEFT )  angle = -Math::PI;
            if ( m_crashMode == TGC_LEFTRIGHT )  angle =  Math::PI;
            if ( m_crashMode == TGC_RIGHT     )  angle =  Math::PI/2.0f;
            if ( m_crashMode == TGC_LEFT      )  angle = -Math::PI/2.0f;
            m_angle = Math::NormAngle(m_object->GetRotationY()+angle);
            m_phase = TGP_CLTURN;
        }
    }

    if ( m_phase == TGP_CLTURN )  // turns after collision?
    {
        angle = Math::NormAngle(m_object->GetRotationY());
        limit = 0.1f;
        if ( fabs(angle-m_angle) < limit )
        {
            m_physics->SetMotorSpeedZ(0.0f);  // stops the rotation
            m_pos = pos;
            m_phase = TGP_CLADVANCE;
        }
    }

    if ( m_phase == TGP_CLADVANCE )  // advance after collision?
    {
        if ( glm::distance(pos, m_pos) >= 10.0f )
        {
            m_phase = TGP_ADVANCE;
            m_try ++;
        }
    }

    if ( m_phase == TGP_MOVE )  // final advance?
    {
        if ( m_bmTimeLimit <= 0.0f )
        {
            m_physics->SetMotorSpeedX(0.0f);  // stops
            Abort();
            return ERR_STOP;
        }

        dist = glm::distance(m_bmFinalPos, m_object->GetPosition());
        if ( dist < m_bmFinalDist )  return ERR_CONTINUE;
        m_physics->SetMotorSpeedX(0.0f);  // stops the advance
        return ERR_STOP;
    }

    return ERR_CONTINUE;
}


// Tries the object is the target position.

CObject* CTaskGoto::SearchTarget(glm::vec3 pos, float margin)
{
    //return CObjectManager::GetInstancePointer()->FindNearest(nullptr, pos, OBJECT_NULL, margin/g_unit);

    /*
     * TODO: FindNearest() can't be used here. Reverted to code from before 4fef3af9ef1fbe61a0c4c3f5c176f56257428efb
     *
     * The reason is that in the case of multiple objects being placed at the same position,
     * this function needs to return the last one in order of creation. FindNearest() does the opposite.
     *
     * Whoever designed goto() so that it has to guess which object the user wants based only on position - thanks
     * for making it so confusing :/
     *
     * This works well enough assuming that portable objects from the level file are always created after the objects
     * they are placed on, for example BlackBox is created after GoalArea, TitaniumOre is created after Converter etc.
     * This is probably required anyway to prevent them from sinking into the ground.
     *
     * User-created objects don't make a difference because there is no way you can place them precisely enough
     * for floats to compare with ==.
     *
     * See issue #732
     */

    CObject* pBest = nullptr;
    float min = 1000000.0f;
    for ( CObject* pObj : CObjectManager::GetInstancePointer()->GetAllObjects() )
    {
        if ( !pObj->GetActive() )  continue;
        if ( IsObjectBeingTransported(pObj) )  continue;  // object transtorted?

        glm::vec3 oPos = pObj->GetPosition();
        float dist = Math::DistanceProjected(pos, oPos);

        if ( dist <= margin && dist <= min )
        {
            min = dist;
            pBest = pObj;
        }
    }

    return pBest;
}

// Adjusts the target as a function of the object.
// Returns true if it is cargo laying on the ground, which can be approached from any site.

bool CTaskGoto::AdjustTarget(CObject* pObj, glm::vec3 &pos, float &distance)
{
    ObjectType  type;
    glm::vec3    goal;
    float       dist, suppl;

    type = m_object->GetType();
    if ( type == OBJECT_BEE  ||
         type == OBJECT_WORM )
    {
        pos = pObj->GetPosition();
        return false;  // single approach
    }

    type = pObj->GetType();

    if ( pObj->Implements(ObjectInterfaceType::Transportable) ||
         type == OBJECT_RUINmobilew1 || // TODO: CRecoverableObject?
         type == OBJECT_RUINmobilew2 ||
         type == OBJECT_RUINmobilet1 ||
         type == OBJECT_RUINmobilet2 ||
         type == OBJECT_RUINmobiler1 ||
         type == OBJECT_RUINmobiler2 )
    {
        pos = m_object->GetPosition();
        goal = pObj->GetPosition();
        dist = glm::distance(goal, pos);
        pos = (pos-goal)*(TAKE_DIST+distance)/dist + goal;
        return true;  // approach from all sites
    }

    if ( type == OBJECT_BASE )
    {
        pos = m_object->GetPosition();
        goal = pObj->GetPosition();
        dist = glm::distance(goal, pos);
        pos = (pos-goal)*(TAKE_DIST+distance)/dist + goal;
        return true;  // approach from all sites
    }

    if ( type == OBJECT_MOBILEfa ||
         type == OBJECT_MOBILEta ||
         type == OBJECT_MOBILEwa ||
         type == OBJECT_MOBILEia ||
         type == OBJECT_MOBILEfb ||
         type == OBJECT_MOBILEtb ||
         type == OBJECT_MOBILEwb ||
         type == OBJECT_MOBILEib ||
         type == OBJECT_MOBILEfs ||
         type == OBJECT_MOBILEts ||
         type == OBJECT_MOBILEws ||
         type == OBJECT_MOBILEis ||
         type == OBJECT_MOBILEfc ||
         type == OBJECT_MOBILEtc ||
         type == OBJECT_MOBILEwc ||
         type == OBJECT_MOBILEic ||
         type == OBJECT_MOBILEfi ||
         type == OBJECT_MOBILEti ||
         type == OBJECT_MOBILEwi ||
         type == OBJECT_MOBILEii ||
         type == OBJECT_MOBILErt ||
         type == OBJECT_MOBILErc ||
         type == OBJECT_MOBILErr ||
         type == OBJECT_MOBILErs ||
         type == OBJECT_MOBILEsa ||
         type == OBJECT_MOBILEtg ||
         type == OBJECT_MOBILEft ||
         type == OBJECT_MOBILEtt ||
         type == OBJECT_MOBILEwt ||
         type == OBJECT_MOBILEit ||
         type == OBJECT_MOBILErp ||
         type == OBJECT_MOBILEst ||
         type == OBJECT_MOBILEdr )
    {
        CSlottedObject *asSlotted = dynamic_cast<CSlottedObject*>(pObj);
        int powerSlotIndex = asSlotted->MapPseudoSlot(CSlottedObject::Pseudoslot::POWER);
        assert(powerSlotIndex >= 0);
        pos = asSlotted->GetSlotPosition(powerSlotIndex);
        // TODO: this only works for a certain slot angle
        pos.x -= TAKE_DIST+TAKE_DIST_OTHER+distance;
        glm::mat4 mat = pObj->GetWorldMatrix(0);
        pos = Math::Transform(mat, pos);
        return false;  // single approach
    }

    if ( GetHotPoint(pObj, goal, true, distance, suppl) )
    {
        pos = goal;
        distance += suppl;
        return false;  // single approach
    }

    pos = pObj->GetPosition();
    distance = 0.0f;
    return false;  // single approach
}

// If you are on an object produced by a building (ore produced by derrick),
// changes the position by report the building.

bool CTaskGoto::AdjustBuilding(glm::vec3 &pos, float margin, float &distance)
{
    for (CObject* obj : CObjectManager::GetInstancePointer()->GetAllObjects())
    {
        if ( !obj->GetActive() )  continue;
        if (IsObjectBeingTransported(obj))  continue;

        glm::vec3 oPos;
        float suppl = 0.0f;
        if ( !GetHotPoint(obj, oPos, false, 0.0f, suppl) )  continue;
        float dist = Math::DistanceProjected(pos, oPos);
        if ( dist <= margin )
        {
            GetHotPoint(obj, pos, true, distance, suppl);
            distance += suppl;
            return true;
        }
    }
    return false;
}

// Returns the item or product or pose is something on a building.

bool CTaskGoto::GetHotPoint(CObject *pObj, glm::vec3 &pos,
                            bool bTake, float distance, float &suppl)
{
    ObjectType  type;

    pos = glm::vec3(0.0f, 0.0f, 0.0f);
    suppl = 0.0f;
    type = pObj->GetType();

    if ( type == OBJECT_DERRICK )
    {
        glm::mat4 mat = pObj->GetWorldMatrix(0);
        pos.x += 8.0f;
        if ( bTake && distance != 0.0f )  suppl = 4.0f;
        if ( bTake )  pos.x += TAKE_DIST+distance+suppl;
        pos = Math::Transform(mat, pos);
        return true;
    }

    if ( type == OBJECT_CONVERT )
    {
        glm::mat4 mat = pObj->GetWorldMatrix(0);
        pos.x += 0.0f;
        if ( bTake && distance != 0.0f )  suppl = 4.0f;
        if ( bTake )  pos.x += TAKE_DIST+distance+suppl;
        pos = Math::Transform(mat, pos);
        return true;
    }

    if ( type == OBJECT_RESEARCH )
    {
        glm::mat4 mat = pObj->GetWorldMatrix(0);
        pos.x += 10.0f;
        if ( bTake && distance != 0.0f )  suppl = 2.5f;
        if ( bTake )  pos.x += TAKE_DIST+TAKE_DIST_OTHER+distance+suppl;
        pos = Math::Transform(mat, pos);
        return true;
    }

    if ( type == OBJECT_ENERGY )
    {
        glm::mat4 mat = pObj->GetWorldMatrix(0);
        pos.x += 6.0f;
        if ( bTake && distance != 0.0f )  suppl = 6.0f;
        if ( bTake )  pos.x += TAKE_DIST+TAKE_DIST_OTHER+distance;
        pos = Math::Transform(mat, pos);
        return true;
    }

    if ( type == OBJECT_TOWER )
    {
        glm::mat4 mat = pObj->GetWorldMatrix(0);
        pos.x += 5.0f;
        if ( bTake && distance != 0.0f )  suppl = 4.0f;
        if ( bTake )  pos.x += TAKE_DIST+TAKE_DIST_OTHER+distance+suppl;
        pos = Math::Transform(mat, pos);
        return true;
    }

    if ( type == OBJECT_LABO )
    {
        glm::mat4 mat = pObj->GetWorldMatrix(0);
        pos.x += 6.0f;
        if ( bTake && distance != 0.0f )  suppl = 6.0f;
        if ( bTake )  pos.x += TAKE_DIST+TAKE_DIST_OTHER+distance;
        pos = Math::Transform(mat, pos);
        return true;
    }

    if ( type == OBJECT_NUCLEAR )
    {
        glm::mat4 mat = pObj->GetWorldMatrix(0);
        pos.x += 22.0f;
        if ( bTake && distance != 0.0f )  suppl = 4.0f;
        if ( bTake )  pos.x += TAKE_DIST+TAKE_DIST_OTHER+distance+suppl;
        pos = Math::Transform(mat, pos);
        return true;
    }

    if ( type == OBJECT_FACTORY )
    {
        glm::mat4 mat = pObj->GetWorldMatrix(0);
        pos.x += 4.0f;
        if ( bTake && distance != 0.0f )  suppl = 6.0f;
        if ( bTake )  pos.x += TAKE_DIST+distance+suppl;
        pos = Math::Transform(mat, pos);
        return true;
    }

    if ( type == OBJECT_STATION )
    {
        glm::mat4 mat = pObj->GetWorldMatrix(0);
        pos.x += 4.0f;
        if ( bTake && distance != 0.0f )  suppl = 4.0f;
        if ( bTake )  pos.x += distance;
        pos = Math::Transform(mat, pos);
        return true;
    }

    if ( type == OBJECT_REPAIR )
    {
        glm::mat4 mat = pObj->GetWorldMatrix(0);
        pos.x += 4.0f;
        if ( bTake && distance != 0.0f )  suppl = 4.0f;
        if ( bTake )  pos.x += distance;
        pos = Math::Transform(mat, pos);
        return true;
    }

    if ( type == OBJECT_PARA && m_object->Implements(ObjectInterfaceType::Flying) )
    {
        glm::mat4 mat = pObj->GetWorldMatrix(0);
        if ( bTake && distance != 0.0f )  suppl = 20.0f;
        if ( bTake )  pos.x += distance+suppl;
        pos = Math::Transform(mat, pos);
        return true;
    }

    suppl = 0.0f;
    return false;
}


// Seeks an object too close that he must flee.

bool CTaskGoto::LeakSearch(glm::vec3 &pos, float &delay)
{
    if (!m_physics->GetLand())  return false;  // in flight?

    Math::Sphere crashSphere = m_object->GetFirstCrashSphere().sphere;

    float min = 100000.0f;
    CObject* obstacle = nullptr;
    Math::Sphere obstacleCrashSphere;
    for (CObject* obj : CObjectManager::GetInstancePointer()->GetAllObjects())
    {
        if ( obj == m_object )  continue;
        if ( !obj->GetDetectable() )  continue;
        if (IsObjectBeingTransported(obj))  continue;

        for (const auto& objCrashSphere : obj->GetAllCrashSpheres())
        {
            float dist = Math::DistanceProjected(crashSphere.pos, objCrashSphere.sphere.pos);
            if (dist < min)
            {
                min = dist;
                obstacleCrashSphere = objCrashSphere.sphere;
                obstacle = obj;
            }
        }
    }
    if (min > crashSphere.radius + obstacleCrashSphere.radius + 4.0f)  return false;

    m_bLeakRecede = false;

    float dist = 4.0f;
    float dir  = 1.0f;
    if (obstacle->GetType() == OBJECT_FACTORY)
    {
        dist = 16.0f;
        dir  = -1.0f;
        m_bLeakRecede = true;  // simply recoils
    }

    pos = obstacleCrashSphere.pos;
    delay = m_physics->GetLinTimeLength(dist, dir);
    return true;
}


// Calculates the force of repulsion due to obstacles.
// The vector length rendered is between 0 and 1.

void CTaskGoto::ComputeRepulse(glm::vec2&dir)
{
    ObjectType  iType, oType;
    glm::vec2   repulse;
    float       gDist, add, addi, fac, dist;
    bool        bAlien;

    dir.x = 0.0f;
    dir.y = 0.0f;

    // The worm goes everywhere and through everything!
    iType = m_object->GetType();
    if ( iType == OBJECT_WORM || iType == OBJECT_CONTROLLER )  return;

    auto firstCrashSphere = m_object->GetFirstCrashSphere();
    glm::vec3 iPos = firstCrashSphere.sphere.pos;
    float iRadius = firstCrashSphere.sphere.radius;

    gDist = glm::distance(iPos, m_goal);

    add = m_physics->GetLinStopLength()*1.1f;  // braking distance
    fac = 2.0f;

    if ( iType == OBJECT_MOBILEwa ||
         iType == OBJECT_MOBILEwb ||
         iType == OBJECT_MOBILEwc ||
         iType == OBJECT_MOBILEwi ||
         iType == OBJECT_MOBILEws ||
         iType == OBJECT_MOBILEwt )  // wheels?
    {
        add = 5.0f;
        fac = 1.5f;
    }
    if ( iType == OBJECT_MOBILEta ||
         iType == OBJECT_MOBILEtb ||
         iType == OBJECT_MOBILEtc ||
         iType == OBJECT_MOBILEti ||
         iType == OBJECT_MOBILEts ||
         iType == OBJECT_MOBILEtt ||
         iType == OBJECT_MOBILEdr )  // caterpillars?
    {
        add = 4.0f;
        fac = 1.5f;
    }
    if ( iType == OBJECT_MOBILEfa ||
         iType == OBJECT_MOBILEfb ||
         iType == OBJECT_MOBILEfc ||
         iType == OBJECT_MOBILEfi ||
         iType == OBJECT_MOBILEfs ||
         iType == OBJECT_MOBILEft )  // flying?
    {
        if ( m_physics->GetLand() )
        {
            add = 5.0f;
            fac = 1.5f;
        }
        else
        {
            add = 10.0f;
            fac = 1.5f;
        }
    }
    if ( iType == OBJECT_MOBILEia ||
         iType == OBJECT_MOBILEib ||
         iType == OBJECT_MOBILEic ||
         iType == OBJECT_MOBILEii ||
         iType == OBJECT_MOBILEis ||
         iType == OBJECT_MOBILEit )  // legs?
    {
        add = 4.0f;
        fac = 1.5f;
    }
    if ( iType == OBJECT_BEE )  // wasp?
    {
        if ( m_physics->GetLand() )
        {
            add = 3.0f;
            fac = 1.5f;
        }
        else
        {
            add = 5.0f;
            fac = 1.5f;
        }
    }

    bAlien = false;
    if ( iType == OBJECT_MOTHER ||
         iType == OBJECT_ANT    ||
         iType == OBJECT_SPIDER ||
         iType == OBJECT_BEE    ||
         iType == OBJECT_WORM   )
    {
        bAlien = true;
    }

    for (CObject* pObj : CObjectManager::GetInstancePointer()->GetAllObjects())
    {
        if ( pObj == m_object )  continue;
        if (IsObjectBeingTransported(pObj))  continue;

        oType = pObj->GetType();

        if ( oType == OBJECT_WORM )  continue;

        if ( bAlien )
        {
            if ( pObj->Implements(ObjectInterfaceType::Transportable) ||
                 oType == OBJECT_BOMB    ||
                (oType >= OBJECT_PLANT0    &&
                 oType <= OBJECT_PLANT19   ) ||
                (oType >= OBJECT_MUSHROOM1 &&
                 oType <= OBJECT_MUSHROOM2 ) )  continue;
        }

        addi = add;
        if ( iType == OBJECT_BEE &&
             oType == OBJECT_BEE )
        {
            addi = 2.0f;  // between wasps, do not annoy too much
        }

        for (const auto& crashSphere : pObj->GetAllCrashSpheres())
        {
            glm::vec3 oPos = crashSphere.sphere.pos;
            float oRadius = crashSphere.sphere.radius;

            if ( oPos.y-oRadius > iPos.y+iRadius )  continue;
            if ( oPos.y+oRadius < iPos.y-iRadius )  continue;

            dist = glm::distance(oPos, m_goal);
            if ( dist <= 1.0f )  continue;  // on purpose?

            oRadius += iRadius+addi;
            dist = Math::DistanceProjected(oPos, iPos);
            if ( dist > gDist )  continue;  // beyond the goal?
            if ( dist <= oRadius )
            {
                repulse.x = iPos.x-oPos.x;
                repulse.y = iPos.z-oPos.z;

                dist = powf(dist/oRadius, fac);
                dist = 0.2f-0.2f*dist;
                repulse.x *= dist;
                repulse.y *= dist;

                dir.x += repulse.x;
                dir.y += repulse.y;
            }
        }
    }
}

// Calculates the force of vertical repulsion according to barriers.
// The vector length is made​between -1 and 1.

void CTaskGoto::ComputeFlyingRepulse(float &dir)
{
    auto firstCrashSphere = m_object->GetFirstCrashSphere();
    glm::vec3 iPos = firstCrashSphere.sphere.pos;
    float iRadius = firstCrashSphere.sphere.radius;

    float add = 0.0f;
    float fac = 1.5f;
    dir = 0.0f;

    for (CObject* pObj : CObjectManager::GetInstancePointer()->GetAllObjects())
    {
        if ( pObj == m_object )  continue;
        if (IsObjectBeingTransported(pObj))  continue;

        ObjectType oType = pObj->GetType();

        if ( oType == OBJECT_WORM )  continue;

        for (const auto& crashSphere : pObj->GetAllCrashSpheres())
        {
            glm::vec3 oPos = crashSphere.sphere.pos;
            float oRadius = crashSphere.sphere.radius;

            oRadius += iRadius+add;
            float dist = Math::DistanceProjected(oPos, iPos);
            if ( dist <= oRadius )
            {
                float repulse = iPos.y-oPos.y;

                dist = powf(dist/oRadius, fac);
                dist = 0.2f-0.2f*dist;
                repulse *= dist;

                dir += repulse;
            }
        }
    }

    if ( dir < -1.0f )  dir = -1.0f;
    if ( dir >  1.0f )  dir =  1.0f;
}



// Among all of the following, seek if there is one allowing to go directly to the crow flies.
// If yes, skip all the unnecessary intermediate points.

int CTaskGoto::PathFindingShortcut()
{
    int     i;

    for ( i=m_bmTotal ; i>=m_bmIndex+2 ; i-- )  // tries from the last
    {
        if ( BitmapTestLine(m_bmPoints[m_bmIndex], m_bmPoints[i]) )
        {
            return i;  // bingo, found
        }
    }

    return m_bmIndex+1;  // simply goes to the next
}

// That's the big start.

void CTaskGoto::PathFindingStart()
{
    glm::vec3    min, max;

    BitmapOpen();
    BitmapObject();

    min = m_object->GetPosition();
    max = m_goal;
    if ( min.x > max.x )  Math::Swap(min.x, max.x);
    if ( min.z > max.z )  Math::Swap(min.z, max.z);
    min.x -= 10.0f*BM_DIM_STEP;
    min.z -= 10.0f*BM_DIM_STEP;
    max.x += 10.0f*BM_DIM_STEP;
    max.z += 10.0f*BM_DIM_STEP;
    BitmapTerrain(min, max);

    if ( LeakSearch(m_leakPos, m_leakDelay) )
    {
        m_phase = TGP_BEAMLEAK;  // must first leak
        m_leakTime = 0.0f;
    }
    else
    {
        m_physics->SetMotorSpeedX(0.0f);  // stops the advance
        m_physics->SetMotorSpeedZ(0.0f);  // stops the rotation
        PathFindingInit();
        m_phase = TGP_BEAMSEARCH;  // will seek the path
    }
}

// Initialization before the first PathFindingSearch.

void CTaskGoto::PathFindingInit()
{
    int     i;

    for ( i=0 ; i<MAXPOINTS ; i++ )
    {
        m_bmIter[i] = -1;
    }
    m_bmStep = 0;
    for (auto& bucket : m_bfsQueue)
    {
        bucket.clear();
    }
    m_bfsQueueMin = 0;
    m_bfsQueueCountPushed = 0;
    m_bfsQueueCountPopped = 0;
    m_bfsQueueCountRepeated = 0;
    m_bfsQueueCountSkipped = 0;
}

static int HeuristicDistance(int nX, int nY, int startX, int startY)
{
    // 8-way connectivity yields a shortest path that
    // consists of a diagonal and a non-diagonal part.
    //      ...+
    //      :  |
    //      :..|
    //      : /:
    //      :/ :
    //      +..:
    const int distX = std::abs(nX - startX);
    const int distY = std::abs(nY - startY);
    const int smaller = std::min(distX, distY);
    const int bigger = std::max(distX, distY);
    // diagonal number of steps: smaller
    // non-diagonal number of steps: bigger - smaller
    return smaller * (7 - 5) + bigger * 5;
}

// Calculates points and passes to go from start to goal.
// Returns:
// ERR_OK if it's good
// ERR_GOTO_IMPOSSIBLE if impossible
// ERR_GOTO_ITER if aborts because too many recursions
// ERR_CONTINUE if not done yet
// goalRadius: distance at which we must approach the goal

Error CTaskGoto::PathFindingSearch(const glm::vec3 &start, const glm::vec3 &goal,
                                   float goalRadius)
{
    m_bmStep ++;

    // Relative postion and distance to neighbors.
    static const int dXs[8] = {-1, 0, 1, -1, 1, -1, 0, 1};
    static const int dYs[8] = {-1, -1, -1, 0, 0, 1, 1, 1};
    // These are the costs of the edges. They must be less than the number of buckets in the queue.
    static const int32_t dDist[8] = {7, 5, 7, 5, 5, 7, 5, 7};

    const int startX = static_cast<int>((start.x+1600.0f)/BM_DIM_STEP);
    const int startY = static_cast<int>((start.z+1600.0f)/BM_DIM_STEP);
    const int goalX = static_cast<int>((goal.x+1600.0f)/BM_DIM_STEP);
    const int goalY = static_cast<int>((goal.z+1600.0f)/BM_DIM_STEP);

    if (m_bfsQueueCountPushed == 0) // New search
    {
        if (startX == goalX && startY == goalY)
        {
            m_bmPoints[0] = start;
            m_bmPoints[1] = goal;
            m_bmTotal = 1;
            return ERR_OK;
        }
        // Enqueue the goal node
        if ( goalX >= 0 && goalX < m_bmSize &&
            goalY >= 0 && goalY < m_bmSize )
        {
            const int indexInMap = goalY * m_bmSize + goalX;
            const int totalDistance = HeuristicDistance(goalX, goalY, startX, startY);
            m_bfsQueueMin = totalDistance;
            m_bfsDistances[indexInMap] = 0;
            m_bfsQueue[totalDistance % NUMQUEUEBUCKETS].push_back(indexInMap);
            m_bfsQueueCountPushed += 1;
            BitmapSetDot(1, goalX, goalY); // Mark as enqueued
        }
        else
        {
            m_bfsQueueMin = std::numeric_limits<int>::max();
        }

        // Enqueue nodes around the goal
        if (goalRadius > 0.0f)
        {
            const int minX = std::max(0, static_cast<int>((goal.x-goalRadius+1600.0f)/BM_DIM_STEP));
            const int minY = std::max(0, static_cast<int>((goal.z-goalRadius+1600.0f)/BM_DIM_STEP));
            const int maxX = std::min(m_bmSize-1, static_cast<int>((goal.x+goalRadius+1600.0f)/BM_DIM_STEP));
            const int maxY = std::min(m_bmSize-1, static_cast<int>((goal.z+goalRadius+1600.0f)/BM_DIM_STEP));
            for (int y = minY; y <= maxY; ++y)
            {
                for (int x = minX; x <= maxX; ++x)
                {
                    float floatX = (x + 0.5f) * BM_DIM_STEP - 1600.0f;
                    float floatY = (y + 0.5f) * BM_DIM_STEP - 1600.0f;
                    if (std::hypot(floatX-goal.x, floatY-goal.z) <= goalRadius &&
                        BitmapTestDotIsVisitable(x, y) &&
                        !BitmapTestDot(1, x, y))
                    {
                        const int indexInMap = y * m_bmSize + x;
                        const int totalDistance = HeuristicDistance(x, y, startX, startY);
                        m_bfsQueueMin = std::min(m_bfsQueueMin, totalDistance);
                        m_bfsDistances[indexInMap] = 0;
                        m_bfsQueue[totalDistance % NUMQUEUEBUCKETS].push_back(indexInMap);
                        m_bfsQueueCountPushed += 1;
                        BitmapSetDot(1, x, y); // Mark as enqueued
                    }
                }
            }
        }
    }

    m_bmIterCounter = 0;

    while (m_bfsQueueCountPushed != m_bfsQueueCountPopped)
    {
        // Pop a node from the queue
        while (m_bfsQueue[m_bfsQueueMin % NUMQUEUEBUCKETS].empty())
        {
            m_bfsQueueMin += 1;
            if (m_bfsQueueMin % NUMQUEUEBUCKETS == 0 && !m_bfsQueue[NUMQUEUEBUCKETS].empty())
            {
                // Process nodes with oversized costs.
                const size_t countBefore = m_bfsQueue[NUMQUEUEBUCKETS].size();
                for (size_t i = 0; i < m_bfsQueue[NUMQUEUEBUCKETS].size();)
                {
                    const uint32_t indexInMap = m_bfsQueue[NUMQUEUEBUCKETS][i];
                    const int x = indexInMap % m_bmSize;
                    const int y = indexInMap / m_bmSize;
                    const int32_t distance = m_bfsDistances[indexInMap];
                    const int totalDistance = distance + HeuristicDistance(x, y, startX, startY);
                    if (totalDistance < m_bfsQueueMin + NUMQUEUEBUCKETS)
                    {
                        // Move node to a regular bucket.
                        m_bfsQueue[totalDistance % NUMQUEUEBUCKETS].push_back(indexInMap);
                        m_bfsQueue[NUMQUEUEBUCKETS][i] = m_bfsQueue[NUMQUEUEBUCKETS].back();
                        m_bfsQueue[NUMQUEUEBUCKETS].pop_back();
                    }
                    else
                    {
                        // Look at next node.
                        i += 1;
                    }
                }
                const size_t countAfter = m_bfsQueue[NUMQUEUEBUCKETS].size();
                GetLogger()->Debug("Redistributed %% of %% nodes from the bucket with oversized costs.",
                    countBefore - countAfter, countBefore);
            }
        }
        auto& bucket = m_bfsQueue[m_bfsQueueMin % NUMQUEUEBUCKETS];
        const uint32_t indexInMap = bucket.back();
        bucket.pop_back();
        m_bfsQueueCountPopped += 1;

        const int x = indexInMap % m_bmSize;
        const int y = indexInMap / m_bmSize;
        const int32_t distance = m_bfsDistances[indexInMap];
        const int totalDistance = distance + HeuristicDistance(x, y, startX, startY);

        if (totalDistance != m_bfsQueueMin)
        {
            if (totalDistance < m_bfsQueueMin)
            {
                // This node has been updated to a lower cost and has allready been processed.
                m_bfsQueueCountSkipped += 1;
                // GetLogger()->Debug("Skipping node with smaller distance, distance: %%, totalDistance: %%, m_bfsQueueMin: %%",
                //     distance, totalDistance, m_bfsQueueMin);
            }
            else
            {
                if (totalDistance < m_bfsQueueMin + NUMQUEUEBUCKETS)
                {
                    // Move node to a regular bucket.
                    m_bfsQueue[totalDistance % NUMQUEUEBUCKETS].push_back(indexInMap);
                    m_bfsQueueCountPushed += 1;
                    GetLogger()->Debug("Moving node with bigger distance into regular bucket, distance: %%, totalDistance: %%, m_bfsQueueMin: %%",
                        distance, totalDistance, m_bfsQueueMin);
                }
                else
                {
                    // Move node to the bucket with oversized costs.
                    m_bfsQueue[NUMQUEUEBUCKETS].push_back(indexInMap);
                    m_bfsQueueCountPushed += 1;
                    GetLogger()->Debug("Moving node with bigger distance into bucket with oversized costs, distance: %%, totalDistance: %%, m_bfsQueueMin: %%",
                        distance, totalDistance, m_bfsQueueMin);
                }
            }
            continue;
        }

        if (x == startX && y == startY)
        {
            // We have reached the start.
            // Follow decreasing distances to find the path.
            m_bmPoints[0] = start;
            int btX = x;
            int btY = y;
            for (m_bmTotal = 1; m_bmTotal < MAXPOINTS; ++m_bmTotal)
            {
                int bestX = -1;
                int bestY = -1;
                int32_t bestDistance = std::numeric_limits<int32_t>::max();
                for (int i = 0; i < 8; ++i)
                {
                    const int nX = btX + dXs[i];
                    const int nY = btY + dYs[i];
                    if (!BitmapTestDot(1, nX, nY)) continue;
                    const int32_t nDistance = m_bfsDistances[nY * m_bmSize + nX];
                    if (nDistance < bestDistance)
                    {
                        bestX = nX;
                        bestY = nY;
                        bestDistance = nDistance;
                    }
                }
                if (bestX == -1)
                {
                    GetLogger()->Debug("Failed to find node parent");
                    return ERR_GOTO_ITER;
                }
                btX = bestX;
                btY = bestY;
                if (btX == goalX && btY == goalY)
                {
                    m_bmPoints[m_bmTotal] = goal;
                }
                else
                {
                    m_bmPoints[m_bmTotal].x = (btX + 0.5f) * BM_DIM_STEP - 1600.f;
                    m_bmPoints[m_bmTotal].z = (btY + 0.5f) * BM_DIM_STEP - 1600.f;
                }

                if (bestDistance == 0)
                {
                    if (goalRadius > 0.0f)
                    {
                        // Find a more exact position by repeatedly bisecting the interval.
                        const float r2 = goalRadius * goalRadius;
                        glm::vec3 inside = m_bmPoints[m_bmTotal] - goal;
                        glm::vec3 outside = m_bmPoints[m_bmTotal-1] - goal;
                        glm::vec3 mid = (inside + outside) * 0.5f;
                        for (int i = 0; i < 10; ++i)
                        {
                            if (mid.x*mid.x + mid.z*mid.z < r2)
                            {
                                inside = mid;
                            }
                            else
                            {
                                outside = mid;
                            }
                            mid = (inside + outside) * 0.5f;
                        }
                        m_bmPoints[m_bmTotal] = mid + goal;
                    }
                    break;
                }
            }

            const float distanceToGoal = Math::DistanceProjected(m_bmPoints[m_bmTotal], goal);
            GetLogger()->Debug("Found path to goal with %% nodes and %% cost. Final distance to goal: %%", m_bmTotal + 1, totalDistance, distanceToGoal);
            GetLogger()->Debug("m_bmStep: %%", m_bmStep);
            GetLogger()->Debug("m_bfsQueueMin: %% mod %% = %%", m_bfsQueueMin, NUMQUEUEBUCKETS, m_bfsQueueMin % NUMQUEUEBUCKETS);
            GetLogger()->Debug("m_bfsQueueCountPushed: %%", m_bfsQueueCountPushed);
            GetLogger()->Debug("m_bfsQueueCountPopped: %%", m_bfsQueueCountPopped);
            GetLogger()->Debug("m_bfsQueueCountRepeated: %%", m_bfsQueueCountRepeated);
            GetLogger()->Debug("m_bfsQueueCountSkipped: %%", m_bfsQueueCountSkipped);
            GetLogger()->Debug("m_bfsQueue sizes:\n");
            for (size_t i = 0; i < m_bfsQueue.size(); ++i)
            {
                if (!m_bfsQueue[i].empty()) GetLogger()->Debug("    %%: %%", i, m_bfsQueue[i].size());
            }
            return ERR_OK;
        }

        // Expand the node
        for (int i = 0; i < 8; ++i)
        {
            const int nX = x + dXs[i];
            const int nY = y + dYs[i];
            if (BitmapTestDotIsVisitable(nX, nY))
            {
                const int neighborIndexInMap = nY * m_bmSize + nX;
                const int32_t newDistance = distance + dDist[i];
                if (BitmapTestDot(1, nX, nY))
                {
                    // We have seen this node before.
                    // Only enqueue previously seen nodes if this is a shorter path.
                    if (newDistance < m_bfsDistances[neighborIndexInMap])
                    {
                        m_bfsQueueCountRepeated += 1;
                    }
                    else
                    {
                        continue;
                    }
                }

                // Enqueue this neighbor
                const int32_t newTotalDistance = newDistance + HeuristicDistance(nX, nY, startX, startY);
                m_bfsDistances[neighborIndexInMap] = newDistance;
                m_bfsQueue[newTotalDistance % NUMQUEUEBUCKETS].push_back(neighborIndexInMap);
                m_bfsQueueCountPushed += 1;
                BitmapSetDot(1, nX, nY); // Mark as enqueued
            }
        }

        // Limit the number of iterations per frame.
        m_bmIterCounter ++;
        if ( m_bmIterCounter >= NB_ITER )  return ERR_CONTINUE;
    }

    return ERR_GOTO_IMPOSSIBLE;
}

// Tests if a path along a straight line is possible.

bool CTaskGoto::BitmapTestLine(const glm::vec3 &start, const glm::vec3 &goal)
{
    if ( m_bmArray == nullptr )  return true;

    const glm::vec2 startInGrid = glm::vec2((start.x+1600.0f)/BM_DIM_STEP, (start.z+1600.0f)/BM_DIM_STEP);
    const glm::vec2 goalInGrid  = glm::vec2((goal.x+1600.0f)/BM_DIM_STEP, (goal.z+1600.0f)/BM_DIM_STEP);

    const int startXInt = static_cast<int>(startInGrid.x);
    const int startYInt = static_cast<int>(startInGrid.y);
    const int goalXInt = static_cast<int>(goalInGrid.x);
    const int goalYInt = static_cast<int>(goalInGrid.y);

    if (startXInt == goalXInt && startYInt == goalYInt)
    {
        return true;
    }

    // Grid traversal based on
    // Amanatides, John, and Andrew Woo. "A fast voxel traversal algorithm for ray tracing." Eurographics. Vol. 87. No. 3. 1987.
    // http://www.cse.yorku.ca/~amana/research/grid.pdf

    glm::vec2 dirInGrid = goalInGrid - startInGrid;
    dirInGrid /= std::hypot(dirInGrid.x, dirInGrid.y);
    const int stepX = dirInGrid.x > 0.0f ? 1 : -1;
    const int stepY = dirInGrid.y > 0.0f ? 1 : -1;

    // At what t does the ray enter the next cell?
    float tMaxX =
        dirInGrid.x > 0.0 ? (std::floor(startInGrid.x) - startInGrid.x + 1) / dirInGrid.x :
        dirInGrid.x < 0.0 ? (std::floor(startInGrid.x) - startInGrid.x) / dirInGrid.x :
        std::numeric_limits<float>::infinity();
    float tMaxY =
        dirInGrid.y > 0.0 ? (std::floor(startInGrid.y) - startInGrid.y + 1) / dirInGrid.y :
        dirInGrid.y < 0.0 ? (std::floor(startInGrid.y) - startInGrid.y) / dirInGrid.y :
        std::numeric_limits<float>::infinity();

    // How much t is needed to step from one column/row to another?
    // stepX = dir.x * t
    // stepX / dir.x = t
    const float tDeltaX = static_cast<float>(stepX) / dirInGrid.x;
    const float tDeltaY = static_cast<float>(stepY) / dirInGrid.y;

    // Traverse the grid
    const int numIntersections =
        std::abs(goalXInt - startXInt) +
        std::abs(goalYInt - startYInt);
    int x = startXInt;
    int y = startYInt;

    for ( int i = 0; i < numIntersections; ++i )
    {
        if ( tMaxX < tMaxY )
        {
            tMaxX += tDeltaX;
            x += stepX;
        }
        else
        {
            tMaxY += tDeltaY;
            y += stepY;
        }
        if ( BitmapTestDot(0, x, y) )
        {
            return false;
        }
    }

    return true;
}

// Adds the objects in the bitmap.

void CTaskGoto::BitmapObject()
{
    auto firstCrashSphere = m_object->GetFirstCrashSphere();
    float iRadius = firstCrashSphere.sphere.radius;

    for (CObject* pObj : CObjectManager::GetInstancePointer()->GetAllObjects())
    {
        ObjectType type = pObj->GetType();

        if ( pObj == m_object )  continue;
        if ( pObj == m_bmCargoObject )  continue;
        if (IsObjectBeingTransported(pObj))  continue;

        float h = m_terrain->GetFloorLevel(pObj->GetPosition(), false);
        if ( m_object->Implements(ObjectInterfaceType::Flying) && m_altitude > 0.0f )
        {
            h += m_altitude;
        }

        for (const auto& crashSphere : pObj->GetAllCrashSpheres())
        {
            glm::vec3 oPos = crashSphere.sphere.pos;
            float oRadius = crashSphere.sphere.radius;

            if ( m_object->Implements(ObjectInterfaceType::Flying) && m_altitude > 0.0f )  // flying?
            {
                if ( oPos.y-oRadius > h+8.0f ||
                     oPos.y+oRadius < h-8.0f )  continue;
            }
            else    // crawling?
            {
                if ( oPos.y-oRadius > h+8.0f )  continue;
            }

            if ( type == OBJECT_PARA )  oRadius -= 2.0f;
            BitmapSetCircle(oPos, oRadius+iRadius+SAFETY_MARGIN);
        }
    }
}

// Adds a section of land in the bitmap.

void CTaskGoto::BitmapTerrain(const glm::vec3 &min, const glm::vec3 &max)
{
    int     minx, miny, maxx, maxy;

    minx = static_cast<int>((min.x+1600.0f)/BM_DIM_STEP);
    miny = static_cast<int>((min.z+1600.0f)/BM_DIM_STEP);
    maxx = static_cast<int>((max.x+1600.0f)/BM_DIM_STEP);
    maxy = static_cast<int>((max.z+1600.0f)/BM_DIM_STEP);

    BitmapTerrain(minx, miny, maxx, maxy);
}

// Adds a section of land in the bitmap.

void CTaskGoto::BitmapTerrain(int minx, int miny, int maxx, int maxy)
{
    ObjectType  type;
    glm::vec3    p;
    float       aLimit, angle, h;
    int         x, y;
    bool        bAcceptWater, bFly;

    if ( minx > maxx )  Math::Swap(minx, maxx);
    if ( miny > maxy )  Math::Swap(miny, maxy);

    if ( minx < 0          )  minx = 0;
    if ( miny < 0          )  miny = 0;
    if ( maxx > m_bmSize-1 )  maxx = m_bmSize-1;
    if ( maxy > m_bmSize-1 )  maxy = m_bmSize-1;

    if ( minx > m_bmMinX )  minx = m_bmMinX;
    if ( miny > m_bmMinY )  miny = m_bmMinY;
    if ( maxx < m_bmMaxX )  maxx = m_bmMaxX;
    if ( maxy < m_bmMaxY )  maxy = m_bmMaxY;

    if ( minx >= m_bmMinX && maxx <= m_bmMaxX &&
         miny >= m_bmMinY && maxy <= m_bmMaxY )  return;

    aLimit = 20.0f*Math::PI/180.0f;
    bAcceptWater = false;
    bFly = false;

    type = m_object->GetType();

    if ( type == OBJECT_MOBILEwa ||
         type == OBJECT_MOBILEwb ||
         type == OBJECT_MOBILEwc ||
         type == OBJECT_MOBILEws ||
         type == OBJECT_MOBILEwi ||
         type == OBJECT_MOBILEwt ||
         type == OBJECT_MOBILEtg )  // wheels?
    {
        aLimit = 20.0f*Math::PI/180.0f;
    }

    if ( type == OBJECT_MOBILEta ||
         type == OBJECT_MOBILEtb ||
         type == OBJECT_MOBILEtc ||
         type == OBJECT_MOBILEti ||
         type == OBJECT_MOBILEts )  // caterpillars?
    {
        aLimit = 35.0f*Math::PI/180.0f;
    }

    if ( type == OBJECT_MOBILErt ||
         type == OBJECT_MOBILErc ||
         type == OBJECT_MOBILErr ||
         type == OBJECT_MOBILErs ||
         type == OBJECT_MOBILErp )  // large caterpillars?
    {
        aLimit = 35.0f*Math::PI/180.0f;
    }

    if ( type == OBJECT_MOBILEsa ||
         type == OBJECT_MOBILEst )  // submarine caterpillars?
    {
        aLimit = 35.0f*Math::PI/180.0f;
        bAcceptWater = true;
    }

    if ( type == OBJECT_MOBILEdr )  // designer caterpillars?
    {
        aLimit = 35.0f*Math::PI/180.0f;
    }

    if ( type == OBJECT_MOBILEfa ||
         type == OBJECT_MOBILEfb ||
         type == OBJECT_MOBILEfc ||
         type == OBJECT_MOBILEfs ||
         type == OBJECT_MOBILEfi ||
         type == OBJECT_MOBILEft )  // flying?
    {
        aLimit = 15.0f*Math::PI/180.0f;
        bFly = true;
    }

    if ( type == OBJECT_MOBILEia ||
         type == OBJECT_MOBILEib ||
         type == OBJECT_MOBILEic ||
         type == OBJECT_MOBILEis ||
         type == OBJECT_MOBILEii )  // insect legs?
    {
        aLimit = 60.0f*Math::PI/180.0f;
    }

    for ( y=miny ; y<=maxy ; y++ )
    {
        for ( x=minx ; x<=maxx ; x++ )
        {
            if ( x >= m_bmMinX && x <= m_bmMaxX &&
                 y >= m_bmMinY && y <= m_bmMaxY )  continue;

            p.x = x*BM_DIM_STEP-1600.0f;
            p.z = y*BM_DIM_STEP-1600.0f;

            if ( bFly )  // flying robot?
            {
                h = m_terrain->GetFloorLevel(p, true);
                if ( h >= m_terrain->GetFlyingMaxHeight()-5.0f )
                {
                    BitmapSetDot(0, x, y);
                }
                continue;
            }

            if ( !bAcceptWater )  // not going underwater?
            {
                h = m_terrain->GetFloorLevel(p, true);
                if ( h < m_water->GetLevel()-2.0f )  // under water (*)?
                {
//?                 BitmapSetDot(0, x, y);
                    BitmapSetCircle(p, BM_DIM_STEP*1.0f);
                    continue;
                }
            }

            angle = m_terrain->GetFineSlope(p);
            if ( angle > aLimit )
            {
                BitmapSetDot(0, x, y);
            }
        }
    }

    m_bmMinX = minx;
    m_bmMinY = miny;
    m_bmMaxX = maxx;
    m_bmMaxY = maxy;  // expanded rectangular area
}

// (*)  Accepts that a robot is 50cm under water, for example Tropica 3!

// Opens an empty bitmap.

bool CTaskGoto::BitmapOpen()
{
    m_bmSize = static_cast<int>(3200.0f/BM_DIM_STEP);
    if (m_bmArray.get() == nullptr) m_bmArray = std::make_unique<unsigned char[]>(m_bmSize * m_bmSize / 8 * 2);
    memset(m_bmArray.get(), 0, m_bmSize*m_bmSize/8*2);
    if (m_bfsDistances.get() == nullptr) m_bfsDistances = std::make_unique<int32_t[]>(m_bmSize * m_bmSize);
    for (auto& bucket : m_bfsQueue)
    {
        bucket.reserve(256);
    }
    m_bmChanged = true;

    m_bmOffset = m_bmSize/2;
    m_bmLine = m_bmSize/8;

    m_bmMinX = m_bmSize;  // non-existent rectangular area
    m_bmMinY = m_bmSize;
    m_bmMaxX = 0;
    m_bmMaxY = 0;

    return true;
}

// Closes the bitmap.

bool CTaskGoto::BitmapClose()
{
    m_bmArray.reset();
    m_bmChanged = true;
    return true;
}

// Puts a circle in the bitmap.

void CTaskGoto::BitmapSetCircle(const glm::vec3 &pos, float radius)
{
    float   d, r;
    int     cx, cy, ix, iy;

    cx = static_cast<int>((pos.x+1600.0f)/BM_DIM_STEP);
    cy = static_cast<int>((pos.z+1600.0f)/BM_DIM_STEP);
    r = radius/BM_DIM_STEP;

    for ( iy=cy-static_cast<int>(r) ; iy<=cy+static_cast<int>(r) ; iy++ )
    {
        for ( ix=cx-static_cast<int>(r) ; ix<=cx+static_cast<int>(r) ; ix++ )
        {
            d = glm::length(glm::vec2(static_cast<float>(ix-cx), static_cast<float>(iy-cy)));
            if ( d > r )  continue;
            BitmapSetDot(0, ix, iy);
        }
    }
}

// Removes a circle in the bitmap.
//TODO this method is almost same as above one
void CTaskGoto::BitmapClearCircle(const glm::vec3 &pos, float radius)
{
    float   d, r;
    int     cx, cy, ix, iy;

    cx = static_cast<int>((pos.x+1600.0f)/BM_DIM_STEP);
    cy = static_cast<int>((pos.z+1600.0f)/BM_DIM_STEP);
    r = radius/BM_DIM_STEP;

    for ( iy=cy-static_cast<int>(r) ; iy<=cy+static_cast<int>(r) ; iy++ )
    {
        for ( ix=cx-static_cast<int>(r) ; ix<=cx+static_cast<int>(r) ; ix++ )
        {
            d = glm::length(glm::vec2(static_cast<float>(ix-cx), static_cast<float>(iy-cy)));
            if ( d > r )  continue;
            BitmapClearDot(0, ix, iy);
        }
    }
}

// Makes a point in the bitmap.
// x:y: 0..m_bmSize-1

void CTaskGoto::BitmapSetDot(int rank, int x, int y)
{
    if ( x < 0 || x >= m_bmSize ||
         y < 0 || y >= m_bmSize )  return;

    m_bmArray[rank*m_bmLine*m_bmSize + m_bmLine*y + x/8] |= (1<<x%8);
    m_bmChanged = true;
}

// Removes a point in the bitmap.
// x:y: 0..m_bmSize-1

void CTaskGoto::BitmapClearDot(int rank, int x, int y)
{
    if ( x < 0 || x >= m_bmSize ||
         y < 0 || y >= m_bmSize )  return;

    m_bmArray[rank*m_bmLine*m_bmSize + m_bmLine*y + x/8] &= ~(1<<x%8);
    m_bmChanged = true;
}

// Tests a point in the bitmap.
// x:y: 0..m_bmSize-1

bool CTaskGoto::BitmapTestDot(int rank, int x, int y)
{
    if ( x < 0 || x >= m_bmSize ||
         y < 0 || y >= m_bmSize )  return false;

    if ( x < m_bmMinX || x > m_bmMaxX ||
         y < m_bmMinY || y > m_bmMaxY )
    {
        BitmapTerrain(x-10,y-10, x+10,y+10);  // remade a layer
    }

    return m_bmArray[rank*m_bmLine*m_bmSize + m_bmLine*y + x/8] & (1<<x%8);
}

bool CTaskGoto::BitmapTestDotIsVisitable(int x, int y)
{
    if ( x < 0 || x >= m_bmSize ||
         y < 0 || y >= m_bmSize )  return false;

    if ( x < m_bmMinX || x > m_bmMaxX ||
         y < m_bmMinY || y > m_bmMaxY )
    {
        BitmapTerrain(x-10,y-10, x+10,y+10);  // remade a layer
    }

    return !(m_bmArray[m_bmLine*y + x/8] & (1<<x%8));
}
