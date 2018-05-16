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


#include "object/motion/motiontoto.h"

#include "app/app.h"

#include "graphics/engine/oldmodelmanager.h"
#include "graphics/engine/terrain.h"
#include "graphics/engine/water.h"

#include "level/robotmain.h"

#include "math/geometry.h"

#include "object/old_object.h"

#include "sound/sound.h"


#include <stdio.h>





// Object's constructor.

CMotionToto::CMotionToto(COldObject* object) : CMotion(object)
{
    m_time = 0.0f;
    m_bDisplayInfo = false;
    m_bQuickPos = false;
    m_bStartAction = false;
    m_speedAction = 20.0f;
    m_soundChannel = -1;
    m_clownRadius = 0.0f;
    m_clownDelay  = 0.0f;
    m_clownTime   = 0.0f;
    m_blinkTime   = 0.0f;
    m_blinkProgress = -1.0f;
    m_lastMotorParticle = 0.0f;
    m_mousePos = Math::Point(0.0f, 0.0f);
}

// Object's destructor.

CMotionToto::~CMotionToto()
{
}


// Removes an object.

void CMotionToto::DeleteObject(bool bAll)
{
    if ( m_soundChannel != -1 )
    {
        m_sound->Stop(m_soundChannel);
        m_soundChannel = -1;
    }
}


// Creates a vehicle traveling any lands on the ground.

void CMotionToto::Create(Math::Vector pos, float angle, ObjectType type,
                         float power, Gfx::COldModelManager* modelManager)
{
    int             rank;


    m_object->SetType(type);

    // Creates the head.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_VEHICLE);  // this is a moving object
    m_object->SetObjectRank(0, rank);
    modelManager->AddModelReference("toto1.mod", false, rank);
    m_object->SetPosition(pos);
    m_object->SetRotationY(angle);

    // Creates mouth.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(1, rank);
    m_object->SetObjectParent(1, 0);
    modelManager->AddModelReference("toto2.mod", false, rank);
    m_object->SetPartPosition(1, Math::Vector(1.00f, 0.17f, 0.00f));

    // Creates the left eye.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(2, rank);
    m_object->SetObjectParent(2, 0);
    modelManager->AddModelReference("toto3.mod", true, rank);
    m_object->SetPartPosition(2, Math::Vector(0.85f, 1.04f, 0.25f));
    m_object->SetPartRotationY(2, -20.0f*Math::PI/180.0f);

    // Creates the right eye.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(3, rank);
    m_object->SetObjectParent(3, 0);
    modelManager->AddModelReference("toto3.mod", false, rank);
    m_object->SetPartPosition(3, Math::Vector(0.85f, 1.04f, -0.25f));
    m_object->SetPartRotationY(3, 20.0f*Math::PI/180.0f);

    // Creates left antenna.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(4, rank);
    m_object->SetObjectParent(4, 0);
    modelManager->AddModelReference("toto4.mod", false, rank);
    m_object->SetPartPosition(4, Math::Vector(0.0f, 1.9f, 0.3f));
    m_object->SetPartRotationX(4, 30.0f*Math::PI/180.0f);

    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(5, rank);
    m_object->SetObjectParent(5, 4);
    modelManager->AddModelReference("toto4.mod", false, rank);
    m_object->SetPartPosition(5, Math::Vector(0.0f, 0.67f, 0.0f));
    m_object->SetPartRotationX(5, 30.0f*Math::PI/180.0f);

    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(6, rank);
    m_object->SetObjectParent(6, 5);
    modelManager->AddModelReference("toto5.mod", false, rank);
    m_object->SetPartPosition(6, Math::Vector(0.0f, 0.70f, 0.0f));
    m_object->SetPartRotationX(6, 30.0f*Math::PI/180.0f);

    // Creates right antenna.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(7, rank);
    m_object->SetObjectParent(7, 0);
    modelManager->AddModelReference("toto4.mod", false, rank);
    m_object->SetPartPosition(7, Math::Vector(0.0f, 1.9f, -0.3f));
    m_object->SetPartRotationX(7, -30.0f*Math::PI/180.0f);

    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(8, rank);
    m_object->SetObjectParent(8, 7);
    modelManager->AddModelReference("toto4.mod", false, rank);
    m_object->SetPartPosition(8, Math::Vector(0.0f, 0.67f, 0.0f));
    m_object->SetPartRotationX(8, -30.0f*Math::PI/180.0f);

    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(9, rank);
    m_object->SetObjectParent(9, 8);
    modelManager->AddModelReference("toto5.mod", false, rank);
    m_object->SetPartPosition(9, Math::Vector(0.0f, 0.70f, 0.0f));
    m_object->SetPartRotationX(9, -30.0f*Math::PI/180.0f);

    m_object->SetScale(0.5f);  // is little
    m_object->SetFloorHeight(0.0f);

    pos = m_object->GetPosition();
    m_object->SetPosition(pos);  // to display the shadows immediately

    m_engine->LoadAllTextures();
}


// Beginning of the display of informations, with foo in the left margin.

void CMotionToto::StartDisplayInfo()
{
return;
//?
    m_bDisplayInfo = true;

    m_actionType = -1;
    m_actionTime = 0.0f;
    m_progress   = 0.0f;

    m_object->SetRotationY(0.0f);
    m_mousePos = Math::Point(0.5f, 0.5f);
}

// End of the display of informations.

void CMotionToto::StopDisplayInfo()
{
    m_bDisplayInfo = false;
    m_bQuickPos = true;
}

// Getes the position of the mouse.

void CMotionToto::SetMousePos(Math::Point pos)
{
    m_mousePos = pos;
}


// Management of an event.

bool CMotionToto::EventProcess(const Event &event)
{
    CMotion::EventProcess(event);

    if ( event.type == EVENT_FRAME )
    {
        return EventFrame(event);
    }

    return true;
}

// Management of an event.

bool CMotionToto::EventFrame(const Event &event)
{
    Math::Matrix*       mat;
    Math::Vector        eye, lookat, dir, perp, nPos, aPos, pos, speed;
    Math::Vector        vibLin, vibCir, dirSpeed, aAntenna;
    Math::Point         dim;
    Math::IntPoint           wDim;
    Gfx::ParticleType   type;
    float           progress, focus, distance, shift, verti, level, zoom;
    float           aAngle, nAngle, mAngle, angle, linSpeed, cirSpeed;
    int             sheet, i, r;
    bool            bHidden;

    if ( m_engine->GetPause() &&
         !m_main->GetInfoLock() )  return true;

    if ( m_bDisplayInfo )  // "looks" mouse?
    {
        bHidden = false;
    }
    else
    {
        bHidden = false;

        if ( m_main->GetMovieLock() )  // current movie?
        {
            bHidden = true;
        }
    }

    if ( bHidden )
    {
        nPos = m_object->GetPosition();
        m_terrain->AdjustToFloor(nPos, true);
        nPos.y -= 100.0f;  // hidden under the ground!
        m_object->SetPosition(nPos);
        return true;
    }

    m_time += event.rTime;
    m_blinkTime -= event.rTime;

    progress = 0.0f;
    if ( m_actionType != -1 )  // current action?
    {
        if ( m_progress < 0.15f )
        {
            progress = m_progress/0.15f;
        }
        else if ( m_progress < 0.85f )
        {
            progress = 1.0f;
        }
        else
        {
            progress = (1.0f-m_progress)/0.15f;
        }
    }

    if ( m_progress >= 1.0f )
    {
        m_actionType = -1;  // action ended
        m_actionTime = 0.0f;
        m_progress   = 0.0f;

        m_clownTime  = 0.0f;
        m_clownDelay = 0.0f;
    }

    focus  = m_engine->GetFocus();
    eye    = m_engine->GetEyePt();
    lookat = m_engine->GetLookatPt();

    vibLin   = Math::Vector(0.0f, 0.0f, 0.0f);
    vibCir   = Math::Vector(0.0f, 0.0f, 0.0f);
    aAntenna = Math::Vector(0.0f, 0.0f, 0.0f);
    aAntenna.x += 30.0f*Math::PI/180.0f;

    // Calculates the new position.
    if ( m_bDisplayInfo )
    {
        wDim = m_engine->GetWindowSize();
        nPos.x = -4.0f*(static_cast< float >(wDim.x)/static_cast< float >(wDim.y))/(640.0f/480.0f);
        nPos.y = -0.5f;
        nPos.z =  7.0f;  // in the left margin

        linSpeed = 0.0f;
    }
    else
    {
        distance = 30.0f-progress*18.0f;  // remoteness
        shift    = 18.0f-progress*11.0f;  // shift is left
        verti    = 10.0f-progress* 8.0f;  // shift at the top

        CObject* selected = m_main->GetSelect();
        ObjectType type = selected != nullptr ? selected->GetType() : OBJECT_NULL;

        if ( m_actionType == -1 &&
             (type == OBJECT_HUMAN    ||
              type == OBJECT_TECH     ||
              type == OBJECT_MOBILEwa ||
              type == OBJECT_MOBILEta ||
              type == OBJECT_MOBILEfa ||
              type == OBJECT_MOBILEia ||
              type == OBJECT_MOBILEwc ||
              type == OBJECT_MOBILEtc ||
              type == OBJECT_MOBILEfc ||
              type == OBJECT_MOBILEic ||
              type == OBJECT_MOBILEwi ||
              type == OBJECT_MOBILEti ||
              type == OBJECT_MOBILEfi ||
              type == OBJECT_MOBILEii ||
              type == OBJECT_MOBILEws ||
              type == OBJECT_MOBILEts ||
              type == OBJECT_MOBILEfs ||
              type == OBJECT_MOBILEis ||
              type == OBJECT_MOBILErt ||
              type == OBJECT_MOBILErc ||
              type == OBJECT_MOBILErr ||
              type == OBJECT_MOBILErs ||
              type == OBJECT_MOBILEsa ||
              type == OBJECT_MOBILEwt ||
              type == OBJECT_MOBILEtt ||
              type == OBJECT_MOBILEft ||
              type == OBJECT_MOBILEit ||
              type == OBJECT_MOBILEdr ) )  // vehicle?
        {
            m_clownTime += event.rTime;
            if ( m_clownTime >= m_clownDelay )
            {
                if ( rand()%10 < 2 )
                {
                    m_clownRadius = 2.0f+Math::Rand()*10.0f;
//?                 m_clownDelay  = m_clownRadius/(2.0f+Math::Rand()*2.0f);
                    m_clownDelay  = 1.5f+Math::Rand()*1.0f;
                }
                else
                {
                    m_clownRadius = 0.0f;
                    m_clownDelay  = 2.0f+Math::Rand()*2.0f;
                }
                pos = m_object->GetPosition();
                if ( pos.y < m_water->GetLevel() )  // underwater?
                {
                    m_clownRadius /= 1.5f;
                    m_clownDelay  *= 2.0f;
                }
                m_clownTime   = 0.0f;
            }
            else
            {
                distance -=               m_clownRadius*sinf(m_clownTime*Math::PI*2.0f/m_clownDelay);
                shift    -= m_clownRadius-m_clownRadius*cosf(m_clownTime*Math::PI*2.0f/m_clownDelay);
            }

            verti += (18.0f-shift)*0.2f;
        }

        distance /= focus;
//?     shift    *= focus;
        verti    /= focus;

        dir = Normalize(lookat-eye);
        nPos = eye + dir*distance;

        perp.x = -dir.z;
        perp.y =  dir.y;
        perp.z =  dir.x;
        nPos = nPos + perp*shift;

        nPos.y += verti;

        if ( m_bQuickPos )  // immediately in place?
        {
            m_bQuickPos = false;
            linSpeed = 0.0f;
        }
        else
        {
            aPos = m_object->GetPosition();
            if ( m_actionType == -1 )
            {
                level = 4.0f;
            }
            else
            {
                if ( m_bStartAction )
                {
                    m_bStartAction = false;
                    m_speedAction = Math::Distance(nPos, aPos)/15.0f;
                    if ( m_speedAction < 20.0f )  m_speedAction = 20.0f;
                }
                level = m_speedAction;
            }
            if ( level > 1.0f/event.rTime )  level = 1.0f/event.rTime;
            nPos = aPos + (nPos-aPos)*event.rTime*level;  // progression aPos -> nPos

            linSpeed = Math::DistanceProjected(nPos, aPos)/event.rTime;
            dirSpeed = (nPos-aPos)/event.rTime;
            nPos.y -= linSpeed*0.015f*(1.0f-progress);  // at ground level if moving fast
        }
    }

    // Calculate the new angle.
    nAngle = Math::NormAngle(Math::RotateAngle(eye.x-lookat.x, lookat.z-eye.z)-0.9f);
    if ( linSpeed == 0.0f || m_actionType != -1 )
    {
        mAngle = nAngle;
    }
    else
    {
        mAngle = Math::NormAngle(Math::RotateAngle(dirSpeed.x, -dirSpeed.z));
    }
    level = Math::Min(linSpeed*0.1f, 1.0f);
    nAngle = nAngle*(1.0f-level) + mAngle*level;
    aAngle = Math::NormAngle(m_object->GetRotationY());

    if ( nAngle < aAngle )
    {
        if ( nAngle+Math::PI*2.0f-aAngle < aAngle-nAngle )  nAngle += Math::PI*2.0f;
    }
    else
    {
        if ( aAngle+Math::PI*2.0f-nAngle < nAngle-aAngle )  aAngle += Math::PI*2.0f;
    }
    nAngle = aAngle + (nAngle-aAngle)*event.rTime*4.0f;

    // Leans quotes if running.
    cirSpeed = (aAngle-nAngle)/event.rTime;
    angle = cirSpeed*0.3f*(1.0f-progress);
    if ( angle >  0.7f )  angle =  0.7f;
    if ( angle < -0.7f )  angle = -0.7f;
    vibCir.x += angle*1.5f;
    aAntenna.x += fabs(angle)*0.8f;  // deviates

    // Leans forward so quickly advance.
    angle = linSpeed*0.10f*(1.0f-progress);
    if ( angle > 1.0f )  angle = 1.0f;
    vibCir.z -= angle/2.0f;  // leans forward
    aAntenna.z -= angle;  // leans forward

    // Calculates the residual motion.
    vibLin.y += (sinf(m_time*2.00f)*0.5f+
                 sinf(m_time*2.11f)*0.2f)*(1.0f-progress);

    vibCir.z += sinf(m_time*Math::PI* 2.01f)*(Math::PI/ 75.0f)+
                sinf(m_time*Math::PI* 2.51f)*(Math::PI/100.0f)+
                sinf(m_time*Math::PI*19.01f)*(Math::PI/200.0f);

    vibCir.x += sinf(m_time*Math::PI* 2.03f)*(Math::PI/ 75.0f)+
                sinf(m_time*Math::PI* 2.52f)*(Math::PI/100.0f)+
                sinf(m_time*Math::PI*19.53f)*(Math::PI/200.0f);

    vibCir.y += (sinf(m_time*Math::PI* 1.07f)*(Math::PI/ 10.0f)+
                 sinf(m_time*Math::PI* 1.19f)*(Math::PI/ 17.0f)+
                 sinf(m_time*Math::PI* 1.57f)*(Math::PI/ 31.0f))*(1.0f-progress);

    // Calculates the animations in action.
    if ( m_actionType == MT_ERROR )  // no-no?
    {
        vibCir.y += progress*sinf(m_progress*Math::PI*11.0f)*1.0f;
        vibCir.z -= progress*0.5f;  // leans forward

        aAntenna.x -= progress*0.4f;  // narrows
        aAntenna.z += progress*1.0f;  // leaning back
    }

    if ( m_actionType == MT_WARNING )  // warning?
    {
        vibCir.x += progress*sinf(m_progress*Math::PI*17.0f)*0.5f;

        aAntenna.x += progress*sinf(m_progress*Math::PI*17.0f)*0.5f;  // deviates
        aAntenna.z += progress*cosf(m_progress*Math::PI*17.0f)*0.5f;  // turns
    }

    if ( m_actionType == MT_INFO )  // yes-yes?
    {
        vibCir.z += progress*sinf(m_progress*Math::PI*19.0f)*0.7f;

        aAntenna.x -= progress*0.2f;  // narrows
        aAntenna.z -= progress*cosf(m_progress*Math::PI*19.0f)*0.9f;  // turns
    }

    if ( m_actionType == MT_MESSAGE )  // message?
    {
        vibCir.x += progress*sinf(m_progress*Math::PI*15.0f)*0.3f;
        vibCir.z += progress*cosf(m_progress*Math::PI*15.0f)*0.3f;

        aAntenna.x -= progress*0.4f;  // narrows
        aAntenna.z -= progress*cosf(m_progress*Math::PI*19.0f)*0.8f;
    }

    // Initialize the object.
    if ( m_bDisplayInfo )  // "looks" mouse?
    {
        if ( m_mousePos.x < 0.15f )
        {
            progress = 1.0f-m_mousePos.x/0.15f;
            vibCir.y += progress*Math::PI/2.0f;
        }
        else
        {
            progress = (m_mousePos.x-0.15f)/0.85f;
            vibCir.y -= progress*Math::PI/3.0f;
        }

        angle = Math::RotateAngle(m_mousePos.x-0.1f, m_mousePos.y-0.5f-vibLin.y*0.2f);
        if ( angle < Math::PI )
        {
            if ( angle > Math::PI*0.5f )  angle = Math::PI-angle;
            if ( angle > Math::PI*0.3f )  angle = Math::PI*0.3f;
            vibCir.z += angle;
        }
        else
        {
            angle = Math::PI*2.0f-angle;
            if ( angle > Math::PI*0.5f )  angle = Math::PI-angle;
            if ( angle > Math::PI*0.3f )  angle = Math::PI*0.3f;
            vibCir.z -= angle;
        }
    }
    else
    {
        nPos.y += vibLin.y;
        level = m_terrain->GetFloorLevel(nPos);
        if ( nPos.y < level+2.0f )
        {
            nPos.y = level+2.0f;  // just above the ground
        }
        nPos.y -= vibLin.y;
    }
    m_object->SetPosition(nPos);
    m_object->SetRotationY(nAngle);

    SetLinVibration(vibLin);
    SetCirVibration(vibCir);

    // Calculates the residual movement of the antennas.
    pos = aAntenna*0.40f;
    pos.x += sinf(m_time*Math::PI*2.07f)*(Math::PI/50.0f)+
             sinf(m_time*Math::PI*2.59f)*(Math::PI/70.0f)+
             sinf(m_time*Math::PI*2.67f)*(Math::PI/90.0f);

    pos.y += sinf(m_time*Math::PI*2.22f)*(Math::PI/50.0f)+
             sinf(m_time*Math::PI*2.36f)*(Math::PI/70.0f)+
             sinf(m_time*Math::PI*3.01f)*(Math::PI/90.0f);

    pos.z += sinf(m_time*Math::PI*2.11f)*(Math::PI/50.0f)+
             sinf(m_time*Math::PI*2.83f)*(Math::PI/70.0f)+
             sinf(m_time*Math::PI*3.09f)*(Math::PI/90.0f);

    m_object->SetPartRotation(4, pos);  // left antenna
    m_object->SetPartRotation(5, pos);  // left antenna
    m_object->SetPartRotation(6, pos);  // left antenna

    pos = aAntenna*0.40f;
    pos.x = -pos.x;
    pos.x += sinf(m_time*Math::PI*2.33f)*(Math::PI/50.0f)+
             sinf(m_time*Math::PI*2.19f)*(Math::PI/70.0f)+
             sinf(m_time*Math::PI*2.07f)*(Math::PI/90.0f);

    pos.y += sinf(m_time*Math::PI*2.44f)*(Math::PI/50.0f)+
             sinf(m_time*Math::PI*2.77f)*(Math::PI/70.0f)+
             sinf(m_time*Math::PI*3.22f)*(Math::PI/90.0f);

    pos.z += sinf(m_time*Math::PI*2.05f)*(Math::PI/50.0f)+
             sinf(m_time*Math::PI*2.38f)*(Math::PI/70.0f)+
             sinf(m_time*Math::PI*2.79f)*(Math::PI/90.0f);

    m_object->SetPartRotation(7, pos);  // right antenna
    m_object->SetPartRotation(8, pos);  // right antenna
    m_object->SetPartRotation(9, pos);  // right antenna

    // Movement of the mouth.
    if ( m_actionType == MT_ERROR )  // no-no?
    {
        m_object->SetPartRotationX(1, 0.0f);
        m_object->SetPartRotationZ(1, 0.2f+sinf(m_time*10.0f)*0.2f);
        m_object->SetPartScaleY(1, 2.0f+sinf(m_time*10.0f));
        m_object->SetPartScaleZ(1, 1.0f);
    }
    else if ( m_actionType == MT_WARNING )  // warning?
    {
        m_object->SetPartRotationX(1, 15.0f*Math::PI/180.0f);
        m_object->SetPartRotationZ(1, 0.0f);
        m_object->SetPartScaleY(1, 1.0f);
        m_object->SetPartScaleZ(1, 1.0f);
    }
    else if ( m_actionType == MT_INFO )  // yes-yes?
    {
        m_object->SetPartRotationX(1, 0.0f);
        m_object->SetPartRotationZ(1, 0.0f);
        m_object->SetPartScaleY(1, 1.0f);
        m_object->SetPartScaleZ(1, 0.7f+sinf(m_time*10.0f)*0.3f);
    }
    else if ( m_actionType == MT_MESSAGE )  // message?
    {
        m_object->SetPartRotationX(1, 0.0f);
        m_object->SetPartRotationZ(1, 0.0f);
        m_object->SetPartScaleY(1, 1.0f);
        m_object->SetPartScaleZ(1, 0.8f+sinf(m_time*7.0f)*0.2f);
    }
    else
    {
        m_object->SetPartRotationX(1, 0.0f);
        m_object->SetPartRotationZ(1, 0.0f);
        m_object->SetPartScaleY(1, 1.0f);
        m_object->SetPartScaleZ(1, 1.0f);
    }

    // Eye blinking management.
    if ( m_blinkTime <= 0.0f && m_blinkProgress == -1.0f )
    {
        m_blinkProgress = 0.0f;
    }

    if ( m_blinkProgress >= 0.0f )
    {
        m_blinkProgress += event.rTime*3.2f;

        if ( m_blinkProgress < 1.0f )
        {
            if ( m_blinkProgress < 0.5f )  zoom = m_blinkProgress/0.5f;
            else                           zoom = 2.0f-m_blinkProgress/0.5f;
            m_object->SetPartScaleY(2, 1.0f-zoom*0.9f);
            m_object->SetPartScaleY(3, 1.0f-zoom*0.9f);
        }
        else
        {
            m_blinkProgress = -1.0f;
            m_blinkTime = 0.1f+Math::Rand()*4.0f;
            m_object->SetPartScaleY(2, 1.0f);
            m_object->SetPartScaleY(3, 1.0f);
        }
    }

    if ( m_actionType == MT_ERROR )  // no-no?
    {
        m_object->SetPartRotationX(2, -30.0f*Math::PI/180.0f);
        m_object->SetPartRotationX(3,  30.0f*Math::PI/180.0f);
    }
    else if ( m_actionType == MT_WARNING )  // warning?
    {
        m_object->SetPartRotationX(2, -15.0f*Math::PI/180.0f);
        m_object->SetPartRotationX(3,  15.0f*Math::PI/180.0f);
    }
    else if ( m_actionType == MT_INFO )  // yes-yes?
    {
        m_object->SetPartRotationX(2,  40.0f*Math::PI/180.0f);
        m_object->SetPartRotationX(3, -40.0f*Math::PI/180.0f);
    }
    else if ( m_actionType == MT_MESSAGE )  // message?
    {
        m_object->SetPartRotationX(2,  20.0f*Math::PI/180.0f);
        m_object->SetPartRotationX(3, -20.0f*Math::PI/180.0f);
    }
    else
    {
        m_object->SetPartRotationX(2, 0.0f);
        m_object->SetPartRotationX(3, 0.0f);
    }

    mat = m_object->GetWorldMatrix(0);  // must be done every time!

    // Generates particles.
    if ( m_time-m_lastMotorParticle >= m_engine->ParticleAdapt(0.05f) )
    {
        m_lastMotorParticle = m_time;

        if ( m_bDisplayInfo )  sheet = Gfx::SH_FRONT;
        else                   sheet = Gfx::SH_WORLD;

        pos = m_object->GetPosition();
        if ( !m_bDisplayInfo             &&
             pos.y < m_water->GetLevel() )  // underwater?
        {
            float t = Math::Mod(m_time, 3.5f);
            if ( t >= 2.2f || ( t >= 1.2f && t <= 1.4f ) )  // breathe?
            {
                pos = Math::Vector(1.0f, 0.2f, 0.0f);
                pos.z += (Math::Rand()-0.5f)*0.5f;

                speed = pos;
                speed.y += 5.0f+Math::Rand()*5.0f;
                speed.x += Math::Rand()*2.0f;
                speed.z += (Math::Rand()-0.5f)*2.0f;

                pos   = Transform(*mat, pos);
                speed = Transform(*mat, speed)-pos;

                dim.x = 0.12f;
                dim.y = dim.x;
                m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIBUBBLE, 3.0f, 0.0f, 0.0f);
            }
        }
        else    // out of water?
        {
            pos = Math::Vector(0.0f, -0.5f, 0.0f);
            pos.z += (Math::Rand()-0.5f)*0.5f;

            speed = pos;
            speed.y -= (1.5f+Math::Rand()*1.5f) + vibLin.y;
            speed.x += (Math::Rand()-0.5f)*2.0f;
            speed.z += (Math::Rand()-0.5f)*2.0f;

//          mat = m_object->GetWorldMatrix(0);
            pos   = Transform(*mat, pos);
            speed = Transform(*mat, speed)-pos;

            dim.x = (Math::Rand()*0.4f+0.4f)*(1.0f+Math::Min(linSpeed*0.1f, 5.0f));
            dim.y = dim.x;
            m_particle->CreateParticle(pos, speed, dim, Gfx::PARTITOTO, 1.0f+Math::Rand()*1.0f, 0.0f, 1.0f, sheet);
        }

        if ( m_actionType != -1  &&  // current action?
             m_progress <= 0.85f )
        {
            pos.x = (Math::Rand()-0.5f)*1.0f;
            pos.y = (Math::Rand()-0.5f)*1.0f+3.5f;
            pos.z = (Math::Rand()-0.5f)*1.0f;
            pos   = Transform(*mat, pos);
            speed = Math::Vector(0.0f, 0.0f, 0.0f);
            dim.x = (Math::Rand()*0.3f+0.3f);
            dim.y = dim.x;
            if ( m_actionType == MT_ERROR   )  type = Gfx::PARTIERROR;
            if ( m_actionType == MT_WARNING )  type = Gfx::PARTIWARNING;
            if ( m_actionType == MT_INFO    )  type = Gfx::PARTIINFO;
            if ( m_actionType == MT_MESSAGE )  type = Gfx::PARTIWARNING;
            m_particle->CreateParticle(pos, speed, dim, type, 0.5f+Math::Rand()*0.5f, 0.0f, 1.0f, sheet);

            pos.x = 0.50f+(Math::Rand()-0.5f)*0.80f;
            pos.y = 0.86f+(Math::Rand()-0.5f)*0.08f;
            pos.z = 0.00f;
            dim.x = (Math::Rand()*0.04f+0.04f);
            dim.y = dim.x/0.75f;
            m_particle->CreateParticle(pos, speed, dim, type, 0.5f+Math::Rand()*0.5f, 0.0f, 1.0f, Gfx::SH_INTERFACE);
        }

//?     if ( m_bDisplayInfo && m_main->GetInterfaceGlint() )
        if ( false )
        {
            pos.x = (Math::Rand()-0.5f)*1.4f;
            pos.y = (Math::Rand()-0.5f)*1.4f+3.5f;
            pos.z = (Math::Rand()-0.5f)*1.4f;
            pos   = Transform(*mat, pos);
            speed = Math::Vector(0.0f, 0.0f, 0.0f);
            dim.x = (Math::Rand()*0.5f+0.5f);
            dim.y = dim.x;
            m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIERROR, 0.5f+Math::Rand()*0.5f, 0.0f, 1.0f, sheet);

            for ( i=0 ; i<10 ; i++ )
            {
                pos.x = 0.60f+(Math::Rand()-0.5f)*0.76f;
                pos.y = 0.47f+(Math::Rand()-0.5f)*0.90f;
                pos.z = 0.00f;
                r = rand()%4;
                     if ( r == 0 )  pos.x = 0.21f;  // the left edge
                else if ( r == 1 )  pos.x = 0.98f;  // the right edge
                else if ( r == 2 )  pos.y = 0.02f;  // on the lower edge
                else                pos.y = 0.92f;  // on the upper edge
                dim.x = (Math::Rand()*0.02f+0.02f);
                dim.y = dim.x/0.75f;
                m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIERROR, 0.5f+Math::Rand()*0.5f, 0.0f, 1.0f, Gfx::SH_INTERFACE);
            }
        }
    }

    // Move the sound.
    if ( m_soundChannel != -1 )
    {
        if ( !m_sound->Position(m_soundChannel, m_object->GetPosition()) )
        {
            m_soundChannel = -1;
        }
    }

    return true;
}


// Starts an action.

Error CMotionToto::SetAction(int action, float time)
{
    SoundType   sound;

    CMotion::SetAction(action, time);

    m_bStartAction = true;

    sound = SOUND_NONE;
    if ( action == MT_ERROR   )  sound = SOUND_ERROR;
    if ( action == MT_WARNING )  sound = SOUND_WARNING;
    if ( action == MT_INFO    )  sound = SOUND_INFO;
    if ( action == MT_MESSAGE )  sound = SOUND_MESSAGE;

    if ( sound != SOUND_NONE )
    {
        m_soundChannel = m_sound->Play(sound, m_object->GetPosition());
    }

    return ERR_OK;
}
