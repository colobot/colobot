/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2014, Daniel Roux, EPSITEC SA & TerranovaTeam
 * http://epsiteс.ch; http://colobot.info; http://github.com/colobot
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

#include "CBot/CBotDll.h"

#include "app/app.h"

#include "common/global.h"
#include "common/restext.h"

#include "graphics/engine/lightman.h"
#include "graphics/engine/lightning.h"
#include "graphics/engine/particle.h"
#include "graphics/engine/pyro_manager.h"
#include "graphics/engine/terrain.h"

#include "math/geometry.h"

#include "object/auto/auto.h"
#include "object/auto/autojostle.h"
#include "object/brain.h"
#include "object/motion/motion.h"
#include "object/motion/motionvehicle.h"
#include "object/robotmain.h"
#include "object/object_manager.h"
#include "object/level/parserline.h"
#include "object/level/parserparam.h"
#include "object/level/parserexceptions.h"

#include "physics/physics.h"

#include "script/cbottoken.h"

#include <boost/lexical_cast.hpp>



#define ADJUST_ONBOARD  0       // 1 -> adjusts the camera ONBOARD
#define ADJUST_ARM  0           // 1 -> adjusts the manipulator arm
const float VIRUS_DELAY     = 60.0f;        // duration of virus infection
const float LOSS_SHIELD     = 0.24f;        // loss of the shield by shot
const float LOSS_SHIELD_H   = 0.10f;        // loss of the shield for humans
const float LOSS_SHIELD_M   = 0.02f;        // loss of the shield for the laying

#if ADJUST_ONBOARD
static float debug_x = 0.0f;
static float debug_y = 0.0f;
static float debug_z = 0.0f;
#endif

#if ADJUST_ARM
static float debug_arm1 = 0.0f;
static float debug_arm2 = 0.0f;
static float debug_arm3 = 0.0f;
#endif




// Updates the class Object.

void uObject(CBotVar* botThis, void* user)
{
    CPhysics*   physics;
    CBotVar     *pVar, *pSub;
    ObjectType  type;
    Math::Vector    pos;
    float       value;

    if ( user == nullptr )  return;

    assert(static_cast<CObject*>(user)->Implements(ObjectInterfaceType::Old));
    COldObject* object = static_cast<COldObject*>(user);

    physics = object->GetPhysics();

    // Updates the object's type.
    pVar = botThis->GetItemList();  // "category"
    type = object->GetType();
    pVar->SetValInt(type, object->GetName());

    // Updates the position of the object.
    pVar = pVar->GetNext();  // "position"
    if (IsObjectBeingTransported(object))
    {
        pSub = pVar->GetItemList();  // "x"
        pSub->SetInit(CBotVar::InitType::IS_NAN);
        pSub = pSub->GetNext();  // "y"
        pSub->SetInit(CBotVar::InitType::IS_NAN);
        pSub = pSub->GetNext();  // "z"
        pSub->SetInit(CBotVar::InitType::IS_NAN);
    }
    else
    {
        pos = object->GetPosition();
        float waterLevel = Gfx::CEngine::GetInstancePointer()->GetWater()->GetLevel();
        pos.y -= waterLevel;  // relative to sea level!
        pSub = pVar->GetItemList();  // "x"
        pSub->SetValFloat(pos.x/g_unit);
        pSub = pSub->GetNext();  // "y"
        pSub->SetValFloat(pos.z/g_unit);
        pSub = pSub->GetNext();  // "z"
        pSub->SetValFloat(pos.y/g_unit);
    }

    // Updates the angle.
    pos = object->GetRotation();
    pos += object->GetTilt();
    pVar = pVar->GetNext();  // "orientation"
    pVar->SetValFloat(360.0f-Math::Mod(pos.y*180.0f/Math::PI, 360.0f));
    pVar = pVar->GetNext();  // "pitch"
    pVar->SetValFloat(pos.z*180.0f/Math::PI);
    pVar = pVar->GetNext();  // "roll"
    pVar->SetValFloat(pos.x*180.0f/Math::PI);

    // Updates the energy level of the object.
    pVar = pVar->GetNext();  // "energyLevel"
    value = object->GetEnergy();
    pVar->SetValFloat(value);

    // Updates the shield level of the object.
    pVar = pVar->GetNext();  // "shieldLevel"
    value = object->GetShield();
    pVar->SetValFloat(value);

    // Updates the temperature of the reactor.
    pVar = pVar->GetNext();  // "temperature"
    if ( physics == 0 )  value = 0.0f;
    else                 value = 1.0f-physics->GetReactorRange();
    pVar->SetValFloat(value);

    // Updates the height above the ground.
    pVar = pVar->GetNext();  // "altitude"
    if ( physics == 0 )  value = 0.0f;
    else                 value = physics->GetFloorHeight();
    pVar->SetValFloat(value/g_unit);

    // Updates the lifetime of the object.
    pVar = pVar->GetNext();  // "lifeTime"
    value = object->GetAbsTime();
    pVar->SetValFloat(value);

    // Updates the type of battery.
    pVar = pVar->GetNext();  // "energyCell"
    if (object->Implements(ObjectInterfaceType::Powered))
    {
        CObject* power = dynamic_cast<CPoweredObject*>(object)->GetPower();
        if (power == nullptr)
        {
            pVar->SetPointer(nullptr);
        }
        else if (power->Implements(ObjectInterfaceType::Old))
        {
            pVar->SetPointer(dynamic_cast<COldObject*>(power)->GetBotVar());
        }
    }

    // Updates the transported object's type.
    pVar = pVar->GetNext();  // "load"
    if (object->Implements(ObjectInterfaceType::Carrier))
    {
        CObject* cargo = dynamic_cast<CCarrierObject*>(object)->GetCargo();
        if (cargo == nullptr)
        {
            pVar->SetPointer(nullptr);
        }
        else if (cargo->Implements(ObjectInterfaceType::Old))
        {
            pVar->SetPointer(dynamic_cast<COldObject*>(cargo)->GetBotVar());
        }
    }

    pVar = pVar->GetNext();  // "id"
    value = object->GetID();
    pVar->SetValInt(value);

    pVar = pVar->GetNext();  // "team"
    value = object->GetTeam();
    pVar->SetValInt(value);
}




// Object's constructor.

COldObject::COldObject(int id)
    : CObject(id, OBJECT_NULL)
    , CInteractiveObject(m_implementedInterfaces)
    , CTransportableObject(m_implementedInterfaces)
    , CProgrammableObject(m_implementedInterfaces)
    , CJostleableObject(m_implementedInterfaces)
    , CCarrierObject(m_implementedInterfaces)
    , CPoweredObject(m_implementedInterfaces)
{
    // A bit of a hack since CBrain is set externally in SetBrain()
    m_implementedInterfaces[static_cast<int>(ObjectInterfaceType::Programmable)] = false;
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
    m_runScript   = nullptr;

    m_type = OBJECT_FIX;
    m_option = 0;
    m_name = "";
    m_shadowLight   = -1;
    m_effectLight   = -1;
    m_linVibration  = Math::Vector(0.0f, 0.0f, 0.0f);
    m_cirVibration  = Math::Vector(0.0f, 0.0f, 0.0f);
    m_tilt   = Math::Vector(0.0f, 0.0f, 0.0f);

    m_power = 0;
    m_cargo  = 0;
    m_transporter = 0;
    m_transporterLink = 0;
    m_energy   = 1.0f;
    m_capacity = 1.0f;
    m_shield   = 1.0f;
    m_range    = 0.0f;
    m_transparency = 0.0f;
    m_lastEnergy = 999.9f;
    m_bSelect = false;
    m_bSelectable = true;
    m_bCheckToken = true;
    m_bVisible = true;
    m_bEnable = true;
    m_bProxyActivate = false;
    m_bTrainer = false;
    m_bToy = false;
    m_bManual = false;
    m_bFixed = false;
    m_bClip = true;
    m_bShowLimit = false;
    m_showLimitRadius = 0.0f;
    m_aTime = 0.0f;
    m_shotTime = 0.0f;
    m_bVirusMode = false;
    m_virusTime = 0.0f;
    m_lastVirusParticle = 0.0f;
    m_bLock  = false;
    m_bIgnoreBuildCheck = false;
    m_bExplo = false;
    m_bCargo = false;
    m_bBurn  = false;
    m_bDead  = false;
    m_bFlat  = false;
    m_gunGoalV = 0.0f;
    m_gunGoalH = 0.0f;
    m_shieldRadius = 0.0f;
    m_defRank = -1;
    m_magnifyDamage = 1.0f;
    m_proxyDistance = 60.0f;
    m_param = 0.0f;
    m_infoReturn = NAN;
    m_team = 0;

    memset(&m_character, 0, sizeof(m_character));
    m_character.wheelFront = 1.0f;
    m_character.wheelBack  = 1.0f;
    m_character.wheelLeft  = 1.0f;
    m_character.wheelRight = 1.0f;

    m_cameraType = Gfx::CAM_TYPE_BACK;
    m_cameraDist = 50.0f;
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

    m_cmdLine.clear();

    DeleteAllCrashSpheres();

    CBotClass* bc = CBotClass::Find("object");
    if ( bc != 0 )
    {
        bc->AddUpdateFunc(uObject);
    }

    m_botVar = CBotVar::Create("", CBotTypResult(CBotTypClass, "object"));
    m_botVar->SetUserPtr(this);
    m_botVar->SetIdent(m_id);
}

// Object's destructor.

COldObject::~COldObject()
{
    if ( m_botVar != nullptr )
    {
        m_botVar->SetUserPtr(OBJECTDELETED);
        delete m_botVar;
        m_botVar = nullptr;
    }
}


// Removes an object.
// If bAll = true, it does not help,
// because all objects in the scene are quickly destroyed!

void COldObject::DeleteObject(bool bAll)
{
    if ( m_botVar != 0 )
    {
        m_botVar->SetUserPtr(OBJECTDELETED);
    }

    if ( m_camera->GetControllingObject() == this )
    {
        m_camera->SetControllingObject(0);
    }
    m_main->RemoveFromSelectionHistory(this);

    if ( !bAll )
    {
#if 0
        type = m_camera->GetType();
        if ( (type == Gfx::CAM_TYPE_BACK   ||
              type == Gfx::CAM_TYPE_FIX    ||
              type == Gfx::CAM_TYPE_EXPLO  ||
              type == Gfx::CAM_TYPE_ONBOARD) &&
             m_camera->GetControllingObject() == this )
        {
            obj = m_main->SearchNearest(GetPosition(), this);
            if ( obj == 0 )
            {
                m_camera->SetControllingObject(0);
                m_camera->SetType(Gfx::CAM_TYPE_FREE);
            }
            else
            {
                m_camera->SetControllingObject(obj);
                m_camera->SetType(Gfx::CAM_TYPE_BACK);
            }
        }
#endif
        m_engine->GetPyroManager()->CutObjectLink(this);

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

    if ( m_effectLight != -1 )
    {
        m_lightMan->DeleteLight(m_effectLight);
        m_effectLight = -1;
    }

    if ( m_physics != nullptr )
    {
        m_physics->DeleteObject(bAll);
    }

    if ( m_brain != nullptr )
    {
        m_brain->DeleteObject(bAll);
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

    if ( m_bShowLimit )
    {
        m_main->FlushShowLimit(0);
        m_bShowLimit = false;
    }

    if ( !bAll )  m_main->CreateShortcuts();
}

// Simplifies a object (he was the brain, among others).

void COldObject::Simplify()
{
    m_implementedInterfaces[static_cast<int>(ObjectInterfaceType::Programmable)] = false;

    if ( m_brain != nullptr )
    {
        m_brain->StopProgram();
    }
    m_main->SaveOneScript(this);

    if ( m_physics != nullptr )
    {
        m_physics->DeleteObject();
        m_physics.reset();
    }

    if ( m_brain != nullptr )
    {
        m_brain->DeleteObject();
        m_brain.reset();
    }

    if ( m_motion != nullptr )
    {
        m_motion->DeleteObject();
        m_motion.reset();
    }

    if ( m_auto != nullptr )
    {
        m_auto->DeleteObject();
        m_auto.reset();
    }

    m_main->CreateShortcuts();
}


// Detonates an object, when struck by a shot.
// If false is returned, the object is still screwed.
// If true is returned, the object is destroyed.

bool COldObject::ExplodeObject(ExplosionType type, float force, float decay)
{
    Gfx::PyroType    pyroType;
    float       loss, shield;

    if ( type == ExplosionType::Burn )
    {
        if ( m_type == OBJECT_MOBILEtg ||
             m_type == OBJECT_METAL    ||
             m_type == OBJECT_POWER    ||
             m_type == OBJECT_ATOMIC   ||
             m_type == OBJECT_TNT      ||
             m_type == OBJECT_SCRAP1   ||
             m_type == OBJECT_SCRAP2   ||
             m_type == OBJECT_SCRAP3   ||
             m_type == OBJECT_SCRAP4   ||
             m_type == OBJECT_SCRAP5   ||
             m_type == OBJECT_BULLET   ||
             m_type == OBJECT_EGG      )  // object that isn't burning?
        {
            type = ExplosionType::Bang;
            force = 1.0f;
            decay = 1.0f;
        }
    }

    if ( type == ExplosionType::Bang )
    {
        if ( m_shotTime < 0.5f )  return false;
        m_shotTime = 0.0f;
    }

    if ( m_type == OBJECT_HUMAN && m_bDead )  return false;

    // Calculate the power lost by the explosion.
    if ( force == 0.0f )
    {
        if ( m_type == OBJECT_HUMAN )
        {
            loss = LOSS_SHIELD_H;
        }
        else if ( m_type == OBJECT_MOTHER )
        {
            loss = LOSS_SHIELD_M;
        }
        else
        {
            loss = LOSS_SHIELD;
        }
    }
    else
    {
        loss = force;
    }
    loss *= m_magnifyDamage;
    loss *= decay;

    // Decreases the power of the shield.
    shield = GetShield();
    shield -= loss;
    if ( shield < 0.0f )  shield = 0.0f;
    SetShield(shield);

    if ( shield > 0.0f )  // not dead yet?
    {
        if ( type == ExplosionType::Water )
        {
            if ( m_type == OBJECT_HUMAN )
            {
                pyroType = Gfx::PT_SHOTH;
            }
            else
            {
                pyroType = Gfx::PT_SHOTW;
            }
        }
        else
        {
            if ( m_type == OBJECT_HUMAN )
            {
                pyroType = Gfx::PT_SHOTH;
            }
            else if ( m_type == OBJECT_MOTHER )
            {
                pyroType = Gfx::PT_SHOTM;
            }
            else
            {
                pyroType = Gfx::PT_SHOTT;
            }
        }
    }
    else    // completely dead?
    {
        if ( type == ExplosionType::Burn )  // burning?
        {
            if ( m_type == OBJECT_MOTHER ||
                 m_type == OBJECT_ANT    ||
                 m_type == OBJECT_SPIDER ||
                 m_type == OBJECT_BEE    ||
                 m_type == OBJECT_WORM   ||
                 m_type == OBJECT_BULLET )
            {
                pyroType = Gfx::PT_BURNO;
                SetBurn(true);
            }
            else if ( m_type == OBJECT_HUMAN )
            {
                pyroType = Gfx::PT_DEADG;
            }
            else
            {
                pyroType = Gfx::PT_BURNT;
                SetBurn(true);
            }
            SetVirusMode(false);
        }
        else if ( type == ExplosionType::Water )
        {
            if ( m_type == OBJECT_HUMAN )
            {
                pyroType = Gfx::PT_DEADW;
            }
            else
            {
                pyroType = Gfx::PT_FRAGW;
            }
        }
        else    // explosion?
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
                      m_type == OBJECT_END      )  // building?
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

        loss = 1.0f;
    }

    m_engine->GetPyroManager()->Create(pyroType, this, loss);

    if ( shield == 0.0f )  // dead?
    {
        if ( m_brain != nullptr )
        {
            m_brain->StopProgram();
        }
        m_main->SaveOneScript(this);
    }

    if ( shield > 0.0f )  return false;  // not dead yet

    if ( GetSelect() )
    {
        SetSelect(false);  // deselects the object
        m_camera->SetType(Gfx::CAM_TYPE_EXPLO);
        m_main->DeselectAll();
    }
    m_main->RemoveFromSelectionHistory(this);

    if ( m_botVar != 0 )
    {
        if ( m_type == OBJECT_STONE   ||
             m_type == OBJECT_URANIUM ||
             m_type == OBJECT_METAL   ||
             m_type == OBJECT_POWER   ||
             m_type == OBJECT_ATOMIC  ||
             m_type == OBJECT_BULLET  ||
             m_type == OBJECT_BBOX    ||
             m_type == OBJECT_TNT     ||
             m_type == OBJECT_SCRAP1  ||
             m_type == OBJECT_SCRAP2  ||
             m_type == OBJECT_SCRAP3  ||
             m_type == OBJECT_SCRAP4  ||
             m_type == OBJECT_SCRAP5  )  // (*)
        {
            m_botVar->SetUserPtr(OBJECTDELETED);
        }
    }

    return true;
}

// (*)  If a robot or cosmonaut dies, the subject must continue to exist,
//  so that programs of the ants continue to operate as usual.


// Initializes a new part.

void COldObject::InitPart(int part)
{
    m_objectPart[part].bUsed      = true;
    m_objectPart[part].object     = -1;
    m_objectPart[part].parentPart = -1;

    m_objectPart[part].position   = Math::Vector(0.0f, 0.0f, 0.0f);
    m_objectPart[part].angle.y    = 0.0f;
    m_objectPart[part].angle.x    = 0.0f;
    m_objectPart[part].angle.z    = 0.0f;
    m_objectPart[part].zoom       = Math::Vector(1.0f, 1.0f, 1.0f);

    m_objectPart[part].bTranslate = true;
    m_objectPart[part].bRotate    = true;
    m_objectPart[part].bZoom      = false;

    m_objectPart[part].matTranslate.LoadIdentity();
    m_objectPart[part].matRotate.LoadIdentity();
    m_objectPart[part].matTransform.LoadIdentity();
    m_objectPart[part].matWorld.LoadIdentity();;

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

    if ( m_type == OBJECT_MOBILErs )
    {
        m_param = 1.0f;  // shield up to default
    }

    if ( m_type == OBJECT_ATOMIC )
    {
        m_capacity = 10.0f;
    }
    else
    {
        m_capacity = 1.0f;
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
    Math::Vector    pos;

    line->AddParam("camera", CLevelParserParamUPtr{new CLevelParserParam(GetCameraType())});

    if ( GetCameraLock() )
        line->AddParam("cameraLock", CLevelParserParamUPtr{new CLevelParserParam(GetCameraLock())});

    if ( GetEnergy() != 0.0f )
        line->AddParam("energy", CLevelParserParamUPtr{new CLevelParserParam(GetEnergy())});

    if ( GetCapacity() != 1.0f )
        line->AddParam("capacity", CLevelParserParamUPtr{new CLevelParserParam(GetCapacity())});

    if ( GetShield() != 1.0f )
        line->AddParam("shield", CLevelParserParamUPtr{new CLevelParserParam(GetShield())});

    if ( GetRange() != 1.0f )
        line->AddParam("range", CLevelParserParamUPtr{new CLevelParserParam(GetRange())});

    if ( !GetSelectable() )
        line->AddParam("selectable", CLevelParserParamUPtr{new CLevelParserParam(GetSelectable())});

    if ( !GetEnable() )
        line->AddParam("enable", CLevelParserParamUPtr{new CLevelParserParam(GetEnable())});

    // TODO: doesn't seem to be used
    if ( GetFixed() )
        line->AddParam("fixed", CLevelParserParamUPtr{new CLevelParserParam(GetFixed())});

    if ( !GetClip() )
        line->AddParam("clip", CLevelParserParamUPtr{new CLevelParserParam(GetClip())});

    if ( GetLock() )
        line->AddParam("lock", CLevelParserParamUPtr{new CLevelParserParam(GetLock())});

    if ( GetProxyActivate() )
    {
        line->AddParam("proxyActivate", CLevelParserParamUPtr{new CLevelParserParam(GetProxyActivate())});
        line->AddParam("proxyDistance", CLevelParserParamUPtr{new CLevelParserParam(GetProxyDistance()/g_unit)});
    }

    if ( GetMagnifyDamage() != 1.0f )
        line->AddParam("magnifyDamage", CLevelParserParamUPtr{new CLevelParserParam(GetMagnifyDamage())});

    if ( GetTeam() != 0 )
        line->AddParam("team", CLevelParserParamUPtr{new CLevelParserParam(GetTeam())});

    if ( GetGunGoalV() != 0.0f )
        line->AddParam("aimV", CLevelParserParamUPtr{new CLevelParserParam(GetGunGoalV())});

    if ( GetGunGoalH() != 0.0f )
        line->AddParam("aimH", CLevelParserParamUPtr{new CLevelParserParam(GetGunGoalH())});

    if ( GetAnimateOnReset() )
    {
        line->AddParam("resetCap", CLevelParserParamUPtr{new CLevelParserParam(GetAnimateOnReset())});
    }

    if ( m_bVirusMode )
        line->AddParam("virusMode", CLevelParserParamUPtr{new CLevelParserParam(m_bVirusMode)});

    if ( m_virusTime != 0.0f )
        line->AddParam("virusTime", CLevelParserParamUPtr{new CLevelParserParam(m_virusTime)});

    line->AddParam("ignoreBuildCheck", CLevelParserParamUPtr{new CLevelParserParam(GetIgnoreBuildCheck())});

    // Sets the parameters of the command line.
    CLevelParserParamVec cmdline;
    for(float value : m_cmdLine)
    {
        cmdline.push_back(CLevelParserParamUPtr{new CLevelParserParam(value)});
    }
    if (cmdline.size() > 0)
        line->AddParam("cmdline", CLevelParserParamUPtr{new CLevelParserParam(std::move(cmdline))});

    if ( m_motion != nullptr )
    {
        m_motion->Write(line);
    }

    if ( m_brain != nullptr )
    {
        m_brain->Write(line);
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
    Gfx::CameraType cType = line->GetParam("camera")->AsCameraType(Gfx::CAM_TYPE_NULL);
    if ( cType != Gfx::CAM_TYPE_NULL )
    {
        SetCameraType(cType);
    }

    SetCameraDist(line->GetParam("cameraDist")->AsFloat(50.0f));
    SetCameraLock(line->GetParam("cameraLock")->AsBool(false));
    SetEnergy(line->GetParam("energy")->AsFloat(0.0f));
    SetCapacity(line->GetParam("capacity")->AsFloat(1.0f));
    SetShield(line->GetParam("shield")->AsFloat(1.0f));
    SetRange(line->GetParam("range")->AsFloat(1.0f));
    SetSelectable(line->GetParam("selectable")->AsBool(true));
    SetEnable(line->GetParam("enable")->AsBool(true));
    SetFixed(line->GetParam("fixed")->AsBool(false));
    SetClip(line->GetParam("clip")->AsBool(true));
    SetLock(line->GetParam("lock")->AsBool(false));
    SetProxyActivate(line->GetParam("proxyActivate")->AsBool(false));
    SetProxyDistance(line->GetParam("proxyDistance")->AsFloat(15.0f)*g_unit);
    SetRange(line->GetParam("range")->AsFloat(30.0f));
    SetMagnifyDamage(line->GetParam("magnifyDamage")->AsFloat(1.0f));
    SetTeam(line->GetParam("team")->AsInt(0));
    SetGunGoalV(line->GetParam("aimV")->AsFloat(0.0f));
    SetGunGoalH(line->GetParam("aimH")->AsFloat(0.0f));

    SetAnimateOnReset(line->GetParam("resetCap")->AsBool(false));
    m_bBurn = line->GetParam("burnMode")->AsBool(false);
    m_bVirusMode = line->GetParam("virusMode")->AsBool(false);
    m_virusTime = line->GetParam("virusTime")->AsFloat(0.0f);
    SetIgnoreBuildCheck(line->GetParam("ignoreBuildCheck")->AsBool(false));

    // Sets the parameters of the command line.
    if (line->GetParam("cmdline")->IsDefined())
    {
        int i = 0;
        for (auto& p : line->GetParam("cmdline")->AsArray())
        {
            SetCmdLine(i, p->AsFloat());
            i++;
        }
    }

    if ( m_motion != nullptr )
    {
        m_motion->Read(line);
    }

    if ( m_brain != nullptr )
    {
        m_brain->Read(line);
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
    crashSphere.radius *= GetScaleX();

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


// Specifies the radius of the shield.

void COldObject::SetShieldRadius(float radius)
{
    m_shieldRadius = radius;
}

// Returns the radius of the shield.

float COldObject::GetShieldRadius()
{
    return m_shieldRadius;
}


// Positioning an object on a certain height, above the ground.

void COldObject::SetFloorHeight(float height)
{
    Math::Vector    pos;

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
    Math::Vector        pos, n;
    Math::Point         nn;
    float           a;

    pos = GetPosition();
    if ( m_terrain->GetNormal(n, pos) )
    {
#if 0
        SetRotationX( sinf(n.z));
        SetRotationZ(-sinf(n.x));
        SetRotationY(0.0f);
#else
        a = GetRotationY();
        nn = Math::RotatePoint(-a, Math::Point(n.z, n.x));
        SetRotationX( sinf(nn.x));
        SetRotationZ(-sinf(nn.y));
#endif
    }
}


// Getes the linear vibration.

void COldObject::SetLinVibration(Math::Vector dir)
{
    if ( m_linVibration.x != dir.x ||
         m_linVibration.y != dir.y ||
         m_linVibration.z != dir.z )
    {
        m_linVibration = dir;
        m_objectPart[0].bTranslate = true;
    }
}

Math::Vector COldObject::GetLinVibration()
{
    return m_linVibration;
}

// Getes the circular vibration.

void COldObject::SetCirVibration(Math::Vector dir)
{
    if ( m_cirVibration.x != dir.x ||
         m_cirVibration.y != dir.y ||
         m_cirVibration.z != dir.z )
    {
        m_cirVibration = dir;
        m_objectPart[0].bRotate = true;
    }
}

Math::Vector COldObject::GetCirVibration()
{
    return m_cirVibration;
}

// Getes the inclination.

void COldObject::SetTilt(Math::Vector dir)
{
    if ( m_tilt.x != dir.x ||
         m_tilt.y != dir.y ||
         m_tilt.z != dir.z )
    {
        m_tilt = dir;
        m_objectPart[0].bRotate = true;
    }
}

Math::Vector COldObject::GetTilt()
{
    return m_tilt;
}


// Getes the position of center of the object.

void COldObject::SetPartPosition(int part, const Math::Vector &pos)
{
    m_objectPart[part].position = pos;
    m_objectPart[part].bTranslate = true;  // it will recalculate the matrices

    if ( part == 0 && !m_bFlat )  // main part?
    {
        int rank = m_objectPart[0].object;

        Math::Vector shPos = pos;
        m_terrain->AdjustToFloor(shPos, true);
        m_engine->SetObjectShadowPos(rank, shPos);

        float height = 0.0f;
        if ( m_physics != nullptr && m_physics->GetType() == TYPE_FLYING )
        {
            height = pos.y-shPos.y;
        }
        m_engine->SetObjectShadowHeight(rank, height);

        m_engine->UpdateObjectShadowNormal(rank);

        if ( m_shadowLight != -1 )
        {
            Math::Vector lightPos = pos;
            lightPos.y += m_shadowHeight;
            m_lightMan->SetLightPos(m_shadowLight, lightPos);
        }

        if ( m_effectLight != -1 )
        {
            Math::Vector lightPos = pos;
            lightPos.y += m_effectHeight;
            m_lightMan->SetLightPos(m_effectLight, lightPos);
        }

        if ( m_bShowLimit )
        {
            m_main->AdjustShowLimit(0, pos);
        }
    }
}

Math::Vector COldObject::GetPartPosition(int part) const
{
    return m_objectPart[part].position;
}

// Getes the rotation around three axis.

void COldObject::SetPartRotation(int part, const Math::Vector &angle)
{
    m_objectPart[part].angle = angle;
    m_objectPart[part].bRotate = true;  // it will recalculate the matrices

    if ( part == 0 && !m_bFlat )  // main part?
    {
        m_engine->SetObjectShadowAngle(m_objectPart[0].object, m_objectPart[0].angle.y);
    }
}

Math::Vector COldObject::GetPartRotation(int part) const
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
        m_engine->SetObjectShadowAngle(m_objectPart[0].object, m_objectPart[0].angle.y);
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

void COldObject::SetPartScale(int part, Math::Vector zoom)
{
    m_objectPart[part].bTranslate = true;  // it will recalculate the matrices
    m_objectPart[part].zoom = zoom;

    m_objectPart[part].bZoom = ( m_objectPart[part].zoom.x != 1.0f ||
                                 m_objectPart[part].zoom.y != 1.0f ||
                                 m_objectPart[part].zoom.z != 1.0f );
}

Math::Vector COldObject::GetPartScale(int part) const
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

void COldObject::SetPower(CObject* power)
{
    m_power = power;
}

CObject* COldObject::GetPower()
{
    return m_power;
}

// Management of the object transport.

void COldObject::SetCargo(CObject* cargo)
{
    m_cargo = cargo;
}

CObject* COldObject::GetCargo()
{
    return m_cargo;
}

// Management of the object "transporter" that transports it.

void COldObject::SetTransporter(CObject* transporter)
{
    m_transporter = transporter;

    // Invisible shadow if the object is transported.
    m_engine->SetObjectShadowHide(m_objectPart[0].object, (m_transporter != 0));
}

CObject* COldObject::GetTransporter()
{
    return m_transporter;
}

// Management of the conveying portion.

void COldObject::SetTransporterPart(int part)
{
    m_transporterLink = part;
}

void COldObject::SetInfoReturn(float value)
{
    m_infoReturn = value;
}

float COldObject::GetInfoReturn()
{
    return m_infoReturn;
}

void COldObject::SetCmdLine(unsigned int rank, float value)
{
    if (rank == m_cmdLine.size())
    {
        m_cmdLine.push_back(value);
    }
    else if (rank < m_cmdLine.size())
    {
        m_cmdLine[rank] = value;
    }
    else
    {
        // should never happen
        assert(false);
    }
}

float COldObject::GetCmdLine(unsigned int rank)
{
    if ( rank >= m_cmdLine.size() )  return 0.0f;
    return m_cmdLine[rank];
}


// Returns matrices of an object portion.

Math::Matrix* COldObject::GetRotateMatrix(int part)
{
    return &m_objectPart[part].matRotate;
}

Math::Matrix* COldObject::GetWorldMatrix(int part)
{
    if ( m_objectPart[0].bTranslate ||
         m_objectPart[0].bRotate    )
    {
        UpdateTransformObject();
    }

    return &m_objectPart[part].matWorld;
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

    Math::Vector pos = GetPosition();
    m_shadowHeight = height;

    Gfx::Light light;
    light.type          = Gfx::LIGHT_SPOT;
    light.diffuse       = color;
    light.ambient       = color * 0.1f;
    light.position      = Math::Vector(pos.x, pos.y+height, pos.z);
    light.direction     = Math::Vector(0.0f, -1.0f, 0.0f); // against the bottom
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

// Creates light for the effects of a vehicle.

bool COldObject::CreateEffectLight(float height, Gfx::Color color)
{
    if ( !m_engine->GetLightMode() )  return true;

    m_effectHeight = height;

    Gfx::Light light;
    light.type       = Gfx::LIGHT_SPOT;
    light.diffuse    = color;
    light.position   = Math::Vector(0.0f, height, 0.0f);
    light.direction  = Math::Vector(0.0f, -1.0f, 0.0f); // against the bottom
    light.spotIntensity = 0.0f;
    light.attenuation0 = 1.0f;
    light.attenuation1 = 0.0f;
    light.attenuation2 = 0.0f;
    light.spotAngle = 90.0f*Math::PI/180.0f;

    m_effectLight = m_lightMan->CreateLight();
    if ( m_effectLight == -1 )  return false;

    m_lightMan->SetLight(m_effectLight, light);
    m_lightMan->SetLightIntensity(m_effectLight, 0.0f);

    return true;
}

// Returns the number of light effects.

int COldObject::GetEffectLight()
{
    return m_effectLight;
}

// Creates the circular shadow underneath a vehicle.

bool COldObject::CreateShadowCircle(float radius, float intensity,
                                 Gfx::EngineShadowType type)
{
    float   zoom;

    if ( intensity == 0.0f )  return true;

    zoom = GetScaleX();

    m_engine->CreateShadow(m_objectPart[0].object);

    m_engine->SetObjectShadowRadius(m_objectPart[0].object, radius*zoom);
    m_engine->SetObjectShadowIntensity(m_objectPart[0].object, intensity);
    m_engine->SetObjectShadowHeight(m_objectPart[0].object, 0.0f);
    m_engine->SetObjectShadowAngle(m_objectPart[0].object, m_objectPart[0].angle.y);
    m_engine->SetObjectShadowType(m_objectPart[0].object, type);

    return true;
}

// Calculates the matrix for transforming the object.
// Returns true if the matrix has changed.
// The rotations occur in the order Y, Z and X.

bool COldObject::UpdateTransformObject(int part, bool bForceUpdate)
{
    Math::Vector    position, angle, eye;
    bool        bModif = false;
    int         parent;

    if ( m_transporter != 0 )  // transported by transporter?
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
            m_objectPart[part].matTranslate.LoadIdentity();
            m_objectPart[part].matTranslate.Set(1, 4, position.x);
            m_objectPart[part].matTranslate.Set(2, 4, position.y);
            m_objectPart[part].matTranslate.Set(3, 4, position.z);
        }

        if ( m_objectPart[part].bRotate )
        {
            Math::LoadRotationZXYMatrix(m_objectPart[part].matRotate, angle);
        }

        if ( m_objectPart[part].bZoom )
        {
            Math::Matrix    mz;
            mz.LoadIdentity();
            mz.Set(1, 1, m_objectPart[part].zoom.x);
            mz.Set(2, 2, m_objectPart[part].zoom.y);
            mz.Set(3, 3, m_objectPart[part].zoom.z);
            m_objectPart[part].matTransform = Math::MultiplyMatrices(m_objectPart[part].matTranslate,
                                                Math::MultiplyMatrices(m_objectPart[part].matRotate, mz));
        }
        else
        {
            m_objectPart[part].matTransform = Math::MultiplyMatrices(m_objectPart[part].matTranslate,
                                                                     m_objectPart[part].matRotate);
        }
        bModif = true;
    }

    if ( bForceUpdate                  ||
         m_objectPart[part].bTranslate ||
         m_objectPart[part].bRotate    )
    {
        parent = m_objectPart[part].parentPart;

        if ( part == 0 && m_transporter != 0 )  // transported by a transporter?
        {
            Math::Matrix*   matWorldTransporter;
            matWorldTransporter = m_transporter->GetWorldMatrix(m_transporterLink);
            m_objectPart[part].matWorld = Math::MultiplyMatrices(*matWorldTransporter,
                                                                 m_objectPart[part].matTransform);
        }
        else
        {
            if ( parent == -1 )  // no parent?
            {
                m_objectPart[part].matWorld = m_objectPart[part].matTransform;
            }
            else
            {
                m_objectPart[part].matWorld = Math::MultiplyMatrices(m_objectPart[parent].matWorld,
                                                                     m_objectPart[part].matTransform);
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
        m_objectPart[i].position.x = m_objectPart[i].matWorld.Get(1, 4);
        m_objectPart[i].position.y = m_objectPart[i].matWorld.Get(2, 4);
        m_objectPart[i].position.z = m_objectPart[i].matWorld.Get(3, 4);

        m_objectPart[i].matWorld.Set(1, 4, 0.0f);
        m_objectPart[i].matWorld.Set(2, 4, 0.0f);
        m_objectPart[i].matWorld.Set(3, 4, 0.0f);

        m_objectPart[i].matTranslate.Set(1, 4, 0.0f);
        m_objectPart[i].matTranslate.Set(2, 4, 0.0f);
        m_objectPart[i].matTranslate.Set(3, 4, 0.0f);

        m_objectPart[i].parentPart = -1;  // more parents
    }

    m_bFlat = true;
}



// Updates the mapping of the texture of the pile.

void COldObject::UpdateEnergyMapping()
{
    if (Math::IsEqual(m_energy, m_lastEnergy, 0.01f))
        return;

    m_lastEnergy = m_energy;

    Gfx::Material mat;
    mat.diffuse = Gfx::Color(1.0f, 1.0f, 1.0f);  // white
    mat.ambient = Gfx::Color(0.5f, 0.5f, 0.5f);

    float a = 0.0f, b = 0.0f;

    if ( m_type == OBJECT_POWER  ||
         m_type == OBJECT_ATOMIC )
    {
        a = 2.0f;
        b = 0.0f;  // dimensions of the battery (according to y)
    }
    else if ( m_type == OBJECT_STATION )
    {
        a = 10.0f;
        b =  4.0f;  // dimensions of the battery (according to y)
    }
    else if ( m_type == OBJECT_ENERGY )
    {
        a = 9.0f;
        b = 3.0f;  // dimensions of the battery (according to y)
    }

    float i = 0.50f+0.25f*m_energy;  // origin
    float s = i+0.25f;  // width

    float au = (s-i)/(b-a);
    float bu = s-b*(s-i)/(b-a);

    m_engine->ChangeTextureMapping(m_objectPart[0].object,
                                   mat, Gfx::ENG_RSTATE_PART3, "objects/lemt.png", "",
                                   Gfx::ENG_TEX_MAPPING_1Y,
                                   au, bu, 1.0f, 0.0f);
}


// Manual action.

bool COldObject::EventProcess(const Event &event)
{
    if ( event.type == EVENT_KEY_DOWN )
    {
#if ADJUST_ONBOARD
        if ( m_bSelect )
        {
            if ( event.param == 'E' )  debug_x += 0.1f;
            if ( event.param == 'D' )  debug_x -= 0.1f;
            if ( event.param == 'R' )  debug_y += 0.1f;
            if ( event.param == 'F' )  debug_y -= 0.1f;
            if ( event.param == 'T' )  debug_z += 0.1f;
            if ( event.param == 'G' )  debug_z -= 0.1f;
        }
#endif
#if ADJUST_ARM
        if ( m_bSelect )
        {
            if ( event.param == 'X' )  debug_arm1 += 5.0f*Math::PI/180.0f;
            if ( event.param == 'C' )  debug_arm1 -= 5.0f*Math::PI/180.0f;
            if ( event.param == 'V' )  debug_arm2 += 5.0f*Math::PI/180.0f;
            if ( event.param == 'B' )  debug_arm2 -= 5.0f*Math::PI/180.0f;
            if ( event.param == 'N' )  debug_arm3 += 5.0f*Math::PI/180.0f;
            if ( event.param == 'M' )  debug_arm3 -= 5.0f*Math::PI/180.0f;
            if ( event.param == 'X' ||
                 event.param == 'C' ||
                 event.param == 'V' ||
                 event.param == 'B' ||
                 event.param == 'N' ||
                 event.param == 'M' )
            {
                SetPartRotationZ(1, debug_arm1);
                SetPartRotationZ(2, debug_arm2);
                SetPartRotationZ(3, debug_arm3);
                char s[100];
                sprintf(s, "a=%.2f b=%.2f c=%.2f", debug_arm1*180.0f/Math::PI, debug_arm2*180.0f/Math::PI, debug_arm3*180.0f/Math::PI);
                m_engine->SetInfoText(5, s);
            }
        }
#endif
    }

    if ( m_physics != nullptr )
    {
        if ( !m_physics->EventProcess(event) )  // object destroyed?
        {
            if ( GetSelect()             &&
                 m_type != OBJECT_ANT    &&
                 m_type != OBJECT_SPIDER &&
                 m_type != OBJECT_BEE    )
            {
                if ( !m_bDead )  m_camera->SetType(Gfx::CAM_TYPE_EXPLO);
                m_main->DeselectAll();
            }
            return false;
        }
    }

    if ( m_auto != nullptr )
    {
        m_auto->EventProcess(event);

        if ( event.type == EVENT_FRAME &&
             m_auto->IsEnded() != ERR_CONTINUE )
        {
            m_auto->DeleteObject();
            m_auto.reset();
        }
    }

    if ( m_motion != nullptr )
    {
        m_motion->EventProcess(event);
    }

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

    if ( m_type != OBJECT_SHOW && m_engine->GetPause() )  return true;

    m_aTime += event.rTime;
    m_shotTime += event.rTime;

    VirusFrame(event.rTime);
    PartiFrame(event.rTime);

    UpdateMapping();
    UpdateTransformObject();
    UpdateSelectParticle();

    if ( m_bProxyActivate )  // active if it is near?
    {
        Math::Vector eye = m_engine->GetLookatPt();
        float dist = Math::Distance(eye, GetPosition());
        if ( dist < m_proxyDistance )
        {
            m_bProxyActivate = false;
            m_main->CreateShortcuts();
            m_sound->Play(SOUND_FINDING);
            m_engine->GetPyroManager()->Create(Gfx::PT_FINDING, this, 0.0f);
            m_main->DisplayError(INFO_FINDING, this);
        }
    }

    return true;
}

// Updates the mapping of the object.

void COldObject::UpdateMapping()
{
    if ( m_type == OBJECT_POWER   ||
         m_type == OBJECT_ATOMIC  ||
         m_type == OBJECT_STATION ||
         m_type == OBJECT_ENERGY  )
    {
        UpdateEnergyMapping();
    }
}


// Management of viruses.

void COldObject::VirusFrame(float rTime)
{
    Gfx::ParticleType   type;
    Math::Vector        pos, speed;
    Math::Point         dim;
    int                 r;

    if ( !m_bVirusMode )  return;  // healthy object?

    m_virusTime += rTime;
    if ( m_virusTime >= VIRUS_DELAY )
    {
        m_bVirusMode = false;  // the virus is no longer active
    }

    if ( m_lastVirusParticle+m_engine->ParticleAdapt(0.2f) <= m_aTime )
    {
        m_lastVirusParticle = m_aTime;

        r = rand()%10;
        if ( r == 0 )  type = Gfx::PARTIVIRUS1;
        if ( r == 1 )  type = Gfx::PARTIVIRUS2;
        if ( r == 2 )  type = Gfx::PARTIVIRUS3;
        if ( r == 3 )  type = Gfx::PARTIVIRUS4;
        if ( r == 4 )  type = Gfx::PARTIVIRUS5;
        if ( r == 5 )  type = Gfx::PARTIVIRUS6;
        if ( r == 6 )  type = Gfx::PARTIVIRUS7;
        if ( r == 7 )  type = Gfx::PARTIVIRUS8;
        if ( r == 8 )  type = Gfx::PARTIVIRUS9;
        if ( r == 9 )  type = Gfx::PARTIVIRUS10;

        pos = GetPosition();
        pos.x += (Math::Rand()-0.5f)*10.0f;
        pos.z += (Math::Rand()-0.5f)*10.0f;
        speed.x = (Math::Rand()-0.5f)*2.0f;
        speed.z = (Math::Rand()-0.5f)*2.0f;
        speed.y = Math::Rand()*4.0f+4.0f;
        dim.x = Math::Rand()*0.3f+0.3f;
        dim.y = dim.x;

        m_particle->CreateParticle(pos, speed, dim, type, 3.0f);
    }
}

// Management particles mistresses.

void COldObject::PartiFrame(float rTime)
{
    Math::Vector    pos, angle, factor;
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
            case 0:  factor = Math::Vector( 0.5f, 0.3f, 0.6f); break;
            case 1:  factor = Math::Vector(-0.3f, 0.4f,-0.2f); break;
            case 2:  factor = Math::Vector( 0.4f,-0.6f,-0.3f); break;
            case 3:  factor = Math::Vector(-0.6f,-0.2f, 0.0f); break;
            case 4:  factor = Math::Vector( 0.4f, 0.1f,-0.7f); break;
        }

        angle = GetPartRotation(i);
        angle += rTime*Math::PI*factor;
        SetPartRotation(i, angle);
    }
}


// Changes the perspective to view if it was like in the vehicle,
// or behind the vehicle.

void COldObject::SetViewFromHere(Math::Vector &eye, float &dirH, float &dirV,
                              Math::Vector  &lookat, Math::Vector &upVec,
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
              m_type == OBJECT_MOBILErs )
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
    else if ( m_type == OBJECT_MOBILEsa )
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
#if ADJUST_ONBOARD
    eye.x += debug_x;
    eye.y += debug_y;
    eye.z += debug_z;
    char s[100];
    sprintf(s, "x=%.2f y=%.2f z=%.2f", eye.x, eye.y, eye.z);
    m_engine->SetInfoText(4, s);
#endif

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
    upVec = Math::Vector(0.0f, 1.0f, 0.0f);
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


// Management of energy contained in a battery.
// Single subject possesses the battery energy, but not the vehicle that carries the battery!

void COldObject::SetEnergy(float level)
{
    if ( level < 0.0f )  level = 0.0f;
    if ( level > 1.0f )  level = 1.0f;
    m_energy = level;
}

float COldObject::GetEnergy()
{
    if ( m_type != OBJECT_POWER   &&
         m_type != OBJECT_ATOMIC  &&
         m_type != OBJECT_STATION &&
         m_type != OBJECT_ENERGY  )  return 0.0f;
    return m_energy;
}


// Management of the capacity of a battery.
// Single subject possesses a battery capacity,
// but not the vehicle that carries the battery!

void COldObject::SetCapacity(float capacity)
{
    m_capacity = capacity;
}

float COldObject::GetCapacity()
{
    return m_capacity;
}


// Management of the shield.

void COldObject::SetShield(float level)
{
    m_shield = level;
}

float COldObject::GetShield()
{
    if ( m_type == OBJECT_FRET     ||
         m_type == OBJECT_STONE    ||
         m_type == OBJECT_URANIUM  ||
         m_type == OBJECT_BULLET   ||
         m_type == OBJECT_METAL    ||
         m_type == OBJECT_BBOX     ||
         m_type == OBJECT_KEYa     ||
         m_type == OBJECT_KEYb     ||
         m_type == OBJECT_KEYc     ||
         m_type == OBJECT_KEYd     ||
         m_type == OBJECT_TNT      ||
         m_type == OBJECT_SCRAP1   ||
         m_type == OBJECT_SCRAP2   ||
         m_type == OBJECT_SCRAP3   ||
         m_type == OBJECT_SCRAP4   ||
         m_type == OBJECT_SCRAP5   ||
         m_type == OBJECT_BOMB     ||
         m_type == OBJECT_WAYPOINT ||
         m_type == OBJECT_FLAGb    ||
         m_type == OBJECT_FLAGr    ||
         m_type == OBJECT_FLAGg    ||
         m_type == OBJECT_FLAGy    ||
         m_type == OBJECT_FLAGv    ||
         m_type == OBJECT_POWER    ||
         m_type == OBJECT_ATOMIC   ||
         m_type == OBJECT_ANT      ||
         m_type == OBJECT_SPIDER   ||
         m_type == OBJECT_BEE      ||
         m_type == OBJECT_WORM     )  return 0.0f;
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


// Management of transparency of the object.

void COldObject::SetTransparency(float value)
{
    int     i;

    m_transparency = value;

    for ( i=0 ; i<m_totalPart ; i++ )
    {
        if ( m_objectPart[i].bUsed )
        {
            if ( m_type == OBJECT_BASE )
            {
                if ( i != 9 )  continue;  // no central pillar?
            }

            m_engine->SetObjectTransparency(m_objectPart[i].object, value);
        }
    }
}

// Indicates whether an object is stationary (ant on the back).

void COldObject::SetFixed(bool bFixed)
{
    m_bFixed = bFixed;
}

bool COldObject::GetFixed()
{
    return m_bFixed;
}


// Indicates whether an object is subjected to clipping (obstacles).

void COldObject::SetClip(bool bClip)
{
    m_bClip = bClip;
}

bool COldObject::GetClip()
{
    return m_bClip;
}


// Controls object team

void COldObject::SetTeam(int team)
{
    m_team = team;
}

int COldObject::GetTeam()
{
    return m_team;
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

        std::unique_ptr<CAutoJostle> autoJostle{new CAutoJostle(this)};
        autoJostle->Start(0, force);
        m_auto = std::move(autoJostle);
    }

    return true;
}


// Beginning of the effect when the instruction "detect" is used.

void COldObject::StartDetectEffect(CObject *target, bool bFound)
{
    Math::Matrix*   mat;
    Math::Vector    pos, goal;
    Math::Point     dim;

    mat = GetWorldMatrix(0);
    pos = Math::Transform(*mat, Math::Vector(2.0f, 3.0f, 0.0f));

    if ( target == 0 )
    {
        goal = Math::Transform(*mat, Math::Vector(50.0f, 3.0f, 0.0f));
    }
    else
    {
        goal = target->GetPosition();
        goal.y += 3.0f;
        goal = Math::SegmentPoint(pos, goal, Math::Distance(pos, goal)-3.0f);
    }

    dim.x = 3.0f;
    dim.y = dim.x;
    m_particle->CreateRay(pos, goal, Gfx::PARTIRAY2, dim, 0.2f);

    if ( target != 0 )
    {
        goal = target->GetPosition();
        goal.y += 3.0f;
        goal = Math::SegmentPoint(pos, goal, Math::Distance(pos, goal)-1.0f);
        dim.x = 6.0f;
        dim.y = dim.x;
        m_particle->CreateParticle(goal, Math::Vector(0.0f, 0.0f, 0.0f), dim,
                                     bFound?Gfx::PARTIGLINT:Gfx::PARTIGLINTr, 0.5f);
    }

    m_sound->Play(bFound?SOUND_BUILD:SOUND_RECOVER);
}


// Management of time from which a virus is active.

void COldObject::SetVirusMode(bool bEnable)
{
    m_bVirusMode = bEnable;
    m_virusTime = 0.0f;

    if ( m_bVirusMode && m_brain != nullptr )
    {
        if ( !m_brain->IntroduceVirus() )  // tries to infect
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

void COldObject::SetCameraDist(float dist)
{
    m_cameraDist = dist;
}

float COldObject::GetCameraDist()
{
    return m_cameraDist;
}

void COldObject::SetCameraLock(bool bLock)
{
    m_bCameraLock = bLock;
}

bool COldObject::GetCameraLock()
{
    return m_bCameraLock;
}



// Management of the demonstration of the object.

void COldObject::SetHighlight(bool mode)
{
    if (mode)
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

void COldObject::SetSelect(bool bMode, bool bDisplayError)
{
    Error       err;

    m_bSelect = bMode;

    if ( m_physics != nullptr )
    {
        m_physics->CreateInterface(m_bSelect);
    }

    if ( m_auto != nullptr )
    {
        m_auto->CreateInterface(m_bSelect);
    }

    CreateSelectParticle();  // creates / removes particles

    if ( !m_bSelect )
    {
        //SetGunGoalH(0.0f);  // puts the cannon right
        return;  // selects if not finished
    }

    err = ERR_OK;
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

bool COldObject::GetSelect(bool bReal)
{
    if ( !bReal && m_main->GetFixScene() )  return false;
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


// Management of the activities of an object.

void COldObject::SetActivity(bool bMode)
{
    if ( m_brain != nullptr )
    {
        m_brain->SetActivity(bMode);
    }
}

bool COldObject::GetActivity()
{
    if ( m_brain != nullptr )
    {
        return m_brain->GetActivity();
    }
    return false;
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


// Management of the visibility of an object.
// The object is not hidden or visually disabled, but ignores detections!
// For example: underground worm.

void COldObject::SetVisible(bool bVisible)
{
    m_bVisible = bVisible;
}


// Management mode of operation of an object.
// An inactive object is an object destroyed, nonexistent.
// This mode is used for objects "resetables"
// during training to simulate destruction.

void COldObject::SetEnable(bool bEnable)
{
    m_bEnable = bEnable;
}

bool COldObject::GetEnable()
{
    return m_bEnable;
}


// Management mode or an object is only active when you're close.

void COldObject::SetProxyActivate(bool bActivate)
{
    m_bProxyActivate = bActivate;
}

bool COldObject::GetProxyActivate()
{
    return m_bProxyActivate;
}

void COldObject::SetProxyDistance(float distance)
{
    m_proxyDistance = distance;
}

float COldObject::GetProxyDistance()
{
    return m_proxyDistance;
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


// Management of free parameter.

void COldObject::SetParam(float value)
{
    m_param = value;
}

float COldObject::GetParam()
{
    return m_param;
}
// Management of the mode "blocked" of an object.
// For example, a cube of titanium is blocked while it is used to make something,
// or a vehicle is blocked as its construction is not finished.

void COldObject::SetLock(bool bLock)
{
    m_bLock = bLock;
}

bool COldObject::GetLock()
{
    return m_bLock;
}

// Ignore checks in build() function

void COldObject::SetIgnoreBuildCheck(bool bIgnoreBuildCheck)
{
    m_bIgnoreBuildCheck = bIgnoreBuildCheck;
}

bool COldObject::GetIgnoreBuildCheck()
{
    return m_bIgnoreBuildCheck;
}

// Management of the mode "current explosion" of an object.
// An object in this mode is not saving.

void COldObject::SetExploding(bool bExplo)
{
    m_bExplo = bExplo;
}

bool COldObject::IsExploding()
{
    return m_bExplo;
}

// Management of the HS mode of an object.

void COldObject::SetBurn(bool bBurn)
{
    m_bBurn = bBurn;

//? if ( m_botVar != 0 )
//? {
//?     if ( m_bBurn )  m_botVar->SetUserPtr(OBJECTDELETED);
//?     else            m_botVar->SetUserPtr(this);
//? }
}

bool COldObject::GetBurn()
{
    return m_bBurn;
}

void COldObject::SetDead(bool bDead)
{
    m_bDead = bDead;

    if ( bDead && m_brain != nullptr )
    {
        m_brain->StopProgram();  // stops the current task
    }

//? if ( m_botVar != 0 )
//? {
//?     if ( m_bDead )  m_botVar->SetUserPtr(OBJECTDELETED);
//?     else            m_botVar->SetUserPtr(this);
//? }
}

bool COldObject::GetDead()
{
    return m_bDead;
}

bool COldObject::GetRuin()
{
    return m_bBurn|m_bFlat;
}

bool COldObject::GetActive()
{
    return !m_bLock && !m_bBurn && !m_bFlat && m_bVisible && m_bEnable;
}


// Management of the point of aim.

void COldObject::SetGunGoalV(float gunGoal)
{
    if ( m_type == OBJECT_MOBILEfc ||
         m_type == OBJECT_MOBILEtc ||
         m_type == OBJECT_MOBILEwc ||
         m_type == OBJECT_MOBILEic )  // fireball?
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
         m_type == OBJECT_MOBILEic )  // fireball?
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



// Shows the limits of the object.

bool COldObject::StartShowLimit()
{
    if ( m_showLimitRadius == 0.0f )  return false;

    m_main->SetShowLimit(0, Gfx::PARTILIMIT1, this, GetPosition(), m_showLimitRadius);
    m_bShowLimit = true;
    return true;
}

void COldObject::StopShowLimit()
{
    m_bShowLimit = false;
}

void COldObject::SetShowLimitRadius(float radius)
{
    m_showLimitRadius = radius;
}

// Indicates whether a program is under execution.

bool COldObject::IsProgram()
{
    if ( m_brain == nullptr )  return false;
    return m_brain->IsProgram();
}


// Creates or removes particles associated to the object.

void COldObject::CreateSelectParticle()
{
    Math::Vector    pos, speed;
    Math::Point     dim;
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
             m_type == OBJECT_MOBILEdr )  // vehicle?
        {
            pos = Math::Vector(0.0f, 0.0f, 0.0f);
            speed = Math::Vector(0.0f, 0.0f, 0.0f);
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
    Math::Vector    pos[4];
    Math::Point     dim[4];
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
         m_type == OBJECT_MOBILErs )  // large caterpillars?
    {
        pos[0] = Math::Vector(4.2f, 2.8f,  1.5f);
        pos[1] = Math::Vector(4.2f, 2.8f, -1.5f);
        dim[0].x = 1.5f;
        dim[1].x = 1.5f;
    }
    else if ( m_type == OBJECT_MOBILEwt ||
              m_type == OBJECT_MOBILEtt ||
              m_type == OBJECT_MOBILEft ||
              m_type == OBJECT_MOBILEit )  // trainer ?
    {
        pos[0] = Math::Vector(4.2f, 2.5f,  1.2f);
        pos[1] = Math::Vector(4.2f, 2.5f, -1.2f);
        dim[0].x = 1.5f;
        dim[1].x = 1.5f;
    }
    else if ( m_type == OBJECT_MOBILEsa )  // submarine?
    {
        pos[0] = Math::Vector(3.6f, 4.0f,  2.0f);
        pos[1] = Math::Vector(3.6f, 4.0f, -2.0f);
    }
    else if ( m_type == OBJECT_MOBILEtg )  // target?
    {
        pos[0] = Math::Vector(3.4f, 6.5f,  2.0f);
        pos[1] = Math::Vector(3.4f, 6.5f, -2.0f);
    }
    else if ( m_type == OBJECT_MOBILEdr )  // designer?
    {
        pos[0] = Math::Vector(4.9f, 3.5f,  2.5f);
        pos[1] = Math::Vector(4.9f, 3.5f, -2.5f);
    }
    else
    {
        pos[0] = Math::Vector(4.2f, 2.5f,  1.5f);
        pos[1] = Math::Vector(4.2f, 2.5f, -1.5f);
    }

    // Red back lens
    if ( m_type == OBJECT_MOBILEfa ||
         m_type == OBJECT_MOBILEfc ||
         m_type == OBJECT_MOBILEfi ||
         m_type == OBJECT_MOBILEfs ||
         m_type == OBJECT_MOBILEft )  // flying?
    {
        pos[2] = Math::Vector(-4.0f, 3.1f,  4.5f);
        pos[3] = Math::Vector(-4.0f, 3.1f, -4.5f);
        dim[2].x = 0.6f;
        dim[3].x = 0.6f;
    }
    if ( m_type == OBJECT_MOBILEwa ||
         m_type == OBJECT_MOBILEwc ||
         m_type == OBJECT_MOBILEwi ||
         m_type == OBJECT_MOBILEws )  // wheels?
    {
        pos[2] = Math::Vector(-4.5f, 2.7f,  2.8f);
        pos[3] = Math::Vector(-4.5f, 2.7f, -2.8f);
    }
    if ( m_type == OBJECT_MOBILEwt )  // wheels?
    {
        pos[2] = Math::Vector(-4.0f, 2.5f,  2.2f);
        pos[3] = Math::Vector(-4.0f, 2.5f, -2.2f);
    }
    if ( m_type == OBJECT_MOBILEia ||
         m_type == OBJECT_MOBILEic ||
         m_type == OBJECT_MOBILEii ||
         m_type == OBJECT_MOBILEis ||
         m_type == OBJECT_MOBILEit )  // legs?
    {
        pos[2] = Math::Vector(-4.5f, 2.7f,  2.8f);
        pos[3] = Math::Vector(-4.5f, 2.7f, -2.8f);
    }
    if ( m_type == OBJECT_MOBILEta ||
         m_type == OBJECT_MOBILEtc ||
         m_type == OBJECT_MOBILEti ||
         m_type == OBJECT_MOBILEts ||
         m_type == OBJECT_MOBILEtt )  // caterpillars?
    {
        pos[2] = Math::Vector(-3.6f, 4.2f,  3.0f);
        pos[3] = Math::Vector(-3.6f, 4.2f, -3.0f);
    }
    if ( m_type == OBJECT_MOBILErt ||
         m_type == OBJECT_MOBILErc ||
         m_type == OBJECT_MOBILErr ||
         m_type == OBJECT_MOBILErs )  // large caterpillars?
    {
        pos[2] = Math::Vector(-5.0f, 5.2f,  2.5f);
        pos[3] = Math::Vector(-5.0f, 5.2f, -2.5f);
    }
    if ( m_type == OBJECT_MOBILEsa )  // submarine?
    {
        pos[2] = Math::Vector(-3.6f, 4.0f,  2.0f);
        pos[3] = Math::Vector(-3.6f, 4.0f, -2.0f);
    }
    if ( m_type == OBJECT_MOBILEtg )  // target?
    {
        pos[2] = Math::Vector(-2.4f, 6.5f,  2.0f);
        pos[3] = Math::Vector(-2.4f, 6.5f, -2.0f);
    }
    if ( m_type == OBJECT_MOBILEdr )  // designer?
    {
        pos[2] = Math::Vector(-5.3f, 2.7f,  1.8f);
        pos[3] = Math::Vector(-5.3f, 2.7f, -1.8f);
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
        pos[i] = Math::Transform(m_objectPart[0].matWorld, pos[i]);
        dim[i].y = dim[i].x;
        m_particle->SetParam(m_partiSel[i], pos[i], dim[i], zoom[i], angle, 1.0f);
    }
}


// Getes the pointer to the current script execution.

void COldObject::SetRunScript(CScript* script)
{
    m_runScript = script;
}

CScript* COldObject::GetRunScript()
{
    return m_runScript;
}

// Returns the variables of "this" for CBOT.

CBotVar* COldObject::GetBotVar()
{
    return m_botVar;
}

// Returns the physics associated to the object.

CPhysics* COldObject::GetPhysics()
{
    return m_physics.get();
}

void COldObject::SetPhysics(std::unique_ptr<CPhysics> physics)
{
    m_physics = std::move(physics);
}

// Returns the brain associated to the object.

CBrain* COldObject::GetBrain()
{
    return m_brain.get();
}

void COldObject::SetBrain(std::unique_ptr<CBrain> brain)
{
    m_implementedInterfaces[static_cast<int>(ObjectInterfaceType::Programmable)] = true;
    m_brain = std::move(brain);
}

// Returns the movement associated to the object.

CMotion* COldObject::GetMotion()
{
    return m_motion.get();
}

void COldObject::SetMotion(std::unique_ptr<CMotion> motion)
{
    m_motion = std::move(motion);
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

// Management of the position in the file definition.

void COldObject::SetDefRank(int rank)
{
    m_defRank = rank;
}

int  COldObject::GetDefRank()
{
    return m_defRank;
}

// Getes the object name for the tooltip.

bool COldObject::GetTooltipName(std::string& name)
{
    GetResource(RES_OBJECT, m_type, name);
    if(GetTeam() != 0) {
        name += " ["+CRobotMain::GetInstancePointer()->GetTeamName(GetTeam())+" ("+boost::lexical_cast<std::string>(GetTeam())+")]";
    }
    return !name.empty();
}

Math::Vector COldObject::GetPosition() const
{
    return GetPartPosition(0);
}

void COldObject::SetPosition(const Math::Vector& pos)
{
    SetPartPosition(0, pos);
}

Math::Vector COldObject::GetRotation() const
{
    return GetPartRotation(0);
}

void COldObject::SetRotation(const Math::Vector& rotation)
{
    SetPartRotation(0, rotation);
}

Math::Vector COldObject::GetScale() const
{
    return GetPartScale(0);
}

void COldObject::SetScale(const Math::Vector& scale)
{
    SetPartScale(0, scale);
}

