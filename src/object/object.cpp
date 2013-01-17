// * This file is part of the COLOBOT source code
// * Copyright (C) 2001-2008, Daniel ROUX & EPSITEC SA, www.epsitec.ch
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


#include "object/object.h"

#include "CBot/CBotDll.h"

#include "app/app.h"

#include "common/global.h"
#include "common/iman.h"
#include "common/restext.h"

#include "graphics/engine/lightman.h"
#include "graphics/engine/lightning.h"
#include "graphics/engine/modelmanager.h"
#include "graphics/engine/particle.h"
#include "graphics/engine/pyro.h"
#include "graphics/engine/terrain.h"

#include "math/geometry.h"

#include "object/auto/auto.h"
#include "object/auto/autobase.h"
#include "object/auto/autoconvert.h"
#include "object/auto/autoderrick.h"
#include "object/auto/autodestroyer.h"
#include "object/auto/autoegg.h"
#include "object/auto/autoenergy.h"
#include "object/auto/autofactory.h"
#include "object/auto/autoflag.h"
#include "object/auto/autohuston.h"
#include "object/auto/autoinfo.h"
#include "object/auto/autojostle.h"
#include "object/auto/autokid.h"
#include "object/auto/autolabo.h"
#include "object/auto/automush.h"
#include "object/auto/autonest.h"
#include "object/auto/autonuclear.h"
#include "object/auto/autopara.h"
#include "object/auto/autoportico.h"
#include "object/auto/autoradar.h"
#include "object/auto/autorepair.h"
#include "object/auto/autoresearch.h"
#include "object/auto/autoroot.h"
#include "object/auto/autosafe.h"
#include "object/auto/autostation.h"
#include "object/auto/autotower.h"
#include "object/brain.h"
#include "object/motion/motion.h"
#include "object/motion/motionant.h"
#include "object/motion/motionbee.h"
#include "object/motion/motionhuman.h"
#include "object/motion/motionmother.h"
#include "object/motion/motionspider.h"
#include "object/motion/motiontoto.h"
#include "object/motion/motionvehicle.h"
#include "object/motion/motionworm.h"
#include "object/robotmain.h"

#include "physics/physics.h"

#include "script/cbottoken.h"
#include "script/cmdtoken.h"

#include "ui/displaytext.h"



#define ADJUST_ONBOARD  false       // true -> adjusts the camera ONBOARD
#define ADJUST_ARM  false           // true -> adjusts the manipulator arm
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
    CObject*    object = static_cast<CObject*>(user);
    CObject*    power;
    CObject*    fret;
    CPhysics*   physics;
    CBotVar     *pVar, *pSub;
    ObjectType  type;
    Math::Vector    pos;
    float       value;
    int         iValue;

    if ( object == 0 )  return;

    physics = object->GetPhysics();

    // Updates the object's type.
    pVar = botThis->GetItemList();  // "category"
    type = object->GetType();
    pVar->SetValInt(type, object->GetName());

    // Updates the position of the object.
    pVar = pVar->GetNext();  // "position"
    if ( object->GetTruck() == 0 )
    {
        pos = object->GetPosition(0);
        pos.y -= object->GetWaterLevel();  // relative to sea level!
        pSub = pVar->GetItemList();  // "x"
        pSub->SetValFloat(pos.x/g_unit);
        pSub = pSub->GetNext();  // "y"
        pSub->SetValFloat(pos.z/g_unit);
        pSub = pSub->GetNext();  // "z"
        pSub->SetValFloat(pos.y/g_unit);
    }
    else    // object transported?
    {
        pSub = pVar->GetItemList();  // "x"
        pSub->SetInit(IS_NAN);
        pSub = pSub->GetNext();  // "y"
        pSub->SetInit(IS_NAN);
        pSub = pSub->GetNext();  // "z"
        pSub->SetInit(IS_NAN);
    }

    // Updates the angle.
    pos = object->GetAngle(0);
    pos += object->GetInclinaison();
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

    // Updates the material of the object.
    pVar = pVar->GetNext();  // "material"
    iValue = object->GetMaterial();
    pVar->SetValInt(iValue);

    // Updates the type of battery.
    pVar = pVar->GetNext();  // "energyCell"
    power = object->GetPower();
    if ( power == 0 )  pVar->SetPointer(0);
    else               pVar->SetPointer(power->GetBotVar());

    // Updates the transported object's type.
    pVar = pVar->GetNext();  // "load"
    fret = object->GetFret();
    if ( fret == 0 )  pVar->SetPointer(0);
    else              pVar->SetPointer(fret->GetBotVar());
}




// Object's constructor.

CObject::CObject(CInstanceManager* iMan)
{
    int     i;

    m_iMan = iMan;
    m_iMan->AddInstance(CLASS_OBJECT, this, 500);

    m_app         = CApplication::GetInstancePointer();
    m_engine      = static_cast<Gfx::CEngine*>(m_iMan->SearchInstance(CLASS_ENGINE));
    m_lightMan    = static_cast<Gfx::CLightManager*>(m_iMan->SearchInstance(CLASS_LIGHT));
    m_terrain     = static_cast<Gfx::CTerrain*>(m_iMan->SearchInstance(CLASS_TERRAIN));
    m_water       = static_cast<Gfx::CWater*>(m_iMan->SearchInstance(CLASS_WATER));
    m_particle    = static_cast<Gfx::CParticle*>(m_iMan->SearchInstance(CLASS_PARTICULE));
    m_camera      = static_cast<Gfx::CCamera*>(m_iMan->SearchInstance(CLASS_CAMERA));
    m_displayText = static_cast<Ui::CDisplayText*>(m_iMan->SearchInstance(CLASS_DISPLAYTEXT));
    m_main        = static_cast<CRobotMain*>(m_iMan->SearchInstance(CLASS_MAIN));
    m_sound       = static_cast<CSoundInterface*>(m_iMan->SearchInstance(CLASS_SOUND));
    m_physics     = 0;
    m_brain       = 0;
    m_motion      = 0;
    m_auto        = 0;
    m_runScript   = 0;

    m_type = OBJECT_FIX;
    m_id = ++g_id;
    m_option = 0;
    m_name[0] = 0;
    m_partiReactor  = -1;
    m_shadowLight   = -1;
    m_effectLight   = -1;
    m_linVibration  = Math::Vector(0.0f, 0.0f, 0.0f);
    m_cirVibration  = Math::Vector(0.0f, 0.0f, 0.0f);
    m_inclinaison   = Math::Vector(0.0f, 0.0f, 0.0f);
    m_lastParticle = 0.0f;

    m_power = 0;
    m_fret  = 0;
    m_truck = 0;
    m_truckLink = 0;
    m_energy   = 1.0f;
    m_capacity = 1.0f;
    m_shield   = 1.0f;
    m_range    = 0.0f;
    m_transparency = 0.0f;
    m_lastEnergy = 999.9f;
    m_bHilite = false;
    m_bSelect = false;
    m_bSelectable = true;
    m_bCheckToken = true;
    m_bVisible = true;
    m_bEnable = true;
    m_bGadget = false;
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
    m_totalDesectList = 0;
    m_bLock  = false;
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

    memset(&m_character, 0, sizeof(m_character));
    m_character.wheelFront = 1.0f;
    m_character.wheelBack  = 1.0f;
    m_character.wheelLeft  = 1.0f;
    m_character.wheelRight = 1.0f;

    m_resetCap      = RESET_NONE;
    m_bResetBusy    = false;
    m_resetPosition = Math::Vector(0.0f, 0.0f, 0.0f);
    m_resetAngle    = Math::Vector(0.0f, 0.0f, 0.0f);
    m_resetRun      = -1;

    m_cameraType = Gfx::CAM_TYPE_BACK;
    m_cameraDist = 50.0f;
    m_bCameraLock = false;

    m_infoTotal = 0;
    m_infoReturn = NAN;
    m_bInfoUpdate = false;

    for ( i=0 ; i<OBJECTMAXPART ; i++ )
    {
        m_objectPart[i].bUsed = false;
    }
    m_totalPart = 0;

    for ( i=0 ; i<4 ; i++ )
    {
        m_partiSel[i] = -1;
    }

    for ( i=0 ; i<OBJECTMAXCMDLINE ; i++ )
    {
        m_cmdLine[i] = NAN;
    }

    FlushCrashShere();
    m_globalSpherePos = Math::Vector(0.0f, 0.0f, 0.0f);
    m_globalSphereRadius = 0.0f;
    m_jotlerSpherePos = Math::Vector(0.0f, 0.0f, 0.0f);
    m_jotlerSphereRadius = 0.0f;

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

CObject::~CObject()
{
    if ( m_botVar != 0 )
    {
        m_botVar->SetUserPtr(OBJECTDELETED);
        delete m_botVar;
        m_botVar = nullptr;
    }

    delete m_physics;
    m_physics = nullptr;
    delete m_brain;
    m_brain = nullptr;
    delete m_motion;
    m_motion = nullptr;
    delete m_auto;
    m_auto = nullptr;

    m_iMan->DeleteInstance(CLASS_OBJECT, this);

    m_app = nullptr;
}


// Removes an object.
// If bAll = true, it does not help,
// because all objects in the scene are quickly destroyed!

void CObject::DeleteObject(bool bAll)
{
    CObject*    pObj;
    Gfx::CPyro* pPyro;
    int         i;

    if ( m_botVar != 0 )
    {
        m_botVar->SetUserPtr(OBJECTDELETED);
    }

    if ( m_camera->GetControllingObject() == this )
    {
        m_camera->SetControllingObject(0);
    }

    for ( i=0 ; i<1000000 ; i++ )
    {
        pObj = static_cast<CObject*>(m_iMan->SearchInstance(CLASS_OBJECT, i));
        if ( pObj == 0 )  break;

        pObj->DeleteDeselList(this);
    }

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
            pObj = m_main->SearchNearest(GetPosition(0), this);
            if ( pObj == 0 )
            {
                m_camera->SetControllingObject(0);
                m_camera->SetType(Gfx::CAM_TYPE_FREE);
            }
            else
            {
                m_camera->SetControllingObject(pObj);
                m_camera->SetType(Gfx::CAM_TYPE_BACK);
            }
        }
#endif
        for ( i=0 ; i<1000000 ; i++ )
        {
            pPyro = static_cast<Gfx::CPyro*>(m_iMan->SearchInstance(CLASS_PYRO, i));
            if ( pPyro == 0 )  break;

            pPyro->CutObjectLink(this);  // the object no longer exists
        }

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
            m_terrain->DeleteBuildingLevel(GetPosition(0));  // flattens the field
        }
    }

    m_type = OBJECT_NULL;  // invalid object until complete destruction

    if ( m_partiReactor != -1 )
    {
        m_particle->DeleteParticle(m_partiReactor);
        m_partiReactor = -1;
    }

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

    if ( m_physics != 0 )
    {
        m_physics->DeleteObject(bAll);
    }

    if ( m_brain != 0 )
    {
        m_brain->DeleteObject(bAll);
    }

    if ( m_motion != 0 )
    {
        m_motion->DeleteObject(bAll);
    }

    if ( m_auto != 0 )
    {
        m_auto->DeleteObject(bAll);
    }

    for ( i=0 ; i<OBJECTMAXPART ; i++ )
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

void CObject::Simplify()
{
    if ( m_brain != 0 )
    {
        m_brain->StopProgram();
    }
    m_main->SaveOneScript(this);

    if ( m_physics != 0 )
    {
        m_physics->DeleteObject();
        delete m_physics;
        m_physics = 0;
    }

    if ( m_brain != 0 )
    {
        m_brain->DeleteObject();
        delete m_brain;
        m_brain = 0;
    }

    if ( m_motion != 0 )
    {
        m_motion->DeleteObject();
        delete m_motion;
        m_motion = 0;
    }

    if ( m_auto != 0 )
    {
        m_auto->DeleteObject();
        delete m_auto;
        m_auto = 0;
    }

    m_main->CreateShortcuts();
}


// Detonates an object, when struck by a shot.
// If false is returned, the object is still screwed.
// If true is returned, the object is destroyed.

bool CObject::ExploObject(ExploType type, float force, float decay)
{
    Gfx::PyroType    pyroType;
    Gfx::CPyro*      pyro;
    float       loss, shield;

    if ( type == EXPLO_BURN )
    {
        if ( m_type == OBJECT_MOBILEtg ||
             m_type == OBJECT_TEEN28    ||  // cylinder?
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
            type = EXPLO_BOUM;
            force = 1.0f;
            decay = 1.0f;
        }
    }

    if ( EXPLO_BOUM )
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
        if ( type == EXPLO_WATER )
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
        if ( type == EXPLO_BURN )  // burning?
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
        else if ( type == EXPLO_WATER )
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
            else if ( m_type == OBJECT_MOBILEtg ||
                      m_type == OBJECT_TEEN28    ||  // cylinder?
                      m_type == OBJECT_TEEN31    )   // basket?
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

    pyro = new Gfx::CPyro(m_iMan);
    pyro->Create(pyroType, this, loss);

    if ( shield == 0.0f )  // dead?
    {
        if ( m_brain != 0 )
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
    DeleteDeselList(this);

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

void CObject::InitPart(int part)
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

// Creates a new part, and returns its number.
// Returns -1 on error.

int CObject::CreatePart()
{
    int     i;

    for ( i=0 ; i<OBJECTMAXPART ; i++ )
    {
        if ( m_objectPart[i].bUsed )  continue;

        InitPart(i);
        UpdateTotalPart();
        return i;
    }
    return -1;
}

// Removes part.

void CObject::DeletePart(int part)
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

void CObject::UpdateTotalPart()
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

void CObject::SetObjectRank(int part, int objRank)
{
    if ( !m_objectPart[part].bUsed )  // object not created?
    {
        InitPart(part);
        UpdateTotalPart();
    }
    m_objectPart[part].object = objRank;
}

// Returns the number of part.

int CObject::GetObjectRank(int part)
{
    if ( !m_objectPart[part].bUsed )  return -1;
    return m_objectPart[part].object;
}

// Specifies what is the parent of a part.
// Reminder: Part 0 is always the father of all
// and therefore the main part (eg the chassis of a car).

void CObject::SetObjectParent(int part, int parent)
{
    m_objectPart[part].parentPart = parent;
}


// Specifies the type of the object.

void CObject::SetType(ObjectType type)
{
    m_type = type;
    strcpy(m_name, GetObjectName(m_type));

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

ObjectType CObject::GetType()
{
    return m_type;
}

char* CObject::GetName()
{
    return m_name;
}


// Choosing the option to use.

void CObject::SetOption(int option)
{
    m_option = option;
}

int CObject::GetOption()
{
    return m_option;
}


// Management of the unique identifier of an object.

void CObject::SetID(int id)
{
    m_id = id;

    if ( m_botVar != 0 )
    {
        m_botVar->SetIdent(m_id);
    }
}

int CObject::GetID()
{
    return m_id;
}


// Saves all the parameters of the object.

bool CObject::Write(char *line)
{
    Math::Vector    pos;
    Info        info;
    char        name[100];
    float       value;
    int         i;

    sprintf(name, " camera=%s", GetCamera(GetCameraType()));
    strcat(line, name);

    if ( GetCameraLock() != 0 )
    {
        sprintf(name, " cameraLock=%d", GetCameraLock());
        strcat(line, name);
    }

    if ( GetEnergy() != 0.0f )
    {
        sprintf(name, " energy=%.2f", GetEnergy());
        strcat(line, name);
    }

    if ( GetCapacity() != 1.0f )
    {
        sprintf(name, " capacity=%.2f", GetCapacity());
        strcat(line, name);
    }

    if ( GetShield() != 1.0f )
    {
        sprintf(name, " shield=%.2f", GetShield());
        strcat(line, name);
    }

    if ( GetRange() != 1.0f )
    {
        sprintf(name, " range=%.2f", GetRange());
        strcat(line, name);
    }

    if ( GetSelectable() != 1 )
    {
        sprintf(name, " selectable=%d", GetSelectable());
        strcat(line, name);
    }

    if ( GetEnable() != 1 )
    {
        sprintf(name, " enable=%d", GetEnable());
        strcat(line, name);
    }

    if ( GetFixed() != 0 )
    {
        sprintf(name, " fixed=%d", GetFixed());
        strcat(line, name);
    }

    if ( GetClip() != 1 )
    {
        sprintf(name, " clip=%d", GetClip());
        strcat(line, name);
    }

    if ( GetLock() != 0 )
    {
        sprintf(name, " lock=%d", GetLock());
        strcat(line, name);
    }

    if ( GetProxyActivate() != 0 )
    {
        sprintf(name, " proxyActivate=%d", GetProxyActivate());
        strcat(line, name);

        sprintf(name, " proxyDistance=%.2f", GetProxyDistance()/g_unit);
        strcat(line, name);
    }

    if ( GetMagnifyDamage() != 1.0f )
    {
        sprintf(name, " magnifyDamage=%.2f", GetMagnifyDamage());
        strcat(line, name);
    }

    if ( GetGunGoalV() != 0.0f )
    {
        sprintf(name, " aimV=%.2f", GetGunGoalV());
        strcat(line, name);
    }
    if ( GetGunGoalH() != 0.0f )
    {
        sprintf(name, " aimH=%.2f", GetGunGoalH());
        strcat(line, name);
    }

    if ( GetParam() != 0.0f )
    {
        sprintf(name, " param=%.2f", GetParam());
        strcat(line, name);
    }

    if ( GetResetCap() != 0 )
    {
        sprintf(name, " resetCap=%d", GetResetCap());
        strcat(line, name);

        pos = GetResetPosition()/g_unit;
        sprintf(name, " resetPos=%.2f;%.2f;%.2f", pos.x, pos.y, pos.z);
        strcat(line, name);

        pos = GetResetAngle()/(Math::PI/180.0f);
        sprintf(name, " resetAngle=%.2f;%.2f;%.2f", pos.x, pos.y, pos.z);
        strcat(line, name);

        sprintf(name, " resetRun=%d", GetResetRun());
        strcat(line, name);
    }

    if ( m_bVirusMode != 0 )
    {
        sprintf(name, " virusMode=%d", m_bVirusMode);
        strcat(line, name);
    }

    if ( m_virusTime != 0.0f )
    {
        sprintf(name, " virusTime=%.2f", m_virusTime);
        strcat(line, name);
    }

    // Puts information in terminal (OBJECT_INFO).
    for ( i=0 ; i<m_infoTotal ; i++ )
    {
        info = GetInfo(i);
        if ( info.name[0] == 0 )  break;

        sprintf(name, " info%d=\"%s=%.2f\"", i+1, info.name, info.value);
        strcat(line, name);
    }

    // Sets the parameters of the command line.
    for ( i=0 ; i<OBJECTMAXCMDLINE ; i++ )
    {
        value = GetCmdLine(i);
        if ( value == NAN )  break;

        if ( i == 0 )  sprintf(name, " cmdline=%.2f", value);
        else           sprintf(name, ";%.2f", value);
        strcat(line, name);
    }

    if ( m_motion != 0 )
    {
        m_motion->Write(line);
    }

    if ( m_brain != 0 )
    {
        m_brain->Write(line);
    }

    if ( m_physics != 0 )
    {
        m_physics->Write(line);
    }

    if ( m_auto != 0 )
    {
        m_auto->Write(line);
    }

    return true;
}

// Returns all parameters of the object.

bool CObject::Read(char *line)
{
    Math::Vector    pos, dir;
    Info            info;
    Gfx::CameraType cType;
    char            op[20];
    char            text[100];
    char*           p;
    float           value;
    int             i;

    cType = OpCamera(line, "camera");
    if ( cType != Gfx::CAM_TYPE_NULL )
    {
        SetCameraType(cType);
    }

    SetCameraLock(OpInt(line, "cameraLock", 0));
    SetEnergy(OpFloat(line, "energy", 0.0f));
    SetCapacity(OpFloat(line, "capacity", 1.0f));
    SetShield(OpFloat(line, "shield", 1.0f));
    SetRange(OpFloat(line, "range", 1.0f));
    SetSelectable(OpInt(line, "selectable", 1));
    SetEnable(OpInt(line, "enable", 1));
    SetFixed(OpInt(line, "fixed", 0));
    SetClip(OpInt(line, "clip", 1));
    SetLock(OpInt(line, "lock", 0));
    SetProxyActivate(OpInt(line, "proxyActivate", 0));
    SetProxyDistance(OpFloat(line, "proxyDistance", 15.0f)*g_unit);
    SetRange(OpFloat(line, "range", 30.0f));
    SetMagnifyDamage(OpFloat(line, "magnifyDamage", 1.0f));
    SetGunGoalV(OpFloat(line, "aimV", 0.0f));
    SetGunGoalH(OpFloat(line, "aimH", 0.0f));
    SetParam(OpFloat(line, "param", 0.0f));
    SetResetCap(static_cast<ResetCap>(OpInt(line, "resetCap", 0)));
    SetResetPosition(OpDir(line, "resetPos")*g_unit);
    SetResetAngle(OpDir(line, "resetAngle")*(Math::PI/180.0f));
    SetResetRun(OpInt(line, "resetRun", 0));
    m_bBurn = OpInt(line, "burnMode", 0);
    m_bVirusMode = OpInt(line, "virusMode", 0);
    m_virusTime = OpFloat(line, "virusTime", 0.0f);

    // Puts information in terminal (OBJECT_INFO).
    for ( i=0 ; i<OBJECTMAXINFO ; i++ )
    {
        sprintf(op, "info%d", i+1);
        OpString(line, op, text);
        if ( text[0] == 0 )  break;
        p = strchr(text, '=');
        if ( p == 0 )  break;
        *p = 0;
        strcpy(info.name, text);
        sscanf(p+1, "%f", &info.value);
        SetInfo(i, info);
    }

    // Sets the parameters of the command line.
    p = SearchOp(line, "cmdline");
    for ( i=0 ; i<OBJECTMAXCMDLINE ; i++ )
    {
        value = GetFloat(p, i, NAN);
        if ( value == NAN )  break;
        SetCmdLine(i, value);
    }

    if ( m_motion != 0 )
    {
        m_motion->Read(line);
    }

    if ( m_brain != 0 )
    {
        m_brain->Read(line);
    }

    if ( m_physics != 0 )
    {
        m_physics->Read(line);
    }

    if ( m_auto != 0 )
    {
        m_auto->Read(line);
    }

    return true;
}



// Seeking the nth son of a father.

int CObject::SearchDescendant(int parent, int n)
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


// Removes all spheres used for collisions.

void CObject::FlushCrashShere()
{
    m_crashSphereUsed = 0;
}

// Adds a new sphere.

int CObject::CreateCrashSphere(Math::Vector pos, float radius, Sound sound,
                               float hardness)
{
    float   zoom;

    if ( m_crashSphereUsed >= MAXCRASHSPHERE )  return -1;

    zoom = GetZoomX(0);
    m_crashSpherePos[m_crashSphereUsed] = pos;
    m_crashSphereRadius[m_crashSphereUsed] = radius*zoom;
    m_crashSphereHardness[m_crashSphereUsed] = hardness;
    m_crashSphereSound[m_crashSphereUsed] = sound;
    return m_crashSphereUsed++;
}

// Returns the number of spheres.

int CObject::GetCrashSphereTotal()
{
    return m_crashSphereUsed;
}

// Returns a sphere for collisions.
// The position is absolute in the world.

bool CObject::GetCrashSphere(int rank, Math::Vector &pos, float &radius)
{
    if ( rank < 0 || rank >= m_crashSphereUsed )
    {
        pos = m_objectPart[0].position;
        radius = 0.0f;
        return false;
    }

    // Returns to the sphere collisions,
    // which ignores the inclination of the vehicle.
    // This is necessary to collisions with vehicles,
    // so as not to reflect SetInclinaison, for example.
    // The sphere must necessarily have a center (0, y, 0).
    if ( rank == 0 && m_crashSphereUsed == 1 &&
         m_crashSpherePos[0].x == 0.0f &&
         m_crashSpherePos[0].z == 0.0f )
    {
        pos = m_objectPart[0].position + m_crashSpherePos[0];
        radius = m_crashSphereRadius[0];
        return true;
    }

    if ( m_objectPart[0].bTranslate ||
         m_objectPart[0].bRotate    )
    {
        UpdateTransformObject();
    }
    pos = Math::Transform(m_objectPart[0].matWorld, m_crashSpherePos[rank]);
    radius = m_crashSphereRadius[rank];
    return true;
}

// Returns the hardness of a sphere.

Sound CObject::GetCrashSphereSound(int rank)
{
    return m_crashSphereSound[rank];
}

// Returns the hardness of a sphere.

float CObject::GetCrashSphereHardness(int rank)
{
    return m_crashSphereHardness[rank];
}

// Deletes a sphere.

void CObject::DeleteCrashSphere(int rank)
{
    int     i;

    if ( rank < 0 || rank >= m_crashSphereUsed )  return;

    for ( i=rank+1 ; i<MAXCRASHSPHERE ; i++ )
    {
        m_crashSpherePos[i-1]    = m_crashSpherePos[i];
        m_crashSphereRadius[i-1] = m_crashSphereRadius[i];
    }
    m_crashSphereUsed --;
}

// Specifies the global sphere, relative to the object.

void CObject::SetGlobalSphere(Math::Vector pos, float radius)
{
    float   zoom;

    zoom = GetZoomX(0);
    m_globalSpherePos    = pos;
    m_globalSphereRadius = radius*zoom;
}

// Returns the global sphere, in the world.

void CObject::GetGlobalSphere(Math::Vector &pos, float &radius)
{
    pos = Math::Transform(m_objectPart[0].matWorld, m_globalSpherePos);
    radius = m_globalSphereRadius;
}


// Specifies the sphere of jostling, relative to the object.

void CObject::SetJotlerSphere(Math::Vector pos, float radius)
{
    m_jotlerSpherePos    = pos;
    m_jotlerSphereRadius = radius;
}

// Specifies the sphere of jostling, in the world.

void CObject::GetJotlerSphere(Math::Vector &pos, float &radius)
{
    pos = Math::Transform(m_objectPart[0].matWorld, m_jotlerSpherePos);
    radius = m_jotlerSphereRadius;
}


// Specifies the radius of the shield.

void CObject::SetShieldRadius(float radius)
{
    m_shieldRadius = radius;
}

// Returns the radius of the shield.

float CObject::GetShieldRadius()
{
    return m_shieldRadius;
}


// Positioning an object on a certain height, above the ground.

void CObject::SetFloorHeight(float height)
{
    Math::Vector    pos;

    pos = m_objectPart[0].position;
    m_terrain->AdjustToFloor(pos);

    if ( m_physics != 0 )
    {
        m_physics->SetLand(height == 0.0f);
        m_physics->SetMotor(height != 0.0f);
    }

    m_objectPart[0].position.y = pos.y+height+m_character.height;
    m_objectPart[0].bTranslate = true;  // it will recalculate the matrices
}

// Adjust the inclination of an object laying on the ground.

void CObject::FloorAdjust()
{
    Math::Vector        pos, n;
    Math::Point         nn;
    float           a;

    pos = GetPosition(0);
    if ( m_terrain->GetNormal(n, pos) )
    {
#if 0
        SetAngleX(0,  sinf(n.z));
        SetAngleZ(0, -sinf(n.x));
        SetAngleY(0, 0.0f);
#else
        a = GetAngleY(0);
        nn = Math::RotatePoint(-a, Math::Point(n.z, n.x));
        SetAngleX(0,  sinf(nn.x));
        SetAngleZ(0, -sinf(nn.y));
#endif
    }
}


// Getes the linear vibration.

void CObject::SetLinVibration(Math::Vector dir)
{
    if ( m_linVibration.x != dir.x ||
         m_linVibration.y != dir.y ||
         m_linVibration.z != dir.z )
    {
        m_linVibration = dir;
        m_objectPart[0].bTranslate = true;
    }
}

Math::Vector CObject::GetLinVibration()
{
    return m_linVibration;
}

// Getes the circular vibration.

void CObject::SetCirVibration(Math::Vector dir)
{
    if ( m_cirVibration.x != dir.x ||
         m_cirVibration.y != dir.y ||
         m_cirVibration.z != dir.z )
    {
        m_cirVibration = dir;
        m_objectPart[0].bRotate = true;
    }
}

Math::Vector CObject::GetCirVibration()
{
    return m_cirVibration;
}

// Getes the inclination.

void CObject::SetInclinaison(Math::Vector dir)
{
    if ( m_inclinaison.x != dir.x ||
         m_inclinaison.y != dir.y ||
         m_inclinaison.z != dir.z )
    {
        m_inclinaison = dir;
        m_objectPart[0].bRotate = true;
    }
}

Math::Vector CObject::GetInclinaison()
{
    return m_inclinaison;
}


// Getes the position of center of the object.

void CObject::SetPosition(int part, const Math::Vector &pos)
{
    Math::Vector    shPos, n[20], norm;
    float       height, radius;
    int         rank, i, j;

    m_objectPart[part].position = pos;
    m_objectPart[part].bTranslate = true;  // it will recalculate the matrices

    if ( part == 0 && !m_bFlat )  // main part?
    {
        rank = m_objectPart[0].object;

        shPos = pos;
        m_terrain->AdjustToFloor(shPos, true);
        m_engine->SetObjectShadowPos(rank, shPos);

        if ( m_physics != 0 && m_physics->GetType() == TYPE_FLYING )
        {
            height = pos.y-shPos.y;
        }
        else
        {
            height = 0.0f;
        }
        m_engine->SetObjectShadowHeight(rank, height);

        // Calculating the normal to the ground in nine strategic locations,
        // then perform a weighted average (the dots in the center are more important).
        radius = m_engine->GetObjectShadowRadius(rank);
        i = 0;

        m_terrain->GetNormal(norm, pos);
        n[i++] = norm;
        n[i++] = norm;
        n[i++] = norm;

        shPos = pos;
        shPos.x += radius*0.6f;
        shPos.z += radius*0.6f;
        m_terrain->GetNormal(norm, shPos);
        n[i++] = norm;
        n[i++] = norm;

        shPos = pos;
        shPos.x -= radius*0.6f;
        shPos.z += radius*0.6f;
        m_terrain->GetNormal(norm, shPos);
        n[i++] = norm;
        n[i++] = norm;

        shPos = pos;
        shPos.x += radius*0.6f;
        shPos.z -= radius*0.6f;
        m_terrain->GetNormal(norm, shPos);
        n[i++] = norm;
        n[i++] = norm;

        shPos = pos;
        shPos.x -= radius*0.6f;
        shPos.z -= radius*0.6f;
        m_terrain->GetNormal(norm, shPos);
        n[i++] = norm;
        n[i++] = norm;

        shPos = pos;
        shPos.x += radius;
        shPos.z += radius;
        m_terrain->GetNormal(norm, shPos);
        n[i++] = norm;

        shPos = pos;
        shPos.x -= radius;
        shPos.z += radius;
        m_terrain->GetNormal(norm, shPos);
        n[i++] = norm;

        shPos = pos;
        shPos.x += radius;
        shPos.z -= radius;
        m_terrain->GetNormal(norm, shPos);
        n[i++] = norm;

        shPos = pos;
        shPos.x -= radius;
        shPos.z -= radius;
        m_terrain->GetNormal(norm, shPos);
        n[i++] = norm;

        norm.LoadZero();
        for ( j=0 ; j<i ; j++ )
        {
            norm += n[j];
        }
        norm /= static_cast<float>(i);  // average vector

        m_engine->SetObjectShadowNormal(rank, norm);

        if ( m_shadowLight != -1 )
        {
            shPos = pos;
            shPos.y += m_shadowHeight;
            m_lightMan->SetLightPos(m_shadowLight, shPos);
        }

        if ( m_effectLight != -1 )
        {
            shPos = pos;
            shPos.y += m_effectHeight;
            m_lightMan->SetLightPos(m_effectLight, shPos);
        }

        if ( m_bShowLimit )
        {
            m_main->AdjustShowLimit(0, pos);
        }
    }
}

Math::Vector CObject::GetPosition(int part)
{
    return m_objectPart[part].position;
}

// Getes the rotation around three axis.

void CObject::SetAngle(int part, const Math::Vector &angle)
{
    m_objectPart[part].angle = angle;
    m_objectPart[part].bRotate = true;  // it will recalculate the matrices

    if ( part == 0 && !m_bFlat )  // main part?
    {
        m_engine->SetObjectShadowAngle(m_objectPart[0].object, m_objectPart[0].angle.y);
    }
}

Math::Vector CObject::GetAngle(int part)
{
    return m_objectPart[part].angle;
}

// Getes the rotation about the axis Y.

void CObject::SetAngleY(int part, float angle)
{
    m_objectPart[part].angle.y = angle;
    m_objectPart[part].bRotate = true;  // it will recalculate the matrices

    if ( part == 0 && !m_bFlat )  // main part?
    {
        m_engine->SetObjectShadowAngle(m_objectPart[0].object, m_objectPart[0].angle.y);
    }
}

// Getes the rotation about the axis X.

void CObject::SetAngleX(int part, float angle)
{
    m_objectPart[part].angle.x = angle;
    m_objectPart[part].bRotate = true;  // it will recalculate the matrices
}

// Getes the rotation about the axis Z.

void CObject::SetAngleZ(int part, float angle)
{
    m_objectPart[part].angle.z = angle;
    m_objectPart[part].bRotate = true;  //it will recalculate the matrices
}

float CObject::GetAngleY(int part)
{
    return m_objectPart[part].angle.y;
}

float CObject::GetAngleX(int part)
{
    return m_objectPart[part].angle.x;
}

float CObject::GetAngleZ(int part)
{
    return m_objectPart[part].angle.z;
}


// Getes the global zoom.

void CObject::SetZoom(int part, float zoom)
{
    m_objectPart[part].bTranslate = true;  // it will recalculate the matrices
    m_objectPart[part].zoom.x = zoom;
    m_objectPart[part].zoom.y = zoom;
    m_objectPart[part].zoom.z = zoom;

    m_objectPart[part].bZoom = ( m_objectPart[part].zoom.x != 1.0f ||
                                 m_objectPart[part].zoom.y != 1.0f ||
                                 m_objectPart[part].zoom.z != 1.0f );
}

void CObject::SetZoom(int part, Math::Vector zoom)
{
    m_objectPart[part].bTranslate = true;  // it will recalculate the matrices
    m_objectPart[part].zoom = zoom;

    m_objectPart[part].bZoom = ( m_objectPart[part].zoom.x != 1.0f ||
                                 m_objectPart[part].zoom.y != 1.0f ||
                                 m_objectPart[part].zoom.z != 1.0f );
}

Math::Vector CObject::GetZoom(int part)
{
    return m_objectPart[part].zoom;
}

void CObject::SetZoomX(int part, float zoom)
{
    m_objectPart[part].bTranslate = true;  // it will recalculate the matrices
    m_objectPart[part].zoom.x = zoom;

    m_objectPart[part].bZoom = ( m_objectPart[part].zoom.x != 1.0f ||
                                 m_objectPart[part].zoom.y != 1.0f ||
                                 m_objectPart[part].zoom.z != 1.0f );
}

void CObject::SetZoomY(int part, float zoom)
{
    m_objectPart[part].bTranslate = true;  // it will recalculate the matrices
    m_objectPart[part].zoom.y = zoom;

    m_objectPart[part].bZoom = ( m_objectPart[part].zoom.x != 1.0f ||
                                 m_objectPart[part].zoom.y != 1.0f ||
                                 m_objectPart[part].zoom.z != 1.0f );
}

void CObject::SetZoomZ(int part, float zoom)
{
    m_objectPart[part].bTranslate = true;  // it will recalculate the matrices
    m_objectPart[part].zoom.z = zoom;

    m_objectPart[part].bZoom = ( m_objectPart[part].zoom.x != 1.0f ||
                                 m_objectPart[part].zoom.y != 1.0f ||
                                 m_objectPart[part].zoom.z != 1.0f );
}

float CObject::GetZoomX(int part)
{
    return m_objectPart[part].zoom.x;
}

float CObject::GetZoomY(int part)
{
    return m_objectPart[part].zoom.y;
}

float CObject::GetZoomZ(int part)
{
    return m_objectPart[part].zoom.z;
}


// Returns the water level.

float CObject::GetWaterLevel()
{
    return m_water->GetLevel();
}


void CObject::SetTrainer(bool bEnable)
{
    m_bTrainer = bEnable;

    if ( m_bTrainer )  // training?
    {
        m_cameraType = Gfx::CAM_TYPE_FIX;
    }
}

bool CObject::GetTrainer()
{
    return m_bTrainer;
}

void CObject::SetToy(bool bEnable)
{
    m_bToy = bEnable;
}

bool CObject::GetToy()
{
    return m_bToy;
}

void CObject::SetManual(bool bManual)
{
    m_bManual = bManual;
}

bool CObject::GetManual()
{
    return m_bManual;
}

void CObject::SetResetCap(ResetCap cap)
{
    m_resetCap = cap;
}

ResetCap CObject::GetResetCap()
{
    return m_resetCap;
}

void CObject::SetResetBusy(bool bBusy)
{
    m_bResetBusy = bBusy;
}

bool CObject::GetResetBusy()
{
    return m_bResetBusy;
}

void CObject::SetResetPosition(const Math::Vector &pos)
{
    m_resetPosition = pos;
}

Math::Vector CObject::GetResetPosition()
{
    return m_resetPosition;
}

void CObject::SetResetAngle(const Math::Vector &angle)
{
    m_resetAngle = angle;
}

Math::Vector CObject::GetResetAngle()
{
    return m_resetAngle;
}

int CObject::GetResetRun()
{
    return m_resetRun;
}

void CObject::SetResetRun(int run)
{
    m_resetRun = run;
}


// Management of the particle master.

void CObject::SetMasterParticle(int part, int parti)
{
    m_objectPart[part].masterParti = parti;
}

int CObject::GetMasterParticle(int part)
{
    return m_objectPart[part].masterParti;
}


// Management of the stack transport.

void CObject::SetPower(CObject* power)
{
    m_power = power;
}

CObject* CObject::GetPower()
{
    return m_power;
}

// Management of the object transport.

void CObject::SetFret(CObject* fret)
{
    m_fret = fret;
}

CObject* CObject::GetFret()
{
    return m_fret;
}

// Management of the object "truck" that transports it.

void CObject::SetTruck(CObject* truck)
{
    m_truck = truck;

    // Invisible shadow if the object is transported.
    m_engine->SetObjectShadowHide(m_objectPart[0].object, (m_truck != 0));
}

CObject* CObject::GetTruck()
{
    return m_truck;
}

// Management of the conveying portion.

void CObject::SetTruckPart(int part)
{
    m_truckLink = part;
}

int CObject::GetTruckPart()
{
    return m_truckLink;
}


// Management of user information.

void CObject::InfoFlush()
{
    m_infoTotal = 0;
    m_bInfoUpdate = true;
}

void CObject::DeleteInfo(int rank)
{
    int     i;

    if ( rank < 0 || rank >= m_infoTotal )  return;

    for ( i=rank ; i<m_infoTotal-1 ; i++ )
    {
        m_info[i] = m_info[i+1];
    }
    m_infoTotal --;
    m_bInfoUpdate = true;
}

void CObject::SetInfo(int rank, Info info)
{
    if ( rank < 0 || rank >= OBJECTMAXINFO )  return;
    m_info[rank] = info;

    if ( rank+1 > m_infoTotal )  m_infoTotal = rank+1;
    m_bInfoUpdate = true;
}

Info CObject::GetInfo(int rank)
{
    if ( rank < 0 || rank >= OBJECTMAXINFO )  rank = 0;
    return m_info[rank];
}

int CObject::GetInfoTotal()
{
    return m_infoTotal;
}

void CObject::SetInfoReturn(float value)
{
    m_infoReturn = value;
}

float CObject::GetInfoReturn()
{
    return m_infoReturn;
}

void CObject::SetInfoUpdate(bool bUpdate)
{
    m_bInfoUpdate = bUpdate;
}

bool CObject::GetInfoUpdate()
{
    return m_bInfoUpdate;
}


bool CObject::SetCmdLine(int rank, float value)
{
    if ( rank < 0 || rank >= OBJECTMAXCMDLINE )  return false;
    m_cmdLine[rank] = value;
    return true;
}

float CObject::GetCmdLine(int rank)
{
    if ( rank < 0 || rank >= OBJECTMAXCMDLINE )  return 0.0f;
    return m_cmdLine[rank];
}


// Returns matrices of an object portion.

Math::Matrix* CObject::GetRotateMatrix(int part)
{
    return &m_objectPart[part].matRotate;
}

Math::Matrix* CObject::GetTranslateMatrix(int part)
{
    return &m_objectPart[part].matTranslate;
}

Math::Matrix* CObject::GetTransformMatrix(int part)
{
    return &m_objectPart[part].matTransform;
}

Math::Matrix* CObject::GetWorldMatrix(int part)
{
    if ( m_objectPart[0].bTranslate ||
         m_objectPart[0].bRotate    )
    {
        UpdateTransformObject();
    }

    return &m_objectPart[part].matWorld;
}


// Indicates whether the object should be drawn below the interface.

void CObject::SetDrawWorld(bool bDraw)
{
    int     i;

    for ( i=0 ; i<OBJECTMAXPART ; i++ )
    {
        if ( m_objectPart[i].bUsed )
        {
            m_engine->SetObjectDrawWorld(m_objectPart[i].object, bDraw);
        }
    }
}

// Indicates whether the object should be drawn over the interface.

void CObject::SetDrawFront(bool bDraw)
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


// Creates a vehicle traveling any pose on the floor.

bool CObject::CreateVehicle(Math::Vector pos, float angle, ObjectType type,
                            float power, bool bTrainer, bool bToy)
{
    m_type = type;

    if ( type == OBJECT_TOTO )
    {
        m_motion = new CMotionToto(m_iMan, this);
        m_motion->Create(pos, angle, type, 1.0f);
        return true;
    }

    SetTrainer(bTrainer);
    SetToy(bToy);

    m_physics = new CPhysics(m_iMan, this);
    m_brain   = new CBrain(m_iMan, this);

    m_physics->SetBrain(m_brain);
    m_brain->SetPhysics(m_physics);

#if 0
    if ( type == OBJECT_MOBILEfc ||
         type == OBJECT_MOBILEtc ||
         type == OBJECT_MOBILEwc ||
         type == OBJECT_MOBILEic )  // fireball cannon?
    {
        m_showLimitRadius = 160.0f;
    }
    if ( type == OBJECT_MOBILEfi ||
         type == OBJECT_MOBILEti ||
         type == OBJECT_MOBILEwi ||
         type == OBJECT_MOBILEii )  // orgaball cannon?
    {
        m_showLimitRadius = 160.0f;
    }
    if ( type == OBJECT_MOBILErc )  // phazer cannon?
    {
        m_showLimitRadius = 160.0f;
    }
    if ( type == OBJECT_MOBILErs )  // robot shield?
    {
        m_showLimitRadius = 50.0f;
    }
#endif
    if ( type == OBJECT_MOBILErt )  // robot thumper?
    {
        m_showLimitRadius = 400.0f;
    }

    if ( type == OBJECT_HUMAN ||
         type == OBJECT_TECH  )
    {
        m_motion = new CMotionHuman(m_iMan, this);
    }
    else
    {
        m_motion = new CMotionVehicle(m_iMan, this);
    }
    if ( m_motion == 0 )  return false;

    m_physics->SetMotion(m_motion);
    m_brain->SetMotion(m_motion);
    m_motion->SetPhysics(m_physics);
    m_motion->SetBrain(m_brain);
    if ( !m_motion->Create(pos, angle, type, power) )
    {
        if ( m_physics != 0 )
        {
            m_physics->DeleteObject();
            delete m_physics;
            m_physics = 0;
        }
        if ( m_brain != 0 )
        {
            m_brain->DeleteObject();
            delete m_brain;
            m_brain = 0;
        }
        if ( m_motion != 0 )
        {
            m_motion->DeleteObject();
            delete m_motion;
            m_motion = 0;
        }
        return false;
    }

    return true;
}

// Creates an insect lands on any ground.

bool CObject::CreateInsect(Math::Vector pos, float angle, ObjectType type)
{
    m_type = type;

    m_physics = new CPhysics(m_iMan, this);
    m_brain   = new CBrain(m_iMan, this);

    m_physics->SetBrain(m_brain);
    m_brain->SetPhysics(m_physics);

    if ( type == OBJECT_MOTHER )
    {
        m_motion = new CMotionMother(m_iMan, this);
    }
    if ( type == OBJECT_ANT )
    {
        m_motion = new CMotionAnt(m_iMan, this);
    }
    if ( type == OBJECT_SPIDER )
    {
        m_motion = new CMotionSpider(m_iMan, this);
    }
    if ( type == OBJECT_BEE )
    {
        m_motion = new CMotionBee(m_iMan, this);
    }
    if ( type == OBJECT_WORM )
    {
        m_motion = new CMotionWorm(m_iMan, this);
    }
    if ( m_motion == 0 )  return false;

    m_physics->SetMotion(m_motion);
    m_brain->SetMotion(m_motion);
    m_motion->SetPhysics(m_physics);
    m_motion->SetBrain(m_brain);
    if ( !m_motion->Create(pos, angle, type, 0.0f) )
    {
        if ( m_physics != 0 )
        {
            m_physics->DeleteObject();
            delete m_physics;
            m_physics = 0;
        }
        if ( m_brain != 0 )
        {
            m_brain->DeleteObject();
            delete m_brain;
            m_brain = 0;
        }
        if ( m_motion != 0 )
        {
            m_motion->DeleteObject();
            delete m_motion;
            m_motion = 0;
        }
        return false;
    }

    return true;
}

// Creates shade under a vehicle as a negative light.

bool CObject::CreateShadowLight(float height, Gfx::Color color)
{
    if ( !m_engine->GetLightMode() )  return true;

    Math::Vector pos = GetPosition(0);
    m_shadowHeight = height;

    Gfx::Light light;
    light.type          = Gfx::LIGHT_SPOT;
    light.diffuse.r     = color.r;
    light.diffuse.g     = color.g;
    light.diffuse.b     = color.b;
    light.position.x    = pos.x;
    light.position.y    = pos.y+height;
    light.position.z    = pos.z;
    light.direction.x   =  0.0f;
    light.direction.y   = -1.0f;  // against the bottom
    light.direction.z   =  0.0f;
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

int CObject::GetShadowLight()
{
    return m_shadowLight;
}

// Creates light for the effects of a vehicle.

bool CObject::CreateEffectLight(float height, Gfx::Color color)
{
    if ( !m_engine->GetLightMode() )  return true;

    m_effectHeight = height;

    Gfx::Light light;
    light.type       = Gfx::LIGHT_SPOT;
    light.diffuse.r  = color.r;
    light.diffuse.g  = color.g;
    light.diffuse.b  = color.b;
    light.position.x  =  0.0f;
    light.position.y  =  0.0f+height;
    light.position.z  =  0.0f;
    light.direction.x =  0.0f;
    light.direction.y = -1.0f;  // against the bottom
    light.direction.z =  0.0f;
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

int CObject::GetEffectLight()
{
    return m_effectLight;
}

// Creates the circular shadow underneath a vehicle.

bool CObject::CreateShadowCircle(float radius, float intensity,
                                 Gfx::EngineShadowType type)
{
    float   zoom;

    if ( intensity == 0.0f )  return true;

    zoom = GetZoomX(0);

    m_engine->CreateShadow(m_objectPart[0].object);

    m_engine->SetObjectShadowRadius(m_objectPart[0].object, radius*zoom);
    m_engine->SetObjectShadowIntensity(m_objectPart[0].object, intensity);
    m_engine->SetObjectShadowHeight(m_objectPart[0].object, 0.0f);
    m_engine->SetObjectShadowAngle(m_objectPart[0].object, m_objectPart[0].angle.y);
    m_engine->SetObjectShadowType(m_objectPart[0].object, type);

    return true;
}

// Creates a building laying on the ground.

bool CObject::CreateBuilding(Math::Vector pos, float angle, float height,
                             ObjectType type, float power)
{
    Math::Point     p;
    int         rank, i;

    Gfx::CModelManager* modelManager = Gfx::CModelManager::GetInstancePointer();

    SetType(type);

    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);  // it is a stationary object
    SetObjectRank(0, rank);

    if ( m_type == OBJECT_PORTICO )
    {
        modelManager->AddModelReference("portico1.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);
        SetFloorHeight(0.0f);

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        SetObjectRank(1, rank);
        SetObjectParent(1, 0);
        modelManager->AddModelReference("portico2.mod", false, rank);
        SetPosition(1, Math::Vector(0.0f, 67.0f, 0.0f));

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        SetObjectRank(2, rank);
        SetObjectParent(2, 1);
        modelManager->AddModelReference("portico3.mod", false, rank);
        SetPosition(2, Math::Vector(0.0f, 0.0f, -33.0f));
        SetAngleY(2, 45.0f*Math::PI/180.0f);

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        SetObjectRank(3, rank);
        SetObjectParent(3, 2);
        modelManager->AddModelReference("portico4.mod", false, rank);
        SetPosition(3, Math::Vector(50.0f, 0.0f, 0.0f));
        SetAngleY(3, -60.0f*Math::PI/180.0f);

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        SetObjectRank(4, rank);
        SetObjectParent(4, 3);
        modelManager->AddModelReference("portico5.mod", false, rank);
        SetPosition(4, Math::Vector(35.0f, 0.0f, 0.0f));
        SetAngleY(4, -55.0f*Math::PI/180.0f);

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        SetObjectRank(5, rank);
        SetObjectParent(5, 1);
        modelManager->AddModelReference("portico3.mod", false, rank);
        SetPosition(5, Math::Vector(0.0f, 0.0f, 33.0f));
        SetAngleY(5, -45.0f*Math::PI/180.0f);

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        SetObjectRank(6, rank);
        SetObjectParent(6, 5);
        modelManager->AddModelReference("portico4.mod", false, rank);
        SetPosition(6, Math::Vector(50.0f, 0.0f, 0.0f));
        SetAngleY(6, 60.0f*Math::PI/180.0f);

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        SetObjectRank(7, rank);
        SetObjectParent(7, 6);
        modelManager->AddModelReference("portico5.mod", false, rank);
        SetPosition(7, Math::Vector(35.0f, 0.0f, 0.0f));
        SetAngleY(7, 55.0f*Math::PI/180.0f);

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        SetObjectRank(8, rank);
        SetObjectParent(8, 0);
        modelManager->AddModelReference("portico6.mod", false, rank);
        SetPosition(8, Math::Vector(-35.0f, 50.0f, -35.0f));
        SetAngleY(8, -Math::PI/2.0f);
        SetZoom(8, 2.0f);

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        SetObjectRank(9, rank);
        SetObjectParent(9, 8);
        modelManager->AddModelReference("portico7.mod", false, rank);
        SetPosition(9, Math::Vector(0.0f, 4.5f, 1.9f));

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        SetObjectRank(10, rank);
        SetObjectParent(10, 0);
        modelManager->AddModelReference("portico6.mod", false, rank);
        SetPosition(10, Math::Vector(-35.0f, 50.0f, 35.0f));
        SetAngleY(10, -Math::PI/2.0f);
        SetZoom(10, 2.0f);

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        SetObjectRank(11, rank);
        SetObjectParent(11, 10);
        modelManager->AddModelReference("portico7.mod", false, rank);
        SetPosition(11, Math::Vector(0.0f, 4.5f, 1.9f));

        CreateCrashSphere(Math::Vector(  0.0f, 28.0f,   0.0f), 45.5f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( 27.0f, 10.0f, -42.0f), 15.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(  0.0f, 10.0f, -42.0f), 15.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-27.0f, 10.0f, -42.0f), 15.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( 27.0f, 10.0f,  42.0f), 15.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(  0.0f, 10.0f,  42.0f), 15.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-27.0f, 10.0f,  42.0f), 15.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-32.0f, 45.0f, -32.0f), 10.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-32.0f, 45.0f,  32.0f), 10.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( 32.0f, 45.0f, -32.0f), 10.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( 32.0f, 45.0f,  32.0f), 10.0f, SOUND_BOUMm, 0.45f);
        SetGlobalSphere(Math::Vector(0.0f, 35.0f, 0.0f), 50.0f);

        CreateShadowCircle(50.0f, 1.0f);
    }

    if ( m_type == OBJECT_BASE )
    {
        modelManager->AddModelReference("base1.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);
        SetFloorHeight(0.0f);

        for ( i=0 ; i<8 ; i++ )
        {
            rank = m_engine->CreateObject();
            m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
            SetObjectRank(1+i, rank);
            SetObjectParent(1+i, 0);
            modelManager->AddModelReference("base2.mod", false, rank);
            p = Math::RotatePoint(-Math::PI/4.0f*i, 27.8f);
            SetPosition(1+i, Math::Vector(p.x, 30.0f, p.y));
            SetAngleY(1+i, Math::PI/4.0f*i);
            SetAngleZ(1+i, Math::PI/2.0f);

            rank = m_engine->CreateObject();
            m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
            SetObjectRank(10+i, rank);
            SetObjectParent(10+i, 1+i);
            modelManager->AddModelReference("base4.mod", false, rank);
            SetPosition(10+i, Math::Vector(23.5f, 0.0f, 7.0f));

            rank = m_engine->CreateObject();
            m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
            SetObjectRank(18+i, rank);
            SetObjectParent(18+i, 1+i);
            modelManager->AddModelReference("base4.mod", true, rank);
            SetPosition(18+i, Math::Vector(23.5f, 0.0f, -7.0f));
        }

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        SetObjectRank(9, rank);
        SetObjectParent(9, 0);
        modelManager->AddModelReference("base3.mod", false, rank); // central pillar

        CreateCrashSphere(Math::Vector(  0.0f, 33.0f,   0.0f),  2.5f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(  0.0f, 39.0f,   0.0f),  2.5f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(  0.0f, 45.0f,   0.0f),  2.5f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(  0.0f, 51.0f,   0.0f),  2.5f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(  0.0f, 57.0f,   0.0f),  2.5f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(  0.0f, 63.0f,   0.0f),  2.5f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(  0.0f, 69.0f,   0.0f),  2.5f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(  0.0f, 82.0f,   0.0f),  8.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( 18.0f, 94.0f,   0.0f), 10.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-18.0f, 94.0f,   0.0f), 10.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(  0.0f, 94.0f,  18.0f), 10.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(  0.0f, 94.0f, -18.0f), 10.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( 13.0f, 94.0f,  13.0f), 10.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-13.0f, 94.0f,  13.0f), 10.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( 13.0f, 94.0f, -13.0f), 10.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-13.0f, 94.0f, -13.0f), 10.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(  0.0f,104.0f,   0.0f), 14.0f, SOUND_BOUMm, 0.45f);

        SetGlobalSphere(Math::Vector(0.0f, 45.0f, 0.0f), 10.0f);

        CreateShadowCircle(60.0f, 1.0f);
        m_showLimitRadius = 200.0f;

        m_terrain->AddBuildingLevel(pos, 28.6f, 73.4f, 30.0f, 0.4f);
    }

    if ( m_type == OBJECT_DERRICK )
    {
        modelManager->AddModelReference("derrick1.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);
        SetFloorHeight(0.0f);

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        SetObjectRank(1, rank);
        SetObjectParent(1, 0);
        modelManager->AddModelReference("derrick2.mod", false, rank);

        CreateCrashSphere(Math::Vector(0.0f,  0.0f, 0.0f), 6.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(0.0f, 10.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(0.0f, 17.0f, 0.0f), 6.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(0.0f, 26.0f, 0.0f), 3.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(7.0f, 17.0f, 0.0f), 3.0f, SOUND_BOUMm, 0.45f);
        SetGlobalSphere(Math::Vector(0.0f, 10.0f, 0.0f), 10.0f);

        CreateShadowCircle(10.0f, 0.4f);
    }

    if ( m_type == OBJECT_RESEARCH )
    {
        modelManager->AddModelReference("search1.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);
        SetFloorHeight(0.0f);

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        SetObjectRank(1, rank);
        SetObjectParent(1, 0);
        modelManager->AddModelReference("search2.mod", false, rank);
        SetPosition(1, Math::Vector(0.0f, 13.0f, 0.0f));

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        SetObjectRank(2, rank);
        SetObjectParent(2, 1);
        modelManager->AddModelReference("search3.mod", false, rank);
        SetPosition(2, Math::Vector(0.0f, 4.0f, 0.0f));
        SetAngleZ(2, 35.0f*Math::PI/180.0f);

        CreateCrashSphere(Math::Vector(0.0f,  0.0f, 0.0f), 9.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(0.0f,  6.0f, 0.0f), 9.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(0.0f, 14.0f, 0.0f), 7.0f, SOUND_BOUMm, 0.45f);
        SetGlobalSphere(Math::Vector(0.0f, 8.0f, 0.0f), 12.0f);

        m_character.posPower = Math::Vector(7.5f, 3.0f, 0.0f);

        CreateShadowCircle(12.0f, 1.0f);
    }

    if ( m_type == OBJECT_RADAR )
    {
        modelManager->AddModelReference("radar1.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);
        SetFloorHeight(0.0f);

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        SetObjectRank(1, rank);
        SetObjectParent(1, 0);
        modelManager->AddModelReference("radar2.mod", false, rank);
        SetPosition(1, Math::Vector(0.0f, 5.0f, 0.0f));

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        SetObjectRank(2, rank);
        SetObjectParent(2, 0);
        modelManager->AddModelReference("radar3.mod", false, rank);
        SetPosition(2, Math::Vector(0.0f, 11.0f, 0.0f));
        SetAngleY(2, -Math::PI/2.0f);

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        SetObjectRank(3, rank);
        SetObjectParent(3, 2);
        modelManager->AddModelReference("radar4.mod", false, rank);
        SetPosition(3, Math::Vector(0.0f, 4.5f, 1.9f));

        CreateCrashSphere(Math::Vector(0.0f,  3.0f, 0.0f), 6.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(0.0f, 11.0f, 0.0f), 6.0f, SOUND_BOUMm, 0.45f);
        SetGlobalSphere(Math::Vector(0.0f, 7.0f, 0.0f), 7.0f);

        CreateShadowCircle(8.0f, 1.0f);
    }

    if ( m_type == OBJECT_INFO )
    {
        modelManager->AddModelReference("info1.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);
        SetFloorHeight(0.0f);

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        SetObjectRank(1, rank);
        SetObjectParent(1, 0);
        modelManager->AddModelReference("info2.mod", false, rank);
        SetPosition(1, Math::Vector(0.0f, 5.0f, 0.0f));

        for ( i=0 ; i<3 ; i++ )
        {
            rank = m_engine->CreateObject();
            m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
            SetObjectRank(2+i*2, rank);
            SetObjectParent(2+i*2, 1);
            modelManager->AddModelReference("info3.mod", false, rank);
            SetPosition(2+i*2, Math::Vector(0.0f, 4.5f, 0.0f));

            rank = m_engine->CreateObject();
            m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
            SetObjectRank(3+i*2, rank);
            SetObjectParent(3+i*2, 2+i*2);
            modelManager->AddModelReference("radar4.mod", false, rank);
            SetPosition(3+i*2, Math::Vector(0.0f, 0.0f, -4.0f));

            SetAngleY(2+i*2, 2.0f*Math::PI/3.0f*i);
        }

        CreateCrashSphere(Math::Vector(0.0f,  3.0f, 0.0f), 6.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(0.0f, 11.0f, 0.0f), 6.0f, SOUND_BOUMm, 0.45f);
        SetGlobalSphere(Math::Vector(0.0f, 5.0f, 0.0f), 6.0f);

        CreateShadowCircle(8.0f, 1.0f);
    }

    if ( m_type == OBJECT_ENERGY )
    {
        modelManager->AddModelReference("energy.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);
        SetFloorHeight(0.0f);

        CreateCrashSphere(Math::Vector(-2.0f, 13.0f, 0.0f), 6.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-7.0f,  3.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( 0.0f,  1.0f, 0.0f), 1.5f, SOUND_BOUMm, 0.45f);
        SetGlobalSphere(Math::Vector(-7.0f, 5.0f, 0.0f), 5.0f);

        m_character.posPower = Math::Vector(0.0f, 3.0f, 0.0f);
        m_energy = power;  // initializes the energy level

        CreateShadowCircle(6.0f, 0.5f);
    }

    if ( m_type == OBJECT_LABO )
    {
        modelManager->AddModelReference("labo1.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);
        SetFloorHeight(0.0f);

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        SetObjectRank(1, rank);
        SetObjectParent(1, 0);
        modelManager->AddModelReference("labo2.mod", false, rank);
        SetPosition(1, Math::Vector(-9.0f, 3.0f, 0.0f));
        SetAngleZ(1, Math::PI/2.0f);

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        SetObjectRank(2, rank);
        SetObjectParent(2, 1);
        modelManager->AddModelReference("labo3.mod", false, rank);
        SetPosition(2, Math::Vector(9.0f, -1.0f, 0.0f));

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        SetObjectRank(3, rank);
        SetObjectParent(3, 2);
        modelManager->AddModelReference("labo4.mod", false, rank);
        SetPosition(3, Math::Vector(0.0f, 0.0f, 0.0f));
        SetAngleZ(3, 80.0f*Math::PI/180.0f);

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        SetObjectRank(4, rank);
        SetObjectParent(4, 2);
        modelManager->AddModelReference("labo4.mod", false, rank);
        SetPosition(4, Math::Vector(0.0f, 0.0f, 0.0f));
        SetAngleZ(4, 80.0f*Math::PI/180.0f);
        SetAngleY(4, Math::PI*2.0f/3.0f);

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        SetObjectRank(5, rank);
        SetObjectParent(5, 2);
        modelManager->AddModelReference("labo4.mod", false, rank);
        SetPosition(5, Math::Vector(0.0f, 0.0f, 0.0f));
        SetAngleZ(5, 80.0f*Math::PI/180.0f);
        SetAngleY(5, -Math::PI*2.0f/3.0f);

        CreateCrashSphere(Math::Vector(  0.0f,  1.0f,  0.0f), 1.5f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(  0.0f, 11.0f,  0.0f), 4.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-10.0f, 10.0f,  0.0f), 4.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-12.0f,  3.0f,  3.0f), 4.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-12.0f,  3.0f, -3.0f), 4.0f, SOUND_BOUMm, 0.45f);
        SetGlobalSphere(Math::Vector(-10.0f, 5.0f, 0.0f), 7.0f);

        m_character.posPower = Math::Vector(0.0f, 3.0f, 0.0f);

        CreateShadowCircle(7.0f, 0.5f);
    }

    if ( m_type == OBJECT_FACTORY )
    {
        modelManager->AddModelReference("factory1.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);
        SetFloorHeight(0.0f);

        for ( i=0 ; i<9 ; i++ )
        {
            rank = m_engine->CreateObject();
            m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
            SetObjectRank(1+i, rank);
            SetObjectParent(1+i, 0);
            modelManager->AddModelReference("factory2.mod", false, rank);
            SetPosition(1+i, Math::Vector(10.0f, 2.0f*i, 10.0f));
            SetAngleZ(1+i, Math::PI/2.0f);
            SetZoomZ(1+i, 0.30f);

            rank = m_engine->CreateObject();
            m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
            SetObjectRank(10+i, rank);
            SetObjectParent(10+i, 0);
            modelManager->AddModelReference("factory2.mod", false, rank);
            SetPosition(10+i, Math::Vector(10.0f, 2.0f*i, -10.0f));
            SetAngleZ(10+i, -Math::PI/2.0f);
            SetAngleY(10+i, Math::PI);
            SetZoomZ(10+i, 0.30f);
        }

        for ( i=0 ; i<2 ; i++ )
        {
            float s = static_cast<float>(i*2-1);
            CreateCrashSphere(Math::Vector(-10.0f,  2.0f, 11.0f*s), 4.0f, SOUND_BOUMm, 0.45f);
            CreateCrashSphere(Math::Vector( -3.0f,  2.0f, 11.0f*s), 4.0f, SOUND_BOUMm, 0.45f);
            CreateCrashSphere(Math::Vector(  3.0f,  2.0f, 11.0f*s), 4.0f, SOUND_BOUMm, 0.45f);
            CreateCrashSphere(Math::Vector( 10.0f,  2.0f, 11.0f*s), 4.0f, SOUND_BOUMm, 0.45f);
            CreateCrashSphere(Math::Vector(-10.0f,  9.0f, 11.0f*s), 4.0f, SOUND_BOUMm, 0.45f);
            CreateCrashSphere(Math::Vector( -3.0f,  9.0f, 11.0f*s), 4.0f, SOUND_BOUMm, 0.45f);
            CreateCrashSphere(Math::Vector(  3.0f,  9.0f, 11.0f*s), 4.0f, SOUND_BOUMm, 0.45f);
            CreateCrashSphere(Math::Vector( 10.0f,  9.0f, 11.0f*s), 4.0f, SOUND_BOUMm, 0.45f);
            CreateCrashSphere(Math::Vector(-10.0f, 16.0f, 11.0f*s), 4.0f, SOUND_BOUMm, 0.45f);
            CreateCrashSphere(Math::Vector( -3.0f, 16.0f, 11.0f*s), 4.0f, SOUND_BOUMm, 0.45f);
            CreateCrashSphere(Math::Vector(  3.0f, 16.0f, 11.0f*s), 4.0f, SOUND_BOUMm, 0.45f);
            CreateCrashSphere(Math::Vector( 10.0f, 16.0f, 11.0f*s), 4.0f, SOUND_BOUMm, 0.45f);
            CreateCrashSphere(Math::Vector(-10.0f, 16.0f,  4.0f*s), 4.0f, SOUND_BOUMm, 0.45f);
            CreateCrashSphere(Math::Vector( -3.0f, 16.0f,  4.0f*s), 4.0f, SOUND_BOUMm, 0.45f);
            CreateCrashSphere(Math::Vector(  3.0f, 16.0f,  4.0f*s), 4.0f, SOUND_BOUMm, 0.45f);
            CreateCrashSphere(Math::Vector( 10.0f, 16.0f,  4.0f*s), 4.0f, SOUND_BOUMm, 0.45f);
            CreateCrashSphere(Math::Vector(-10.0f,  2.0f,  4.0f*s), 4.0f, SOUND_BOUMm, 0.45f);
            CreateCrashSphere(Math::Vector(-10.0f,  9.0f,  4.0f*s), 4.0f, SOUND_BOUMm, 0.45f);
        }
        CreateCrashSphere(Math::Vector(-10.0f, 21.0f, -4.0f), 3.0f, SOUND_BOUMm, 0.45f);
        SetGlobalSphere(Math::Vector(0.0f, 10.0f, 0.0f), 18.0f);

        CreateShadowCircle(24.0f, 0.3f);
    }

    if ( m_type == OBJECT_REPAIR )
    {
        modelManager->AddModelReference("repair1.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);
        SetFloorHeight(0.0f);

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        SetObjectRank(1, rank);
        SetObjectParent(1, 0);
        modelManager->AddModelReference("repair2.mod", false, rank);
        SetPosition(1, Math::Vector(-11.0f, 13.5f, 0.0f));
        SetAngleZ(1, Math::PI/2.0f);

        m_terrain->AddBuildingLevel(pos, 7.0f, 9.0f, 1.0f, 0.5f);

        CreateCrashSphere(Math::Vector(-11.0f,  0.0f,  4.0f), 5.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-11.0f,  0.0f,  0.0f), 5.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-11.0f,  0.0f, -4.0f), 5.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-11.0f, 10.0f,  0.0f), 5.0f, SOUND_BOUMm, 0.45f);
        SetGlobalSphere(Math::Vector(-11.0f, 13.0f, 0.0f), 15.0f);
    }

    if ( m_type == OBJECT_DESTROYER )
    {
        modelManager->AddModelReference("destroy1.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);
        SetFloorHeight(0.0f);

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        SetObjectRank(1, rank);
        SetObjectParent(1, 0);
        modelManager->AddModelReference("destroy2.mod", false, rank);
        SetPosition(1, Math::Vector(0.0f, 0.0f, 0.0f));

        m_terrain->AddBuildingLevel(pos, 7.0f, 9.0f, 1.0f, 0.5f);

        CreateCrashSphere(Math::Vector(-3.5f, 0.0f, -13.5f), 4.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( 3.5f, 0.0f, -13.5f), 4.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-3.5f, 0.0f,  13.5f), 4.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( 3.5f, 0.0f,  13.5f), 4.0f, SOUND_BOUMm, 0.45f);

        CreateShadowCircle(19.0f, 1.0f);
    }

    if ( m_type == OBJECT_STATION )
    {
        modelManager->AddModelReference("station.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);
        SetFloorHeight(0.0f);

        m_terrain->AddBuildingLevel(pos, 7.0f, 9.0f, 1.0f, 0.5f);

        CreateCrashSphere(Math::Vector(-15.0f, 2.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-15.0f, 6.0f, 0.0f), 4.0f, SOUND_BOUMm, 0.45f);
        SetGlobalSphere(Math::Vector(-15.0f, 5.0f, 0.0f), 6.0f);

        m_energy = power;  // initialise le niveau d'�nergie
    }

    if ( m_type == OBJECT_CONVERT )
    {
        modelManager->AddModelReference("convert1.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);
        SetFloorHeight(0.0f);

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        SetObjectRank(1, rank);
        SetObjectParent(1, 0);
        modelManager->AddModelReference("convert2.mod", false, rank);
        SetPosition(1, Math::Vector(0.0f, 14.0f, 0.0f));

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        SetObjectRank(2, rank);
        SetObjectParent(2, 0);
        modelManager->AddModelReference("convert3.mod", false, rank);
        SetPosition(2, Math::Vector(0.0f, 11.5f, 0.0f));
        SetAngleX(2, -Math::PI*0.35f);

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        SetObjectRank(3, rank);
        SetObjectParent(3, 0);
        modelManager->AddModelReference("convert3.mod", false, rank);
        SetPosition(3, Math::Vector(0.0f, 11.5f, 0.0f));
        SetAngleY(3, Math::PI);
        SetAngleX(3, -Math::PI*0.35f);

        m_terrain->AddBuildingLevel(pos, 7.0f, 9.0f, 1.0f, 0.5f);

        CreateCrashSphere(Math::Vector(-10.0f,  2.0f,  4.0f), 5.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-10.0f,  2.0f, -4.0f), 5.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-10.0f,  9.0f,  0.0f), 6.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(  0.0f, 14.0f,  0.0f), 1.5f, SOUND_BOUMm, 0.45f);
        SetGlobalSphere(Math::Vector(-3.0f, 8.0f, 0.0f), 14.0f);
    }

    if ( m_type == OBJECT_TOWER )
    {
        modelManager->AddModelReference("tower.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);
        SetFloorHeight(0.0f);

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        SetObjectRank(1, rank);
        SetObjectParent(1, 0);
        modelManager->AddModelReference("roller2c.mod", false, rank);
        SetPosition(1, Math::Vector(0.0f, 20.0f, 0.0f));
        SetAngleZ(1, Math::PI/2.0f);

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        SetObjectRank(2, rank);
        SetObjectParent(2, 1);
        modelManager->AddModelReference("roller3c.mod", false, rank);
        SetPosition(2, Math::Vector(4.5f, 0.0f, 0.0f));
        SetAngleZ(2, 0.0f);

        CreateCrashSphere(Math::Vector(0.0f,  0.0f, 0.0f), 6.5f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(0.0f,  8.0f, 0.0f), 4.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(0.0f, 15.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(0.0f, 24.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
        SetGlobalSphere(Math::Vector(0.0f, 5.0f, 0.0f), 7.0f);

        m_character.posPower = Math::Vector(5.0f, 3.0f, 0.0f);

        CreateShadowCircle(6.0f, 1.0f);
        m_showLimitRadius = Gfx::LTNG_PROTECTION_RADIUS;
    }

    if ( m_type == OBJECT_NUCLEAR )
    {
        modelManager->AddModelReference("nuclear1.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);
        SetFloorHeight(0.0f);

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        SetObjectRank(1, rank);
        SetObjectParent(1, 0);
        modelManager->AddModelReference("nuclear2.mod", false, rank);
        SetPosition(1, Math::Vector(20.0f, 10.0f, 0.0f));
        SetAngleZ(1, 135.0f*Math::PI/180.0f);

        CreateCrashSphere(Math::Vector( 0.0f,  0.0f, 0.0f), 19.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( 0.0f, 24.0f, 0.0f), 15.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(22.0f,  1.0f, 0.0f),  1.5f, SOUND_BOUMm, 0.45f);
        SetGlobalSphere(Math::Vector(0.0f, 17.0f, 0.0f), 26.0f);

        m_character.posPower = Math::Vector(22.0f, 3.0f, 0.0f);

        CreateShadowCircle(21.0f, 1.0f);
    }

    if ( m_type == OBJECT_PARA )
    {
        modelManager->AddModelReference("para.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);
        SetFloorHeight(0.0f);

        m_terrain->AddBuildingLevel(pos, 16.0f, 18.0f, 1.0f, 0.5f);

        CreateCrashSphere(Math::Vector( 13.0f,  3.0f,  13.0f),  3.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( 11.0f, 15.0f,  11.0f),  2.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-13.0f,  3.0f,  13.0f),  3.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-11.0f, 15.0f, -11.0f),  2.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( 13.0f,  3.0f, -13.0f),  3.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( 11.0f, 15.0f, -11.0f),  2.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-13.0f,  3.0f, -13.0f),  3.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-11.0f, 15.0f, -11.0f),  2.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(  0.0f, 26.0f,   0.0f),  9.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(  0.0f, 54.0f,   0.0f), 14.0f, SOUND_BOUMm, 0.45f);
        SetGlobalSphere(Math::Vector(0.0f, 10.0f, 0.0f), 20.0f);

        CreateShadowCircle(21.0f, 1.0f);
        m_showLimitRadius = Gfx::LTNG_PROTECTION_RADIUS;
    }

    if ( m_type == OBJECT_SAFE )
    {
        modelManager->AddModelReference("safe1.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);
        SetFloorHeight(0.0f);

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        SetObjectRank(1, rank);
        SetObjectParent(1, 0);
        modelManager->AddModelReference("safe2.mod", false, rank);
        SetZoom(1, 1.05f);

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        SetObjectRank(2, rank);
        SetObjectParent(2, 0);
        modelManager->AddModelReference("safe3.mod", false, rank);
        SetZoom(2, 1.05f);

        m_terrain->AddBuildingLevel(pos, 18.0f, 20.0f, 1.0f, 0.5f);

        CreateCrashSphere(Math::Vector(0.0f, 1.0f, 0.0f), 13.0f, SOUND_BOUMm, 0.45f);
        SetGlobalSphere(Math::Vector(0.0f, 1.0f, 0.0f), 13.0f);

        CreateShadowCircle(23.0f, 1.0f);
    }

    if ( m_type == OBJECT_HUSTON )
    {
        modelManager->AddModelReference("huston1.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);
        SetFloorHeight(0.0f);

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        SetObjectRank(1, rank);
        SetObjectParent(1, 0);
        modelManager->AddModelReference("huston2.mod", false, rank);
        SetPosition(1, Math::Vector(0.0f, 39.0f, 30.0f));
        SetAngleY(1, -Math::PI/2.0f);
        SetZoom(1, 3.0f);

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        SetObjectRank(2, rank);
        SetObjectParent(2, 1);
        modelManager->AddModelReference("huston3.mod", false, rank);
        SetPosition(2, Math::Vector(0.0f, 4.5f, 1.9f));

        CreateCrashSphere(Math::Vector( 15.0f,  6.0f, -53.0f), 16.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-15.0f,  6.0f, -53.0f), 16.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( 15.0f,  6.0f, -26.0f), 16.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-15.0f,  6.0f, -26.0f), 16.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( 15.0f,  6.0f,   0.0f), 16.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-15.0f,  6.0f,   0.0f), 16.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( 15.0f,  6.0f,  26.0f), 16.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-15.0f,  6.0f,  26.0f), 16.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( 15.0f,  6.0f,  53.0f), 16.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-15.0f,  6.0f,  53.0f), 16.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(  0.0f, 27.0f,  30.0f), 12.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(  0.0f, 45.0f,  30.0f), 14.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( 26.0f,  4.0f, -61.0f),  5.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-26.0f,  4.0f, -61.0f),  5.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( 26.0f,  4.0f,  61.0f),  5.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-26.0f,  4.0f,  61.0f),  5.0f, SOUND_BOUMm, 0.45f);
    }

    if ( m_type == OBJECT_TARGET1 )
    {
        modelManager->AddModelReference("target1.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);
        SetZoom(0, 1.5f);
        SetFloorHeight(0.0f);

        CreateCrashSphere(Math::Vector(  0.0f, 50.0f+14.0f, 0.0f),  3.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( -7.0f, 50.0f+12.0f, 0.0f),  3.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(  7.0f, 50.0f+12.0f, 0.0f),  3.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-12.0f, 50.0f+ 7.0f, 0.0f),  3.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( 12.0f, 50.0f+ 7.0f, 0.0f),  3.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-14.0f, 50.0f+ 0.0f, 0.0f),  3.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( 14.0f, 50.0f+ 0.0f, 0.0f),  3.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-12.0f, 50.0f- 7.0f, 0.0f),  3.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( 12.0f, 50.0f- 7.0f, 0.0f),  3.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( -7.0f, 50.0f-12.0f, 0.0f),  3.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(  7.0f, 50.0f-12.0f, 0.0f),  3.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(  0.0f, 50.0f-14.0f, 0.0f),  3.0f, SOUND_BOUMm, 0.45f);

        CreateCrashSphere(Math::Vector(0.0f, 30.0f, 0.0f), 2.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(0.0f, 24.0f, 0.0f), 3.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(0.0f, 16.0f, 0.0f), 4.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(0.0f,  4.0f, 0.0f), 8.0f, SOUND_BOUMm, 0.45f);

        CreateShadowCircle(15.0f, 1.0f);
    }

    if ( m_type == OBJECT_TARGET2 )
    {
        modelManager->AddModelReference("target2.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);
        SetFloorHeight(0.0f);

        height += 50.0f*1.5f;
    }

    if ( m_type == OBJECT_NEST )
    {
        modelManager->AddModelReference("nest.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);
        SetFloorHeight(0.0f);

        m_terrain->AddBuildingLevel(pos, 3.0f, 5.0f, 1.0f, 0.5f);

        CreateShadowCircle(4.0f, 1.0f);
    }

    if ( m_type == OBJECT_START )
    {
        modelManager->AddModelReference("start.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);
        SetFloorHeight(0.0f);

        m_terrain->AddBuildingLevel(pos, 7.0f, 9.0f, 1.0f, 0.5f);
    }

    if ( m_type == OBJECT_END )
    {
        modelManager->AddModelReference("end.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);
        SetFloorHeight(0.0f);

        m_terrain->AddBuildingLevel(pos, 7.0f, 9.0f, 1.0f, 0.5f);
    }

#if 0
    if ( power > 0.0f )  // creates a battery?
    {
        CObject*    pPower;

        pPower = new CObject(m_iMan);
        pPower->SetType(power<=1.0f?OBJECT_POWER:OBJECT_ATOMIC);

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        pPower->SetObjectRank(0, rank);

        if ( power <= 1.0f )  modelManager->AddModelReference("power.mod", false, rank);
        else                  modelManager->AddModelReference("atomic.mod", false, rank);

        pPower->SetPosition(0, GetCharacter()->posPower);
        pPower->CreateCrashSphere(Math::Vector(0.0f, 1.0f, 0.0f), 1.0f, SOUND_BOUMm, 0.45f);
        pPower->SetGlobalSphere(Math::Vector(0.0f, 1.0f, 0.0f), 1.5f);

        pPower->SetTruck(this);
        SetPower(pPower);

        if ( power <= 1.0f )  pPower->SetEnergy(power);
        else                  pPower->SetEnergy(power/100.0f);
    }
#endif

    pos = GetPosition(0);
    pos.y += height;
    SetPosition(0, pos);  // to display the shadows immediately

    CreateOtherObject(type);
    m_engine->LoadAllTextures();

    return true;
}

// Creates a small resource set on the ground.

bool CObject::CreateResource(Math::Vector pos, float angle, ObjectType type,
                             float power)
{
    int         rank;
    float       radius, height;

    Gfx::CModelManager* modelManager = Gfx::CModelManager::GetInstancePointer();

    SetType(type);

    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);  // it is a stationary object
    SetObjectRank(0, rank);
    SetEnergy(power);

    std::string name;
    if ( type == OBJECT_STONE       )  name = "stone.mod";
    if ( type == OBJECT_URANIUM     )  name = "uranium.mod";
    if ( type == OBJECT_METAL       )  name = "metal.mod";
    if ( type == OBJECT_POWER       )  name = "power.mod";
    if ( type == OBJECT_ATOMIC      )  name = "atomic.mod";
    if ( type == OBJECT_BULLET      )  name = "bullet.mod";
    if ( type == OBJECT_BBOX        )  name = "bbox.mod";
    if ( type == OBJECT_KEYa        )  name = "keya.mod";
    if ( type == OBJECT_KEYb        )  name = "keyb.mod";
    if ( type == OBJECT_KEYc        )  name = "keyc.mod";
    if ( type == OBJECT_KEYd        )  name = "keyd.mod";
    if ( type == OBJECT_TNT         )  name = "tnt.mod";
    if ( type == OBJECT_SCRAP1      )  name = "scrap1.mod";
    if ( type == OBJECT_SCRAP2      )  name = "scrap2.mod";
    if ( type == OBJECT_SCRAP3      )  name = "scrap3.mod";
    if ( type == OBJECT_SCRAP4      )  name = "scrap4.mod";
    if ( type == OBJECT_SCRAP5      )  name = "scrap5.mod";
    if ( type == OBJECT_BOMB        )  name = "bomb.mod";
    if ( type == OBJECT_WAYPOINT    )  name = "waypoint.mod";
    if ( type == OBJECT_SHOW        )  name = "show.mod";
    if ( type == OBJECT_WINFIRE     )  name = "winfire.mod";
    if ( type == OBJECT_BAG         )  name = "bag.mod";
    if ( type == OBJECT_MARKSTONE   )  name = "cross1.mod";
    if ( type == OBJECT_MARKURANIUM )  name = "cross3.mod";
    if ( type == OBJECT_MARKPOWER   )  name = "cross2.mod";
    if ( type == OBJECT_MARKKEYa    )  name = "crossa.mod";
    if ( type == OBJECT_MARKKEYb    )  name = "crossb.mod";
    if ( type == OBJECT_MARKKEYc    )  name = "crossc.mod";
    if ( type == OBJECT_MARKKEYd    )  name = "crossd.mod";
    if ( type == OBJECT_EGG         )  name = "egg.mod";

    modelManager->AddModelReference(name, false, rank);

    SetPosition(0, pos);
    SetAngleY(0, angle);

    if ( type == OBJECT_SHOW )  // remains in the air?
    {
        return true;
    }

    radius = 1.5f;
    height = 0.0f;

    if ( type == OBJECT_MARKSTONE   ||
         type == OBJECT_MARKURANIUM ||
         type == OBJECT_MARKKEYa    ||
         type == OBJECT_MARKKEYb    ||
         type == OBJECT_MARKKEYc    ||
         type == OBJECT_MARKKEYd    ||
         type == OBJECT_MARKPOWER   ||
         type == OBJECT_WAYPOINT    )
    {
    }
    else if ( type == OBJECT_EGG )
    {
        CreateCrashSphere(Math::Vector(-1.0f, 2.8f, 0.0f), 3.0f, SOUND_BOUMm, 0.45f);
        SetGlobalSphere(Math::Vector(0.0f, 5.0f, 0.0f), 10.0f);
        radius = 3.0f;
    }
    else if ( type == OBJECT_BOMB )
    {
        CreateCrashSphere(Math::Vector(0.0f, 0.0f, 0.0f), 3.0f, SOUND_BOUMm, 0.45f);
        SetGlobalSphere(Math::Vector(0.0f, 0.0f, 0.0f), 3.0f);
        radius = 3.0f;
    }
    else if ( type == OBJECT_BAG )
    {
        CreateCrashSphere(Math::Vector(0.0f, 0.0f, 0.0f), 4.0f, SOUND_BOUMm, 0.45f);
        SetGlobalSphere(Math::Vector(0.0f, 0.0f, 0.0f), 4.0f);
        SetZoom(0, 1.5f);
        radius =  5.0f;
        height = -1.4f;
    }
    else
    {
        CreateCrashSphere(Math::Vector(0.0f, 1.0f, 0.0f), 1.0f, SOUND_BOUMm, 0.45f);
        SetGlobalSphere(Math::Vector(0.0f, 1.0f, 0.0f), 1.5f);
    }
    CreateShadowCircle(radius, 1.0f);

    SetFloorHeight(0.0f);
    CreateOtherObject(type);
    m_engine->LoadAllTextures();
    FloorAdjust();

    pos = GetPosition(0);
    pos.y += height;
    SetPosition(0, pos);  // to display the shadows immediately

    return true;
}

// Creates a flag placed on the ground.

bool CObject::CreateFlag(Math::Vector pos, float angle, ObjectType type)
{
    int         rank, i;

    Gfx::CModelManager* modelManager = Gfx::CModelManager::GetInstancePointer();

    SetType(type);

    std::string name;

    name = "";
    if ( type == OBJECT_FLAGb )  name = "flag1b.mod";
    if ( type == OBJECT_FLAGr )  name = "flag1r.mod";
    if ( type == OBJECT_FLAGg )  name = "flag1g.mod";
    if ( type == OBJECT_FLAGy )  name = "flag1y.mod";
    if ( type == OBJECT_FLAGv )  name = "flag1v.mod";

    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);  // it is a stationary object
    SetObjectRank(0, rank);
    modelManager->AddModelReference(name, false, rank);
    SetPosition(0, pos);
    SetAngleY(0, angle);

    name = "";
    if ( type == OBJECT_FLAGb )  name = "flag2b.mod";
    if ( type == OBJECT_FLAGr )  name = "flag2r.mod";
    if ( type == OBJECT_FLAGg )  name = "flag2g.mod";
    if ( type == OBJECT_FLAGy )  name = "flag2y.mod";
    if ( type == OBJECT_FLAGv )  name = "flag2v.mod";

    for ( i=0 ; i<4 ; i++ )
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        SetObjectRank(1+i, rank);
        SetObjectParent(1+i, i);
        modelManager->AddModelReference(name, false, rank);
        if ( i == 0 )  SetPosition(1+i, Math::Vector(0.15f, 5.0f, 0.0f));
        else           SetPosition(1+i, Math::Vector(0.79f, 0.0f, 0.0f));
    }

    SetJotlerSphere(Math::Vector(0.0f, 4.0f, 0.0f), 1.0f);
    CreateShadowCircle(2.0f, 0.3f);

    SetFloorHeight(0.0f);
    CreateOtherObject(type);
    m_engine->LoadAllTextures();
    FloorAdjust();

    pos = GetPosition(0);
    SetPosition(0, pos);  // to display the shadows immediately

    return true;
}

// Creates a barrier placed on the ground.

bool CObject::CreateBarrier(Math::Vector pos, float angle, float height,
                            ObjectType type)
{
    int         rank;

    Gfx::CModelManager* modelManager = Gfx::CModelManager::GetInstancePointer();

    SetType(type);

    if ( type == OBJECT_BARRIER0 )
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        SetObjectRank(0, rank);
        modelManager->AddModelReference("barrier0.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);

        CreateCrashSphere(Math::Vector( 3.5f, 3.0f, 0.0f), 0.7f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( 0.0f, 3.0f, 0.0f), 0.7f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-3.5f, 3.0f, 0.0f), 0.7f, SOUND_BOUMm, 0.45f);

        CreateShadowCircle(6.0f, 0.5f, Gfx::ENG_SHADOW_WORM);
    }

    if ( type == OBJECT_BARRIER1 )
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        SetObjectRank(0, rank);
        modelManager->AddModelReference("barrier1.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);

        CreateCrashSphere(Math::Vector( 8.5f, 3.0f, 0.0f), 0.7f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( 3.5f, 3.0f, 0.0f), 0.7f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( 0.0f, 3.0f, 0.0f), 0.7f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-3.5f, 3.0f, 0.0f), 0.7f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-8.5f, 3.0f, 0.0f), 0.7f, SOUND_BOUMm, 0.45f);

        CreateShadowCircle(12.0f, 0.5f, Gfx::ENG_SHADOW_WORM);
    }

    if ( type == OBJECT_BARRIER2 )  // cardboard?
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        SetObjectRank(0, rank);
        modelManager->AddModelReference("barrier2.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);

        CreateCrashSphere(Math::Vector( 8.5f, 3.0f, 0.0f), 0.7f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( 3.5f, 3.0f, 0.0f), 0.7f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( 0.0f, 3.0f, 0.0f), 0.7f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-3.5f, 3.0f, 0.0f), 0.7f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-8.5f, 3.0f, 0.0f), 0.7f, SOUND_BOUMm, 0.45f);

        CreateShadowCircle(12.0f, 0.8f, Gfx::ENG_SHADOW_WORM);
    }

    if ( type == OBJECT_BARRIER3 )  // match + straw?
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        SetObjectRank(0, rank);
        modelManager->AddModelReference("barrier3.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);

        CreateCrashSphere(Math::Vector( 8.5f, 3.0f, 0.0f), 0.7f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( 3.5f, 3.0f, 0.0f), 0.7f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( 0.0f, 3.0f, 0.0f), 0.7f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-3.5f, 3.0f, 0.0f), 0.7f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-8.5f, 3.0f, 0.0f), 0.7f, SOUND_BOUMm, 0.45f);

        CreateShadowCircle(10.0f, 0.5f, Gfx::ENG_SHADOW_WORM);
    }

    pos = GetPosition(0);
    SetPosition(0, pos);  // to display the shadows immediately

    SetFloorHeight(0.0f);
    CreateOtherObject(type);
    FloorAdjust();

    pos = GetPosition(0);
    pos.y += height;
    SetPosition(0, pos);

    return true;
}

// Creates a plant placed on the ground.

bool CObject::CreatePlant(Math::Vector pos, float angle, float height,
                          ObjectType type)
{
    int         rank;

    Gfx::CModelManager* modelManager = Gfx::CModelManager::GetInstancePointer();

    SetType(type);

    if ( type == OBJECT_PLANT0 ||
         type == OBJECT_PLANT1 ||
         type == OBJECT_PLANT2 ||
         type == OBJECT_PLANT3 ||
         type == OBJECT_PLANT4 )  // standard?
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        SetObjectRank(0, rank);
        if ( type == OBJECT_PLANT0 )  modelManager->AddModelReference("plant0.mod", false, rank);
        if ( type == OBJECT_PLANT1 )  modelManager->AddModelReference("plant1.mod", false, rank);
        if ( type == OBJECT_PLANT2 )  modelManager->AddModelReference("plant2.mod", false, rank);
        if ( type == OBJECT_PLANT3 )  modelManager->AddModelReference("plant3.mod", false, rank);
        if ( type == OBJECT_PLANT4 )  modelManager->AddModelReference("plant4.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);

        height -= 2.0f;

        CreateCrashSphere(Math::Vector(0.0f, 0.0f, 0.0f), 4.0f, SOUND_BOUM, 0.10f);
        SetGlobalSphere(Math::Vector(0.0f, 3.0f, 0.0f), 6.0f);
        SetJotlerSphere(Math::Vector(0.0f, 0.0f, 0.0f), 8.0f);

        CreateShadowCircle(8.0f, 0.5f);
    }

    if ( type == OBJECT_PLANT5 ||
         type == OBJECT_PLANT6 ||
         type == OBJECT_PLANT7 )  // clover?
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        SetObjectRank(0, rank);
        if ( type == OBJECT_PLANT5 )  modelManager->AddModelReference("plant5.mod", false, rank);
        if ( type == OBJECT_PLANT6 )  modelManager->AddModelReference("plant6.mod", false, rank);
        if ( type == OBJECT_PLANT7 )  modelManager->AddModelReference("plant7.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);

//?     CreateCrashSphere(Math::Vector(0.0f, 0.0f, 0.0f), 3.0f, SOUND_BOUM, 0.10f);
        SetJotlerSphere(Math::Vector(0.0f, 0.0f, 0.0f), 4.0f);

        CreateShadowCircle(5.0f, 0.3f);
    }

    if ( type == OBJECT_PLANT8 ||
         type == OBJECT_PLANT9 )  // squash?
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        SetObjectRank(0, rank);
        if ( type == OBJECT_PLANT8 )  modelManager->AddModelReference("plant8.mod", false, rank);
        if ( type == OBJECT_PLANT9 )  modelManager->AddModelReference("plant9.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);

        CreateCrashSphere(Math::Vector(0.0f,  2.0f, 0.0f), 4.0f, SOUND_BOUM, 0.10f);
        CreateCrashSphere(Math::Vector(0.0f, 10.0f, 0.0f), 4.0f, SOUND_BOUM, 0.10f);

        CreateShadowCircle(10.0f, 0.5f);
    }

    if ( type == OBJECT_PLANT10 ||
         type == OBJECT_PLANT11 ||
         type == OBJECT_PLANT12 ||
         type == OBJECT_PLANT13 ||
         type == OBJECT_PLANT14 )  // succulent?
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        SetObjectRank(0, rank);
        if ( type == OBJECT_PLANT10 )  modelManager->AddModelReference("plant10.mod", false, rank);
        if ( type == OBJECT_PLANT11 )  modelManager->AddModelReference("plant11.mod", false, rank);
        if ( type == OBJECT_PLANT12 )  modelManager->AddModelReference("plant12.mod", false, rank);
        if ( type == OBJECT_PLANT13 )  modelManager->AddModelReference("plant13.mod", false, rank);
        if ( type == OBJECT_PLANT14 )  modelManager->AddModelReference("plant14.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);

        CreateCrashSphere(Math::Vector(0.0f, 12.0f, 0.0f), 5.0f, SOUND_BOUM, 0.10f);
        SetGlobalSphere(Math::Vector(0.0f, 6.0f, 0.0f), 6.0f);
        SetJotlerSphere(Math::Vector(0.0f, 4.0f, 0.0f), 8.0f);

        CreateShadowCircle(8.0f, 0.3f);
    }

    if ( type == OBJECT_PLANT15 ||
         type == OBJECT_PLANT16 ||
         type == OBJECT_PLANT17 ||
         type == OBJECT_PLANT18 ||
         type == OBJECT_PLANT19 )  // fern?
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        SetObjectRank(0, rank);
        if ( type == OBJECT_PLANT15 )  modelManager->AddModelReference("plant15.mod", false, rank);
        if ( type == OBJECT_PLANT16 )  modelManager->AddModelReference("plant16.mod", false, rank);
        if ( type == OBJECT_PLANT17 )  modelManager->AddModelReference("plant17.mod", false, rank);
        if ( type == OBJECT_PLANT18 )  modelManager->AddModelReference("plant18.mod", false, rank);
        if ( type == OBJECT_PLANT19 )  modelManager->AddModelReference("plant19.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);

        if ( type != OBJECT_PLANT19 )
        {
            CreateCrashSphere(Math::Vector(0.0f, 0.0f, 0.0f), 4.0f, SOUND_BOUM, 0.10f);
            SetGlobalSphere(Math::Vector(0.0f, 3.0f, 0.0f), 6.0f);
        }
        SetJotlerSphere(Math::Vector(0.0f, 0.0f, 0.0f), 8.0f);

        CreateShadowCircle(8.0f, 0.5f);
    }

    if ( type == OBJECT_TREE0 )
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        SetObjectRank(0, rank);
        modelManager->AddModelReference("tree0.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);

        CreateCrashSphere(Math::Vector( 0.0f,  3.0f, 2.0f), 3.0f, SOUND_BOUMs, 0.20f);
        CreateCrashSphere(Math::Vector(-1.0f, 10.0f, 1.0f), 2.0f, SOUND_BOUMs, 0.20f);
        CreateCrashSphere(Math::Vector( 0.0f, 17.0f, 0.0f), 2.0f, SOUND_BOUMs, 0.20f);
        CreateCrashSphere(Math::Vector( 1.0f, 27.0f, 0.0f), 2.0f, SOUND_BOUMs, 0.20f);

        CreateShadowCircle(8.0f, 0.5f);
    }

    if ( type == OBJECT_TREE1 )
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        SetObjectRank(0, rank);
        modelManager->AddModelReference("tree1.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);

        CreateCrashSphere(Math::Vector( 0.0f,  3.0f, 2.0f), 3.0f, SOUND_BOUMs, 0.20f);
        CreateCrashSphere(Math::Vector(-2.0f, 11.0f, 1.0f), 2.0f, SOUND_BOUMs, 0.20f);
        CreateCrashSphere(Math::Vector(-2.0f, 19.0f, 2.0f), 2.0f, SOUND_BOUMs, 0.20f);
        CreateCrashSphere(Math::Vector( 2.0f, 26.0f, 0.0f), 2.0f, SOUND_BOUMs, 0.20f);
        CreateCrashSphere(Math::Vector( 2.0f, 34.0f,-2.0f), 2.0f, SOUND_BOUMs, 0.20f);

        CreateShadowCircle(8.0f, 0.5f);
    }

    if ( type == OBJECT_TREE2 )
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        SetObjectRank(0, rank);
        modelManager->AddModelReference("tree2.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);

        CreateCrashSphere(Math::Vector( 0.0f,  3.0f, 1.0f), 3.0f, SOUND_BOUMs, 0.20f);
        CreateCrashSphere(Math::Vector(-2.0f, 10.0f, 1.0f), 2.0f, SOUND_BOUMs, 0.20f);
        CreateCrashSphere(Math::Vector(-2.0f, 19.0f, 2.0f), 2.0f, SOUND_BOUMs, 0.20f);
        CreateCrashSphere(Math::Vector( 2.0f, 25.0f, 0.0f), 2.0f, SOUND_BOUMs, 0.20f);
        CreateCrashSphere(Math::Vector( 3.0f, 32.0f,-2.0f), 2.0f, SOUND_BOUMs, 0.20f);

        CreateShadowCircle(8.0f, 0.5f);
    }

    if ( type == OBJECT_TREE3 )
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        SetObjectRank(0, rank);
        modelManager->AddModelReference("tree3.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);

        CreateCrashSphere(Math::Vector(-2.0f,  3.0f, 2.0f), 3.0f, SOUND_BOUMs, 0.20f);
        CreateCrashSphere(Math::Vector(-3.0f,  9.0f, 1.0f), 2.0f, SOUND_BOUMs, 0.20f);
        CreateCrashSphere(Math::Vector( 0.0f, 18.0f, 0.0f), 2.0f, SOUND_BOUMs, 0.20f);
        CreateCrashSphere(Math::Vector( 0.0f, 27.0f, 7.0f), 2.0f, SOUND_BOUMs, 0.20f);

        CreateShadowCircle(8.0f, 0.5f);
    }

    if ( type == OBJECT_TREE4 )
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        SetObjectRank(0, rank);
        modelManager->AddModelReference("tree4.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);

        CreateCrashSphere(Math::Vector(0.0f, 10.0f, 0.0f), 10.0f, SOUND_BOUMs, 0.20f);
        CreateCrashSphere(Math::Vector(0.0f, 21.0f, 0.0f),  8.0f, SOUND_BOUMs, 0.20f);
        CreateCrashSphere(Math::Vector(0.0f, 32.0f, 0.0f),  7.0f, SOUND_BOUMs, 0.20f);

        CreateShadowCircle(8.0f, 0.5f);
    }

    if ( type == OBJECT_TREE5 )  // giant tree (for the world "teen")
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        SetObjectRank(0, rank);
        modelManager->AddModelReference("tree5.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);

        CreateCrashSphere(Math::Vector(  0.0f, 5.0f,-10.0f), 25.0f, SOUND_BOUMs, 0.20f);
        CreateCrashSphere(Math::Vector(-65.0f, 5.0f, 65.0f), 20.0f, SOUND_BOUMs, 0.20f);
        CreateCrashSphere(Math::Vector( 38.0f, 5.0f, 21.0f), 18.0f, SOUND_BOUMs, 0.20f);

        CreateShadowCircle(50.0f, 0.5f);
    }

    pos = GetPosition(0);
    SetPosition(0, pos);  // to display the shadows immediately

    SetFloorHeight(0.0f);
    CreateOtherObject(type);

    pos = GetPosition(0);
    pos.y += height;
    SetPosition(0, pos);

    return true;
}

// Creates a mushroom placed on the ground.

bool CObject::CreateMushroom(Math::Vector pos, float angle, float height,
                             ObjectType type)
{
    int         rank;

    Gfx::CModelManager* modelManager = Gfx::CModelManager::GetInstancePointer();

    SetType(type);

    if ( type == OBJECT_MUSHROOM1 )
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        SetObjectRank(0, rank);
        modelManager->AddModelReference("mush1.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);

        CreateCrashSphere(Math::Vector(0.0f, 4.0f, 0.0f), 3.0f, SOUND_BOUM, 0.10f);
        SetGlobalSphere(Math::Vector(0.0f, 3.0f, 0.0f), 5.5f);
        SetJotlerSphere(Math::Vector(0.0f, 3.0f, 0.0f), 5.5f);

        CreateShadowCircle(6.0f, 0.5f);
    }

    if ( type == OBJECT_MUSHROOM2 )
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        SetObjectRank(0, rank);
        modelManager->AddModelReference("mush2.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);

        CreateCrashSphere(Math::Vector(0.0f, 5.0f, 0.0f), 3.0f, SOUND_BOUM, 0.10f);
        SetGlobalSphere(Math::Vector(0.0f, 4.0f, 0.0f), 5.5f);
        SetJotlerSphere(Math::Vector(0.0f, 4.0f, 0.0f), 5.5f);

        CreateShadowCircle(5.0f, 0.5f);
    }

    pos = GetPosition(0);
    SetPosition(0, pos);  // to display the shadows immediately

    SetFloorHeight(0.0f);
    CreateOtherObject(type);

    pos = GetPosition(0);
    pos.y += height;
    SetPosition(0, pos);

    return true;
}

// Creates a toy placed on the ground.

bool CObject::CreateTeen(Math::Vector pos, float angle, float zoom, float height,
                         ObjectType type)
{
    Math::Matrix*       mat;
    Gfx::Color          color;
    int                 rank;
    float               fShadow;
    bool                bFloorAdjust = true;

    Gfx::CModelManager* modelManager = Gfx::CModelManager::GetInstancePointer();

    SetType(type);

    fShadow = Math::Norm(1.0f-height/10.0f);

    if ( type == OBJECT_TEEN0 )  // orange pencil lg=10
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        SetObjectRank(0, rank);
        modelManager->AddModelReference("teen0.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);
        SetZoom(0, zoom);

        CreateCrashSphere(Math::Vector( 5.0f, 1.0f, 0.0f), 1.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( 2.5f, 1.0f, 0.0f), 1.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( 0.0f, 1.0f, 0.0f), 1.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-2.5f, 1.0f, 0.0f), 1.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-5.0f, 1.0f, 0.0f), 1.0f, SOUND_BOUMm, 0.45f);

        CreateShadowCircle(5.0f, 0.8f*fShadow, Gfx::ENG_SHADOW_WORM);
    }

    if ( type == OBJECT_TEEN1 )  // blue pencil lg=14
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        SetObjectRank(0, rank);
        modelManager->AddModelReference("teen1.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);
        SetZoom(0, zoom);

        CreateCrashSphere(Math::Vector( 6.0f, 1.0f, 0.0f), 1.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( 4.0f, 1.0f, 0.0f), 1.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( 2.0f, 1.0f, 0.0f), 1.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( 0.0f, 1.0f, 0.0f), 1.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-2.0f, 1.0f, 0.0f), 1.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-4.0f, 1.0f, 0.0f), 1.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-6.0f, 1.0f, 0.0f), 1.0f, SOUND_BOUMm, 0.45f);

        CreateShadowCircle(6.0f, 0.8f*fShadow, Gfx::ENG_SHADOW_WORM);
    }

    if ( type == OBJECT_TEEN2 )  // red pencil lg=16
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        SetObjectRank(0, rank);
        modelManager->AddModelReference("teen2.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);
        SetZoom(0, zoom);

        CreateCrashSphere(Math::Vector( 7.0f, 1.0f, 0.0f), 1.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( 4.7f, 1.0f, 0.0f), 1.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( 2.3f, 1.0f, 0.0f), 1.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( 0.0f, 1.0f, 0.0f), 1.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-2.3f, 1.0f, 0.0f), 1.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-4.7f, 1.0f, 0.0f), 1.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-7.0f, 1.0f, 0.0f), 1.0f, SOUND_BOUMm, 0.45f);

        CreateShadowCircle(6.0f, 0.8f*fShadow, Gfx::ENG_SHADOW_WORM);
    }

    if ( type == OBJECT_TEEN3 )  // jar with pencils
    {
        rank = m_engine->CreateObject();
//?     m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_METAL);
        SetObjectRank(0, rank);
        modelManager->AddModelReference("teen3.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);
        SetZoom(0, zoom);

        CreateCrashSphere(Math::Vector( 0.0f, 4.0f, 0.0f), 4.0f, SOUND_BOUMm, 0.45f);
        SetGlobalSphere(Math::Vector(0.0f, 4.0f, 0.0f), 4.0f);
        CreateShadowCircle(6.0f, 0.5f*fShadow);
    }

    if ( type == OBJECT_TEEN4 )  // scissors
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        SetObjectRank(0, rank);
        modelManager->AddModelReference("teen4.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);
        SetZoom(0, zoom);

        CreateCrashSphere(Math::Vector(-9.0f, 1.0f, 0.0f), 1.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-6.0f, 1.0f, 0.0f), 1.1f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-3.0f, 1.0f, 0.0f), 1.2f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( 0.0f, 1.0f, 0.0f), 1.3f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( 5.1f, 1.0f,-1.3f), 2.6f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( 8.0f, 1.0f, 2.2f), 2.3f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( 9.4f, 1.0f,-2.0f), 2.0f, SOUND_BOUMm, 0.45f);

        CreateShadowCircle(10.0f, 0.5f*fShadow, Gfx::ENG_SHADOW_WORM);
    }

    if ( type == OBJECT_TEEN5 )  // CD
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        SetObjectRank(0, rank);
        modelManager->AddModelReference("teen5.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);
        SetZoom(0, zoom);
        SetFloorHeight(0.0f);
        bFloorAdjust = false;

        m_terrain->AddBuildingLevel(pos, 5.9f, 6.1f, 0.2f, 0.5f);
        CreateShadowCircle(8.0f, 0.2f*fShadow);
    }

    if ( type == OBJECT_TEEN6 )  // book 1
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        SetObjectRank(0, rank);
        modelManager->AddModelReference("teen6.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);
        SetZoom(0, zoom);

        CreateCrashSphere(Math::Vector(-5.0f, 3.0f, 7.5f), 5.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( 4.5f, 3.0f, 7.5f), 5.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-5.0f, 3.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( 4.5f, 3.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-5.0f, 3.0f,-7.5f), 5.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( 4.5f, 3.0f,-7.5f), 5.0f, SOUND_BOUMm, 0.45f);

        CreateShadowCircle(20.0f, 0.2f*fShadow);
    }

    if ( type == OBJECT_TEEN7 )  // book 2
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        SetObjectRank(0, rank);
        modelManager->AddModelReference("teen7.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);
        SetZoom(0, zoom);

        CreateCrashSphere(Math::Vector(-5.0f, 3.0f, 7.5f), 5.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( 4.5f, 3.0f, 7.5f), 5.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-5.0f, 3.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( 4.5f, 3.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-5.0f, 3.0f,-7.5f), 5.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( 4.5f, 3.0f,-7.5f), 5.0f, SOUND_BOUMm, 0.45f);

        CreateShadowCircle(20.0f, 0.2f*fShadow);
    }

    if ( type == OBJECT_TEEN8 )  // a stack of books 1
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        SetObjectRank(0, rank);
        modelManager->AddModelReference("teen8.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);
        SetZoom(0, zoom);

        CreateCrashSphere(Math::Vector(-5.0f, 3.0f, 7.5f), 5.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( 4.5f, 3.0f, 7.5f), 5.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-5.0f, 3.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( 4.5f, 3.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-5.0f, 3.0f,-7.5f), 5.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( 4.5f, 3.0f,-7.5f), 5.0f, SOUND_BOUMm, 0.45f);

        SetGlobalSphere(Math::Vector(0.0f, 10.0f, 0.0f), 12.0f);
        CreateShadowCircle(20.0f, 0.2f*fShadow);
    }

    if ( type == OBJECT_TEEN9 )  // a stack of books 2
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        SetObjectRank(0, rank);
        modelManager->AddModelReference("teen9.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);
        SetZoom(0, zoom);

        CreateCrashSphere(Math::Vector(-5.0f, 3.0f, 7.5f), 5.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( 4.5f, 3.0f, 7.5f), 5.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-5.0f, 3.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( 4.5f, 3.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-5.0f, 3.0f,-7.5f), 5.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( 4.5f, 3.0f,-7.5f), 5.0f, SOUND_BOUMm, 0.45f);

        SetGlobalSphere(Math::Vector(0.0f, 10.0f, 0.0f), 12.0f);
        CreateShadowCircle(20.0f, 0.2f*fShadow);
    }

    if ( type == OBJECT_TEEN10 )  // bookcase
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        SetObjectRank(0, rank);
        modelManager->AddModelReference("teen10.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);
        SetZoom(0, zoom);

        CreateCrashSphere(Math::Vector(-26.0f, 3.0f, 0.0f), 6.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-15.0f, 3.0f,-4.0f), 6.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-15.0f, 3.0f, 5.0f), 6.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( -4.0f, 3.0f,-4.0f), 6.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( -4.0f, 3.0f, 5.0f), 6.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(  6.0f, 3.0f,-4.0f), 6.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(  6.0f, 3.0f, 4.0f), 6.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( 14.0f, 3.0f,-3.0f), 6.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( 14.0f, 3.0f, 2.0f), 6.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( 24.0f, 3.0f, 5.0f), 6.0f, SOUND_BOUMm, 0.45f);

        SetGlobalSphere(Math::Vector(0.0f, 6.0f, 0.0f), 20.0f);
        CreateShadowCircle(40.0f, 0.2f*fShadow);
    }

    if ( type == OBJECT_TEEN11 )  // lamp
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        SetObjectRank(0, rank);
        modelManager->AddModelReference("teen11.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);
        SetFloorHeight(0.0f);
        SetZoom(0, zoom);

        mat = GetWorldMatrix(0);
        pos = Math::Transform(*mat, Math::Vector(-56.0f, 22.0f, 0.0f));
        m_particle->CreateParticle(pos, Math::Vector(0.0f, 0.0f, 0.0f), Math::Point(20.0f, 20.0f), Gfx::PARTISELY, 1.0f, 0.0f, 0.0f);

        pos = Math::Transform(*mat, Math::Vector(-65.0f, 40.0f, 0.0f));
        color.r = 4.0f;
        color.g = 2.0f;
        color.b = 0.0f;  // yellow-orange
        color.a = 0.0f;
        m_main->CreateSpot(pos, color);
    }

    if ( type == OBJECT_TEEN12 )  // coke
    {
        rank = m_engine->CreateObject();
//?     m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_METAL);
        SetObjectRank(0, rank);
        modelManager->AddModelReference("teen12.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);
        SetZoom(0, zoom);

        CreateCrashSphere(Math::Vector( 0.0f, 4.0f, 0.0f), 4.0f, SOUND_BOUMm, 0.45f);
        SetGlobalSphere(Math::Vector(0.0f, 9.0f, 0.0f), 5.0f);
        CreateShadowCircle(4.5f, 1.0f*fShadow);
    }

    if ( type == OBJECT_TEEN13 )  // cardboard farm
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        SetObjectRank(0, rank);
        modelManager->AddModelReference("teen13.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);
        SetZoom(0, zoom);

        CreateCrashSphere(Math::Vector(-10.0f, 4.0f,-7.0f), 5.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(  0.0f, 4.0f,-7.0f), 5.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( 10.0f, 4.0f,-7.0f), 5.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-10.0f, 4.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(  0.0f, 4.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( 10.0f, 4.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-10.0f, 4.0f, 7.0f), 5.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(  0.0f, 4.0f, 7.0f), 5.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( 10.0f, 4.0f, 7.0f), 5.0f, SOUND_BOUMm, 0.45f);

        SetGlobalSphere(Math::Vector(0.0f, 5.0f, 0.0f), 15.0f);
        CreateShadowCircle(20.0f, 1.0f*fShadow);
    }

    if ( type == OBJECT_TEEN14 )  // open box
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        SetObjectRank(0, rank);
        modelManager->AddModelReference("teen14.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);
        SetZoom(0, zoom);

        CreateCrashSphere(Math::Vector(-10.0f, 4.0f,-7.0f), 5.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(  0.0f, 4.0f,-7.0f), 5.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( 10.0f, 4.0f,-7.0f), 5.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-10.0f, 4.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(  0.0f, 4.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( 10.0f, 4.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-10.0f, 4.0f, 7.0f), 5.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(  0.0f, 4.0f, 7.0f), 5.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( 10.0f, 4.0f, 7.0f), 5.0f, SOUND_BOUMm, 0.45f);

        SetGlobalSphere(Math::Vector(0.0f, 5.0f, 0.0f), 15.0f);
        CreateShadowCircle(20.0f, 1.0f*fShadow);
    }

    if ( type == OBJECT_TEEN15 )  // stack of cartons
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        SetObjectRank(0, rank);
        modelManager->AddModelReference("teen15.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);
        SetZoom(0, zoom);

        CreateCrashSphere(Math::Vector(-10.0f, 4.0f,-7.0f), 5.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(  0.0f, 4.0f,-7.0f), 5.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( 10.0f, 4.0f,-7.0f), 5.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-10.0f, 4.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(  0.0f, 4.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( 10.0f, 4.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-10.0f, 4.0f, 7.0f), 5.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(  0.0f, 4.0f, 7.0f), 5.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( 10.0f, 4.0f, 7.0f), 5.0f, SOUND_BOUMm, 0.45f);

        SetGlobalSphere(Math::Vector(0.0f, 5.0f, 0.0f), 15.0f);
        CreateShadowCircle(20.0f, 1.0f*fShadow);
    }

    if ( type == OBJECT_TEEN16 )  // watering can
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        SetObjectRank(0, rank);
        modelManager->AddModelReference("teen16.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);
        SetZoom(0, zoom);

        CreateCrashSphere(Math::Vector(-8.0f, 4.0f, 0.0f), 12.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( 8.0f, 4.0f, 0.0f), 12.0f, SOUND_BOUMm, 0.45f);

        SetGlobalSphere(Math::Vector(0.0f, 13.0f, 0.0f), 20.0f);
        CreateShadowCircle(18.0f, 1.0f*fShadow);
    }

    if ( type == OBJECT_TEEN17 )  // wheel |
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        SetObjectRank(0, rank);
        modelManager->AddModelReference("teen17.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);
        SetZoom(0, zoom);

        CreateCrashSphere(Math::Vector( 0.0f, 31.0f, 0.0f), 31.0f, SOUND_BOUMm, 0.45f);
        SetGlobalSphere(Math::Vector(0.0f, 31.0f, 0.0f), 31.0f);
        CreateShadowCircle(24.0f, 0.5f*fShadow);
    }

    if ( type == OBJECT_TEEN18 )  // wheel /
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        SetObjectRank(0, rank);
        modelManager->AddModelReference("teen18.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);
        SetZoom(0, zoom);

        CreateCrashSphere(Math::Vector( 0.0f, 31.0f, 0.0f), 31.0f, SOUND_BOUMm, 0.45f);
        SetGlobalSphere(Math::Vector(0.0f, 31.0f, 0.0f), 31.0f);
        CreateShadowCircle(24.0f, 0.5f*fShadow);
    }

    if ( type == OBJECT_TEEN19 )  // wheel =
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        SetObjectRank(0, rank);
        modelManager->AddModelReference("teen19.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);
        SetZoom(0, zoom);

        CreateCrashSphere(Math::Vector( 0.0f, 10.0f, 0.0f), 32.0f, SOUND_BOUMm, 0.45f);
        SetGlobalSphere(Math::Vector(0.0f, 10.0f, 0.0f), 32.0f);
        CreateShadowCircle(33.0f, 1.0f*fShadow);
    }

    if ( type == OBJECT_TEEN20 )  // wall with shelf
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        SetObjectRank(0, rank);
        modelManager->AddModelReference("teen20.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);
        SetZoom(0, zoom);

        CreateCrashSphere(Math::Vector(-175.0f, 0.0f,  -5.0f), 4.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-175.0f, 0.0f, -35.0f), 4.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( -55.0f, 0.0f,  -5.0f), 4.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( -55.0f, 0.0f, -35.0f), 4.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( -37.0f, 0.0f,  -5.0f), 4.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( -37.0f, 0.0f, -35.0f), 4.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(  83.0f, 0.0f,  -5.0f), 4.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(  83.0f, 0.0f, -35.0f), 4.0f, SOUND_BOUMm, 0.45f);
    }

    if ( type == OBJECT_TEEN21 )  // wall with window
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        SetObjectRank(0, rank);
        modelManager->AddModelReference("teen21.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);
        SetZoom(0, zoom);
    }

    if ( type == OBJECT_TEEN22 )  // wall with door and shelf
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        SetObjectRank(0, rank);
        modelManager->AddModelReference("teen22.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);
        SetZoom(0, zoom);

        CreateCrashSphere(Math::Vector(-135.0f, 0.0f,  -5.0f), 4.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-135.0f, 0.0f, -35.0f), 4.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( -15.0f, 0.0f,  -5.0f), 4.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( -15.0f, 0.0f, -35.0f), 4.0f, SOUND_BOUMm, 0.45f);
    }

    if ( type == OBJECT_TEEN23 )  // skateboard on wheels
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        SetObjectRank(0, rank);
        modelManager->AddModelReference("teen23.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);
        SetZoom(0, zoom);

        if ( m_option == 1 )  // passage under the prohibited skateboard?
        {
            CreateCrashSphere(Math::Vector(-10.0f, 2.0f, 0.0f), 11.0f, SOUND_BOUMm, 0.45f);
            CreateCrashSphere(Math::Vector( 10.0f, 2.0f, 0.0f), 11.0f, SOUND_BOUMm, 0.45f);
        }

        CreateCrashSphere(Math::Vector(-23.0f, 2.0f, 7.0f), 3.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-23.0f, 2.0f, 0.0f), 3.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-23.0f, 2.0f,-7.0f), 3.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( 23.0f, 2.0f, 7.0f), 3.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( 23.0f, 2.0f, 0.0f), 3.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( 23.0f, 2.0f,-7.0f), 3.0f, SOUND_BOUMm, 0.45f);

        CreateShadowCircle(35.0f, 0.8f*fShadow, Gfx::ENG_SHADOW_WORM);
    }

    if ( type == OBJECT_TEEN24 )  // skate /
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        SetObjectRank(0, rank);
        modelManager->AddModelReference("teen24.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);
        SetZoom(0, zoom);

        CreateCrashSphere(Math::Vector(-12.0f, 0.0f, -3.0f), 3.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-12.0f, 0.0f,  3.0f), 3.0f, SOUND_BOUMm, 0.45f);
        CreateShadowCircle(20.0f, 0.2f*fShadow);
    }

    if ( type == OBJECT_TEEN25 )  // skate /
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        SetObjectRank(0, rank);
        modelManager->AddModelReference("teen25.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);
        SetZoom(0, zoom);

        CreateCrashSphere(Math::Vector(-12.0f, 0.0f, -3.0f), 3.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-12.0f, 0.0f,  3.0f), 3.0f, SOUND_BOUMm, 0.45f);
        CreateShadowCircle(20.0f, 0.2f*fShadow);
    }

    if ( type == OBJECT_TEEN26 )  // ceiling lamp
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        SetObjectRank(0, rank);
        modelManager->AddModelReference("teen26.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);
        SetZoom(0, zoom);
        SetFloorHeight(0.0f);

        mat = GetWorldMatrix(0);
        pos = Math::Transform(*mat, Math::Vector(0.0f, 50.0f, 0.0f));
        m_particle->CreateParticle(pos, Math::Vector(0.0f, 0.0f, 0.0f), Math::Point(100.0f, 100.0f), Gfx::PARTISELY, 1.0f, 0.0f, 0.0f);

        pos = Math::Transform(*mat, Math::Vector(0.0f, 50.0f, 0.0f));
        color.r = 4.0f;
        color.g = 2.0f;
        color.b = 0.0f;  // yellow-orange
        color.a = 0.0f;
        m_main->CreateSpot(pos, color);
    }

    if ( type == OBJECT_TEEN27 )  // large plant?
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        SetObjectRank(0, rank);
        modelManager->AddModelReference("teen27.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);
        SetZoom(0, zoom);

        CreateCrashSphere(Math::Vector(0.0f, 0.0f, 0.0f), 4.0f, SOUND_BOUM, 0.10f);
        CreateShadowCircle(40.0f, 0.5f);
    }

    if ( type == OBJECT_TEEN28 )  // bottle?
    {
        rank = m_engine->CreateObject();
//?     m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_METAL);
        SetObjectRank(0, rank);
        modelManager->AddModelReference("teen28.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);
        SetZoom(0, zoom);

        CreateCrashSphere(Math::Vector(0.0f, 2.0f, 0.0f), 5.0f, SOUND_BOUM, 0.10f);
        CreateShadowCircle(7.0f, 0.6f*fShadow);
    }

    if ( type == OBJECT_TEEN29 )  // bridge?
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        SetObjectRank(0, rank);
        modelManager->AddModelReference("teen29.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);
        SetZoom(0, zoom);
        bFloorAdjust = false;
    }

    if ( type == OBJECT_TEEN30 )  // jump?
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        SetObjectRank(0, rank);
        modelManager->AddModelReference("teen30.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);
        SetZoom(0, zoom);

        CreateCrashSphere(Math::Vector(0.0f, 4.0f, 0.0f), 15.0f, SOUND_BOUM, 0.10f);
        SetGlobalSphere(Math::Vector(0.0f, 15.0f, 0.0f), 17.0f);
        CreateShadowCircle(20.0f, 1.0f*fShadow);
    }

    if ( type == OBJECT_TEEN31 )  // basket?
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        SetObjectRank(0, rank);
        modelManager->AddModelReference("teen31.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);
        SetZoom(0, zoom);

        CreateCrashSphere(Math::Vector(-10.0f, 2.0f, 0.0f), 5.0f, SOUND_BOUM, 0.10f);
        CreateCrashSphere(Math::Vector(  0.0f, 2.0f, 0.0f), 6.0f, SOUND_BOUM, 0.10f);
        CreateCrashSphere(Math::Vector(  9.0f, 4.0f, 1.0f), 6.0f, SOUND_BOUM, 0.10f);

        SetGlobalSphere(Math::Vector(0.0f, 0.0f, 0.0f), 10.0f);
        CreateShadowCircle(16.0f, 0.6f*fShadow);
    }

    if ( type == OBJECT_TEEN32 )  // chair?
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        SetObjectRank(0, rank);
        modelManager->AddModelReference("teen32.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);
        SetZoom(0, zoom);

        CreateCrashSphere(Math::Vector( 17.5f, 1.0f,  17.5f), 3.5f, SOUND_BOUM, 0.10f);
        CreateCrashSphere(Math::Vector( 17.5f, 1.0f, -17.5f), 3.5f, SOUND_BOUM, 0.10f);
        CreateCrashSphere(Math::Vector(-17.5f, 1.0f,  17.5f), 3.5f, SOUND_BOUM, 0.10f);
        CreateCrashSphere(Math::Vector(-17.5f, 1.0f, -17.5f), 3.5f, SOUND_BOUM, 0.10f);
        SetGlobalSphere(Math::Vector(0.0f, 0.0f, 0.0f), 26.0f);
        CreateShadowCircle(35.0f, 0.3f*fShadow);
    }

    if ( type == OBJECT_TEEN33 )  // panel?
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        SetObjectRank(0, rank);
        modelManager->AddModelReference("teen33.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);
        SetZoom(0, zoom);

        CreateCrashSphere(Math::Vector(0.0f, 2.0f, 0.0f), 4.0f, SOUND_BOUM, 0.10f);
        CreateShadowCircle(10.0f, 0.3f*fShadow);
    }

    if ( type == OBJECT_TEEN34 )  // stone?
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        SetObjectRank(0, rank);
        modelManager->AddModelReference("teen34.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);
        SetZoom(0, zoom);

        CreateCrashSphere(Math::Vector(0.0f, 2.0f, 0.0f), 4.0f, SOUND_BOUM, 0.10f);
        CreateShadowCircle(3.0f, 1.0f*fShadow);
    }

    if ( type == OBJECT_TEEN35 )  // pipe?
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        SetObjectRank(0, rank);
        modelManager->AddModelReference("teen35.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);
        SetZoom(0, zoom);

        CreateCrashSphere(Math::Vector(-40.0f, 5.0f, 0.0f), 10.0f, SOUND_BOUM, 0.10f);
        CreateCrashSphere(Math::Vector(-20.0f, 5.0f, 0.0f), 10.0f, SOUND_BOUM, 0.10f);
        CreateCrashSphere(Math::Vector(  0.0f, 5.0f, 0.0f), 10.0f, SOUND_BOUM, 0.10f);
        CreateCrashSphere(Math::Vector( 20.0f, 5.0f, 0.0f), 10.0f, SOUND_BOUM, 0.10f);
        CreateCrashSphere(Math::Vector( 40.0f, 5.0f, 0.0f), 10.0f, SOUND_BOUM, 0.10f);
        CreateShadowCircle(40.0f, 0.8f*fShadow, Gfx::ENG_SHADOW_WORM);
    }

    if ( type == OBJECT_TEEN36 )  // trunk?
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        SetObjectRank(0, rank);
        modelManager->AddModelReference("teen36.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);
        SetZoom(0, zoom);
        bFloorAdjust = false;
    }

    if ( type == OBJECT_TEEN37 )  // boat?
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        SetObjectRank(0, rank);
        modelManager->AddModelReference("teen37.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);
        SetZoom(0, zoom);
        bFloorAdjust = false;
    }

    if ( type == OBJECT_TEEN38 )  // fan?
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        SetObjectRank(0, rank);
        modelManager->AddModelReference("teen38a.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);
        SetZoom(0, zoom);

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        SetObjectRank(1, rank);
        SetObjectParent(1, 0);
        modelManager->AddModelReference("teen38b.mod", false, rank);  // engine
        SetPosition(1, Math::Vector(0.0f, 30.0f, 0.0f));

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        SetObjectRank(2, rank);
        SetObjectParent(2, 1);
        modelManager->AddModelReference("teen38c.mod", false, rank);  // propeller
        SetPosition(2, Math::Vector(0.0f, 0.0f, 0.0f));

        CreateCrashSphere(Math::Vector(0.0f, 2.0f, 0.0f), 10.0f, SOUND_BOUM, 0.10f);
        SetGlobalSphere(Math::Vector(0.0f, 2.0f, 0.0f), 10.0f);
        CreateShadowCircle(15.0f, 0.5f*fShadow);
    }

    if ( type == OBJECT_TEEN39 )  // potted plant?
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        SetObjectRank(0, rank);
        modelManager->AddModelReference("teen39.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);
        SetZoom(0, zoom);

        CreateCrashSphere(Math::Vector(0.0f, 2.0f, 0.0f), 8.5f, SOUND_BOUM, 0.10f);
        SetGlobalSphere(Math::Vector(0.0f, 2.0f, 0.0f), 8.5f);
        CreateShadowCircle(10.0f, 1.0f*fShadow);
    }

    if ( type == OBJECT_TEEN40 )  // balloon?
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        SetObjectRank(0, rank);
        modelManager->AddModelReference("teen40.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);
        SetZoom(0, zoom);

        CreateCrashSphere(Math::Vector(0.0f, 5.0f, 0.0f), 11.0f, SOUND_BOUM, 0.10f);
        SetGlobalSphere(Math::Vector(0.0f, 14.0f, 0.0f), 15.0f);
        CreateShadowCircle(15.0f, 0.7f*fShadow);
    }

    if ( type == OBJECT_TEEN41 )  // fence?
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        SetObjectRank(0, rank);
        modelManager->AddModelReference("teen41.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);
        SetZoom(0, zoom);
    }

    if ( type == OBJECT_TEEN42 )  // clover?
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        SetObjectRank(0, rank);
        modelManager->AddModelReference("teen42.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);
        SetZoom(0, zoom);

        CreateCrashSphere(Math::Vector(0.0f, 2.0f, 0.0f), 2.0f, SOUND_BOUM, 0.10f);
        CreateShadowCircle(15.0f, 0.4f*fShadow);
    }

    if ( type == OBJECT_TEEN43 )  // clover?
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        SetObjectRank(0, rank);
        modelManager->AddModelReference("teen43.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);
        SetZoom(0, zoom);

        CreateCrashSphere(Math::Vector(0.0f, 2.0f, 0.0f), 2.0f, SOUND_BOUM, 0.10f);
        CreateShadowCircle(15.0f, 0.4f*fShadow);
    }

    if ( type == OBJECT_TEEN44 )  // car?
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        SetObjectRank(0, rank);
        modelManager->AddModelReference("teen44.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);
        SetZoom(0, zoom);

        CreateCrashSphere(Math::Vector(0.0f, 10.0f, 0.0f), 55.0f, SOUND_BOUM, 0.10f);
        SetGlobalSphere(Math::Vector(0.0f, 10.0f, 0.0f), 55.0f);
        CreateShadowCircle(55.0f, 1.0f*fShadow);
    }

    pos = GetPosition(0);
    SetPosition(0, pos);  // to display the shadows immediately

    if ( bFloorAdjust )
    {
        SetFloorHeight(0.0f);
        FloorAdjust();
    }

    CreateOtherObject(type);

    pos = GetPosition(0);
    pos.y += height;
    SetPosition(0, pos);

    return true;
}

// Creates a crystal placed on the ground.

bool CObject::CreateQuartz(Math::Vector pos, float angle, float height,
                           ObjectType type)
{
    float       radius;
    int         rank;

    Gfx::CModelManager* modelManager = Gfx::CModelManager::GetInstancePointer();

    SetType(type);

    if ( type == OBJECT_QUARTZ0 )
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_QUARTZ);
        SetObjectRank(0, rank);
        modelManager->AddModelReference("quartz0.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);

        CreateCrashSphere(Math::Vector(0.0f, 2.0f, 0.0f), 3.5f, SOUND_BOUMm, 0.45f);
        SetGlobalSphere(Math::Vector(0.0f, 2.0f, 0.0f), 3.5f);

        CreateShadowCircle(4.0f, 0.5f);
    }
    if ( type == OBJECT_QUARTZ1 )
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_QUARTZ);
        SetObjectRank(0, rank);
        modelManager->AddModelReference("quartz1.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);

        CreateCrashSphere(Math::Vector(0.0f, 4.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
        SetGlobalSphere(Math::Vector(0.0f, 4.0f, 0.0f), 5.0f);

        CreateShadowCircle(5.0f, 0.5f);
    }
    if ( type == OBJECT_QUARTZ2 )
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_QUARTZ);
        SetObjectRank(0, rank);
        modelManager->AddModelReference("quartz2.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);

        CreateCrashSphere(Math::Vector(0.0f, 6.0f, 0.0f), 6.0f, SOUND_BOUMm, 0.45f);
        SetGlobalSphere(Math::Vector(0.0f, 6.0f, 0.0f), 6.0f);

        CreateShadowCircle(6.0f, 0.5f);
    }
    if ( type == OBJECT_QUARTZ3 )
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_QUARTZ);
        SetObjectRank(0, rank);
        modelManager->AddModelReference("quartz3.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);

        CreateCrashSphere(Math::Vector(0.0f, 10.0f, 0.0f), 10.0f, SOUND_BOUMm, 0.45f);
        SetGlobalSphere(Math::Vector(0.0f, 10.0f, 0.0f), 10.0f);

        CreateShadowCircle(10.0f, 0.5f);
    }

    pos = GetPosition(0);
    SetPosition(0, pos);  // to display the shadows immediately

    SetFloorHeight(0.0f);
    CreateOtherObject(type);

    pos = GetPosition(0);
    pos.y += height;
    SetPosition(0, pos);

    if ( type == OBJECT_QUARTZ0 )
    {
        pos.y += 4.0f;
        radius = 2.0f;
    }
    if ( type == OBJECT_QUARTZ1 )
    {
        pos.y += 6.0f;
        radius = 4.0f;
    }
    if ( type == OBJECT_QUARTZ2 )
    {
        pos.y += 10.0f;
        radius = 5.0f;
    }
    if ( type == OBJECT_QUARTZ3 )
    {
        pos.y += 16.0f;
        radius = 8.0f;
    }
    m_particle->CreateParticle(pos, pos, Math::Point(2.0f, 2.0f), Gfx::PARTIQUARTZ, 0.7f+Math::Rand()*0.7f, radius, 0.0f);
    m_particle->CreateParticle(pos, pos, Math::Point(2.0f, 2.0f), Gfx::PARTIQUARTZ, 0.7f+Math::Rand()*0.7f, radius, 0.0f);

    return true;
}

// Creates a root placed on the ground.

bool CObject::CreateRoot(Math::Vector pos, float angle, float height,
                         ObjectType type)
{
    int         rank;

    Gfx::CModelManager* modelManager = Gfx::CModelManager::GetInstancePointer();

    SetType(type);

    if ( type == OBJECT_ROOT0 )
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        SetObjectRank(0, rank);
        modelManager->AddModelReference("root0.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);
        SetZoom(0, 2.0f);

        CreateCrashSphere(Math::Vector(-5.0f,  1.0f,  0.0f), 2.0f, SOUND_BOUMv, 0.15f);
        CreateCrashSphere(Math::Vector( 4.0f,  1.0f,  2.0f), 2.0f, SOUND_BOUMv, 0.15f);
        CreateCrashSphere(Math::Vector( 4.0f,  1.0f, -3.0f), 2.0f, SOUND_BOUMv, 0.15f);
        CreateCrashSphere(Math::Vector( 2.0f,  5.0f, -1.0f), 1.5f, SOUND_BOUMv, 0.15f);
        CreateCrashSphere(Math::Vector(-4.0f,  5.0f, -1.0f), 1.0f, SOUND_BOUMv, 0.15f);
        CreateCrashSphere(Math::Vector(-2.0f,  8.0f, -0.5f), 1.0f, SOUND_BOUMv, 0.15f);
        CreateCrashSphere(Math::Vector( 0.0f, 10.0f, -0.5f), 1.0f, SOUND_BOUMv, 0.15f);
//?     SetGlobalSphere(Math::Vector(0.0f, 6.0f, 0.0f), 11.0f);

        CreateShadowCircle(16.0f, 0.5f);
    }
    if ( type == OBJECT_ROOT1 )
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        SetObjectRank(0, rank);
        modelManager->AddModelReference("root1.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);
        SetZoom(0, 2.0f);

        CreateCrashSphere(Math::Vector(-4.0f,  1.0f,  1.0f), 2.0f, SOUND_BOUMv, 0.15f);
        CreateCrashSphere(Math::Vector( 0.0f,  1.0f,  2.0f), 1.5f, SOUND_BOUMv, 0.15f);
        CreateCrashSphere(Math::Vector( 3.0f,  1.0f, -2.0f), 2.0f, SOUND_BOUMv, 0.15f);
        CreateCrashSphere(Math::Vector(-2.0f,  5.0f,  1.0f), 1.0f, SOUND_BOUMv, 0.15f);
        CreateCrashSphere(Math::Vector( 2.0f,  5.0f,  0.0f), 1.0f, SOUND_BOUMv, 0.15f);
        CreateCrashSphere(Math::Vector( 0.0f,  8.0f,  1.0f), 1.0f, SOUND_BOUMv, 0.15f);
        CreateCrashSphere(Math::Vector( 0.0f, 12.0f,  1.0f), 1.0f, SOUND_BOUMv, 0.15f);
//?     SetGlobalSphere(Math::Vector(0.0f, 6.0f, 0.0f), 12.0f);

        CreateShadowCircle(16.0f, 0.5f);
    }
    if ( type == OBJECT_ROOT2 )
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        SetObjectRank(0, rank);
        modelManager->AddModelReference("root2.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);
        SetZoom(0, 2.0f);

        CreateCrashSphere(Math::Vector(-3.0f,  1.0f,  0.5f), 2.0f, SOUND_BOUMv, 0.15f);
        CreateCrashSphere(Math::Vector( 3.0f,  1.0f, -1.0f), 2.0f, SOUND_BOUMv, 0.15f);
        CreateCrashSphere(Math::Vector(-1.0f,  4.5f,  0.0f), 1.0f, SOUND_BOUMv, 0.15f);
        CreateCrashSphere(Math::Vector( 3.0f,  7.0f,  1.0f), 1.0f, SOUND_BOUMv, 0.15f);
        CreateCrashSphere(Math::Vector( 0.0f,  7.0f, -1.0f), 1.0f, SOUND_BOUMv, 0.15f);
        CreateCrashSphere(Math::Vector( 4.0f, 11.0f,  1.0f), 1.0f, SOUND_BOUMv, 0.15f);
//?     SetGlobalSphere(Math::Vector(0.0f, 6.0f, 0.0f), 10.0f);

        CreateShadowCircle(16.0f, 0.5f);
    }
    if ( type == OBJECT_ROOT3 )
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        SetObjectRank(0, rank);
        modelManager->AddModelReference("root3.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);
        SetZoom(0, 2.0f);

        CreateCrashSphere(Math::Vector(-4.0f,  1.0f,  1.0f), 3.0f, SOUND_BOUMv, 0.15f);
        CreateCrashSphere(Math::Vector( 4.0f,  1.0f, -3.0f), 3.0f, SOUND_BOUMv, 0.15f);
        CreateCrashSphere(Math::Vector( 6.0f,  1.0f,  4.0f), 3.0f, SOUND_BOUMv, 0.15f);
        CreateCrashSphere(Math::Vector(-2.5f,  7.0f,  2.0f), 2.0f, SOUND_BOUMv, 0.15f);
        CreateCrashSphere(Math::Vector( 4.0f,  7.0f,  2.0f), 2.0f, SOUND_BOUMv, 0.15f);
        CreateCrashSphere(Math::Vector( 3.0f,  6.0f, -1.0f), 1.0f, SOUND_BOUMv, 0.15f);
        CreateCrashSphere(Math::Vector( 0.0f, 12.0f,  0.0f), 2.0f, SOUND_BOUMv, 0.15f);
        CreateCrashSphere(Math::Vector( 1.0f, 16.0f,  0.0f), 1.0f, SOUND_BOUMv, 0.15f);
//?     SetGlobalSphere(Math::Vector(0.0f, 10.0f, 0.0f), 14.0f);

        CreateShadowCircle(22.0f, 0.5f);
    }
    if ( type == OBJECT_ROOT4 )
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        SetObjectRank(0, rank);
        modelManager->AddModelReference("root4.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);
        SetZoom(0, 2.0f);

        CreateCrashSphere(Math::Vector( -7.0f,  2.0f,  3.0f), 4.0f, SOUND_BOUMv, 0.15f);
        CreateCrashSphere(Math::Vector(  5.0f,  2.0f, -6.0f), 4.0f, SOUND_BOUMv, 0.15f);
        CreateCrashSphere(Math::Vector(  6.0f,  2.0f,  6.0f), 3.0f, SOUND_BOUMv, 0.15f);
        CreateCrashSphere(Math::Vector(-11.0f,  1.0f, -2.0f), 2.0f, SOUND_BOUMv, 0.15f);
        CreateCrashSphere(Math::Vector(  1.0f,  1.0f, -7.0f), 2.0f, SOUND_BOUMv, 0.15f);
        CreateCrashSphere(Math::Vector( -4.0f, 10.0f,  3.0f), 2.0f, SOUND_BOUMv, 0.15f);
        CreateCrashSphere(Math::Vector(  1.0f, 11.0f,  7.0f), 2.0f, SOUND_BOUMv, 0.15f);
        CreateCrashSphere(Math::Vector(  3.0f, 11.0f, -3.0f), 2.0f, SOUND_BOUMv, 0.15f);
        CreateCrashSphere(Math::Vector( -3.0f, 17.0f,  1.0f), 2.0f, SOUND_BOUMv, 0.15f);
        CreateCrashSphere(Math::Vector( -3.0f, 23.0f, -1.0f), 2.0f, SOUND_BOUMv, 0.15f);
//?     SetGlobalSphere(Math::Vector(0.0f, 12.0f, 0.0f), 20.0f);

        CreateShadowCircle(30.0f, 0.5f);
    }
    if ( type == OBJECT_ROOT5 )  // gravity root ?
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        SetObjectRank(0, rank);
        modelManager->AddModelReference("root4.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);
        SetZoom(0, 2.0f);

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        SetObjectRank(1, rank);
        SetObjectParent(1, 0);
        modelManager->AddModelReference("root5.mod", false, rank);
        SetPosition(1, Math::Vector(-5.0f, 28.0f, -4.0f));
        SetAngleX(1, -30.0f*Math::PI/180.0f);
        SetAngleZ(1,  20.0f*Math::PI/180.0f);

        CreateCrashSphere(Math::Vector( -7.0f,  2.0f,  3.0f), 4.0f, SOUND_BOUMv, 0.15f);
        CreateCrashSphere(Math::Vector(  5.0f,  2.0f, -6.0f), 4.0f, SOUND_BOUMv, 0.15f);
        CreateCrashSphere(Math::Vector(  6.0f,  2.0f,  6.0f), 3.0f, SOUND_BOUMv, 0.15f);
        CreateCrashSphere(Math::Vector(-11.0f,  1.0f, -2.0f), 2.0f, SOUND_BOUMv, 0.15f);
        CreateCrashSphere(Math::Vector(  1.0f,  1.0f, -7.0f), 2.0f, SOUND_BOUMv, 0.15f);
        CreateCrashSphere(Math::Vector( -4.0f, 10.0f,  3.0f), 2.0f, SOUND_BOUMv, 0.15f);
        CreateCrashSphere(Math::Vector(  1.0f, 11.0f,  7.0f), 2.0f, SOUND_BOUMv, 0.15f);
        CreateCrashSphere(Math::Vector(  3.0f, 11.0f, -3.0f), 2.0f, SOUND_BOUMv, 0.15f);
        CreateCrashSphere(Math::Vector( -3.0f, 17.0f,  1.0f), 2.0f, SOUND_BOUMv, 0.15f);
        CreateCrashSphere(Math::Vector( -3.0f, 23.0f, -1.0f), 2.0f, SOUND_BOUMv, 0.15f);
//?     SetGlobalSphere(Math::Vector(0.0f, 12.0f, 0.0f), 20.0f);

        CreateShadowCircle(30.0f, 0.5f);
    }

    pos = GetPosition(0);
    SetPosition(0, pos);  // to display the shadows immediately

    SetFloorHeight(0.0f);
    CreateOtherObject(type);

    pos = GetPosition(0);
    pos.y += height;
    SetPosition(0, pos);

    return true;
}

// Creates a small home.

bool CObject::CreateHome(Math::Vector pos, float angle, float height,
                         ObjectType type)
{
    int         rank;

    Gfx::CModelManager* modelManager = Gfx::CModelManager::GetInstancePointer();

    SetType(type);

    if ( type == OBJECT_HOME1 )
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        SetObjectRank(0, rank);
        modelManager->AddModelReference("home1.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);
        SetZoom(0, 1.3f);

        CreateCrashSphere(Math::Vector(0.0f, 5.0f, 0.0f), 10.0f, SOUND_BOUMs, 0.25f);
//?     SetGlobalSphere(Math::Vector(0.0f, 6.0f, 0.0f), 11.0f);
        CreateShadowCircle(16.0f, 0.5f);
    }

    pos = GetPosition(0);
    SetPosition(0, pos);  // to display the shadows immediately

    SetFloorHeight(0.0f);
    CreateOtherObject(type);

    pos = GetPosition(0);
    pos.y += height;
    SetPosition(0, pos);

    return true;
}

// Creates ruin placed on the ground.

bool CObject::CreateRuin(Math::Vector pos, float angle, float height,
                         ObjectType type)
{
    int         rank;

    Gfx::CModelManager* modelManager = Gfx::CModelManager::GetInstancePointer();

    SetType(type);

    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);  // it is a stationary object
    SetObjectRank(0, rank);

    std::string name;
    if ( type == OBJECT_RUINmobilew1 )  name = "ruin1.mod";
    if ( type == OBJECT_RUINmobilew2 )  name = "ruin1.mod";
    if ( type == OBJECT_RUINmobilet1 )  name = "ruin2.mod";
    if ( type == OBJECT_RUINmobilet2 )  name = "ruin2.mod";
    if ( type == OBJECT_RUINmobiler1 )  name = "ruin3.mod";
    if ( type == OBJECT_RUINmobiler2 )  name = "ruin3.mod";
    if ( type == OBJECT_RUINfactory  )  name = "ruin4.mod";
    if ( type == OBJECT_RUINdoor     )  name = "ruin5.mod";
    if ( type == OBJECT_RUINsupport  )  name = "ruin6.mod";
    if ( type == OBJECT_RUINradar    )  name = "ruin7.mod";
    if ( type == OBJECT_RUINconvert  )  name = "ruin8.mod";
    if ( type == OBJECT_RUINbase     )  name = "ruin9.mod";
    if ( type == OBJECT_RUINhead     )  name = "ruin10.mod";

    modelManager->AddModelReference(name, false, rank);

    SetPosition(0, pos);
    SetAngleY(0, angle);

    if ( type == OBJECT_RUINmobilew1 )  // vehicle had wheels?
    {
        // Creates the right-back wheel.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        SetObjectRank(6, rank);
        SetObjectParent(6, 0);

        modelManager->AddModelReference("ruin1w.mod", false, rank);

        SetPosition(6, Math::Vector(-3.0f, 1.8f, -4.0f));
        SetAngleX(6, -Math::PI/2.0f);

        // Creates the left-back wheel.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        SetObjectRank(7, rank);
        SetObjectParent(7, 0);

        modelManager->AddModelReference("ruin1w.mod", false, rank);

        SetPosition(7, Math::Vector(-3.0f, 1.0f, 3.0f));
        SetAngleY(7, Math::PI-0.3f);
        SetAngleX(7, -0.3f);

        // Creates the right-front wheel.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        SetObjectRank(8, rank);
        SetObjectParent(8, 0);

        modelManager->AddModelReference("ruin1w.mod", false, rank);

        SetPosition(8, Math::Vector(2.0f, 1.6f, -3.0f));
        SetAngleY(8, 0.3f);

        // Creates the left-front wheel.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        SetObjectRank(9, rank);
        SetObjectParent(9, 0);

        modelManager->AddModelReference("ruin1w.mod", false, rank);

        SetPosition(9, Math::Vector(2.0f, 1.0f, 3.0f));
        SetAngleY(9, Math::PI-0.2f);
        SetAngleX(9, 0.2f);

        CreateCrashSphere(Math::Vector(0.0f, 2.8f, 0.0f), 3.0f, SOUND_BOUMm, 0.45f);
//?     SetGlobalSphere(Math::Vector(0.0f, 5.0f, 0.0f), 10.0f);

        CreateShadowCircle(4.0f, 1.0f);
    }

    if ( type == OBJECT_RUINmobilew2 )  // vehicle has wheels?
    {
        // Creates the left-back wheel.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        SetObjectRank(7, rank);
        SetObjectParent(7, 0);

        modelManager->AddModelReference("ruin1w.mod", false, rank);

        SetPosition(7, Math::Vector(-3.0f, 1.0f, 3.0f));
        SetAngleY(7, Math::PI+0.3f);
        SetAngleX(7, 0.4f);

        // Creates the left-front wheel.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        SetObjectRank(9, rank);
        SetObjectParent(9, 0);

        modelManager->AddModelReference("ruin1w.mod", false, rank);

        SetPosition(9, Math::Vector(2.0f, 1.0f, 3.0f));
        SetAngleY(9, Math::PI+0.3f);
        SetAngleX(9, -0.3f);

        CreateCrashSphere(Math::Vector(0.0f, 2.8f, 0.0f), 3.0f, SOUND_BOUMm, 0.45f);
//?     SetGlobalSphere(Math::Vector(0.0f, 5.0f, 0.0f), 10.0f);

        CreateShadowCircle(4.0f, 1.0f);
    }

    if ( type == OBJECT_RUINmobilet1 )  // vehicle have caterpillars?
    {
        // Creates the cannon.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        SetObjectRank(1, rank);
        SetObjectParent(1, 0);

        modelManager->AddModelReference("ruin2c.mod", false, rank);

        SetPosition(1, Math::Vector(3.0f, 5.0f, -2.5f));
        SetAngleX(1, -Math::PI*0.85f);
        SetAngleY(1, -0.4f);
        SetAngleZ(1, -0.1f);

        CreateCrashSphere(Math::Vector(1.0f, 2.8f, -1.0f), 5.0f, SOUND_BOUMm, 0.45f);
//?     SetGlobalSphere(Math::Vector(1.0f, 5.0f, -1.0f), 10.0f);

        CreateShadowCircle(5.0f, 1.0f);
    }

    if ( type == OBJECT_RUINmobilet2 )  // vehicle have caterpillars?
    {
        CreateCrashSphere(Math::Vector(0.0f, 2.8f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
//?     SetGlobalSphere(Math::Vector(0.0f, 5.0f, 0.0f), 10.0f);

        CreateShadowCircle(5.0f, 1.0f);
    }

    if ( type == OBJECT_RUINmobiler1 )  // vehicle skating?
    {
        CreateCrashSphere(Math::Vector(1.0f, 2.8f, -1.0f), 5.0f, SOUND_BOUMm, 0.45f);
        SetGlobalSphere(Math::Vector(1.0f, 5.0f, -1.0f), 10.0f);

        CreateShadowCircle(5.0f, 1.0f);
    }

    if ( type == OBJECT_RUINmobiler2 )  // vehicle skating?
    {
        CreateCrashSphere(Math::Vector(0.0f, 1.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
        SetGlobalSphere(Math::Vector(0.0f, 5.0f, 0.0f), 10.0f);

        CreateShadowCircle(6.0f, 1.0f);
    }

    if ( type == OBJECT_RUINfactory )  // factory ?
    {
        CreateCrashSphere(Math::Vector(  9.0f,  1.0f, -11.0f), 5.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(  0.0f,  2.0f, -11.0f), 4.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-10.0f,  4.0f, -10.0f), 5.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-12.0f, 11.0f,  -4.0f), 3.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-10.0f,  4.0f,  -2.0f), 3.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-11.0f,  8.0f,   3.0f), 3.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-11.0f,  2.0f,   4.0f), 3.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-11.0f,  2.0f,  10.0f), 3.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( -4.0f,  0.0f,  10.0f), 3.0f, SOUND_BOUMm, 0.45f);
        SetGlobalSphere(Math::Vector(0.0f, 0.0f, 0.0f), 18.0f);

        CreateShadowCircle(20.0f, 0.7f);
    }

    if ( type == OBJECT_RUINdoor )  // converter holder?
    {
        CreateCrashSphere(Math::Vector(0.0f, 0.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
//?     SetGlobalSphere(Math::Vector(0.0f, 0.0f, 0.0f), 6.0f);

        CreateShadowCircle(6.0f, 1.0f);
    }

    if ( type == OBJECT_RUINsupport )  // radar holder?
    {
        CreateCrashSphere(Math::Vector(0.0f, 0.0f, 0.0f), 3.0f, SOUND_BOUMm, 0.45f);
//?     SetGlobalSphere(Math::Vector(0.0f, 0.0f, 0.0f), 4.0f);

        CreateShadowCircle(3.0f, 1.0f);
    }

    if ( type == OBJECT_RUINradar )  // radar base?
    {
        CreateCrashSphere(Math::Vector(0.0f, 0.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
//?     SetGlobalSphere(Math::Vector(0.0f, 0.0f, 0.0f), 6.0f);

        CreateShadowCircle(6.0f, 1.0f);
    }

    if ( type == OBJECT_RUINconvert )  // converter?
    {
        m_terrain->AddBuildingLevel(pos, 7.0f, 9.0f, 1.0f, 0.5f);

        CreateCrashSphere(Math::Vector(-10.0f,  0.0f,  4.0f), 5.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-10.0f,  0.0f, -4.0f), 5.0f, SOUND_BOUMm, 0.45f);
//?     SetGlobalSphere(Math::Vector(-3.0f, 0.0f, 0.0f), 14.0f);
    }

    if ( type == OBJECT_RUINbase )  // base?
    {
        CreateCrashSphere(Math::Vector(  0.0f, 15.0f,   0.0f),28.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( 17.0f,  6.0f,  42.0f), 6.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( 17.0f, 17.0f,  42.0f), 4.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-17.0f,  6.0f,  42.0f), 6.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-17.0f, 17.0f,  42.0f), 4.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-42.0f,  6.0f,  17.0f), 6.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-42.0f, 17.0f,  17.0f), 4.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-42.0f,  6.0f, -17.0f), 6.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-42.0f, 17.0f, -17.0f), 4.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-17.0f,  6.0f, -42.0f), 6.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-17.0f, 10.0f, -42.0f), 4.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( 15.0f, 13.0f, -34.0f), 3.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( 31.0f, 15.0f, -13.0f), 3.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( 21.0f,  8.0f, -39.0f), 5.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( 26.0f,  8.0f, -33.0f), 5.0f, SOUND_BOUMm, 0.45f);
        SetGlobalSphere(Math::Vector(0.0f, 0.0f, 0.0f), 48.0f);

        CreateShadowCircle(40.0f, 1.0f);
    }

    if ( type == OBJECT_RUINhead )  // base cap?
    {
        CreateCrashSphere(Math::Vector(  0.0f, 13.0f,   0.0f),20.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(  0.0f, -8.0f,   0.0f), 5.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(  0.0f,-16.0f,   0.0f), 3.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(  0.0f,-22.0f,   0.0f), 3.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-21.0f,  7.0f,   9.0f), 8.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( -9.0f,  7.0f,  21.0f), 8.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( 21.0f,  7.0f,   9.0f), 8.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(  9.0f,  7.0f,  21.0f), 8.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-21.0f,  7.0f,  -9.0f), 8.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( -9.0f,  7.0f, -21.0f), 8.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( 21.0f,  7.0f,  -9.0f), 8.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(  9.0f,  7.0f, -21.0f), 8.0f, SOUND_BOUMm, 0.45f);
        SetGlobalSphere(Math::Vector(0.0f, 0.0f, 0.0f), 35.0f);

        CreateShadowCircle(30.0f, 1.0f);
    }

    pos = GetPosition(0);
    SetPosition(0, pos);  //to display the shadows immediately

    SetFloorHeight(0.0f);
    CreateOtherObject(type);

    if ( type != OBJECT_RUINfactory &&
         type != OBJECT_RUINconvert &&
         type != OBJECT_RUINbase    )
    {
        FloorAdjust();
    }

    pos = GetPosition(0);
    pos.y += height;
    SetPosition(0, pos);  //to display the shadows immediately

    if ( type == OBJECT_RUINmobilew1 )
    {
        pos = GetPosition(0);
        pos.y -= 0.5f;
        SetPosition(0, pos);

        angle = GetAngleX(0)-0.1f;
        SetAngleX(0, angle);
    }

    if ( type == OBJECT_RUINmobilew2 )
    {
        pos = GetPosition(0);
        pos.y -= 1.5f;
        SetPosition(0, pos);

        angle = GetAngleX(0)-0.9f;
        SetAngleX(0, angle);

        angle = GetAngleZ(0)-0.1f;
        SetAngleZ(0, angle);
    }

    if ( type == OBJECT_RUINmobilet1 )
    {
        pos = GetPosition(0);
        pos.y -= 0.9f;
        SetPosition(0, pos);

        angle = GetAngleX(0)-0.3f;
        SetAngleX(0, angle);
    }

    if ( type == OBJECT_RUINmobilet2 )
    {
        pos = GetPosition(0);
        pos.y -= 1.5f;
        SetPosition(0, pos);

        angle = GetAngleX(0)-0.3f;
        SetAngleX(0, angle);

        angle = GetAngleZ(0)+0.8f;
        SetAngleZ(0, angle);
    }

    if ( type == OBJECT_RUINmobiler1 )
    {
        pos = GetPosition(0);
        pos.y += 4.0f;
        SetPosition(0, pos);

        angle = GetAngleX(0)-Math::PI*0.6f;
        SetAngleX(0, angle);

        angle = GetAngleZ(0)-0.2f;
        SetAngleZ(0, angle);
    }

    if ( type == OBJECT_RUINmobiler2 )
    {
        pos = GetPosition(0);
        pos.y += 2.0f;
        SetPosition(0, pos);

        angle = GetAngleX(0)-0.1f;
        SetAngleX(0, angle);

        angle = GetAngleZ(0)-0.3f;
        SetAngleZ(0, angle);
    }

    if ( type == OBJECT_RUINdoor )
    {
        pos = GetPosition(0);
        pos.y -= 0.5f;
        SetPosition(0, pos);

        angle = GetAngleZ(0)-0.1f;
        SetAngleZ(0, angle);
    }

    if ( type == OBJECT_RUINsupport )
    {
        pos = GetPosition(0);
        pos.y += 0.5f;
        SetPosition(0, pos);

//?     angle = GetAngleY(0)+0.1f;
//?     SetAngleY(0, angle);

        angle = GetAngleX(0)+0.1f;
        SetAngleX(0, angle);

        angle = GetAngleZ(0)+0.1f;
        SetAngleZ(0, angle);
    }

    if ( type == OBJECT_RUINradar )
    {
        pos = GetPosition(0);
        pos.y -= 0.5f;
        SetPosition(0, pos);

        angle = GetAngleX(0)+0.15f;
        SetAngleX(0, angle);

        angle = GetAngleZ(0)+0.1f;
        SetAngleZ(0, angle);
    }

    if ( type == OBJECT_RUINconvert )
    {
        pos = GetPosition(0);
        pos.y -= 1.0f;
        SetPosition(0, pos);
    }

    if ( type == OBJECT_RUINbase )
    {
        pos = GetPosition(0);
        pos.y -= 1.0f;
        SetPosition(0, pos);

        angle = GetAngleX(0)+0.15f;
        SetAngleX(0, angle);
    }

    if ( type == OBJECT_RUINhead )
    {
        pos = GetPosition(0);
        pos.y += 8.0f;
        SetPosition(0, pos);

        angle = GetAngleX(0)+Math::PI*0.4f;
        SetAngleX(0, angle);
    }

    return true;
}

// Creates a gadget apollo.

bool CObject::CreateApollo(Math::Vector pos, float angle, ObjectType type)
{
    int         rank, i;

    Gfx::CModelManager* modelManager = Gfx::CModelManager::GetInstancePointer();

    SetType(type);

    if ( type == OBJECT_APOLLO1 )  // LEM ?
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);  // it is a stationary object
        SetObjectRank(0, rank);
        modelManager->AddModelReference("apollol1.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);
        SetZoom(0, 1.2f);
        SetFloorHeight(0.0f);

        for ( i=0 ; i<4 ; i++ )  // creates feet
        {
            rank = m_engine->CreateObject();
            m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
            SetObjectRank(i+1, rank);
            SetObjectParent(i+1, 0);
            modelManager->AddModelReference("apollol2.mod", false, rank);
            SetAngleY(i+1, Math::PI/2.0f*i);
        }

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        SetObjectRank(5, rank);
        SetObjectParent(5, 0);
        modelManager->AddModelReference("apollol3.mod", false, rank);  // ladder

//?     m_terrain->AddBuildingLevel(pos, 10.0f, 13.0f, 12.0f, 0.0f);

        CreateCrashSphere(Math::Vector(  0.0f, 4.0f,   0.0f), 9.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( 11.0f, 5.0f,   0.0f), 3.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-11.0f, 5.0f,   0.0f), 3.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(  0.0f, 5.0f, -11.0f), 3.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(  0.0f, 5.0f,  11.0f), 3.0f, SOUND_BOUMm, 0.45f);

        SetGlobalSphere(Math::Vector(0.0f, 4.0f, 0.0f), 9.0f);

        CreateShadowCircle(16.0f, 0.5f);
    }

    if ( type == OBJECT_APOLLO2 )  // jeep
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);  //it is a stationary object
        SetObjectRank(0, rank);
        modelManager->AddModelReference("apolloj1.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);
        SetFloorHeight(0.0f);

        // Wheels.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        SetObjectRank(1, rank);
        SetObjectParent(1, 0);
        modelManager->AddModelReference("apolloj4.mod", false, rank);  // wheel
        SetPosition(1, Math::Vector(-5.75f, 1.65f, -5.0f));

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        SetObjectRank(2, rank);
        SetObjectParent(2, 0);
        modelManager->AddModelReference("apolloj4.mod", false, rank);  // wheel
        SetPosition(2, Math::Vector(-5.75f, 1.65f, 5.0f));

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        SetObjectRank(3, rank);
        SetObjectParent(3, 0);
        modelManager->AddModelReference("apolloj4.mod", false, rank);  // wheel
        SetPosition(3, Math::Vector(5.75f, 1.65f, -5.0f));

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        SetObjectRank(4, rank);
        SetObjectParent(4, 0);
        modelManager->AddModelReference("apolloj4.mod", false, rank);  // wheel
        SetPosition(4, Math::Vector(5.75f, 1.65f, 5.0f));

        // Accessories:
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        SetObjectRank(5, rank);
        SetObjectParent(5, 0);
        modelManager->AddModelReference("apolloj2.mod", false, rank);  // antenna
        SetPosition(5, Math::Vector(5.5f, 8.8f, 2.0f));
        SetAngleY(5, -120.0f*Math::PI/180.0f);
        SetAngleZ(5,   45.0f*Math::PI/180.0f);

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        SetObjectRank(6, rank);
        SetObjectParent(6, 0);
        modelManager->AddModelReference("apolloj3.mod", false, rank);  // camera
        SetPosition(6, Math::Vector(5.5f, 2.8f, -2.0f));
        SetAngleY(6, 30.0f*Math::PI/180.0f);

        CreateCrashSphere(Math::Vector( 3.0f, 2.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector(-3.0f, 2.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
        CreateCrashSphere(Math::Vector( 7.0f, 9.0f, 2.0f), 2.0f, SOUND_BOUMm, 0.20f);

        CreateShadowCircle(7.0f, 0.8f);

        FloorAdjust();
    }

    if ( type == OBJECT_APOLLO3 )  // flag?
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);  // it is a stationary object
        SetObjectRank(0, rank);
        modelManager->AddModelReference("apollof.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);
        SetFloorHeight(0.0f);

        SetJotlerSphere(Math::Vector(0.0f, 4.0f, 0.0f), 1.0f);
        CreateShadowCircle(2.0f, 0.3f);
    }

    if ( type == OBJECT_APOLLO4 )  // module?
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);  // it is a stationary object
        SetObjectRank(0, rank);
        modelManager->AddModelReference("apollom.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);
        SetFloorHeight(0.0f);

        CreateCrashSphere(Math::Vector(0.0f, 2.0f, 0.0f), 2.0f, SOUND_BOUMm, 0.45f);
        CreateShadowCircle(5.0f, 0.8f);

        FloorAdjust();
    }

    if ( type == OBJECT_APOLLO5 )  // antenna?
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);  // it is a stationary object
        SetObjectRank(0, rank);
        modelManager->AddModelReference("apolloa.mod", false, rank);
        SetPosition(0, pos);
        SetAngleY(0, angle);
        SetFloorHeight(0.0f);

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        SetObjectRank(1, rank);
        SetObjectParent(1, 0);
        modelManager->AddModelReference("apolloj2.mod", false, rank);  // antenna
        SetPosition(1, Math::Vector(0.0f, 5.0f, 0.0f));
        SetAngleY(1, -120.0f*Math::PI/180.0f);
        SetAngleZ(1,   45.0f*Math::PI/180.0f);

        CreateCrashSphere(Math::Vector(0.0f, 4.0f, 0.0f), 3.0f, SOUND_BOUMm, 0.35f);
        CreateShadowCircle(3.0f, 0.7f);
    }

    CreateOtherObject(type);

    pos = GetPosition(0);
    SetPosition(0, pos);  // to display the shadows immediately

    return true;
}

// Creates all sub-objects for managing the object.

void CObject::CreateOtherObject(ObjectType type)
{
    if ( type == OBJECT_BASE )
    {
        m_auto = new CAutoBase(m_iMan, this);
    }
    if ( type == OBJECT_PORTICO )
    {
        m_auto = new CAutoPortico(m_iMan, this);
    }
    if ( type == OBJECT_DERRICK )
    {
        m_auto = new CAutoDerrick(m_iMan, this);
    }
    if ( type == OBJECT_FACTORY )
    {
        m_auto = new CAutoFactory(m_iMan, this);
    }
    if ( type == OBJECT_REPAIR )
    {
        m_auto = new CAutoRepair(m_iMan, this);
    }
    if ( type == OBJECT_DESTROYER )
    {
        m_auto = new CAutoDestroyer(m_iMan, this);
    }
    if ( type == OBJECT_STATION )
    {
        m_auto = new CAutoStation(m_iMan, this);
    }
    if ( type == OBJECT_CONVERT )
    {
        m_auto = new CAutoConvert(m_iMan, this);
    }
    if ( type == OBJECT_TOWER )
    {
        m_auto = new CAutoTower(m_iMan, this);
    }
    if ( type == OBJECT_RESEARCH )
    {
        m_auto = new CAutoResearch(m_iMan, this);
    }
    if ( type == OBJECT_RADAR )
    {
        m_auto = new CAutoRadar(m_iMan, this);
    }
    if ( type == OBJECT_INFO )
    {
        m_auto = new CAutoInfo(m_iMan, this);
    }
    if ( type == OBJECT_ENERGY )
    {
        m_auto = new CAutoEnergy(m_iMan, this);
    }
    if ( type == OBJECT_LABO )
    {
        m_auto = new CAutoLabo(m_iMan, this);
    }
    if ( type == OBJECT_NUCLEAR )
    {
        m_auto = new CAutoNuclear(m_iMan, this);
    }
    if ( type == OBJECT_PARA )
    {
        m_auto = new CAutoPara(m_iMan, this);
    }
    if ( type == OBJECT_SAFE )
    {
        m_auto = new CAutoSafe(m_iMan, this);
    }
    if ( type == OBJECT_HUSTON )
    {
        m_auto = new CAutoHuston(m_iMan, this);
    }
    if ( type == OBJECT_EGG )
    {
        m_auto = new CAutoEgg(m_iMan, this);
    }
    if ( type == OBJECT_NEST )
    {
        m_auto = new CAutoNest(m_iMan, this);
    }
    if ( type == OBJECT_ROOT5 )
    {
        m_auto = new CAutoRoot(m_iMan, this);
    }
    if ( type == OBJECT_MUSHROOM2 )
    {
        m_auto = new CAutoMush(m_iMan, this);
    }
    if ( type == OBJECT_FLAGb ||
         type == OBJECT_FLAGr ||
         type == OBJECT_FLAGg ||
         type == OBJECT_FLAGy ||
         type == OBJECT_FLAGv )
    {
        m_auto = new CAutoFlag(m_iMan, this);
    }
    if ( type == OBJECT_TEEN36 ||  // trunk?
         type == OBJECT_TEEN37 ||  // boat?
         type == OBJECT_TEEN38 )   // fan?
    {
        m_auto = new CAutoKid(m_iMan, this);
    }
}


// Reads a program.

bool CObject::ReadProgram(int rank, const char* filename)
{
    if ( m_brain != 0 )
    {
        return m_brain->ReadProgram(rank, filename);
    }
    return false;
}

// Writes a program.

bool CObject::WriteProgram(int rank, char* filename)
{
    if ( m_brain != 0 )
    {
        return m_brain->WriteProgram(rank, filename);
    }
    return false;
}

// Starts a program.

bool CObject::RunProgram(int rank)
{
    if ( m_brain != 0 )
    {
        m_brain->RunProgram(rank);
        return true;
    }
    if ( m_auto != 0 )
    {
        m_auto->Start(rank);
        return true;
    }
    return false;
}




// Calculates the matrix for transforming the object.
// Returns true if the matrix has changed.
// The rotations occur in the order Y, Z and X.

bool CObject::UpdateTransformObject(int part, bool bForceUpdate)
{
    Math::Vector    position, angle, eye;
    bool        bModif = false;
    int         parent;

    if ( m_truck != 0 )  // transported by truck?
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
        angle    += m_cirVibration+m_inclinaison;
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

        if ( part == 0 && m_truck != 0 )  // transported by a truck?
        {
            Math::Matrix*   matWorldTruck;
            matWorldTruck = m_truck->GetWorldMatrix(m_truckLink);
            m_objectPart[part].matWorld = Math::MultiplyMatrices(*matWorldTruck,
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

bool CObject::UpdateTransformObject()
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

void CObject::FlatParent()
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

void CObject::UpdateEnergyMapping()
{
    Gfx::Material   mat;
    float           a, b, i, s, au, bu;
    float           limit[6];
    int             j;

    if ( fabs(m_energy-m_lastEnergy) < 0.01f )  return;
    m_lastEnergy = m_energy;

    memset(&mat, 0, sizeof(mat));
    mat.diffuse.r = 1.0f;
    mat.diffuse.g = 1.0f;
    mat.diffuse.b = 1.0f;  // white
    mat.ambient.r = 0.5f;
    mat.ambient.g = 0.5f;
    mat.ambient.b = 0.5f;

    if ( m_type == OBJECT_POWER  ||
         m_type == OBJECT_ATOMIC )
    {
        a = 2.0f;
        b = 0.0f;  // dimensions of the battery (according to y)
    }
    if ( m_type == OBJECT_STATION )
    {
        a = 10.0f;
        b =  4.0f;  // dimensions of the battery (according to y)
    }
    if ( m_type == OBJECT_ENERGY )
    {
        a = 9.0f;
        b = 3.0f;  // dimensions of the battery (according to y)
    }

    i = 0.50f+0.25f*m_energy;  // origin
    s = i+0.25f;  // width

    au = (s-i)/(b-a);
    bu = s-b*(s-i)/(b-a);

    limit[0] = 0.0f;
    limit[1] = m_engine->GetLimitLOD(0);
    limit[2] = limit[1];
    limit[3] = m_engine->GetLimitLOD(1);
    limit[4] = limit[3];
    limit[5] = 1000000.0f;

    for ( j=0 ; j<3 ; j++ )
    {
        m_engine->ChangeTextureMapping(m_objectPart[0].object,
                                       mat, Gfx::ENG_RSTATE_PART3, "lemt.png", "",
                                       limit[j*2+0], limit[j*2+1], Gfx::ENG_TEX_MAPPING_1Y,
                                       au, bu, 1.0f, 0.0f);
    }
}


// Manual action.

bool CObject::EventProcess(const Event &event)
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
                SetAngleZ(1, debug_arm1);
                SetAngleZ(2, debug_arm2);
                SetAngleZ(3, debug_arm3);
                char s[100];
                sprintf(s, "a=%.2f b=%.2f c=%.2f", debug_arm1*180.0f/Math::PI, debug_arm2*180.0f/Math::PI, debug_arm3*180.0f/Math::PI);
                m_engine->SetInfoText(5, s);
            }
        }
#endif
    }

    if ( m_physics != 0 )
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

    if ( m_auto != 0 )
    {
        m_auto->EventProcess(event);

        if ( event.type == EVENT_FRAME &&
             m_auto->IsEnded() != ERR_CONTINUE )
        {
            m_auto->DeleteObject();
            delete m_auto;
            m_auto = 0;
        }
    }

    if ( m_motion != 0 )
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

bool CObject::EventFrame(const Event &event)
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
        Gfx::CPyro*      pyro;
        Math::Vector    eye;
        float       dist;

        eye = m_engine->GetLookatPt();
        dist = Math::Distance(eye, GetPosition(0));
        if ( dist < m_proxyDistance )
        {
            m_bProxyActivate = false;
            m_main->CreateShortcuts();
            m_sound->Play(SOUND_FINDING);
            pyro = new Gfx::CPyro(m_iMan);
            pyro->Create(Gfx::PT_FINDING, this, 0.0f);
            m_displayText->DisplayError(INFO_FINDING, this);
        }
    }

    return true;
}

// Updates the mapping of the object.

void CObject::UpdateMapping()
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

void CObject::VirusFrame(float rTime)
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

        pos = GetPosition(0);
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

void CObject::PartiFrame(float rTime)
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

        SetPosition(i, pos);

        // Each song spins differently.
        switch( i%5 )
        {
            case 0:  factor = Math::Vector( 0.5f, 0.3f, 0.6f); break;
            case 1:  factor = Math::Vector(-0.3f, 0.4f,-0.2f); break;
            case 2:  factor = Math::Vector( 0.4f,-0.6f,-0.3f); break;
            case 3:  factor = Math::Vector(-0.6f,-0.2f, 0.0f); break;
            case 4:  factor = Math::Vector( 0.4f, 0.1f,-0.7f); break;
        }

        angle = GetAngle(i);
        angle += rTime*Math::PI*factor;
        SetAngle(i, angle);
    }
}


// Changes the perspective to view if it was like in the vehicle,
// or behind the vehicle.

void CObject::SetViewFromHere(Math::Vector &eye, float &dirH, float &dirV,
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
    if ( m_physics != 0 )
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

void CObject::SetCharacter(Character* character)
{
    memcpy(&m_character, character, sizeof(Character));
}

void CObject::GetCharacter(Character* character)
{
    memcpy(character, &m_character, sizeof(Character));
}

Character* CObject::GetCharacter()
{
    return &m_character;
}


// Returns the absolute time.

float CObject::GetAbsTime()
{
    return m_aTime;
}


// Management of energy contained in a battery.
// Single subject possesses the battery energy, but not the vehicle that carries the battery!

void CObject::SetEnergy(float level)
{
    if ( level < 0.0f )  level = 0.0f;
    if ( level > 1.0f )  level = 1.0f;
    m_energy = level;
}

float CObject::GetEnergy()
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

void CObject::SetCapacity(float capacity)
{
    m_capacity = capacity;
}

float CObject::GetCapacity()
{
    return m_capacity;
}


// Management of the shield.

void CObject::SetShield(float level)
{
    m_shield = level;
}

float CObject::GetShield()
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
         m_type == OBJECT_TEEN31    ||  // basket?
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

void CObject::SetRange(float delay)
{
    m_range = delay;
}

float CObject::GetRange()
{
    return m_range;
}


// Management of transparency of the object.

void CObject::SetTransparency(float value)
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

float CObject::GetTransparency()
{
    return m_transparency;
}


// Management of the object matter.

ObjectMaterial CObject::GetMaterial()
{
    if ( m_type == OBJECT_HUMAN )
    {
        return OM_HUMAN;
    }

    if ( m_type == OBJECT_SCRAP4 ||
         m_type == OBJECT_SCRAP5 )
    {
        return OM_HUMAN;
    }

    return OM_METAL;
}


// Indicates whether the gadget is a nonessential.

void CObject::SetGadget(bool bMode)
{
    m_bGadget = bMode;
}

bool CObject::GetGadget()
{
    return m_bGadget;
}


// Indicates whether an object is stationary (ant on the back).

void CObject::SetFixed(bool bFixed)
{
    m_bFixed = bFixed;
}

bool CObject::GetFixed()
{
    return m_bFixed;
}


// Indicates whether an object is subjected to clipping (obstacles).

void CObject::SetClip(bool bClip)
{
    m_bClip = bClip;
}

bool CObject::GetClip()
{
    return m_bClip;
}



// Pushes an object.

bool CObject::JostleObject(float force)
{
    CAutoJostle*    pa;

    if ( m_type == OBJECT_FLAGb ||
         m_type == OBJECT_FLAGr ||
         m_type == OBJECT_FLAGg ||
         m_type == OBJECT_FLAGy ||
         m_type == OBJECT_FLAGv )  // flag?
    {
        if ( m_auto == 0 )  return false;

        m_auto->Start(1);
    }
    else
    {
        if ( m_auto != 0 )  return false;

        m_auto = new CAutoJostle(m_iMan, this);
        pa = static_cast<CAutoJostle*>(m_auto);
        pa->Start(0, force);
    }

    return true;
}


// Beginning of the effect when the instruction "detect" is used.

void CObject::StartDetectEffect(CObject *target, bool bFound)
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
        goal = target->GetPosition(0);
        goal.y += 3.0f;
        goal = Math::SegmentPoint(pos, goal, Math::Distance(pos, goal)-3.0f);
    }

    dim.x = 3.0f;
    dim.y = dim.x;
    m_particle->CreateRay(pos, goal, Gfx::PARTIRAY2, dim, 0.2f);

    if ( target != 0 )
    {
        goal = target->GetPosition(0);
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

void CObject::SetVirusMode(bool bEnable)
{
    m_bVirusMode = bEnable;
    m_virusTime = 0.0f;

    if ( m_bVirusMode && m_brain != 0 )
    {
        if ( !m_brain->IntroduceVirus() )  // tries to infect
        {
            m_bVirusMode = false;  // program was not contaminated!
        }
    }
}

bool CObject::GetVirusMode()
{
    return m_bVirusMode;
}

float CObject::GetVirusTime()
{
    return m_virusTime;
}


// Management mode of the camera.

void CObject::SetCameraType(Gfx::CameraType type)
{
    m_cameraType = type;
}

Gfx::CameraType CObject::GetCameraType()
{
    return m_cameraType;
}

void CObject::SetCameraDist(float dist)
{
    m_cameraDist = dist;
}

float CObject::GetCameraDist()
{
    return m_cameraDist;
}

void CObject::SetCameraLock(bool bLock)
{
    m_bCameraLock = bLock;
}

bool CObject::GetCameraLock()
{
    return m_bCameraLock;
}



// Management of the demonstration of the object.

void CObject::SetHilite(bool bMode)
{
    int     list[OBJECTMAXPART+1];
    int     i, j;

    m_bHilite = bMode;

    if ( m_bHilite )
    {
        j = 0;
        for ( i=0 ; i<m_totalPart ; i++ )
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

bool CObject::GetHilite()
{
    return m_bHilite;
}


// Indicates whether the object is selected or not.

void CObject::SetSelect(bool bMode, bool bDisplayError)
{
    Error       err;

    m_bSelect = bMode;

    if ( m_physics != 0 )
    {
        m_physics->CreateInterface(m_bSelect);
    }

    if ( m_auto != 0 )
    {
        m_auto->CreateInterface(m_bSelect);
    }

    CreateSelectParticle();  // creates / removes particles

    if ( !m_bSelect )
    {
        SetGunGoalH(0.0f);  // puts the cannon right
        return;  // selects if not finished
    }

    err = ERR_OK;
    if ( m_physics != 0 )
    {
        err = m_physics->GetError();
    }
    if ( m_auto != 0 )
    {
        err = m_auto->GetError();
    }
    if ( err != ERR_OK && bDisplayError )
    {
        m_displayText->DisplayError(err, this);
    }
}

// Indicates whether the object is selected or not.

bool CObject::GetSelect(bool bReal)
{
    if ( !bReal && m_main->GetFixScene() )  return false;
    return m_bSelect;
}


// Indicates whether the object is selectable or not.

void CObject::SetSelectable(bool bMode)
{
    m_bSelectable = bMode;
}

// Indicates whether the object is selecionnable or not.

bool CObject::GetSelectable()
{
    return m_bSelectable;
}


// Management of the activities of an object.

void CObject::SetActivity(bool bMode)
{
    if ( m_brain != 0 )
    {
        m_brain->SetActivity(bMode);
    }
}

bool CObject::GetActivity()
{
    if ( m_brain != 0 )
    {
        return m_brain->GetActivity();
    }
    return false;
}


// Indicates if necessary to check the tokens of the object.

void CObject::SetCheckToken(bool bMode)
{
    m_bCheckToken = bMode;
}

// Indicates if necessary to check the tokens of the object.

bool CObject::GetCheckToken()
{
    return m_bCheckToken;
}


// Management of the visibility of an object.
// The object is not hidden or visually disabled, but ignores detections!
// For example: underground worm.

void CObject::SetVisible(bool bVisible)
{
    m_bVisible = bVisible;
}

bool CObject::GetVisible()
{
    return m_bVisible;
}


// Management mode of operation of an object.
// An inactive object is an object destroyed, nonexistent.
// This mode is used for objects "resetables"
// during training to simulate destruction.

void CObject::SetEnable(bool bEnable)
{
    m_bEnable = bEnable;
}

bool CObject::GetEnable()
{
    return m_bEnable;
}


// Management mode or an object is only active when you're close.

void CObject::SetProxyActivate(bool bActivate)
{
    m_bProxyActivate = bActivate;
}

bool CObject::GetProxyActivate()
{
    return m_bProxyActivate;
}

void CObject::SetProxyDistance(float distance)
{
    m_proxyDistance = distance;
}

float CObject::GetProxyDistance()
{
    return m_proxyDistance;
}


// Management of the method of increasing damage.

void CObject::SetMagnifyDamage(float factor)
{
    m_magnifyDamage = factor;
}

float CObject::GetMagnifyDamage()
{
    return m_magnifyDamage;
}


// Management of free parameter.

void CObject::SetParam(float value)
{
    m_param = value;
}

float CObject::GetParam()
{
    return m_param;
}


// Management of the mode "blocked" of an object.
// For example, a cube of titanium is blocked while it is used to make something,
//or a vehicle is blocked as its construction is not finished.

void CObject::SetLock(bool bLock)
{
    m_bLock = bLock;
}

bool CObject::GetLock()
{
    return m_bLock;
}

// Management of the mode "current explosion" of an object.
// An object in this mode is not saving.

void CObject::SetExplo(bool bExplo)
{
    m_bExplo = bExplo;
}

bool CObject::GetExplo()
{
    return m_bExplo;
}


// Mode management "cargo ship" during movies.

void CObject::SetCargo(bool bCargo)
{
    m_bCargo = bCargo;
}

bool CObject::GetCargo()
{
    return m_bCargo;
}


// Management of the HS mode of an object.

void CObject::SetBurn(bool bBurn)
{
    m_bBurn = bBurn;

//? if ( m_botVar != 0 )
//? {
//?     if ( m_bBurn )  m_botVar->SetUserPtr(OBJECTDELETED);
//?     else            m_botVar->SetUserPtr(this);
//? }
}

bool CObject::GetBurn()
{
    return m_bBurn;
}

void CObject::SetDead(bool bDead)
{
    m_bDead = bDead;

    if ( bDead && m_brain != 0 )
    {
        m_brain->StopProgram();  // stops the current task
    }

//? if ( m_botVar != 0 )
//? {
//?     if ( m_bDead )  m_botVar->SetUserPtr(OBJECTDELETED);
//?     else            m_botVar->SetUserPtr(this);
//? }
}

bool CObject::GetDead()
{
    return m_bDead;
}

bool CObject::GetRuin()
{
    return m_bBurn|m_bFlat;
}

bool CObject::GetActif()
{
    return !m_bLock && !m_bBurn && !m_bFlat && m_bVisible && m_bEnable;
}


// Management of the point of aim.

void CObject::SetGunGoalV(float gunGoal)
{
    if ( m_type == OBJECT_MOBILEfc ||
         m_type == OBJECT_MOBILEtc ||
         m_type == OBJECT_MOBILEwc ||
         m_type == OBJECT_MOBILEic )  // fireball?
    {
        if ( gunGoal >  10.0f*Math::PI/180.0f )  gunGoal =  10.0f*Math::PI/180.0f;
        if ( gunGoal < -20.0f*Math::PI/180.0f )  gunGoal = -20.0f*Math::PI/180.0f;
        SetAngleZ(1, gunGoal);
    }
    else if ( m_type == OBJECT_MOBILEfi ||
              m_type == OBJECT_MOBILEti ||
              m_type == OBJECT_MOBILEwi ||
              m_type == OBJECT_MOBILEii )  // orgaball?
    {
        if ( gunGoal >  20.0f*Math::PI/180.0f )  gunGoal =  20.0f*Math::PI/180.0f;
        if ( gunGoal < -20.0f*Math::PI/180.0f )  gunGoal = -20.0f*Math::PI/180.0f;
        SetAngleZ(1, gunGoal);
    }
    else if ( m_type == OBJECT_MOBILErc )  // phazer?
    {
        if ( gunGoal >  45.0f*Math::PI/180.0f )  gunGoal =  45.0f*Math::PI/180.0f;
        if ( gunGoal < -20.0f*Math::PI/180.0f )  gunGoal = -20.0f*Math::PI/180.0f;
        SetAngleZ(2, gunGoal);
    }
    else
    {
        gunGoal = 0.0f;
    }

    m_gunGoalV = gunGoal;
}

void CObject::SetGunGoalH(float gunGoal)
{
    if ( m_type == OBJECT_MOBILEfc ||
         m_type == OBJECT_MOBILEtc ||
         m_type == OBJECT_MOBILEwc ||
         m_type == OBJECT_MOBILEic )  // fireball?
    {
        if ( gunGoal >  40.0f*Math::PI/180.0f )  gunGoal =  40.0f*Math::PI/180.0f;
        if ( gunGoal < -40.0f*Math::PI/180.0f )  gunGoal = -40.0f*Math::PI/180.0f;
        SetAngleY(1, gunGoal);
    }
    else if ( m_type == OBJECT_MOBILEfi ||
              m_type == OBJECT_MOBILEti ||
              m_type == OBJECT_MOBILEwi ||
              m_type == OBJECT_MOBILEii )  // orgaball?
    {
        if ( gunGoal >  40.0f*Math::PI/180.0f )  gunGoal =  40.0f*Math::PI/180.0f;
        if ( gunGoal < -40.0f*Math::PI/180.0f )  gunGoal = -40.0f*Math::PI/180.0f;
        SetAngleY(1, gunGoal);
    }
    else if ( m_type == OBJECT_MOBILErc )  // phazer?
    {
        if ( gunGoal >  40.0f*Math::PI/180.0f )  gunGoal =  40.0f*Math::PI/180.0f;
        if ( gunGoal < -40.0f*Math::PI/180.0f )  gunGoal = -40.0f*Math::PI/180.0f;
        SetAngleY(2, gunGoal);
    }
    else
    {
        gunGoal = 0.0f;
    }

    m_gunGoalH = gunGoal;
}

float CObject::GetGunGoalV()
{
    return m_gunGoalV;
}

float CObject::GetGunGoalH()
{
    return m_gunGoalH;
}



// Shows the limits of the object.

bool CObject::StartShowLimit()
{
    if ( m_showLimitRadius == 0.0f )  return false;

    m_main->SetShowLimit(0, Gfx::PARTILIMIT1, this, GetPosition(0), m_showLimitRadius);
    m_bShowLimit = true;
    return true;
}

void CObject::StopShowLimit()
{
    m_bShowLimit = false;
}



// Indicates whether a program is under execution.

bool CObject::IsProgram()
{
    if ( m_brain == 0 )  return false;
    return m_brain->IsProgram();
}


// Creates or removes particles associated to the object.

void CObject::CreateSelectParticle()
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

    if ( m_bSelect || IsProgram() )
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

void CObject::UpdateSelectParticle()
{
    Math::Vector    pos[4];
    Math::Point     dim[4];
    float       zoom[4];
    float       angle;
    int         i;

    if ( !m_bSelect && !IsProgram() )  return;

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

    angle = GetAngleY(0)/Math::PI;

    zoom[0] = 1.0f;
    zoom[1] = 1.0f;
    zoom[2] = 1.0f;
    zoom[3] = 1.0f;

    if ( IsProgram() &&  // current program?
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

void CObject::SetRunScript(CScript* script)
{
    m_runScript = script;
}

CScript* CObject::GetRunScript()
{
    return m_runScript;
}

// Returns the variables of "this" for CBOT.

CBotVar* CObject::GetBotVar()
{
    return m_botVar;
}

// Returns the physics associated to the object.

CPhysics* CObject::GetPhysics()
{
    return m_physics;
}

// Returns the brain associated to the object.

CBrain* CObject::GetBrain()
{
    return m_brain;
}

// Returns the movement associated to the object.

CMotion* CObject::GetMotion()
{
    return m_motion;
}

// Returns the controller associated to the object.

CAuto* CObject::GetAuto()
{
    return m_auto;
}

void CObject::SetAuto(CAuto* automat)
{
    m_auto = automat;
}



// Management of the position in the file definition.

void CObject::SetDefRank(int rank)
{
    m_defRank = rank;
}

int  CObject::GetDefRank()
{
    return m_defRank;
}


// Getes the object name for the tooltip.

bool CObject::GetTooltipName(char* name)
{
    GetResource(RES_OBJECT, m_type, name);
    return ( name[0] != 0 );
}


// Adds the object previously selected in the list.

void CObject::AddDeselList(CObject* pObj)
{
    int     i;

    if ( m_totalDesectList >= OBJECTMAXDESELLIST )
    {
        for ( i=0 ; i<OBJECTMAXDESELLIST-1 ; i++ )
        {
            m_objectDeselectList[i] = m_objectDeselectList[i+1];
        }
        m_totalDesectList --;
    }

    m_objectDeselectList[m_totalDesectList++] = pObj;
}

// Removes the previously selected object in the list.

CObject* CObject::SubDeselList()
{
    if ( m_totalDesectList == 0 )  return 0;

    return m_objectDeselectList[--m_totalDesectList];
}

// Removes an object reference if it is in the list.

void CObject::DeleteDeselList(CObject* pObj)
{
    int     i, j;

    j = 0;
    for ( i=0 ; i<m_totalDesectList ; i++ )
    {
        if ( m_objectDeselectList[i] != pObj )
        {
            m_objectDeselectList[j++] = m_objectDeselectList[i];
        }
    }
    m_totalDesectList = j;
}



// Management of the state of the pencil drawing robot.

bool CObject::GetTraceDown()
{
    if (m_motion == nullptr) return false;
    CMotionVehicle* mv = dynamic_cast<CMotionVehicle*>(m_motion);
    if (mv == nullptr)
    {
        GetLogger()->Debug("GetTraceDown() invalid m_motion class!\n");
        return false;
    }
    return mv->GetTraceDown();
}

void CObject::SetTraceDown(bool bDown)
{
    if (m_motion == nullptr) return;
    CMotionVehicle* mv = dynamic_cast<CMotionVehicle*>(m_motion);
    if (mv == nullptr)
    {
        GetLogger()->Debug("SetTraceDown() invalid m_motion class!\n");
        return;
    }
    mv->SetTraceDown(bDown);
}

int CObject::GetTraceColor()
{
    if (m_motion == nullptr) return 0;
    CMotionVehicle* mv = dynamic_cast<CMotionVehicle*>(m_motion);
    if (mv == nullptr)
    {
        GetLogger()->Debug("GetTraceColor() invalid m_motion class!\n");
        return 0;
    }
    return mv->GetTraceColor();
}

void CObject::SetTraceColor(int color)
{
    if (m_motion == nullptr) return;
    CMotionVehicle* mv = dynamic_cast<CMotionVehicle*>(m_motion);
    if (mv == nullptr)
    {
        GetLogger()->Debug("SetTraceColor() invalid m_motion class!\n");
        return;
    }
    mv->SetTraceColor(color);
}

float CObject::GetTraceWidth()
{
    if (m_motion == nullptr) return 0.0f;
    CMotionVehicle* mv = dynamic_cast<CMotionVehicle*>(m_motion);
    if (mv == nullptr)
    {
        GetLogger()->Debug("GetTraceWidth() invalid m_motion class!\n");
        return 0.0f;
    }
    return mv->GetTraceWidth();
}

void CObject::SetTraceWidth(float width)
{
    if (m_motion == nullptr) return;
    CMotionVehicle* mv = dynamic_cast<CMotionVehicle*>(m_motion);
    if (mv == nullptr)
    {
        GetLogger()->Debug("SetTraceWidth() invalid m_motion class!\n");
        return;
    }
    mv->SetTraceWidth(width);
}


