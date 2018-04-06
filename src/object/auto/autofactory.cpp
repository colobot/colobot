/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2016, Daniel Roux, EPSITEC SA & TerranovaTeam
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


#include "object/auto/autofactory.h"

#include "common/make_unique.h"

#include "level/robotmain.h"

#include "level/parser/parser.h"
#include "level/parser/parserline.h"
#include "level/parser/parserparam.h"

#include "math/geometry.h"

#include "object/object_create_params.h"
#include "object/object_manager.h"
#include "object/old_object.h"

#include "object/interface/program_storage_object.h"
#include "object/interface/programmable_object.h"
#include "object/interface/transportable_object.h"

#include "physics/physics.h"

#include "script/script.h"

#include "sound/sound.h"

#include "ui/controls/interface.h"
#include "ui/controls/window.h"


#include <boost/regex.hpp>



// Object's constructor.

CAutoFactory::CAutoFactory(COldObject* object) : CAuto(object)
{
    Init();
    m_type  = OBJECT_MOBILEws;
    m_phase = AFP_WAIT;  // paused until the first Init ()
    m_channelSound = -1;
}

// Object's destructor.

CAutoFactory::~CAutoFactory()
{
}


// Destroys the object.

void CAutoFactory::DeleteObject(bool all)
{
    if ( !all )
    {
        CObject* cargo = SearchCargo();  // transform metal?
        if ( cargo != nullptr )
        {
            CObjectManager::GetInstancePointer()->DeleteObject(cargo);
        }

        CObject* vehicle = SearchVehicle();
        if ( vehicle != nullptr )
        {
            CObjectManager::GetInstancePointer()->DeleteObject(vehicle);
        }
    }

    if ( m_channelSound != -1 )
    {
        m_sound->FlushEnvelope(m_channelSound);
        m_sound->AddEnvelope(m_channelSound, 0.0f, 1.0f, 1.0f, SOPER_STOP);
        m_channelSound = -1;
    }

    CAuto::DeleteObject(all);
}


// Initialize the object.

void CAutoFactory::Init()
{
    m_phase    = AFP_WAIT;
    m_progress = 0.0f;
    m_speed    = 1.0f/2.0f;

    m_time = 0.0f;
    m_lastParticle = 0.0f;

    m_cargoPos = m_object->GetPosition();

    m_program = "";

    CAuto::Init();
}


// Starts an action

Error CAutoFactory::StartAction(int param)
{
    CObject*   cargo;
    ObjectType type = static_cast<ObjectType>(param);

    if ( type != OBJECT_NULL )
    {
        if ( m_phase != AFP_WAIT )
        {
            return ERR_OBJ_BUSY;
        }

        m_type = type;

        cargo = SearchCargo();  // transform metal?
        if ( cargo == nullptr )
        {
            return ERR_FACTORY_NULL;
        }
        if ( NearestVehicle() )
        {
            return ERR_FACTORY_NEAR;
        }

        m_program = "";
        SetBusy(true);
        InitProgressTotal(3.0f+2.0f+15.0f+2.0f+3.0f);
        UpdateInterface();

        cargo->SetLock(true);  // usable metal
        SoundManip(3.0f, 1.0f, 0.5f);

        m_phase    = AFP_CLOSE_S;
        m_progress = 0.0f;
        m_speed    = 1.0f/3.0f;
        return ERR_OK;
    }
    return ERR_UNKNOWN;
}


// Sets program for created robot

void CAutoFactory::SetProgram(const std::string& program)
{
    m_program = program;
}

ObjectType ObjectTypeFromFactoryButton(EventType eventType)
{
    if ( eventType == EVENT_OBJECT_FACTORYwa )  return OBJECT_MOBILEwa;
    if ( eventType == EVENT_OBJECT_FACTORYta )  return OBJECT_MOBILEta;
    if ( eventType == EVENT_OBJECT_FACTORYfa )  return OBJECT_MOBILEfa;
    if ( eventType == EVENT_OBJECT_FACTORYia )  return OBJECT_MOBILEia;
    if ( eventType == EVENT_OBJECT_FACTORYws )  return OBJECT_MOBILEws;
    if ( eventType == EVENT_OBJECT_FACTORYts )  return OBJECT_MOBILEts;
    if ( eventType == EVENT_OBJECT_FACTORYfs )  return OBJECT_MOBILEfs;
    if ( eventType == EVENT_OBJECT_FACTORYis )  return OBJECT_MOBILEis;
    if ( eventType == EVENT_OBJECT_FACTORYwc )  return OBJECT_MOBILEwc;
    if ( eventType == EVENT_OBJECT_FACTORYtc )  return OBJECT_MOBILEtc;
    if ( eventType == EVENT_OBJECT_FACTORYfc )  return OBJECT_MOBILEfc;
    if ( eventType == EVENT_OBJECT_FACTORYic )  return OBJECT_MOBILEic;
    if ( eventType == EVENT_OBJECT_FACTORYwi )  return OBJECT_MOBILEwi;
    if ( eventType == EVENT_OBJECT_FACTORYti )  return OBJECT_MOBILEti;
    if ( eventType == EVENT_OBJECT_FACTORYfi )  return OBJECT_MOBILEfi;
    if ( eventType == EVENT_OBJECT_FACTORYii )  return OBJECT_MOBILEii;
    if ( eventType == EVENT_OBJECT_FACTORYrt )  return OBJECT_MOBILErt;
    if ( eventType == EVENT_OBJECT_FACTORYrc )  return OBJECT_MOBILErc;
    if ( eventType == EVENT_OBJECT_FACTORYrr )  return OBJECT_MOBILErr;
    if ( eventType == EVENT_OBJECT_FACTORYrs )  return OBJECT_MOBILErs;
    if ( eventType == EVENT_OBJECT_FACTORYsa )  return OBJECT_MOBILEsa;

    return OBJECT_NULL;
}

// Management of an event.

bool CAutoFactory::EventProcess(const Event &event)
{
    ObjectType  type;
    CObject*    cargo;
    CObject*    vehicle;
    Math::Matrix*   mat;
    CPhysics*   physics;
    Math::Vector    pos, speed;
    Math::Point     dim;
    float       zoom, angle, prog;
    int         i;

    CAuto::EventProcess(event);

    if ( m_engine->GetPause() )  return true;

    if ( m_object->GetSelect() )  // factory selected?
    {
        if ( event.type == EVENT_UPDINTERFACE )
        {
            CreateInterface(true);
        }

        type = ObjectTypeFromFactoryButton(event.type);

        Error err = StartAction(type);
        if( err != ERR_OK && err != ERR_UNKNOWN )
            m_main->DisplayError(err, m_object);

        if( err != ERR_UNKNOWN )
            return false;
    }

    if ( event.type != EVENT_FRAME )  return true;

    m_progress += event.rTime*m_speed;
    EventProgress(event.rTime);

    if ( m_phase == AFP_WAIT )
    {
        if ( m_progress >= 1.0f )
        {
            m_phase    = AFP_WAIT;  // still waiting ...
            m_progress = 0.0f;
            m_speed    = 1.0f/2.0f;
        }
    }

    if ( m_phase == AFP_CLOSE_S )
    {
        if ( m_progress < 1.0f )
        {
            for ( i=0 ; i<9 ; i++ )
            {
                zoom = 0.30f+(m_progress-0.5f+i/16.0f)*2.0f*0.70f;
                if ( zoom < 0.30f )  zoom = 0.30f;
                if ( zoom > 1.00f )  zoom = 1.00f;
                m_object->SetPartScaleZ( 1+i, zoom);
                m_object->SetPartScaleZ(10+i, zoom);
            }
        }
        else
        {
            for ( i=0 ; i<9 ; i++ )
            {
                m_object->SetPartScaleZ( 1+i, 1.0f);
                m_object->SetPartScaleZ(10+i, 1.0f);
            }

            SoundManip(2.0f, 1.0f, 1.2f);

            m_phase    = AFP_CLOSE_T;
            m_progress = 0.0f;
            m_speed    = 1.0f/2.0f;
        }
    }

    if ( m_phase == AFP_CLOSE_T )
    {
        if ( m_progress < 1.0f )
        {
            for ( i=0 ; i<9 ; i++ )
            {
                angle = -m_progress*(Math::PI/2.0f)+Math::PI/2.0f;
                m_object->SetPartRotationZ( 1+i,  angle);
                m_object->SetPartRotationZ(10+i, -angle);
            }
        }
        else
        {
            for ( i=0 ; i<9 ; i++ )
            {
                m_object->SetPartRotationZ( 1+i, 0.0f);
                m_object->SetPartRotationZ(10+i, 0.0f);
            }

            m_channelSound = m_sound->Play(SOUND_FACTORY, m_object->GetPosition(), 0.0f, 1.0f, true);
            m_sound->AddEnvelope(m_channelSound, 1.0f, 1.0f,  2.0f, SOPER_CONTINUE);
            m_sound->AddEnvelope(m_channelSound, 1.0f, 1.0f, 11.0f, SOPER_CONTINUE);
            m_sound->AddEnvelope(m_channelSound, 0.0f, 1.0f,  2.0f, SOPER_STOP);

            m_phase    = AFP_BUILD;
            m_progress = 0.0f;
            m_speed    = 1.0f/15.0f;
        }
    }

    if ( m_phase == AFP_BUILD )
    {
        if ( m_progress == 0.0f )
        {
            if ( !CreateVehicle() )
            {
                cargo = SearchCargo();  // transform metal?
                if ( cargo != nullptr )
                {
                    cargo->SetLock(false);  // metal usable again
                }

                if ( m_channelSound != -1 )
                {
                    m_sound->FlushEnvelope(m_channelSound);
                    m_sound->AddEnvelope(m_channelSound, 0.0f, 1.0f, 1.0f, SOPER_STOP);
                    m_channelSound = -1;
                }

                m_phase    = AFP_OPEN_T;
                m_progress = 0.0f;
                m_speed    = 1.0f/2.0f;
                return true;
            }
        }

        if ( m_progress < 1.0f )
        {
            if ( m_type == OBJECT_MOBILErt ||
                 m_type == OBJECT_MOBILErc ||
                 m_type == OBJECT_MOBILErr ||
                 m_type == OBJECT_MOBILErs )
            {
                prog = 1.0f-m_progress*1.5f;
                if ( prog < 0.0f )  prog = 0.0f;
            }
            else
            {
                prog = 1.0f-m_progress;
            }
            angle = powf(prog*10.0f, 2.0f)+m_object->GetRotationY();

            vehicle = SearchVehicle();
            if ( vehicle != nullptr )
            {
                vehicle->SetRotationY(angle+Math::PI);
                vehicle->SetScale(m_progress);
            }

            cargo = SearchCargo();  // transform metal?
            if ( cargo != nullptr )
            {
                cargo->SetScale(1.0f-m_progress);
            }

            if ( m_lastParticle+m_engine->ParticleAdapt(0.05f) <= m_time )
            {
                m_lastParticle = m_time;

                mat = m_object->GetWorldMatrix(0);
                pos = Math::Vector(-12.0f, 20.0f, -4.0f);  // position of chimney
                pos = Math::Transform(*mat, pos);
                pos.y += 2.0f;
                pos.x += (Math::Rand()-0.5f)*2.0f;
                pos.z += (Math::Rand()-0.5f)*2.0f;
                speed.x = 0.0f;
                speed.z = 0.0f;
                speed.y = 6.0f+Math::Rand()*6.0f;
                dim.x = Math::Rand()*1.5f+1.0f;
                dim.y = dim.x;
                m_particle->CreateParticle(pos, speed, dim, Gfx::PARTISMOKE3, 4.0f);
            }
        }
        else
        {
            m_main->DisplayError(INFO_FACTORY, m_object);
            SoundManip(2.0f, 1.0f, 1.2f);

            cargo = SearchCargo();  // transform metal?
            if ( cargo != nullptr )
            {
                CObjectManager::GetInstancePointer()->DeleteObject(cargo);
            }

            vehicle = SearchVehicle();
            if ( vehicle != nullptr )
            {
                assert(vehicle->Implements(ObjectInterfaceType::Movable));
                physics = dynamic_cast<CMovableObject*>(vehicle)->GetPhysics();
                physics->SetFreeze(false);  // can move

                vehicle->SetLock(false);  // vehicle useable
                vehicle->SetRotationY(m_object->GetRotationY()+Math::PI);
                vehicle->SetScale(1.0f);

                if ( !m_program.empty() )
                {
                    if (vehicle->Implements(ObjectInterfaceType::Programmable) && vehicle->Implements(ObjectInterfaceType::ProgramStorage))
                    {
                        Program* program = dynamic_cast<CProgramStorageObject*>(vehicle)->AddProgram();

                        if (boost::regex_match(m_program, boost::regex("[A-Za-z0-9_]+"))) // Public function name?
                        {
                            std::string code = "extern void object::Start_"+m_program+"()\n{\n\t\n\t//Automatically generated by object.factory()\n\t"+m_program+"();\n\t\n}\n";
                            program->script->SendScript(code.c_str());
                        }
                        else if (boost::regex_match(m_program, boost::regex(".*\\.txt"))) // File name (with .txt extension)?
                        {
                            program->script->ReadScript(m_program.c_str());
                        }
                        else // Program code?
                        {
                            program->script->SendScript(m_program.c_str());
                        }

                        dynamic_cast<CProgrammableObject*>(vehicle)->RunProgram(program);
                    }
                }
            }

            m_main->CreateShortcuts();

            m_phase    = AFP_OPEN_T;
            m_progress = 0.0f;
            m_speed    = 1.0f/2.0f;
        }
    }

    if ( m_phase == AFP_OPEN_T )
    {
        if ( m_progress < 1.0f )
        {
            for ( i=0 ; i<9 ; i++ )
            {
                angle = -(1.0f-m_progress)*(Math::PI/2.0f)+Math::PI/2.0f;
                m_object->SetPartRotationZ( 1+i,  angle);
                m_object->SetPartRotationZ(10+i, -angle);
            }

            if ( m_lastParticle+m_engine->ParticleAdapt(0.1f) <= m_time )
            {
                m_lastParticle = m_time;

                pos = m_cargoPos;
                pos.x += (Math::Rand()-0.5f)*10.0f;
                pos.z += (Math::Rand()-0.5f)*10.0f;
                pos.y += Math::Rand()*10.0f;
                speed = Math::Vector(0.0f, 0.0f, 0.0f);
                dim.x = 2.0f;
                dim.y = dim.x;
                m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIGLINT, 2.0f, 0.0f, 0.0f);
            }
        }
        else
        {
            for ( i=0 ; i<9 ; i++ )
            {
                m_object->SetPartRotationZ( 1+i,  Math::PI/2.0f);
                m_object->SetPartRotationZ(10+i, -Math::PI/2.0f);
            }

            SoundManip(3.0f, 1.0f, 0.5f);

            m_phase    = AFP_OPEN_S;
            m_progress = 0.0f;
            m_speed    = 1.0f/3.0f;
        }
    }

    if ( m_phase == AFP_OPEN_S )
    {
        if ( m_progress < 1.0f )
        {
            for ( i=0 ; i<9 ; i++ )
            {
                zoom = 0.30f+((1.0f-m_progress)-0.5f+i/16.0f)*2.0f*0.70f;
                if ( zoom < 0.30f )  zoom = 0.30f;
                if ( zoom > 1.00f )  zoom = 1.00f;
                m_object->SetPartScaleZ( 1+i, zoom);
                m_object->SetPartScaleZ(10+i, zoom);
            }

            if ( m_lastParticle+m_engine->ParticleAdapt(0.1f) <= m_time )
            {
                m_lastParticle = m_time;

                pos = m_cargoPos;
                pos.x += (Math::Rand()-0.5f)*10.0f;
                pos.z += (Math::Rand()-0.5f)*10.0f;
                pos.y += Math::Rand()*10.0f;
                speed = Math::Vector(0.0f, 0.0f, 0.0f);
                dim.x = 2.0f;
                dim.y = dim.x;
                m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIGLINT, 2.0f, 0.0f, 0.0f);
            }
        }
        else
        {
            for ( i=0 ; i<9 ; i++ )
            {
                m_object->SetPartScaleZ( 1+i, 0.30f);
                m_object->SetPartScaleZ(10+i, 0.30f);
            }

            SetBusy(false);
            UpdateInterface();

            m_phase    = AFP_WAIT;
            m_progress = 0.0f;
            m_speed    = 1.0f/2.0f;
        }
    }

    return true;
}


// Saves all parameters of the controller.

bool CAutoFactory::Write(CLevelParserLine* line)
{
    if ( m_phase == AFP_WAIT )  return false;

    line->AddParam("aExist", MakeUnique<CLevelParserParam>(true));
    CAuto::Write(line);
    line->AddParam("aPhase", MakeUnique<CLevelParserParam>(static_cast<int>(m_phase)));
    line->AddParam("aProgress", MakeUnique<CLevelParserParam>(m_progress));
    line->AddParam("aSpeed", MakeUnique<CLevelParserParam>(m_speed));

    return true;
}

// Restores all parameters of the controller

bool CAutoFactory::Read(CLevelParserLine* line)
{
    if ( !line->GetParam("aExist")->AsBool(false) )  return false;

    CAuto::Read(line);
    m_phase = static_cast< AutoFactoryPhase >(line->GetParam("aPhase")->AsInt(AFP_WAIT));
    m_progress = line->GetParam("aProgress")->AsFloat(0.0f);
    m_speed = line->GetParam("aSpeed")->AsFloat(1.0f);

    m_lastParticle = 0.0f;
    m_cargoPos = m_object->GetPosition();

    return true;
}


//Seeks the cargo.

CObject* CAutoFactory::SearchCargo()
{
    for (CObject* obj : CObjectManager::GetInstancePointer()->GetAllObjects())
    {
        ObjectType type = obj->GetType();
        if ( type != OBJECT_METAL )  continue;
        if (IsObjectBeingTransported(obj))  continue;

        Math::Vector oPos = obj->GetPosition();
        float dist = Math::Distance(oPos, m_cargoPos);

        if ( dist < 8.0f )  return obj;
    }

    return nullptr;
}

// Search if a vehicle is too close.

bool CAutoFactory::NearestVehicle()
{
    Math::Vector cPos = m_object->GetPosition();

    for (CObject* obj : CObjectManager::GetInstancePointer()->GetAllObjects())
    {
        ObjectType type = obj->GetType();
        if ( type != OBJECT_HUMAN    &&
             type != OBJECT_MOBILEfa &&
             type != OBJECT_MOBILEta &&
             type != OBJECT_MOBILEwa &&
             type != OBJECT_MOBILEia &&
             type != OBJECT_MOBILEfc &&
             type != OBJECT_MOBILEtc &&
             type != OBJECT_MOBILEwc &&
             type != OBJECT_MOBILEic &&
             type != OBJECT_MOBILEfi &&
             type != OBJECT_MOBILEti &&
             type != OBJECT_MOBILEwi &&
             type != OBJECT_MOBILEii &&
             type != OBJECT_MOBILEfs &&
             type != OBJECT_MOBILEts &&
             type != OBJECT_MOBILEws &&
             type != OBJECT_MOBILEis &&
             type != OBJECT_MOBILErt &&
             type != OBJECT_MOBILErc &&
             type != OBJECT_MOBILErr &&
             type != OBJECT_MOBILErs &&
             type != OBJECT_MOBILEsa &&
             type != OBJECT_MOBILEtg &&
             type != OBJECT_MOBILEft &&
             type != OBJECT_MOBILEtt &&
             type != OBJECT_MOBILEwt &&
             type != OBJECT_MOBILEit &&
             type != OBJECT_MOBILEdr &&
             type != OBJECT_MOTHER   &&
             type != OBJECT_ANT      &&
             type != OBJECT_SPIDER   &&
             type != OBJECT_BEE      &&
             type != OBJECT_WORM     )  continue;

        if (obj->GetCrashSphereCount() == 0) continue;

        auto crashSphere = obj->GetFirstCrashSphere();
        if (Math::DistanceToSphere(cPos, crashSphere.sphere) < 10.0f)
            return true;
    }

    return false;
}


// Creates a vehicle.

bool CAutoFactory::CreateVehicle()
{
    float angle = m_object->GetRotationY();

    Math::Vector pos;
    if ( m_type == OBJECT_MOBILErt ||
         m_type == OBJECT_MOBILErc ||
         m_type == OBJECT_MOBILErr ||
         m_type == OBJECT_MOBILErs )
    {
        pos = Math::Vector(2.0f, 0.0f, 0.0f);
    }
    else
    {
        pos = Math::Vector(4.0f, 0.0f, 0.0f);
    }
    Math::Matrix* mat = m_object->GetWorldMatrix(0);
    pos = Transform(*mat, pos);

    ObjectCreateParams params;
    params.pos = pos;
    params.angle = angle;
    params.type = m_type;
    params.team = m_object->GetTeam();
    CObject* vehicle = CObjectManager::GetInstancePointer()->CreateObject(params);

    vehicle->SetLock(true);  // not usable

    assert(vehicle->Implements(ObjectInterfaceType::Movable));
    CPhysics* physics = dynamic_cast<CMovableObject*>(vehicle)->GetPhysics();
    physics->SetFreeze(true);  // it doesn't move

    if (vehicle->Implements(ObjectInterfaceType::ProgramStorage))
    {
        CProgramStorageObject* programStorage = dynamic_cast<CProgramStorageObject*>(vehicle);
        for (const std::string& name : m_main->GetNewScriptNames(m_type))
        {
            Program* prog = programStorage->AddProgram();
            programStorage->ReadProgram(prog, InjectLevelPathsForCurrentLevel(name));
            prog->readOnly = true;
            prog->filename = name;
        }
    }

    return true;
}

// Seeking the vehicle during manufacture.

CObject* CAutoFactory::SearchVehicle()
{
    for (CObject* obj : CObjectManager::GetInstancePointer()->GetAllObjects())
    {
        if ( !obj->GetLock() )  continue;

        ObjectType  type = obj->GetType();
        if ( type != m_type )  continue;
        if (IsObjectBeingTransported(obj))  continue;

        Math::Vector oPos = obj->GetPosition();
        float dist = Math::Distance(oPos, m_cargoPos);

        if ( dist < 8.0f )  return obj;
    }

    return nullptr;
}

// Creates all the interface when the object is selected.

bool CAutoFactory::CreateInterface(bool bSelect)
{
    Ui::CWindow*    pw;
    Math::Point     pos, dim, ddim;
    float       ox, oy, sx, sy;

    CAuto::CreateInterface(bSelect);

    if ( !bSelect )  return true;

    pw = static_cast< Ui::CWindow* >(m_interface->SearchControl(EVENT_WINDOW0));
    if ( pw == nullptr )  return false;

    dim.x = 22.0f/640.0f;
    dim.y = 22.0f/480.0f;
    ox = 3.0f/640.0f;
    oy = 3.0f/480.0f;
    sx = 22.0f/640.0f;
    sy = 22.0f/480.0f;

    pos.x = ox+sx*4.5f;
    pos.y = oy+sy*1.5f;
    pw->CreateButton(pos, dim, 128+9, EVENT_OBJECT_FACTORYwa);
    pos.x += dim.x;
    pw->CreateButton(pos, dim, 128+10, EVENT_OBJECT_FACTORYta);
    pos.x = ox+sx*4.5f;
    pos.y = oy+sy*0.5f;
    pw->CreateButton(pos, dim, 128+11, EVENT_OBJECT_FACTORYfa);
    pos.x += dim.x;
    pw->CreateButton(pos, dim, 128+22, EVENT_OBJECT_FACTORYia);

    pos.x = ox+sx*7.0f;
    pos.y = oy+sy*1.5f;
    pw->CreateButton(pos, dim, 128+12, EVENT_OBJECT_FACTORYws);
    pos.x += dim.x;
    pw->CreateButton(pos, dim, 128+13, EVENT_OBJECT_FACTORYts);
    pos.x = ox+sx*7.0f;
    pos.y = oy+sy*0.5f;
    pw->CreateButton(pos, dim, 128+14, EVENT_OBJECT_FACTORYfs);
    pos.x += dim.x;
    pw->CreateButton(pos, dim, 128+24, EVENT_OBJECT_FACTORYis);

    pos.x = ox+sx*9.5f;
    pos.y = oy+sy*1.5f;
    pw->CreateButton(pos, dim, 128+15, EVENT_OBJECT_FACTORYwc);
    pos.x += dim.x;
    pw->CreateButton(pos, dim, 128+16, EVENT_OBJECT_FACTORYtc);
    pos.x = ox+sx*9.5f;
    pos.y = oy+sy*0.5f;
    pw->CreateButton(pos, dim, 128+17, EVENT_OBJECT_FACTORYfc);
    pos.x += dim.x;
    pw->CreateButton(pos, dim, 128+23, EVENT_OBJECT_FACTORYic);

    pos.x = ox+sx*12.0f;
    pos.y = oy+sy*1.5f;
    pw->CreateButton(pos, dim, 128+25, EVENT_OBJECT_FACTORYwi);
    pos.x += dim.x;
    pw->CreateButton(pos, dim, 128+26, EVENT_OBJECT_FACTORYti);
    pos.x = ox+sx*12.0f;
    pos.y = oy+sy*0.5f;
    pw->CreateButton(pos, dim, 128+27, EVENT_OBJECT_FACTORYfi);
    pos.x += dim.x;
    pw->CreateButton(pos, dim, 128+28, EVENT_OBJECT_FACTORYii);

    pos.x = ox+sx*14.5f;
    pos.y = oy+sy*1.5f;
    pw->CreateButton(pos, dim, 128+18, EVENT_OBJECT_FACTORYrt);
    pos.x += dim.x;
    pw->CreateButton(pos, dim, 128+19, EVENT_OBJECT_FACTORYrc);
    pos.x = ox+sx*14.5f;
    pos.y = oy+sy*0.5f;
    pw->CreateButton(pos, dim, 128+20, EVENT_OBJECT_FACTORYrr);
    pos.x += dim.x;
    pw->CreateButton(pos, dim, 128+29, EVENT_OBJECT_FACTORYrs);

    pos.x = ox+sx*17.0f;
    pos.y = oy+sy*1.0f;
    pw->CreateButton(pos, dim, 128+21, EVENT_OBJECT_FACTORYsa);

    pos.x = ox+sx*0.0f;
    pos.y = oy+sy*0;
    ddim.x = 66.0f/640.0f;
    ddim.y = 66.0f/480.0f;
    pw->CreateGroup(pos, ddim, 101, EVENT_OBJECT_TYPE);

    UpdateInterface();
    return true;
}

// Updates the status of all interface buttons.

void CAutoFactory::UpdateInterface()
{
    Ui::CWindow*    pw;

    if ( !m_object->GetSelect() )  return;

    CAuto::UpdateInterface();

    pw = static_cast< Ui::CWindow* >(m_interface->SearchControl(EVENT_WINDOW0));

    UpdateButton(pw, EVENT_OBJECT_FACTORYwa, m_bBusy);
    UpdateButton(pw, EVENT_OBJECT_FACTORYta, m_bBusy);
    UpdateButton(pw, EVENT_OBJECT_FACTORYfa, m_bBusy);
    UpdateButton(pw, EVENT_OBJECT_FACTORYia, m_bBusy);
    UpdateButton(pw, EVENT_OBJECT_FACTORYws, m_bBusy);
    UpdateButton(pw, EVENT_OBJECT_FACTORYts, m_bBusy);
    UpdateButton(pw, EVENT_OBJECT_FACTORYfs, m_bBusy);
    UpdateButton(pw, EVENT_OBJECT_FACTORYis, m_bBusy);
    UpdateButton(pw, EVENT_OBJECT_FACTORYwc, m_bBusy);
    UpdateButton(pw, EVENT_OBJECT_FACTORYtc, m_bBusy);
    UpdateButton(pw, EVENT_OBJECT_FACTORYfc, m_bBusy);
    UpdateButton(pw, EVENT_OBJECT_FACTORYic, m_bBusy);
    UpdateButton(pw, EVENT_OBJECT_FACTORYwi, m_bBusy);
    UpdateButton(pw, EVENT_OBJECT_FACTORYti, m_bBusy);
    UpdateButton(pw, EVENT_OBJECT_FACTORYfi, m_bBusy);
    UpdateButton(pw, EVENT_OBJECT_FACTORYii, m_bBusy);
    UpdateButton(pw, EVENT_OBJECT_FACTORYrt, m_bBusy);
    UpdateButton(pw, EVENT_OBJECT_FACTORYrc, m_bBusy);
    UpdateButton(pw, EVENT_OBJECT_FACTORYrr, m_bBusy);
    UpdateButton(pw, EVENT_OBJECT_FACTORYrs, m_bBusy);
    UpdateButton(pw, EVENT_OBJECT_FACTORYsa, m_bBusy);
}

// Updates the status of one interface button.

void CAutoFactory::UpdateButton(Ui::CWindow *pw, EventType event, bool bBusy)
{
    EnableInterface(pw, event, !bBusy);
    DeadInterface(pw, event, m_main->CanFactory(ObjectTypeFromFactoryButton(event), m_object->GetTeam()));
}

// Plays the sound of the manipulator arm.

void CAutoFactory::SoundManip(float time, float amplitude, float frequency)
{
    int     i;

    i = m_sound->Play(SOUND_MANIP, m_object->GetPosition(), 0.0f, 0.3f*frequency, true);
    m_sound->AddEnvelope(i, 0.5f*amplitude, 1.0f*frequency, 0.1f, SOPER_CONTINUE);
    m_sound->AddEnvelope(i, 0.5f*amplitude, 1.0f*frequency, time-0.1f, SOPER_CONTINUE);
    m_sound->AddEnvelope(i, 0.0f, 0.3f*frequency, 0.1f, SOPER_STOP);
}
