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

#define STRICT
#define D3D_OVERLOADS

#include <windows.h>
#include <stdio.h>
#include <d3d.h>

#include "cbot/cbotdll.h"
#include "struct.h"
#include "D3DEngine.h"
#include "D3DMath.h"
#include "global.h"
#include "event.h"
#include "misc.h"
#include "iman.h"
#include "restext.h"
#include "math3d.h"
#include "mainmovie.h"
#include "robotmain.h"
#include "light.h"
#include "terrain.h"
#include "water.h"
#include "blitz.h"
#include "camera.h"
#include "particule.h"
#include "physics.h"
#include "brain.h"
#include "motion.h"
#include "motionhuman.h"
#include "motiontoto.h"
#include "motionvehicle.h"
#include "motionmother.h"
#include "motionant.h"
#include "motionspider.h"
#include "motionbee.h"
#include "motionworm.h"
#include "modfile.h"
#include "auto.h"
#include "autobase.h"
#include "autoportico.h"
#include "autoderrick.h"
#include "autofactory.h"
#include "autorepair.h"
#include "autodestroyer.h"
#include "autostation.h"
#include "autoenergy.h"
#include "autoconvert.h"
#include "autotower.h"
#include "autoresearch.h"
#include "autolabo.h"
#include "autonuclear.h"
#include "autoradar.h"
#include "autoegg.h"
#include "autonest.h"
#include "autoroot.h"
#include "autoflag.h"
#include "autoinfo.h"
#include "autojostle.h"
#include "autopara.h"
#include "autosafe.h"
#include "autohuston.h"
#include "automush.h"
#include "autokid.h"
#include "task.h"
#include "pyro.h"
#include "displaytext.h"
#include "cmdtoken.h"
#include "cbottoken.h"
#include "sound.h"
#include "object.h"



#define ADJUST_ONBOARD	FALSE		// TRUE -> ajuste la cam�ra ONBOARD
#define ADJUST_ARM		FALSE		// TRUE -> ajuste le bras manipulateur
#define VIRUS_DELAY		60.0f		// dur�e d'infection d'un virus
#define LOSS_SHIELD		0.24f		// perte du bouclier par coup
#define LOSS_SHIELD_H	0.10f		// perte du bouclier pour l'homme
#define LOSS_SHIELD_M	0.02f		// perte du bouclier pour la pondeuse

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




// Met � jour la classe Object.

void uObject(CBotVar* botThis, void* user)
{
	CObject*	object = (CObject*)user;
	CObject*	power;
	CObject*	fret;
	CPhysics*	physics;
	CBotVar		*pVar, *pSub;
	ObjectType	type;
	D3DVECTOR	pos;
	float		value;
	int			iValue;

	if ( object == 0 )  return;

	physics = object->RetPhysics();

	// Met � jour le type de l'objet.
	pVar = botThis->GivItemList();  // "category"
	type = object->RetType();
	pVar->SetValInt(type, object->RetName());

	// Met � jour la position de l'objet.
	pVar = pVar->GivNext();  // "position"
	if ( object->RetTruck() == 0 )
	{
		pos = object->RetPosition(0);
		pos.y -= object->RetWaterLevel();  // relatif au niveau de la mer !
		pSub = pVar->GivItemList();  // "x"
		pSub->SetValFloat(pos.x/g_unit);
		pSub = pSub->GivNext();  // "y"
		pSub->SetValFloat(pos.z/g_unit);
		pSub = pSub->GivNext();  // "z"
		pSub->SetValFloat(pos.y/g_unit);
	}
	else	// objet transport� ?
	{
		pSub = pVar->GivItemList();  // "x"
		pSub->SetInit(IS_NAN);
		pSub = pSub->GivNext();  // "y"
		pSub->SetInit(IS_NAN);
		pSub = pSub->GivNext();  // "z"
		pSub->SetInit(IS_NAN);
	}

	// Met � jour l'angle.
	pos = object->RetAngle(0);
	pos += object->RetInclinaison();
	pVar = pVar->GivNext();  // "orientation"
	pVar->SetValFloat(360.0f-Mod(pos.y*180.0f/PI, 360.0f));
	pVar = pVar->GivNext();  // "pitch"
	pVar->SetValFloat(pos.z*180.0f/PI);
	pVar = pVar->GivNext();  // "roll"
	pVar->SetValFloat(pos.x*180.0f/PI);

	// Met � jour le niveau d'�nergie de l'objet.
	pVar = pVar->GivNext();  // "energyLevel"
	value = object->RetEnergy();
	pVar->SetValFloat(value);

	// Met � jour le niveau du bouclier de l'objet.
	pVar = pVar->GivNext();  // "shieldLevel"
	value = object->RetShield();
	pVar->SetValFloat(value);

	// Met � jour la temp�rature du bouclier.
	pVar = pVar->GivNext();  // "temperature"
	if ( physics == 0 )  value = 0.0f;
	else                 value = 1.0f-physics->RetReactorRange();
	pVar->SetValFloat(value);

	// Met � jour la hauteur au-dessus du sol.
	pVar = pVar->GivNext();  // "altitude"
	if ( physics == 0 )  value = 0.0f;
	else                 value = physics->RetFloorHeight();
	pVar->SetValFloat(value/g_unit);

	// Met � jour le temps de l'objet.
	pVar = pVar->GivNext();  // "lifeTime"
	value = object->RetAbsTime();
	pVar->SetValFloat(value);

	// Met � jour la mati�re de l'objet.
	pVar = pVar->GivNext();  // "material"
	iValue = object->RetMaterial();
	pVar->SetValInt(iValue);

	// Met � jour le type de la pile.
	pVar = pVar->GivNext();  // "energyCell"
	power = object->RetPower();
	if ( power == 0 )  pVar->SetPointer(0);
	else               pVar->SetPointer(power->RetBotVar());

	// Met � jour le type de l'objet transport�.
	pVar = pVar->GivNext();  // "load"
	fret = object->RetFret();
	if ( fret == 0 )  pVar->SetPointer(0);
	else              pVar->SetPointer(fret->RetBotVar());
}




// Constructeur de l'objet.

CObject::CObject(CInstanceManager* iMan)
{
	int		i;

	m_iMan = iMan;
	m_iMan->AddInstance(CLASS_OBJECT, this, 500);

	m_engine      = (CD3DEngine*)m_iMan->SearchInstance(CLASS_ENGINE);
	m_light       = (CLight*)m_iMan->SearchInstance(CLASS_LIGHT);
	m_terrain     = (CTerrain*)m_iMan->SearchInstance(CLASS_TERRAIN);
	m_water       = (CWater*)m_iMan->SearchInstance(CLASS_WATER);
	m_particule   = (CParticule*)m_iMan->SearchInstance(CLASS_PARTICULE);
	m_camera      = (CCamera*)m_iMan->SearchInstance(CLASS_CAMERA);
	m_displayText = (CDisplayText*)m_iMan->SearchInstance(CLASS_DISPLAYTEXT);
	m_main        = (CRobotMain*)m_iMan->SearchInstance(CLASS_MAIN);
	m_sound       = (CSound*)m_iMan->SearchInstance(CLASS_SOUND);
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
	m_linVibration  = D3DVECTOR(0.0f, 0.0f, 0.0f);
	m_cirVibration  = D3DVECTOR(0.0f, 0.0f, 0.0f);
	m_inclinaison   = D3DVECTOR(0.0f, 0.0f, 0.0f);
	m_lastParticule = 0.0f;

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
	m_bHilite = FALSE;
	m_bSelect = FALSE;
	m_bSelectable = TRUE;
	m_bCheckToken = TRUE;
	m_bVisible = TRUE;
	m_bEnable = TRUE;
	m_bGadget = FALSE;
	m_bProxyActivate = FALSE;
	m_bTrainer = FALSE;
	m_bToy = FALSE;
	m_bManual = FALSE;
	m_bFixed = FALSE;
	m_bClip = TRUE;
	m_bShowLimit = FALSE;
	m_showLimitRadius = 0.0f;
	m_aTime = 0.0f;
	m_shotTime = 0.0f;
	m_bVirusMode = FALSE;
	m_virusTime = 0.0f;
	m_lastVirusParticule = 0.0f;
	m_totalDesectList = 0;
	m_bLock  = FALSE;
	m_bExplo = FALSE;
	m_bCargo = FALSE;
	m_bBurn  = FALSE;
	m_bDead  = FALSE;
	m_bFlat  = FALSE;
	m_gunGoalV = 0.0f;
	m_gunGoalH = 0.0f;
	m_shieldRadius = 0.0f;
	m_defRank = -1;
	m_magnifyDamage = 1.0f;
	m_proxyDistance = 60.0f;
	m_param = 0.0f;

	ZeroMemory(&m_character, sizeof(Character));
	m_character.wheelFront = 1.0f;
	m_character.wheelBack  = 1.0f;
	m_character.wheelLeft  = 1.0f;
	m_character.wheelRight = 1.0f;

	m_resetCap      = RESET_NONE;
	m_bResetBusy    = FALSE;
	m_resetPosition = D3DVECTOR(0.0f, 0.0f, 0.0f);
	m_resetAngle    = D3DVECTOR(0.0f, 0.0f, 0.0f);
	m_resetRun      = -1;

	m_cameraType = CAMERA_BACK;
	m_cameraDist = 50.0f;
	m_bCameraLock = FALSE;

	m_infoTotal = 0;
	m_infoReturn = NAN;
	m_bInfoUpdate = FALSE;

	for ( i=0 ; i<OBJECTMAXPART ; i++ )
	{
		m_objectPart[i].bUsed = FALSE;
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
	m_globalSpherePos = D3DVECTOR(0.0f, 0.0f, 0.0f);
	m_globalSphereRadius = 0.0f;
	m_jotlerSpherePos = D3DVECTOR(0.0f, 0.0f, 0.0f);
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

// Destructeur de l'objet.

CObject::~CObject()
{
	if ( m_botVar != 0 )
	{
		m_botVar->SetUserPtr(OBJECTDELETED);
		delete m_botVar;
	}

	delete m_physics;
	delete m_brain;
	delete m_motion;
	delete m_auto;

	m_iMan->DeleteInstance(CLASS_OBJECT, this);
}


// Supprime un objet.
// Si bAll=TRUE, on n'arrange rien, car tous les objets de la
// sc�ne sont d�truits rapidement !

void CObject::DeleteObject(BOOL bAll)
{
	CObject*	pObj;
	CPyro*		pPyro;
	int			i;

	if ( m_botVar != 0 )
	{
		m_botVar->SetUserPtr(OBJECTDELETED);
	}

	if ( m_camera->RetObject() == this )
	{
		m_camera->SetObject(0);
	}

	for ( i=0 ; i<1000000 ; i++ )
	{
		pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
		if ( pObj == 0 )  break;

		pObj->DeleteDeselList(this);
	}

	if ( !bAll )
	{
#if 0
		type = m_camera->RetType();
		if ( (type == CAMERA_BACK   ||
			  type == CAMERA_FIX    ||
			  type == CAMERA_EXPLO  ||
			  type == CAMERA_ONBOARD) &&
			 m_camera->RetObject() == this )
		{
			pObj = m_main->SearchNearest(RetPosition(0), this);
			if ( pObj == 0 )
			{
				m_camera->SetObject(0);
				m_camera->SetType(CAMERA_FREE);
			}
			else
			{
				m_camera->SetObject(pObj);
				m_camera->SetType(CAMERA_BACK);
			}
		}
#endif
		for ( i=0 ; i<1000000 ; i++ )
		{
			pPyro = (CPyro*)m_iMan->SearchInstance(CLASS_PYRO, i);
			if ( pPyro == 0 )  break;

			pPyro->CutObjectLink(this);  // l'objet n'existe plus
		}

		if ( m_bSelect )
		{
			SetSelect(FALSE);
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
			 m_type == OBJECT_END      )  // batiment?
		{
			m_terrain->DeleteBuildingLevel(RetPosition(0));  // applanit le terrain
		}
	}

	m_type = OBJECT_NULL;  // objet invalide jusqu'� destruction compl�te

	if ( m_partiReactor != -1 )
	{
		m_particule->DeleteParticule(m_partiReactor);
		m_partiReactor = -1;
	}

	if ( m_shadowLight != -1 )
	{
		m_light->DeleteLight(m_shadowLight);
		m_shadowLight = -1;
	}

	if ( m_effectLight != -1 )
	{
		m_light->DeleteLight(m_effectLight);
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
			m_objectPart[i].bUsed = FALSE;
			m_engine->DeleteObject(m_objectPart[i].object);

			if ( m_objectPart[i].masterParti != -1 )
			{
				m_particule->DeleteParticule(m_objectPart[i].masterParti);
				m_objectPart[i].masterParti = -1;
			}
		}
	}

	if ( m_bShowLimit )
	{
		m_main->FlushShowLimit(0);
		m_bShowLimit = FALSE;
	}

	if ( !bAll )  m_main->CreateShortcuts();
}

// Simplifie un objet (on lui �te le cerveau, entre autres).

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


// Fait exploser un objet, lorsqu'il est touch� par un projectile.
// Si FALSE est retourn�, l'objet n'est pas encore foutu.
// Si TRUE est retourn�, l'objet est d�truit.

BOOL CObject::ExploObject(ExploType type, float force, float decay)
{
	PyroType	pyroType;
	CPyro*		pyro;
	float		loss, shield;

	if ( type == EXPLO_BURN )
	{
		if ( m_type == OBJECT_MOBILEtg ||
			 m_type == OBJECT_TEEN28    ||  // bouteille ?
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
			 m_type == OBJECT_EGG      )  // objet qui ne br�le pas ?
		{
			type = EXPLO_BOUM;
			force = 1.0f;
			decay = 1.0f;
		}
	}

	if ( EXPLO_BOUM )
	{
		if ( m_shotTime < 0.5f )  return FALSE;
		m_shotTime = 0.0f;
	}

	if ( m_type == OBJECT_HUMAN && m_bDead )  return FALSE;

	// Calcule la puissance perdue par l'explosion.
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

	// Diminue la puissance du bouclier.
	shield = RetShield();
	shield -= loss;
	if ( shield < 0.0f )  shield = 0.0f;
	SetShield(shield);

	if ( shield > 0.0f )  // pas encore mort ?
	{
		if ( type == EXPLO_WATER )
		{
			if ( m_type == OBJECT_HUMAN )
			{
				pyroType = PT_SHOTH;
			}
			else
			{
				pyroType = PT_SHOTW;
			}
		}
		else
		{
			if ( m_type == OBJECT_HUMAN )
			{
				pyroType = PT_SHOTH;
			}
			else if ( m_type == OBJECT_MOTHER )
			{
				pyroType = PT_SHOTM;
			}
			else
			{
				pyroType = PT_SHOTT;
			}
		}
	}
	else	// compl�tement mort ?
	{
		if ( type == EXPLO_BURN )  // br�le ?
		{
			if ( m_type == OBJECT_MOTHER ||
				 m_type == OBJECT_ANT    ||
				 m_type == OBJECT_SPIDER ||
				 m_type == OBJECT_BEE    ||
				 m_type == OBJECT_WORM   ||
				 m_type == OBJECT_BULLET )
			{
				pyroType = PT_BURNO;
				SetBurn(TRUE);
			}
			else if ( m_type == OBJECT_HUMAN )
			{
				pyroType = PT_DEADG;
			}
			else
			{
				pyroType = PT_BURNT;
				SetBurn(TRUE);
			}
			SetVirusMode(FALSE);
		}
		else if ( type == EXPLO_WATER )
		{
			if ( m_type == OBJECT_HUMAN )
			{
				pyroType = PT_DEADW;
			}
			else
			{
				pyroType = PT_FRAGW;
			}
		}
		else	// explosion ?
		{
			if ( m_type == OBJECT_ANT    ||
				 m_type == OBJECT_SPIDER ||
				 m_type == OBJECT_BEE    ||
				 m_type == OBJECT_WORM   )
			{
				pyroType = PT_EXPLOO;
			}
			else if ( m_type == OBJECT_MOTHER ||
					  m_type == OBJECT_NEST   ||
					  m_type == OBJECT_BULLET )
			{
				pyroType = PT_FRAGO;
			}
			else if ( m_type == OBJECT_HUMAN )
			{
				pyroType = PT_DEADG;
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
					  m_type == OBJECT_END      )  // batiment ?
			{
				pyroType = PT_FRAGT;
			}
			else if ( m_type == OBJECT_MOBILEtg ||
					  m_type == OBJECT_TEEN28    ||  // bouteille ?
					  m_type == OBJECT_TEEN31    )   // basket ?
			{
				pyroType = PT_FRAGT;
			}
			else
			{
				pyroType = PT_EXPLOT;
			}
		}

		loss = 1.0f;
	}

	pyro = new CPyro(m_iMan);
	pyro->Create(pyroType, this, loss);

	if ( shield == 0.0f )  // mort ?
	{
		if ( m_brain != 0 )
		{
			m_brain->StopProgram();
		}
		m_main->SaveOneScript(this);
	}

	if ( shield > 0.0f )  return FALSE;  // pas encore mort

	if ( RetSelect() )
	{
		SetSelect(FALSE);  // d�s�lectionne l'objet
		m_camera->SetType(CAMERA_EXPLO);
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

	return TRUE;
}

// (*)	Si un robot ou le cosmonaute meurt, l'objet doit continuer
//		d'exister, pour que les programmes des fourmis continuent
//		de fonctionner comme si de rien �tait !


// Initialise une nouvelle partie.

void CObject::InitPart(int part)
{
	m_objectPart[part].bUsed      = TRUE;
	m_objectPart[part].object     = -1;
	m_objectPart[part].parentPart = -1;

	m_objectPart[part].position   = D3DVECTOR(0.0f, 0.0f, 0.0f);
	m_objectPart[part].angle.y    = 0.0f;
	m_objectPart[part].angle.x    = 0.0f;
	m_objectPart[part].angle.z    = 0.0f;
	m_objectPart[part].zoom       = D3DVECTOR(1.0f, 1.0f, 1.0f);

	m_objectPart[part].bTranslate = TRUE;
	m_objectPart[part].bRotate    = TRUE;
	m_objectPart[part].bZoom      = FALSE;

	D3DUtil_SetIdentityMatrix(m_objectPart[part].matTranslate);
	D3DUtil_SetIdentityMatrix(m_objectPart[part].matRotate);
	D3DUtil_SetIdentityMatrix(m_objectPart[part].matTransform);
	D3DUtil_SetIdentityMatrix(m_objectPart[part].matWorld);

	m_objectPart[part].masterParti = -1;
}

// Cr�e une nouvelle partie, et retourne son num�ro.
// Retourne -1 en cas d'erreur.

int CObject::CreatePart()
{
	int		i;

	for ( i=0 ; i<OBJECTMAXPART ; i++ )
	{
		if ( m_objectPart[i].bUsed )  continue;

		InitPart(i);
		UpdateTotalPart();
		return i;
	}
	return -1;
}

// Supprime une partie.

void CObject::DeletePart(int part)
{
	if ( !m_objectPart[part].bUsed )  return;

	if ( m_objectPart[part].masterParti != -1 )
	{
		m_particule->DeleteParticule(m_objectPart[part].masterParti);
		m_objectPart[part].masterParti = -1;
	}

	m_objectPart[part].bUsed = FALSE;
	m_engine->DeleteObject(m_objectPart[part].object);
	UpdateTotalPart();
}

void CObject::UpdateTotalPart()
{
	int		i;

	m_totalPart = 0;
	for ( i=0 ; i<OBJECTMAXPART ; i++ )
	{
		if ( m_objectPart[i].bUsed )
		{
			m_totalPart = i+1;
		}
	}
}


// Sp�cifie le num�ro de l'objet d'une partie.

void CObject::SetObjectRank(int part, int objRank)
{
	if ( !m_objectPart[part].bUsed )  // objet pas cr�� ?
	{
		InitPart(part);
		UpdateTotalPart();
	}
	m_objectPart[part].object = objRank;
}

// Retourne le num�ro d'une partie.

int CObject::RetObjectRank(int part)
{
	if ( !m_objectPart[part].bUsed )  return -1;
	return m_objectPart[part].object;
}

// Sp�cifie quel est le parent d'ue partie.
// Rappel: la partie 0 est toujours le p�re de tous et donc la
// partie principale (par exemple le chassis d'une voiture).

void CObject::SetObjectParent(int part, int parent)
{
	m_objectPart[part].parentPart = parent;
}


// Sp�cifie le type de l'objet.

void CObject::SetType(ObjectType type)
{
	m_type = type;
	strcpy(m_name, RetObjectName(m_type));

	if ( m_type == OBJECT_MOBILErs )
	{
		m_param = 1.0f;  // bouclier au maximum par d�faut
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
		 m_type == OBJECT_MOBILErc )  // v�hicule canon ?
	{
		m_cameraType = CAMERA_ONBOARD;
	}
}

ObjectType CObject::RetType()
{
	return m_type;
}

char* CObject::RetName()
{
	return m_name;
}


// Choix de l'option � utiliser.

void CObject::SetOption(int option)
{
	m_option = option;
}

int CObject::RetOption()
{
	return m_option;
}


// Gestion de l'identificateur unique d'un objet.

void CObject::SetID(int id)
{
	m_id = id;

	if ( m_botVar != 0 )
	{
		m_botVar->SetIdent(m_id);
	}
}

int CObject::RetID()
{
	return m_id;
}


// Sauve tous les param�tres de l'objet.

BOOL CObject::Write(char *line)
{
	D3DVECTOR	pos;
	Info		info;
	char		name[100];
	float		value;
	int			i;

	sprintf(name, " camera=%s", GetCamera(RetCameraType()));
	strcat(line, name);

	if ( RetCameraLock() != 0 )
	{
		sprintf(name, " cameraLock=%d", RetCameraLock());
		strcat(line, name);
	}

	if ( RetEnergy() != 0.0f )
	{
		sprintf(name, " energy=%.2f", RetEnergy());
		strcat(line, name);
	}

	if ( RetCapacity() != 1.0f )
	{
		sprintf(name, " capacity=%.2f", RetCapacity());
		strcat(line, name);
	}

	if ( RetShield() != 1.0f )
	{
		sprintf(name, " shield=%.2f", RetShield());
		strcat(line, name);
	}

	if ( RetRange() != 1.0f )
	{
		sprintf(name, " range=%.2f", RetRange());
		strcat(line, name);
	}

	if ( RetSelectable() != 1 )
	{
		sprintf(name, " selectable=%d", RetSelectable());
		strcat(line, name);
	}

	if ( RetEnable() != 1 )
	{
		sprintf(name, " enable=%d", RetEnable());
		strcat(line, name);
	}

	if ( RetFixed() != 0 )
	{
		sprintf(name, " fixed=%d", RetFixed());
		strcat(line, name);
	}

	if ( RetClip() != 1 )
	{
		sprintf(name, " clip=%d", RetClip());
		strcat(line, name);
	}

	if ( RetLock() != 0 )
	{
		sprintf(name, " lock=%d", RetLock());
		strcat(line, name);
	}

	if ( RetProxyActivate() != 0 )
	{
		sprintf(name, " proxyActivate=%d", RetProxyActivate());
		strcat(line, name);

		sprintf(name, " proxyDistance=%.2f", RetProxyDistance()/g_unit);
		strcat(line, name);
	}

	if ( RetMagnifyDamage() != 1.0f )
	{
		sprintf(name, " magnifyDamage=%.2f", RetMagnifyDamage());
		strcat(line, name);
	}

	if ( RetGunGoalV() != 0.0f )
	{
		sprintf(name, " aimV=%.2f", RetGunGoalV());
		strcat(line, name);
	}
	if ( RetGunGoalH() != 0.0f )
	{
		sprintf(name, " aimH=%.2f", RetGunGoalH());
		strcat(line, name);
	}

	if ( RetParam() != 0.0f )
	{
		sprintf(name, " param=%.2f", RetParam());
		strcat(line, name);
	}

	if ( RetResetCap() != 0 )
	{
		sprintf(name, " resetCap=%d", RetResetCap());
		strcat(line, name);

		pos = RetResetPosition()/g_unit;
		sprintf(name, " resetPos=%.2f;%.2f;%.2f", pos.x, pos.y, pos.z);
		strcat(line, name);

		pos = RetResetAngle()/(PI/180.0f);
		sprintf(name, " resetAngle=%.2f;%.2f;%.2f", pos.x, pos.y, pos.z);
		strcat(line, name);

		sprintf(name, " resetRun=%d", RetResetRun());
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

	// Met les infos dans borne (OBJECT_INFO).
	for ( i=0 ; i<m_infoTotal ; i++ )
	{
		info = RetInfo(i);
		if ( info.name[0] == 0 )  break;

		sprintf(name, " info%d=\"%s=%.2f\"", i+1, info.name, info.value);
		strcat(line, name);
	}

	// Met les param�tres de la ligne de commande.
	for ( i=0 ; i<OBJECTMAXCMDLINE ; i++ )
	{
		value = RetCmdLine(i);
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

	return TRUE;
}

// Restitue tous les param�tres de l'objet.

BOOL CObject::Read(char *line)
{
	D3DVECTOR	pos, dir;
	Info		info;
	CameraType	cType;
	char		op[20];
	char		text[100];
	char*		p;
	float		value;
	int			i;

	cType = OpCamera(line, "camera");
	if ( cType != CAMERA_NULL )
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
	SetResetCap((ResetCap)OpInt(line, "resetCap", 0));
	SetResetPosition(OpDir(line, "resetPos")*g_unit);
	SetResetAngle(OpDir(line, "resetAngle")*(PI/180.0f));
	SetResetRun(OpInt(line, "resetRun", 0));
	m_bBurn = OpInt(line, "burnMode", 0);
	m_bVirusMode = OpInt(line, "virusMode", 0);
	m_virusTime = OpFloat(line, "virusTime", 0.0f);

	// Met les infos dans borne (OBJECT_INFO).
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

	// Met les param�tres de la ligne de commande.
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

	return TRUE;
}



// Cherche le ni�me fils d'un p�re.

int CObject::SearchDescendant(int parent, int n)
{
	int		i;

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


// Supprime toutes les sph�res utilis�es pour les collisions.

void CObject::FlushCrashShere()
{
	m_crashSphereUsed = 0;
}

// Ajoute une nouvelle sph�re.

int CObject::CreateCrashSphere(D3DVECTOR pos, float radius, Sound sound,
							   float hardness)
{
	float	zoom;

	if ( m_crashSphereUsed >= MAXCRASHSPHERE )  return -1;

	zoom = RetZoomX(0);
	m_crashSpherePos[m_crashSphereUsed] = pos;
	m_crashSphereRadius[m_crashSphereUsed] = radius*zoom;
	m_crashSphereHardness[m_crashSphereUsed] = hardness;
	m_crashSphereSound[m_crashSphereUsed] = sound;
	return m_crashSphereUsed++;
}

// Retourne le nombre de sph�res.

int CObject::RetCrashSphereTotal()
{
	return m_crashSphereUsed;
}

// Retourne une sph�re pour les collisions.
// La position est absolue dans le monde.

BOOL CObject::GetCrashSphere(int rank, D3DVECTOR &pos, float &radius)
{
	if ( rank < 0 || rank >= m_crashSphereUsed )
	{
		pos = m_objectPart[0].position;
		radius = 0.0f;
		return FALSE;
	}

	// Retourne la sph�re pour les collisions, qui ne tient pas
	// compte de l'inclinaison du v�hicule. Ceci est n�cessaire
	// pour les collisions avec les v�hicules, afin de ne pas tenir
	// compte de SetInclinaison, par exemple.
	// La sph�re doit avoir obligatoirement un centre (0;y;0).
	if ( rank == 0 && m_crashSphereUsed == 1 &&
		 m_crashSpherePos[0].x == 0.0f &&
		 m_crashSpherePos[0].z == 0.0f )
	{
		pos = m_objectPart[0].position + m_crashSpherePos[0];
		radius = m_crashSphereRadius[0];
		return TRUE;
	}

	if ( m_objectPart[0].bTranslate ||
		 m_objectPart[0].bRotate    )
	{
		UpdateTransformObject();
	}
	pos = Transform(m_objectPart[0].matWorld, m_crashSpherePos[rank]);
	radius = m_crashSphereRadius[rank];
	return TRUE;
}

// Retourne la duret� d'une sph�re.

Sound CObject::RetCrashSphereSound(int rank)
{
	return m_crashSphereSound[rank];
}

// Retourne la duret� d'une sph�re.

float CObject::RetCrashSphereHardness(int rank)
{
	return m_crashSphereHardness[rank];
}

// Supprime une sph�re.

void CObject::DeleteCrashSphere(int rank)
{
	int		i;

	if ( rank < 0 || rank >= m_crashSphereUsed )  return;

	for ( i=rank+1 ; i<MAXCRASHSPHERE ; i++ )
	{
		m_crashSpherePos[i-1]    = m_crashSpherePos[i];
		m_crashSphereRadius[i-1] = m_crashSphereRadius[i];
	}
	m_crashSphereUsed --;
}

// Sp�cifie la sph�re globale, relative � l'objet.

void CObject::SetGlobalSphere(D3DVECTOR pos, float radius)
{
	float	zoom;

	zoom = RetZoomX(0);
	m_globalSpherePos    = pos;
	m_globalSphereRadius = radius*zoom;
}

// Retourne la sph�re globale, dans l'univers.

void CObject::GetGlobalSphere(D3DVECTOR &pos, float &radius)
{
	pos = Transform(m_objectPart[0].matWorld, m_globalSpherePos);
	radius = m_globalSphereRadius;
}


// Sp�cifie la sph�re de bousculade, relative � l'objet.

void CObject::SetJotlerSphere(D3DVECTOR pos, float radius)
{
	m_jotlerSpherePos    = pos;
	m_jotlerSphereRadius = radius;
}

// Retourne la sph�re de bousculade, dans l'univers.

void CObject::GetJotlerSphere(D3DVECTOR &pos, float &radius)
{
	pos = Transform(m_objectPart[0].matWorld, m_jotlerSpherePos);
	radius = m_jotlerSphereRadius;
}


// Sp�cifie le rayon du bouclier.

void CObject::SetShieldRadius(float radius)
{
	m_shieldRadius = radius;
}

// Retourne le rayon du bouclier.

float CObject::RetShieldRadius()
{
	return m_shieldRadius;
}


// Positionne un objet � une certaine hauteur par-rapport au sol.

void CObject::SetFloorHeight(float height)
{
	D3DVECTOR	pos;

	pos = m_objectPart[0].position;
	m_terrain->MoveOnFloor(pos);

	if ( m_physics != 0 )
	{
		m_physics->SetLand(height == 0.0f);
		m_physics->SetMotor(height != 0.0f);
	}

	m_objectPart[0].position.y = pos.y+height+m_character.height;
	m_objectPart[0].bTranslate = TRUE;  // il faudra recalculer les matrices
}

// Ajuste l'inclinaison d'un objet pos� sur le sol.

void CObject::FloorAdjust()
{
	D3DVECTOR		pos, n;
	FPOINT			nn;
	float			a;

	pos = RetPosition(0);
	if ( m_terrain->GetNormal(n, pos) )
	{
#if 0
		SetAngleX(0,  sinf(n.z));
		SetAngleZ(0, -sinf(n.x));
		SetAngleY(0, 0.0f);
#else
		a = RetAngleY(0);
		nn = RotatePoint(-a, FPOINT(n.z, n.x));
		SetAngleX(0,  sinf(nn.x));
		SetAngleZ(0, -sinf(nn.y));
#endif
	}
}


// Donne la vibration lin�aire.

void CObject::SetLinVibration(D3DVECTOR dir)
{
	if ( m_linVibration.x != dir.x ||
		 m_linVibration.y != dir.y ||
		 m_linVibration.z != dir.z )
	{
		m_linVibration = dir;
		m_objectPart[0].bTranslate = TRUE;
	}
}

D3DVECTOR CObject::RetLinVibration()
{
	return m_linVibration;
}

// Donne la vibration circulaire.

void CObject::SetCirVibration(D3DVECTOR dir)
{
	if ( m_cirVibration.x != dir.x ||
		 m_cirVibration.y != dir.y ||
		 m_cirVibration.z != dir.z )
	{
		m_cirVibration = dir;
		m_objectPart[0].bRotate = TRUE;
	}
}

D3DVECTOR CObject::RetCirVibration()
{
	return m_cirVibration;
}

// Donne l'inclinaison.

void CObject::SetInclinaison(D3DVECTOR dir)
{
	if ( m_inclinaison.x != dir.x ||
		 m_inclinaison.y != dir.y ||
		 m_inclinaison.z != dir.z )
	{
		m_inclinaison = dir;
		m_objectPart[0].bRotate = TRUE;
	}
}

D3DVECTOR CObject::RetInclinaison()
{
	return m_inclinaison;
}


// Donne la position du centre de l'objet.

void CObject::SetPosition(int part, const D3DVECTOR &pos)
{
	D3DVECTOR	shPos, n[20], norm;
	float		height, radius;
	int			rank, i, j;

	m_objectPart[part].position = pos;
	m_objectPart[part].bTranslate = TRUE;  // il faudra recalculer les matrices

	if ( part == 0 && !m_bFlat )  // partie principale ?
	{
		rank = m_objectPart[0].object;

		shPos = pos;
		m_terrain->MoveOnFloor(shPos, TRUE);
		m_engine->SetObjectShadowPos(rank, shPos);

		if ( m_physics != 0 && m_physics->RetType() == TYPE_FLYING )
		{
			height = pos.y-shPos.y;
		}
		else
		{
			height = 0.0f;
		}
		m_engine->SetObjectShadowHeight(rank, height);

		// Calcul la normale au terrain en 9 points strat�giques,
		// puis effectue une moyenne pond�r�e (les points au centre
		// ont plus d'importance).
		radius = m_engine->RetObjectShadowRadius(rank);
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

		norm = 0.0f;
		for ( j=0 ; j<i ; j++ )
		{
			norm += n[j];
		}
		norm /= (float)i;  // moyenne vectorielle

		m_engine->SetObjectShadowNormal(rank, norm);

		if ( m_shadowLight != -1 )
		{
			shPos = pos;
			shPos.y += m_shadowHeight;
			m_light->SetLightPos(m_shadowLight, shPos);
		}

		if ( m_effectLight != -1 )
		{
			shPos = pos;
			shPos.y += m_effectHeight;
			m_light->SetLightPos(m_effectLight, shPos);
		}

		if ( m_bShowLimit )
		{
			m_main->AdjustShowLimit(0, pos);
		}
	}
}

D3DVECTOR CObject::RetPosition(int part)
{
	return m_objectPart[part].position;
}

// Donne la rotation autour des 3 axes.

void CObject::SetAngle(int part, const D3DVECTOR &angle)
{
	m_objectPart[part].angle = angle;
	m_objectPart[part].bRotate = TRUE;  // il faudra recalculer les matrices

	if ( part == 0 && !m_bFlat )  // partie principale ?
	{
		m_engine->SetObjectShadowAngle(m_objectPart[0].object, m_objectPart[0].angle.y);
	}
}

D3DVECTOR CObject::RetAngle(int part)
{
	return m_objectPart[part].angle;
}

// Donne la rotation autour de l'axe Y.

void CObject::SetAngleY(int part, float angle)
{
	m_objectPart[part].angle.y = angle;
	m_objectPart[part].bRotate = TRUE;  // il faudra recalculer les matrices

	if ( part == 0 && !m_bFlat )  // partie principale ?
	{
		m_engine->SetObjectShadowAngle(m_objectPart[0].object, m_objectPart[0].angle.y);
	}
}

// Donne la rotation autour de l'axe X.

void CObject::SetAngleX(int part, float angle)
{
	m_objectPart[part].angle.x = angle;
	m_objectPart[part].bRotate = TRUE;  // il faudra recalculer les matrices
}

// Donne la rotation autour de l'axe Z.

void CObject::SetAngleZ(int part, float angle)
{
	m_objectPart[part].angle.z = angle;
	m_objectPart[part].bRotate = TRUE;  // il faudra recalculer les matrices
}

float CObject::RetAngleY(int part)
{
	return m_objectPart[part].angle.y;
}

float CObject::RetAngleX(int part)
{
	return m_objectPart[part].angle.x;
}

float CObject::RetAngleZ(int part)
{
	return m_objectPart[part].angle.z;
}


// Donne le zoom lobal.

void CObject::SetZoom(int part, float zoom)
{
	m_objectPart[part].bTranslate = TRUE;  // il faudra recalculer les matrices
	m_objectPart[part].zoom.x = zoom;
	m_objectPart[part].zoom.y = zoom;
	m_objectPart[part].zoom.z = zoom;

	m_objectPart[part].bZoom = ( m_objectPart[part].zoom.x != 1.0f ||
								 m_objectPart[part].zoom.y != 1.0f ||
								 m_objectPart[part].zoom.z != 1.0f );
}

void CObject::SetZoom(int part, D3DVECTOR zoom)
{
	m_objectPart[part].bTranslate = TRUE;  // il faudra recalculer les matrices
	m_objectPart[part].zoom = zoom;

	m_objectPart[part].bZoom = ( m_objectPart[part].zoom.x != 1.0f ||
								 m_objectPart[part].zoom.y != 1.0f ||
								 m_objectPart[part].zoom.z != 1.0f );
}

D3DVECTOR CObject::RetZoom(int part)
{
	return m_objectPart[part].zoom;
}

void CObject::SetZoomX(int part, float zoom)
{
	m_objectPart[part].bTranslate = TRUE;  // il faudra recalculer les matrices
	m_objectPart[part].zoom.x = zoom;

	m_objectPart[part].bZoom = ( m_objectPart[part].zoom.x != 1.0f ||
								 m_objectPart[part].zoom.y != 1.0f ||
								 m_objectPart[part].zoom.z != 1.0f );
}

void CObject::SetZoomY(int part, float zoom)
{
	m_objectPart[part].bTranslate = TRUE;  // il faudra recalculer les matrices
	m_objectPart[part].zoom.y = zoom;

	m_objectPart[part].bZoom = ( m_objectPart[part].zoom.x != 1.0f ||
								 m_objectPart[part].zoom.y != 1.0f ||
								 m_objectPart[part].zoom.z != 1.0f );
}

void CObject::SetZoomZ(int part, float zoom)
{
	m_objectPart[part].bTranslate = TRUE;  // il faudra recalculer les matrices
	m_objectPart[part].zoom.z = zoom;

	m_objectPart[part].bZoom = ( m_objectPart[part].zoom.x != 1.0f ||
								 m_objectPart[part].zoom.y != 1.0f ||
								 m_objectPart[part].zoom.z != 1.0f );
}

float CObject::RetZoomX(int part)
{
	return m_objectPart[part].zoom.x;
}

float CObject::RetZoomY(int part)
{
	return m_objectPart[part].zoom.y;
}

float CObject::RetZoomZ(int part)
{
	return m_objectPart[part].zoom.z;
}


// Retourne le niveau de l'eau.

float CObject::RetWaterLevel()
{
	return m_water->RetLevel();
}


void CObject::SetTrainer(BOOL bEnable)
{
	m_bTrainer = bEnable;

	if ( m_bTrainer )  // entra�nement ?
	{
		m_cameraType = CAMERA_FIX;
	}
}

BOOL CObject::RetTrainer()
{
	return m_bTrainer;
}

void CObject::SetToy(BOOL bEnable)
{
	m_bToy = bEnable;
}

BOOL CObject::RetToy()
{
	return m_bToy;
}

void CObject::SetManual(BOOL bManual)
{
	m_bManual = bManual;
}

BOOL CObject::RetManual()
{
	return m_bManual;
}

void CObject::SetResetCap(ResetCap cap)
{
	m_resetCap = cap;
}

ResetCap CObject::RetResetCap()
{
	return m_resetCap;
}

void CObject::SetResetBusy(BOOL bBusy)
{
	m_bResetBusy = bBusy;
}

BOOL CObject::RetResetBusy()
{
	return m_bResetBusy;
}

void CObject::SetResetPosition(const D3DVECTOR &pos)
{
	m_resetPosition = pos;
}

D3DVECTOR CObject::RetResetPosition()
{
	return m_resetPosition;
}

void CObject::SetResetAngle(const D3DVECTOR &angle)
{
	m_resetAngle = angle;
}

D3DVECTOR CObject::RetResetAngle()
{
	return m_resetAngle;
}

int CObject::RetResetRun()
{
	return m_resetRun;
}

void CObject::SetResetRun(int run)
{
	m_resetRun = run;
}


// Gestion de la particule ma�tresse.

void CObject::SetMasterParticule(int part, int parti)
{
	m_objectPart[part].masterParti = parti;
}

int CObject::RetMasterParticule(int part)
{
	return m_objectPart[part].masterParti;
}


// Gestion de la pile transport�e.

void CObject::SetPower(CObject* power)
{
	m_power = power;
}

CObject* CObject::RetPower()
{
	return m_power;
}

// Gestion de l'objet transport�.

void CObject::SetFret(CObject* fret)
{
	m_fret = fret;
}

CObject* CObject::RetFret()
{
	return m_fret;
}

// Gestion de l'objet "camion" qui transporte celui-ci.

void CObject::SetTruck(CObject* truck)
{
	m_truck = truck;

	// Ombre invisible si l'objet est transport�.
	m_engine->SetObjectShadowHide(m_objectPart[0].object, (m_truck != 0));
}

CObject* CObject::RetTruck()
{
	return m_truck;
}

// Gestion de la partie transporteuse.

void CObject::SetTruckPart(int part)
{
	m_truckLink = part;
}

int CObject::RetTruckPart()
{
	return m_truckLink;
}


// Gestion des informations utilisateur.

void CObject::InfoFlush()
{
	m_infoTotal = 0;
	m_bInfoUpdate = TRUE;
}

void CObject::DeleteInfo(int rank)
{
	int		i;

	if ( rank < 0 || rank >= m_infoTotal )  return;

	for ( i=rank ; i<m_infoTotal-1 ; i++ )
	{
		m_info[i] = m_info[i+1];
	}
	m_infoTotal --;
	m_bInfoUpdate = TRUE;
}

void CObject::SetInfo(int rank, Info info)
{
	if ( rank < 0 || rank >= OBJECTMAXINFO )  return;
	m_info[rank] = info;

	if ( rank+1 > m_infoTotal )  m_infoTotal = rank+1;
	m_bInfoUpdate = TRUE;
}

Info CObject::RetInfo(int rank)
{
	if ( rank < 0 || rank >= OBJECTMAXINFO )  rank = 0;
	return m_info[rank];
}

int CObject::RetInfoTotal()
{
	return m_infoTotal;
}

void CObject::SetInfoReturn(float value)
{
	m_infoReturn = value;
}

float CObject::RetInfoReturn()
{
	return m_infoReturn;
}

void CObject::SetInfoUpdate(BOOL bUpdate)
{
	m_bInfoUpdate = bUpdate;
}

BOOL CObject::RetInfoUpdate()
{
	return m_bInfoUpdate;
}


BOOL CObject::SetCmdLine(int rank, float value)
{
	if ( rank < 0 || rank >= OBJECTMAXCMDLINE )  return FALSE;
	m_cmdLine[rank] = value;
	return TRUE;
}

float CObject::RetCmdLine(int rank)
{
	if ( rank < 0 || rank >= OBJECTMAXCMDLINE )  return 0.0f;
	return m_cmdLine[rank];
}


// Retourne les matrices d'une partie d'objet.

D3DMATRIX* CObject::RetRotateMatrix(int part)
{
	return &m_objectPart[part].matRotate;
}

D3DMATRIX* CObject::RetTranslateMatrix(int part)
{
	return &m_objectPart[part].matTranslate;
}

D3DMATRIX* CObject::RetTransformMatrix(int part)
{
	return &m_objectPart[part].matTransform;
}

D3DMATRIX* CObject::RetWorldMatrix(int part)
{
	if ( m_objectPart[0].bTranslate ||
		 m_objectPart[0].bRotate    )
	{
		UpdateTransformObject();
	}

	return &m_objectPart[part].matWorld;
}


// Indique si l'objet doit �tre dessin� par dessous l'interface.

void CObject::SetDrawWorld(BOOL bDraw)
{
	int		i;

	for ( i=0 ; i<OBJECTMAXPART ; i++ )
	{
		if ( m_objectPart[i].bUsed )
		{
			m_engine->SetDrawWorld(m_objectPart[i].object, bDraw);
		}
	}
}

// Indique si l'objet doit �tre dessin� par dessus l'interface.

void CObject::SetDrawFront(BOOL bDraw)
{
	int		i;

	for ( i=0 ; i<OBJECTMAXPART ; i++ )
	{
		if ( m_objectPart[i].bUsed )
		{
			m_engine->SetDrawFront(m_objectPart[i].object, bDraw);
		}
	}
}


// Cr�e un v�hicule roulant quelconque pos� sur le sol.

BOOL CObject::CreateVehicle(D3DVECTOR pos, float angle, ObjectType type,
							float power, BOOL bTrainer, BOOL bToy)
{
	m_type = type;

	if ( type == OBJECT_TOTO )
	{
		m_motion = new CMotionToto(m_iMan, this);
		m_motion->Create(pos, angle, type, 1.0f);
		return TRUE;
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
		 type == OBJECT_MOBILEic )  // canon fireball ?
	{
		m_showLimitRadius = 160.0f;
	}
	if ( type == OBJECT_MOBILEfi ||
		 type == OBJECT_MOBILEti ||
		 type == OBJECT_MOBILEwi ||
		 type == OBJECT_MOBILEii )  // canon orgaball ?
	{
		m_showLimitRadius = 160.0f;
	}
	if ( type == OBJECT_MOBILErc )  // canon phazer ?
	{
		m_showLimitRadius = 160.0f;
	}
	if ( type == OBJECT_MOBILErs )  // robot bouclier ?
	{
		m_showLimitRadius = 50.0f;
	}
#endif
	if ( type == OBJECT_MOBILErt )  // robot secoueur ?
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
	if ( m_motion == 0 )  return FALSE;

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
		return FALSE;
	}

	return TRUE;
}

// Cr�e un insecte quelconque pos� sur le sol.

BOOL CObject::CreateInsect(D3DVECTOR pos, float angle, ObjectType type)
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
	if ( m_motion == 0 )  return FALSE;

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
		return FALSE;
	}

	return TRUE;
}

// Cr�e l'ombre sous un v�hicule sous forme d'une lumi�re
// n�gative.

BOOL CObject::CreateShadowLight(float height, D3DCOLORVALUE color)
{
	D3DLIGHT7	light;
	D3DVECTOR	pos;

	if ( !m_engine->RetLightMode() )  return TRUE;

	pos = RetPosition(0);
	m_shadowHeight = height;

    ZeroMemory( &light, sizeof(light) );
	light.dltType       = D3DLIGHT_SPOT;
	light.dcvDiffuse.r  = color.r;
	light.dcvDiffuse.g  = color.g;
	light.dcvDiffuse.b  = color.b;
	light.dvPosition.x  = pos.x;
	light.dvPosition.y  = pos.y+height;
	light.dvPosition.z  = pos.z;
	light.dvDirection.x =  0.0f;
	light.dvDirection.y = -1.0f;  // contre en bas
	light.dvDirection.z =  0.0f;
	light.dvRange = D3DLIGHT_RANGE_MAX;
	light.dvFalloff = 1.0f;
	light.dvAttenuation0 = 1.0f;
	light.dvAttenuation1 = 0.0f;
	light.dvAttenuation2 = 0.0f;
	light.dvTheta = 0.0f;
	light.dvPhi = PI/4.0f;

	m_shadowLight = m_light->CreateLight();
	if ( m_shadowLight == -1 )  return FALSE;

	m_light->SetLight(m_shadowLight, light);

	// N'�claire que les objets du terrain.
	m_light->SetLightIncluType(m_shadowLight, TYPETERRAIN);

	return TRUE;
}

// Retourne le num�ro de la lumi�re d'ombre n�gative.

int CObject::RetShadowLight()
{
	return m_shadowLight;
}

// Cr�e la lumi�re pour les effects d'un v�hicule.

BOOL CObject::CreateEffectLight(float height, D3DCOLORVALUE color)
{
	D3DLIGHT7	light;

	if ( !m_engine->RetLightMode() )  return TRUE;

	m_effectHeight = height;

    ZeroMemory( &light, sizeof(light) );
	light.dltType       = D3DLIGHT_SPOT;
	light.dcvDiffuse.r  = color.r;
	light.dcvDiffuse.g  = color.g;
	light.dcvDiffuse.b  = color.b;
	light.dvPosition.x  =  0.0f;
	light.dvPosition.y  =  0.0f+height;
	light.dvPosition.z  =  0.0f;
	light.dvDirection.x =  0.0f;
	light.dvDirection.y = -1.0f;  // contre en bas
	light.dvDirection.z =  0.0f;
	light.dvRange = D3DLIGHT_RANGE_MAX;
	light.dvFalloff = 1.0f;
	light.dvAttenuation0 = 1.0f;
	light.dvAttenuation1 = 0.0f;
	light.dvAttenuation2 = 0.0f;
	light.dvTheta = 0.0f;
	light.dvPhi = PI/4.0f;

	m_effectLight = m_light->CreateLight();
	if ( m_effectLight == -1 )  return FALSE;

	m_light->SetLight(m_effectLight, light);
	m_light->SetLightIntensity(m_effectLight, 0.0f);

	return TRUE;
}

// Retourne le num�ro de la lumi�re des effets.

int CObject::RetEffectLight()
{
	return m_effectLight;
}

// Cr�e l'ombre circulaire sous un v�hicule.

BOOL CObject::CreateShadowCircle(float radius, float intensity,
								 D3DShadowType type)
{
	float	zoom;

	if ( intensity == 0.0f )  return TRUE;

	zoom = RetZoomX(0);

	m_engine->ShadowCreate(m_objectPart[0].object);

	m_engine->SetObjectShadowRadius(m_objectPart[0].object, radius*zoom);
	m_engine->SetObjectShadowIntensity(m_objectPart[0].object, intensity);
	m_engine->SetObjectShadowHeight(m_objectPart[0].object, 0.0f);
	m_engine->SetObjectShadowAngle(m_objectPart[0].object, m_objectPart[0].angle.y);
	m_engine->SetObjectShadowType(m_objectPart[0].object, type);

	return TRUE;
}

// Cr�e un batiment quelconque pos� sur le sol.

BOOL CObject::CreateBuilding(D3DVECTOR pos, float angle, float height,
							 ObjectType type, float power)
{
	CModFile*	pModFile;
	FPOINT		p;
	int			rank, i;

	if ( m_engine->RetRestCreate() < 20 )  return FALSE;

	pModFile = new CModFile(m_iMan);

	SetType(type);

	rank = m_engine->CreateObject();
	m_engine->SetObjectType(rank, TYPEFIX);  // c'est un objet fixe
	SetObjectRank(0, rank);

	if ( m_type == OBJECT_PORTICO )
	{
		pModFile->ReadModel("objects\\portico1.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(1, rank);
		SetObjectParent(1, 0);
		pModFile->ReadModel("objects\\portico2.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(1, D3DVECTOR(0.0f, 67.0f, 0.0f));

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(2, rank);
		SetObjectParent(2, 1);
		pModFile->ReadModel("objects\\portico3.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(2, D3DVECTOR(0.0f, 0.0f, -33.0f));
		SetAngleY(2, 45.0f*PI/180.0f);

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(3, rank);
		SetObjectParent(3, 2);
		pModFile->ReadModel("objects\\portico4.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(3, D3DVECTOR(50.0f, 0.0f, 0.0f));
		SetAngleY(3, -60.0f*PI/180.0f);

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(4, rank);
		SetObjectParent(4, 3);
		pModFile->ReadModel("objects\\portico5.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(4, D3DVECTOR(35.0f, 0.0f, 0.0f));
		SetAngleY(4, -55.0f*PI/180.0f);
		
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(5, rank);
		SetObjectParent(5, 1);
		pModFile->ReadModel("objects\\portico3.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(5, D3DVECTOR(0.0f, 0.0f, 33.0f));
		SetAngleY(5, -45.0f*PI/180.0f);

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(6, rank);
		SetObjectParent(6, 5);
		pModFile->ReadModel("objects\\portico4.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(6, D3DVECTOR(50.0f, 0.0f, 0.0f));
		SetAngleY(6, 60.0f*PI/180.0f);

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(7, rank);
		SetObjectParent(7, 6);
		pModFile->ReadModel("objects\\portico5.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(7, D3DVECTOR(35.0f, 0.0f, 0.0f));
		SetAngleY(7, 55.0f*PI/180.0f);
		
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(8, rank);
		SetObjectParent(8, 0);
		pModFile->ReadModel("objects\\portico6.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(8, D3DVECTOR(-35.0f, 50.0f, -35.0f));
		SetAngleY(8, -PI/2.0f);
		SetZoom(8, 2.0f);

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(9, rank);
		SetObjectParent(9, 8);
		pModFile->ReadModel("objects\\portico7.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(9, D3DVECTOR(0.0f, 4.5f, 1.9f));

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(10, rank);
		SetObjectParent(10, 0);
		pModFile->ReadModel("objects\\portico6.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(10, D3DVECTOR(-35.0f, 50.0f, 35.0f));
		SetAngleY(10, -PI/2.0f);
		SetZoom(10, 2.0f);

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(11, rank);
		SetObjectParent(11, 10);
		pModFile->ReadModel("objects\\portico7.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(11, D3DVECTOR(0.0f, 4.5f, 1.9f));

		CreateCrashSphere(D3DVECTOR(  0.0f, 28.0f,   0.0f), 45.5f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 27.0f, 10.0f, -42.0f), 15.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(  0.0f, 10.0f, -42.0f), 15.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-27.0f, 10.0f, -42.0f), 15.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 27.0f, 10.0f,  42.0f), 15.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(  0.0f, 10.0f,  42.0f), 15.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-27.0f, 10.0f,  42.0f), 15.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-32.0f, 45.0f, -32.0f), 10.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-32.0f, 45.0f,  32.0f), 10.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 32.0f, 45.0f, -32.0f), 10.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 32.0f, 45.0f,  32.0f), 10.0f, SOUND_BOUMm, 0.45f);
		SetGlobalSphere(D3DVECTOR(0.0f, 35.0f, 0.0f), 50.0f);

		CreateShadowCircle(50.0f, 1.0f);
	}

	if ( m_type == OBJECT_BASE )
	{
		pModFile->ReadModel("objects\\base1.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);

		for ( i=0 ; i<8 ; i++ )
		{
			rank = m_engine->CreateObject();
			m_engine->SetObjectType(rank, TYPEDESCENDANT);
			SetObjectRank(1+i, rank);
			SetObjectParent(1+i, 0);
			pModFile->ReadModel("objects\\base2.mod");
			pModFile->CreateEngineObject(rank);
			p = RotatePoint(-PI/4.0f*i, 27.8f);
			SetPosition(1+i, D3DVECTOR(p.x, 30.0f, p.y));
			SetAngleY(1+i, PI/4.0f*i);
			SetAngleZ(1+i, PI/2.0f);

			rank = m_engine->CreateObject();
			m_engine->SetObjectType(rank, TYPEDESCENDANT);
			SetObjectRank(10+i, rank);
			SetObjectParent(10+i, 1+i);
			pModFile->ReadModel("objects\\base4.mod");
			pModFile->CreateEngineObject(rank);
			SetPosition(10+i, D3DVECTOR(23.5f, 0.0f, 7.0f));

			rank = m_engine->CreateObject();
			m_engine->SetObjectType(rank, TYPEDESCENDANT);
			SetObjectRank(18+i, rank);
			SetObjectParent(18+i, 1+i);
			pModFile->ReadModel("objects\\base4.mod");
			pModFile->Mirror();
			pModFile->CreateEngineObject(rank);
			SetPosition(18+i, D3DVECTOR(23.5f, 0.0f, -7.0f));
		}

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(9, rank);
		SetObjectParent(9, 0);
		pModFile->ReadModel("objects\\base3.mod");  // pilier central
		pModFile->CreateEngineObject(rank);

		CreateCrashSphere(D3DVECTOR(  0.0f, 33.0f,   0.0f),  2.5f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(  0.0f, 39.0f,   0.0f),  2.5f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(  0.0f, 45.0f,   0.0f),  2.5f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(  0.0f, 51.0f,   0.0f),  2.5f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(  0.0f, 57.0f,   0.0f),  2.5f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(  0.0f, 63.0f,   0.0f),  2.5f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(  0.0f, 69.0f,   0.0f),  2.5f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(  0.0f, 82.0f,   0.0f),  8.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 18.0f, 94.0f,   0.0f), 10.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-18.0f, 94.0f,   0.0f), 10.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(  0.0f, 94.0f,  18.0f), 10.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(  0.0f, 94.0f, -18.0f), 10.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 13.0f, 94.0f,  13.0f), 10.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-13.0f, 94.0f,  13.0f), 10.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 13.0f, 94.0f, -13.0f), 10.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-13.0f, 94.0f, -13.0f), 10.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(  0.0f,104.0f,   0.0f), 14.0f, SOUND_BOUMm, 0.45f);

		SetGlobalSphere(D3DVECTOR(0.0f, 45.0f, 0.0f), 10.0f);

		CreateShadowCircle(60.0f, 1.0f);
		m_showLimitRadius = 200.0f;

		m_terrain->AddBuildingLevel(pos, 28.6f, 73.4f, 30.0f, 0.4f);
	}

	if ( m_type == OBJECT_DERRICK )
	{
		pModFile->ReadModel("objects\\derrick1.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(1, rank);
		SetObjectParent(1, 0);
		pModFile->ReadModel("objects\\derrick2.mod");
		pModFile->CreateEngineObject(rank);

		CreateCrashSphere(D3DVECTOR(0.0f,  0.0f, 0.0f), 6.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(0.0f, 10.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(0.0f, 17.0f, 0.0f), 6.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(0.0f, 26.0f, 0.0f), 3.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(7.0f, 17.0f, 0.0f), 3.0f, SOUND_BOUMm, 0.45f);
		SetGlobalSphere(D3DVECTOR(0.0f, 10.0f, 0.0f), 10.0f);

		CreateShadowCircle(10.0f, 0.4f);
	}

	if ( m_type == OBJECT_RESEARCH )
	{
		pModFile->ReadModel("objects\\search1.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(1, rank);
		SetObjectParent(1, 0);
		pModFile->ReadModel("objects\\search2.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(1, D3DVECTOR(0.0f, 13.0f, 0.0f));

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(2, rank);
		SetObjectParent(2, 1);
		pModFile->ReadModel("objects\\search3.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(2, D3DVECTOR(0.0f, 4.0f, 0.0f));
		SetAngleZ(2, 35.0f*PI/180.0f);

		CreateCrashSphere(D3DVECTOR(0.0f,  0.0f, 0.0f), 9.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(0.0f,  6.0f, 0.0f), 9.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(0.0f, 14.0f, 0.0f), 7.0f, SOUND_BOUMm, 0.45f);
		SetGlobalSphere(D3DVECTOR(0.0f, 8.0f, 0.0f), 12.0f);

		m_character.posPower = D3DVECTOR(7.5f, 3.0f, 0.0f);

		CreateShadowCircle(12.0f, 1.0f);
	}

	if ( m_type == OBJECT_RADAR )
	{
		pModFile->ReadModel("objects\\radar1.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(1, rank);
		SetObjectParent(1, 0);
		pModFile->ReadModel("objects\\radar2.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(1, D3DVECTOR(0.0f, 5.0f, 0.0f));

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(2, rank);
		SetObjectParent(2, 0);
		pModFile->ReadModel("objects\\radar3.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(2, D3DVECTOR(0.0f, 11.0f, 0.0f));
		SetAngleY(2, -PI/2.0f);

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(3, rank);
		SetObjectParent(3, 2);
		pModFile->ReadModel("objects\\radar4.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(3, D3DVECTOR(0.0f, 4.5f, 1.9f));

		CreateCrashSphere(D3DVECTOR(0.0f,  3.0f, 0.0f), 6.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(0.0f, 11.0f, 0.0f), 6.0f, SOUND_BOUMm, 0.45f);
		SetGlobalSphere(D3DVECTOR(0.0f, 7.0f, 0.0f), 7.0f);

		CreateShadowCircle(8.0f, 1.0f);
	}

	if ( m_type == OBJECT_INFO )
	{
		pModFile->ReadModel("objects\\info1.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(1, rank);
		SetObjectParent(1, 0);
		pModFile->ReadModel("objects\\info2.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(1, D3DVECTOR(0.0f, 5.0f, 0.0f));

		for ( i=0 ; i<3 ; i++ )
		{
			rank = m_engine->CreateObject();
			m_engine->SetObjectType(rank, TYPEDESCENDANT);
			SetObjectRank(2+i*2, rank);
			SetObjectParent(2+i*2, 1);
			pModFile->ReadModel("objects\\info3.mod");
			pModFile->CreateEngineObject(rank);
			SetPosition(2+i*2, D3DVECTOR(0.0f, 4.5f, 0.0f));

			rank = m_engine->CreateObject();
			m_engine->SetObjectType(rank, TYPEDESCENDANT);
			SetObjectRank(3+i*2, rank);
			SetObjectParent(3+i*2, 2+i*2);
			pModFile->ReadModel("objects\\radar4.mod");
			pModFile->CreateEngineObject(rank);
			SetPosition(3+i*2, D3DVECTOR(0.0f, 0.0f, -4.0f));

			SetAngleY(2+i*2, 2.0f*PI/3.0f*i);
		}

		CreateCrashSphere(D3DVECTOR(0.0f,  3.0f, 0.0f), 6.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(0.0f, 11.0f, 0.0f), 6.0f, SOUND_BOUMm, 0.45f);
		SetGlobalSphere(D3DVECTOR(0.0f, 5.0f, 0.0f), 6.0f);

		CreateShadowCircle(8.0f, 1.0f);
	}

	if ( m_type == OBJECT_ENERGY )
	{
		pModFile->ReadModel("objects\\energy.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);

		CreateCrashSphere(D3DVECTOR(-2.0f, 13.0f, 0.0f), 6.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-7.0f,  3.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 0.0f,  1.0f, 0.0f), 1.5f, SOUND_BOUMm, 0.45f);
		SetGlobalSphere(D3DVECTOR(-7.0f, 5.0f, 0.0f), 5.0f);

		m_character.posPower = D3DVECTOR(0.0f, 3.0f, 0.0f);
		m_energy = power;  // initialise le niveau d'�nergie

		CreateShadowCircle(6.0f, 0.5f);
	}

	if ( m_type == OBJECT_LABO )
	{
		pModFile->ReadModel("objects\\labo1.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(1, rank);
		SetObjectParent(1, 0);
		pModFile->ReadModel("objects\\labo2.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(1, D3DVECTOR(-9.0f, 3.0f, 0.0f));
		SetAngleZ(1, PI/2.0f);

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(2, rank);
		SetObjectParent(2, 1);
		pModFile->ReadModel("objects\\labo3.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(2, D3DVECTOR(9.0f, -1.0f, 0.0f));

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(3, rank);
		SetObjectParent(3, 2);
		pModFile->ReadModel("objects\\labo4.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(3, D3DVECTOR(0.0f, 0.0f, 0.0f));
		SetAngleZ(3, 80.0f*PI/180.0f);

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(4, rank);
		SetObjectParent(4, 2);
		pModFile->ReadModel("objects\\labo4.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(4, D3DVECTOR(0.0f, 0.0f, 0.0f));
		SetAngleZ(4, 80.0f*PI/180.0f);
		SetAngleY(4, PI*2.0f/3.0f);

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(5, rank);
		SetObjectParent(5, 2);
		pModFile->ReadModel("objects\\labo4.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(5, D3DVECTOR(0.0f, 0.0f, 0.0f));
		SetAngleZ(5, 80.0f*PI/180.0f);
		SetAngleY(5, -PI*2.0f/3.0f);

		CreateCrashSphere(D3DVECTOR(  0.0f,  1.0f,  0.0f), 1.5f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(  0.0f, 11.0f,  0.0f), 4.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-10.0f, 10.0f,  0.0f), 4.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-12.0f,  3.0f,  3.0f), 4.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-12.0f,  3.0f, -3.0f), 4.0f, SOUND_BOUMm, 0.45f);
		SetGlobalSphere(D3DVECTOR(-10.0f, 5.0f, 0.0f), 7.0f);

		m_character.posPower = D3DVECTOR(0.0f, 3.0f, 0.0f);

		CreateShadowCircle(7.0f, 0.5f);
	}

	if ( m_type == OBJECT_FACTORY )
	{
		pModFile->ReadModel("objects\\factory1.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);

		for ( i=0 ; i<9 ; i++ )
		{
			rank = m_engine->CreateObject();
			m_engine->SetObjectType(rank, TYPEDESCENDANT);
			SetObjectRank(1+i, rank);
			SetObjectParent(1+i, 0);
			pModFile->ReadModel("objects\\factory2.mod");
			pModFile->CreateEngineObject(rank);
			SetPosition(1+i, D3DVECTOR(10.0f, 2.0f*i, 10.0f));
			SetAngleZ(1+i, PI/2.0f);
			SetZoomZ(1+i, 0.30f);

			rank = m_engine->CreateObject();
			m_engine->SetObjectType(rank, TYPEDESCENDANT);
			SetObjectRank(10+i, rank);
			SetObjectParent(10+i, 0);
			pModFile->ReadModel("objects\\factory2.mod");
			pModFile->CreateEngineObject(rank);
			SetPosition(10+i, D3DVECTOR(10.0f, 2.0f*i, -10.0f));
			SetAngleZ(10+i, -PI/2.0f);
			SetAngleY(10+i, PI);
			SetZoomZ(10+i, 0.30f);
		}

		for ( i=0 ; i<2 ; i++ )
		{
			float s = (float)(i*2-1);
			CreateCrashSphere(D3DVECTOR(-10.0f,  2.0f, 11.0f*s), 4.0f, SOUND_BOUMm, 0.45f);
			CreateCrashSphere(D3DVECTOR( -3.0f,  2.0f, 11.0f*s), 4.0f, SOUND_BOUMm, 0.45f);
			CreateCrashSphere(D3DVECTOR(  3.0f,  2.0f, 11.0f*s), 4.0f, SOUND_BOUMm, 0.45f);
			CreateCrashSphere(D3DVECTOR( 10.0f,  2.0f, 11.0f*s), 4.0f, SOUND_BOUMm, 0.45f);
			CreateCrashSphere(D3DVECTOR(-10.0f,  9.0f, 11.0f*s), 4.0f, SOUND_BOUMm, 0.45f);
			CreateCrashSphere(D3DVECTOR( -3.0f,  9.0f, 11.0f*s), 4.0f, SOUND_BOUMm, 0.45f);
			CreateCrashSphere(D3DVECTOR(  3.0f,  9.0f, 11.0f*s), 4.0f, SOUND_BOUMm, 0.45f);
			CreateCrashSphere(D3DVECTOR( 10.0f,  9.0f, 11.0f*s), 4.0f, SOUND_BOUMm, 0.45f);
			CreateCrashSphere(D3DVECTOR(-10.0f, 16.0f, 11.0f*s), 4.0f, SOUND_BOUMm, 0.45f);
			CreateCrashSphere(D3DVECTOR( -3.0f, 16.0f, 11.0f*s), 4.0f, SOUND_BOUMm, 0.45f);
			CreateCrashSphere(D3DVECTOR(  3.0f, 16.0f, 11.0f*s), 4.0f, SOUND_BOUMm, 0.45f);
			CreateCrashSphere(D3DVECTOR( 10.0f, 16.0f, 11.0f*s), 4.0f, SOUND_BOUMm, 0.45f);
			CreateCrashSphere(D3DVECTOR(-10.0f, 16.0f,  4.0f*s), 4.0f, SOUND_BOUMm, 0.45f);
			CreateCrashSphere(D3DVECTOR( -3.0f, 16.0f,  4.0f*s), 4.0f, SOUND_BOUMm, 0.45f);
			CreateCrashSphere(D3DVECTOR(  3.0f, 16.0f,  4.0f*s), 4.0f, SOUND_BOUMm, 0.45f);
			CreateCrashSphere(D3DVECTOR( 10.0f, 16.0f,  4.0f*s), 4.0f, SOUND_BOUMm, 0.45f);
			CreateCrashSphere(D3DVECTOR(-10.0f,  2.0f,  4.0f*s), 4.0f, SOUND_BOUMm, 0.45f);
			CreateCrashSphere(D3DVECTOR(-10.0f,  9.0f,  4.0f*s), 4.0f, SOUND_BOUMm, 0.45f);
		}
		CreateCrashSphere(D3DVECTOR(-10.0f, 21.0f, -4.0f), 3.0f, SOUND_BOUMm, 0.45f);
		SetGlobalSphere(D3DVECTOR(0.0f, 10.0f, 0.0f), 18.0f);

		CreateShadowCircle(24.0f, 0.3f);
	}

	if ( m_type == OBJECT_REPAIR )
	{
		pModFile->ReadModel("objects\\repair1.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(1, rank);
		SetObjectParent(1, 0);
		pModFile->ReadModel("objects\\repair2.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(1, D3DVECTOR(-11.0f, 13.5f, 0.0f));
		SetAngleZ(1, PI/2.0f);

		m_terrain->AddBuildingLevel(pos, 7.0f, 9.0f, 1.0f, 0.5f);

		CreateCrashSphere(D3DVECTOR(-11.0f,  0.0f,  4.0f), 5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-11.0f,  0.0f,  0.0f), 5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-11.0f,  0.0f, -4.0f), 5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-11.0f, 10.0f,  0.0f), 5.0f, SOUND_BOUMm, 0.45f);
		SetGlobalSphere(D3DVECTOR(-11.0f, 13.0f, 0.0f), 15.0f);
	}

	if ( m_type == OBJECT_DESTROYER )
	{
		pModFile->ReadModel("objects\\destroy1.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(1, rank);
		SetObjectParent(1, 0);
		pModFile->ReadModel("objects\\destroy2.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(1, D3DVECTOR(0.0f, 0.0f, 0.0f));

		m_terrain->AddBuildingLevel(pos, 7.0f, 9.0f, 1.0f, 0.5f);

		CreateCrashSphere(D3DVECTOR(-3.5f, 0.0f, -13.5f), 4.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 3.5f, 0.0f, -13.5f), 4.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-3.5f, 0.0f,  13.5f), 4.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 3.5f, 0.0f,  13.5f), 4.0f, SOUND_BOUMm, 0.45f);

		CreateShadowCircle(19.0f, 1.0f);
	}

	if ( m_type == OBJECT_STATION )
	{
		pModFile->ReadModel("objects\\station.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);

		m_terrain->AddBuildingLevel(pos, 7.0f, 9.0f, 1.0f, 0.5f);

		CreateCrashSphere(D3DVECTOR(-15.0f, 2.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-15.0f, 6.0f, 0.0f), 4.0f, SOUND_BOUMm, 0.45f);
		SetGlobalSphere(D3DVECTOR(-15.0f, 5.0f, 0.0f), 6.0f);

		m_energy = power;  // initialise le niveau d'�nergie
	}

	if ( m_type == OBJECT_CONVERT )
	{
		pModFile->ReadModel("objects\\convert1.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(1, rank);
		SetObjectParent(1, 0);
		pModFile->ReadModel("objects\\convert2.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(1, D3DVECTOR(0.0f, 14.0f, 0.0f));

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(2, rank);
		SetObjectParent(2, 0);
		pModFile->ReadModel("objects\\convert3.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(2, D3DVECTOR(0.0f, 11.5f, 0.0f));
		SetAngleX(2, -PI*0.35f);

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(3, rank);
		SetObjectParent(3, 0);
		pModFile->ReadModel("objects\\convert3.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(3, D3DVECTOR(0.0f, 11.5f, 0.0f));
		SetAngleY(3, PI);
		SetAngleX(3, -PI*0.35f);

		m_terrain->AddBuildingLevel(pos, 7.0f, 9.0f, 1.0f, 0.5f);

		CreateCrashSphere(D3DVECTOR(-10.0f,  2.0f,  4.0f), 5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-10.0f,  2.0f, -4.0f), 5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-10.0f,  9.0f,  0.0f), 6.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(  0.0f, 14.0f,  0.0f), 1.5f, SOUND_BOUMm, 0.45f);
		SetGlobalSphere(D3DVECTOR(-3.0f, 8.0f, 0.0f), 14.0f);
	}

	if ( m_type == OBJECT_TOWER )
	{
		pModFile->ReadModel("objects\\tower.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(1, rank);
		SetObjectParent(1, 0);
		pModFile->ReadModel("objects\\roller2c.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(1, D3DVECTOR(0.0f, 20.0f, 0.0f));
		SetAngleZ(1, PI/2.0f);

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(2, rank);
		SetObjectParent(2, 1);
		pModFile->ReadModel("objects\\roller3c.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(2, D3DVECTOR(4.5f, 0.0f, 0.0f));
		SetAngleZ(2, 0.0f);

		CreateCrashSphere(D3DVECTOR(0.0f,  0.0f, 0.0f), 6.5f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(0.0f,  8.0f, 0.0f), 4.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(0.0f, 15.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(0.0f, 24.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
		SetGlobalSphere(D3DVECTOR(0.0f, 5.0f, 0.0f), 7.0f);

		m_character.posPower = D3DVECTOR(5.0f, 3.0f, 0.0f);

		CreateShadowCircle(6.0f, 1.0f);
		m_showLimitRadius = BLITZPARA;
	}

	if ( m_type == OBJECT_NUCLEAR )
	{
		pModFile->ReadModel("objects\\nuclear1.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(1, rank);
		SetObjectParent(1, 0);
		pModFile->ReadModel("objects\\nuclear2.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(1, D3DVECTOR(20.0f, 10.0f, 0.0f));
		SetAngleZ(1, 135.0f*PI/180.0f);

		CreateCrashSphere(D3DVECTOR( 0.0f,  0.0f, 0.0f), 19.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 0.0f, 24.0f, 0.0f), 15.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(22.0f,  1.0f, 0.0f),  1.5f, SOUND_BOUMm, 0.45f);
		SetGlobalSphere(D3DVECTOR(0.0f, 17.0f, 0.0f), 26.0f);

		m_character.posPower = D3DVECTOR(22.0f, 3.0f, 0.0f);

		CreateShadowCircle(21.0f, 1.0f);
	}

	if ( m_type == OBJECT_PARA )
	{
		pModFile->ReadModel("objects\\para.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);

		m_terrain->AddBuildingLevel(pos, 16.0f, 18.0f, 1.0f, 0.5f);

		CreateCrashSphere(D3DVECTOR( 13.0f,  3.0f,  13.0f),  3.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 11.0f, 15.0f,  11.0f),  2.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-13.0f,  3.0f,  13.0f),  3.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-11.0f, 15.0f, -11.0f),  2.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 13.0f,  3.0f, -13.0f),  3.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 11.0f, 15.0f, -11.0f),  2.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-13.0f,  3.0f, -13.0f),  3.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-11.0f, 15.0f, -11.0f),  2.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(  0.0f, 26.0f,   0.0f),  9.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(  0.0f, 54.0f,   0.0f), 14.0f, SOUND_BOUMm, 0.45f);
		SetGlobalSphere(D3DVECTOR(0.0f, 10.0f, 0.0f), 20.0f);

		CreateShadowCircle(21.0f, 1.0f);
		m_showLimitRadius = BLITZPARA;
	}

	if ( m_type == OBJECT_SAFE )
	{
		pModFile->ReadModel("objects\\safe1.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(1, rank);
		SetObjectParent(1, 0);
		pModFile->ReadModel("objects\\safe2.mod");
		pModFile->CreateEngineObject(rank);
		SetZoom(1, 1.05f);

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(2, rank);
		SetObjectParent(2, 0);
		pModFile->ReadModel("objects\\safe3.mod");
		pModFile->CreateEngineObject(rank);
		SetZoom(2, 1.05f);

		m_terrain->AddBuildingLevel(pos, 18.0f, 20.0f, 1.0f, 0.5f);

		CreateCrashSphere(D3DVECTOR(0.0f, 1.0f, 0.0f), 13.0f, SOUND_BOUMm, 0.45f);
		SetGlobalSphere(D3DVECTOR(0.0f, 1.0f, 0.0f), 13.0f);

		CreateShadowCircle(23.0f, 1.0f);
	}

	if ( m_type == OBJECT_HUSTON )
	{
		pModFile->ReadModel("objects\\huston1.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(1, rank);
		SetObjectParent(1, 0);
		pModFile->ReadModel("objects\\huston2.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(1, D3DVECTOR(0.0f, 39.0f, 30.0f));
		SetAngleY(1, -PI/2.0f);
		SetZoom(1, 3.0f);

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(2, rank);
		SetObjectParent(2, 1);
		pModFile->ReadModel("objects\\huston3.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(2, D3DVECTOR(0.0f, 4.5f, 1.9f));

		CreateCrashSphere(D3DVECTOR( 15.0f,  6.0f, -53.0f), 16.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-15.0f,  6.0f, -53.0f), 16.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 15.0f,  6.0f, -26.0f), 16.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-15.0f,  6.0f, -26.0f), 16.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 15.0f,  6.0f,   0.0f), 16.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-15.0f,  6.0f,   0.0f), 16.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 15.0f,  6.0f,  26.0f), 16.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-15.0f,  6.0f,  26.0f), 16.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 15.0f,  6.0f,  53.0f), 16.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-15.0f,  6.0f,  53.0f), 16.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(  0.0f, 27.0f,  30.0f), 12.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(  0.0f, 45.0f,  30.0f), 14.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 26.0f,  4.0f, -61.0f),  5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-26.0f,  4.0f, -61.0f),  5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 26.0f,  4.0f,  61.0f),  5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-26.0f,  4.0f,  61.0f),  5.0f, SOUND_BOUMm, 0.45f);
	}

	if ( m_type == OBJECT_TARGET1 )
	{
		pModFile->ReadModel("objects\\target1.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetZoom(0, 1.5f);
		SetFloorHeight(0.0f);

		CreateCrashSphere(D3DVECTOR(  0.0f, 50.0f+14.0f, 0.0f),  3.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( -7.0f, 50.0f+12.0f, 0.0f),  3.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(  7.0f, 50.0f+12.0f, 0.0f),  3.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-12.0f, 50.0f+ 7.0f, 0.0f),  3.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 12.0f, 50.0f+ 7.0f, 0.0f),  3.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-14.0f, 50.0f+ 0.0f, 0.0f),  3.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 14.0f, 50.0f+ 0.0f, 0.0f),  3.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-12.0f, 50.0f- 7.0f, 0.0f),  3.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 12.0f, 50.0f- 7.0f, 0.0f),  3.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( -7.0f, 50.0f-12.0f, 0.0f),  3.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(  7.0f, 50.0f-12.0f, 0.0f),  3.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(  0.0f, 50.0f-14.0f, 0.0f),  3.0f, SOUND_BOUMm, 0.45f);

		CreateCrashSphere(D3DVECTOR(0.0f, 30.0f, 0.0f), 2.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(0.0f, 24.0f, 0.0f), 3.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(0.0f, 16.0f, 0.0f), 4.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(0.0f,  4.0f, 0.0f), 8.0f, SOUND_BOUMm, 0.45f);

		CreateShadowCircle(15.0f, 1.0f);
	}

	if ( m_type == OBJECT_TARGET2 )
	{
		pModFile->ReadModel("objects\\target2.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);

		height += 50.0f*1.5f;
	}

	if ( m_type == OBJECT_NEST )
	{
		pModFile->ReadModel("objects\\nest.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);

		m_terrain->AddBuildingLevel(pos, 3.0f, 5.0f, 1.0f, 0.5f);

		CreateShadowCircle(4.0f, 1.0f);
	}

	if ( m_type == OBJECT_START )
	{
		pModFile->ReadModel("objects\\start.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);

		m_terrain->AddBuildingLevel(pos, 7.0f, 9.0f, 1.0f, 0.5f);
	}

	if ( m_type == OBJECT_END )
	{
		pModFile->ReadModel("objects\\end.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);

		m_terrain->AddBuildingLevel(pos, 7.0f, 9.0f, 1.0f, 0.5f);
	}

#if 0
	if ( power > 0.0f )  // cr�e une pile ?
	{
		CObject*	pPower;

		pPower = new CObject(m_iMan);
		pPower->SetType(power<=1.0f?OBJECT_POWER:OBJECT_ATOMIC);

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		pPower->SetObjectRank(0, rank);

		if ( power <= 1.0f )  pModFile->ReadModel("objects\\power.mod");
		else                  pModFile->ReadModel("objects\\atomic.mod");
		pModFile->CreateEngineObject(rank);

		pPower->SetPosition(0, RetCharacter()->posPower);
		pPower->CreateCrashSphere(D3DVECTOR(0.0f, 1.0f, 0.0f), 1.0f, SOUND_BOUMm, 0.45f);
		pPower->SetGlobalSphere(D3DVECTOR(0.0f, 1.0f, 0.0f), 1.5f);

		pPower->SetTruck(this);
		SetPower(pPower);

		if ( power <= 1.0f )  pPower->SetEnergy(power);
		else                  pPower->SetEnergy(power/100.0f);
	}
#endif

	pos = RetPosition(0);
	pos.y += height;
	SetPosition(0, pos);  // pour afficher les ombres tout de suite

	CreateOtherObject(type);
	m_engine->LoadAllTexture();

	delete pModFile;
	return TRUE;
}

// Cr�e une petite ressource pos�e sur le sol.

BOOL CObject::CreateResource(D3DVECTOR pos, float angle, ObjectType type,
							 float power)
{
	CModFile*	pModFile;
	char		name[50];
	int			rank;
	float		radius, height;

	if ( type != OBJECT_SHOW )
	{
		if ( m_engine->RetRestCreate() < 1 )  return FALSE;
	}

	pModFile = new CModFile(m_iMan);

	SetType(type);

	rank = m_engine->CreateObject();
	m_engine->SetObjectType(rank, TYPEFIX);  // c'est un objet fixe
	SetObjectRank(0, rank);
	SetEnergy(power);

	name[0] = 0;
	if ( type == OBJECT_STONE       )  strcpy(name, "objects\\stone.mod");
	if ( type == OBJECT_URANIUM     )  strcpy(name, "objects\\uranium.mod");
	if ( type == OBJECT_METAL       )  strcpy(name, "objects\\metal.mod");
	if ( type == OBJECT_POWER       )  strcpy(name, "objects\\power.mod");
	if ( type == OBJECT_ATOMIC      )  strcpy(name, "objects\\atomic.mod");
	if ( type == OBJECT_BULLET      )  strcpy(name, "objects\\bullet.mod");
	if ( type == OBJECT_BBOX        )  strcpy(name, "objects\\bbox.mod");
	if ( type == OBJECT_KEYa        )  strcpy(name, "objects\\keya.mod");
	if ( type == OBJECT_KEYb        )  strcpy(name, "objects\\keyb.mod");
	if ( type == OBJECT_KEYc        )  strcpy(name, "objects\\keyc.mod");
	if ( type == OBJECT_KEYd        )  strcpy(name, "objects\\keyd.mod");
	if ( type == OBJECT_TNT         )  strcpy(name, "objects\\tnt.mod");
	if ( type == OBJECT_SCRAP1      )  strcpy(name, "objects\\scrap1.mod");
	if ( type == OBJECT_SCRAP2      )  strcpy(name, "objects\\scrap2.mod");
	if ( type == OBJECT_SCRAP3      )  strcpy(name, "objects\\scrap3.mod");
	if ( type == OBJECT_SCRAP4      )  strcpy(name, "objects\\scrap4.mod");
	if ( type == OBJECT_SCRAP5      )  strcpy(name, "objects\\scrap5.mod");
	if ( type == OBJECT_BOMB        )  strcpy(name, "objects\\bomb.mod");
	if ( type == OBJECT_WAYPOINT    )  strcpy(name, "objects\\waypoint.mod");
	if ( type == OBJECT_SHOW        )  strcpy(name, "objects\\show.mod");
	if ( type == OBJECT_WINFIRE     )  strcpy(name, "objects\\winfire.mod");
	if ( type == OBJECT_BAG         )  strcpy(name, "objects\\bag.mod");
	if ( type == OBJECT_MARKSTONE   )  strcpy(name, "objects\\cross1.mod");
	if ( type == OBJECT_MARKURANIUM )  strcpy(name, "objects\\cross3.mod");
	if ( type == OBJECT_MARKPOWER   )  strcpy(name, "objects\\cross2.mod");
	if ( type == OBJECT_MARKKEYa    )  strcpy(name, "objects\\crossa.mod");
	if ( type == OBJECT_MARKKEYb    )  strcpy(name, "objects\\crossb.mod");
	if ( type == OBJECT_MARKKEYc    )  strcpy(name, "objects\\crossc.mod");
	if ( type == OBJECT_MARKKEYd    )  strcpy(name, "objects\\crossd.mod");
	if ( type == OBJECT_EGG         )  strcpy(name, "objects\\egg.mod");

	pModFile->ReadModel(name);
	pModFile->CreateEngineObject(rank);

	SetPosition(0, pos);
	SetAngleY(0, angle);

	if ( type == OBJECT_SHOW )  // reste en l'air ?
	{
		delete pModFile;
		return TRUE;
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
		CreateCrashSphere(D3DVECTOR(-1.0f, 2.8f, 0.0f), 3.0f, SOUND_BOUMm, 0.45f);
		SetGlobalSphere(D3DVECTOR(0.0f, 5.0f, 0.0f), 10.0f);
		radius = 3.0f;
	}
	else if ( type == OBJECT_BOMB )
	{
		CreateCrashSphere(D3DVECTOR(0.0f, 0.0f, 0.0f), 3.0f, SOUND_BOUMm, 0.45f);
		SetGlobalSphere(D3DVECTOR(0.0f, 0.0f, 0.0f), 3.0f);
		radius = 3.0f;
	}
	else if ( type == OBJECT_BAG )
	{
		CreateCrashSphere(D3DVECTOR(0.0f, 0.0f, 0.0f), 4.0f, SOUND_BOUMm, 0.45f);
		SetGlobalSphere(D3DVECTOR(0.0f, 0.0f, 0.0f), 4.0f);
		SetZoom(0, 1.5f);
		radius =  5.0f;
		height = -1.4f;
	}
	else
	{
		CreateCrashSphere(D3DVECTOR(0.0f, 1.0f, 0.0f), 1.0f, SOUND_BOUMm, 0.45f);
		SetGlobalSphere(D3DVECTOR(0.0f, 1.0f, 0.0f), 1.5f);
	}
	CreateShadowCircle(radius, 1.0f);

	SetFloorHeight(0.0f);
	CreateOtherObject(type);
	m_engine->LoadAllTexture();
	FloorAdjust();

	pos = RetPosition(0);
	pos.y += height;
	SetPosition(0, pos);  // pour afficher les ombres tout de suite

	delete pModFile;
	return TRUE;
}

// Cr�e un drapeau pos� sur le sol.

BOOL CObject::CreateFlag(D3DVECTOR pos, float angle, ObjectType type)
{
	CModFile*	pModFile;
	char		name[50];
	int			rank, i;

	if ( m_engine->RetRestCreate() < 1+4 )  return FALSE;

	pModFile = new CModFile(m_iMan);

	SetType(type);

	name[0] = 0;
	if ( type == OBJECT_FLAGb )  strcpy(name, "objects\\flag1b.mod");
	if ( type == OBJECT_FLAGr )  strcpy(name, "objects\\flag1r.mod");
	if ( type == OBJECT_FLAGg )  strcpy(name, "objects\\flag1g.mod");
	if ( type == OBJECT_FLAGy )  strcpy(name, "objects\\flag1y.mod");
	if ( type == OBJECT_FLAGv )  strcpy(name, "objects\\flag1v.mod");

	rank = m_engine->CreateObject();
	m_engine->SetObjectType(rank, TYPEFIX);  // c'est un objet fixe
	SetObjectRank(0, rank);
	pModFile->ReadModel(name);
	pModFile->CreateEngineObject(rank);
	SetPosition(0, pos);
	SetAngleY(0, angle);

	name[0] = 0;
	if ( type == OBJECT_FLAGb )  strcpy(name, "objects\\flag2b.mod");
	if ( type == OBJECT_FLAGr )  strcpy(name, "objects\\flag2r.mod");
	if ( type == OBJECT_FLAGg )  strcpy(name, "objects\\flag2g.mod");
	if ( type == OBJECT_FLAGy )  strcpy(name, "objects\\flag2y.mod");
	if ( type == OBJECT_FLAGv )  strcpy(name, "objects\\flag2v.mod");

	for ( i=0 ; i<4 ; i++ )
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(1+i, rank);
		SetObjectParent(1+i, i);
		pModFile->ReadModel(name);
		pModFile->CreateEngineObject(rank);
		if ( i == 0 )  SetPosition(1+i, D3DVECTOR(0.15f, 5.0f, 0.0f));
		else           SetPosition(1+i, D3DVECTOR(0.79f, 0.0f, 0.0f));
	}

	SetJotlerSphere(D3DVECTOR(0.0f, 4.0f, 0.0f), 1.0f);
	CreateShadowCircle(2.0f, 0.3f);

	SetFloorHeight(0.0f);
	CreateOtherObject(type);
	m_engine->LoadAllTexture();
	FloorAdjust();

	pos = RetPosition(0);
	SetPosition(0, pos);  // pour afficher les ombres tout de suite

	delete pModFile;
	return TRUE;
}

// Cr�e une barri�re pos�e sur le sol.

BOOL CObject::CreateBarrier(D3DVECTOR pos, float angle, float height,
							ObjectType type)
{
	CModFile*	pModFile;
	int			rank;

	if ( m_engine->RetRestCreate() < 1 )  return FALSE;

	pModFile = new CModFile(m_iMan);

	SetType(type);

	if ( type == OBJECT_BARRIER0 )
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\barrier0.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);

		CreateCrashSphere(D3DVECTOR( 3.5f, 3.0f, 0.0f), 0.7f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 0.0f, 3.0f, 0.0f), 0.7f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-3.5f, 3.0f, 0.0f), 0.7f, SOUND_BOUMm, 0.45f);

		CreateShadowCircle(6.0f, 0.5f, D3DSHADOWWORM);
	}

	if ( type == OBJECT_BARRIER1 )
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\barrier1.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);

		CreateCrashSphere(D3DVECTOR( 8.5f, 3.0f, 0.0f), 0.7f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 3.5f, 3.0f, 0.0f), 0.7f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 0.0f, 3.0f, 0.0f), 0.7f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-3.5f, 3.0f, 0.0f), 0.7f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-8.5f, 3.0f, 0.0f), 0.7f, SOUND_BOUMm, 0.45f);

		CreateShadowCircle(12.0f, 0.5f, D3DSHADOWWORM);
	}

	if ( type == OBJECT_BARRIER2 )  // en carton ?
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\barrier2.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);

		CreateCrashSphere(D3DVECTOR( 8.5f, 3.0f, 0.0f), 0.7f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 3.5f, 3.0f, 0.0f), 0.7f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 0.0f, 3.0f, 0.0f), 0.7f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-3.5f, 3.0f, 0.0f), 0.7f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-8.5f, 3.0f, 0.0f), 0.7f, SOUND_BOUMm, 0.45f);

		CreateShadowCircle(12.0f, 0.8f, D3DSHADOWWORM);
	}

	if ( type == OBJECT_BARRIER3 )  // allumettes + paille ?
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\barrier3.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);

		CreateCrashSphere(D3DVECTOR( 8.5f, 3.0f, 0.0f), 0.7f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 3.5f, 3.0f, 0.0f), 0.7f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 0.0f, 3.0f, 0.0f), 0.7f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-3.5f, 3.0f, 0.0f), 0.7f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-8.5f, 3.0f, 0.0f), 0.7f, SOUND_BOUMm, 0.45f);

		CreateShadowCircle(10.0f, 0.5f, D3DSHADOWWORM);
	}

	pos = RetPosition(0);
	SetPosition(0, pos);  // pour afficher les ombres tout de suite

	SetFloorHeight(0.0f);
	CreateOtherObject(type);
	FloorAdjust();

	pos = RetPosition(0);
	pos.y += height;
	SetPosition(0, pos);

	delete pModFile;
	return TRUE;
}

// Cr�e une plante pos�e sur le sol.

BOOL CObject::CreatePlant(D3DVECTOR pos, float angle, float height,
						  ObjectType type)
{
	CModFile*	pModFile;
	int			rank;

	if ( m_engine->RetRestCreate() < 1 )  return FALSE;

	pModFile = new CModFile(m_iMan);

	SetType(type);

	if ( type == OBJECT_PLANT0 ||
		 type == OBJECT_PLANT1 ||
		 type == OBJECT_PLANT2 ||
		 type == OBJECT_PLANT3 ||
		 type == OBJECT_PLANT4 )  // standard ?
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		if ( type == OBJECT_PLANT0 )  pModFile->ReadModel("objects\\plant0.mod");
		if ( type == OBJECT_PLANT1 )  pModFile->ReadModel("objects\\plant1.mod");
		if ( type == OBJECT_PLANT2 )  pModFile->ReadModel("objects\\plant2.mod");
		if ( type == OBJECT_PLANT3 )  pModFile->ReadModel("objects\\plant3.mod");
		if ( type == OBJECT_PLANT4 )  pModFile->ReadModel("objects\\plant4.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);

		height -= 2.0f;

		CreateCrashSphere(D3DVECTOR(0.0f, 0.0f, 0.0f), 4.0f, SOUND_BOUM, 0.10f);
		SetGlobalSphere(D3DVECTOR(0.0f, 3.0f, 0.0f), 6.0f);
		SetJotlerSphere(D3DVECTOR(0.0f, 0.0f, 0.0f), 8.0f);

		CreateShadowCircle(8.0f, 0.5f);
	}

	if ( type == OBJECT_PLANT5 ||
		 type == OBJECT_PLANT6 ||
		 type == OBJECT_PLANT7 )  // tr�fle ?
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		if ( type == OBJECT_PLANT5 )  pModFile->ReadModel("objects\\plant5.mod");
		if ( type == OBJECT_PLANT6 )  pModFile->ReadModel("objects\\plant6.mod");
		if ( type == OBJECT_PLANT7 )  pModFile->ReadModel("objects\\plant7.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);

//?		CreateCrashSphere(D3DVECTOR(0.0f, 0.0f, 0.0f), 3.0f, SOUND_BOUM, 0.10f);
		SetJotlerSphere(D3DVECTOR(0.0f, 0.0f, 0.0f), 4.0f);

		CreateShadowCircle(5.0f, 0.3f);
	}

	if ( type == OBJECT_PLANT8 ||
		 type == OBJECT_PLANT9 )  // courgette ?
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		if ( type == OBJECT_PLANT8 )  pModFile->ReadModel("objects\\plant8.mod");
		if ( type == OBJECT_PLANT9 )  pModFile->ReadModel("objects\\plant9.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);

		CreateCrashSphere(D3DVECTOR(0.0f,  2.0f, 0.0f), 4.0f, SOUND_BOUM, 0.10f);
		CreateCrashSphere(D3DVECTOR(0.0f, 10.0f, 0.0f), 4.0f, SOUND_BOUM, 0.10f);

		CreateShadowCircle(10.0f, 0.5f);
	}

	if ( type == OBJECT_PLANT10 ||
		 type == OBJECT_PLANT11 ||
		 type == OBJECT_PLANT12 ||
		 type == OBJECT_PLANT13 ||
		 type == OBJECT_PLANT14 )  // plante grasse ?
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		if ( type == OBJECT_PLANT10 )  pModFile->ReadModel("objects\\plant10.mod");
		if ( type == OBJECT_PLANT11 )  pModFile->ReadModel("objects\\plant11.mod");
		if ( type == OBJECT_PLANT12 )  pModFile->ReadModel("objects\\plant12.mod");
		if ( type == OBJECT_PLANT13 )  pModFile->ReadModel("objects\\plant13.mod");
		if ( type == OBJECT_PLANT14 )  pModFile->ReadModel("objects\\plant14.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);

		CreateCrashSphere(D3DVECTOR(0.0f, 12.0f, 0.0f), 5.0f, SOUND_BOUM, 0.10f);
		SetGlobalSphere(D3DVECTOR(0.0f, 6.0f, 0.0f), 6.0f);
		SetJotlerSphere(D3DVECTOR(0.0f, 4.0f, 0.0f), 8.0f);

		CreateShadowCircle(8.0f, 0.3f);
	}

	if ( type == OBJECT_PLANT15 ||
		 type == OBJECT_PLANT16 ||
		 type == OBJECT_PLANT17 ||
		 type == OBJECT_PLANT18 ||
		 type == OBJECT_PLANT19 )  // foug�re ?
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		if ( type == OBJECT_PLANT15 )  pModFile->ReadModel("objects\\plant15.mod");
		if ( type == OBJECT_PLANT16 )  pModFile->ReadModel("objects\\plant16.mod");
		if ( type == OBJECT_PLANT17 )  pModFile->ReadModel("objects\\plant17.mod");
		if ( type == OBJECT_PLANT18 )  pModFile->ReadModel("objects\\plant18.mod");
		if ( type == OBJECT_PLANT19 )  pModFile->ReadModel("objects\\plant19.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);

		if ( type != OBJECT_PLANT19 )
		{
			CreateCrashSphere(D3DVECTOR(0.0f, 0.0f, 0.0f), 4.0f, SOUND_BOUM, 0.10f);
			SetGlobalSphere(D3DVECTOR(0.0f, 3.0f, 0.0f), 6.0f);
		}
		SetJotlerSphere(D3DVECTOR(0.0f, 0.0f, 0.0f), 8.0f);

		CreateShadowCircle(8.0f, 0.5f);
	}

	if ( type == OBJECT_TREE0 )
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\tree0.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);

		CreateCrashSphere(D3DVECTOR( 0.0f,  3.0f, 2.0f), 3.0f, SOUND_BOUMs, 0.20f);
		CreateCrashSphere(D3DVECTOR(-1.0f, 10.0f, 1.0f), 2.0f, SOUND_BOUMs, 0.20f);
		CreateCrashSphere(D3DVECTOR( 0.0f, 17.0f, 0.0f), 2.0f, SOUND_BOUMs, 0.20f);
		CreateCrashSphere(D3DVECTOR( 1.0f, 27.0f, 0.0f), 2.0f, SOUND_BOUMs, 0.20f);

		CreateShadowCircle(8.0f, 0.5f);
	}

	if ( type == OBJECT_TREE1 )
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\tree1.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);

		CreateCrashSphere(D3DVECTOR( 0.0f,  3.0f, 2.0f), 3.0f, SOUND_BOUMs, 0.20f);
		CreateCrashSphere(D3DVECTOR(-2.0f, 11.0f, 1.0f), 2.0f, SOUND_BOUMs, 0.20f);
		CreateCrashSphere(D3DVECTOR(-2.0f, 19.0f, 2.0f), 2.0f, SOUND_BOUMs, 0.20f);
		CreateCrashSphere(D3DVECTOR( 2.0f, 26.0f, 0.0f), 2.0f, SOUND_BOUMs, 0.20f);
		CreateCrashSphere(D3DVECTOR( 2.0f, 34.0f,-2.0f), 2.0f, SOUND_BOUMs, 0.20f);

		CreateShadowCircle(8.0f, 0.5f);
	}

	if ( type == OBJECT_TREE2 )
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\tree2.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);

		CreateCrashSphere(D3DVECTOR( 0.0f,  3.0f, 1.0f), 3.0f, SOUND_BOUMs, 0.20f);
		CreateCrashSphere(D3DVECTOR(-2.0f, 10.0f, 1.0f), 2.0f, SOUND_BOUMs, 0.20f);
		CreateCrashSphere(D3DVECTOR(-2.0f, 19.0f, 2.0f), 2.0f, SOUND_BOUMs, 0.20f);
		CreateCrashSphere(D3DVECTOR( 2.0f, 25.0f, 0.0f), 2.0f, SOUND_BOUMs, 0.20f);
		CreateCrashSphere(D3DVECTOR( 3.0f, 32.0f,-2.0f), 2.0f, SOUND_BOUMs, 0.20f);

		CreateShadowCircle(8.0f, 0.5f);
	}

	if ( type == OBJECT_TREE3 )
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\tree3.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);

		CreateCrashSphere(D3DVECTOR(-2.0f,  3.0f, 2.0f), 3.0f, SOUND_BOUMs, 0.20f);
		CreateCrashSphere(D3DVECTOR(-3.0f,  9.0f, 1.0f), 2.0f, SOUND_BOUMs, 0.20f);
		CreateCrashSphere(D3DVECTOR( 0.0f, 18.0f, 0.0f), 2.0f, SOUND_BOUMs, 0.20f);
		CreateCrashSphere(D3DVECTOR( 0.0f, 27.0f, 7.0f), 2.0f, SOUND_BOUMs, 0.20f);

		CreateShadowCircle(8.0f, 0.5f);
	}

	if ( type == OBJECT_TREE4 )
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\tree4.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);

		CreateCrashSphere(D3DVECTOR(0.0f, 10.0f, 0.0f), 10.0f, SOUND_BOUMs, 0.20f);
		CreateCrashSphere(D3DVECTOR(0.0f, 21.0f, 0.0f),  8.0f, SOUND_BOUMs, 0.20f);
		CreateCrashSphere(D3DVECTOR(0.0f, 32.0f, 0.0f),  7.0f, SOUND_BOUMs, 0.20f);

		CreateShadowCircle(8.0f, 0.5f);
	}

	if ( type == OBJECT_TREE5 )  // arbre g�ant (pour monde "teen")
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\tree5.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);

		CreateCrashSphere(D3DVECTOR(  0.0f, 5.0f,-10.0f), 25.0f, SOUND_BOUMs, 0.20f);
		CreateCrashSphere(D3DVECTOR(-65.0f, 5.0f, 65.0f), 20.0f, SOUND_BOUMs, 0.20f);
		CreateCrashSphere(D3DVECTOR( 38.0f, 5.0f, 21.0f), 18.0f, SOUND_BOUMs, 0.20f);

		CreateShadowCircle(50.0f, 0.5f);
	}

	pos = RetPosition(0);
	SetPosition(0, pos);  // pour afficher les ombres tout de suite

	SetFloorHeight(0.0f);
	CreateOtherObject(type);

	pos = RetPosition(0);
	pos.y += height;
	SetPosition(0, pos);

	delete pModFile;
	return TRUE;
}

// Cr�e un champignon pos� sur le sol.

BOOL CObject::CreateMushroom(D3DVECTOR pos, float angle, float height,
							 ObjectType type)
{
	CModFile*	pModFile;
	int			rank;

	if ( m_engine->RetRestCreate() < 1 )  return FALSE;

	pModFile = new CModFile(m_iMan);

	SetType(type);

	if ( type == OBJECT_MUSHROOM1 )
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\mush1.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);

		CreateCrashSphere(D3DVECTOR(0.0f, 4.0f, 0.0f), 3.0f, SOUND_BOUM, 0.10f);
		SetGlobalSphere(D3DVECTOR(0.0f, 3.0f, 0.0f), 5.5f);
		SetJotlerSphere(D3DVECTOR(0.0f, 3.0f, 0.0f), 5.5f);

		CreateShadowCircle(6.0f, 0.5f);
	}

	if ( type == OBJECT_MUSHROOM2 )
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\mush2.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);

		CreateCrashSphere(D3DVECTOR(0.0f, 5.0f, 0.0f), 3.0f, SOUND_BOUM, 0.10f);
		SetGlobalSphere(D3DVECTOR(0.0f, 4.0f, 0.0f), 5.5f);
		SetJotlerSphere(D3DVECTOR(0.0f, 4.0f, 0.0f), 5.5f);

		CreateShadowCircle(5.0f, 0.5f);
	}

	pos = RetPosition(0);
	SetPosition(0, pos);  // pour afficher les ombres tout de suite

	SetFloorHeight(0.0f);
	CreateOtherObject(type);

	pos = RetPosition(0);
	pos.y += height;
	SetPosition(0, pos);

	delete pModFile;
	return TRUE;
}

// Cr�e un jouet pos� sur le sol.

BOOL CObject::CreateTeen(D3DVECTOR pos, float angle, float zoom, float height,
						 ObjectType type)
{
	CModFile*		pModFile;
	D3DMATRIX*		mat;
	D3DCOLORVALUE	color;
	int				rank;
	float			fShadow;
	BOOL			bFloorAdjust = TRUE;

	if ( m_engine->RetRestCreate() < 1 )  return FALSE;

	pModFile = new CModFile(m_iMan);

	SetType(type);

	fShadow = Norm(1.0f-height/10.0f);

	if ( type == OBJECT_TEEN0 )  // crayon orange lg=10
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\teen0.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetZoom(0, zoom);

		CreateCrashSphere(D3DVECTOR( 5.0f, 1.0f, 0.0f), 1.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 2.5f, 1.0f, 0.0f), 1.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 0.0f, 1.0f, 0.0f), 1.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-2.5f, 1.0f, 0.0f), 1.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-5.0f, 1.0f, 0.0f), 1.0f, SOUND_BOUMm, 0.45f);

		CreateShadowCircle(5.0f, 0.8f*fShadow, D3DSHADOWWORM);
	}

	if ( type == OBJECT_TEEN1 )  // crayon bleu lg=14
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\teen1.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetZoom(0, zoom);

		CreateCrashSphere(D3DVECTOR( 6.0f, 1.0f, 0.0f), 1.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 4.0f, 1.0f, 0.0f), 1.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 2.0f, 1.0f, 0.0f), 1.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 0.0f, 1.0f, 0.0f), 1.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-2.0f, 1.0f, 0.0f), 1.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-4.0f, 1.0f, 0.0f), 1.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-6.0f, 1.0f, 0.0f), 1.0f, SOUND_BOUMm, 0.45f);

		CreateShadowCircle(6.0f, 0.8f*fShadow, D3DSHADOWWORM);
	}

	if ( type == OBJECT_TEEN2 )  // crayon rouge lg=16
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\teen2.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetZoom(0, zoom);

		CreateCrashSphere(D3DVECTOR( 7.0f, 1.0f, 0.0f), 1.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 4.7f, 1.0f, 0.0f), 1.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 2.3f, 1.0f, 0.0f), 1.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 0.0f, 1.0f, 0.0f), 1.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-2.3f, 1.0f, 0.0f), 1.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-4.7f, 1.0f, 0.0f), 1.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-7.0f, 1.0f, 0.0f), 1.0f, SOUND_BOUMm, 0.45f);

		CreateShadowCircle(6.0f, 0.8f*fShadow, D3DSHADOWWORM);
	}

	if ( type == OBJECT_TEEN3 )  // bocal avec crayon
	{
		rank = m_engine->CreateObject();
//?		m_engine->SetObjectType(rank, TYPEFIX);
		m_engine->SetObjectType(rank, TYPEMETAL);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\teen3.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetZoom(0, zoom);

		CreateCrashSphere(D3DVECTOR( 0.0f, 4.0f, 0.0f), 4.0f, SOUND_BOUMm, 0.45f);
		SetGlobalSphere(D3DVECTOR(0.0f, 4.0f, 0.0f), 4.0f);
		CreateShadowCircle(6.0f, 0.5f*fShadow);
	}

	if ( type == OBJECT_TEEN4 )  // ciseaux
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\teen4.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetZoom(0, zoom);

		CreateCrashSphere(D3DVECTOR(-9.0f, 1.0f, 0.0f), 1.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-6.0f, 1.0f, 0.0f), 1.1f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-3.0f, 1.0f, 0.0f), 1.2f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 0.0f, 1.0f, 0.0f), 1.3f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 5.1f, 1.0f,-1.3f), 2.6f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 8.0f, 1.0f, 2.2f), 2.3f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 9.4f, 1.0f,-2.0f), 2.0f, SOUND_BOUMm, 0.45f);

		CreateShadowCircle(10.0f, 0.5f*fShadow, D3DSHADOWWORM);
	}

	if ( type == OBJECT_TEEN5 )  // CD
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\teen5.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetZoom(0, zoom);
		SetFloorHeight(0.0f);
		bFloorAdjust = FALSE;

		m_terrain->AddBuildingLevel(pos, 5.9f, 6.1f, 0.2f, 0.5f);
		CreateShadowCircle(8.0f, 0.2f*fShadow);
	}

	if ( type == OBJECT_TEEN6 )  // livre 1
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\teen6.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetZoom(0, zoom);

		CreateCrashSphere(D3DVECTOR(-5.0f, 3.0f, 7.5f), 5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 4.5f, 3.0f, 7.5f), 5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-5.0f, 3.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 4.5f, 3.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-5.0f, 3.0f,-7.5f), 5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 4.5f, 3.0f,-7.5f), 5.0f, SOUND_BOUMm, 0.45f);

		CreateShadowCircle(20.0f, 0.2f*fShadow);
	}

	if ( type == OBJECT_TEEN7 )  // livre 2
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\teen7.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetZoom(0, zoom);

		CreateCrashSphere(D3DVECTOR(-5.0f, 3.0f, 7.5f), 5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 4.5f, 3.0f, 7.5f), 5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-5.0f, 3.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 4.5f, 3.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-5.0f, 3.0f,-7.5f), 5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 4.5f, 3.0f,-7.5f), 5.0f, SOUND_BOUMm, 0.45f);

		CreateShadowCircle(20.0f, 0.2f*fShadow);
	}

	if ( type == OBJECT_TEEN8 )  // pile de livres 1
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\teen8.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetZoom(0, zoom);

		CreateCrashSphere(D3DVECTOR(-5.0f, 3.0f, 7.5f), 5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 4.5f, 3.0f, 7.5f), 5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-5.0f, 3.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 4.5f, 3.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-5.0f, 3.0f,-7.5f), 5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 4.5f, 3.0f,-7.5f), 5.0f, SOUND_BOUMm, 0.45f);

		SetGlobalSphere(D3DVECTOR(0.0f, 10.0f, 0.0f), 12.0f);
		CreateShadowCircle(20.0f, 0.2f*fShadow);
	}

	if ( type == OBJECT_TEEN9 )  // pile de livres 2
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\teen9.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetZoom(0, zoom);

		CreateCrashSphere(D3DVECTOR(-5.0f, 3.0f, 7.5f), 5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 4.5f, 3.0f, 7.5f), 5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-5.0f, 3.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 4.5f, 3.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-5.0f, 3.0f,-7.5f), 5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 4.5f, 3.0f,-7.5f), 5.0f, SOUND_BOUMm, 0.45f);

		SetGlobalSphere(D3DVECTOR(0.0f, 10.0f, 0.0f), 12.0f);
		CreateShadowCircle(20.0f, 0.2f*fShadow);
	}

	if ( type == OBJECT_TEEN10 )  // biblioth�que
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\teen10.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetZoom(0, zoom);

		CreateCrashSphere(D3DVECTOR(-26.0f, 3.0f, 0.0f), 6.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-15.0f, 3.0f,-4.0f), 6.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-15.0f, 3.0f, 5.0f), 6.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( -4.0f, 3.0f,-4.0f), 6.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( -4.0f, 3.0f, 5.0f), 6.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(  6.0f, 3.0f,-4.0f), 6.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(  6.0f, 3.0f, 4.0f), 6.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 14.0f, 3.0f,-3.0f), 6.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 14.0f, 3.0f, 2.0f), 6.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 24.0f, 3.0f, 5.0f), 6.0f, SOUND_BOUMm, 0.45f);

		SetGlobalSphere(D3DVECTOR(0.0f, 6.0f, 0.0f), 20.0f);
		CreateShadowCircle(40.0f, 0.2f*fShadow);
	}

	if ( type == OBJECT_TEEN11 )  // lampe
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\teen11.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);
		SetZoom(0, zoom);

		mat = RetWorldMatrix(0);
		pos = Transform(*mat, D3DVECTOR(-56.0f, 22.0f, 0.0f));
		m_particule->CreateParticule(pos, D3DVECTOR(0.0f, 0.0f, 0.0f), FPOINT(20.0f, 20.0f), PARTISELY, 1.0f, 0.0f, 0.0f);

		pos = Transform(*mat, D3DVECTOR(-65.0f, 40.0f, 0.0f));
		color.r = 4.0f;
		color.g = 2.0f;
		color.b = 0.0f;  // jaune-orange
		color.a = 0.0f;
		m_main->CreateSpot(pos, color);
	}

	if ( type == OBJECT_TEEN12 )  // coca
	{
		rank = m_engine->CreateObject();
//?		m_engine->SetObjectType(rank, TYPEFIX);
		m_engine->SetObjectType(rank, TYPEMETAL);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\teen12.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetZoom(0, zoom);

		CreateCrashSphere(D3DVECTOR( 0.0f, 4.0f, 0.0f), 4.0f, SOUND_BOUMm, 0.45f);
		SetGlobalSphere(D3DVECTOR(0.0f, 9.0f, 0.0f), 5.0f);
		CreateShadowCircle(4.5f, 1.0f*fShadow);
	}

	if ( type == OBJECT_TEEN13 )  // carton ferm�
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\teen13.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetZoom(0, zoom);

		CreateCrashSphere(D3DVECTOR(-10.0f, 4.0f,-7.0f), 5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(  0.0f, 4.0f,-7.0f), 5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 10.0f, 4.0f,-7.0f), 5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-10.0f, 4.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(  0.0f, 4.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 10.0f, 4.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-10.0f, 4.0f, 7.0f), 5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(  0.0f, 4.0f, 7.0f), 5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 10.0f, 4.0f, 7.0f), 5.0f, SOUND_BOUMm, 0.45f);

		SetGlobalSphere(D3DVECTOR(0.0f, 5.0f, 0.0f), 15.0f);
		CreateShadowCircle(20.0f, 1.0f*fShadow);
	}

	if ( type == OBJECT_TEEN14 )  // carton ouvert
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\teen14.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetZoom(0, zoom);

		CreateCrashSphere(D3DVECTOR(-10.0f, 4.0f,-7.0f), 5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(  0.0f, 4.0f,-7.0f), 5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 10.0f, 4.0f,-7.0f), 5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-10.0f, 4.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(  0.0f, 4.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 10.0f, 4.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-10.0f, 4.0f, 7.0f), 5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(  0.0f, 4.0f, 7.0f), 5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 10.0f, 4.0f, 7.0f), 5.0f, SOUND_BOUMm, 0.45f);

		SetGlobalSphere(D3DVECTOR(0.0f, 5.0f, 0.0f), 15.0f);
		CreateShadowCircle(20.0f, 1.0f*fShadow);
	}

	if ( type == OBJECT_TEEN15 )  // pile de cartons
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\teen15.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetZoom(0, zoom);

		CreateCrashSphere(D3DVECTOR(-10.0f, 4.0f,-7.0f), 5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(  0.0f, 4.0f,-7.0f), 5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 10.0f, 4.0f,-7.0f), 5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-10.0f, 4.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(  0.0f, 4.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 10.0f, 4.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-10.0f, 4.0f, 7.0f), 5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(  0.0f, 4.0f, 7.0f), 5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 10.0f, 4.0f, 7.0f), 5.0f, SOUND_BOUMm, 0.45f);

		SetGlobalSphere(D3DVECTOR(0.0f, 5.0f, 0.0f), 15.0f);
		CreateShadowCircle(20.0f, 1.0f*fShadow);
	}

	if ( type == OBJECT_TEEN16 )  // arrosoir
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\teen16.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetZoom(0, zoom);

		CreateCrashSphere(D3DVECTOR(-8.0f, 4.0f, 0.0f), 12.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 8.0f, 4.0f, 0.0f), 12.0f, SOUND_BOUMm, 0.45f);

		SetGlobalSphere(D3DVECTOR(0.0f, 13.0f, 0.0f), 20.0f);
		CreateShadowCircle(18.0f, 1.0f*fShadow);
	}

	if ( type == OBJECT_TEEN17 )  // roue |
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\teen17.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetZoom(0, zoom);

		CreateCrashSphere(D3DVECTOR( 0.0f, 31.0f, 0.0f), 31.0f, SOUND_BOUMm, 0.45f);
		SetGlobalSphere(D3DVECTOR(0.0f, 31.0f, 0.0f), 31.0f);
		CreateShadowCircle(24.0f, 0.5f*fShadow);
	}

	if ( type == OBJECT_TEEN18 )  // roue /
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\teen18.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetZoom(0, zoom);

		CreateCrashSphere(D3DVECTOR( 0.0f, 31.0f, 0.0f), 31.0f, SOUND_BOUMm, 0.45f);
		SetGlobalSphere(D3DVECTOR(0.0f, 31.0f, 0.0f), 31.0f);
		CreateShadowCircle(24.0f, 0.5f*fShadow);
	}

	if ( type == OBJECT_TEEN19 )  // roue =
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\teen19.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetZoom(0, zoom);

		CreateCrashSphere(D3DVECTOR( 0.0f, 10.0f, 0.0f), 32.0f, SOUND_BOUMm, 0.45f);
		SetGlobalSphere(D3DVECTOR(0.0f, 10.0f, 0.0f), 32.0f);
		CreateShadowCircle(33.0f, 1.0f*fShadow);
	}

	if ( type == OBJECT_TEEN20 )  // mur avec �tag�re
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\teen20.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetZoom(0, zoom);

		CreateCrashSphere(D3DVECTOR(-175.0f, 0.0f,  -5.0f), 4.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-175.0f, 0.0f, -35.0f), 4.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( -55.0f, 0.0f,  -5.0f), 4.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( -55.0f, 0.0f, -35.0f), 4.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( -37.0f, 0.0f,  -5.0f), 4.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( -37.0f, 0.0f, -35.0f), 4.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(  83.0f, 0.0f,  -5.0f), 4.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(  83.0f, 0.0f, -35.0f), 4.0f, SOUND_BOUMm, 0.45f);
	}

	if ( type == OBJECT_TEEN21 )  // mur avec fen�tre
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\teen21.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetZoom(0, zoom);
	}

	if ( type == OBJECT_TEEN22 )  // mur avec porte et �tag�re
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\teen22.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetZoom(0, zoom);

		CreateCrashSphere(D3DVECTOR(-135.0f, 0.0f,  -5.0f), 4.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-135.0f, 0.0f, -35.0f), 4.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( -15.0f, 0.0f,  -5.0f), 4.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( -15.0f, 0.0f, -35.0f), 4.0f, SOUND_BOUMm, 0.45f);
	}

	if ( type == OBJECT_TEEN23 )  // skate sur ses roues
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\teen23.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetZoom(0, zoom);

		if ( m_option == 1 )  // passage sous le skate interdit ?
		{
			CreateCrashSphere(D3DVECTOR(-10.0f, 2.0f, 0.0f), 11.0f, SOUND_BOUMm, 0.45f);
			CreateCrashSphere(D3DVECTOR( 10.0f, 2.0f, 0.0f), 11.0f, SOUND_BOUMm, 0.45f);
		}

		CreateCrashSphere(D3DVECTOR(-23.0f, 2.0f, 7.0f), 3.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-23.0f, 2.0f, 0.0f), 3.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-23.0f, 2.0f,-7.0f), 3.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 23.0f, 2.0f, 7.0f), 3.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 23.0f, 2.0f, 0.0f), 3.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 23.0f, 2.0f,-7.0f), 3.0f, SOUND_BOUMm, 0.45f);

		CreateShadowCircle(35.0f, 0.8f*fShadow, D3DSHADOWWORM);
	}

	if ( type == OBJECT_TEEN24 )  // skate /
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\teen24.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetZoom(0, zoom);

		CreateCrashSphere(D3DVECTOR(-12.0f, 0.0f, -3.0f), 3.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-12.0f, 0.0f,  3.0f), 3.0f, SOUND_BOUMm, 0.45f);
		CreateShadowCircle(20.0f, 0.2f*fShadow);
	}

	if ( type == OBJECT_TEEN25 )  // skate /
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\teen25.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetZoom(0, zoom);

		CreateCrashSphere(D3DVECTOR(-12.0f, 0.0f, -3.0f), 3.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-12.0f, 0.0f,  3.0f), 3.0f, SOUND_BOUMm, 0.45f);
		CreateShadowCircle(20.0f, 0.2f*fShadow);
	}

	if ( type == OBJECT_TEEN26 )  // lampe au plafond
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\teen26.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetZoom(0, zoom);
		SetFloorHeight(0.0f);

		mat = RetWorldMatrix(0);
		pos = Transform(*mat, D3DVECTOR(0.0f, 50.0f, 0.0f));
		m_particule->CreateParticule(pos, D3DVECTOR(0.0f, 0.0f, 0.0f), FPOINT(100.0f, 100.0f), PARTISELY, 1.0f, 0.0f, 0.0f);

		pos = Transform(*mat, D3DVECTOR(0.0f, 50.0f, 0.0f));
		color.r = 4.0f;
		color.g = 2.0f;
		color.b = 0.0f;  // jaune-orange
		color.a = 0.0f;
		m_main->CreateSpot(pos, color);
	}

	if ( type == OBJECT_TEEN27 )  // grande plante ?
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\teen27.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetZoom(0, zoom);

		CreateCrashSphere(D3DVECTOR(0.0f, 0.0f, 0.0f), 4.0f, SOUND_BOUM, 0.10f);
		CreateShadowCircle(40.0f, 0.5f);
	}

	if ( type == OBJECT_TEEN28 )  // bouteille ?
	{
		rank = m_engine->CreateObject();
//?		m_engine->SetObjectType(rank, TYPEFIX);
		m_engine->SetObjectType(rank, TYPEMETAL);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\teen28.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetZoom(0, zoom);

		CreateCrashSphere(D3DVECTOR(0.0f, 2.0f, 0.0f), 5.0f, SOUND_BOUM, 0.10f);
		CreateShadowCircle(7.0f, 0.6f*fShadow);
	}

	if ( type == OBJECT_TEEN29 )  // pont ?
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\teen29.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetZoom(0, zoom);
		bFloorAdjust = FALSE;
	}

	if ( type == OBJECT_TEEN30 )  // saut ?
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\teen30.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetZoom(0, zoom);

		CreateCrashSphere(D3DVECTOR(0.0f, 4.0f, 0.0f), 15.0f, SOUND_BOUM, 0.10f);
		SetGlobalSphere(D3DVECTOR(0.0f, 15.0f, 0.0f), 17.0f);
		CreateShadowCircle(20.0f, 1.0f*fShadow);
	}

	if ( type == OBJECT_TEEN31 )  // basket ?
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\teen31.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetZoom(0, zoom);

		CreateCrashSphere(D3DVECTOR(-10.0f, 2.0f, 0.0f), 5.0f, SOUND_BOUM, 0.10f);
		CreateCrashSphere(D3DVECTOR(  0.0f, 2.0f, 0.0f), 6.0f, SOUND_BOUM, 0.10f);
		CreateCrashSphere(D3DVECTOR(  9.0f, 4.0f, 1.0f), 6.0f, SOUND_BOUM, 0.10f);

		SetGlobalSphere(D3DVECTOR(0.0f, 0.0f, 0.0f), 10.0f);
		CreateShadowCircle(16.0f, 0.6f*fShadow);
	}

	if ( type == OBJECT_TEEN32 )  // chaise ?
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\teen32.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetZoom(0, zoom);

		CreateCrashSphere(D3DVECTOR( 17.5f, 1.0f,  17.5f), 3.5f, SOUND_BOUM, 0.10f);
		CreateCrashSphere(D3DVECTOR( 17.5f, 1.0f, -17.5f), 3.5f, SOUND_BOUM, 0.10f);
		CreateCrashSphere(D3DVECTOR(-17.5f, 1.0f,  17.5f), 3.5f, SOUND_BOUM, 0.10f);
		CreateCrashSphere(D3DVECTOR(-17.5f, 1.0f, -17.5f), 3.5f, SOUND_BOUM, 0.10f);
		SetGlobalSphere(D3DVECTOR(0.0f, 0.0f, 0.0f), 26.0f);
		CreateShadowCircle(35.0f, 0.3f*fShadow);
	}

	if ( type == OBJECT_TEEN33 )  // panneau ?
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\teen33.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetZoom(0, zoom);

		CreateCrashSphere(D3DVECTOR(0.0f, 2.0f, 0.0f), 4.0f, SOUND_BOUM, 0.10f);
		CreateShadowCircle(10.0f, 0.3f*fShadow);
	}

	if ( type == OBJECT_TEEN34 )  // caillou ?
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\teen34.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetZoom(0, zoom);

		CreateCrashSphere(D3DVECTOR(0.0f, 2.0f, 0.0f), 4.0f, SOUND_BOUM, 0.10f);
		CreateShadowCircle(3.0f, 1.0f*fShadow);
	}

	if ( type == OBJECT_TEEN35 )  // tuyau ?
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\teen35.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetZoom(0, zoom);

		CreateCrashSphere(D3DVECTOR(-40.0f, 5.0f, 0.0f), 10.0f, SOUND_BOUM, 0.10f);
		CreateCrashSphere(D3DVECTOR(-20.0f, 5.0f, 0.0f), 10.0f, SOUND_BOUM, 0.10f);
		CreateCrashSphere(D3DVECTOR(  0.0f, 5.0f, 0.0f), 10.0f, SOUND_BOUM, 0.10f);
		CreateCrashSphere(D3DVECTOR( 20.0f, 5.0f, 0.0f), 10.0f, SOUND_BOUM, 0.10f);
		CreateCrashSphere(D3DVECTOR( 40.0f, 5.0f, 0.0f), 10.0f, SOUND_BOUM, 0.10f);
		CreateShadowCircle(40.0f, 0.8f*fShadow, D3DSHADOWWORM);
	}

	if ( type == OBJECT_TEEN36 )  // tronc ?
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\teen36.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetZoom(0, zoom);
		bFloorAdjust = FALSE;
	}

	if ( type == OBJECT_TEEN37 )  // bateau ?
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\teen37.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetZoom(0, zoom);
		bFloorAdjust = FALSE;
	}

	if ( type == OBJECT_TEEN38 )  // ventillateur ?
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\teen38a.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetZoom(0, zoom);

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(1, rank);
		SetObjectParent(1, 0);
		pModFile->ReadModel("objects\\teen38b.mod");  // moteur
		pModFile->CreateEngineObject(rank);
		SetPosition(1, D3DVECTOR(0.0f, 30.0f, 0.0f));

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(2, rank);
		SetObjectParent(2, 1);
		pModFile->ReadModel("objects\\teen38c.mod");  // h�lice
		pModFile->CreateEngineObject(rank);
		SetPosition(2, D3DVECTOR(0.0f, 0.0f, 0.0f));

		CreateCrashSphere(D3DVECTOR(0.0f, 2.0f, 0.0f), 10.0f, SOUND_BOUM, 0.10f);
		SetGlobalSphere(D3DVECTOR(0.0f, 2.0f, 0.0f), 10.0f);
		CreateShadowCircle(15.0f, 0.5f*fShadow);
	}

	if ( type == OBJECT_TEEN39 )  // plante en pot ?
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\teen39.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetZoom(0, zoom);

		CreateCrashSphere(D3DVECTOR(0.0f, 2.0f, 0.0f), 8.5f, SOUND_BOUM, 0.10f);
		SetGlobalSphere(D3DVECTOR(0.0f, 2.0f, 0.0f), 8.5f);
		CreateShadowCircle(10.0f, 1.0f*fShadow);
	}

	if ( type == OBJECT_TEEN40 )  // ballon ?
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\teen40.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetZoom(0, zoom);

		CreateCrashSphere(D3DVECTOR(0.0f, 5.0f, 0.0f), 11.0f, SOUND_BOUM, 0.10f);
		SetGlobalSphere(D3DVECTOR(0.0f, 14.0f, 0.0f), 15.0f);
		CreateShadowCircle(15.0f, 0.7f*fShadow);
	}

	if ( type == OBJECT_TEEN41 )  // cl�ture ?
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\teen41.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetZoom(0, zoom);
	}

	if ( type == OBJECT_TEEN42 )  // tr�fle ?
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\teen42.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetZoom(0, zoom);

		CreateCrashSphere(D3DVECTOR(0.0f, 2.0f, 0.0f), 2.0f, SOUND_BOUM, 0.10f);
		CreateShadowCircle(15.0f, 0.4f*fShadow);
	}

	if ( type == OBJECT_TEEN43 )  // tr�fle ?
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\teen43.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetZoom(0, zoom);

		CreateCrashSphere(D3DVECTOR(0.0f, 2.0f, 0.0f), 2.0f, SOUND_BOUM, 0.10f);
		CreateShadowCircle(15.0f, 0.4f*fShadow);
	}

	if ( type == OBJECT_TEEN44 )  // caisse ?
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\teen44.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetZoom(0, zoom);

		CreateCrashSphere(D3DVECTOR(0.0f, 10.0f, 0.0f), 55.0f, SOUND_BOUM, 0.10f);
		SetGlobalSphere(D3DVECTOR(0.0f, 10.0f, 0.0f), 55.0f);
		CreateShadowCircle(55.0f, 1.0f*fShadow);
	}

	pos = RetPosition(0);
	SetPosition(0, pos);  // pour afficher les ombres tout de suite

	if ( bFloorAdjust )
	{
		SetFloorHeight(0.0f);
		FloorAdjust();
	}

	CreateOtherObject(type);

	pos = RetPosition(0);
	pos.y += height;
	SetPosition(0, pos);

	delete pModFile;
	return TRUE;
}

// Cr�e un quartz pos� sur le sol.

BOOL CObject::CreateQuartz(D3DVECTOR pos, float angle, float height,
						   ObjectType type)
{
	CModFile*	pModFile;
	float		radius;
	int			rank;

	if ( m_engine->RetRestCreate() < 1 )  return FALSE;

	pModFile = new CModFile(m_iMan);

	SetType(type);

	if ( type == OBJECT_QUARTZ0 )
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEQUARTZ);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\quartz0.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);

		CreateCrashSphere(D3DVECTOR(0.0f, 2.0f, 0.0f), 3.5f, SOUND_BOUMm, 0.45f);
		SetGlobalSphere(D3DVECTOR(0.0f, 2.0f, 0.0f), 3.5f);

		CreateShadowCircle(4.0f, 0.5f);
	}
	if ( type == OBJECT_QUARTZ1 )
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEQUARTZ);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\quartz1.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);

		CreateCrashSphere(D3DVECTOR(0.0f, 4.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
		SetGlobalSphere(D3DVECTOR(0.0f, 4.0f, 0.0f), 5.0f);

		CreateShadowCircle(5.0f, 0.5f);
	}
	if ( type == OBJECT_QUARTZ2 )
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEQUARTZ);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\quartz2.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);

		CreateCrashSphere(D3DVECTOR(0.0f, 6.0f, 0.0f), 6.0f, SOUND_BOUMm, 0.45f);
		SetGlobalSphere(D3DVECTOR(0.0f, 6.0f, 0.0f), 6.0f);

		CreateShadowCircle(6.0f, 0.5f);
	}
	if ( type == OBJECT_QUARTZ3 )
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEQUARTZ);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\quartz3.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);

		CreateCrashSphere(D3DVECTOR(0.0f, 10.0f, 0.0f), 10.0f, SOUND_BOUMm, 0.45f);
		SetGlobalSphere(D3DVECTOR(0.0f, 10.0f, 0.0f), 10.0f);

		CreateShadowCircle(10.0f, 0.5f);
	}

	pos = RetPosition(0);
	SetPosition(0, pos);  // pour afficher les ombres tout de suite

	SetFloorHeight(0.0f);
	CreateOtherObject(type);

	pos = RetPosition(0);
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
	m_particule->CreateParticule(pos, pos, FPOINT(2.0f, 2.0f), PARTIQUARTZ, 0.7f+Rand()*0.7f, radius, 0.0f);
	m_particule->CreateParticule(pos, pos, FPOINT(2.0f, 2.0f), PARTIQUARTZ, 0.7f+Rand()*0.7f, radius, 0.0f);

	delete pModFile;
	return TRUE;
}

// Cr�e une racine pos�e sur le sol.

BOOL CObject::CreateRoot(D3DVECTOR pos, float angle, float height,
						 ObjectType type)
{
	CModFile*	pModFile;
	int			rank;

	if ( m_engine->RetRestCreate() < 1 )  return FALSE;

	pModFile = new CModFile(m_iMan);

	SetType(type);

	if ( type == OBJECT_ROOT0 )
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\root0.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetZoom(0, 2.0f);

		CreateCrashSphere(D3DVECTOR(-5.0f,  1.0f,  0.0f), 2.0f, SOUND_BOUMv, 0.15f);
		CreateCrashSphere(D3DVECTOR( 4.0f,  1.0f,  2.0f), 2.0f, SOUND_BOUMv, 0.15f);
		CreateCrashSphere(D3DVECTOR( 4.0f,  1.0f, -3.0f), 2.0f, SOUND_BOUMv, 0.15f);
		CreateCrashSphere(D3DVECTOR( 2.0f,  5.0f, -1.0f), 1.5f, SOUND_BOUMv, 0.15f);
		CreateCrashSphere(D3DVECTOR(-4.0f,  5.0f, -1.0f), 1.0f, SOUND_BOUMv, 0.15f);
		CreateCrashSphere(D3DVECTOR(-2.0f,  8.0f, -0.5f), 1.0f, SOUND_BOUMv, 0.15f);
		CreateCrashSphere(D3DVECTOR( 0.0f, 10.0f, -0.5f), 1.0f, SOUND_BOUMv, 0.15f);
//?		SetGlobalSphere(D3DVECTOR(0.0f, 6.0f, 0.0f), 11.0f);

		CreateShadowCircle(16.0f, 0.5f);
	}
	if ( type == OBJECT_ROOT1 )
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\root1.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetZoom(0, 2.0f);

		CreateCrashSphere(D3DVECTOR(-4.0f,  1.0f,  1.0f), 2.0f, SOUND_BOUMv, 0.15f);
		CreateCrashSphere(D3DVECTOR( 0.0f,  1.0f,  2.0f), 1.5f, SOUND_BOUMv, 0.15f);
		CreateCrashSphere(D3DVECTOR( 3.0f,  1.0f, -2.0f), 2.0f, SOUND_BOUMv, 0.15f);
		CreateCrashSphere(D3DVECTOR(-2.0f,  5.0f,  1.0f), 1.0f, SOUND_BOUMv, 0.15f);
		CreateCrashSphere(D3DVECTOR( 2.0f,  5.0f,  0.0f), 1.0f, SOUND_BOUMv, 0.15f);
		CreateCrashSphere(D3DVECTOR( 0.0f,  8.0f,  1.0f), 1.0f, SOUND_BOUMv, 0.15f);
		CreateCrashSphere(D3DVECTOR( 0.0f, 12.0f,  1.0f), 1.0f, SOUND_BOUMv, 0.15f);
//?		SetGlobalSphere(D3DVECTOR(0.0f, 6.0f, 0.0f), 12.0f);

		CreateShadowCircle(16.0f, 0.5f);
	}
	if ( type == OBJECT_ROOT2 )
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\root2.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetZoom(0, 2.0f);

		CreateCrashSphere(D3DVECTOR(-3.0f,  1.0f,  0.5f), 2.0f, SOUND_BOUMv, 0.15f);
		CreateCrashSphere(D3DVECTOR( 3.0f,  1.0f, -1.0f), 2.0f, SOUND_BOUMv, 0.15f);
		CreateCrashSphere(D3DVECTOR(-1.0f,  4.5f,  0.0f), 1.0f, SOUND_BOUMv, 0.15f);
		CreateCrashSphere(D3DVECTOR( 3.0f,  7.0f,  1.0f), 1.0f, SOUND_BOUMv, 0.15f);
		CreateCrashSphere(D3DVECTOR( 0.0f,  7.0f, -1.0f), 1.0f, SOUND_BOUMv, 0.15f);
		CreateCrashSphere(D3DVECTOR( 4.0f, 11.0f,  1.0f), 1.0f, SOUND_BOUMv, 0.15f);
//?		SetGlobalSphere(D3DVECTOR(0.0f, 6.0f, 0.0f), 10.0f);

		CreateShadowCircle(16.0f, 0.5f);
	}
	if ( type == OBJECT_ROOT3 )
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\root3.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetZoom(0, 2.0f);

		CreateCrashSphere(D3DVECTOR(-4.0f,  1.0f,  1.0f), 3.0f, SOUND_BOUMv, 0.15f);
		CreateCrashSphere(D3DVECTOR( 4.0f,  1.0f, -3.0f), 3.0f, SOUND_BOUMv, 0.15f);
		CreateCrashSphere(D3DVECTOR( 6.0f,  1.0f,  4.0f), 3.0f, SOUND_BOUMv, 0.15f);
		CreateCrashSphere(D3DVECTOR(-2.5f,  7.0f,  2.0f), 2.0f, SOUND_BOUMv, 0.15f);
		CreateCrashSphere(D3DVECTOR( 4.0f,  7.0f,  2.0f), 2.0f, SOUND_BOUMv, 0.15f);
		CreateCrashSphere(D3DVECTOR( 3.0f,  6.0f, -1.0f), 1.0f, SOUND_BOUMv, 0.15f);
		CreateCrashSphere(D3DVECTOR( 0.0f, 12.0f,  0.0f), 2.0f, SOUND_BOUMv, 0.15f);
		CreateCrashSphere(D3DVECTOR( 1.0f, 16.0f,  0.0f), 1.0f, SOUND_BOUMv, 0.15f);
//?		SetGlobalSphere(D3DVECTOR(0.0f, 10.0f, 0.0f), 14.0f);

		CreateShadowCircle(22.0f, 0.5f);
	}
	if ( type == OBJECT_ROOT4 )
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\root4.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetZoom(0, 2.0f);

		CreateCrashSphere(D3DVECTOR( -7.0f,  2.0f,  3.0f), 4.0f, SOUND_BOUMv, 0.15f);
		CreateCrashSphere(D3DVECTOR(  5.0f,  2.0f, -6.0f), 4.0f, SOUND_BOUMv, 0.15f);
		CreateCrashSphere(D3DVECTOR(  6.0f,  2.0f,  6.0f), 3.0f, SOUND_BOUMv, 0.15f);
		CreateCrashSphere(D3DVECTOR(-11.0f,  1.0f, -2.0f), 2.0f, SOUND_BOUMv, 0.15f);
		CreateCrashSphere(D3DVECTOR(  1.0f,  1.0f, -7.0f), 2.0f, SOUND_BOUMv, 0.15f);
		CreateCrashSphere(D3DVECTOR( -4.0f, 10.0f,  3.0f), 2.0f, SOUND_BOUMv, 0.15f);
		CreateCrashSphere(D3DVECTOR(  1.0f, 11.0f,  7.0f), 2.0f, SOUND_BOUMv, 0.15f);
		CreateCrashSphere(D3DVECTOR(  3.0f, 11.0f, -3.0f), 2.0f, SOUND_BOUMv, 0.15f);
		CreateCrashSphere(D3DVECTOR( -3.0f, 17.0f,  1.0f), 2.0f, SOUND_BOUMv, 0.15f);
		CreateCrashSphere(D3DVECTOR( -3.0f, 23.0f, -1.0f), 2.0f, SOUND_BOUMv, 0.15f);
//?		SetGlobalSphere(D3DVECTOR(0.0f, 12.0f, 0.0f), 20.0f);

		CreateShadowCircle(30.0f, 0.5f);
	}
	if ( type == OBJECT_ROOT5 )  // gravity root ?
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\root4.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetZoom(0, 2.0f);

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(1, rank);
		SetObjectParent(1, 0);
		pModFile->ReadModel("objects\\root5.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(1, D3DVECTOR(-5.0f, 28.0f, -4.0f));
		SetAngleX(1, -30.0f*PI/180.0f);
		SetAngleZ(1,  20.0f*PI/180.0f);

		CreateCrashSphere(D3DVECTOR( -7.0f,  2.0f,  3.0f), 4.0f, SOUND_BOUMv, 0.15f);
		CreateCrashSphere(D3DVECTOR(  5.0f,  2.0f, -6.0f), 4.0f, SOUND_BOUMv, 0.15f);
		CreateCrashSphere(D3DVECTOR(  6.0f,  2.0f,  6.0f), 3.0f, SOUND_BOUMv, 0.15f);
		CreateCrashSphere(D3DVECTOR(-11.0f,  1.0f, -2.0f), 2.0f, SOUND_BOUMv, 0.15f);
		CreateCrashSphere(D3DVECTOR(  1.0f,  1.0f, -7.0f), 2.0f, SOUND_BOUMv, 0.15f);
		CreateCrashSphere(D3DVECTOR( -4.0f, 10.0f,  3.0f), 2.0f, SOUND_BOUMv, 0.15f);
		CreateCrashSphere(D3DVECTOR(  1.0f, 11.0f,  7.0f), 2.0f, SOUND_BOUMv, 0.15f);
		CreateCrashSphere(D3DVECTOR(  3.0f, 11.0f, -3.0f), 2.0f, SOUND_BOUMv, 0.15f);
		CreateCrashSphere(D3DVECTOR( -3.0f, 17.0f,  1.0f), 2.0f, SOUND_BOUMv, 0.15f);
		CreateCrashSphere(D3DVECTOR( -3.0f, 23.0f, -1.0f), 2.0f, SOUND_BOUMv, 0.15f);
//?		SetGlobalSphere(D3DVECTOR(0.0f, 12.0f, 0.0f), 20.0f);

		CreateShadowCircle(30.0f, 0.5f);
	}

	pos = RetPosition(0);
	SetPosition(0, pos);  // pour afficher les ombres tout de suite

	SetFloorHeight(0.0f);
	CreateOtherObject(type);

	pos = RetPosition(0);
	pos.y += height;
	SetPosition(0, pos);

	delete pModFile;
	return TRUE;
}

// Cr�e une petite maison.

BOOL CObject::CreateHome(D3DVECTOR pos, float angle, float height,
						 ObjectType type)
{
	CModFile*	pModFile;
	int			rank;

	if ( m_engine->RetRestCreate() < 1 )  return FALSE;

	pModFile = new CModFile(m_iMan);

	SetType(type);

	if ( type == OBJECT_HOME1 )
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\home1.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetZoom(0, 1.3f);

		CreateCrashSphere(D3DVECTOR(0.0f, 5.0f, 0.0f), 10.0f, SOUND_BOUMs, 0.25f);
//?		SetGlobalSphere(D3DVECTOR(0.0f, 6.0f, 0.0f), 11.0f);
		CreateShadowCircle(16.0f, 0.5f);
	}

	pos = RetPosition(0);
	SetPosition(0, pos);  // pour afficher les ombres tout de suite

	SetFloorHeight(0.0f);
	CreateOtherObject(type);

	pos = RetPosition(0);
	pos.y += height;
	SetPosition(0, pos);

	delete pModFile;
	return TRUE;
}

// Cr�e une ruine pos�e sur le sol.

BOOL CObject::CreateRuin(D3DVECTOR pos, float angle, float height,
						 ObjectType type)
{
	CModFile*	pModFile;
	char		name[50];
	int			rank;

	if ( m_engine->RetRestCreate() < 1+4 )  return FALSE;

	pModFile = new CModFile(m_iMan);

	SetType(type);

	rank = m_engine->CreateObject();
	m_engine->SetObjectType(rank, TYPEFIX);  // c'est un objet fixe
	SetObjectRank(0, rank);

	name[0] = 0;
	if ( type == OBJECT_RUINmobilew1 )  strcpy(name, "objects\\ruin1.mod");
	if ( type == OBJECT_RUINmobilew2 )  strcpy(name, "objects\\ruin1.mod");
	if ( type == OBJECT_RUINmobilet1 )  strcpy(name, "objects\\ruin2.mod");
	if ( type == OBJECT_RUINmobilet2 )  strcpy(name, "objects\\ruin2.mod");
	if ( type == OBJECT_RUINmobiler1 )  strcpy(name, "objects\\ruin3.mod");
	if ( type == OBJECT_RUINmobiler2 )  strcpy(name, "objects\\ruin3.mod");
	if ( type == OBJECT_RUINfactory  )  strcpy(name, "objects\\ruin4.mod");
	if ( type == OBJECT_RUINdoor     )  strcpy(name, "objects\\ruin5.mod");
	if ( type == OBJECT_RUINsupport  )  strcpy(name, "objects\\ruin6.mod");
	if ( type == OBJECT_RUINradar    )  strcpy(name, "objects\\ruin7.mod");
	if ( type == OBJECT_RUINconvert  )  strcpy(name, "objects\\ruin8.mod");
	if ( type == OBJECT_RUINbase     )  strcpy(name, "objects\\ruin9.mod");
	if ( type == OBJECT_RUINhead     )  strcpy(name, "objects\\ruin10.mod");

	pModFile->ReadModel(name);
	pModFile->CreateEngineObject(rank);

	SetPosition(0, pos);
	SetAngleY(0, angle);

	if ( type == OBJECT_RUINmobilew1 )  // v�hicule � roues ?
	{
		// Cr�e la roue arri�re-droite.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(6, rank);
		SetObjectParent(6, 0);

		pModFile->ReadModel("objects\\ruin1w.mod");
		pModFile->CreateEngineObject(rank);

		SetPosition(6, D3DVECTOR(-3.0f, 1.8f, -4.0f));
		SetAngleX(6, -PI/2.0f);

		// Cr�e la roue arri�re-gauche.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(7, rank);
		SetObjectParent(7, 0);

		pModFile->ReadModel("objects\\ruin1w.mod");
		pModFile->CreateEngineObject(rank);

		SetPosition(7, D3DVECTOR(-3.0f, 1.0f, 3.0f));
		SetAngleY(7, PI-0.3f);
		SetAngleX(7, -0.3f);

		// Cr�e la roue avant-droite.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(8, rank);
		SetObjectParent(8, 0);

		pModFile->ReadModel("objects\\ruin1w.mod");
		pModFile->CreateEngineObject(rank);

		SetPosition(8, D3DVECTOR(2.0f, 1.6f, -3.0f));
		SetAngleY(8, 0.3f);

		// Cr�e la roue avant-gauche.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(9, rank);
		SetObjectParent(9, 0);

		pModFile->ReadModel("objects\\ruin1w.mod");
		pModFile->CreateEngineObject(rank);

		SetPosition(9, D3DVECTOR(2.0f, 1.0f, 3.0f));
		SetAngleY(9, PI-0.2f);
		SetAngleX(9, 0.2f);

		CreateCrashSphere(D3DVECTOR(0.0f, 2.8f, 0.0f), 3.0f, SOUND_BOUMm, 0.45f);
//?		SetGlobalSphere(D3DVECTOR(0.0f, 5.0f, 0.0f), 10.0f);

		CreateShadowCircle(4.0f, 1.0f);
	}

	if ( type == OBJECT_RUINmobilew2 )  // v�hicule � roues ?
	{
		// Cr�e la roue arri�re-gauche.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(7, rank);
		SetObjectParent(7, 0);

		pModFile->ReadModel("objects\\ruin1w.mod");
		pModFile->CreateEngineObject(rank);

		SetPosition(7, D3DVECTOR(-3.0f, 1.0f, 3.0f));
		SetAngleY(7, PI+0.3f);
		SetAngleX(7, 0.4f);

		// Cr�e la roue avant-gauche.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(9, rank);
		SetObjectParent(9, 0);

		pModFile->ReadModel("objects\\ruin1w.mod");
		pModFile->CreateEngineObject(rank);

		SetPosition(9, D3DVECTOR(2.0f, 1.0f, 3.0f));
		SetAngleY(9, PI+0.3f);
		SetAngleX(9, -0.3f);

		CreateCrashSphere(D3DVECTOR(0.0f, 2.8f, 0.0f), 3.0f, SOUND_BOUMm, 0.45f);
//?		SetGlobalSphere(D3DVECTOR(0.0f, 5.0f, 0.0f), 10.0f);

		CreateShadowCircle(4.0f, 1.0f);
	}

	if ( type == OBJECT_RUINmobilet1 )  // v�hicule � chenilles ?
	{
		// Cr�e le canon.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(1, rank);
		SetObjectParent(1, 0);

		pModFile->ReadModel("objects\\ruin2c.mod");
		pModFile->CreateEngineObject(rank);

		SetPosition(1, D3DVECTOR(3.0f, 5.0f, -2.5f));
		SetAngleX(1, -PI*0.85f);
		SetAngleY(1, -0.4f);
		SetAngleZ(1, -0.1f);

		CreateCrashSphere(D3DVECTOR(1.0f, 2.8f, -1.0f), 5.0f, SOUND_BOUMm, 0.45f);
//?		SetGlobalSphere(D3DVECTOR(1.0f, 5.0f, -1.0f), 10.0f);

		CreateShadowCircle(5.0f, 1.0f);
	}

	if ( type == OBJECT_RUINmobilet2 )  // v�hicule � chenilles ?
	{
		CreateCrashSphere(D3DVECTOR(0.0f, 2.8f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
//?		SetGlobalSphere(D3DVECTOR(0.0f, 5.0f, 0.0f), 10.0f);

		CreateShadowCircle(5.0f, 1.0f);
	}

	if ( type == OBJECT_RUINmobiler1 )  // v�hicule roller ?
	{
		CreateCrashSphere(D3DVECTOR(1.0f, 2.8f, -1.0f), 5.0f, SOUND_BOUMm, 0.45f);
		SetGlobalSphere(D3DVECTOR(1.0f, 5.0f, -1.0f), 10.0f);

		CreateShadowCircle(5.0f, 1.0f);
	}

	if ( type == OBJECT_RUINmobiler2 )  // v�hicule roller ?
	{
		CreateCrashSphere(D3DVECTOR(0.0f, 1.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
		SetGlobalSphere(D3DVECTOR(0.0f, 5.0f, 0.0f), 10.0f);

		CreateShadowCircle(6.0f, 1.0f);
	}

	if ( type == OBJECT_RUINfactory )  // factory ?
	{
		CreateCrashSphere(D3DVECTOR(  9.0f,  1.0f, -11.0f), 5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(  0.0f,  2.0f, -11.0f), 4.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-10.0f,  4.0f, -10.0f), 5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-12.0f, 11.0f,  -4.0f), 3.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-10.0f,  4.0f,  -2.0f), 3.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-11.0f,  8.0f,   3.0f), 3.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-11.0f,  2.0f,   4.0f), 3.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-11.0f,  2.0f,  10.0f), 3.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( -4.0f,  0.0f,  10.0f), 3.0f, SOUND_BOUMm, 0.45f);
		SetGlobalSphere(D3DVECTOR(0.0f, 0.0f, 0.0f), 18.0f);

		CreateShadowCircle(20.0f, 0.7f);
	}

	if ( type == OBJECT_RUINdoor )  // porte convert ?
	{
		CreateCrashSphere(D3DVECTOR(0.0f, 0.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
//?		SetGlobalSphere(D3DVECTOR(0.0f, 0.0f, 0.0f), 6.0f);

		CreateShadowCircle(6.0f, 1.0f);
	}

	if ( type == OBJECT_RUINsupport )  // porte radar ?
	{
		CreateCrashSphere(D3DVECTOR(0.0f, 0.0f, 0.0f), 3.0f, SOUND_BOUMm, 0.45f);
//?		SetGlobalSphere(D3DVECTOR(0.0f, 0.0f, 0.0f), 4.0f);

		CreateShadowCircle(3.0f, 1.0f);
	}

	if ( type == OBJECT_RUINradar )  // base radar ?
	{
		CreateCrashSphere(D3DVECTOR(0.0f, 0.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
//?		SetGlobalSphere(D3DVECTOR(0.0f, 0.0f, 0.0f), 6.0f);

		CreateShadowCircle(6.0f, 1.0f);
	}

	if ( type == OBJECT_RUINconvert )  // convert ?
	{
		m_terrain->AddBuildingLevel(pos, 7.0f, 9.0f, 1.0f, 0.5f);

		CreateCrashSphere(D3DVECTOR(-10.0f,  0.0f,  4.0f), 5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-10.0f,  0.0f, -4.0f), 5.0f, SOUND_BOUMm, 0.45f);
//?		SetGlobalSphere(D3DVECTOR(-3.0f, 0.0f, 0.0f), 14.0f);
	}

	if ( type == OBJECT_RUINbase )  // base ?
	{
		CreateCrashSphere(D3DVECTOR(  0.0f, 15.0f,   0.0f),28.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 17.0f,  6.0f,  42.0f), 6.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 17.0f, 17.0f,  42.0f), 4.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-17.0f,  6.0f,  42.0f), 6.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-17.0f, 17.0f,  42.0f), 4.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-42.0f,  6.0f,  17.0f), 6.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-42.0f, 17.0f,  17.0f), 4.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-42.0f,  6.0f, -17.0f), 6.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-42.0f, 17.0f, -17.0f), 4.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-17.0f,  6.0f, -42.0f), 6.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-17.0f, 10.0f, -42.0f), 4.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 15.0f, 13.0f, -34.0f), 3.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 31.0f, 15.0f, -13.0f), 3.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 21.0f,  8.0f, -39.0f), 5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 26.0f,  8.0f, -33.0f), 5.0f, SOUND_BOUMm, 0.45f);
		SetGlobalSphere(D3DVECTOR(0.0f, 0.0f, 0.0f), 48.0f);

		CreateShadowCircle(40.0f, 1.0f);
	}

	if ( type == OBJECT_RUINhead )  // coiffe base ?
	{
		CreateCrashSphere(D3DVECTOR(  0.0f, 13.0f,   0.0f),20.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(  0.0f, -8.0f,   0.0f), 5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(  0.0f,-16.0f,   0.0f), 3.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(  0.0f,-22.0f,   0.0f), 3.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-21.0f,  7.0f,   9.0f), 8.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( -9.0f,  7.0f,  21.0f), 8.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 21.0f,  7.0f,   9.0f), 8.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(  9.0f,  7.0f,  21.0f), 8.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-21.0f,  7.0f,  -9.0f), 8.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( -9.0f,  7.0f, -21.0f), 8.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 21.0f,  7.0f,  -9.0f), 8.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(  9.0f,  7.0f, -21.0f), 8.0f, SOUND_BOUMm, 0.45f);
		SetGlobalSphere(D3DVECTOR(0.0f, 0.0f, 0.0f), 35.0f);

		CreateShadowCircle(30.0f, 1.0f);
	}

	pos = RetPosition(0);
	SetPosition(0, pos);  // pour afficher les ombres tout de suite

	SetFloorHeight(0.0f);
	CreateOtherObject(type);

	if ( type != OBJECT_RUINfactory &&
		 type != OBJECT_RUINconvert &&
		 type != OBJECT_RUINbase    )
	{
		FloorAdjust();
	}

	pos = RetPosition(0);
	pos.y += height;
	SetPosition(0, pos);  // pour afficher les ombres tout de suite

	if ( type == OBJECT_RUINmobilew1 )
	{
		pos = RetPosition(0);
		pos.y -= 0.5f;
		SetPosition(0, pos);

		angle = RetAngleX(0)-0.1f;
		SetAngleX(0, angle);
	}

	if ( type == OBJECT_RUINmobilew2 )
	{
		pos = RetPosition(0);
		pos.y -= 1.5f;
		SetPosition(0, pos);

		angle = RetAngleX(0)-0.9f;
		SetAngleX(0, angle);

		angle = RetAngleZ(0)-0.1f;
		SetAngleZ(0, angle);
	}

	if ( type == OBJECT_RUINmobilet1 )
	{
		pos = RetPosition(0);
		pos.y -= 0.9f;
		SetPosition(0, pos);

		angle = RetAngleX(0)-0.3f;
		SetAngleX(0, angle);
	}

	if ( type == OBJECT_RUINmobilet2 )
	{
		pos = RetPosition(0);
		pos.y -= 1.5f;
		SetPosition(0, pos);

		angle = RetAngleX(0)-0.3f;
		SetAngleX(0, angle);

		angle = RetAngleZ(0)+0.8f;
		SetAngleZ(0, angle);
	}

	if ( type == OBJECT_RUINmobiler1 )
	{
		pos = RetPosition(0);
		pos.y += 4.0f;
		SetPosition(0, pos);

		angle = RetAngleX(0)-PI*0.6f;
		SetAngleX(0, angle);

		angle = RetAngleZ(0)-0.2f;
		SetAngleZ(0, angle);
	}

	if ( type == OBJECT_RUINmobiler2 )
	{
		pos = RetPosition(0);
		pos.y += 2.0f;
		SetPosition(0, pos);

		angle = RetAngleX(0)-0.1f;
		SetAngleX(0, angle);

		angle = RetAngleZ(0)-0.3f;
		SetAngleZ(0, angle);
	}

	if ( type == OBJECT_RUINdoor )
	{
		pos = RetPosition(0);
		pos.y -= 0.5f;
		SetPosition(0, pos);

		angle = RetAngleZ(0)-0.1f;
		SetAngleZ(0, angle);
	}

	if ( type == OBJECT_RUINsupport )
	{
		pos = RetPosition(0);
		pos.y += 0.5f;
		SetPosition(0, pos);

//?		angle = RetAngleY(0)+0.1f;
//?		SetAngleY(0, angle);

		angle = RetAngleX(0)+0.1f;
		SetAngleX(0, angle);

		angle = RetAngleZ(0)+0.1f;
		SetAngleZ(0, angle);
	}

	if ( type == OBJECT_RUINradar )
	{
		pos = RetPosition(0);
		pos.y -= 0.5f;
		SetPosition(0, pos);

		angle = RetAngleX(0)+0.15f;
		SetAngleX(0, angle);

		angle = RetAngleZ(0)+0.1f;
		SetAngleZ(0, angle);
	}

	if ( type == OBJECT_RUINconvert )
	{
		pos = RetPosition(0);
		pos.y -= 1.0f;
		SetPosition(0, pos);
	}

	if ( type == OBJECT_RUINbase )
	{
		pos = RetPosition(0);
		pos.y -= 1.0f;
		SetPosition(0, pos);

		angle = RetAngleX(0)+0.15f;
		SetAngleX(0, angle);
	}

	if ( type == OBJECT_RUINhead )
	{
		pos = RetPosition(0);
		pos.y += 8.0f;
		SetPosition(0, pos);

		angle = RetAngleX(0)+PI*0.4f;
		SetAngleX(0, angle);
	}

	delete pModFile;
	return TRUE;
}

// Cr�e un gadget apollo.

BOOL CObject::CreateApollo(D3DVECTOR pos, float angle, ObjectType type)
{
	CModFile*	pModFile;
	int			rank, i;

	if ( m_engine->RetRestCreate() < 6 )  return FALSE;

	pModFile = new CModFile(m_iMan);

	SetType(type);

	if ( type == OBJECT_APOLLO1 )  // LEM ?
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);  // c'est un objet fixe
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\apollol1.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetZoom(0, 1.2f);
		SetFloorHeight(0.0f);

		for ( i=0 ; i<4 ; i++ )  // cr�e les pieds
		{
			rank = m_engine->CreateObject();
			m_engine->SetObjectType(rank, TYPEDESCENDANT);
			SetObjectRank(i+1, rank);
			SetObjectParent(i+1, 0);
			pModFile->ReadModel("objects\\apollol2.mod");
			pModFile->CreateEngineObject(rank);
			SetAngleY(i+1, PI/2.0f*i);
		}

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(5, rank);
		SetObjectParent(5, 0);
		pModFile->ReadModel("objects\\apollol3.mod");  // �chelle
		pModFile->CreateEngineObject(rank);

//?		m_terrain->AddBuildingLevel(pos, 10.0f, 13.0f, 12.0f, 0.0f);

		CreateCrashSphere(D3DVECTOR(  0.0f, 4.0f,   0.0f), 9.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 11.0f, 5.0f,   0.0f), 3.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-11.0f, 5.0f,   0.0f), 3.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(  0.0f, 5.0f, -11.0f), 3.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(  0.0f, 5.0f,  11.0f), 3.0f, SOUND_BOUMm, 0.45f);

		SetGlobalSphere(D3DVECTOR(0.0f, 4.0f, 0.0f), 9.0f);

		CreateShadowCircle(16.0f, 0.5f);
	}

	if ( type == OBJECT_APOLLO2 )  // jeep ?
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);  // c'est un objet fixe
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\apolloj1.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);

		// Roues.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(1, rank);
		SetObjectParent(1, 0);
		pModFile->ReadModel("objects\\apolloj4.mod");  // roue
		pModFile->CreateEngineObject(rank);
		SetPosition(1, D3DVECTOR(-5.75f, 1.65f, -5.0f));

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(2, rank);
		SetObjectParent(2, 0);
		pModFile->ReadModel("objects\\apolloj4.mod");  // roue
		pModFile->CreateEngineObject(rank);
		SetPosition(2, D3DVECTOR(-5.75f, 1.65f, 5.0f));

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(3, rank);
		SetObjectParent(3, 0);
		pModFile->ReadModel("objects\\apolloj4.mod");  // roue
		pModFile->CreateEngineObject(rank);
		SetPosition(3, D3DVECTOR(5.75f, 1.65f, -5.0f));

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(4, rank);
		SetObjectParent(4, 0);
		pModFile->ReadModel("objects\\apolloj4.mod");  // roue
		pModFile->CreateEngineObject(rank);
		SetPosition(4, D3DVECTOR(5.75f, 1.65f, 5.0f));

		// Accessoirs :
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(5, rank);
		SetObjectParent(5, 0);
		pModFile->ReadModel("objects\\apolloj2.mod");  // antenne
		pModFile->CreateEngineObject(rank);
		SetPosition(5, D3DVECTOR(5.5f, 8.8f, 2.0f));
		SetAngleY(5, -120.0f*PI/180.0f);
		SetAngleZ(5,   45.0f*PI/180.0f);

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(6, rank);
		SetObjectParent(6, 0);
		pModFile->ReadModel("objects\\apolloj3.mod");  // cam�ra
		pModFile->CreateEngineObject(rank);
		SetPosition(6, D3DVECTOR(5.5f, 2.8f, -2.0f));
		SetAngleY(6, 30.0f*PI/180.0f);

		CreateCrashSphere(D3DVECTOR( 3.0f, 2.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-3.0f, 2.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 7.0f, 9.0f, 2.0f), 2.0f, SOUND_BOUMm, 0.20f);

		CreateShadowCircle(7.0f, 0.8f);

		FloorAdjust();
	}

	if ( type == OBJECT_APOLLO3 )  // flag ?
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);  // c'est un objet fixe
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\apollof.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);

		SetJotlerSphere(D3DVECTOR(0.0f, 4.0f, 0.0f), 1.0f);
		CreateShadowCircle(2.0f, 0.3f);
	}

	if ( type == OBJECT_APOLLO4 )  // module ?
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);  // c'est un objet fixe
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\apollom.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);

		CreateCrashSphere(D3DVECTOR(0.0f, 2.0f, 0.0f), 2.0f, SOUND_BOUMm, 0.45f);
		CreateShadowCircle(5.0f, 0.8f);

		FloorAdjust();
	}

	if ( type == OBJECT_APOLLO5 )  // antenna ?
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);  // c'est un objet fixe
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\apolloa.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(1, rank);
		SetObjectParent(1, 0);
		pModFile->ReadModel("objects\\apolloj2.mod");  // antenne
		pModFile->CreateEngineObject(rank);
		SetPosition(1, D3DVECTOR(0.0f, 5.0f, 0.0f));
		SetAngleY(1, -120.0f*PI/180.0f);
		SetAngleZ(1,   45.0f*PI/180.0f);

		CreateCrashSphere(D3DVECTOR(0.0f, 4.0f, 0.0f), 3.0f, SOUND_BOUMm, 0.35f);
		CreateShadowCircle(3.0f, 0.7f);
	}

	CreateOtherObject(type);

	pos = RetPosition(0);
	SetPosition(0, pos);  // pour afficher les ombres tout de suite

	delete pModFile;
	return TRUE;
}

// Cr�e tous les sous-objets permettant de g�rer cet objet.

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
	if ( type == OBJECT_TEEN36 ||  // tronc ?
		 type == OBJECT_TEEN37 ||  // bateau ?
		 type == OBJECT_TEEN38 )   // ventillateur ?
	{
		m_auto = new CAutoKid(m_iMan, this);
	}
}


// Lit un programme.

BOOL CObject::ReadProgram(int rank, char* filename)
{
	if ( m_brain != 0 )
	{
		return m_brain->ReadProgram(rank, filename);
	}
	return FALSE;
}

// Ecrit un programme.

BOOL CObject::WriteProgram(int rank, char* filename)
{
	if ( m_brain != 0 )
	{
		return m_brain->WriteProgram(rank, filename);
	}
	return FALSE;
}

// D�marre un programme.

BOOL CObject::RunProgram(int rank)
{
	if ( m_brain != 0 )
	{
		m_brain->RunProgram(rank);
		return TRUE;
	}
	if ( m_auto != 0 )
	{
		m_auto->Start(rank);
		return TRUE;
	}
	return FALSE;
}




// Calcule la matrice permettant de transformer l'objet.
// Retourne TRUE si la matrice a chang�.
// Les rotations ont lieu dans l'ordre Y, Z et X.

BOOL CObject::UpdateTransformObject(int part, BOOL bForceUpdate)
{
	D3DVECTOR	position, angle, eye;
	BOOL		bModif = FALSE;
	int			parent;

	if ( m_truck != 0 )  // transport� par camion ?
	{
		m_objectPart[part].bTranslate = TRUE;
		m_objectPart[part].bRotate = TRUE;
	}

	if ( !bForceUpdate                  &&
		 !m_objectPart[part].bTranslate &&
		 !m_objectPart[part].bRotate    )  return FALSE;

	position = m_objectPart[part].position;
	angle    = m_objectPart[part].angle;

	if ( part == 0 )  // partie principale ?
	{
		position += m_linVibration;
		angle    += m_cirVibration+m_inclinaison;
	}

	if ( m_objectPart[part].bTranslate ||
		 m_objectPart[part].bRotate    )
	{
		if ( m_objectPart[part].bTranslate )
		{
			D3DUtil_SetIdentityMatrix(m_objectPart[part].matTranslate);
			m_objectPart[part].matTranslate._41 = position.x;
			m_objectPart[part].matTranslate._42 = position.y;
			m_objectPart[part].matTranslate._43 = position.z;
		}

		if ( m_objectPart[part].bRotate )
		{
			MatRotateZXY(m_objectPart[part].matRotate, angle);
		}

		if ( m_objectPart[part].bZoom )
		{
			D3DMATRIX	mz;
			D3DUtil_SetIdentityMatrix(mz);
			mz._11 = m_objectPart[part].zoom.x;
			mz._22 = m_objectPart[part].zoom.y;
			mz._33 = m_objectPart[part].zoom.z;
			m_objectPart[part].matTransform = mz *
											  m_objectPart[part].matRotate *
											  m_objectPart[part].matTranslate;
		}
		else
		{
			m_objectPart[part].matTransform = m_objectPart[part].matRotate *
											  m_objectPart[part].matTranslate;
		}
		bModif = TRUE;
	}

	if ( bForceUpdate                  ||
		 m_objectPart[part].bTranslate ||
		 m_objectPart[part].bRotate    )
	{
		parent = m_objectPart[part].parentPart;

		if ( part == 0 && m_truck != 0 )  // transport� par un camion ?
		{
			D3DMATRIX*	matWorldTruck;
			matWorldTruck = m_truck->RetWorldMatrix(m_truckLink);
			m_objectPart[part].matWorld = m_objectPart[part].matTransform *
										  *matWorldTruck;
		}
		else
		{
			if ( parent == -1 )  // pas de parent ?
			{
				m_objectPart[part].matWorld = m_objectPart[part].matTransform;
			}
			else
			{
				m_objectPart[part].matWorld = m_objectPart[part].matTransform *
											  m_objectPart[parent].matWorld;
			}
		}
		bModif = TRUE;
	}

	if ( bModif )
	{
		m_engine->SetObjectTransform(m_objectPart[part].object,
									 m_objectPart[part].matWorld);
	}

	m_objectPart[part].bTranslate = FALSE;
	m_objectPart[part].bRotate    = FALSE;

	return bModif;
}

// Met � jour toutes les matrices pour transformer l'objet p�re
// et tous ses fils.
// On suppose un maximum de 4 degr�s de libert�. Cela convient,
// par exemple, pour un corps, un bras, un avant-bras, une main
// et des doigts.

BOOL CObject::UpdateTransformObject()
{
	BOOL	bUpdate1, bUpdate2, bUpdate3, bUpdate4;
	int		level1, level2, level3, level4, rank;
	int		parent1, parent2, parent3, parent4;

	if ( m_bFlat )
	{
		for ( level1=0 ; level1<m_totalPart ; level1++ )
		{
			if ( !m_objectPart[level1].bUsed )  continue;
			UpdateTransformObject(level1, FALSE);
		}
	}
	else
	{
		parent1 = 0;
		bUpdate1 = UpdateTransformObject(parent1, FALSE);

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

	return TRUE;
}


// Met toute la descendance � plat (il n'y a plus que des p�res).
// Ceci permet de faire partir les d�bris ind�pendamment les uns
// des autres dans tous les sens.

void CObject::FlatParent()
{
	int		i;

	for ( i=0 ; i<m_totalPart ; i++ )
	{
		m_objectPart[i].position.x = m_objectPart[i].matWorld._41;
		m_objectPart[i].position.y = m_objectPart[i].matWorld._42;
		m_objectPart[i].position.z = m_objectPart[i].matWorld._43;

		m_objectPart[i].matWorld._41 = 0.0f;
		m_objectPart[i].matWorld._42 = 0.0f;
		m_objectPart[i].matWorld._43 = 0.0f;

		m_objectPart[i].matTranslate._41 = 0.0f;
		m_objectPart[i].matTranslate._42 = 0.0f;
		m_objectPart[i].matTranslate._43 = 0.0f;

		m_objectPart[i].parentPart = -1;  // plus de parent
	}

	m_bFlat = TRUE;
}



// Met � jour la mapping de la texture de la pile.

void CObject::UpdateEnergyMapping()
{
	D3DMATERIAL7	mat;
	float			a, b, i, s, au, bu;
	float			limit[6];
	int				j;

	if ( Abs(m_energy-m_lastEnergy) < 0.01f )  return;
	m_lastEnergy = m_energy;

	ZeroMemory( &mat, sizeof(D3DMATERIAL7) );
	mat.diffuse.r = 1.0f;
	mat.diffuse.g = 1.0f;
	mat.diffuse.b = 1.0f;  // blanc
	mat.ambient.r = 0.5f;
	mat.ambient.g = 0.5f;
	mat.ambient.b = 0.5f;

	if ( m_type == OBJECT_POWER  ||
		 m_type == OBJECT_ATOMIC )
	{
		a = 2.0f;
		b = 0.0f;  // dimensions de la pile (selon y)
	}
	if ( m_type == OBJECT_STATION )
	{
		a = 10.0f;
		b =  4.0f;  // dimensions de la pile (selon y)
	}
	if ( m_type == OBJECT_ENERGY )
	{
		a = 9.0f;
		b = 3.0f;  // dimensions de la pile (selon y)
	}

	i = 0.50f+0.25f*m_energy;  // origine
	s = i+0.25f;  // largeur

	au = (s-i)/(b-a);
	bu = s-b*(s-i)/(b-a);

	limit[0] = 0.0f;
	limit[1] = m_engine->RetLimitLOD(0);
	limit[2] = limit[1];
	limit[3] = m_engine->RetLimitLOD(1);
	limit[4] = limit[3];
	limit[5] = 1000000.0f;

	for ( j=0 ; j<3 ; j++ )
	{
		m_engine->ChangeTextureMapping(m_objectPart[0].object,
									   mat, D3DSTATEPART3, "lemt.tga", "",
									   limit[j*2+0], limit[j*2+1], D3DMAPPING1Y,
									   au, bu, 1.0f, 0.0f);
	}
}


// Action manuelle.

BOOL CObject::EventProcess(const Event &event)
{
	if ( event.event == EVENT_KEYDOWN )
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
			if ( event.param == 'X' )  debug_arm1 += 5.0f*PI/180.0f;
			if ( event.param == 'C' )  debug_arm1 -= 5.0f*PI/180.0f;
			if ( event.param == 'V' )  debug_arm2 += 5.0f*PI/180.0f;
			if ( event.param == 'B' )  debug_arm2 -= 5.0f*PI/180.0f;
			if ( event.param == 'N' )  debug_arm3 += 5.0f*PI/180.0f;
			if ( event.param == 'M' )  debug_arm3 -= 5.0f*PI/180.0f;
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
				sprintf(s, "a=%.2f b=%.2f c=%.2f", debug_arm1*180.0f/PI, debug_arm2*180.0f/PI, debug_arm3*180.0f/PI);
				m_engine->SetInfoText(5, s);
			}
		}
#endif
	}

	if ( m_physics != 0 )
	{
		if ( !m_physics->EventProcess(event) )  // objet d�truit ?
		{
			if ( RetSelect()             &&
				 m_type != OBJECT_ANT    &&
				 m_type != OBJECT_SPIDER &&
				 m_type != OBJECT_BEE    )
			{
				if ( !m_bDead )  m_camera->SetType(CAMERA_EXPLO);
				m_main->DeselectAll();
			}
			return FALSE;
		}
	}

	if ( m_auto != 0 )
	{
		m_auto->EventProcess(event);

		if ( event.event == EVENT_FRAME &&
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

	if ( event.event == EVENT_FRAME )
	{
		return EventFrame(event);
	}

	return TRUE;
}


// Anime l'objet.

BOOL CObject::EventFrame(const Event &event)
{
	if ( m_type == OBJECT_HUMAN && m_main->RetMainMovie() == MM_SATCOMopen )
	{
		UpdateTransformObject();
		return TRUE;
	}

	if ( m_type != OBJECT_SHOW && m_engine->RetPause() )  return TRUE;

	m_aTime += event.rTime;
	m_shotTime += event.rTime;

	VirusFrame(event.rTime);
	PartiFrame(event.rTime);

	UpdateMapping();
	UpdateTransformObject();
	UpdateSelectParticule();

	if ( m_bProxyActivate )  // active si on est proche ?
	{
		CPyro*		pyro;
		D3DVECTOR	eye;
		float		dist;

		eye = m_engine->RetLookatPt();
		dist = Length(eye, RetPosition(0));
		if ( dist < m_proxyDistance )
		{
			m_bProxyActivate = FALSE;
			m_main->CreateShortcuts();
			m_sound->Play(SOUND_FINDING);
			pyro = new CPyro(m_iMan);
			pyro->Create(PT_FINDING, this, 0.0f);
			m_displayText->DisplayError(INFO_FINDING, this);
		}
	}

	return TRUE;
}

// Met � jour le mapping de l'objet.

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


// Gestion d'un virus.

void CObject::VirusFrame(float rTime)
{
	ParticuleType	type;
	D3DVECTOR		pos, speed;
	FPOINT			dim;
	int				r;

	if ( !m_bVirusMode )  return;  // object sain ?

	m_virusTime += rTime;
	if ( m_virusTime >= VIRUS_DELAY )
	{
		m_bVirusMode = FALSE;  // le virus n'est plus actif
	}

	if ( m_lastVirusParticule+m_engine->ParticuleAdapt(0.2f) <= m_aTime )
	{
		m_lastVirusParticule = m_aTime;

		r = rand()%10;
		if ( r == 0 )  type = PARTIVIRUS1;
		if ( r == 1 )  type = PARTIVIRUS2;
		if ( r == 2 )  type = PARTIVIRUS3;
		if ( r == 3 )  type = PARTIVIRUS4;
		if ( r == 4 )  type = PARTIVIRUS5;
		if ( r == 5 )  type = PARTIVIRUS6;
		if ( r == 6 )  type = PARTIVIRUS7;
		if ( r == 7 )  type = PARTIVIRUS8;
		if ( r == 8 )  type = PARTIVIRUS9;
		if ( r == 9 )  type = PARTIVIRUS10;

		pos = RetPosition(0);
		pos.x += (Rand()-0.5f)*10.0f;
		pos.z += (Rand()-0.5f)*10.0f;
		speed.x = (Rand()-0.5f)*2.0f;
		speed.z = (Rand()-0.5f)*2.0f;
		speed.y = Rand()*4.0f+4.0f;
		dim.x = Rand()*0.3f+0.3f;
		dim.y = dim.x;

		m_particule->CreateParticule(pos, speed, dim, type, 3.0f);
	}
}

// Gestion des particules ma�tresses.

void CObject::PartiFrame(float rTime)
{
	D3DVECTOR	pos, angle, factor;
	int			i, channel;

	for ( i=0 ; i<OBJECTMAXPART ; i++ )
	{
		if ( !m_objectPart[i].bUsed )  continue;

		channel = m_objectPart[i].masterParti;
		if ( channel == -1 )  continue;

		if ( !m_particule->GetPosition(channel, pos) )
		{
			m_objectPart[i].masterParti = -1;  // particule n'existe plus !
			continue;
		}

		SetPosition(i, pos);

		// Chaque morceau tournoie diff�remment.
		switch( i%5 )
		{
			case 0:  factor = D3DVECTOR( 0.5f, 0.3f, 0.6f); break;
			case 1:  factor = D3DVECTOR(-0.3f, 0.4f,-0.2f); break;
			case 2:  factor = D3DVECTOR( 0.4f,-0.6f,-0.3f); break;
			case 3:  factor = D3DVECTOR(-0.6f,-0.2f, 0.0f); break;
			case 4:  factor = D3DVECTOR( 0.4f, 0.1f,-0.7f); break;
		}

		angle = RetAngle(i);
		angle += rTime*PI*factor;
		SetAngle(i, angle);
	}
}


// Modifie le point de vue pour voir comme si on �tait
// dans le v�hicule, ou derri�re le v�hicule.

void CObject::SetViewFromHere(D3DVECTOR &eye, float &dirH, float &dirV,
							  D3DVECTOR	&lookat, D3DVECTOR &upVec,
							  CameraType type)
{
	float	speed;
	int		part;

	UpdateTransformObject();

	part = 0;
	if ( m_type == OBJECT_HUMAN ||
		 m_type == OBJECT_TECH  )
	{
		eye.x = -0.2f;
		eye.y =  3.3f;
		eye.z =  0.0f;
//?		eye.x =  1.0f;
//?		eye.y =  3.3f;
//?		eye.z =  0.0f;
	}
	else if ( m_type == OBJECT_MOBILErt ||
			  m_type == OBJECT_MOBILErr ||
			  m_type == OBJECT_MOBILErs )
	{
		eye.x = -1.1f;  // sur le capot
		eye.y =  7.9f;
		eye.z =  0.0f;
	}
	else if ( m_type == OBJECT_MOBILEwc ||
			  m_type == OBJECT_MOBILEtc ||
			  m_type == OBJECT_MOBILEfc ||
			  m_type == OBJECT_MOBILEic )  // fireball ?
	{
//?		eye.x = -0.9f;  // sur le canon
//?		eye.y =  3.0f;
//?		eye.z =  0.0f;
//?		part = 1;
		eye.x = -0.9f;  // sur le canon
		eye.y =  8.3f;
		eye.z =  0.0f;
	}
	else if ( m_type == OBJECT_MOBILEwi ||
			  m_type == OBJECT_MOBILEti ||
			  m_type == OBJECT_MOBILEfi ||
			  m_type == OBJECT_MOBILEii )  // orgaball ?
	{
//?		eye.x = -3.5f;  // sur le canon
//?		eye.y =  5.1f;
//?		eye.z =  0.0f;
//?		part = 1;
		eye.x = -2.5f;  // sur le canon
		eye.y = 10.4f;
		eye.z =  0.0f;
	}
	else if ( m_type == OBJECT_MOBILErc )
	{
//?		eye.x =  2.0f;  // dans le canon
//?		eye.y =  0.0f;
//?		eye.z =  0.0f;
//?		part = 2;
		eye.x =  4.0f;  // sur le canon
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
		eye.x = 0.7f;  // entre les supports
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

	if ( type == CAMERA_BACK )
	{
		eye.x -= 20.0f;
		eye.y +=  1.0f;
	}

	lookat.x = eye.x+1.0f;
	lookat.y = eye.y+0.0f;
	lookat.z = eye.z+0.0f;

	eye    = Transform(m_objectPart[part].matWorld, eye);
	lookat = Transform(m_objectPart[part].matWorld, lookat);

	// Penche la cam�ra dans les virages.
	upVec = D3DVECTOR(0.0f, 1.0f, 0.0f);
	if ( m_physics != 0 )
	{
		if ( m_physics->RetLand() )  // au sol ?
		{
			speed = m_physics->RetLinMotionX(MO_REASPEED);
			lookat.y -= speed*0.002f;

			speed = m_physics->RetCirMotionY(MO_REASPEED);
			upVec.z -= speed*0.04f;
		}
		else	// en vol ?
		{
			speed = m_physics->RetLinMotionX(MO_REASPEED);
			lookat.y += speed*0.002f;

			speed = m_physics->RetCirMotionY(MO_REASPEED);
			upVec.z += speed*0.08f;
		}
	}
	upVec = Transform(m_objectPart[0].matRotate, upVec);

	dirH = -(m_objectPart[part].angle.y+PI/2.0f);
	dirV = 0.0f;

}


// Gestion des caract�ristiques.

void CObject::SetCharacter(Character* character)
{
	CopyMemory(&m_character, character, sizeof(Character));
}

void CObject::GetCharacter(Character* character)
{
	CopyMemory(character, &m_character, sizeof(Character));
}

Character* CObject::RetCharacter()
{
	return &m_character;
}


// Retourne le temps absolu.

float CObject::RetAbsTime()
{
	return m_aTime;
}


// Gestion de l'�nergie contenue dans une pile.
// Seul l'objet pile poss�de de l'�nergie, mais pas le v�hicule
// qui transporte la pile !

void CObject::SetEnergy(float level)
{
	if ( level < 0.0f )  level = 0.0f;
	if ( level > 1.0f )  level = 1.0f;
	m_energy = level;
}

float CObject::RetEnergy()
{
	if ( m_type != OBJECT_POWER   &&
		 m_type != OBJECT_ATOMIC  &&
		 m_type != OBJECT_STATION &&
		 m_type != OBJECT_ENERGY  )  return 0.0f;
	return m_energy;
}


// Gestion de la capacit� d'une pile.
// Seul l'objet pile poss�de une capacit�, mais pas le v�hicule
// qui transporte la pile !

void CObject::SetCapacity(float capacity)
{
	m_capacity = capacity;
}

float CObject::RetCapacity()
{
	return m_capacity;
}


// Gestion du bouclier.

void CObject::SetShield(float level)
{
	m_shield = level;
}

float CObject::RetShield()
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
		 m_type == OBJECT_TEEN31    ||  // basket ?
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


// Gestion de l'autonomie de vol (z�ro = infini).

void CObject::SetRange(float delay)
{
	m_range = delay;
}

float CObject::RetRange()
{
	return m_range;
}


// Gestion du facteur de transparence de l'objet.

void CObject::SetTransparency(float value)
{
	int		i;

	m_transparency = value;

	for ( i=0 ; i<m_totalPart ; i++ )
	{
		if ( m_objectPart[i].bUsed )
		{
			if ( m_type == OBJECT_BASE )
			{
				if ( i != 9 )  continue;  // pas pilier central ?
			}

			m_engine->SetObjectTransparency(m_objectPart[i].object, value);
		}
	}
}

float CObject::RetTransparency()
{
	return m_transparency;
}


// Gestion de la mati�re de l'objet.

ObjectMaterial CObject::RetMaterial()
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


// Indique si l'objet est un gadget non indispensable.

void CObject::SetGadget(BOOL bMode)
{
	m_bGadget = bMode;
}

BOOL CObject::RetGadget()
{
	return m_bGadget;
}


// Indique si un objet est immobile (fourmi sur le dos).

void CObject::SetFixed(BOOL bFixed)
{
	m_bFixed = bFixed;
}

BOOL CObject::RetFixed()
{
	return m_bFixed;
}


// Indique si un objet est soumis au clipping (obstacles).

void CObject::SetClip(BOOL bClip)
{
	m_bClip = bClip;
}

BOOL CObject::RetClip()
{
	return m_bClip;
}



// Bouscule un objet.

BOOL CObject::JostleObject(float force)
{
	CAutoJostle*	pa;

	if ( m_type == OBJECT_FLAGb ||
		 m_type == OBJECT_FLAGr ||
		 m_type == OBJECT_FLAGg ||
		 m_type == OBJECT_FLAGy ||
		 m_type == OBJECT_FLAGv )  // drapeau ?
	{
		if ( m_auto == 0 )  return FALSE;

		m_auto->Start(1);
	}
	else
	{
		if ( m_auto != 0 )  return FALSE;

		m_auto = new CAutoJostle(m_iMan, this);
		pa = (CAutoJostle*)m_auto;
		pa->Start(0, force);
	}

	return TRUE;
}


// D�but de l'effet lorsque l'instruction "detect" est utilis�e.

void CObject::StartDetectEffect(CObject *target, BOOL bFound)
{
	D3DMATRIX*	mat;
	D3DVECTOR	pos, goal;
	FPOINT		dim;

	mat = RetWorldMatrix(0);
	pos = Transform(*mat, D3DVECTOR(2.0f, 3.0f, 0.0f));

	if ( target == 0 )
	{
		goal = Transform(*mat, D3DVECTOR(50.0f, 3.0f, 0.0f));
	}
	else
	{
		goal = target->RetPosition(0);
		goal.y += 3.0f;
		goal = SegmentDist(pos, goal, Length(pos, goal)-3.0f);
	}

	dim.x = 3.0f;
	dim.y = dim.x;
	m_particule->CreateRay(pos, goal, PARTIRAY2, dim, 0.2f);

	if ( target != 0 )
	{
		goal = target->RetPosition(0);
		goal.y += 3.0f;
		goal = SegmentDist(pos, goal, Length(pos, goal)-1.0f);
		dim.x = 6.0f;
		dim.y = dim.x;
		m_particule->CreateParticule(goal, D3DVECTOR(0.0f, 0.0f, 0.0f), dim,
									 bFound?PARTIGLINT:PARTIGLINTr, 0.5f);
	}

	m_sound->Play(bFound?SOUND_BUILD:SOUND_RECOVER);
}


// Gestion du temps depuis lequel un virus est actif.

void CObject::SetVirusMode(BOOL bEnable)
{
	m_bVirusMode = bEnable;
	m_virusTime = 0.0f;

	if ( m_bVirusMode && m_brain != 0 )
	{
		if ( !m_brain->IntroduceVirus() )  // essaye de contaminer
		{
			m_bVirusMode = FALSE;  // pas de programme � contaminer !
		}
	}
}

BOOL CObject::RetVirusMode()
{
	return m_bVirusMode;
}

float CObject::RetVirusTime()
{
	return m_virusTime;
}


// Gestion du mode de la cam�ra.

void CObject::SetCameraType(CameraType type)
{
	m_cameraType = type;
}

CameraType CObject::RetCameraType()
{
	return m_cameraType;
}

void CObject::SetCameraDist(float dist)
{
	m_cameraDist = dist;
}

float CObject::RetCameraDist()
{
	return m_cameraDist;
}

void CObject::SetCameraLock(BOOL bLock)
{
	m_bCameraLock = bLock;
}

BOOL CObject::RetCameraLock()
{
	return m_bCameraLock;
}



// Gestion de la mise en �vidence de l'objet.

void CObject::SetHilite(BOOL bMode)
{
	int		list[OBJECTMAXPART+1];
	int		i, j;

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
		list[j] = -1;  // terminateur

		m_engine->SetHiliteRank(list);  // donne la liste des parties s�lectionn�es
	}
}

BOOL CObject::RetHilite()
{
	return m_bHilite;
}


// Indique si l'objet est s�lecionn� ou non.

void CObject::SetSelect(BOOL bMode, BOOL bDisplayError)
{
	Error		err;

	m_bSelect = bMode;

	if ( m_physics != 0 )
	{
		m_physics->CreateInterface(m_bSelect);
	}

	if ( m_auto != 0 )
	{
		m_auto->CreateInterface(m_bSelect);
	}

	CreateSelectParticule();  // cr�e/supprime les particules

	if ( !m_bSelect )
	{
		SetGunGoalH(0.0f);  // met le canon droit
		return;  // fini si pas s�lectionn�
	}

	err = ERR_OK;
	if ( m_physics != 0 )
	{
		err = m_physics->RetError();
	}
	if ( m_auto != 0 )
	{
		err = m_auto->RetError();
	}
	if ( err != ERR_OK && bDisplayError )
	{
		m_displayText->DisplayError(err, this);
	}
}

// Indique si l'objet est s�lectionn� ou non.

BOOL CObject::RetSelect(BOOL bReal)
{
	if ( !bReal && m_main->RetFixScene() )  return FALSE;
	return m_bSelect;
}


// Indique si l'objet est s�lecionnable ou non.

void CObject::SetSelectable(BOOL bMode)
{
	m_bSelectable = bMode;
}

// Indique si l'objet est s�lecionnable ou non.

BOOL CObject::RetSelectable()
{
	return m_bSelectable;
}


// Gestion de l'activit� d'un objet.

void CObject::SetActivity(BOOL bMode)
{
	if ( m_brain != 0 )
	{
		m_brain->SetActivity(bMode);
	}
}

BOOL CObject::RetActivity()
{
	if ( m_brain != 0 )
	{
		return m_brain->RetActivity();
	}
	return FALSE;
}


// Indique si faut v�rifier les tokens de l'objet.

void CObject::SetCheckToken(BOOL bMode)
{
	m_bCheckToken = bMode;
}

// Indique si faut v�rifier les tokens de l'objet.

BOOL CObject::RetCheckToken()
{
	return m_bCheckToken;
}


// Gestion de la visibilit� d'un objet.
// L'objet n'est pas cach� visuellement ni inactif, mais ignor�
// des d�tections ! Par exemple: ver sous terre.

void CObject::SetVisible(BOOL bVisible)
{
	m_bVisible = bVisible;
}

BOOL CObject::RetVisible()
{
	return m_bVisible;
}


// Gestion du mode de fonctionnement d'un objet. Un objet
// inactif est identique � un objet d�truit, inexistant.
// Ce mode est utilis� pour les objets "resetables" lors
// d'entra�nement, pour simuler une destruction.

void CObject::SetEnable(BOOL bEnable)
{
	m_bEnable = bEnable;
}

BOOL CObject::RetEnable()
{
	return m_bEnable;
}


// Gestion du mode o� un objet n'est activ� que lorsqu'on
// est proche.

void CObject::SetProxyActivate(BOOL bActivate)
{
	m_bProxyActivate = bActivate;
}

BOOL CObject::RetProxyActivate()
{
	return m_bProxyActivate;
}

void CObject::SetProxyDistance(float distance)
{
	m_proxyDistance = distance;
}

float CObject::RetProxyDistance()
{
	return m_proxyDistance;
}


// Gestion du mode d'augmentation des dommages.

void CObject::SetMagnifyDamage(float factor)
{
	m_magnifyDamage = factor;
}

float CObject::RetMagnifyDamage()
{
	return m_magnifyDamage;
}


// Gestion du param�tre libre.

void CObject::SetParam(float value)
{
	m_param = value;
}

float CObject::RetParam()
{
	return m_param;
}


// Gestion du mode "bloqu�" d'un objet.
// Par exemple, un cube de titanium est bloqu� pendant qu'il est utilis�
// pour fabriquer qq chose, ou un v�hicule est bloqu� tant que sa
// construction n'est pas termin�e.

void CObject::SetLock(BOOL bLock)
{
	m_bLock = bLock;
}

BOOL CObject::RetLock()
{
	return m_bLock;
}

// Gestion du mode "en cours d'explosion" d'un objet.
// Un objet dans ce mode n'est pas sauvegard�.

void CObject::SetExplo(BOOL bExplo)
{
	m_bExplo = bExplo;
}

BOOL CObject::RetExplo()
{
	return m_bExplo;
}


// Gestion du mode "cargaison du vaisseau" pendant les films.

void CObject::SetCargo(BOOL bCargo)
{
	m_bCargo = bCargo;
}

BOOL CObject::RetCargo()
{
	return m_bCargo;
}


// Gestion du mode HS d'un objet.

void CObject::SetBurn(BOOL bBurn)
{
	m_bBurn = bBurn;

//?	if ( m_botVar != 0 )
//?	{
//?		if ( m_bBurn )  m_botVar->SetUserPtr(OBJECTDELETED);
//?		else            m_botVar->SetUserPtr(this);
//?	}
}

BOOL CObject::RetBurn()
{
	return m_bBurn;
}

void CObject::SetDead(BOOL bDead)
{
	m_bDead = bDead;

	if ( bDead && m_brain != 0 )
	{
		m_brain->StopProgram();  // stoppe la t�che en cours
	}

//?	if ( m_botVar != 0 )
//?	{
//?		if ( m_bDead )  m_botVar->SetUserPtr(OBJECTDELETED);
//?		else            m_botVar->SetUserPtr(this);
//?	}
}

BOOL CObject::RetDead()
{
	return m_bDead;
}

BOOL CObject::RetRuin()
{
	return m_bBurn|m_bFlat;
}

BOOL CObject::RetActif()
{
	return !m_bLock && !m_bBurn && !m_bFlat && m_bVisible && m_bEnable;
}


// Gestion du point de vis�e.

void CObject::SetGunGoalV(float gunGoal)
{
	if ( m_type == OBJECT_MOBILEfc ||
		 m_type == OBJECT_MOBILEtc ||
		 m_type == OBJECT_MOBILEwc ||
		 m_type == OBJECT_MOBILEic )  // fireball ?
	{
		if ( gunGoal >  10.0f*PI/180.0f )  gunGoal =  10.0f*PI/180.0f;
		if ( gunGoal < -20.0f*PI/180.0f )  gunGoal = -20.0f*PI/180.0f;
		SetAngleZ(1, gunGoal);
	}
	else if ( m_type == OBJECT_MOBILEfi ||
			  m_type == OBJECT_MOBILEti ||
			  m_type == OBJECT_MOBILEwi ||
			  m_type == OBJECT_MOBILEii )  // orgaball ?
	{
		if ( gunGoal >  20.0f*PI/180.0f )  gunGoal =  20.0f*PI/180.0f;
		if ( gunGoal < -20.0f*PI/180.0f )  gunGoal = -20.0f*PI/180.0f;
		SetAngleZ(1, gunGoal);
	}
	else if ( m_type == OBJECT_MOBILErc )  // phazer ?
	{
		if ( gunGoal >  45.0f*PI/180.0f )  gunGoal =  45.0f*PI/180.0f;
		if ( gunGoal < -20.0f*PI/180.0f )  gunGoal = -20.0f*PI/180.0f;
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
		 m_type == OBJECT_MOBILEic )  // fireball ?
	{
		if ( gunGoal >  40.0f*PI/180.0f )  gunGoal =  40.0f*PI/180.0f;
		if ( gunGoal < -40.0f*PI/180.0f )  gunGoal = -40.0f*PI/180.0f;
		SetAngleY(1, gunGoal);
	}
	else if ( m_type == OBJECT_MOBILEfi ||
			  m_type == OBJECT_MOBILEti ||
			  m_type == OBJECT_MOBILEwi ||
			  m_type == OBJECT_MOBILEii )  // orgaball ?
	{
		if ( gunGoal >  40.0f*PI/180.0f )  gunGoal =  40.0f*PI/180.0f;
		if ( gunGoal < -40.0f*PI/180.0f )  gunGoal = -40.0f*PI/180.0f;
		SetAngleY(1, gunGoal);
	}
	else if ( m_type == OBJECT_MOBILErc )  // phazer ?
	{
		if ( gunGoal >  40.0f*PI/180.0f )  gunGoal =  40.0f*PI/180.0f;
		if ( gunGoal < -40.0f*PI/180.0f )  gunGoal = -40.0f*PI/180.0f;
		SetAngleY(2, gunGoal);
	}
	else
	{
		gunGoal = 0.0f;
	}

	m_gunGoalH = gunGoal;
}

float CObject::RetGunGoalV()
{
	return m_gunGoalV;
}

float CObject::RetGunGoalH()
{
	return m_gunGoalH;
}



// Montre les limites de l'objet.

BOOL CObject::StartShowLimit()
{
	if ( m_showLimitRadius == 0.0f )  return FALSE;

	m_main->SetShowLimit(0, PARTILIMIT1, this, RetPosition(0), m_showLimitRadius);
	m_bShowLimit = TRUE;
	return TRUE;
}

void CObject::StopShowLimit()
{
	m_bShowLimit = FALSE;
}



// Indique si un programme est en cours d'ex�cution.

BOOL CObject::IsProgram()
{
	if ( m_brain == 0 )  return FALSE;
	return m_brain->IsProgram();
}


// Cr�e ou supprime les particules associ�es � l'objet.

void CObject::CreateSelectParticule()
{
	D3DVECTOR	pos, speed;
	FPOINT		dim;
	int			i;

	// Supprime les particules pr�c�dentes.
	for ( i=0 ; i<4 ; i++ )
	{
		if ( m_partiSel[i] != -1 )
		{
			m_particule->DeleteParticule(m_partiSel[i]);
			m_partiSel[i] = -1;
		}
	}

	if ( m_bSelect || IsProgram() )
	{
		// Cr�e les particules lens pour les phares.
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
			 m_type == OBJECT_MOBILEdr )  // v�hicule ?
		{
			pos = D3DVECTOR(0.0f, 0.0f, 0.0f);
			speed = D3DVECTOR(0.0f, 0.0f, 0.0f);
			dim.x = 0.0f;
			dim.y = 0.0f;
			m_partiSel[0] = m_particule->CreateParticule(pos, speed, dim, PARTISELY, 1.0f, 0.0f, 0.0f);
			m_partiSel[1] = m_particule->CreateParticule(pos, speed, dim, PARTISELY, 1.0f, 0.0f, 0.0f);
			m_partiSel[2] = m_particule->CreateParticule(pos, speed, dim, PARTISELR, 1.0f, 0.0f, 0.0f);
			m_partiSel[3] = m_particule->CreateParticule(pos, speed, dim, PARTISELR, 1.0f, 0.0f, 0.0f);
			UpdateSelectParticule();
		}
	}
}

// Met � jour les particules associ�es � l'objet.

void CObject::UpdateSelectParticule()
{
	D3DVECTOR	pos[4];
	FPOINT		dim[4];
	float		zoom[4];
	float		angle;
	int			i;

	if ( !m_bSelect && !IsProgram() )  return;

	dim[0].x = 1.0f;
	dim[1].x = 1.0f;
	dim[2].x = 1.2f;
	dim[3].x = 1.2f;

	// Lens avants jaunes.
	if ( m_type == OBJECT_MOBILErt ||
		 m_type == OBJECT_MOBILErc ||
		 m_type == OBJECT_MOBILErr ||
		 m_type == OBJECT_MOBILErs )  // grosses chenilles ?
	{
		pos[0] = D3DVECTOR(4.2f, 2.8f,  1.5f);
		pos[1] = D3DVECTOR(4.2f, 2.8f, -1.5f);
		dim[0].x = 1.5f;
		dim[1].x = 1.5f;
	}
	else if ( m_type == OBJECT_MOBILEwt ||
			  m_type == OBJECT_MOBILEtt ||
			  m_type == OBJECT_MOBILEft ||
			  m_type == OBJECT_MOBILEit )  // trainer ?
	{
		pos[0] = D3DVECTOR(4.2f, 2.5f,  1.2f);
		pos[1] = D3DVECTOR(4.2f, 2.5f, -1.2f);
		dim[0].x = 1.5f;
		dim[1].x = 1.5f;
	}
	else if ( m_type == OBJECT_MOBILEsa )  // sous-marin ?
	{
		pos[0] = D3DVECTOR(3.6f, 4.0f,  2.0f);
		pos[1] = D3DVECTOR(3.6f, 4.0f, -2.0f);
	}
	else if ( m_type == OBJECT_MOBILEtg )  // cible ?
	{
		pos[0] = D3DVECTOR(3.4f, 6.5f,  2.0f);
		pos[1] = D3DVECTOR(3.4f, 6.5f, -2.0f);
	}
	else if ( m_type == OBJECT_MOBILEdr )  // dessinateur ?
	{
		pos[0] = D3DVECTOR(4.9f, 3.5f,  2.5f);
		pos[1] = D3DVECTOR(4.9f, 3.5f, -2.5f);
	}
	else
	{
		pos[0] = D3DVECTOR(4.2f, 2.5f,  1.5f);
		pos[1] = D3DVECTOR(4.2f, 2.5f, -1.5f);
	}

	// Lens arri�res rouges.
	if ( m_type == OBJECT_MOBILEfa ||
		 m_type == OBJECT_MOBILEfc ||
		 m_type == OBJECT_MOBILEfi ||
		 m_type == OBJECT_MOBILEfs ||
		 m_type == OBJECT_MOBILEft )  // volant ?
	{
		pos[2] = D3DVECTOR(-4.0f, 3.1f,  4.5f);
		pos[3] = D3DVECTOR(-4.0f, 3.1f, -4.5f);
		dim[2].x = 0.6f;
		dim[3].x = 0.6f;
	}
	if ( m_type == OBJECT_MOBILEwa ||
		 m_type == OBJECT_MOBILEwc ||
		 m_type == OBJECT_MOBILEwi ||
		 m_type == OBJECT_MOBILEws )  // roues ?
	{
		pos[2] = D3DVECTOR(-4.5f, 2.7f,  2.8f);
		pos[3] = D3DVECTOR(-4.5f, 2.7f, -2.8f);
	}
	if ( m_type == OBJECT_MOBILEwt )  // roues ?
	{
		pos[2] = D3DVECTOR(-4.0f, 2.5f,  2.2f);
		pos[3] = D3DVECTOR(-4.0f, 2.5f, -2.2f);
	}
	if ( m_type == OBJECT_MOBILEia ||
		 m_type == OBJECT_MOBILEic ||
		 m_type == OBJECT_MOBILEii ||
		 m_type == OBJECT_MOBILEis ||
		 m_type == OBJECT_MOBILEit )  // pattes ?
	{
		pos[2] = D3DVECTOR(-4.5f, 2.7f,  2.8f);
		pos[3] = D3DVECTOR(-4.5f, 2.7f, -2.8f);
	}
	if ( m_type == OBJECT_MOBILEta ||
		 m_type == OBJECT_MOBILEtc ||
		 m_type == OBJECT_MOBILEti ||
		 m_type == OBJECT_MOBILEts ||
		 m_type == OBJECT_MOBILEtt )  // chenilles ?
	{
		pos[2] = D3DVECTOR(-3.6f, 4.2f,  3.0f);
		pos[3] = D3DVECTOR(-3.6f, 4.2f, -3.0f);
	}
	if ( m_type == OBJECT_MOBILErt ||
		 m_type == OBJECT_MOBILErc ||
		 m_type == OBJECT_MOBILErr ||
		 m_type == OBJECT_MOBILErs )  // grosses chenilles ?
	{
		pos[2] = D3DVECTOR(-5.0f, 5.2f,  2.5f);
		pos[3] = D3DVECTOR(-5.0f, 5.2f, -2.5f);
	}
	if ( m_type == OBJECT_MOBILEsa )  // sous-marin ?
	{
		pos[2] = D3DVECTOR(-3.6f, 4.0f,  2.0f);
		pos[3] = D3DVECTOR(-3.6f, 4.0f, -2.0f);
	}
	if ( m_type == OBJECT_MOBILEtg )  // cible ?
	{
		pos[2] = D3DVECTOR(-2.4f, 6.5f,  2.0f);
		pos[3] = D3DVECTOR(-2.4f, 6.5f, -2.0f);
	}
	if ( m_type == OBJECT_MOBILEdr )  // dessinateur ?
	{
		pos[2] = D3DVECTOR(-5.3f, 2.7f,  1.8f);
		pos[3] = D3DVECTOR(-5.3f, 2.7f, -1.8f);
	}

	angle = RetAngleY(0)/PI;

	zoom[0] = 1.0f;
	zoom[1] = 1.0f;
	zoom[2] = 1.0f;
	zoom[3] = 1.0f;

	if ( IsProgram() &&  // programme en cours ?
		 Mod(m_aTime, 0.7f) < 0.3f )
	{
		zoom[0] = 0.0f;  // clignotte
		zoom[1] = 0.0f;
		zoom[2] = 0.0f;
		zoom[3] = 0.0f;
	}

	// Met � jour tous les lens.
	for ( i=0 ; i<4 ; i++ )
	{
		pos[i] = Transform(m_objectPart[0].matWorld, pos[i]);
		dim[i].y = dim[i].x;
		m_particule->SetParam(m_partiSel[i], pos[i], dim[i], zoom[i], angle, 1.0f);
	}
}


// Donne le pointeur au script en cours d'ex�cution.

void CObject::SetRunScript(CScript* script)
{
	m_runScript = script;
}

CScript* CObject::RetRunScript()
{
	return m_runScript;
}

// Retourne les variables du "this" pour CBOT.

CBotVar* CObject::RetBotVar()
{
	return m_botVar;
}

// Retourne la physique associ�e � l'objet.

CPhysics* CObject::RetPhysics()
{
	return m_physics;
}

// Retourne le cerveau associ� � l'objet.

CBrain* CObject::RetBrain()
{
	return m_brain;
}

// Retourne le mouvement associ� � l'objet.

CMotion* CObject::RetMotion()
{
	return m_motion;
}

// Retourne l'automate associ� � l'objet.

CAuto* CObject::RetAuto()
{
	return m_auto;
}

void CObject::SetAuto(CAuto* automat)
{
	m_auto = automat;
}



// Gestion du rang dans le fichier de d�finition.

void CObject::SetDefRank(int rank)
{
	m_defRank = rank;
}

int  CObject::RetDefRank()
{
	return m_defRank;
}


// Donne le nom de l'objet pour le tooltip.

BOOL CObject::GetTooltipName(char* name)
{
	GetResource(RES_OBJECT, m_type, name);
	return ( name[0] != 0 );
}


// Ajoute l'objet pr�c�demment s�lectionn� dans la liste.

void CObject::AddDeselList(CObject* pObj)
{
	int		i;

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

// Enl�ve l'objet pr�c�demment s�lectionn� dans la liste.

CObject* CObject::SubDeselList()
{
	if ( m_totalDesectList == 0 )  return 0;

	return m_objectDeselectList[--m_totalDesectList];
}

// Supprime un objet s'il est r�f�renc� dans la liste.

void CObject::DeleteDeselList(CObject* pObj)
{
	int		i, j;

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



// Gestion de l'�tat du crayon du robot dessinateur.

BOOL CObject::RetTraceDown()
{
	CMotionVehicle*	mv;
	if ( m_motion == 0 )  return FALSE;
	mv = (CMotionVehicle*)m_motion;
	return mv->RetTraceDown();
}

void CObject::SetTraceDown(BOOL bDown)
{
	CMotionVehicle*	mv;
	if ( m_motion == 0 )  return;
	mv = (CMotionVehicle*)m_motion;
	mv->SetTraceDown(bDown);
}

int CObject::RetTraceColor()
{
	CMotionVehicle*	mv;
	if ( m_motion == 0 )  return 0;
	mv = (CMotionVehicle*)m_motion;
	return mv->RetTraceColor();
}

void CObject::SetTraceColor(int color)
{
	CMotionVehicle*	mv;
	if ( m_motion == 0 )  return;
	mv = (CMotionVehicle*)m_motion;
	mv->SetTraceColor(color);
}

float CObject::RetTraceWidth()
{
	CMotionVehicle*	mv;
	if ( m_motion == 0 )  return 0.0f;
	mv = (CMotionVehicle*)m_motion;
	return mv->RetTraceWidth();
}

void CObject::SetTraceWidth(float width)
{
	CMotionVehicle*	mv;
	if ( m_motion == 0 )  return;
	mv = (CMotionVehicle*)m_motion;
	mv->SetTraceWidth(width);
}


