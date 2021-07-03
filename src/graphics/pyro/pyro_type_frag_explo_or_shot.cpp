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

CFragExploOrShotPyro::CFragExploOrShotPyro(PyroType type, CObject *obj, float force)
    : CPyro(type, obj),
      m_force(force)
{}

void CFragExploOrShotPyro::AfterCreate()
{
    ObjectType oType = m_object->GetType();

    // Plays the sound of a pyrotechnic effect.
    if ( m_type == PT_FRAGT  ||
         m_type == PT_FRAGW  ||
         m_type == PT_EXPLOT ||
         m_type == PT_EXPLOW )
    {
        SoundType sound;
        if ( m_power )
        {
            sound = SOUND_EXPLOp;
        }
        else
        {
            sound = SOUND_EXPLO;
        }
        if ( oType == OBJECT_STONE   ||
             oType == OBJECT_METAL   ||
             oType == OBJECT_BULLET  ||
             oType == OBJECT_BBOX    ||
             oType == OBJECT_KEYa    ||
             oType == OBJECT_KEYb    ||
             oType == OBJECT_KEYc    ||
             oType == OBJECT_KEYd    )
        {
            sound = SOUND_EXPLOl;
        }
        if ( oType == OBJECT_URANIUM ||
             oType == OBJECT_POWER   ||
             oType == OBJECT_ATOMIC  ||
             oType == OBJECT_TNT     ||
             oType == OBJECT_BOMB    )
        {
            sound = SOUND_EXPLOlp;
        }
        m_sound->Play(sound, m_pos);
    }
    if ( m_type == PT_FRAGO  ||
         m_type == PT_EXPLOO ||
         m_type == PT_SHOTM  )
    {
        m_sound->Play(SOUND_EXPLOi, m_pos);
    }
    if ( oType == OBJECT_HUMAN )
    {
        assert(m_object->Implements(ObjectInterfaceType::Controllable));
        if ( m_type == PT_SHOTH && dynamic_cast<CControllableObject&>(*m_object).GetSelect() )
        {
            m_sound->Play(SOUND_AIE, m_pos);
            m_sound->Play(SOUND_AIE, m_engine->GetEyePt());
        }
    }

    if ( m_type == PT_FRAGT ||
         m_type == PT_FRAGO ||
         m_type == PT_FRAGW )
    {
        m_engine->DeleteShadowSpot(m_object->GetObjectRank(0));
    }

    if ( m_type == PT_SHOTT ||
         m_type == PT_SHOTM )
    {
        m_camera->StartEffect(CAM_EFFECT_SHOT, m_pos, m_force);
        m_speed = 1.0f/1.0f;
    }
    if ( m_type == PT_SHOTH )
    {
        assert(m_object->Implements(ObjectInterfaceType::Controllable));
        if ( m_camera->GetBlood() && dynamic_cast<CControllableObject&>(*m_object).GetSelect() )
        {
            m_camera->StartOver(CAM_OVER_EFFECT_BLOOD, m_pos, m_force);
        }
        m_speed = 1.0f/0.2f;
    }

    if ( m_type == PT_SHOTW )
    {
        m_speed = 1.0f/1.0f;
    }


    if ( m_type == PT_EXPLOT ||
         m_type == PT_EXPLOO ||
         m_type == PT_EXPLOW )
    {
        CreateTriangle(m_object, m_object->GetType(), 0);
        m_engine->DeleteShadowSpot(m_object->GetObjectRank(0));
        ExploStart();
    }
    // Generates the triangles of the explosion.
    if ( m_type == PT_FRAGT  ||
         m_type == PT_FRAGO  ||
         m_type == PT_FRAGW  ||
        (m_type == PT_EXPLOT && oType == OBJECT_MOBILEtg) ||
        (m_type == PT_EXPLOT && oType == OBJECT_TEEN28  ) ||
        (m_type == PT_EXPLOT && oType == OBJECT_TEEN31  ) )
    {
        for (int part = 0; part < OBJECTMAXPART; part++)
        {
            CreateTriangle(m_object, m_object->GetType(), part);
        }
    }

    if ( m_type == PT_FRAGT  ||
         m_type == PT_EXPLOT )
    {
        if ( m_power )
        {
            int total = static_cast<int>(10.0f*m_engine->GetParticleDensity());
            if ( oType == OBJECT_TNT  ||
                 oType == OBJECT_BOMB )  total *= 3;
            for (int i = 0; i < total; i++)
            {
                Math::Vector pos = m_posPower;
                Math::Vector speed;
                speed.x = (Math::Rand()-0.5f)*30.0f;
                speed.z = (Math::Rand()-0.5f)*30.0f;
                speed.y = Math::Rand()*30.0f;
                Math::Point dim;
                dim.x = 1.0f;
                dim.y = dim.x;
                float duration = Math::Rand()*3.0f+2.0f;
                float mass = Math::Rand()*10.0f+15.0f;
                m_particle->CreateTrack(pos, speed, dim, PARTITRACK1,
                                         duration, mass, Math::Rand()+0.7f, 1.0f);
            }
        }

        if (m_size > 10.0f)  // large enough (freight excluded)?
        {
            Math::Vector pos;
            if (m_power)
            {
                pos = m_posPower;
            }
            else
            {
                pos = m_pos;
                m_terrain->AdjustToFloor(pos);
                pos.y += 1.0f;
            }
            Math::Point dim;
            dim.x = m_size*0.4f;
            dim.y = dim.x;
            m_particle->CreateParticle(pos, Math::Vector(0.0f,0.0f,0.0f), dim, PARTISPHERE0, 2.0f, 0.0f, 0.0f);
        }
    }

    if ( m_type == PT_FRAGO  ||
         m_type == PT_EXPLOO )
    {
        int total = static_cast<int>(10.0f*m_engine->GetParticleDensity());
        for (int i = 0; i < total; i++)
        {
            Math::Vector pos = m_pos;
            Math::Vector speed;
            speed.x = (Math::Rand()-0.5f)*30.0f;
            speed.z = (Math::Rand()-0.5f)*30.0f;
            speed.y = Math::Rand()*50.0f;
            Math::Point dim;
            dim.x = 1.0f;
            dim.y = dim.x;
            float duration = Math::Rand()*1.0f+0.8f;
            float mass = Math::Rand()*10.0f+15.0f;
            m_particle->CreateParticle(pos, speed, dim, PARTIORGANIC1,
                                         duration, mass);
        }
        total = static_cast<int>(5.0f*m_engine->GetParticleDensity());
        for (int i = 0; i < total; i++)
        {
            Math::Vector pos = m_pos;
            Math::Vector speed;
            speed.x = (Math::Rand()-0.5f)*30.0f;
            speed.z = (Math::Rand()-0.5f)*30.0f;
            speed.y = Math::Rand()*50.0f;
            Math::Point dim;
            dim.x = 1.0f;
            dim.y = dim.x;
            float duration = Math::Rand()*2.0f+1.4f;
            float mass = Math::Rand()*10.0f+15.0f;
            m_particle->CreateTrack(pos, speed, dim, PARTITRACK4,
                                     duration, mass, duration*0.5f, dim.x*2.0f);
        }
    }

    if ( m_type == PT_FRAGT  ||
         m_type == PT_FRAGW  ||
         m_type == PT_EXPLOT ||
         m_type == PT_EXPLOW )
    {
        if (m_size > 10.0f || m_power)
        {
            Math::Vector pos = m_pos;
            Math::Vector speed(0.0f, 0.0f, 0.0f);
            Math::Point dim;
            dim.x = m_size;
            dim.y = dim.x;
            m_particle->CreateParticle(pos, speed, dim, PARTICHOC, 2.0f);
        }
    }

    if ( m_type == PT_FRAGO  ||
         m_type == PT_EXPLOO )
    {
        LightOperAdd(0.00f, 0.0f, -1.0f, -0.5f, -1.0f);  // dark green
        LightOperAdd(0.05f, 1.0f, -1.0f, -0.5f, -1.0f);  // dark green
        LightOperAdd(1.00f, 0.0f, -1.0f, -0.5f, -1.0f);  // dark green
    }
    else if ( m_type == PT_FRAGT  ||
              m_type == PT_EXPLOT )
    {
        LightOperAdd(0.00f, 1.0f,  4.0f,  4.0f,  2.0f);  // yellow
        LightOperAdd(0.02f, 1.0f,  4.0f,  2.0f,  0.0f);  // red-orange
        LightOperAdd(0.16f, 1.0f, -0.8f, -0.8f, -0.8f);  // dark gray
        LightOperAdd(1.00f, 0.0f, -0.8f, -0.8f, -0.8f);  // dark gray
    }
    else if ( m_type == PT_FRAGW  ||
              m_type == PT_EXPLOW ||
              m_type == PT_SHOTW  )
    {
        LightOperAdd(0.00f, 0.0f, -0.5f, -0.5f, -1.0f);  // dark yellow
        LightOperAdd(0.05f, 1.0f, -0.5f, -0.5f, -1.0f);  // dark yellow
        LightOperAdd(1.00f, 0.0f, -0.5f, -0.5f, -1.0f);  // dark yellow
    }

    if (m_type == PT_SHOTW)
    {
        // PT_SHOTW might be unused? In the previous code structure, this was a default case
        LightOperAdd(0.00f, 0.0f, -0.8f, -0.8f, -0.8f);  // dark gray
        LightOperAdd(0.05f, 1.0f, -0.8f, -0.8f, -0.8f);  // dark gray
        LightOperAdd(1.00f, 0.0f, -0.8f, -0.8f, -0.8f);  // dark gray
    }

    if (m_type == PT_FRAGT ||
        m_type == PT_FRAGO ||
        m_type == PT_FRAGW ||
        m_type == PT_EXPLOT ||
        m_type == PT_EXPLOO ||
        m_type == PT_EXPLOW ||
        m_type == PT_SHOTW)
    {
        // PT_SHOTW might be unused? In the previous code structure, this was a default case
        m_camera->StartEffect(CAM_EFFECT_EXPLO, m_pos, m_force);
    }

    if (m_type == PT_FRAGT || m_type == PT_EXPLOT)
    {
        CreateLight(m_pos, m_size*2.0f);
    }
    else if (m_type == PT_FRAGO || m_type == PT_EXPLOO || m_type == PT_FRAGW || m_type == PT_EXPLOW || m_type == PT_SHOTW)
    {
        // PT_SHOTW might be unused? In the previous code structure, this was a default case
        CreateLight(m_pos, 40.0f);
    }
}

Error CFragExploOrShotPyro::IsEnded() {
    // Destroys the object that exploded.
    //It should not be destroyed at the end of the Create,
    //because it is sometimes the object itself that makes the Create:
    //  pyro->Create(PT_FRAGT, this);
    if ( m_type == PT_FRAGT  ||
         m_type == PT_FRAGO  ||
         m_type == PT_FRAGW  )
    {
        DeleteObject(true, true);
    }

    return CPyro::IsEnded();
}

void CFragExploOrShotPyro::ExploStart()
{
    m_burnType = m_object->GetType();

    Math::Vector oPos = m_object->GetPosition();
    m_burnFall = m_terrain->GetHeightToFloor(oPos, true);

    m_object->Simplify();
    m_object->SetLock(true);  // ruin not usable yet
    assert(m_object->Implements(ObjectInterfaceType::Destroyable));
    dynamic_cast<CDestroyableObject&>(*m_object).SetDying(DeathType::Exploding);  // being destroyed
    m_object->FlatParent();

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

        // TODO: temporary hack (hopefully)
        assert(m_object->Implements(ObjectInterfaceType::Old));
        Math::Vector pos = dynamic_cast<COldObject&>(*m_object).GetPartPosition(i);

        Math::Vector speed;
        float weight;

        if (i == 0)  // main part?
        {
            weight = 0.0f;

            speed.y = -1.0f;
            speed.x = 0.0f;
            speed.z = 0.0f;
        }
        else
        {
            Math::Vector min, max;
            m_engine->GetObjectBBox(objRank, min, max);
            weight = Math::Distance(min, max);  // weight according to size!

            speed.y = 10.0f+Math::Rand()*20.0f;
            speed.x = (Math::Rand()-0.5f)*20.0f;
            speed.z = (Math::Rand()-0.5f)*20.0f;
        }

        int channel = m_particle->CreatePart(pos, speed, PARTIPART, 10.0f, 20.0f, weight, 0.5f);
        if (channel != -1)
            m_object->SetMasterParticle(i, channel);
    }
    m_engine->LoadTexture("textures/dirty04.png");

    DeleteObject(false, true);  // destroys the object transported + the battery
}

void CFragExploOrShotPyro::UpdateEffect()
{
    if ( m_type == PT_SHOTT &&
         m_lastParticle+m_engine->ParticleAdapt(0.05f) <= m_time )
    {
        m_lastParticle = m_time;

        if (m_crashSpheres.size() > 0)
        {
            int i = rand() % m_crashSpheres.size();
            Math::Vector pos = m_crashSpheres[i].pos;
            float radius = m_crashSpheres[i].radius;
            pos.x += (Math::Rand()-0.5f)*radius*2.0f;
            pos.z += (Math::Rand()-0.5f)*radius*2.0f;
            Math::Vector speed;
            speed.x = (Math::Rand()-0.5f)*radius*0.5f;
            speed.z = (Math::Rand()-0.5f)*radius*0.5f;
            speed.y = Math::Rand()*radius*1.0f;
            Math::Point dim;
            dim.x = Math::Rand()*radius*0.5f+radius*0.75f*m_force;
            dim.y = dim.x;
            m_particle->CreateParticle(pos, speed, dim, PARTISMOKE1, 3.0f);
        }
        else
        {
            Math::Vector pos = m_pos;
            pos.x += (Math::Rand()-0.5f)*m_size*0.3f;
            pos.z += (Math::Rand()-0.5f)*m_size*0.3f;
            Math::Vector speed;
            speed.x = (Math::Rand()-0.5f)*m_size*0.1f;
            speed.z = (Math::Rand()-0.5f)*m_size*0.1f;
            speed.y = Math::Rand()*m_size*0.2f;
            Math::Point dim;
            dim.x = Math::Rand()*m_size/10.0f+m_size/10.0f*m_force;
            dim.y = dim.x;
            m_particle->CreateParticle(pos, speed, dim, PARTISMOKE1, 3.0f);
        }
    }

    if ( m_camera->GetBlood() && m_type == PT_SHOTH &&
         m_lastParticle+m_engine->ParticleAdapt(0.05f) <= m_time )
    {
        m_lastParticle = m_time;

        for (int i = 0; i < 10; i++)
        {
            Math::Vector pos = m_pos;
            pos.x += (Math::Rand()-0.5f)*m_size*0.2f;
            pos.z += (Math::Rand()-0.5f)*m_size*0.2f;
            pos.y += (Math::Rand()-0.5f)*m_size*0.5f;
            Math::Vector speed;
            speed.x = (Math::Rand()-0.5f)*5.0f;
            speed.z = (Math::Rand()-0.5f)*5.0f;
            speed.y = Math::Rand()*1.0f;
            Math::Point dim;
            dim.x = 1.0f;
            dim.y = dim.x;
            m_particle->CreateParticle(pos, speed, dim, PARTIBLOOD, Math::Rand()*3.0f+3.0f, Math::Rand()*10.0f+15.0f, 0.5f);
        }
    }

    if ( m_camera->GetBlood() && m_type == PT_SHOTM &&
         m_lastParticle+m_engine->ParticleAdapt(0.05f) <= m_time )
    {
        m_lastParticle = m_time;

        int r = static_cast<int>(10.0f*m_engine->GetParticleDensity());
        for (int i = 0; i < r; i++)
        {
            Math::Vector pos = m_pos;
            pos.x += (Math::Rand()-0.5f)*20.0f;
            pos.z += (Math::Rand()-0.5f)*20.0f;
            pos.y += 8.0f;
            Math::Vector speed;
            speed.x = (Math::Rand()-0.5f)*40.0f;
            speed.z = (Math::Rand()-0.5f)*40.0f;
            speed.y = Math::Rand()*40.0f;
            Math::Point dim;
            dim.x = Math::Rand()*8.0f+8.0f*m_force;
            dim.y = dim.x;

            m_particle->CreateParticle(pos, speed, dim, PARTIBLOODM, 2.0f, 50.0f, 0.0f);
        }
    }

    if ( m_type == PT_SHOTW &&
         m_lastParticle+m_engine->ParticleAdapt(0.05f) <= m_time )
    {
        m_lastParticle = m_time;

        if (m_crashSpheres.size() > 0)
        {
            int i = rand() % m_crashSpheres.size();
            Math::Vector pos = m_crashSpheres[i].pos;
            float radius = m_crashSpheres[i].radius;
            pos.x += (Math::Rand()-0.5f)*radius*2.0f;
            pos.z += (Math::Rand()-0.5f)*radius*2.0f;
            Math::Vector speed;
            speed.x = (Math::Rand()-0.5f)*radius*0.5f;
            speed.z = (Math::Rand()-0.5f)*radius*0.5f;
            speed.y = Math::Rand()*radius*1.0f;
            Math::Point dim;
            dim.x = 1.0f*m_force;
            dim.y = dim.x;
            m_particle->CreateParticle(pos, speed, dim, PARTIBLITZ, 0.5f, 0.0f, 0.0f);
        }
        else
        {
            Math::Vector pos = m_pos;
            pos.x += (Math::Rand()-0.5f)*m_size*0.3f;
            pos.z += (Math::Rand()-0.5f)*m_size*0.3f;
            Math::Vector speed;
            speed.x = (Math::Rand()-0.5f)*m_size*0.1f;
            speed.z = (Math::Rand()-0.5f)*m_size*0.1f;
            speed.y = Math::Rand()*m_size*0.2f;
            Math::Point dim;
            dim.x = 1.0f*m_force;
            dim.y = dim.x;
            m_particle->CreateParticle(pos, speed, dim, PARTIBLITZ, 0.5f, 0.0f, 0.0f);
        }
    }

    if ( m_type == PT_SHOTW &&
         m_lastParticleSmoke+m_engine->ParticleAdapt(0.10f) <= m_time )
    {
        m_lastParticleSmoke = m_time;

        Math::Vector pos = m_pos;
        pos.y -= 2.0f;
        pos.x += (Math::Rand()-0.5f)*4.0f;
        pos.z += (Math::Rand()-0.5f)*4.0f;
        Math::Vector speed;
        speed.x = 0.0f;
        speed.z = 0.0f;
        speed.y = 10.0f+Math::Rand()*10.0f;
        Math::Point dim;
        dim.x = Math::Rand()*2.5f+2.0f*m_force;
        dim.y = dim.x;
        m_particle->CreateParticle(pos, speed, dim, PARTICRASH, 4.0f);
    }

    if ( (m_type == PT_FRAGT || m_type == PT_EXPLOT) &&
         m_progress < 0.05f &&
         m_lastParticle+m_engine->ParticleAdapt(0.05f) <= m_time )
    {
        m_lastParticle = m_time;

        Math::Vector pos = m_pos;
        Math::Vector speed;
        speed.x = (Math::Rand()-0.5f)*m_size*1.0f;
        speed.z = (Math::Rand()-0.5f)*m_size*1.0f;
        speed.y = Math::Rand()*m_size*0.50f;
        Math::Point dim;
        dim.x = Math::Rand()*m_size/5.0f+m_size/5.0f;
        dim.y = dim.x;

        m_particle->CreateParticle(pos, speed, dim, PARTIEXPLOT);
    }

    if ( (m_type == PT_FRAGT || m_type == PT_EXPLOT) &&
         m_progress < 0.10f &&
         m_lastParticleSmoke+m_engine->ParticleAdapt(0.10f) <= m_time )
    {
        m_lastParticleSmoke = m_time;

        Math::Point dim;
        dim.x = Math::Rand()*m_size/3.0f+m_size/3.0f;
        dim.y = dim.x;
        Math::Vector pos = m_pos;
        pos.x += (Math::Rand()-0.5f)*m_size*0.5f;
        pos.z += (Math::Rand()-0.5f)*m_size*0.5f;
        m_terrain->AdjustToFloor(pos);
        Math::Vector speed;
        speed.x = 0.0f;
        speed.z = 0.0f;
        speed.y = -dim.x/2.0f/4.0f;
        pos.y += dim.x/2.0f;

        ParticleType type;
        int r = rand()%2;
        if (r == 0) type = PARTISMOKE1;
        if (r == 1) type = PARTISMOKE2;
        m_particle->CreateParticle(pos, speed, dim, type, 6.0f);
    }

    if ( (m_type == PT_FRAGO || m_type == PT_EXPLOO) &&
         m_progress < 0.03f &&
         m_lastParticle+m_engine->ParticleAdapt(0.1f) <= m_time )
    {
        m_lastParticle = m_time;

        Math::Vector pos = m_pos;
        Math::Vector speed;
        speed.x = (Math::Rand()-0.5f)*m_size*2.0f;
        speed.z = (Math::Rand()-0.5f)*m_size*2.0f;
        speed.y = Math::Rand()*m_size*1.0f;
        Math::Point dim;
        dim.x = Math::Rand()*m_size/2.0f+m_size/2.0f;
        dim.y = dim.x;

        m_particle->CreateParticle(pos, speed, dim, PARTIEXPLOO);
    }

    if ( (m_type == PT_FRAGW || m_type == PT_EXPLOW) &&
         m_progress < 0.05f &&
         m_lastParticle+m_engine->ParticleAdapt(0.05f) <= m_time )
    {
        m_lastParticle = m_time;

        Math::Vector pos = m_pos;
        Math::Vector speed;
        speed.x = (Math::Rand()-0.5f)*m_size*1.0f;
        speed.z = (Math::Rand()-0.5f)*m_size*1.0f;
        speed.y = Math::Rand()*m_size*0.50f;
        Math::Point dim;
        dim.x = 1.0f;
        dim.y = dim.x;

        m_particle->CreateParticle(pos, speed, dim, PARTIBLITZ, 0.5f, 0.0f, 0.0f);
    }

    if ( (m_type == PT_FRAGW || m_type == PT_EXPLOW) &&
         m_progress < 0.25f &&
         m_lastParticleSmoke+m_engine->ParticleAdapt(0.05f) <= m_time )
    {
        m_lastParticleSmoke = m_time;

        Math::Vector pos = m_pos;
        pos.y -= 2.0f;
        pos.x += (Math::Rand()-0.5f)*4.0f;
        pos.z += (Math::Rand()-0.5f)*4.0f;
        Math::Vector speed;
        speed.x = 0.0f;
        speed.z = 0.0f;
        speed.y = 4.0f+Math::Rand()*4.0f;
        Math::Point dim;
        dim.x = Math::Rand()*2.5f+2.0f;
        dim.y = dim.x;
        m_particle->CreateParticle(pos, speed, dim, PARTICRASH, 4.0f);
    }
}

void CFragExploOrShotPyro::AfterEnd()
{
    if ( m_type == PT_EXPLOT ||
         m_type == PT_EXPLOO ||
         m_type == PT_EXPLOW )  // explosion?
    {
        DeleteObject(true, false);  // removes the main object
    }
}
