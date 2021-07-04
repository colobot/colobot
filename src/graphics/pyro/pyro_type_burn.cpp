/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2020, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include "graphics/engine/particle.h"
#include "graphics/engine/terrain.h"

#include "graphics/pyro/pyro.h"
#include "graphics/pyro/pyro_manager.h"

#include "level/robotmain.h"

#include "object/object.h"
#include "object/old_object.h"

#include "sound/sound.h"

using namespace Gfx;

CBurnPyro::CBurnPyro(CObject *obj)
    : CPyro(PT_OTHER, obj)
    , m_organicBurn(obj->GetType() == OBJECT_MOTHER ||
                    obj->GetType() == OBJECT_ANT    ||
                    obj->GetType() == OBJECT_SPIDER ||
                    obj->GetType() == OBJECT_BEE    ||
                    obj->GetType() == OBJECT_WORM   ||
                    obj->GetType() == OBJECT_BULLET)
{}

void CBurnPyro::AfterCreate()
{
    m_soundChannel = m_sound->Play(SOUND_BURN, m_pos, 1.0f, 1.0f, true);
    m_sound->AddEnvelope(m_soundChannel, 1.0f, 1.0f, 12.0f, SOPER_CONTINUE);
    m_sound->AddEnvelope(m_soundChannel, 0.0f, 1.0f,  5.0f, SOPER_STOP);

    if (m_organicBurn)
    {
        m_sound->Play(SOUND_DEADi, m_pos);
        m_sound->Play(SOUND_DEADi, m_engine->GetEyePt());
    }
    else
    {
        BurnStart();
    }

    m_speed = 1.0f/15.0f;

    LightOperAdd(0.00f, 0.0f,  2.0f,  1.0f,  0.0f);  // red-orange
    LightOperAdd(0.30f, 1.0f, -0.8f, -0.8f, -0.8f);  // dark gray
    LightOperAdd(0.80f, 1.0f, -0.8f, -0.8f, -0.8f);  // dark gray
    LightOperAdd(1.00f, 0.0f, -0.8f, -0.8f, -0.8f);  // dark gray
    CreateLight(m_pos, 40.0f);
}

void CBurnPyro::UpdateEffect()
{
    if ( m_lastParticle+m_engine->ParticleAdapt(0.05f) <= m_time )
    {
        m_lastParticle = m_time;

        float factor = m_size/25.0f;  // 1 = standard size

        Math::Vector pos = m_object->GetPosition();
        pos.y -= m_object->GetCharacter()->height;
        pos.x += (Math::Rand()-0.5f)*(4.0f+8.0f*m_progress)*factor;
        pos.z += (Math::Rand()-0.5f)*(4.0f+8.0f*m_progress)*factor;
        Math::Vector speed;
        speed.x = 0.0f;
        speed.z = 0.0f;
        speed.y = 0.0f;
        Math::Point dim;
        dim.x = (Math::Rand()*2.5f+1.0f)*factor;
        dim.y = dim.x;
        m_particle->CreateParticle(pos, speed, dim, PARTIFLAME, 2.0f, 0.0f, 0.2f);

        pos = m_object->GetPosition();
        pos.y -= m_object->GetCharacter()->height;
        pos.x += (Math::Rand()-0.5f)*(2.0f+4.0f*m_progress)*factor;
        pos.z += (Math::Rand()-0.5f)*(2.0f+4.0f*m_progress)*factor;
        speed.x = 0.0f;
        speed.z = 0.0f;
        speed.y = (Math::Rand()*5.0f*m_progress+3.0f)*factor;
        dim.x = (Math::Rand()*2.0f+1.0f)*factor;
        dim.y = dim.x;
        m_particle->CreateParticle(pos, speed, dim, PARTIFLAME, 2.0f, 0.0f, 0.2f);

        pos = m_object->GetPosition();
        pos.y -= 2.0f;
        pos.x += (Math::Rand()-0.5f)*5.0f*factor;
        pos.z += (Math::Rand()-0.5f)*5.0f*factor;
        speed.x = 0.0f;
        speed.z = 0.0f;
        speed.y = (6.0f+Math::Rand()*6.0f+m_progress*6.0f)*factor;
        dim.x = (Math::Rand()*1.5f+1.0f+m_progress*3.0f)*factor;
        dim.y = dim.x;
        m_particle->CreateParticle(pos, speed, dim, PARTISMOKE3, 4.0f);
    }

    if ( !m_organicBurn )
    {
        BurnProgress();
    }
    else
    {
        Math::Vector speed;
        speed.y = 0.0f;
        speed.x = (Math::Rand()-0.5f)*m_progress*1.0f;
        speed.z = (Math::Rand()-0.5f)*m_progress*1.0f;
        if ( m_progress > 0.8f )
        {
            float prog = (m_progress-0.8f)/0.2f;  // 0..1
            speed.y = -prog*6.0f;  // sinks into the ground
            m_object->SetScale(1.0f-prog*0.5f);
        }
        m_object->SetLinVibration(speed);
    }
}

void CBurnPyro::BurnStart()
{
    m_burnType = m_object->GetType();

    Math::Vector oPos = m_object->GetPosition();
    m_burnFall = m_terrain->GetHeightToFloor(oPos, true);

    m_object->Simplify();
    m_object->SetLock(true);  // ruin not usable yet

    if ( m_object->Implements(ObjectInterfaceType::Controllable) && dynamic_cast<CControllableObject&>(*m_object).GetSelect() )
    {
        dynamic_cast<CControllableObject&>(*m_object).SetSelect(false);  // deselects the object
        m_camera->SetType(CAM_TYPE_EXPLO);
        m_main->DeselectAll();
    }
    m_main->RemoveFromSelectionHistory(m_object);

    for (int i = 0; i < OBJECTMAXPART; i++)
    {
        int objRank = m_object->GetObjectRank(i);
        if (objRank == -1) continue;

        // TODO: refactor later to material change
        int oldBaseObjRank = m_engine->GetObjectBaseRank(objRank);
        if (oldBaseObjRank != -1)
        {
            int newBaseObjRank = m_engine->CreateBaseObject();
            m_engine->CopyBaseObject(oldBaseObjRank, newBaseObjRank);
            m_engine->SetObjectBaseRank(objRank, newBaseObjRank);

            m_engine->ChangeSecondTexture(objRank, "dirty04.png");
        }
    }
    m_engine->LoadTexture("textures/dirty04.png");

    m_burnPartTotal = 0;

    Math::Vector pos, angle;

    if ( m_burnType == OBJECT_DERRICK  ||
         m_burnType == OBJECT_FACTORY  ||
         m_burnType == OBJECT_REPAIR   ||
         m_burnType == OBJECT_DESTROYER||
         m_burnType == OBJECT_CONVERT  ||
         m_burnType == OBJECT_TOWER    ||
         m_burnType == OBJECT_RESEARCH ||
         m_burnType == OBJECT_ENERGY   ||
         m_burnType == OBJECT_LABO     )
    {
        pos.x =   0.0f;
        pos.y = -(4.0f+Math::Rand()*4.0f);
        pos.z =   0.0f;
        angle.x = (Math::Rand()-0.5f)*0.4f;
        angle.y = 0.0f;
        angle.z = (Math::Rand()-0.5f)*0.4f;
    }
    else if ( m_burnType == OBJECT_STATION ||
              m_burnType == OBJECT_RADAR   ||
              m_burnType == OBJECT_INFO    )
    {
        pos.x =   0.0f;
        pos.y = -(1.0f+Math::Rand()*1.0f);
        pos.z =   0.0f;
        angle.x = (Math::Rand()-0.5f)*0.2f;
        angle.y = 0.0f;
        angle.z = (Math::Rand()-0.5f)*0.2f;
    }
    else if ( m_burnType == OBJECT_NUCLEAR )
    {
        pos.x =   0.0f;
        pos.y = -(10.0f+Math::Rand()*10.0f);
        pos.z =   0.0f;
        angle.x = (Math::Rand()-0.5f)*0.4f;
        angle.y = 0.0f;
        angle.z = (Math::Rand()-0.5f)*0.4f;
    }
    else if ( m_burnType == OBJECT_PARA )
    {
        pos.x =   0.0f;
        pos.y = -(10.0f+Math::Rand()*10.0f);
        pos.z =   0.0f;
        angle.x = (Math::Rand()-0.5f)*0.4f;
        angle.y = 0.0f;
        angle.z = (Math::Rand()-0.5f)*0.4f;
    }
    else if ( m_burnType == OBJECT_SAFE )
    {
        pos.x =   0.0f;
        pos.y = -(10.0f+Math::Rand()*10.0f);
        pos.z =   0.0f;
        angle.x = (Math::Rand()-0.5f)*0.4f;
        angle.y = 0.0f;
        angle.z = (Math::Rand()-0.5f)*0.4f;
    }
    else if ( m_burnType == OBJECT_HUSTON )
    {
        pos.x =   0.0f;
        pos.y = -(10.0f+Math::Rand()*10.0f);
        pos.z =   0.0f;
        angle.x = (Math::Rand()-0.5f)*0.4f;
        angle.y = 0.0f;
        angle.z = (Math::Rand()-0.5f)*0.4f;
    }
    else if ( m_burnType == OBJECT_MOBILEwa ||
              m_burnType == OBJECT_MOBILEwb ||
              m_burnType == OBJECT_MOBILEwc ||
              m_burnType == OBJECT_MOBILEwi ||
              m_burnType == OBJECT_MOBILEws ||
              m_burnType == OBJECT_MOBILEwt )
    {
        pos.x =   0.0f;
        pos.y = -(0.5f+Math::Rand()*1.0f);
        pos.z =   0.0f;
        angle.x = (Math::Rand()-0.5f)*0.8f;
        angle.y = 0.0f;
        angle.z = (Math::Rand()-0.5f)*0.4f;
    }
    else if ( m_burnType == OBJECT_TEEN31 )  // basket?
    {
        pos.x =   0.0f;
        pos.y =   0.0f;
        pos.z =   0.0f;
        angle.x = (Math::Rand()-0.5f)*0.8f;
        angle.y = 0.0f;
        angle.z = (Math::Rand()-0.5f)*0.2f;
    }
    else
    {
        pos.x =   0.0f;
        pos.y = -(2.0f+Math::Rand()*2.0f);
        pos.z =   0.0f;
        angle.x = (Math::Rand()-0.5f)*0.8f;
        angle.y = 0.0f;
        angle.z = (Math::Rand()-0.5f)*0.8f;
    }
    BurnAddPart(0, pos, angle);  // movement of the main part

    m_burnKeepPart[0] = -1;  // nothing to keep

    if ( m_burnType == OBJECT_DERRICK )
    {
        pos.x =   0.0f;
        pos.y = -40.0f;
        pos.z =   0.0f;
        angle.x = 0.0f;
        angle.y = 0.0f;
        angle.z = 0.0f;
        BurnAddPart(1, pos, angle);  // down the drill
    }

    if ( m_burnType == OBJECT_REPAIR )
    {
        pos.x =   0.0f;
        pos.y = -12.0f;
        pos.z =   0.0f;
        angle.x = (Math::Rand()-0.5f)*0.2f;
        angle.y = (Math::Rand()-0.5f)*0.2f;
        angle.z = -90.0f*Math::PI/180.0f;
        BurnAddPart(1, pos, angle);  // down the sensor
    }

    if ( m_burnType == OBJECT_DESTROYER )
    {
        pos.x =   0.0f;
        pos.y = -12.0f;
        pos.z =   0.0f;
        angle.x = (Math::Rand()-0.5f)*0.2f;
        angle.y = (Math::Rand()-0.5f)*0.2f;
        angle.z = -90.0f*Math::PI/180.0f;
        BurnAddPart(1, pos, angle);  // down the sensor
    }

    if ( m_burnType == OBJECT_CONVERT )
    {
        pos.x =    0.0f;
        pos.y = -200.0f;
        pos.z =    0.0f;
        angle.x = (Math::Rand()-0.5f)*0.5f;
        angle.y = (Math::Rand()-0.5f)*0.5f;
        angle.z = 0.0f;
        BurnAddPart(1, pos, angle);  // down the cover
        BurnAddPart(2, pos, angle);
        BurnAddPart(3, pos, angle);
    }

    if ( m_burnType == OBJECT_TOWER )
    {
        pos.x =  0.0f;
        pos.y = -7.0f;
        pos.z =  0.0f;
        angle.x = (Math::Rand()-0.5f)*0.4f;
        angle.y = (Math::Rand()-0.5f)*0.4f;
        angle.z = 0.0f;
        BurnAddPart(1, pos, angle);  // down the cannon
    }

    if ( m_burnType == OBJECT_RESEARCH )
    {
        pos.x =  0.0f;
        pos.y = -7.0f;
        pos.z =  0.0f;
        angle.x = (Math::Rand()-0.5f)*0.2f;
        angle.y = (Math::Rand()-0.5f)*0.2f;
        angle.z = 0.0f;
        BurnAddPart(1, pos, angle);  // down the anemometer
    }

    if ( m_burnType == OBJECT_RADAR )
    {
        pos.x =   0.0f;
        pos.y = -14.0f;
        pos.z =   0.0f;
        angle.x = (Math::Rand()-0.5f)*0.4f;
        angle.y = (Math::Rand()-0.5f)*0.4f;
        angle.z = 0.0f;
        BurnAddPart(1, pos, angle);  // down the radar
        BurnAddPart(2, pos, angle);
    }

    if ( m_burnType == OBJECT_INFO )
    {
        pos.x =   0.0f;
        pos.y = -14.0f;
        pos.z =   0.0f;
        angle.x = (Math::Rand()-0.5f)*0.4f;
        angle.y = (Math::Rand()-0.5f)*0.4f;
        angle.z = 0.0f;
        BurnAddPart(1, pos, angle);  // down the information terminal
        BurnAddPart(2, pos, angle);
    }

    if ( m_burnType == OBJECT_LABO )
    {
        pos.x =   0.0f;
        pos.y = -12.0f;
        pos.z =   0.0f;
        angle.x = 0.0f;
        angle.y = 0.0f;
        angle.z = 0.0f;
        BurnAddPart(1, pos, angle);  // down the arm
    }

    if ( m_burnType == OBJECT_NUCLEAR )
    {
        pos.x = 0.0f;
        pos.y = 0.0f;
        pos.z = 0.0f;
        angle.x = 0.0f;
        angle.y = 0.0f;
        angle.z = -135.0f*Math::PI/180.0f;
        BurnAddPart(1, pos, angle);  // down the cover
    }

    if ( m_burnType == OBJECT_MOBILEfa ||
         m_burnType == OBJECT_MOBILEta ||
         m_burnType == OBJECT_MOBILEwa ||
         m_burnType == OBJECT_MOBILEia )
    {
        pos.x =  2.0f;
        pos.y = -5.0f;
        pos.z =  0.0f;
        angle.x = (Math::Rand()-0.5f)*0.2f;
        angle.y = (Math::Rand()-0.5f)*0.2f;
        angle.z = 40.0f*Math::PI/180.0f;
        BurnAddPart(1, pos, angle);  // down the arm
    }

    if ( m_burnType == OBJECT_MOBILEfs ||
         m_burnType == OBJECT_MOBILEts ||
         m_burnType == OBJECT_MOBILEws ||
         m_burnType == OBJECT_MOBILEis )
    {
        pos.x =  0.0f;
        pos.y = -7.0f;
        pos.z =  0.0f;
        angle.x = (Math::Rand()-0.5f)*0.2f;
        angle.y = (Math::Rand()-0.5f)*0.2f;
        angle.z = 50.0f*Math::PI/180.0f;
        BurnAddPart(1, pos, angle);  // down the sensor
    }

    if ( m_burnType == OBJECT_MOBILEfc ||
         m_burnType == OBJECT_MOBILEtc ||
         m_burnType == OBJECT_MOBILEwc ||
         m_burnType == OBJECT_MOBILEic )
    {
        pos.x = -1.5f;
        pos.y = -5.0f;
        pos.z =  0.0f;
        angle.x = (Math::Rand()-0.5f)*0.2f;
        angle.y = (Math::Rand()-0.5f)*0.2f;
        angle.z = -25.0f*Math::PI/180.0f;
        BurnAddPart(1, pos, angle);  // down the cannon
    }

    if ( m_burnType == OBJECT_MOBILEfi ||
         m_burnType == OBJECT_MOBILEti ||
         m_burnType == OBJECT_MOBILEwi ||
         m_burnType == OBJECT_MOBILEii )
    {
        pos.x = -1.5f;
        pos.y = -5.0f;
        pos.z =  0.0f;
        angle.x = (Math::Rand()-0.5f)*0.2f;
        angle.y = (Math::Rand()-0.5f)*0.2f;
        angle.z = -25.0f*Math::PI/180.0f;
        BurnAddPart(1, pos, angle);  // down the insect-cannon
    }

    if ( m_burnType == OBJECT_MOBILEfb ||
         m_burnType == OBJECT_MOBILEtb ||
         m_burnType == OBJECT_MOBILEwb ||
         m_burnType == OBJECT_MOBILEib )
    {
        pos.x = -1.5f;
        pos.y = -5.0f;
        pos.z =  0.0f;
        angle.x = (Math::Rand()-0.5f)*0.2f;
        angle.y = (Math::Rand()-0.5f)*0.2f;
        angle.z = -25.0f*Math::PI/180.0f;
        BurnAddPart(1, pos, angle);  // down the neutron gun
    }

    if ( m_burnType == OBJECT_MOBILErt ||
         m_burnType == OBJECT_MOBILErc )
    {
        pos.x =   0.0f;
        pos.y = -10.0f;
        pos.z =   0.0f;
        angle.x = 0.0f;
        angle.y = 0.0f;
        angle.z = 0.0f;
        BurnAddPart(1, pos, angle);  // down the holder

        pos.x =   0.0f;
        pos.y = -10.0f;
        pos.z =   0.0f;
        angle.x = 0.0f;
        angle.y = 0.0f;
        angle.z = 0.0f;
        BurnAddPart(2, pos, angle);  // down the pestle/cannon
    }

    if ( m_burnType == OBJECT_MOBILErr )
    {
        pos.x =   0.0f;
        pos.y = -10.0f;
        pos.z =   0.0f;
        angle.x = 0.0f;
        angle.y = 0.0f;
        angle.z = 0.0f;
        BurnAddPart(1, pos, angle);  // down the holder

        pos.x =   0.0f;
        pos.y =   0.0f;
        pos.z =   0.0f;
        angle.x = 0.0f;
        angle.y = 0.0f;
        angle.z = -Math::PI/2.0f;
        BurnAddPart(4, pos, angle);

        pos.x =   0.0f;
        pos.y =   0.0f;
        pos.z =   0.0f;
        angle.x = 0.0f;
        angle.y = 0.0f;
        angle.z = Math::PI/2.5f;
        BurnAddPart(2, pos, angle);
    }

    if ( m_burnType == OBJECT_MOBILErs )
    {
        pos.x =   0.0f;
        pos.y = -10.0f;
        pos.z =   0.0f;
        angle.x = 0.0f;
        angle.y = 0.0f;
        angle.z = 0.0f;
        BurnAddPart(1, pos, angle);  // down the holder

        pos.x =   0.0f;
        pos.y =  -5.0f;
        pos.z =   0.0f;
        angle.x = 0.0f;
        angle.y = 0.0f;
        angle.z = 0.0f;
        BurnAddPart(2, pos, angle);

        pos.x =   0.0f;
        pos.y =  -5.0f;
        pos.z =   0.0f;
        angle.x = 0.0f;
        angle.y = 0.0f;
        angle.z = 0.0f;
        BurnAddPart(3, pos, angle);
    }

    if ( m_burnType == OBJECT_MOBILEsa )
    {
        pos.x =   0.0f;
        pos.y = -10.0f;
        pos.z =   0.0f;
        angle.x = 0.0f;
        angle.y = 0.0f;
        angle.z = 0.0f;
        BurnAddPart(1, pos, angle);  // down the holder
    }

    if ( m_burnType == OBJECT_MOBILEwa ||
         m_burnType == OBJECT_MOBILEwb ||
         m_burnType == OBJECT_MOBILEwc ||
         m_burnType == OBJECT_MOBILEwi ||
         m_burnType == OBJECT_MOBILEws ||
         m_burnType == OBJECT_MOBILEwt )  // wheels?
    {
        int i = 0;
        for (; i < 4; i++)
        {
            pos.x = 0.0f;
            pos.y = Math::Rand()*0.5f;
            pos.z = 0.0f;
            angle.x = (Math::Rand()-0.5f)*Math::PI/2.0f;
            angle.y = (Math::Rand()-0.5f)*Math::PI/2.0f;
            angle.z = 0.0f;
            BurnAddPart(6+i, pos, angle);  // wheel

            m_burnKeepPart[i] = 6+i;  // we keep the wheels
        }
        m_burnKeepPart[i] = -1;
    }

    if ( m_burnType == OBJECT_MOBILEta ||
         m_burnType == OBJECT_MOBILEtb ||
         m_burnType == OBJECT_MOBILEtc ||
         m_burnType == OBJECT_MOBILEti ||
         m_burnType == OBJECT_MOBILEts ||
         m_burnType == OBJECT_MOBILEtt ||
         m_burnType == OBJECT_MOBILErt ||
         m_burnType == OBJECT_MOBILErc ||
         m_burnType == OBJECT_MOBILErr ||
         m_burnType == OBJECT_MOBILErs ||
         m_burnType == OBJECT_MOBILErp ||
         m_burnType == OBJECT_MOBILEsa ||
         m_burnType == OBJECT_MOBILEst ||
         m_burnType == OBJECT_MOBILEdr )  // caterpillars?
    {
        pos.x =   0.0f;
        pos.y =  -4.0f;
        pos.z =   2.0f;
        angle.x = (Math::Rand()-0.5f)*20.0f*Math::PI/180.0f;
        angle.y = (Math::Rand()-0.5f)*10.0f*Math::PI/180.0f;
        angle.z = (Math::Rand()-0.5f)*30.0f*Math::PI/180.0f;
        BurnAddPart(6, pos, angle);  // down the right caterpillar

        pos.x =   0.0f;
        pos.y =  -4.0f;
        pos.z =  -2.0f;
        angle.x = (Math::Rand()-0.5f)*20.0f*Math::PI/180.0f;
        angle.y = (Math::Rand()-0.5f)*10.0f*Math::PI/180.0f;
        angle.z = (Math::Rand()-0.5f)*30.0f*Math::PI/180.0f;
        BurnAddPart(7, pos, angle);  // down the left caterpillar
    }

    if ( m_burnType == OBJECT_MOBILEfa ||
         m_burnType == OBJECT_MOBILEfb ||
         m_burnType == OBJECT_MOBILEfc ||
         m_burnType == OBJECT_MOBILEfi ||
         m_burnType == OBJECT_MOBILEfs ||
         m_burnType == OBJECT_MOBILEft )  // flying?
    {
        int i = 0;
        for (; i<3; i++)
        {
            pos.x =  0.0f;
            pos.y = -3.0f;
            pos.z =  0.0f;
            angle.x = 0.0f;
            angle.y = 0.0f;
            angle.z = (Math::Rand()-0.5f)*Math::PI/2.0f;
            BurnAddPart(6+i, pos, angle);  // foot
        }
        m_burnKeepPart[i] = -1;
    }

    if ( m_burnType == OBJECT_MOBILEia ||
         m_burnType == OBJECT_MOBILEib ||
         m_burnType == OBJECT_MOBILEic ||
         m_burnType == OBJECT_MOBILEii ||
         m_burnType == OBJECT_MOBILEis ||
         m_burnType == OBJECT_MOBILEit )  // legs?
    {
        for (int i = 0; i < 6; i++)
        {
            pos.x =  0.0f;
            pos.y = -3.0f;
            pos.z =  0.0f;
            angle.x = 0.0f;
            angle.y = (Math::Rand()-0.5f)*Math::PI/4.0f;
            angle.z = (Math::Rand()-0.5f)*Math::PI/4.0f;
            BurnAddPart(6+i, pos, angle);  // leg
        }
    }
}

void CBurnPyro::BurnAddPart(int part, Math::Vector pos, Math::Vector angle)
{
    // TODO: temporary hack (hopefully)
    assert(m_object->Implements(ObjectInterfaceType::Old));
    COldObject& oldObj = dynamic_cast<COldObject&>(*m_object);

    int i = m_burnPartTotal;
    m_burnPart[i].part = part;
    m_burnPart[i].initialPos = oldObj.GetPartPosition(part);
    m_burnPart[i].finalPos = m_burnPart[i].initialPos+pos;
    m_burnPart[i].initialAngle = oldObj.GetPartRotation(part);
    m_burnPart[i].finalAngle = m_burnPart[i].initialAngle+angle;

    m_burnPartTotal++;
}

void CBurnPyro::BurnProgress()
{
    if ( m_burnType == OBJECT_TEEN31 )  // basket?
    {
        m_object->SetScaleY(1.0f-m_progress*0.5f);  // slight flattening
    }

    for (int i = 0; i < m_burnPartTotal; i++)
    {
        Math::Vector pos = m_burnPart[i].initialPos + m_progress*(m_burnPart[i].finalPos-m_burnPart[i].initialPos);
        if ( i == 0 && m_burnFall > 0.0f )
        {
            float h = powf(m_progress, 2.0f)*1000.0f;
            if ( h > m_burnFall )  h = m_burnFall;
            pos.y -= h;
        }

        // TODO: temporary hack (hopefully)
        assert(m_object->Implements(ObjectInterfaceType::Old));
        COldObject& oldObj = dynamic_cast<COldObject&>(*m_object);

        oldObj.SetPartPosition(m_burnPart[i].part, pos);

        pos = m_burnPart[i].initialAngle + m_progress*(m_burnPart[i].finalAngle-m_burnPart[i].initialAngle);
        oldObj.SetPartRotation(m_burnPart[i].part, pos);
    }

    if (m_object->Implements(ObjectInterfaceType::Powered))
    {
        CObject* sub = dynamic_cast<CPoweredObject&>(*m_object).GetPower();
        if (sub != nullptr)  // is there a battery?
            sub->SetScaleY(1.0f - m_progress);  // complete flattening
    }
}

bool CBurnPyro::BurnIsKeepPart(int part)
{
    int i = 0;
    while (m_burnKeepPart[i] != -1)
    {
        if (part == m_burnKeepPart[i++]) return true;  // must keep
    }
    return false;  // must destroy
}

void CBurnPyro::AfterEnd()
{
    if (m_object == nullptr)
        return;

    if (m_organicBurn)  // organic object is burning?
    {
        DeleteObject(true, true);  // removes the insect
        return;
    }

    for (int i = 1; i < OBJECTMAXPART; i++)
    {
        int objRank = m_object->GetObjectRank(i);
        if (objRank == -1) continue;
        if (BurnIsKeepPart(i)) continue;

        m_object->DeletePart(i);
    }

    DeleteObject(false, true);  // destroys the object transported + the battery

    if ( m_burnType == OBJECT_DERRICK  ||
         m_burnType == OBJECT_STATION  ||
         m_burnType == OBJECT_FACTORY  ||
         m_burnType == OBJECT_REPAIR   ||
         m_burnType == OBJECT_DESTROYER||
         m_burnType == OBJECT_CONVERT  ||
         m_burnType == OBJECT_TOWER    ||
         m_burnType == OBJECT_RESEARCH ||
         m_burnType == OBJECT_RADAR    ||
         m_burnType == OBJECT_INFO     ||
         m_burnType == OBJECT_ENERGY   ||
         m_burnType == OBJECT_LABO     ||
         m_burnType == OBJECT_NUCLEAR  ||
         m_burnType == OBJECT_PARA     ||
         m_burnType == OBJECT_SAFE     ||
         m_burnType == OBJECT_HUSTON   ||
         m_burnType == OBJECT_START    ||
         m_burnType == OBJECT_END      )
    {
        m_object->SetType(OBJECT_RUINfactory); // Ruin
    }
    else
    {
        m_object->SetType(OBJECT_RUINmobilew1); // Wreck (recoverable by Recycler)
    }
    dynamic_cast<CDestroyableObject&>(*m_object).SetDying(DeathType::Alive);
    m_object->SetLock(false);
}
