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


#include "object/old_object.h"

#include "app/app.h"

#include "common/global.h"
#include "common/settings.h"
#include "common/stringutils.h"

#include "graphics/engine/engine.h"
#include "graphics/engine/lightman.h"
#include "graphics/engine/lightning.h"
#include "graphics/engine/particle.h"
#include "graphics/engine/pyro_manager.h"
#include "graphics/engine/terrain.h"

#include "level/robotmain.h"
#include "level/scoreboard.h"

#include "level/parser/parserexceptions.h"
#include "level/parser/parserline.h"
#include "level/parser/parserparam.h"

#include "math/geometry.h"

#include "object/object_manager.h"

#include "object/auto/auto.h"
#include "object/auto/autobase.h"
#include "object/auto/autojostle.h"

#include "object/motion/motion.h"
#include "object/motion/motionvehicle.h"

#include "object/subclass/base_alien.h"
#include "object/subclass/exchange_post.h"

#include "physics/physics.h"

#include "script/cbottoken.h"
#include "script/script.h"
#include "script/scriptfunc.h"

#include "ui/object_interface.h"
#include "ui/studio.h"

#include "ui/controls/edit.h"

#include <iomanip>


const float VIRUS_DELAY     = 60.0f;        // duration of virus infection

// Object's constructor.

COldObject::COldObject(int id)
    : CObject(id, OBJECT_NULL),
      CInteractiveObject(m_implementedInterfaces),
      CTransportableObject(m_implementedInterfaces),
      CTaskExecutorObjectImpl(m_implementedInterfaces, this),
      CProgramStorageObjectImpl(m_implementedInterfaces, this),
      CProgrammableObjectImpl(m_implementedInterfaces, this),
      CJostleableObject(m_implementedInterfaces),
      CSlottedObject(m_implementedInterfaces),
      CJetFlyingObject(m_implementedInterfaces),
      CControllableObject(m_implementedInterfaces),
      CPowerContainerObjectImpl(m_implementedInterfaces),
      CRangedObject(m_implementedInterfaces),
      CTraceDrawingObject(m_implementedInterfaces),
      CShieldedAutoRegenObject(m_implementedInterfaces),
      m_partiSel()
{
    // A bit of a hack since we don't have subclasses yet, set externally in SetProgrammable()
    m_implementedInterfaces[static_cast<int>(ObjectInterfaceType::ProgramStorage)] = false;
    m_implementedInterfaces[static_cast<int>(ObjectInterfaceType::Programmable)] = false;
    // Another hack, see SetMovable()
    m_implementedInterfaces[static_cast<int>(ObjectInterfaceType::Movable)] = false;
    // Another hack
    m_implementedInterfaces[static_cast<int>(ObjectInterfaceType::Jostleable)] = false;

    m_implementedInterfaces[static_cast<int>(ObjectInterfaceType::Old)] = true;

    m_sound       = CApplication::GetInstancePointer()->GetSound();
    m_engine      = Gfx::CEngine::GetInstancePointer();
    m_lightMan    = m_engine->GetLightManager();
    m_particle    = m_engine->GetParticle();
    m_main        = CRobotMain::GetInstancePointer();
    m_terrain     = m_main->GetTerrain();
    m_camera      = m_main->GetCamera();

    m_type = OBJECT_NULL;
    m_option = 0;
    m_name = "";
    m_shadowLight   = -1;
    m_shadowHeight  = 0.0f;
    m_linVibration  = glm::vec3(0.0f, 0.0f, 0.0f);
    m_cirVibration  = glm::vec3(0.0f, 0.0f, 0.0f);
    m_tilt   = glm::vec3(0.0f, 0.0f, 0.0f);

    m_power = nullptr;
    m_cargo  = nullptr;
    m_transporter = nullptr;
    m_transporterLink = 0;
    m_shield   = 1.0f;
    m_range    = 30.0f;
    m_lastEnergy = 999.9f;
    m_bSelect = false;
    m_bSelectable = true;
    m_bCheckToken = true;
    m_underground = false;
    m_bTrainer = false;
    m_bToy = false;
    m_bManual = false;
    m_aTime = 0.0f;
    m_shotTime = 0.0f;
    m_bVirusMode = false;
    m_virusTime = 0.0f;
    m_lastVirusParticle = 0.0f;
    m_damaging = false;
    m_damageTime = 0.0f;
    m_dying = DeathType::Alive;
    m_bFlat  = false;
    m_gunGoalV = 0.0f;
    m_gunGoalH = 0.0f;
    m_shieldRadius = 0.0f;
    m_magnifyDamage = 1.0f;
    m_hasPowerSlot = false;
    m_hasCargoSlot = false;

    m_character = Character();
    m_character.wheelFront = 1.0f;
    m_character.wheelBack  = 1.0f;
    m_character.wheelLeft  = 1.0f;
    m_character.wheelRight = 1.0f;

    m_cameraType = Gfx::CAM_TYPE_BACK;
    m_bCameraLock = false;

    for (int i=0 ; i<OBJECTMAXPART ; i++ )
    {
        m_objectPart[i].bUsed = false;
    }
    m_totalPart = 0;

    for (int i=0 ; i<4 ; i++ )
    {
        m_partiSel[i] = -1;
    }

    m_time = 0.0f;
    m_burnTime = 0.0f;

    m_buttonAxe    = EVENT_NULL;

    m_reactorRange       = 1.0f;

    m_traceDown = false;
    m_traceColor = TraceColor::Black;
    m_traceWidth = 0.5f;

    DeleteAllCrashSpheres();
}

// Object's destructor.

COldObject::~COldObject()
{
    m_main->HideDropZone(this);
}


// Removes an object.
// If bAll = true, it does not help,
// because all objects in the scene are quickly destroyed!

void COldObject::DeleteObject(bool bAll)
{
    CScriptFunctions::DestroyObjectVar(m_botVar, false);

    if ( m_camera->GetControllingObject() == this )
    {
        m_camera->SetControllingObject(nullptr);
    }
    m_main->RemoveFromSelectionHistory(this);

    if ( !bAll )
    {
        m_engine->GetPyroManager()->CutObjectLink(this);
        m_particle->CutObjectLink(this);

        if ( m_bSelect )
        {
            SetSelect(false);
        }

        if ( m_type == OBJECT_BASE     ||
             m_type == OBJECT_FACTORY  ||
             m_type == OBJECT_REPAIR   ||
             m_type == OBJECT_DESTROYER||
             m_type == OBJECT_DERRICK  ||
             m_type == OBJECT_STATION  ||
             m_type == OBJECT_CONVERT  ||
             m_type == OBJECT_TOWER    ||
             m_type == OBJECT_RESEARCH ||
             m_type == OBJECT_RADAR    ||
             m_type == OBJECT_INFO     ||
             m_type == OBJECT_ENERGY   ||
             m_type == OBJECT_LABO     ||
             m_type == OBJECT_NUCLEAR  ||
             m_type == OBJECT_PARA     ||
             m_type == OBJECT_SAFE     ||
             m_type == OBJECT_HUSTON   ||
             m_type == OBJECT_START    ||
             m_type == OBJECT_END      )  // building?
        {
            m_terrain->DeleteBuildingLevel(GetPosition());  // flattens the field
        }
    }

    m_type = OBJECT_NULL;  // invalid object until complete destruction

    if ( m_shadowLight != -1 )
    {
        m_lightMan->DeleteLight(m_shadowLight);
        m_shadowLight = -1;
    }

    if ( m_physics != nullptr )
    {
        m_physics->DeleteObject(bAll);
    }

    if ( m_objectInterface != nullptr )
    {
        m_objectInterface->DeleteObject(bAll);
    }

    if ( m_motion != nullptr )
    {
        m_motion->DeleteObject(bAll);
    }

    if ( m_auto != nullptr )
    {
        m_auto->DeleteObject(bAll);
    }

    for (int i=0 ; i<OBJECTMAXPART ; i++ )
    {
        if ( m_objectPart[i].bUsed )
        {
            m_objectPart[i].bUsed = false;
            m_engine->DeleteObject(m_objectPart[i].object);

            if ( m_objectPart[i].masterParti != -1 )
            {
                m_particle->DeleteParticle(m_objectPart[i].masterParti);
                m_objectPart[i].masterParti = -1;
            }
        }
    }

    if (!bAll)
    {
        if (m_power != nullptr)
        {
            if (m_power->Implements(ObjectInterfaceType::Old))
            {
                dynamic_cast<COldObject&>(*m_power).SetTransporter(nullptr);
                dynamic_cast<COldObject&>(*m_power).DeleteObject(bAll);
            }
            m_power = nullptr;
        }
        if (m_cargo != nullptr)
        {
            if (m_cargo->Implements(ObjectInterfaceType::Old))
            {
                dynamic_cast<COldObject&>(*m_cargo).SetTransporter(nullptr);
                dynamic_cast<COldObject&>(*m_cargo).DeleteObject(bAll);
            }
            m_cargo = nullptr;
        }
    }

    if ( !bAll )  m_main->CreateShortcuts();
}

// Simplifies a object (destroys all logic classes, making it a static object)

void COldObject::Simplify()
{
    if ( Implements(ObjectInterfaceType::Programmable) )
    {
        StopProgram();
    }
    m_main->SaveOneScript(this);

    StopForegroundTask();
    StopBackgroundTask();

    m_implementedInterfaces[static_cast<int>(ObjectInterfaceType::ProgramStorage)] = false;
    m_implementedInterfaces[static_cast<int>(ObjectInterfaceType::Programmable)] = false;

    if ( m_physics != nullptr )
    {
        m_physics->DeleteObject();
        m_physics.reset();
    }
    if ( m_motion != nullptr )
    {
        m_motion->DeleteObject();
        m_motion.reset();
    }
    m_implementedInterfaces[static_cast<int>(ObjectInterfaceType::Movable)] = false;

    if ( m_objectInterface != nullptr )
    {
        m_objectInterface->DeleteObject();
        m_objectInterface.reset();
    }

    if ( m_auto != nullptr )
    {
        m_auto->DeleteObject();
        m_auto.reset();
    }

    m_main->CreateShortcuts();
}


bool COldObject::DamageObject(DamageType type, float force, CObject* killer)
{
    assert(Implements(ObjectInterfaceType::Damageable));
    assert(!Implements(ObjectInterfaceType::Destroyable) || Implements(ObjectInterfaceType::Shielded) || Implements(ObjectInterfaceType::Fragile));

    if ( IsDying() )  return false;
    if ( Implements(ObjectInterfaceType::Jostleable) ) return false;

    if ( m_type == OBJECT_ANT    ||
         m_type == OBJECT_WORM   ||
         m_type == OBJECT_SPIDER ||
         m_type == OBJECT_BEE     )
    {
        // Fragile, but can have fire effect
        // TODO: IsBurnable()
        force = -1.0f;
    }
    else if ( Implements(ObjectInterfaceType::Fragile) )
    {
        if ((m_type == OBJECT_BOMB         ||
             m_type == OBJECT_RUINmobilew1 ||
             m_type == OBJECT_RUINmobilew2 ||
             m_type == OBJECT_RUINmobilet1 ||
             m_type == OBJECT_RUINmobilet2 ||
             m_type == OBJECT_RUINmobiler1 ||
             m_type == OBJECT_RUINmobiler2 ||
             m_type == OBJECT_RUINfactory  ||
             m_type == OBJECT_RUINdoor     ||
             m_type == OBJECT_RUINsupport  ||
             m_type == OBJECT_RUINradar    ||
             m_type == OBJECT_RUINconvert   ) && type != DamageType::Explosive ) return false; // Mines and ruins can't be destroyed by shooting
        if ( m_type == OBJECT_URANIUM && (type == DamageType::Fire || type == DamageType::Organic) ) return false; // UraniumOre is not destroyable by shooting or aliens (see #777)
        if ( m_type == OBJECT_STONE && (type == DamageType::Fire || type == DamageType::Organic) ) return false; // TitaniumOre is not destroyable either
        // PowerCell, NuclearCell and Titanium are destroyable by shooting, but not by collisions!
        if ( m_type == OBJECT_METAL && type == DamageType::Collision ) return false;
        if ( m_type == OBJECT_POWER && type == DamageType::Collision ) return false;
        if ( m_type == OBJECT_NUCLEAR && type == DamageType::Collision ) return false;

        if ( m_magnifyDamage * m_main->GetGlobalMagnifyDamage() == 0 ) return false; // Don't destroy if magnifyDamage=0

        DestroyObject(DestructionType::Explosion, killer);
        return true;
    }

    if ( type != DamageType::Phazer && m_type == OBJECT_MOTHER ) return false; // AlienQueen can be destroyed only by PhazerShooter

    if ( type == DamageType::Organic )
    {
        // TODO: I don't understand, why does it apply damage only once every 0.5 second?
        if ( m_shotTime < 0.5f )  return false;
        m_shotTime = 0.0f;
    }

    float loss = 1.0f;
    bool dead = true;
    if (Implements(ObjectInterfaceType::Shielded))
    {
        float magnifyDamage = m_magnifyDamage * m_main->GetGlobalMagnifyDamage();
        if (force != std::numeric_limits<float>::infinity())
        {
            // Calculate the shield lost by the explosion
            loss = force * magnifyDamage;
            if (m_type == OBJECT_HUMAN) loss /= 2.5f; // Me is more resistant
            if (loss > 1.0f) loss = 1.0f;

            // Decreases the the shield
            float shield = GetShield();
            shield -= loss;
            SetShield(shield);

            // Sending info about taking damage
            if (!m_damaging)
            {
                SetDamaging(true);
                m_main->UpdateShortcuts();
            }
            m_damageTime = m_time;
        }
        else
        {
            if ( magnifyDamage != 0.0f )
            {
                // Dead immediately
                SetShield(0.0f);
                SetDamaging(false);
            }
        }
        dead = (GetShield() <= 0.0f);
    }

    if (dead && Implements(ObjectInterfaceType::Destroyable))
    {
        if (type == DamageType::Fire)
        {
            DestroyObject(DestructionType::Burn, killer);
        }
        else
        {
            DestroyObject(DestructionType::Explosion, killer);
        }
        return true;
    }

    if ( m_type == OBJECT_HUMAN )
    {
        m_engine->GetPyroManager()->Create(Gfx::PT_SHOTH, this, loss);
    }
    else if ( m_type == OBJECT_MOTHER )
    {
        m_engine->GetPyroManager()->Create(Gfx::PT_SHOTM, this, loss);
    }
    else
    {
        m_engine->GetPyroManager()->Create(Gfx::PT_SHOTT, this, loss);
    }
    return false;
}

void COldObject::DestroyObject(DestructionType type, CObject* killer)
{
    assert(Implements(ObjectInterfaceType::Destroyable));

    if(type == DestructionType::NoEffect) assert(!!"DestructionType::NoEffect should not be passed to DestroyObject()!");
    assert(type != DestructionType::Drowned || m_type == OBJECT_HUMAN);

    if ( IsDying() )  return;

    if (Implements(ObjectInterfaceType::Shielded))
    {
        SetShield(0.0f);
        SetDamaging(false);
    }

    Gfx::PyroType pyroType = Gfx::PT_NULL;
    if ( type == DestructionType::Explosion )   // explosion?
    {
        if ( m_type == OBJECT_ANT    ||
             m_type == OBJECT_SPIDER ||
             m_type == OBJECT_BEE    ||
             m_type == OBJECT_WORM   )
        {
            pyroType = Gfx::PT_EXPLOO;
        }
        else if ( m_type == OBJECT_MOTHER ||
                  m_type == OBJECT_NEST   ||
                  m_type == OBJECT_BULLET )
        {
            pyroType = Gfx::PT_FRAGO;
        }
        else if ( m_type == OBJECT_HUMAN )
        {
            pyroType = Gfx::PT_DEADG;
        }
        else if ( m_type == OBJECT_BASE     ||
                  m_type == OBJECT_DERRICK  ||
                  m_type == OBJECT_FACTORY  ||
                  m_type == OBJECT_STATION  ||
                  m_type == OBJECT_CONVERT  ||
                  m_type == OBJECT_REPAIR   ||
                  m_type == OBJECT_DESTROYER||
                  m_type == OBJECT_TOWER    ||
                  m_type == OBJECT_NEST     ||
                  m_type == OBJECT_RESEARCH ||
                  m_type == OBJECT_RADAR    ||
                  m_type == OBJECT_INFO     ||
                  m_type == OBJECT_ENERGY   ||
                  m_type == OBJECT_LABO     ||
                  m_type == OBJECT_NUCLEAR  ||
                  m_type == OBJECT_PARA     ||
                  m_type == OBJECT_SAFE     ||
                  m_type == OBJECT_HUSTON   ||
                  m_type == OBJECT_START    ||
                  m_type == OBJECT_END      ||
                  m_type == OBJECT_RUINfactory ||
                  m_type == OBJECT_RUINdoor    ||
                  m_type == OBJECT_RUINsupport ||
                  m_type == OBJECT_RUINradar   ||
                  m_type == OBJECT_RUINconvert  )  // building?
        {
            pyroType = Gfx::PT_FRAGT;
        }
        else if ( m_type == OBJECT_MOBILEtg )
        {
            pyroType = Gfx::PT_FRAGT;
        }
        else
        {
            pyroType = Gfx::PT_EXPLOT;
        }
    }
    else if ( type == DestructionType::ExplosionWater )
    {
        pyroType = Gfx::PT_FRAGW;
    }
    else if ( type == DestructionType::Burn )  // burning?
    {
        if ( m_type == OBJECT_MOTHER ||
             m_type == OBJECT_ANT    ||
             m_type == OBJECT_SPIDER ||
             m_type == OBJECT_BEE    ||
             m_type == OBJECT_WORM   ||
             m_type == OBJECT_BULLET )
        {
            pyroType = Gfx::PT_BURNO;
            SetDying(DeathType::Burning);
        }
        else if ( m_type == OBJECT_HUMAN )
        {
            pyroType = Gfx::PT_DEADG;
        }
        else
        {
            pyroType = Gfx::PT_BURNT;
            SetDying(DeathType::Burning);
        }
        SetVirusMode(false);
    }
    else if ( type == DestructionType::Drowned )
    {
        pyroType = Gfx::PT_DEADW;
    }
    else if ( type == DestructionType::Win )
    {
        pyroType = Gfx::PT_WPCHECK;
    }
    else if ( type == DestructionType::Squash )
    {
        pyroType = Gfx::PT_SQUASH;
        DeleteAllCrashSpheres();
    }
    assert(pyroType != Gfx::PT_NULL);
    if (pyroType == Gfx::PT_FRAGT ||
        pyroType == Gfx::PT_FRAGO ||
        pyroType == Gfx::PT_FRAGW)
    {
        SetDying(DeathType::Exploding);
    }
    m_engine->GetPyroManager()->Create(pyroType, this);

    if ( Implements(ObjectInterfaceType::Programmable) )
    {
        StopProgram();
    }
    m_main->SaveOneScript(this);

    if ( GetSelect() )
    {
        SetSelect(false);  // deselects the object
        m_camera->SetType(Gfx::CAM_TYPE_EXPLO);
        m_main->DeselectAll();
    }
    m_main->RemoveFromSelectionHistory(this);

    CScoreboard* scoreboard = m_main->GetScoreboard();
    if (scoreboard)
        scoreboard->ProcessKill(this, killer);

    m_team = 0; // Back to neutral on destruction

    if ( m_botVar != nullptr )
    {
        if ( Implements(ObjectInterfaceType::Transportable) )  // (*)
        {
            CScriptFunctions::DestroyObjectVar(m_botVar, false);
        }
    }
}

// (*)  If a robot or cosmonaut dies, the subject must continue to exist,
//  so that programs of the ants continue to operate as usual.


// Initializes a new part.

void COldObject::InitPart(int part)
{
    m_objectPart[part].bUsed      = true;
    m_objectPart[part].object     = -1;
    m_objectPart[part].parentPart = -1;

    m_objectPart[part].position   = glm::vec3(0.0f, 0.0f, 0.0f);
    m_objectPart[part].angle.y    = 0.0f;
    m_objectPart[part].angle.x    = 0.0f;
    m_objectPart[part].angle.z    = 0.0f;
    m_objectPart[part].zoom       = glm::vec3(1.0f, 1.0f, 1.0f);

    m_objectPart[part].bTranslate = true;
    m_objectPart[part].bRotate    = true;
    m_objectPart[part].bZoom      = false;

    m_objectPart[part].matTranslate = glm::mat4(1.0f);
    m_objectPart[part].matRotate = glm::mat4(1.0f);
    m_objectPart[part].matTransform = glm::mat4(1.0f);
    m_objectPart[part].matWorld = glm::mat4(1.0f);

    m_objectPart[part].masterParti = -1;
}

// Removes part.

void COldObject::DeletePart(int part)
{
    if ( !m_objectPart[part].bUsed )  return;

    if ( m_objectPart[part].masterParti != -1 )
    {
        m_particle->DeleteParticle(m_objectPart[part].masterParti);
        m_objectPart[part].masterParti = -1;
    }

    m_objectPart[part].bUsed = false;
    m_engine->DeleteObject(m_objectPart[part].object);
    UpdateTotalPart();
}

void COldObject::UpdateTotalPart()
{
    int     i;

    m_totalPart = 0;
    for ( i=0 ; i<OBJECTMAXPART ; i++ )
    {
        if ( m_objectPart[i].bUsed )
        {
            m_totalPart = i+1;
        }
    }
}


// Specifies the number of the object of a part.

void COldObject::SetObjectRank(int part, int objRank)
{
    if ( !m_objectPart[part].bUsed )  // object not created?
    {
        InitPart(part);
        UpdateTotalPart();
    }
    m_objectPart[part].object = objRank;
}

// Returns the number of part.

int COldObject::GetObjectRank(int part)
{
    if ( !m_objectPart[part].bUsed )  return -1;
    return m_objectPart[part].object;
}

// Specifies what is the parent of a part.
// Reminder: Part 0 is always the father of all
// and therefore the main part (eg the chassis of a car).

void COldObject::SetObjectParent(int part, int parent)
{
    m_objectPart[part].parentPart = parent;
}


// Specifies the type of the object.

void COldObject::SetType(ObjectType type)
{
    m_type = type;
    m_name = GetObjectName(m_type);

    SetSelectable(IsSelectableByDefault(m_type));

    // TODO: Temporary hack
    if ( m_type == OBJECT_MOBILEfa || // WingedGrabber
         m_type == OBJECT_MOBILEfb || // WingedBuilder
         m_type == OBJECT_MOBILEfs || // WingedSniffer
         m_type == OBJECT_MOBILEfc || // WingedShooter
         m_type == OBJECT_MOBILEfi || // WingedOrgaShooter
         m_type == OBJECT_MOBILEft || // WingedTrainer
         m_type == OBJECT_HUMAN    || // Me
         m_type == OBJECT_TECH     || // Tech
         m_type == OBJECT_CONTROLLER)
    {
        m_implementedInterfaces[static_cast<int>(ObjectInterfaceType::Flying)] = true;
        m_implementedInterfaces[static_cast<int>(ObjectInterfaceType::JetFlying)] = true;
    }
    else if ( m_type == OBJECT_BEE )
    {
        m_implementedInterfaces[static_cast<int>(ObjectInterfaceType::Flying)] = true;
        m_implementedInterfaces[static_cast<int>(ObjectInterfaceType::JetFlying)] = false;
    }
    else
    {
        m_implementedInterfaces[static_cast<int>(ObjectInterfaceType::Flying)] = false;
        m_implementedInterfaces[static_cast<int>(ObjectInterfaceType::JetFlying)] = false;
    }

    // TODO: Another temporary hack
    if (m_type == OBJECT_MOBILEfa ||
        m_type == OBJECT_MOBILEta ||
        m_type == OBJECT_MOBILEwa ||
        m_type == OBJECT_MOBILEia ||
        m_type == OBJECT_MOBILEfb ||
        m_type == OBJECT_MOBILEtb ||
        m_type == OBJECT_MOBILEwb ||
        m_type == OBJECT_MOBILEib ||
        m_type == OBJECT_MOBILEfc ||
        m_type == OBJECT_MOBILEtc ||
        m_type == OBJECT_MOBILEwc ||
        m_type == OBJECT_MOBILEic ||
        m_type == OBJECT_MOBILEfi ||
        m_type == OBJECT_MOBILEti ||
        m_type == OBJECT_MOBILEwi ||
        m_type == OBJECT_MOBILEii ||
        m_type == OBJECT_MOBILEfs ||
        m_type == OBJECT_MOBILEts ||
        m_type == OBJECT_MOBILEws ||
        m_type == OBJECT_MOBILEis ||
        m_type == OBJECT_MOBILErt ||
        m_type == OBJECT_MOBILErc ||
        m_type == OBJECT_MOBILErr ||
        m_type == OBJECT_MOBILErs ||
        m_type == OBJECT_MOBILEsa ||
        m_type == OBJECT_MOBILEtg ||
        m_type == OBJECT_MOBILEft ||
        m_type == OBJECT_MOBILEtt ||
        m_type == OBJECT_MOBILEwt ||
        m_type == OBJECT_MOBILEit ||
        m_type == OBJECT_MOBILErp ||
        m_type == OBJECT_MOBILEst ||
        m_type == OBJECT_TOWER    ||
        m_type == OBJECT_RESEARCH ||
        m_type == OBJECT_ENERGY   || // TODO not actually a power cell slot
        m_type == OBJECT_LABO     || // TODO not actually a power cell slot
        m_type == OBJECT_NUCLEAR  ) // TODO not actually a power cell slot
    {
        m_hasPowerSlot = true;
    }
    else
    {
        m_hasPowerSlot = false;
    }

    if ( m_type == OBJECT_HUMAN ||
         m_type == OBJECT_TECH ||
         m_type == OBJECT_MOBILEfa || // Grabbers
         m_type == OBJECT_MOBILEta ||
         m_type == OBJECT_MOBILEwa ||
         m_type == OBJECT_MOBILEia ||
         m_type == OBJECT_MOBILEsa || // subber
         m_type == OBJECT_BEE)
    {
        m_hasCargoSlot = true;
    }
    else
    {
        m_hasCargoSlot = false;
    }

    m_implementedInterfaces[static_cast<int>(ObjectInterfaceType::Slotted)] = (m_hasPowerSlot || m_hasCargoSlot);

    // TODO: Hacking some more
    if ( m_type == OBJECT_MOBILEtg ||
         m_type == OBJECT_STONE    ||
         m_type == OBJECT_METAL    ||
         m_type == OBJECT_URANIUM  ||
         m_type == OBJECT_POWER    ||
         m_type == OBJECT_ATOMIC   ||
         m_type == OBJECT_TNT      ||
         m_type == OBJECT_BULLET   ||
         m_type == OBJECT_EGG      ||
         m_type == OBJECT_BOMB     ||
         m_type == OBJECT_ANT      ||
         m_type == OBJECT_WORM     ||
         m_type == OBJECT_SPIDER   ||
         m_type == OBJECT_BEE      ||
         m_type == OBJECT_TEEN28    )
    {
        m_implementedInterfaces[static_cast<int>(ObjectInterfaceType::Damageable)] = true;
        m_implementedInterfaces[static_cast<int>(ObjectInterfaceType::Destroyable)] = true;
        m_implementedInterfaces[static_cast<int>(ObjectInterfaceType::Fragile)] = true;
        m_implementedInterfaces[static_cast<int>(ObjectInterfaceType::Shielded)] = false;
    }
    else if (m_type == OBJECT_HUMAN ||
         m_type == OBJECT_MOBILEfa ||
         m_type == OBJECT_MOBILEta ||
         m_type == OBJECT_MOBILEwa ||
         m_type == OBJECT_MOBILEia ||
         m_type == OBJECT_MOBILEfb ||
         m_type == OBJECT_MOBILEtb ||
         m_type == OBJECT_MOBILEwb ||
         m_type == OBJECT_MOBILEib ||
         m_type == OBJECT_MOBILEfc ||
         m_type == OBJECT_MOBILEtc ||
         m_type == OBJECT_MOBILEwc ||
         m_type == OBJECT_MOBILEic ||
         m_type == OBJECT_MOBILEfi ||
         m_type == OBJECT_MOBILEti ||
         m_type == OBJECT_MOBILEwi ||
         m_type == OBJECT_MOBILEii ||
         m_type == OBJECT_MOBILEfs ||
         m_type == OBJECT_MOBILEts ||
         m_type == OBJECT_MOBILEws ||
         m_type == OBJECT_MOBILEis ||
         m_type == OBJECT_MOBILErt ||
         m_type == OBJECT_MOBILErc ||
         m_type == OBJECT_MOBILErr ||
         m_type == OBJECT_MOBILErs ||
         m_type == OBJECT_MOBILEsa ||
         m_type == OBJECT_MOBILEft ||
         m_type == OBJECT_MOBILEtt ||
         m_type == OBJECT_MOBILEwt ||
         m_type == OBJECT_MOBILEit ||
         m_type == OBJECT_MOBILErp ||
         m_type == OBJECT_MOBILEst ||
         m_type == OBJECT_FACTORY  ||
         m_type == OBJECT_REPAIR   ||
         m_type == OBJECT_DESTROYER||
         m_type == OBJECT_DERRICK  ||
         m_type == OBJECT_STATION  ||
         m_type == OBJECT_CONVERT  ||
         m_type == OBJECT_TOWER    ||
         m_type == OBJECT_RESEARCH ||
         m_type == OBJECT_RADAR    ||
         m_type == OBJECT_INFO     ||
         m_type == OBJECT_ENERGY   ||
         m_type == OBJECT_LABO     ||
         m_type == OBJECT_NUCLEAR  ||
         m_type == OBJECT_PARA     ||
         m_type == OBJECT_MOTHER    )
    {
        m_implementedInterfaces[static_cast<int>(ObjectInterfaceType::Damageable)] = true;
        m_implementedInterfaces[static_cast<int>(ObjectInterfaceType::Destroyable)] = true;
        m_implementedInterfaces[static_cast<int>(ObjectInterfaceType::Fragile)] = false;
        m_implementedInterfaces[static_cast<int>(ObjectInterfaceType::Shielded)] = true;
    }
    else if (m_type == OBJECT_HUSTON ||
             m_type == OBJECT_BASE    )
    {
        m_implementedInterfaces[static_cast<int>(ObjectInterfaceType::Damageable)] = true;
        m_implementedInterfaces[static_cast<int>(ObjectInterfaceType::Destroyable)] = false;
        m_implementedInterfaces[static_cast<int>(ObjectInterfaceType::Fragile)] = false;
        m_implementedInterfaces[static_cast<int>(ObjectInterfaceType::Shielded)] = false;
    }
    else if (m_type == OBJECT_RUINmobilew1 ||
             m_type == OBJECT_RUINmobilew2 ||
             m_type == OBJECT_RUINmobilet1 ||
             m_type == OBJECT_RUINmobilet2 ||
             m_type == OBJECT_RUINmobiler1 ||
             m_type == OBJECT_RUINmobiler2 ||
             m_type == OBJECT_RUINfactory  ||
             m_type == OBJECT_RUINdoor     ||
             m_type == OBJECT_RUINsupport  ||
             m_type == OBJECT_RUINradar    ||
             m_type == OBJECT_RUINconvert   )
    {
        m_implementedInterfaces[static_cast<int>(ObjectInterfaceType::Damageable)] = true;
        m_implementedInterfaces[static_cast<int>(ObjectInterfaceType::Destroyable)] = true;
        m_implementedInterfaces[static_cast<int>(ObjectInterfaceType::Fragile)] = true;
        m_implementedInterfaces[static_cast<int>(ObjectInterfaceType::Shielded)] = false;
    }
    else if (m_type == OBJECT_PLANT0  ||
             m_type == OBJECT_PLANT1  ||
             m_type == OBJECT_PLANT2  ||
             m_type == OBJECT_PLANT3  ||
             m_type == OBJECT_PLANT4  ||
             m_type == OBJECT_PLANT15 ||
             m_type == OBJECT_PLANT16 ||
             m_type == OBJECT_PLANT17 ||
             m_type == OBJECT_PLANT18 )
    {
        m_implementedInterfaces[static_cast<int>(ObjectInterfaceType::Damageable)] = true;
        m_implementedInterfaces[static_cast<int>(ObjectInterfaceType::Destroyable)] = true;
        m_implementedInterfaces[static_cast<int>(ObjectInterfaceType::Fragile)] = true;
        m_implementedInterfaces[static_cast<int>(ObjectInterfaceType::Shielded)] = false;
    }
    else
    {
        m_implementedInterfaces[static_cast<int>(ObjectInterfaceType::Damageable)] = false;
        m_implementedInterfaces[static_cast<int>(ObjectInterfaceType::Destroyable)] = false;
        m_implementedInterfaces[static_cast<int>(ObjectInterfaceType::Fragile)] = false;
        m_implementedInterfaces[static_cast<int>(ObjectInterfaceType::Shielded)] = false;
    }

    // TODO: #TooMuchHacking
    m_implementedInterfaces[static_cast<int>(ObjectInterfaceType::ShieldedAutoRegen)] = (m_type == OBJECT_HUMAN);

    // TODO: Hacking in progress...
    if ( m_type == OBJECT_STONE   ||
         m_type == OBJECT_URANIUM ||
         m_type == OBJECT_BULLET  ||
         m_type == OBJECT_METAL   ||
         m_type == OBJECT_POWER   ||
         m_type == OBJECT_ATOMIC  ||
         m_type == OBJECT_BBOX    ||
         m_type == OBJECT_KEYa    ||
         m_type == OBJECT_KEYb    ||
         m_type == OBJECT_KEYc    ||
         m_type == OBJECT_KEYd    ||
         m_type == OBJECT_TNT     )
    {
        m_implementedInterfaces[static_cast<int>(ObjectInterfaceType::Transportable)] = true;
    }
    else
    {
        m_implementedInterfaces[static_cast<int>(ObjectInterfaceType::Transportable)] = false;
    }

    // TODO: You have been hacked!
    if (m_type == OBJECT_HUMAN    ||
        m_type == OBJECT_TOTO     ||
        m_type == OBJECT_MOBILEfa ||
        m_type == OBJECT_MOBILEta ||
        m_type == OBJECT_MOBILEwa ||
        m_type == OBJECT_MOBILEia ||
        m_type == OBJECT_MOBILEfb ||
        m_type == OBJECT_MOBILEtb ||
        m_type == OBJECT_MOBILEwb ||
        m_type == OBJECT_MOBILEib ||
        m_type == OBJECT_MOBILEfc ||
        m_type == OBJECT_MOBILEtc ||
        m_type == OBJECT_MOBILEwc ||
        m_type == OBJECT_MOBILEic ||
        m_type == OBJECT_MOBILEfi ||
        m_type == OBJECT_MOBILEti ||
        m_type == OBJECT_MOBILEwi ||
        m_type == OBJECT_MOBILEii ||
        m_type == OBJECT_MOBILEfs ||
        m_type == OBJECT_MOBILEts ||
        m_type == OBJECT_MOBILEws ||
        m_type == OBJECT_MOBILEis ||
        m_type == OBJECT_MOBILErt ||
        m_type == OBJECT_MOBILErc ||
        m_type == OBJECT_MOBILErr ||
        m_type == OBJECT_MOBILErs ||
        m_type == OBJECT_MOBILEsa ||
        m_type == OBJECT_MOBILEft ||
        m_type == OBJECT_MOBILEtt ||
        m_type == OBJECT_MOBILEwt ||
        m_type == OBJECT_MOBILEit ||
        m_type == OBJECT_MOBILErp ||
        m_type == OBJECT_MOBILEst ||
        m_type == OBJECT_MOBILEtg ||
        m_type == OBJECT_MOBILEdr ||
        m_type == OBJECT_APOLLO2  ||
        m_type == OBJECT_BASE     ||
        m_type == OBJECT_DERRICK  ||
        m_type == OBJECT_FACTORY  ||
        m_type == OBJECT_REPAIR   ||
        m_type == OBJECT_DESTROYER||
        m_type == OBJECT_STATION  ||
        m_type == OBJECT_CONVERT  ||
        m_type == OBJECT_TOWER    ||
        m_type == OBJECT_RESEARCH ||
        m_type == OBJECT_RADAR    ||
        m_type == OBJECT_INFO     ||
        m_type == OBJECT_ENERGY   ||
        m_type == OBJECT_LABO     ||
        m_type == OBJECT_NUCLEAR  ||
        m_type == OBJECT_PARA     ||
        m_type == OBJECT_SAFE     ||
        m_type == OBJECT_HUSTON   ||
        m_type == OBJECT_ANT      ||
        m_type == OBJECT_WORM     ||
        m_type == OBJECT_SPIDER   ||
        m_type == OBJECT_BEE      ||
        m_type == OBJECT_MOTHER   ||
        m_type == OBJECT_CONTROLLER)
    {
        m_implementedInterfaces[static_cast<int>(ObjectInterfaceType::Controllable)] = true;
    }
    else
    {
        m_implementedInterfaces[static_cast<int>(ObjectInterfaceType::Controllable)] = false;
    }

    // TODO: Another one? :/
    if ( m_type == OBJECT_POWER   || // PowerCell
         m_type == OBJECT_ATOMIC  || // NuclearCell
         m_type == OBJECT_STATION || // PowerStation
         m_type == OBJECT_ENERGY   ) // PowerPlant
    {
        m_implementedInterfaces[static_cast<int>(ObjectInterfaceType::PowerContainer)] = true;
    }
    else
    {
        m_implementedInterfaces[static_cast<int>(ObjectInterfaceType::PowerContainer)] = false;
    }


    if ( m_type == OBJECT_MOBILEwc ||
         m_type == OBJECT_MOBILEtc ||
         m_type == OBJECT_MOBILEfc ||
         m_type == OBJECT_MOBILEic ||
         m_type == OBJECT_MOBILEwi ||
         m_type == OBJECT_MOBILEti ||
         m_type == OBJECT_MOBILEfi ||
         m_type == OBJECT_MOBILEii ||
         m_type == OBJECT_MOBILErc )  // cannon vehicle?
    {
        m_cameraType = Gfx::CAM_TYPE_ONBOARD;
    }
}

const char* COldObject::GetName()
{
    return m_name.c_str();
}

// Choosing the option to use.

void COldObject::SetOption(int option)
{
    m_option = option;
}

int COldObject::GetOption()
{
    return m_option;
}


// Saves all the parameters of the object.

void COldObject::Write(CLevelParserLine* line)
{
    line->AddParam("camera", std::make_unique<CLevelParserParam>(GetCameraType()));

    if ( GetCameraLock() )
        line->AddParam("cameraLock", std::make_unique<CLevelParserParam>(GetCameraLock()));

    if ( IsBulletWall() )
        line->AddParam("bulletWall", std::make_unique<CLevelParserParam>(IsBulletWall()));

    if ( GetEnergyLevel() != 0.0f )
        line->AddParam("energy", std::make_unique<CLevelParserParam>(GetEnergyLevel()));

    if ( GetShield() != 1.0f )
        line->AddParam("shield", std::make_unique<CLevelParserParam>(GetShield()));

    if ( GetRange() != 1.0f )
        line->AddParam("range", std::make_unique<CLevelParserParam>(GetRange()));

    if ( !GetSelectable() )
        line->AddParam("selectable", std::make_unique<CLevelParserParam>(GetSelectable()));

    if ( !GetCollisions() )
        line->AddParam("clip", std::make_unique<CLevelParserParam>(GetCollisions()));

    if ( GetLockForSave() )
        line->AddParam("lock", std::make_unique<CLevelParserParam>(true));

    if ( !GetActivity() )
        line->AddParam("activity", std::make_unique<CLevelParserParam>(GetActivity()));

    if ( GetProxyActivate() )
    {
        line->AddParam("proxyActivate", std::make_unique<CLevelParserParam>(GetProxyActivate()));
        line->AddParam("proxyDistance", std::make_unique<CLevelParserParam>(GetProxyDistance()/g_unit));
    }

    if ( GetMagnifyDamage() != 1.0f )
        line->AddParam("magnifyDamage", std::make_unique<CLevelParserParam>(GetMagnifyDamage()));

    if ( GetTeam() != 0 )
        line->AddParam("team", std::make_unique<CLevelParserParam>(GetTeam()));

    if ( GetGunGoalV() != 0.0f )
        line->AddParam("aimV", std::make_unique<CLevelParserParam>(GetGunGoalV()));

    if ( GetGunGoalH() != 0.0f )
        line->AddParam("aimH", std::make_unique<CLevelParserParam>(GetGunGoalH()));

    if ( GetAnimateOnReset() )
    {
        line->AddParam("reset", std::make_unique<CLevelParserParam>(GetAnimateOnReset()));
    }

    if ( m_bVirusMode )
        line->AddParam("virusMode", std::make_unique<CLevelParserParam>(m_bVirusMode));

    if ( m_virusTime != 0.0f )
        line->AddParam("virusTime", std::make_unique<CLevelParserParam>(m_virusTime));

    line->AddParam("lifetime", std::make_unique<CLevelParserParam>(m_aTime));

    // Sets the parameters of the command line.
    CLevelParserParamVec cmdline;
    for(float value : GetCmdLine())
    {
        cmdline.push_back(std::make_unique<CLevelParserParam>(value));
    }
    if (cmdline.size() > 0)
        line->AddParam("cmdline", std::make_unique<CLevelParserParam>(std::move(cmdline)));

    if ( m_motion != nullptr )
    {
        m_motion->Write(line);
    }

    if ( Implements(ObjectInterfaceType::Programmable) )
    {
        line->AddParam("bVirusActive", std::make_unique<CLevelParserParam>(GetActiveVirus()));
    }

    if ( m_physics != nullptr )
    {
        m_physics->Write(line);
    }

    if ( m_auto != nullptr )
    {
        m_auto->Write(line);
    }
}

// Returns all parameters of the object.

void COldObject::Read(CLevelParserLine* line)
{
    glm::vec3 zoom = line->GetParam("zoom")->AsPoint(glm::vec3(1.0f, 1.0f, 1.0f));
    if (zoom.x != 1.0f || zoom.y != 1.0f || zoom.z != 1.0f)
        SetScale(zoom);

    if (line->GetParam("camera")->IsDefined())
        SetCameraType(line->GetParam("camera")->AsCameraType());
    SetCameraLock(line->GetParam("cameraLock")->AsBool(false));

    if (line->GetParam("pyro")->IsDefined())
        m_engine->GetPyroManager()->Create(line->GetParam("pyro")->AsPyroType(), this);

    SetBulletWall(line->GetParam("bulletWall")->AsBool(IsBulletWallByDefault(m_type)));

    SetProxyActivate(line->GetParam("proxyActivate")->AsBool(false));
    SetProxyDistance(line->GetParam("proxyDistance")->AsFloat(15.0f)*g_unit);
    SetCollisions(line->GetParam("clip")->AsBool(true));
    SetAnimateOnReset(line->GetParam("reset")->AsBool(false));
    if (Implements(ObjectInterfaceType::Controllable))
    {
        SetSelectable(line->GetParam("selectable")->AsBool(IsSelectableByDefault(m_type)));
    }
    if (Implements(ObjectInterfaceType::JetFlying))
    {
        SetRange(line->GetParam("range")->AsFloat(30.0f));
    }
    if (Implements(ObjectInterfaceType::Fragile))
    {
        SetMagnifyDamage(line->GetParam("magnifyDamage")->AsFloat(1.0f)); // TODO: This is a temporary hack for now - CFragileObject doesn't have SetMagnifyDamage ~krzys_h
    }
    if (Implements(ObjectInterfaceType::Shielded))
    {
        SetShield(line->GetParam("shield")->AsFloat(1.0f));
        SetMagnifyDamage(line->GetParam("magnifyDamage")->AsFloat(1.0f));
    }
    if (Implements(ObjectInterfaceType::Programmable))
    {
        SetCheckToken(!line->GetParam("checkToken")->IsDefined() ? GetSelectable() : line->GetParam("checkToken")->AsBool(true));

        if (line->GetParam("cmdline")->IsDefined())
        {
            const auto& cmdline = line->GetParam("cmdline")->AsArray();
            for (unsigned int i = 0; i < cmdline.size(); i++)
            {
                SetCmdLine(i, cmdline[i]->AsFloat());
            }
        }
    }

    // SetManual will affect bot speed
    if (m_type == OBJECT_MOBILEdr)
    {
        // TODO: Merge these two settings?
        SetManual(!GetTrainer());
    }

    // AlienWorm time up/down
    // TODO: Refactor function names
    if (m_type == OBJECT_WORM)
    {
        assert(Implements(ObjectInterfaceType::Movable));
        CMotion* motion = GetMotion();
        if (line->GetParam("param")->IsDefined())
        {
            const auto& p = line->GetParam("param")->AsArray();
            for (unsigned int i = 0; i < 10 && i < p.size(); i++)
            {
                motion->SetParam(i, p[i]->AsFloat());
            }
        }
    }

    if (m_auto != nullptr)
    {
        // TODO: Is it used for anything else than AlienEggs?
        m_auto->SetType(line->GetParam("autoType")->AsObjectType(OBJECT_NULL));
        for (int i = 0; i < 5; i++)
        {
            std::string op = "autoValue" + StrUtils::ToString(i+1); // autoValue1..autoValue5
            m_auto->SetValue(i, line->GetParam(op)->AsFloat(0.0f));
        }
        m_auto->SetString(const_cast<char*>(line->GetParam("autoString")->AsString("").c_str()));

        int i = line->GetParam("run")->AsInt(-1);
        if (i != -1)
        {
            if (i != PARAM_FIXSCENE && !CSettings::GetInstancePointer()->GetMovies()) i = 0;
            m_auto->Start(i);  // starts the film
        }
    }


    // Everthing below is for use only by saved scenes
    if (line->GetParam("energy")->IsDefined())
        SetEnergyLevel(line->GetParam("energy")->AsFloat());
    SetLock(line->GetParam("lock")->AsBool(false));
    SetActivity(line->GetParam("activity")->AsBool(true));
    SetGunGoalV(line->GetParam("aimV")->AsFloat(0.0f));
    SetGunGoalH(line->GetParam("aimH")->AsFloat(0.0f));

    if (line->GetParam("burnMode")->AsBool(false))
        SetDying(DeathType::Burning);
    m_bVirusMode = line->GetParam("virusMode")->AsBool(false);
    m_virusTime = line->GetParam("virusTime")->AsFloat(0.0f);

    m_aTime = line->GetParam("lifetime")->AsFloat(0.0f);

    if ( m_motion != nullptr )
    {
        m_motion->Read(line);
    }

    if (Implements(ObjectInterfaceType::Programmable))
    {
        SetActiveVirus(line->GetParam("bVirusActive")->AsBool(false));
    }

    if ( m_physics != nullptr )
    {
        m_physics->Read(line);
    }

    if ( m_auto != nullptr )
    {
        m_auto->Read(line);
    }
}



// Seeking the nth son of a father.

int COldObject::SearchDescendant(int parent, int n)
{
    int     i;

    for ( i=0 ; i<m_totalPart ; i++ )
    {
        if ( !m_objectPart[i].bUsed )  continue;

        if ( parent == m_objectPart[i].parentPart )
        {
            if ( n-- == 0 )  return i;
        }
    }
    return -1;
}

void COldObject::TransformCrashSphere(Math::Sphere& crashSphere)
{
    if(!Implements(ObjectInterfaceType::Jostleable)) crashSphere.radius *= GetScaleX();

    // Returns to the sphere collisions,
    // which ignores the tilt of the vehicle.
    // This is necessary to collisions with vehicles,
    // so as not to reflect SetTilt, for example.
    // The sphere must necessarily have a center (0, y, 0).
    if (m_crashSpheres.size() == 1 &&
        crashSphere.pos.x == 0.0f &&
        crashSphere.pos.z == 0.0f )
    {
        crashSphere.pos += m_objectPart[0].position;
        return;
    }

    if (m_objectPart[0].bTranslate ||
        m_objectPart[0].bRotate)
    {
        UpdateTransformObject();
    }

    crashSphere.pos = Math::Transform(m_objectPart[0].matWorld, crashSphere.pos);
}

void COldObject::TransformCameraCollisionSphere(Math::Sphere& collisionSphere)
{
    collisionSphere.pos = Math::Transform(m_objectPart[0].matWorld, collisionSphere.pos);
    collisionSphere.radius *= GetScaleX();
}


// Specifies the sphere of jostling, relative to the object.

void COldObject::SetJostlingSphere(const Math::Sphere& jostlingSphere)
{
    m_jostlingSphere = jostlingSphere;
    m_implementedInterfaces[static_cast<int>(ObjectInterfaceType::Jostleable)] = true;
}

// Specifies the sphere of jostling, in the world.

Math::Sphere COldObject::GetJostlingSphere() const
{
    Math::Sphere transformedJostlingSphere = m_jostlingSphere;
    transformedJostlingSphere.pos = Math::Transform(m_objectPart[0].matWorld, transformedJostlingSphere.pos);
    return transformedJostlingSphere;
}


// Positioning an object on a certain height, above the ground.

void COldObject::SetFloorHeight(float height)
{
    glm::vec3    pos;

    pos = m_objectPart[0].position;
    m_terrain->AdjustToFloor(pos);

    if ( m_physics != nullptr )
    {
        m_physics->SetLand(height == 0.0f);
        m_physics->SetMotor(height != 0.0f);
    }

    m_objectPart[0].position.y = pos.y+height+m_character.height;
    m_objectPart[0].bTranslate = true;  // it will recalculate the matrices
}

// Adjust the inclination of an object laying on the ground.

void COldObject::FloorAdjust()
{
    glm::vec3        pos, n;
    glm::vec2           nn;
    float           a;

    pos = GetPosition();
    if ( m_terrain->GetNormal(n, pos) )
    {
        a = GetRotationY();
        nn = Math::RotatePoint(-a, { n.z, n.x });
        SetRotationX( sinf(nn.x));
        SetRotationZ(-sinf(nn.y));
    }
}


// Getes the linear vibration.

void COldObject::SetLinVibration(glm::vec3 dir)
{
    if ( m_linVibration.x != dir.x ||
         m_linVibration.y != dir.y ||
         m_linVibration.z != dir.z )
    {
        m_linVibration = dir;
        m_objectPart[0].bTranslate = true;
    }
}

glm::vec3 COldObject::GetLinVibration()
{
    return m_linVibration;
}

// Getes the circular vibration.

void COldObject::SetCirVibration(glm::vec3 dir)
{
    if ( m_cirVibration.x != dir.x ||
         m_cirVibration.y != dir.y ||
         m_cirVibration.z != dir.z )
    {
        m_cirVibration = dir;
        m_objectPart[0].bRotate = true;
    }
}

glm::vec3 COldObject::GetCirVibration()
{
    return m_cirVibration;
}

// Getes the inclination.

void COldObject::SetTilt(glm::vec3 dir)
{
    if ( m_tilt.x != dir.x ||
         m_tilt.y != dir.y ||
         m_tilt.z != dir.z )
    {
        m_tilt = dir;
        m_objectPart[0].bRotate = true;
    }
}

glm::vec3 COldObject::GetTilt()
{
    return m_tilt;
}


// Getes the position of center of the object.

void COldObject::SetPartPosition(int part, const glm::vec3 &pos)
{
    m_objectPart[part].position = pos;
    m_objectPart[part].bTranslate = true;  // it will recalculate the matrices

    if ( part == 0 && !m_bFlat )  // main part?
    {
        int rank = m_objectPart[0].object;

        glm::vec3 shPos = pos;
        m_terrain->AdjustToFloor(shPos, true);
        m_engine->SetObjectShadowSpotPos(rank, shPos);

        float height = 0.0f;
        if ( Implements(ObjectInterfaceType::Flying) )
        {
            height = pos.y-shPos.y;
        }
        m_engine->SetObjectShadowSpotHeight(rank, height);

        m_engine->UpdateObjectShadowSpotNormal(rank);

        if ( m_shadowLight != -1 )
        {
            glm::vec3 lightPos = pos;
            lightPos.y += m_shadowHeight;
            m_lightMan->SetLightPos(m_shadowLight, lightPos);
        }
    }
}

glm::vec3 COldObject::GetPartPosition(int part) const
{
    return m_objectPart[part].position;
}

// Getes the rotation around three axis.

void COldObject::SetPartRotation(int part, const glm::vec3 &angle)
{
    m_objectPart[part].angle = angle;
    m_objectPart[part].bRotate = true;  // it will recalculate the matrices

    if ( part == 0 && !m_bFlat )  // main part?
    {
        m_engine->SetObjectShadowSpotAngle(m_objectPart[0].object, m_objectPart[0].angle.y);
    }
}

glm::vec3 COldObject::GetPartRotation(int part) const
{
    return m_objectPart[part].angle;
}

// Getes the rotation about the axis Y.

void COldObject::SetPartRotationY(int part, float angle)
{
    m_objectPart[part].angle.y = angle;
    m_objectPart[part].bRotate = true;  // it will recalculate the matrices

    if ( part == 0 && !m_bFlat )  // main part?
    {
        m_engine->SetObjectShadowSpotAngle(m_objectPart[0].object, m_objectPart[0].angle.y);
    }
}

// Getes the rotation about the axis X.

void COldObject::SetPartRotationX(int part, float angle)
{
    m_objectPart[part].angle.x = angle;
    m_objectPart[part].bRotate = true;  // it will recalculate the matrices
}

// Getes the rotation about the axis Z.

void COldObject::SetPartRotationZ(int part, float angle)
{
    m_objectPart[part].angle.z = angle;
    m_objectPart[part].bRotate = true;  //it will recalculate the matrices
}

float COldObject::GetPartRotationY(int part)
{
    return m_objectPart[part].angle.y;
}

float COldObject::GetPartRotationX(int part)
{
    return m_objectPart[part].angle.x;
}

float COldObject::GetPartRotationZ(int part)
{
    return m_objectPart[part].angle.z;
}


// Getes the global zoom.

void COldObject::SetPartScale(int part, float zoom)
{
    m_objectPart[part].bTranslate = true;  // it will recalculate the matrices
    m_objectPart[part].zoom.x = zoom;
    m_objectPart[part].zoom.y = zoom;
    m_objectPart[part].zoom.z = zoom;

    m_objectPart[part].bZoom = ( m_objectPart[part].zoom.x != 1.0f ||
                                 m_objectPart[part].zoom.y != 1.0f ||
                                 m_objectPart[part].zoom.z != 1.0f );
}

void COldObject::SetPartScale(int part, glm::vec3 zoom)
{
    m_objectPart[part].bTranslate = true;  // it will recalculate the matrices
    m_objectPart[part].zoom = zoom;

    m_objectPart[part].bZoom = ( m_objectPart[part].zoom.x != 1.0f ||
                                 m_objectPart[part].zoom.y != 1.0f ||
                                 m_objectPart[part].zoom.z != 1.0f );
}

glm::vec3 COldObject::GetPartScale(int part) const
{
    return m_objectPart[part].zoom;
}

void COldObject::SetPartScaleX(int part, float zoom)
{
    m_objectPart[part].bTranslate = true;  // it will recalculate the matrices
    m_objectPart[part].zoom.x = zoom;

    m_objectPart[part].bZoom = ( m_objectPart[part].zoom.x != 1.0f ||
                                 m_objectPart[part].zoom.y != 1.0f ||
                                 m_objectPart[part].zoom.z != 1.0f );
}

void COldObject::SetPartScaleY(int part, float zoom)
{
    m_objectPart[part].bTranslate = true;  // it will recalculate the matrices
    m_objectPart[part].zoom.y = zoom;

    m_objectPart[part].bZoom = ( m_objectPart[part].zoom.x != 1.0f ||
                                 m_objectPart[part].zoom.y != 1.0f ||
                                 m_objectPart[part].zoom.z != 1.0f );
}

void COldObject::SetPartScaleZ(int part, float zoom)
{
    m_objectPart[part].bTranslate = true;  // it will recalculate the matrices
    m_objectPart[part].zoom.z = zoom;

    m_objectPart[part].bZoom = ( m_objectPart[part].zoom.x != 1.0f ||
                                 m_objectPart[part].zoom.y != 1.0f ||
                                 m_objectPart[part].zoom.z != 1.0f );
}

float COldObject::GetPartScaleX(int part)
{
    return m_objectPart[part].zoom.x;
}

float COldObject::GetPartScaleY(int part)
{
    return m_objectPart[part].zoom.y;
}

float COldObject::GetPartScaleZ(int part)
{
    return m_objectPart[part].zoom.z;
}

void COldObject::SetTrainer(bool bEnable)
{
    m_bTrainer = bEnable;

    if ( m_bTrainer )  // training?
    {
        m_cameraType = Gfx::CAM_TYPE_FIX;
    }
}

bool COldObject::GetTrainer()
{
    return m_bTrainer;
}

bool COldObject::GetPlusTrainer()
{
    return m_main->GetPlusTrainer();
}

void COldObject::SetToy(bool bEnable)
{
    m_bToy = bEnable;
}

bool COldObject::GetToy()
{
    return m_bToy;
}

void COldObject::SetManual(bool bManual)
{
    m_bManual = bManual;
}

bool COldObject::GetManual()
{
    return m_bManual;
}

// Management of the particle master.

void COldObject::SetMasterParticle(int part, int parti)
{
    m_objectPart[part].masterParti = parti;
}


// Management of the stack transport.


int COldObject::GetNumSlots()
{
    assert(m_hasPowerSlot || m_hasCargoSlot); // otherwise implemented[CSlottedObject] is false
    return (m_hasPowerSlot ? 1 : 0) + (m_hasCargoSlot ? 1 : 0);
}
int COldObject::MapPseudoSlot(Pseudoslot pseudoslot)
{
    switch (pseudoslot)
    {
    case Pseudoslot::POWER:
        return m_hasPowerSlot ? 0 : -1;
    case Pseudoslot::CARRYING:
        return m_hasCargoSlot ? (m_hasPowerSlot ? 1 : 0) : -1;
    default:
        return -1;
    }
}
glm::vec3 COldObject::GetSlotPosition(int slotNum)
{
    if (slotNum == 0 && m_hasPowerSlot)
        return m_powerPosition;
    else
    {
        assert(m_hasCargoSlot && slotNum == (m_hasPowerSlot ? 1 : 0));
        int grabPartNum;
        glm::vec3 grabRelPos;
        // See CTaskManip::TransporterTakeObject call to SetTransporterPart and SetPosition
        switch (m_type)
        {
        case OBJECT_HUMAN:
        case OBJECT_TECH:
            grabPartNum = 4;
            grabRelPos = glm::vec3(1.7f, -0.5f, 1.1f);
            break;
        case OBJECT_MOBILEsa: // subber
            grabPartNum = 2;
            grabRelPos = glm::vec3(1.1f, -1.0f, 1.0f);
            break;
        case OBJECT_MOBILEfa: // Grabbers
        case OBJECT_MOBILEta:
        case OBJECT_MOBILEwa:
        case OBJECT_MOBILEia:
            grabPartNum = 3;
            grabRelPos = glm::vec3(4.7f, 0.0f, 0.0f);
            break;
        case OBJECT_BEE:
            grabPartNum = 0;
            grabRelPos = glm::vec3(0.0f, -3.0f, 0.0f);
            break;
        default: // unreachable, only the above objects have cargo slots
            assert(!m_hasCargoSlot);
            return m_powerPosition;
        }

        return Math::Transform(glm::inverse(GetWorldMatrix(0)), Math::Transform(GetWorldMatrix(grabPartNum), grabRelPos));
    }
}
float COldObject::GetSlotAngle(int slotNum)
{
    if (slotNum == 0 && m_hasPowerSlot)
    {
        switch (m_type)
        {
        case OBJECT_TOWER:
        case OBJECT_RESEARCH:
        case OBJECT_ENERGY:
        case OBJECT_LABO:
        case OBJECT_NUCLEAR:
            return 0;
        default: // robots
            return Math::PI;
        }
    }
    else
    {
        assert(m_hasCargoSlot && slotNum == (m_hasPowerSlot ? 1 : 0));
        return 0;
    }
}
float COldObject::GetSlotAcceptanceAngle(int slotNum)
{
    if (slotNum == 0 && m_hasPowerSlot)
    {
        switch (m_type)
        {
        case OBJECT_TOWER:
        case OBJECT_RESEARCH:
            return 45.0f*Math::PI/180.0f;
        case OBJECT_ENERGY:
            return 90.0f*Math::PI/180.0f;
        case OBJECT_LABO:
            return 120.0f*Math::PI/180.0f;
        case OBJECT_NUCLEAR:
            return 45.0f*Math::PI/180.0f;
        default:
            return 45.0f*Math::PI/180.0f;
        }
    }
    else
    {
        assert(m_hasCargoSlot && slotNum == (m_hasPowerSlot ? 1 : 0));
        return 0; // no acceptance angle for cargo slot
    }
}
CObject *COldObject::GetSlotContainedObject(int slotNum)
{
    if (slotNum == 0 && m_hasPowerSlot)
        return m_power;
    else
    {
        assert(m_hasCargoSlot && slotNum == (m_hasPowerSlot ? 1 : 0));
        return m_cargo;
    }
}
void COldObject::SetSlotContainedObject(int slotNum, CObject *object)
{
    if (slotNum == 0 && m_hasPowerSlot)
        m_power = object;
    else
    {
        assert(m_hasCargoSlot && slotNum == (m_hasPowerSlot ? 1 : 0));
        m_cargo = object;
    }
}
// not part of CSlottedObject; just used for initialization
void COldObject::SetPowerPosition(const glm::vec3& powerPosition)
{
    m_powerPosition = powerPosition;
}



// Management of the object "transporter" that transports it.

void COldObject::SetTransporter(CObject* transporter)
{
    m_transporter = transporter;

    // Invisible shadow if the object is transported.
    m_engine->SetObjectShadowSpotHide(m_objectPart[0].object, (m_transporter != nullptr));
}

CObject* COldObject::GetTransporter() const
{
    return m_transporter;
}

// Management of the conveying portion.

void COldObject::SetTransporterPart(int part)
{
    m_transporterLink = part;
}


// Returns matrices of an object portion.

glm::mat4 COldObject::GetRotateMatrix(int part)
{
    return m_objectPart[part].matRotate;
}

glm::mat4 COldObject::GetWorldMatrix(int part)
{
    if ( m_objectPart[0].bTranslate ||
         m_objectPart[0].bRotate    )
    {
        UpdateTransformObject();
    }

    return m_objectPart[part].matWorld;
}


// Indicates whether the object should be drawn over the interface.

void COldObject::SetDrawFront(bool bDraw)
{
    int     i;

    for ( i=0 ; i<OBJECTMAXPART ; i++ )
    {
        if ( m_objectPart[i].bUsed )
        {
            m_engine->SetObjectDrawFront(m_objectPart[i].object, bDraw);
        }
    }
}

// Creates shade under a vehicle as a negative light.

bool COldObject::CreateShadowLight(float height, Gfx::Color color)
{
    if ( !m_engine->GetLightMode() )  return true;

    glm::vec3 pos = GetPosition();
    m_shadowHeight = height;

    Gfx::Light light;
    light.type          = Gfx::LIGHT_SPOT;
    light.diffuse       = color;
    light.ambient       = color * 0.1f;
    light.position      = glm::vec3(pos.x, pos.y+height, pos.z);
    light.direction     = glm::vec3(0.0f, -1.0f, 0.0f); // against the bottom
    light.spotIntensity = 128;
    light.attenuation0  = 1.0f;
    light.attenuation1  = 0.0f;
    light.attenuation2  = 0.0f;
    light.spotAngle = 90.0f*Math::PI/180.0f;

    m_shadowLight = m_lightMan->CreateLight();
    if ( m_shadowLight == -1 )  return false;

    m_lightMan->SetLight(m_shadowLight, light);

    // Only illuminates the objects on the ground.
    m_lightMan->SetLightIncludeType(m_shadowLight, Gfx::ENG_OBJTYPE_TERRAIN);

    return true;
}

// Returns the number of negative light shade.

int COldObject::GetShadowLight()
{
    return m_shadowLight;
}

// Creates the circular shadow underneath a vehicle.

bool COldObject::CreateShadowCircle(float radius, float intensity,
                                 Gfx::EngineShadowType type)
{
    float   zoom;

    if ( intensity == 0.0f )  return true;

    zoom = GetScaleX();

    m_engine->CreateShadowSpot(m_objectPart[0].object);

    m_engine->SetObjectShadowSpotRadius(m_objectPart[0].object, radius*zoom);
    m_engine->SetObjectShadowSpotIntensity(m_objectPart[0].object, intensity);
    m_engine->SetObjectShadowSpotHeight(m_objectPart[0].object, 0.0f);
    m_engine->SetObjectShadowSpotAngle(m_objectPart[0].object, m_objectPart[0].angle.y);
    m_engine->SetObjectShadowSpotType(m_objectPart[0].object, type);

    return true;
}

// Calculates the matrix for transforming the object.
// Returns true if the matrix has changed.
// The rotations occur in the order Y, Z and X.

bool COldObject::UpdateTransformObject(int part, bool bForceUpdate)
{
    glm::vec3    position, angle;
    bool        bModif = false;
    int         parent;

    if ( m_transporter != nullptr )  // transported by transporter?
    {
        m_objectPart[part].bTranslate = true;
        m_objectPart[part].bRotate = true;
    }

    if ( !bForceUpdate                  &&
         !m_objectPart[part].bTranslate &&
         !m_objectPart[part].bRotate    )  return false;

    position = m_objectPart[part].position;
    angle    = m_objectPart[part].angle;

    if ( part == 0 )  // main part?
    {
        position += m_linVibration;
        angle    += m_cirVibration+m_tilt;
    }

    if ( m_objectPart[part].bTranslate ||
         m_objectPart[part].bRotate    )
    {
        if ( m_objectPart[part].bTranslate )
        {
            m_objectPart[part].matTranslate = glm::mat4(1.0f);
            m_objectPart[part].matTranslate[3][0] = position.x;
            m_objectPart[part].matTranslate[3][1] = position.y;
            m_objectPart[part].matTranslate[3][2] = position.z;
        }

        if ( m_objectPart[part].bRotate )
        {
            Math::LoadRotationZXYMatrix(m_objectPart[part].matRotate, angle);
        }

        if ( m_objectPart[part].bZoom )
        {
            glm::mat4 mz = glm::mat4(1.0f);
            mz[0][0] = m_objectPart[part].zoom.x;
            mz[1][1] = m_objectPart[part].zoom.y;
            mz[2][2] = m_objectPart[part].zoom.z;
            m_objectPart[part].matTransform = m_objectPart[part].matTranslate * m_objectPart[part].matRotate * mz;
        }
        else
        {
            m_objectPart[part].matTransform = m_objectPart[part].matTranslate * m_objectPart[part].matRotate;
        }
        bModif = true;
    }

    if ( bForceUpdate                  ||
         m_objectPart[part].bTranslate ||
         m_objectPart[part].bRotate    )
    {
        parent = m_objectPart[part].parentPart;

        if ( part == 0 && m_transporter != nullptr )  // transported by a transporter?
        {
            glm::mat4 matWorldTransporter = m_transporter->GetWorldMatrix(m_transporterLink);
            m_objectPart[part].matWorld = matWorldTransporter * m_objectPart[part].matTransform;
        }
        else
        {
            if ( parent == -1 )  // no parent?
            {
                m_objectPart[part].matWorld = m_objectPart[part].matTransform;
            }
            else
            {
                m_objectPart[part].matWorld = m_objectPart[parent].matWorld * m_objectPart[part].matTransform;
            }
        }
        bModif = true;
    }

    if ( bModif )
    {
        m_engine->SetObjectTransform(m_objectPart[part].object,
                                     m_objectPart[part].matWorld);
    }

    m_objectPart[part].bTranslate = false;
    m_objectPart[part].bRotate    = false;

    return bModif;
}

// Updates all matrices to transform the object father and all his sons.
// Assume a maximum of 4 degrees of freedom.
// Appropriate, for example, to a body, an arm, forearm, hand and fingers.

bool COldObject::UpdateTransformObject()
{
    bool    bUpdate1, bUpdate2, bUpdate3, bUpdate4;
    int     level1, level2, level3, level4, rank;
    int     parent1, parent2, parent3, parent4;

    if ( m_bFlat )
    {
        for ( level1=0 ; level1<m_totalPart ; level1++ )
        {
            if ( !m_objectPart[level1].bUsed )  continue;
            UpdateTransformObject(level1, false);
        }
    }
    else
    {
        parent1 = 0;
        bUpdate1 = UpdateTransformObject(parent1, false);

        for ( level1=0 ; level1<m_totalPart ; level1++ )
        {
            rank = SearchDescendant(parent1, level1);
            if ( rank == -1 )  break;

            parent2 = rank;
            bUpdate2 = UpdateTransformObject(rank, bUpdate1);

            for ( level2=0 ; level2<m_totalPart ; level2++ )
            {
                rank = SearchDescendant(parent2, level2);
                if ( rank == -1 )  break;

                parent3 = rank;
                bUpdate3 = UpdateTransformObject(rank, bUpdate2);

                for ( level3=0 ; level3<m_totalPart ; level3++ )
                {
                    rank = SearchDescendant(parent3, level3);
                    if ( rank == -1 )  break;

                    parent4 = rank;
                    bUpdate4 = UpdateTransformObject(rank, bUpdate3);

                    for ( level4=0 ; level4<m_totalPart ; level4++ )
                    {
                        rank = SearchDescendant(parent4, level4);
                        if ( rank == -1 )  break;

                        UpdateTransformObject(rank, bUpdate4);
                    }
                }
            }
        }
    }

    return true;
}


// Puts all the progeny flat (there is more than fathers).
// This allows for debris independently from each other in all directions.

void COldObject::FlatParent()
{
    int     i;

    for ( i=0 ; i<m_totalPart ; i++ )
    {
        m_objectPart[i].position.x = m_objectPart[i].matWorld[3][0];
        m_objectPart[i].position.y = m_objectPart[i].matWorld[3][1];
        m_objectPart[i].position.z = m_objectPart[i].matWorld[3][2];

        m_objectPart[i].matWorld[3][0] = 0.0f;
        m_objectPart[i].matWorld[3][1] = 0.0f;
        m_objectPart[i].matWorld[3][2] = 0.0f;

        m_objectPart[i].matTranslate[3][0] = 0.0f;
        m_objectPart[i].matTranslate[3][1] = 0.0f;
        m_objectPart[i].matTranslate[3][2] = 0.0f;

        m_objectPart[i].parentPart = -1;  // more parents
    }

    m_bFlat = true;
}



// Updates the mapping of the texture of the pile.

void COldObject::UpdateEnergyMapping()
{
    if (m_lastEnergy == GetEnergyLevel())
        return;

    m_lastEnergy = GetEnergyLevel();

    m_engine->SetUVTransform(m_objectPart[0].object, "energy",
        { 0.0f, 0.25f * (GetEnergyLevel() - 1.0f) }, { 1.0f, 1.0f });
}


// Manual action.

bool COldObject::EventProcess(const Event &event)
{
    // NOTE: This should be called before CProgrammableObjectImpl::EventProcess, see the other note inside CTaskExecutorObjectImpl::EventProcess
    if (!CTaskExecutorObjectImpl::EventProcess(event)) return false;

    if ( m_physics != nullptr )
    {
        if ( !m_physics->EventProcess(event) )  // object destroyed?
        {
            if ( GetSelect()             &&
                 m_type != OBJECT_ANT    &&
                 m_type != OBJECT_SPIDER &&
                 m_type != OBJECT_BEE    )
            {
                if ( !IsDying() )  m_camera->SetType(Gfx::CAM_TYPE_EXPLO);
                m_main->DeselectAll();
            }
            return false;
        }
    }

    if (Implements(ObjectInterfaceType::Movable) && m_physics != nullptr)
    {
        bool deselectedStop = !GetSelect();
        if (Implements(ObjectInterfaceType::Programmable))
        {
            deselectedStop = deselectedStop && !IsProgram();
        }
        if (Implements(ObjectInterfaceType::TaskExecutor))
        {
            deselectedStop = deselectedStop && !IsForegroundTask();
        }

        if ( deselectedStop )
        {
            float axeX = 0.0f;
            float axeY = 0.0f;
            float axeZ = 0.0f;
            if ( GetDying() == DeathType::Burning )  // Burning?
            {
                axeZ = -1.0f;  // tomb

                if ( (m_type == OBJECT_ANT    ||
                      m_type == OBJECT_SPIDER ||
                      m_type == OBJECT_WORM   ) )
                {
                    // TODO: Move to CBaseAlien?
                    CBaseAlien* alien = dynamic_cast<CBaseAlien*>(this);
                    assert(alien != nullptr);
                    if (!alien->GetFixed())
                    {
                        axeY = 2.0f;  // zigzag disorganized fast
                        if ( m_type == OBJECT_WORM )  axeY = 5.0f;
                        axeX = 0.5f+sinf(m_time* 1.0f)*0.5f+
                                    sinf(m_time* 6.0f)*2.0f+
                                    sinf(m_time*21.0f)*0.2f;
                        float factor = 1.0f-m_burnTime/15.0f;  // slow motion
                        if ( factor < 0.0f )  factor = 0.0f;
                        axeY *= factor;
                        axeX *= factor;
                    }
                }
            }
            m_physics->SetMotorSpeedX(axeY);  // move forward/move back
            m_physics->SetMotorSpeedY(axeZ);  // up / down
            m_physics->SetMotorSpeedZ(axeX);  // rotate
        }
        else if (GetSelect())
        {
            bool canMove = true;
            if (Implements(ObjectInterfaceType::TaskExecutor))
            {
                canMove = canMove && (!IsForegroundTask() || GetForegroundTask()->IsPilot());
            }
            if (Implements(ObjectInterfaceType::Programmable))
            {
                canMove = canMove && !IsProgram();
            }

            if ( canMove )
            {
                if ( event.type == EVENT_OBJECT_LEFT    ||
                     event.type == EVENT_OBJECT_RIGHT   ||
                     event.type == EVENT_OBJECT_UP      ||
                     event.type == EVENT_OBJECT_DOWN    ||
                     event.type == EVENT_OBJECT_GASUP   ||
                     event.type == EVENT_OBJECT_GASDOWN )
                {
                    m_buttonAxe = event.type;
                }
                if ( event.type == EVENT_MOUSE_BUTTON_UP )
                {
                    m_buttonAxe = EVENT_NULL;
                }

                float axeX = event.motionInput.x;
                float axeY = event.motionInput.y;
                float axeZ = event.motionInput.z;

                if ( (!m_main->GetTrainerPilot() &&
                      GetTrainer()) ||
                     !m_main->CanPlayerInteract() )  // drive vehicle?
                {
                    axeX = 0.0f;
                    axeY = 0.0f;
                    axeZ = 0.0f;  // Remote control impossible!
                }

                if ( m_buttonAxe == EVENT_OBJECT_LEFT    )  axeX = -1.0f;
                if ( m_buttonAxe == EVENT_OBJECT_RIGHT   )  axeX =  1.0f;
                if ( m_buttonAxe == EVENT_OBJECT_UP      )  axeY =  1.0f;
                if ( m_buttonAxe == EVENT_OBJECT_DOWN    )  axeY = -1.0f;
                if ( m_buttonAxe == EVENT_OBJECT_GASUP   )  axeZ =  1.0f;
                if ( m_buttonAxe == EVENT_OBJECT_GASDOWN )  axeZ = -1.0f;

                if ( m_type == OBJECT_MOBILEdr && GetManual() )  // scribbler in manual mode?
                {
                    if ( axeX != 0.0f )  axeY = 0.0f;  // if running -> not moving!
                    axeX *= 0.5f;
                    axeY *= 0.5f;
                }

                if ( !m_main->IsResearchDone(RESEARCH_FLY, GetTeam()) )
                {
                    axeZ = -1.0f;  // tomb
                }

                if ( axeX >  1.0f )  axeX =  1.0f;
                if ( axeX < -1.0f )  axeX = -1.0f;

                m_physics->SetMotorSpeedX(axeY);  // move forward/move back
                m_physics->SetMotorSpeedY(axeZ);  // up/down
                m_physics->SetMotorSpeedZ(axeX);  // rotate
            }
        }
    }

    if ( m_objectInterface != nullptr )
    {
        m_objectInterface->EventProcess(event);
    }

    if ( m_auto != nullptr )
    {
        if (!GetLock())
        {
            m_auto->EventProcess(event);
        }

        if ( event.type == EVENT_FRAME &&
             m_auto->IsEnded() != ERR_CONTINUE )
        {
            m_auto->DeleteObject();
            m_auto.reset();
        }
    }

    if ( m_motion != nullptr )
    {
        if (!m_motion->EventProcess(event)) return false;
    }

    if (!CProgrammableObjectImpl::EventProcess(event)) return false;

    if ( event.type == EVENT_FRAME )
    {
        return EventFrame(event);
    }

    return true;
}


// Animates the object.

bool COldObject::EventFrame(const Event &event)
{
    if ( m_type == OBJECT_HUMAN && m_main->GetMainMovie() == MM_SATCOMopen )
    {
        UpdateTransformObject();
        return true;
    }

    m_time += event.rTime;

    if ( m_engine->GetPause() && m_type != OBJECT_SHOW )  return true;

    if ( GetDying() == DeathType::Burning )  m_burnTime += event.rTime;

    m_aTime += event.rTime;
    m_shotTime += event.rTime;

    VirusFrame(event.rTime);
    PartiFrame(event.rTime);

    UpdateMapping();
    UpdateTransformObject();
    UpdateSelectParticle();

    if (Implements(ObjectInterfaceType::ShieldedAutoRegen))
    {
        SetShield(GetShield() + event.rTime*(1.0f/GetShieldFullRegenTime()));
    }

    if (m_damaging && m_time - m_damageTime > 2.0f)
    {
        SetDamaging(false);
        m_main->UpdateShortcuts();
    }

    return true;
}

// Updates the mapping of the object.

void COldObject::UpdateMapping()
{
    if ( Implements(ObjectInterfaceType::PowerContainer) )
    {
        UpdateEnergyMapping();
    }
}


// Management of viruses.

void COldObject::VirusFrame(float rTime)
{
    if ( !m_bVirusMode )  return;  // healthy object?

    m_virusTime += rTime;
    if ( m_virusTime >= VIRUS_DELAY )
    {
        m_bVirusMode = false;  // the virus is no longer active
    }

    if ( m_lastVirusParticle+m_engine->ParticleAdapt(0.2f) <= m_aTime )
    {
        m_lastVirusParticle = m_aTime;

        glm::vec3 pos = GetPosition();
        pos.x += (Math::Rand()-0.5f)*10.0f;
        pos.z += (Math::Rand()-0.5f)*10.0f;
        glm::vec3 speed;
        speed.x = (Math::Rand()-0.5f)*2.0f;
        speed.z = (Math::Rand()-0.5f)*2.0f;
        speed.y = Math::Rand()*4.0f+4.0f;
        glm::vec2 dim;
        dim.x = Math::Rand()*0.3f+0.3f;
        dim.y = dim.x;

        m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIVIRUS, 3.0f);
    }
}

// Management particles mistresses.

void COldObject::PartiFrame(float rTime)
{
    glm::vec3    pos, angle, factor;
    int         i, channel;

    for ( i=0 ; i<OBJECTMAXPART ; i++ )
    {
        if ( !m_objectPart[i].bUsed )  continue;

        channel = m_objectPart[i].masterParti;
        if ( channel == -1 )  continue;

        if ( !m_particle->GetPosition(channel, pos) )
        {
            m_objectPart[i].masterParti = -1;  // particle no longer exists!
            continue;
        }

        SetPartPosition(i, pos);

        // Each song spins differently.
        switch( i%5 )
        {
            case 0:  factor = glm::vec3( 0.5f, 0.3f, 0.6f); break;
            case 1:  factor = glm::vec3(-0.3f, 0.4f,-0.2f); break;
            case 2:  factor = glm::vec3( 0.4f,-0.6f,-0.3f); break;
            case 3:  factor = glm::vec3(-0.6f,-0.2f, 0.0f); break;
            case 4:  factor = glm::vec3( 0.4f, 0.1f,-0.7f); break;
        }

        angle = GetPartRotation(i);
        angle += rTime*Math::PI*factor;
        SetPartRotation(i, angle);
    }
}


// Changes the perspective to view if it was like in the vehicle,
// or behind the vehicle.

void COldObject::AdjustCamera(glm::vec3 &eye, float &dirH, float &dirV,
                              glm::vec3  &lookat, glm::vec3 &upVec,
                              Gfx::CameraType type)
{
    float   speed;
    int     part;

    UpdateTransformObject();

    part = 0;
    if ( m_type == OBJECT_HUMAN ||
         m_type == OBJECT_TECH  )
    {
        eye.x = -0.2f;
        eye.y =  3.3f;
        eye.z =  0.0f;
//?     eye.x =  1.0f;
//?     eye.y =  3.3f;
//?     eye.z =  0.0f;
    }
    else if ( m_type == OBJECT_MOBILErt ||
              m_type == OBJECT_MOBILErr ||
              m_type == OBJECT_MOBILErs ||
              m_type == OBJECT_MOBILErp )
    {
        eye.x = -1.1f;  // on the cap
        eye.y =  7.9f;
        eye.z =  0.0f;
    }
    else if ( m_type == OBJECT_MOBILEwc ||
              m_type == OBJECT_MOBILEtc ||
              m_type == OBJECT_MOBILEfc ||
              m_type == OBJECT_MOBILEic )  // fireball?
    {
//?     eye.x = -0.9f;  // on the cannon
//?     eye.y =  3.0f;
//?     eye.z =  0.0f;
//?     part = 1;
        eye.x = -0.9f;  // on the cannon
        eye.y =  8.3f;
        eye.z =  0.0f;
    }
    else if ( m_type == OBJECT_MOBILEwi ||
              m_type == OBJECT_MOBILEti ||
              m_type == OBJECT_MOBILEfi ||
              m_type == OBJECT_MOBILEii )  // orgaball ?
    {
//?     eye.x = -3.5f;  // on the cannon
//?     eye.y =  5.1f;
//?     eye.z =  0.0f;
//?     part = 1;
        eye.x = -2.5f;  // on the cannon
        eye.y = 10.4f;
        eye.z =  0.0f;
    }
    else if ( m_type == OBJECT_MOBILErc )
    {
//?     eye.x =  2.0f;  // in the cannon
//?     eye.y =  0.0f;
//?     eye.z =  0.0f;
//?     part = 2;
        eye.x =  4.0f;  // on the cannon
        eye.y = 11.0f;
        eye.z =  0.0f;
    }
    else if ( m_type == OBJECT_MOBILEsa ||
              m_type == OBJECT_MOBILEst )
    {
        eye.x =  3.0f;
        eye.y =  4.5f;
        eye.z =  0.0f;
    }
    else if ( m_type == OBJECT_MOBILEdr )
    {
        eye.x =  1.0f;
        eye.y =  6.5f;
        eye.z =  0.0f;
    }
    else if ( m_type == OBJECT_APOLLO2 )
    {
        eye.x = -3.0f;
        eye.y =  6.0f;
        eye.z = -2.0f;
    }
    else
    {
        eye.x = 0.7f;  // between the brackets
        eye.y = 4.8f;
        eye.z = 0.0f;
    }

    if ( type == Gfx::CAM_TYPE_BACK )
    {
        eye.x -= 20.0f;
        eye.y +=  1.0f;
    }

    lookat.x = eye.x+1.0f;
    lookat.y = eye.y+0.0f;
    lookat.z = eye.z+0.0f;

    eye    = Math::Transform(m_objectPart[part].matWorld, eye);
    lookat = Math::Transform(m_objectPart[part].matWorld, lookat);

    // Camera tilts when turning.
    upVec = glm::vec3(0.0f, 1.0f, 0.0f);
    if ( m_physics != nullptr )
    {
        if ( m_physics->GetLand() )  // on ground?
        {
            speed = m_physics->GetLinMotionX(MO_REASPEED);
            lookat.y -= speed*0.002f;

            speed = m_physics->GetCirMotionY(MO_REASPEED);
            upVec.z -= speed*0.04f;
        }
        else    // in flight?
        {
            speed = m_physics->GetLinMotionX(MO_REASPEED);
            lookat.y += speed*0.002f;

            speed = m_physics->GetCirMotionY(MO_REASPEED);
            upVec.z += speed*0.08f;
        }
    }
    upVec = Math::Transform(m_objectPart[0].matRotate, upVec);

    dirH = -(m_objectPart[part].angle.y+Math::PI/2.0f);
    dirV = 0.0f;

}


// Management of features.

Character* COldObject::GetCharacter()
{
    return &m_character;
}


// Returns the absolute time.

float COldObject::GetAbsTime()
{
    return m_aTime;
}


float COldObject::GetCapacity()
{
    return m_type == OBJECT_ATOMIC ? m_main->GetGlobalNuclearCapacity() : m_main->GetGlobalCellCapacity() ;
}

bool COldObject::IsRechargeable()
{
    return m_type == OBJECT_POWER;
}


// Management of the shield.

void COldObject::SetShield(float level)
{
    if (level > 1.0f) level = 1.0f;
    if (level < 0.0f) level = 0.0f;
    m_shield = level;
}

float COldObject::GetShield()
{
    if (Implements(ObjectInterfaceType::Fragile))  return 0.0f;
    return m_shield;
}


// Management of flight range (zero = infinity).

void COldObject::SetRange(float delay)
{
    m_range = delay;
}

float COldObject::GetRange()
{
    return m_range;
}

void COldObject::SetReactorRange(float reactorRange)
{
    if (reactorRange > 1.0f) reactorRange = 1.0f;
    if (reactorRange < 0.0f) reactorRange = 0.0f;
    m_reactorRange = reactorRange;
}

float COldObject::GetReactorRange()
{
    return m_reactorRange;
}


// Management of transparency of the object.

void COldObject::SetGhostMode(bool enabled)
{
    int     i;

    for ( i=0 ; i<m_totalPart ; i++ )
    {
        if ( m_objectPart[i].bUsed )
        {
            if ( m_type == OBJECT_BASE )
            {
                if ( i != 9 )  continue;  // no central pillar?
            }

            m_engine->SetObjectGhostMode(m_objectPart[i].object, enabled);
        }
    }
}


// Pushes an object.

bool COldObject::JostleObject(float force)
{
    if ( m_type == OBJECT_FLAGb ||
         m_type == OBJECT_FLAGr ||
         m_type == OBJECT_FLAGg ||
         m_type == OBJECT_FLAGy ||
         m_type == OBJECT_FLAGv )  // flag?
    {
        if ( m_auto == nullptr )  return false;

        m_auto->Start(1);
    }
    else
    {
        if ( m_auto != nullptr )  return false;

        auto autoJostle = std::make_unique<CAutoJostle>(this);
        autoJostle->Start(0, force);
        m_auto = std::move(autoJostle);
    }

    return true;
}



// Management of time from which a virus is active.

void COldObject::SetVirusMode(bool bEnable)
{
    m_bVirusMode = bEnable;
    m_virusTime = 0.0f;

    if ( m_bVirusMode && Implements(ObjectInterfaceType::Programmable) )
    {
        if ( !IntroduceVirus() )  // tries to infect
        {
            m_bVirusMode = false;  // program was not contaminated!
        }
    }
}

bool COldObject::GetVirusMode()
{
    return m_bVirusMode;
}


// Management mode of the camera.

void COldObject::SetCameraType(Gfx::CameraType type)
{
    m_cameraType = type;
}

Gfx::CameraType COldObject::GetCameraType()
{
    return m_cameraType;
}

void COldObject::SetCameraLock(bool lock)
{
    m_bCameraLock = lock;
}

bool COldObject::GetCameraLock()
{
    return m_bCameraLock;
}



// Management of the demonstration of the object.

void COldObject::SetHighlight(bool highlight)
{
    if (highlight)
    {
        int list[OBJECTMAXPART+1];

        int j = 0;
        for (int i = 0; i < m_totalPart; i++)
        {
            if ( m_objectPart[i].bUsed )
            {
                list[j++] = m_objectPart[i].object;
            }
        }
        list[j] = -1;  // terminate

        m_engine->SetHighlightRank(list);  // gives the list of selected parts
    }
}


// Indicates whether the object is selected or not.

void COldObject::SetSelect(bool select, bool bDisplayError)
{
    m_bSelect = select;

    // NOTE: Right now, Ui::CObjectInterface is only for programmable objects. Right now all selectable objects are programmable anyway.
    // TODO: All UI-related stuff should be moved out of CObject classes
    if (Implements(ObjectInterfaceType::Programmable))
    {
        if ( m_objectInterface == nullptr )
        {
            m_objectInterface = std::make_unique<Ui::CObjectInterface>(this);
        }
        m_objectInterface->CreateInterface(m_bSelect);
    }

    if ( m_auto != nullptr )
    {
        m_auto->CreateInterface(m_bSelect);
    }

    CreateSelectParticle();  // creates / removes particles

    if ( !m_bSelect )
        return;  // if not selected, we're done

    Error err = ERR_OK;
    if ( m_physics != nullptr )
    {
        err = m_physics->GetError();
    }
    if ( m_auto != nullptr )
    {
        err = m_auto->GetError();
    }
    if ( err != ERR_OK && bDisplayError )
    {
        m_main->DisplayError(err, this);
    }
}

// Indicates whether the object is selected or not.

bool COldObject::GetSelect()
{
    return m_bSelect;
}


// Indicates whether the object is selectable or not.

void COldObject::SetSelectable(bool bMode)
{
    m_bSelectable = bMode;
}

// Indicates whether the object is selecionnable or not.

bool COldObject::GetSelectable()
{
    return m_bSelectable;
}


// Indicates if necessary to check the tokens of the object.

void COldObject::SetCheckToken(bool bMode)
{
    m_bCheckToken = bMode;
}

// Indicates if necessary to check the tokens of the object.

bool COldObject::GetCheckToken()
{
    return m_bCheckToken;
}


// Sets if this object is underground or not. Underground objects are not detectable. Used by AlienWorm

void COldObject::SetUnderground(bool underground)
{
    m_underground = underground;
}


// Management of the method of increasing damage.

void COldObject::SetMagnifyDamage(float factor)
{
    m_magnifyDamage = factor;
}

float COldObject::GetMagnifyDamage()
{
    return m_magnifyDamage;
}

void COldObject::SetDamaging(bool damaging)
{
    m_damaging = damaging;
}

bool COldObject::IsDamaging()
{
    return m_damaging;
}

void COldObject::SetDying(DeathType deathType)
{
    m_dying = deathType;
    m_burnTime = 0.0f;

    if ( IsDying() && Implements(ObjectInterfaceType::Programmable) )
    {
        StopProgram();  // stops the current task
    }
}

DeathType COldObject::GetDying()
{
    return m_dying;
}

bool COldObject::IsDying() const
{
    return m_dying != DeathType::Alive;
}

bool COldObject::GetActive()
{
    // Dying astronaut (m_dying == DeathType::Dead) should be treated as active
    // This is for EndMissionTake to not detect him as actually dead until the animation is finished

    return !GetLock() && !(Implements(ObjectInterfaceType::Destroyable) && IsDying() && GetDying() != DeathType::Dead) && !m_bFlat;
}

bool COldObject::GetDetectable()
{
    return GetActive() && !m_underground;
}


// Management of the point of aim.

void COldObject::SetGunGoalV(float gunGoal)
{
    if ( m_type == OBJECT_MOBILEfc ||
         m_type == OBJECT_MOBILEtc ||
         m_type == OBJECT_MOBILEwc ||
         m_type == OBJECT_MOBILEic ||
         m_type == OBJECT_MOBILEfb ||
         m_type == OBJECT_MOBILEtb ||
         m_type == OBJECT_MOBILEwb ||
         m_type == OBJECT_MOBILEib)  // fireball?
    {
        if ( gunGoal >  10.0f*Math::PI/180.0f )  gunGoal =  10.0f*Math::PI/180.0f;
        if ( gunGoal < -20.0f*Math::PI/180.0f )  gunGoal = -20.0f*Math::PI/180.0f;
        SetPartRotationZ(1, gunGoal);
    }
    else if ( m_type == OBJECT_MOBILEfi ||
              m_type == OBJECT_MOBILEti ||
              m_type == OBJECT_MOBILEwi ||
              m_type == OBJECT_MOBILEii )  // orgaball?
    {
        if ( gunGoal >  20.0f*Math::PI/180.0f )  gunGoal =  20.0f*Math::PI/180.0f;
        if ( gunGoal < -20.0f*Math::PI/180.0f )  gunGoal = -20.0f*Math::PI/180.0f;
        SetPartRotationZ(1, gunGoal);
    }
    else if ( m_type == OBJECT_MOBILErc )  // phazer?
    {
        if ( gunGoal >  45.0f*Math::PI/180.0f )  gunGoal =  45.0f*Math::PI/180.0f;
        if ( gunGoal < -20.0f*Math::PI/180.0f )  gunGoal = -20.0f*Math::PI/180.0f;
        SetPartRotationZ(2, gunGoal);
    }
    else
    {
        gunGoal = 0.0f;
    }

    m_gunGoalV = gunGoal;
}

void COldObject::SetGunGoalH(float gunGoal)
{
    if ( m_type == OBJECT_MOBILEfc ||
         m_type == OBJECT_MOBILEtc ||
         m_type == OBJECT_MOBILEwc ||
         m_type == OBJECT_MOBILEic ||
         m_type == OBJECT_MOBILEfb ||
         m_type == OBJECT_MOBILEtb ||
         m_type == OBJECT_MOBILEwb ||
         m_type == OBJECT_MOBILEib)  // fireball?
    {
        if ( gunGoal >  40.0f*Math::PI/180.0f )  gunGoal =  40.0f*Math::PI/180.0f;
        if ( gunGoal < -40.0f*Math::PI/180.0f )  gunGoal = -40.0f*Math::PI/180.0f;
        SetPartRotationY(1, gunGoal);
    }
    else if ( m_type == OBJECT_MOBILEfi ||
              m_type == OBJECT_MOBILEti ||
              m_type == OBJECT_MOBILEwi ||
              m_type == OBJECT_MOBILEii )  // orgaball?
    {
        if ( gunGoal >  40.0f*Math::PI/180.0f )  gunGoal =  40.0f*Math::PI/180.0f;
        if ( gunGoal < -40.0f*Math::PI/180.0f )  gunGoal = -40.0f*Math::PI/180.0f;
        SetPartRotationY(1, gunGoal);
    }
    else if ( m_type == OBJECT_MOBILErc )  // phazer?
    {
        if ( gunGoal >  40.0f*Math::PI/180.0f )  gunGoal =  40.0f*Math::PI/180.0f;
        if ( gunGoal < -40.0f*Math::PI/180.0f )  gunGoal = -40.0f*Math::PI/180.0f;
        SetPartRotationY(2, gunGoal);
    }
    else
    {
        gunGoal = 0.0f;
    }

    m_gunGoalH = gunGoal;
}

float COldObject::GetGunGoalV()
{
    return m_gunGoalV;
}

float COldObject::GetGunGoalH()
{
    return m_gunGoalH;
}

float COldObject::GetShowLimitRadius()
{
    if ( m_type == OBJECT_BASE     ) return 200.0f; // SpaceShip
    if ( m_type == OBJECT_MOBILErt ) return 400.0f; // Thumper
    if ( m_type == OBJECT_TOWER    ) return Gfx::LTNG_PROTECTION_RADIUS; // DefenseTower
    if ( m_type == OBJECT_PARA     ) return Gfx::LTNG_PROTECTION_RADIUS; // PowerCaptor
    return 0.0f;
}


// Creates or removes particles associated to the object.

void COldObject::CreateSelectParticle()
{
    glm::vec3    pos, speed;
    glm::vec2     dim;
    int         i;

    // Removes particles preceding.
    for ( i=0 ; i<4 ; i++ )
    {
        if ( m_partiSel[i] != -1 )
        {
            m_particle->DeleteParticle(m_partiSel[i]);
            m_partiSel[i] = -1;
        }
    }

    if ( m_bSelect || IsProgram() || m_main->GetMissionType() == MISSION_RETRO )
    {
        // Creates particles lens for the headlights.
        if ( m_type == OBJECT_MOBILEfa ||
             m_type == OBJECT_MOBILEta ||
             m_type == OBJECT_MOBILEwa ||
             m_type == OBJECT_MOBILEia ||
             m_type == OBJECT_MOBILEfb ||
             m_type == OBJECT_MOBILEtb ||
             m_type == OBJECT_MOBILEwb ||
             m_type == OBJECT_MOBILEib ||
             m_type == OBJECT_MOBILEfc ||
             m_type == OBJECT_MOBILEtc ||
             m_type == OBJECT_MOBILEwc ||
             m_type == OBJECT_MOBILEic ||
             m_type == OBJECT_MOBILEfi ||
             m_type == OBJECT_MOBILEti ||
             m_type == OBJECT_MOBILEwi ||
             m_type == OBJECT_MOBILEii ||
             m_type == OBJECT_MOBILEfs ||
             m_type == OBJECT_MOBILEts ||
             m_type == OBJECT_MOBILEws ||
             m_type == OBJECT_MOBILEis ||
             m_type == OBJECT_MOBILErt ||
             m_type == OBJECT_MOBILErc ||
             m_type == OBJECT_MOBILErr ||
             m_type == OBJECT_MOBILErs ||
             m_type == OBJECT_MOBILEsa ||
             m_type == OBJECT_MOBILEtg ||
             m_type == OBJECT_MOBILEft ||
             m_type == OBJECT_MOBILEtt ||
             m_type == OBJECT_MOBILEwt ||
             m_type == OBJECT_MOBILEit ||
             m_type == OBJECT_MOBILErp ||
             m_type == OBJECT_MOBILEst ||
             m_type == OBJECT_MOBILEdr )  // vehicle?
        {
            pos = glm::vec3(0.0f, 0.0f, 0.0f);
            speed = glm::vec3(0.0f, 0.0f, 0.0f);
            dim.x = 0.0f;
            dim.y = 0.0f;
            m_partiSel[0] = m_particle->CreateParticle(pos, speed, dim, Gfx::PARTISELY, 1.0f, 0.0f, 0.0f);
            m_partiSel[1] = m_particle->CreateParticle(pos, speed, dim, Gfx::PARTISELY, 1.0f, 0.0f, 0.0f);
            m_partiSel[2] = m_particle->CreateParticle(pos, speed, dim, Gfx::PARTISELR, 1.0f, 0.0f, 0.0f);
            m_partiSel[3] = m_particle->CreateParticle(pos, speed, dim, Gfx::PARTISELR, 1.0f, 0.0f, 0.0f);
            UpdateSelectParticle();
        }
    }
}

// Updates the particles associated to the object.

void COldObject::UpdateSelectParticle()
{
    glm::vec3    pos[4];
    glm::vec2     dim[4];
    float       zoom[4];
    float       angle;
    int         i;

    if ( !m_bSelect && !IsProgram() && m_main->GetMissionType() != MISSION_RETRO )  return;

    dim[0].x = 1.0f;
    dim[1].x = 1.0f;
    dim[2].x = 1.2f;
    dim[3].x = 1.2f;

    // Lens front yellow.
    if ( m_type == OBJECT_MOBILErt ||
         m_type == OBJECT_MOBILErc ||
         m_type == OBJECT_MOBILErr ||
         m_type == OBJECT_MOBILErs ||
         m_type == OBJECT_MOBILErp )  // large caterpillars?
    {
        pos[0] = glm::vec3(4.2f, 2.8f,  1.5f);
        pos[1] = glm::vec3(4.2f, 2.8f, -1.5f);
        dim[0].x = 1.5f;
        dim[1].x = 1.5f;
    }
    else if ( m_type == OBJECT_MOBILEsa ||
              m_type == OBJECT_MOBILEst )  // submarine?
    {
        pos[0] = glm::vec3(3.6f, 4.0f,  2.0f);
        pos[1] = glm::vec3(3.6f, 4.0f, -2.0f);
    }
    else if ( m_type == OBJECT_MOBILEtg )  // target?
    {
        pos[0] = glm::vec3(3.4f, 6.5f,  2.0f);
        pos[1] = glm::vec3(3.4f, 6.5f, -2.0f);
    }
    else if ( m_type == OBJECT_MOBILEdr )  // designer?
    {
        pos[0] = glm::vec3(4.9f, 3.5f,  2.5f);
        pos[1] = glm::vec3(4.9f, 3.5f, -2.5f);
    }
    else if ( m_type == OBJECT_MOBILEwt ||
              m_type == OBJECT_MOBILEtt ||
              m_type == OBJECT_MOBILEft ||
              m_type == OBJECT_MOBILEit ||
              GetTrainer())                // trainer ?
    {
        pos[0] = glm::vec3(4.2f, 2.5f,  1.2f);
        pos[1] = glm::vec3(4.2f, 2.5f, -1.2f);
        dim[0].x = 1.5f;
        dim[1].x = 1.5f;
    }
    else
    {
        pos[0] = glm::vec3(4.2f, 2.5f,  1.5f);
        pos[1] = glm::vec3(4.2f, 2.5f, -1.5f);
    }

    // Red back lens
    if ( m_type == OBJECT_MOBILEwt ||
         m_type == OBJECT_MOBILEtt ||
         m_type == OBJECT_MOBILEft ||
         m_type == OBJECT_MOBILEit ||
         GetTrainer())               // trainer?
    {
        pos[2] = glm::vec3(-4.0f, 2.5f,  2.2f);
        pos[3] = glm::vec3(-4.0f, 2.5f, -2.2f);
    }
    else if ( m_type == OBJECT_MOBILEfa ||
              m_type == OBJECT_MOBILEfb ||
              m_type == OBJECT_MOBILEfc ||
              m_type == OBJECT_MOBILEfi ||
              m_type == OBJECT_MOBILEfs )  // flying?
    {
        pos[2] = glm::vec3(-4.0f, 3.1f,  4.5f);
        pos[3] = glm::vec3(-4.0f, 3.1f, -4.5f);
        dim[2].x = 0.6f;
        dim[3].x = 0.6f;
    }
    else if ( m_type == OBJECT_MOBILEwa ||
              m_type == OBJECT_MOBILEwb ||
              m_type == OBJECT_MOBILEwc ||
              m_type == OBJECT_MOBILEwi ||
              m_type == OBJECT_MOBILEws )  // wheels?
    {
        pos[2] = glm::vec3(-4.5f, 2.7f,  2.8f);
        pos[3] = glm::vec3(-4.5f, 2.7f, -2.8f);
    }
    else if ( m_type == OBJECT_MOBILEia ||
              m_type == OBJECT_MOBILEib ||
              m_type == OBJECT_MOBILEic ||
              m_type == OBJECT_MOBILEii ||
              m_type == OBJECT_MOBILEis )  // legs?
    {
        pos[2] = glm::vec3(-4.5f, 2.7f,  2.8f);
        pos[3] = glm::vec3(-4.5f, 2.7f, -2.8f);
    }
    else if ( m_type == OBJECT_MOBILEta ||
              m_type == OBJECT_MOBILEtb ||
              m_type == OBJECT_MOBILEtc ||
              m_type == OBJECT_MOBILEti ||
              m_type == OBJECT_MOBILEts )  // caterpillars?
    {
        pos[2] = glm::vec3(-3.6f, 4.2f,  3.0f);
        pos[3] = glm::vec3(-3.6f, 4.2f, -3.0f);
    }
    else if ( m_type == OBJECT_MOBILErt ||
              m_type == OBJECT_MOBILErc ||
              m_type == OBJECT_MOBILErr ||
              m_type == OBJECT_MOBILErs )  // large caterpillars?
    {
        pos[2] = glm::vec3(-5.0f, 5.2f,  2.5f);
        pos[3] = glm::vec3(-5.0f, 5.2f, -2.5f);
    }
    if ( m_type == OBJECT_MOBILErp || ( GetTrainer() &&
       ( m_type == OBJECT_MOBILErt ||
         m_type == OBJECT_MOBILErc ||
         m_type == OBJECT_MOBILErr ||
         m_type == OBJECT_MOBILErs)))  // large caterpillars (trainer)?
    {
        pos[2] = glm::vec3(-4.6f, 5.2f,  2.6f);
        pos[3] = glm::vec3(-4.6f, 5.2f, -2.6f);
    }
    if ( m_type == OBJECT_MOBILEsa ||
         m_type == OBJECT_MOBILEst )  // submarine?
    {
        pos[2] = glm::vec3(-3.6f, 4.0f,  2.0f);
        pos[3] = glm::vec3(-3.6f, 4.0f, -2.0f);
    }
    if ( m_type == OBJECT_MOBILEtg )  // target?
    {
        pos[2] = glm::vec3(-2.4f, 6.5f,  2.0f);
        pos[3] = glm::vec3(-2.4f, 6.5f, -2.0f);
    }
    if ( m_type == OBJECT_MOBILEdr )  // designer?
    {
        pos[2] = glm::vec3(-5.3f, 2.7f,  1.8f);
        pos[3] = glm::vec3(-5.3f, 2.7f, -1.8f);
    }

    angle = GetRotationY()/Math::PI;

    zoom[0] = 1.0f;
    zoom[1] = 1.0f;
    zoom[2] = 1.0f;
    zoom[3] = 1.0f;

    if ( ( IsProgram() ||  // current program?
         m_main->GetMissionType() == MISSION_RETRO ) && // Retro mode?
         Math::Mod(m_aTime, 0.7f) < 0.3f )
    {
        zoom[0] = 0.0f;  // blinks
        zoom[1] = 0.0f;
        zoom[2] = 0.0f;
        zoom[3] = 0.0f;
    }

    // Updates lens.
    for ( i=0 ; i<4 ; i++ )
    {
        if (m_partiSel[i] == -1) continue;
        pos[i] = Math::Transform(m_objectPart[0].matWorld, pos[i]);
        dim[i].y = dim[i].x;
        m_particle->SetParam(m_partiSel[i], pos[i], dim[i], zoom[i], angle, 1.0f);
    }
}


// Returns the physics associated to the object.

CPhysics* COldObject::GetPhysics()
{
    return m_physics.get();
}

// Returns the movement associated to the object.

CMotion* COldObject::GetMotion()
{
    return m_motion.get();
}

// TODO: Temporary hack until we'll have subclasses for objects
void COldObject::SetProgrammable()
{
    m_implementedInterfaces[static_cast<int>(ObjectInterfaceType::ProgramStorage)] = true;
    m_implementedInterfaces[static_cast<int>(ObjectInterfaceType::Programmable)] = true;
}

// TODO: Another hack
void COldObject::SetMovable(std::unique_ptr<CMotion> motion, std::unique_ptr<CPhysics> physics)
{
    m_motion = std::move(motion);
    m_physics = std::move(physics);
    m_implementedInterfaces[static_cast<int>(ObjectInterfaceType::Movable)] = true;
}

// Returns the controller associated to the object.

CAuto* COldObject::GetAuto()
{
    return m_auto.get();
}

void COldObject::SetAuto(std::unique_ptr<CAuto> automat)
{
    m_auto = std::move(automat);
}


glm::vec3 COldObject::GetPosition() const
{
    return GetPartPosition(0);
}

void COldObject::SetPosition(const glm::vec3& pos)
{
    SetPartPosition(0, pos);
}

glm::vec3 COldObject::GetRotation() const
{
    return GetPartRotation(0);
}

void COldObject::SetRotation(const glm::vec3& rotation)
{
    SetPartRotation(0, rotation);
}

glm::vec3 COldObject::GetScale() const
{
    return GetPartScale(0);
}

void COldObject::SetScale(const glm::vec3& scale)
{
    SetPartScale(0, scale);
}

void COldObject::UpdateInterface()
{
    if (m_objectInterface != nullptr && GetSelect())
    {
        m_objectInterface->UpdateInterface();
    }

    CreateSelectParticle();
    m_main->UpdateShortcuts();
}

void COldObject::StopProgram()
{
    CProgrammableObjectImpl::StopProgram();

    //TODO: I don't want CProgrammableObjectImpl to depend on motion and physics, refactor this somehow
    m_physics->SetMotorSpeedX(0.0f);
    m_physics->SetMotorSpeedY(0.0f);
    m_physics->SetMotorSpeedZ(0.0f);

    if (m_type != OBJECT_HUMAN) // Be sure not to stop the death animation!
    {
        m_motion->SetAction(-1);
    }
}

// State management of the pencil drawing robot.

bool COldObject::GetTraceDown()
{
    return m_traceDown;
}

void COldObject::SetTraceDown(bool down)
{
    m_traceDown = down;
}

TraceColor COldObject::GetTraceColor()
{
    return m_traceColor;
}

void COldObject::SetTraceColor(TraceColor color)
{
    m_traceColor = color;
}

float COldObject::GetTraceWidth()
{
    return m_traceWidth;
}

void COldObject::SetTraceWidth(float width)
{
    m_traceWidth = width;
}

bool COldObject::IsRepairable()
{
    if (m_type == OBJECT_HUMAN) return false;
    return true;
}

float COldObject::GetShieldFullRegenTime()
{
    if (m_type == OBJECT_HUMAN) return 120.0f;
    assert(false);
    return 0.0f;
}

float COldObject::GetLightningHitProbability()
{
    if ( m_type == OBJECT_BASE     ||
         m_type == OBJECT_DERRICK  ||
         m_type == OBJECT_FACTORY  ||
         m_type == OBJECT_REPAIR   ||
         m_type == OBJECT_DESTROYER||
         m_type == OBJECT_STATION  ||
         m_type == OBJECT_CONVERT  ||
         m_type == OBJECT_TOWER    ||
         m_type == OBJECT_RESEARCH ||
         m_type == OBJECT_RADAR    ||
         m_type == OBJECT_INFO     ||
         m_type == OBJECT_ENERGY   ||
         m_type == OBJECT_LABO     ||
         m_type == OBJECT_NUCLEAR  ||
         m_type == OBJECT_PARA     ||
         m_type == OBJECT_SAFE     ||
         m_type == OBJECT_HUSTON   )  // building?
    {
        return 1.0f;
    }
    if ( m_type == OBJECT_METAL    ||
         m_type == OBJECT_POWER    ||
         m_type == OBJECT_ATOMIC   ) // resource?
    {
        return 0.3f;
    }
    if ( m_type == OBJECT_MOBILEfa ||
         m_type == OBJECT_MOBILEta ||
         m_type == OBJECT_MOBILEwa ||
         m_type == OBJECT_MOBILEia ||
         m_type == OBJECT_MOBILEfb ||
         m_type == OBJECT_MOBILEtb ||
         m_type == OBJECT_MOBILEwb ||
         m_type == OBJECT_MOBILEib ||
         m_type == OBJECT_MOBILEfc ||
         m_type == OBJECT_MOBILEtc ||
         m_type == OBJECT_MOBILEwc ||
         m_type == OBJECT_MOBILEic ||
         m_type == OBJECT_MOBILEfi ||
         m_type == OBJECT_MOBILEti ||
         m_type == OBJECT_MOBILEwi ||
         m_type == OBJECT_MOBILEii ||
         m_type == OBJECT_MOBILEfs ||
         m_type == OBJECT_MOBILEts ||
         m_type == OBJECT_MOBILEws ||
         m_type == OBJECT_MOBILEis ||
         m_type == OBJECT_MOBILErt ||
         m_type == OBJECT_MOBILErc ||
         m_type == OBJECT_MOBILErr ||
         m_type == OBJECT_MOBILErs ||
         m_type == OBJECT_MOBILEsa ||
         m_type == OBJECT_MOBILEft ||
         m_type == OBJECT_MOBILEtt ||
         m_type == OBJECT_MOBILEwt ||
         m_type == OBJECT_MOBILEit ||
         m_type == OBJECT_MOBILErp ||
         m_type == OBJECT_MOBILEst ||
         m_type == OBJECT_MOBILEtg ||
         m_type == OBJECT_MOBILEdr )  // robot?
    {
        return 0.5f;
    }
    return 0.0f;
}

bool COldObject::IsSelectableByDefault(ObjectType type)
{
    if ( type == OBJECT_MOTHER   ||
         type == OBJECT_ANT      ||
         type == OBJECT_SPIDER   ||
         type == OBJECT_BEE      ||
         type == OBJECT_WORM     )
    {
        return false;
    }
    return true;
}

void COldObject::SetBulletWall(bool bulletWall)
{
    m_bulletWall = bulletWall;
}

bool COldObject::IsBulletWall()
{
    return m_bulletWall;
}

bool COldObject::IsBulletWallByDefault(ObjectType type)
{
    if ( type == OBJECT_BARRICADE0 ||
         type == OBJECT_BARRICADE1 )
    {
        return true;
    }
    return false;
}
