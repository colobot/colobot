// object.cpp

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
#include "recorder.h"
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
#include "motionvehicle.h"
#include "motionbot.h"
#include "modfile.h"
#include "auto.h"
#include "autotower.h"
#include "autoroot.h"
#include "autojostle.h"
#include "autoalien.h"
#include "autodoor.h"
#include "autodock.h"
#include "autoremote.h"
#include "autostand.h"
#include "autogenerator.h"
#include "autocomputer.h"
#include "autorepair.h"
#include "autofire.h"
#include "autohelico.h"
#include "autocompass.h"
#include "autoblitzer.h"
#include "autoinca.h"
#include "autohook.h"
#include "autobarrel.h"
#include "autobomb.h"
#include "autohome.h"
#include "task.h"
#include "pyro.h"
#include "sound.h"
#include "displaytext.h"
#include "cmdtoken.h"
#include "cbottoken.h"
#include "object.h"



#define ADJUST_ONBOARD	FALSE		// TRUE -> ajuste la caméra ONBOARD
#define ADJUST_ARM		FALSE		// TRUE -> ajuste le bras manipulateur
#define LOSS_SHIELD		0.24f		// perte du bouclier par coup

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




// Met à jour la classe Object.

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

	if ( object == 0 )  return;

	physics = object->RetPhysics();

	// Met à jour le type de l'objet.
	pVar = botThis->GivItemList();  // "category"
	type = object->RetType();
	pVar->SetValInt(type, "");

	// Met à jour la position de l'objet.
	pVar = pVar->GivNext();  // "position"
	if ( object->RetTruck() == 0 )
	{
		pos = object->RetPosition(0);
		pos.y -= object->RetWaterLevel();  // relatif au niveau de la mer !
		pSub = pVar->GivItemList();  // "x"
		pSub->SetValFloat(pos.x/UNIT);
		pSub = pSub->GivNext();  // "y"
		pSub->SetValFloat(pos.z/UNIT);
		pSub = pSub->GivNext();  // "z"
		pSub->SetValFloat(pos.y/UNIT);
	}
	else	// objet transporté ?
	{
		pSub = pVar->GivItemList();  // "x"
		pSub->SetInit(IS_NAN);
		pSub = pSub->GivNext();  // "y"
		pSub->SetInit(IS_NAN);
		pSub = pSub->GivNext();  // "z"
		pSub->SetInit(IS_NAN);
	}

	// Met à jour l'angle.
	pos = object->RetAngle(0);
	pos += object->RetInclinaison();
	pVar = pVar->GivNext();  // "orientation"
	pVar->SetValFloat(360.0f-Mod(pos.y*180.0f/PI, 360.0f));
	pVar = pVar->GivNext();  // "pitch"
	pVar->SetValFloat(pos.z*180.0f/PI);
	pVar = pVar->GivNext();  // "roll"
	pVar->SetValFloat(pos.x*180.0f/PI);

	// Met à jour le niveau d'énergie de l'objet.
	pVar = pVar->GivNext();  // "energyLevel"
	pVar->SetValFloat(1.0f);

	// Met à jour le niveau du bouclier de l'objet.
	pVar = pVar->GivNext();  // "shieldLevel"
	value = object->RetShield();
	pVar->SetValFloat(value);

	// Met à jour la température du bouclier.
	pVar = pVar->GivNext();  // "temperature"
	pVar->SetValFloat(1.0f);

	// Met à jour la hauteur au-dessus du sol.
	pVar = pVar->GivNext();  // "altitude"
	if ( physics == 0 )  value = 0.0f;
	else                 value = physics->RetFloorHeight();
	pVar->SetValFloat(value/UNIT);

	// Met à jour le temps de l'objet.
	pVar = pVar->GivNext();  // "lifeTime"
	value = object->RetAbsTime();
	pVar->SetValFloat(value);

	// Met à jour le type de la pile.
	pVar = pVar->GivNext();  // "energyCell"
	power = object->RetPower();
	if ( power == 0 )  pVar->SetPointer(0);
	else               pVar->SetPointer(power->RetBotVar());

	// Met à jour le type de l'objet transporté.
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
	m_model = 0;
	m_subModel = 0;
	m_id = ++g_id;
	m_option = 0;
	m_shadowLight   = -1;
	m_effectLight   = -1;
	m_linVibration  = D3DVECTOR(0.0f, 0.0f, 0.0f);
	m_cirVibration  = D3DVECTOR(0.0f, 0.0f, 0.0f);
	m_cirChoc       = D3DVECTOR(0.0f, 0.0f, 0.0f);
	m_inclinaison   = D3DVECTOR(0.0f, 0.0f, 0.0f);
	m_lastParticule = 0.0f;

	m_power = 0;
	m_fret  = 0;
	m_truck = 0;
	m_truckLink  = 0;
	m_shield     = 1.0f;
	m_burnShield = 1.0f;
	m_transparency = 0.0f;
	m_bHilite = FALSE;
	m_bSelect = FALSE;
	m_bSelectable = TRUE;
	m_bVisible = TRUE;
	m_bEnable = TRUE;
	m_bGadget = FALSE;
	m_bTrainer = FALSE;
	m_bFixed = FALSE;
	m_bClip = TRUE;
	m_bGhost = FALSE;
	m_bGround = FALSE;
	m_aTime = 0.0f;
	m_shotTime = 0.0f;
	m_bLock     = FALSE;
	m_bStarting = FALSE;
	m_bExplo    = FALSE;
	m_bBurn     = FALSE;
	m_bDead     = FALSE;
	m_bFlat     = FALSE;
	m_defRank = -1;
	m_magnifyDamage = 1.0f;
	m_param = 0.0f;
	m_passCounter = 0;
	m_rankCounter = -1;

	ZeroMemory(&m_character, sizeof(Character));
	m_character.wheelFrontPos = D3DVECTOR( 1.0f, 1.0f, 1.0f);
	m_character.wheelBackPos  = D3DVECTOR(-1.0f, 1.0f, 1.0f);
	m_character.wheelFrontDim = 1.0f;
	m_character.wheelBackDim  = 1.0f;
	m_character.mass          = 1000.0f;

	m_cameraType = CAMERA_BACK;
	m_cameraDist = 50.0f;
	m_bCameraLock = FALSE;

	for ( i=0 ; i<OBJECTMAXPART ; i++ )
	{
		m_objectPart[i].bUsed = FALSE;
	}
	m_totalPart = 0;

	for ( i=0 ; i<10 ; i++ )
	{
		m_partiSel[i] = -1;
	}

	for ( i=0 ; i<OBJECTMAXCMDLINE ; i++ )
	{
		m_cmdLine[i] = NAN;
	}

	FlushCrashShere();
	FlushCrashLine();
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

	m_bRecorderRecord = FALSE;
	m_bRecorderPlay   = FALSE;
	m_recorder = 0;
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
// scène sont détruits rapidement !

void CObject::DeleteObject(BOOL bAll)
{
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

		if ( m_type == OBJECT_FACTORY1 ||
			 m_type == OBJECT_FACTORY2 ||
			 m_type == OBJECT_FACTORY3 ||
			 m_type == OBJECT_FACTORY4 ||
			 m_type == OBJECT_FACTORY5 ||
			 m_type == OBJECT_FACTORY6 ||
			 m_type == OBJECT_FACTORY7 ||
			 m_type == OBJECT_FACTORY8 ||
			 m_type == OBJECT_FACTORY9 ||
			 m_type == OBJECT_FACTORY10||
			 m_type == OBJECT_FACTORY11||
			 m_type == OBJECT_FACTORY12||
			 m_type == OBJECT_FACTORY13||
			 m_type == OBJECT_FACTORY14||
			 m_type == OBJECT_FACTORY15||
			 m_type == OBJECT_FACTORY16||
			 m_type == OBJECT_FACTORY17||
			 m_type == OBJECT_FACTORY18||
			 m_type == OBJECT_FACTORY19||
			 m_type == OBJECT_FACTORY20||
			 m_type == OBJECT_TOWER    ||
			 m_type == OBJECT_NUCLEAR  ||
			 m_type == OBJECT_PARA     ||
			 m_type == OBJECT_COMPUTER ||
			 m_type == OBJECT_REPAIR   ||
			 m_type == OBJECT_SWEET    ||
			 m_type == OBJECT_DOOR1    ||
			 m_type == OBJECT_DOOR2    ||
			 m_type == OBJECT_DOOR3    ||
			 m_type == OBJECT_DOOR4    ||
			 m_type == OBJECT_DOOR5    ||
			 m_type == OBJECT_DOCK     ||
			 m_type == OBJECT_REMOTE   ||
			 m_type == OBJECT_STAND    ||
			 m_type == OBJECT_GENERATOR||
			 m_type == OBJECT_START    ||
			 m_type == OBJECT_END      ||
			 m_type == OBJECT_SUPPORT  )  // batiment?
		{
			m_terrain->DeleteBuildingLevel(RetPosition(0));  // applanit le terrain
		}
	}

	m_type = OBJECT_NULL;  // objet invalide jusqu'à destruction complète

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
}

// Simplifie un objet (on lui ôte le cerveau, entre autres).

void CObject::Simplify()
{
	if ( m_brain != 0 )
	{
		m_brain->StopProgram();
	}

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
}


// Fait exploser un objet, lorsqu'il est touché par un projectile.
// Si FALSE est retourné, l'objet n'est pas encore foutu.
// Si TRUE est retourné, l'objet est détruit.

BOOL CObject::ExploObject(ExploType type, float force, D3DVECTOR impact)
{
	PyroType	pyroType;
	CPyro*		pyro;
	float		loss, shield;
	int			i, nb, level, part, objRank, total;

	if ( type == EXPLO_BURN )
	{
		if ( m_type == OBJECT_MOBILEtg ||
			 m_type == OBJECT_TRAX     ||
			 m_type == OBJECT_UFO      ||
			 m_type == OBJECT_METAL    ||
			 m_type == OBJECT_BARREL   ||
			 m_type == OBJECT_BARRELa  ||
			 m_type == OBJECT_ATOMIC   ||
			 m_type == OBJECT_TNT      ||
			 m_type == OBJECT_BULLET   )  // objet qui ne brûle pas ?
		{
			type = EXPLO_BOUM;
			force = 1.0f;
		}
	}

	if ( type == EXPLO_BOUM )
	{
		if ( m_shotTime < 0.5f )  return FALSE;
		m_shotTime = 0.0f;
	}

	if ( m_type == OBJECT_HUMAN && m_bDead )  return FALSE;

	// Calcule la puissance perdue par l'explosion.
	if ( force == 0.0f )
	{
		loss = LOSS_SHIELD;
	}
	else
	{
		loss = force;
	}
	loss *= m_magnifyDamage;

	// Diminue la puissance du bouclier.
	if ( m_type == OBJECT_CAR )  // voiture ?
	{
		m_motion->TwistPart(impact, force);

//?		nb = (int)(loss*60.0f);
		level = m_main->RetLevel()-1;  // 0..3
		if ( level < 1 )  level = 1;  // 1..3
		nb = (int)(loss*20.0f*level);
		if ( nb == 0 )  nb = 1;
		if ( nb >  5 )  nb = 5;
		if ( ExploPart(nb, force) )  // perd qq pièces
		{
			pyro = new CPyro(m_iMan);
			pyro->SetImpact(impact);
			pyro->Create(PT_EXPLOP, this, 1.0f);
			return FALSE;
		}
		else
		{
			return TRUE;  // voiture détruite
		}
	}

	if ( m_type == OBJECT_UFO )
	{
		total = 0;
		for ( i=0 ; i<100 ; i++ )
		{
			part = 1+rand()%18;
			objRank = RetObjectRank(part);
			if ( objRank != -1 )
			{
				ExploPiece(part);
				total ++;
				if ( total >= 6 )  break;
			}
		}
	}

	shield = RetShield();
	shield -= loss;
	if ( shield < 0.0f )  shield = 0.0f;  // mort
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
			else
			{
				pyroType = PT_SHOTT;
			}
		}
	}
	else	// complètement mort ?
	{
		if ( type == EXPLO_BURN )  // brûle ?
		{
			if ( m_type == OBJECT_BULLET )
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
			if ( m_type == OBJECT_BULLET )
			{
				pyroType = PT_FRAGO;
			}
			else if ( m_type == OBJECT_HUMAN )
			{
				pyroType = PT_DEADG;
			}
			else if ( m_type == OBJECT_FACTORY1 ||
					  m_type == OBJECT_FACTORY2 ||
					  m_type == OBJECT_FACTORY3 ||
					  m_type == OBJECT_FACTORY4 ||
					  m_type == OBJECT_FACTORY5 ||
					  m_type == OBJECT_FACTORY6 ||
					  m_type == OBJECT_FACTORY7 ||
					  m_type == OBJECT_FACTORY8 ||
					  m_type == OBJECT_FACTORY9 ||
					  m_type == OBJECT_FACTORY10||
					  m_type == OBJECT_FACTORY11||
					  m_type == OBJECT_FACTORY12||
					  m_type == OBJECT_FACTORY13||
					  m_type == OBJECT_FACTORY14||
					  m_type == OBJECT_FACTORY15||
					  m_type == OBJECT_FACTORY16||
					  m_type == OBJECT_FACTORY17||
					  m_type == OBJECT_FACTORY18||
					  m_type == OBJECT_FACTORY19||
					  m_type == OBJECT_FACTORY20||
					  m_type == OBJECT_TOWER    ||
					  m_type == OBJECT_NUCLEAR  ||
					  m_type == OBJECT_PARA     ||
					  m_type == OBJECT_COMPUTER ||
					  m_type == OBJECT_REPAIR   ||
					  m_type == OBJECT_SWEET    ||
					  m_type == OBJECT_DOOR1    ||
					  m_type == OBJECT_DOOR2    ||
					  m_type == OBJECT_DOOR3    ||
					  m_type == OBJECT_DOOR4    ||
					  m_type == OBJECT_DOOR5    ||
					  m_type == OBJECT_DOCK     ||
					  m_type == OBJECT_REMOTE   ||
					  m_type == OBJECT_STAND    ||
					  m_type == OBJECT_GENERATOR||
					  m_type == OBJECT_START    ||
					  m_type == OBJECT_END      ||
					  m_type == OBJECT_SUPPORT  )  // batiment ?
			{
				pyroType = PT_FRAGT;
			}
			else if ( m_type == OBJECT_MOBILEtg )
			{
				pyroType = PT_FRAGT;
			}
			else if ( m_type == OBJECT_MARK )
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
	}

	if ( shield > 0.0f )  return FALSE;  // pas encore mort

	if ( RetSelect() )
	{
		SetSelect(FALSE);  // désélectionne l'objet
		m_camera->SetType(CAMERA_EXPLO);
		m_main->DeselectAll();
	}
	
	if ( m_botVar != 0 )
	{
		if ( m_type == OBJECT_STONE   ||
			 m_type == OBJECT_URANIUM ||
			 m_type == OBJECT_METAL   ||
			 m_type == OBJECT_BARREL  ||
			 m_type == OBJECT_BARRELa ||
			 m_type == OBJECT_ATOMIC  ||
			 m_type == OBJECT_BULLET  ||
			 m_type == OBJECT_BBOX    ||
			 m_type == OBJECT_TNT     ||  // (*)
			(m_type >= OBJECT_ROADSIGN1 && m_type <= OBJECT_ROADSIGN30) )
		{
			m_botVar->SetUserPtr(OBJECTDELETED);
		}
	}

	return TRUE;
}

// (*)	Si un robot ou le cosmonaute meurt, l'objet doit continuer
//		d'exister, pour que les programmes des fourmis continuent
//		de fonctionner comme si de rien était !


// Crée des morceaux d'objet qui partent.
// Retourne FALSE si la voiture est détruite.

BOOL CObject::ExploPart(int total, float force)
{
	D3DVECTOR	p1, p2, p3, p4;
	CPyro*		pyro;
	float		dim;
	int			i, part, param;

	// Crée une tache d'huile au sol.
	if ( force > 0.2f &&
		 m_objectPart[0].position.y > m_water->RetLevel() )
	{
		dim = force*20.0f;
		if ( dim > 20.0f )  dim = 20.0f;
		p1 = p2 = p3 = p4 = RetPosition(0);
		p1.x -= dim;  p1.z += dim;
		p2.x += dim;  p2.z += dim;
		p3.x -= dim;  p3.z -= dim;
		p4.x += dim;  p4.z -= dim;
		m_particule->CreateWheelTrace(p1, p2, p3, p4, PARTITRACE4);
	}

	if ( total > 1 && RetSelect() )
	{
		m_camera->StartOver(OE_CRASH, RetPosition(0), force);
	}

	for ( i=0 ; i<total ; i++ )
	{
		part = m_motion->RetRemovePart(param);
		if ( part == -1 )
		{
			if ( total == 999 )  return TRUE;

			pyro = new CPyro(m_iMan);
			pyro->Create(PT_EXPLOS, this, 1.0f);

			pyro = new CPyro(m_iMan);
			pyro->Create(PT_BURNS, this, 1.0f);

			DetachPart(3);
			DetachPart(4);
			DetachPart(5);
			DetachPart(6);  // 4 roues
			DetachPart(7);  // moteur
			DetachPart(8);  // volant

			if ( m_fret != 0 )  // transporte un robot ?
			{
				pyro = new CPyro(m_iMan);
				pyro->Create(PT_EXPLOT, m_fret, 1.0f);
			}

			SetDead(TRUE);
			m_camera->SetType(CAMERA_BACK);
			return FALSE;  // voiture détruite
		}

		DetachPiece(part, param);
	}
	return TRUE;  // pas encore détruite
}

// Crée un morceau d'objet qui part.

BOOL CObject::DetachPart(int part, D3DVECTOR speed)
{
	D3DVECTOR	pos, min, max;
	FPOINT		dim, p;
	float		speedx, speedy, duration, mass;
	int			channel, objRank;

	if ( !FlatParent(part) )  return FALSE;

	pos = RetPosition(part);
	pos.y += 2.0f;

	if ( speed.x == 0.0f && speed.y == 0.0f && speed.z == 0.0f )
	{
		if ( m_physics == 0 )
		{
			speedx = 0.0f;
			speedy = 0.0f;
		}
		else
		{
			speedx = m_physics->RetLinMotionX(MO_REASPEED)/m_physics->RetLinMotionX(MO_ADVSPEED);
			speedy = m_physics->RetCirMotionY(MO_REASPEED)/m_physics->RetCirMotionY(MO_ADVSPEED);
		}

		speed.x = speedx*0.8f;
		speed.y = 10.0f+speedx*10.0f;
		speed.z = speedy*0.8f;
		p = RotatePoint(-RetAngleY(part), FPOINT(speed.x, speed.z));
		speed.x = p.x;
		speed.z = p.y;
		speed *= 2.0f;
	}

	objRank = RetObjectRank(part);
	m_engine->GetBBox(objRank, min, max);
	dim.x = Length(min, max)/2.0f;
	dim.y = dim.x;
	duration = 6.0f+Rand()*8.0f;
	mass = 12.0f+Rand()*16.0f;

	channel = m_particule->CreatePart(pos, speed, dim, PARTIPART, duration, mass, 10.0f);
	if ( channel != -1 )
	{
		SetMasterParticule(part, channel);
	}
	return TRUE;
}

// Crée un morceau d'objet qui devient une pièce indépendante.

BOOL CObject::DetachPiece(int part, int param, D3DVECTOR speed)
{
	CObject*	pObj;
	CPyro*		pyro;
	D3DVECTOR	pos, angle, min, max, dim;
	float		radius;
	int			objRank;

	if ( !FlatParent(part) )  return FALSE;
	pos = RetPosition(part);
	angle = RetAngle(0);

	objRank = RetObjectRank(part);
	m_engine->GetBBox(objRank, min, max);
	dim = max-min;

	m_objectPart[part].bUsed = FALSE;  // supprime pièce à la voiture
	UpdateTotalPart();

	pObj = new CObject(m_iMan);
	pObj->SetType(OBJECT_PIECE);
	m_engine->SetObjectType(objRank, TYPEFIX);
	pObj->SetObjectRank(0, objRank);
	pObj->SetPosition(0, pos);
	pObj->SetAngle(0, angle);

	radius = (dim.x+dim.y+dim.z)/3.0f/2.0f;
	pObj->CreateCrashSphere(D3DVECTOR(0.0f, 0.0f, 0.0f), radius, SOUND_CHOCo, 0.45f);
	pObj->CreateShadowCircle(radius*1.5f, 0.3f);

	pyro = new CPyro(m_iMan);
	pyro->Create(PT_PIECE, pObj, 1.0f, param);  // voltige

	return TRUE;
}

// Fait exploser un morceau d'objet.

BOOL CObject::ExploPiece(int part)
{
	CObject*	pObj;
	CPyro*		pyro;
	D3DVECTOR	pos, angle, min, max, dim;
	int			objRank;

	if ( !FlatParent(part) )  return FALSE;
	pos = RetPosition(part);
	angle = RetAngle(0);

	objRank = RetObjectRank(part);
	m_engine->GetBBox(objRank, min, max);
	dim = max-min;

	m_objectPart[part].bUsed = FALSE;  // supprime pièce à la voiture
	UpdateTotalPart();

	pObj = new CObject(m_iMan);
	pObj->SetType(OBJECT_PIECE);
	m_engine->SetObjectType(objRank, TYPEFIX);
	pObj->SetObjectRank(0, objRank);
	pObj->SetPosition(0, pos);
	pObj->SetAngle(0, angle);

	pyro = new CPyro(m_iMan);
	pyro->Create(PT_FRAGT, pObj);  // explosion

	return TRUE;
}


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

// Crée une nouvelle partie, et retourne son numéro.
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


// Spécifie le numéro de l'objet d'une partie.

void CObject::SetObjectRank(int part, int objRank)
{
	if ( !m_objectPart[part].bUsed )  // objet pas créé ?
	{
		InitPart(part);
		UpdateTotalPart();
	}
	m_objectPart[part].object = objRank;
}

// Retourne le numéro d'une partie.

int CObject::RetObjectRank(int part)
{
	if ( !m_objectPart[part].bUsed )  return -1;
	return m_objectPart[part].object;
}

// Spécifie quel est le parent d'ue partie.
// Rappel: la partie 0 est toujours le père de tous et donc la
// partie principale (par exemple le chassis d'une voiture).

void CObject::SetObjectParent(int part, int parent)
{
	m_objectPart[part].parentPart = parent;
}


// Spécifie le type de l'objet.

void CObject::SetType(ObjectType type)
{
	m_type = type;
}

ObjectType CObject::RetType()
{
	return m_type;
}

// Spécifie le modèle de voiture.

void CObject::SetModel(int model)
{
	m_model = model;
}

int CObject::RetModel()
{
	return m_model;
}

void CObject::SetSubModel(int subModel)
{
	m_subModel = subModel;
}

int CObject::RetSubModel()
{
	return m_subModel;
}

// Choix de l'option à utiliser.

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



// Cherche le nième fils d'un père.

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


// Supprime toutes les sphères utilisées pour les collisions.

void CObject::FlushCrashShere()
{
	m_crashSphereUsed = 0;
}

// Ajoute une nouvelle sphère.

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

// Déplace une sphère.

void CObject::MoveCrashSphere(int rank, D3DVECTOR pos, float radius)
{
	m_crashSpherePos[rank] = pos;
	m_crashSphereRadius[rank] = radius;
}

// Retourne le nombre de sphères.

int CObject::RetCrashSphereTotal()
{
	return m_crashSphereUsed;
}

// Retourne une sphère pour les collisions.
// La position est absolue dans le monde.

BOOL CObject::GetCrashSphere(int rank, D3DVECTOR &pos, float &radius)
{
	if ( rank < 0 || rank >= m_crashSphereUsed )
	{
		pos = m_objectPart[0].position;
		radius = 0.0f;
		return FALSE;
	}

	// Retourne la sphère pour les collisions, qui ne tient pas
	// compte de l'inclinaison du véhicule. Ceci est nécessaire
	// pour les collisions avec les véhicules, afin de ne pas tenir
	// compte de SetInclinaison, par exemple.
	// La sphère doit avoir obligatoirement un centre (0;y;0).
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

// Retourne la dureté d'une sphère.

Sound CObject::RetCrashSphereSound(int rank)
{
	return m_crashSphereSound[rank];
}

// Retourne la dureté d'une sphère.

float CObject::RetCrashSphereHardness(int rank)
{
	return m_crashSphereHardness[rank];
}

// Supprime une sphère.

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

// Spécifie la sphère globale, relative à l'objet.

void CObject::SetGlobalSphere(D3DVECTOR pos, float radius)
{
	float	zoom;

	zoom = RetZoomX(0);
	m_globalSpherePos    = pos;
	m_globalSphereRadius = radius*zoom;
}

// Retourne la sphère globale, dans l'univers.

void CObject::GetGlobalSphere(D3DVECTOR &pos, float &radius)
{
	pos = Transform(m_objectPart[0].matWorld, m_globalSpherePos);
	radius = m_globalSphereRadius;
}


// Spécifie la sphère de bousculade, relative à l'objet.

void CObject::SetJotlerSphere(D3DVECTOR pos, float radius)
{
	m_jotlerSpherePos    = pos;
	m_jotlerSphereRadius = radius;
}

// Retourne la sphère de bousculade, dans l'univers.

void CObject::GetJotlerSphere(D3DVECTOR &pos, float &radius)
{
	pos = Transform(m_objectPart[0].matWorld, m_jotlerSpherePos);
	radius = m_jotlerSphereRadius;
}


// Supprime toutes les lignes utilisées pour les collisions.

void CObject::FlushCrashLine()
{
	m_crashLineUsed = 0;
	m_crashLineMin = FPOINT( 10000.0f,  10000.0f);
	m_crashLineMax = FPOINT(-10000.0f, -10000.0f);
	m_crashLineHeight = NAN;
}

// Spécifie la hauteur de l'objet (à partir du sol).

void CObject::SetCrashLineHeight(float h)
{
	m_crashLineHeight = h;
}

// Retourne la hauteur de l'objet.

float CObject::RetCrashLineHeight()
{
	return m_crashLineHeight;
}

// Ajoute une nouvelle ligne.

int CObject::CreateCrashLine(FPOINT pos, Sound sound, float hardness, BOOL bNew)
{
#if 0
	D3DVECTOR	p;
	float		zoom;

	if ( m_crashLineUsed >= MAXCRASHLINE )  return -1;

	zoom = RetZoomX(0);
	m_crashLinePos[m_crashLineUsed] = pos;
	m_crashLineNew[m_crashLineUsed] = bNew;
	m_crashLineHardness[m_crashLineUsed] = hardness;
	m_crashLineSound[m_crashLineUsed] = sound;

	if ( m_objectPart[0].bTranslate ||
		 m_objectPart[0].bRotate    )
	{
		UpdateTransformObject();
	}
	p.x = pos.x;
	p.z = pos.y;
	p.y = m_objectPart[0].position.y;
	p = Transform(m_objectPart[0].matWorld, p);
	if ( p.x < m_crashLineMin.x )  m_crashLineMin.x = p.x;
	if ( p.z < m_crashLineMin.y )  m_crashLineMin.y = p.z;
	if ( p.x > m_crashLineMax.x )  m_crashLineMax.x = p.x;
	if ( p.z > m_crashLineMax.y )  m_crashLineMax.y = p.z;

	return m_crashLineUsed++;
#else
	FPOINT		p;
	float		zoom;

	if ( m_crashLineUsed >= MAXCRASHLINE )  return -1;

	zoom = RetZoomX(0);
	m_crashLinePos[m_crashLineUsed] = pos;
	m_crashLineNew[m_crashLineUsed] = bNew;
	m_crashLineHardness[m_crashLineUsed] = hardness;
	m_crashLineSound[m_crashLineUsed] = sound;

	p = RotatePoint(-m_objectPart[0].angle.y, pos);
	p.x += m_objectPart[0].position.x;
	p.y += m_objectPart[0].position.z;
	if ( p.x < m_crashLineMin.x )  m_crashLineMin.x = p.x;
	if ( p.y < m_crashLineMin.y )  m_crashLineMin.y = p.y;
	if ( p.x > m_crashLineMax.x )  m_crashLineMax.x = p.x;
	if ( p.y > m_crashLineMax.y )  m_crashLineMax.y = p.y;

	return m_crashLineUsed++;
#endif
}

// Met à jour la bbox.

void CObject::UpdateBBoxCrashLine()
{
#if 0
	D3DVECTOR	p;
	int			i;

	if ( m_objectPart[0].bTranslate ||
		 m_objectPart[0].bRotate    )
	{
		UpdateTransformObject();
	}

	m_crashLineMin = FPOINT( 10000.0f,  10000.0f);
	m_crashLineMax = FPOINT(-10000.0f, -10000.0f);

	for ( i=0 ; i<m_crashLineUsed ; i++ )
	{
		p.x = m_crashLinePos[i].x;
		p.z = m_crashLinePos[i].y;
		p.y = m_objectPart[0].position.y;
		p = Transform(m_objectPart[0].matWorld, p);
		if ( p.x < m_crashLineMin.x )  m_crashLineMin.x = p.x;
		if ( p.z < m_crashLineMin.y )  m_crashLineMin.y = p.z;
		if ( p.x > m_crashLineMax.x )  m_crashLineMax.x = p.x;
		if ( p.z > m_crashLineMax.y )  m_crashLineMax.y = p.z;
	}
#else
	FPOINT		p;
	int			i;

	if ( m_objectPart[0].bTranslate ||
		 m_objectPart[0].bRotate    )
	{
		UpdateTransformObject();
	}

	m_crashLineMin = FPOINT( 10000.0f,  10000.0f);
	m_crashLineMax = FPOINT(-10000.0f, -10000.0f);

	for ( i=0 ; i<m_crashLineUsed ; i++ )
	{
		p = RotatePoint(-m_objectPart[0].angle.y, m_crashLinePos[i]);
		p.x += m_objectPart[0].position.x;
		p.y += m_objectPart[0].position.z;
		if ( p.x < m_crashLineMin.x )  m_crashLineMin.x = p.x;
		if ( p.y < m_crashLineMin.y )  m_crashLineMin.y = p.y;
		if ( p.x > m_crashLineMax.x )  m_crashLineMax.x = p.x;
		if ( p.y > m_crashLineMax.y )  m_crashLineMax.y = p.y;
	}
#endif
}

// Retourne le nombre de lignes.

int CObject::RetCrashLineTotal()
{
	return m_crashLineUsed;
}

// Retourne une ligne pour les collisions.
// La position est absolue dans le monde.

BOOL CObject::GetCrashLine(int rank, FPOINT &pos, BOOL &bNew)
{
#if 0
	D3DVECTOR	p;

	if ( rank >= m_crashLineUsed )  return FALSE;

	if ( m_objectPart[0].bTranslate ||
		 m_objectPart[0].bRotate    )
	{
		UpdateTransformObject();
	}
	p.x = m_crashLinePos[rank].x;
	p.z = m_crashLinePos[rank].y;
	p.y = 0.0f;
	p = Transform(m_objectPart[0].matWorld, p);
	pos.x = p.x;
	pos.y = p.z;

	bNew = m_crashLineNew[rank];
	return TRUE;
#else
	if ( rank >= m_crashLineUsed )  return FALSE;

	pos = RotatePoint(-m_objectPart[0].angle.y, m_crashLinePos[rank]);
	pos.x += m_objectPart[0].position.x;
	pos.y += m_objectPart[0].position.z;

	bNew = m_crashLineNew[rank];
	return TRUE;
#endif
}

// Retourne la dureté d'une ligne.

Sound CObject::RetCrashLineSound(int rank)
{
	return m_crashLineSound[rank];
}

// Retourne la dureté d'une ligne.

float CObject::RetCrashLineHardness(int rank)
{
	return m_crashLineHardness[rank];
}

// Retourne la bbox de toutes les lignes.

void CObject::RetCrashLineBBox(FPOINT &min, FPOINT &max)
{
	min = m_crashLineMin;
	max = m_crashLineMax;
}

// Indique s'il s'agit d'un objet fusionnable.
// L'objet doit obligatoirement être indestructible !

BOOL CObject::IsCrashLineFusion()
{
	return ( m_type == OBJECT_BARRIER6  ||
			 m_type == OBJECT_BARRIER7  ||
			 m_type == OBJECT_BARRIER8  ||
			 m_type == OBJECT_BARRIER9  ||
			 m_type == OBJECT_BARRIER10 ||
			 m_type == OBJECT_BARRIER11 ||
			 m_type == OBJECT_BARRIER12 ||
			 m_type == OBJECT_BARRIER14 ||
			 m_type == OBJECT_BARRIER15 ||
			 m_type == OBJECT_BARRIER16 );
}

// Essaye de fusionner les lignes de collision de l'objet courant
// avec un autre. Ainsi, plusieurs barrières mises bout à bout ne
// formeront plus qu'un obstacle, regroupé dans la première barrière
// crée. L'objet courant n'a alors plus aucune ligne de collision.

void CObject::CrashLineFusion()
{
	CObject*	pObj;
	D3DVECTOR	iCenter, oCenter;
	FPOINT		iPos[4], oPos[4];
	float		iLen, oLen, iWidth, oWidth, h;
	int			i, j;
	BOOL		bNew;

	if ( !IsCrashLineFusion() )  return;
	if ( RetCrashLineTotal() != 5 )  return;

	iLen = Length(m_crashLinePos[0], m_crashLinePos[1]);
	iWidth = Length(m_crashLinePos[1], m_crashLinePos[2]);

	for ( j=0 ; j<4 ; j++ )
	{
		if ( !GetCrashLine(j, iPos[j], bNew) )  return;
	}

	for ( i=0 ; i<1000000 ; i++ )
	{
		pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
		if ( pObj == 0 )  break;

		if ( !pObj->IsCrashLineFusion() )  continue;
		if ( pObj->RetCrashLineTotal() != 5 )  continue;
		if ( pObj->RetAngleY(0) != RetAngleY(0) )  continue;

		oLen = Length(pObj->m_crashLinePos[0], pObj->m_crashLinePos[1]);
		oWidth = Length(pObj->m_crashLinePos[1], pObj->m_crashLinePos[2]);

		if ( iWidth != oWidth )  continue;

		for ( j=0 ; j<4 ; j++ )
		{
			if ( !pObj->GetCrashLine(j, oPos[j], bNew) )  continue;
		}

		if ( Abs(iPos[1].x-oPos[0].x) < 5.0f &&
			 Abs(iPos[1].y-oPos[0].y) < 5.0f &&
			 Abs(iPos[2].x-oPos[3].x) < 5.0f &&
			 Abs(iPos[2].y-oPos[3].y) < 5.0f )
		{
			pObj->m_crashLinePos[0].x -= iLen;
			pObj->m_crashLinePos[3].x -= iLen;
			pObj->m_crashLinePos[4].x -= iLen;
			pObj->UpdateBBoxCrashLine();
			if ( pObj->RetCrashLineHeight() != NAN )
			{
				iCenter = RetPosition(0);
				oCenter = pObj->RetPosition(0);
				h = iCenter.y-oCenter.y;
				if ( h < 0.0f)  h = 0.0f;
				pObj->SetCrashLineHeight(pObj->RetCrashLineHeight()+h);
			}
			FlushCrashLine();
//?CreateResource(RetPosition(0), 0.0f, 1.0f, OBJECT_WAYPOINT, FALSE);
			return;
		}

		if ( Abs(iPos[0].x-oPos[1].x) < 5.0f &&
			 Abs(iPos[0].y-oPos[1].y) < 5.0f &&
			 Abs(iPos[3].x-oPos[2].x) < 5.0f &&
			 Abs(iPos[3].y-oPos[2].y) < 5.0f )
		{
			pObj->m_crashLinePos[1].x += iLen;
			pObj->m_crashLinePos[2].x += iLen;
			pObj->UpdateBBoxCrashLine();
			if ( pObj->RetCrashLineHeight() != NAN )
			{
				iCenter = RetPosition(0);
				oCenter = pObj->RetPosition(0);
				h = iCenter.y-oCenter.y;
				if ( h < 0.0f)  h = 0.0f;
				pObj->SetCrashLineHeight(pObj->RetCrashLineHeight()+h);
			}
			FlushCrashLine();
//?CreateResource(RetPosition(0), 0.0f, 1.0f, OBJECT_WAYPOINT, FALSE);
			return;
		}
	}
}

// Indique si la caméra est obstruée par un objet.

BOOL CObject::IsOccludeCamera(const D3DVECTOR &eye, const D3DVECTOR &look)
{
	FPOINT	oP1,oP2,oP3,oP4, e,l, inter;
	BOOL	bNew;
	int		u;

	if ( m_type != OBJECT_BARRIER14 &&
		 m_type != OBJECT_BARRIER15 &&
		 m_type != OBJECT_BARRIER16 )  return TRUE;

	e.x = eye.x;
	e.y = eye.z;
	l.x = look.x;
	l.y = look.z;

	u = m_crashLineUsed;
	m_crashLineUsed = 5;  // à cause de CrashLineFusion
	GetCrashLine(0, oP1, bNew);
	GetCrashLine(1, oP2, bNew);
	GetCrashLine(2, oP3, bNew);
	GetCrashLine(3, oP4, bNew);
	m_crashLineUsed = u;

	if ( IntersectSegment(e,l, oP1,oP2, inter) )  return TRUE;
	if ( IntersectSegment(e,l, oP3,oP4, inter) )  return TRUE;

	return FALSE;
}


// Positionne un objet à une certaine hauteur par-rapport au sol.

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

// Ajuste l'inclinaison d'un objet posé sur le sol.

void CObject::FloorAdjust()
{
	D3DVECTOR		pos, n;
	FPOINT			nn;
	float			a;

	pos = RetPosition(0);
	if ( m_terrain->GetNormal(n, pos) )
	{
		a = RetAngleY(0);
		nn = RotatePoint(-a, FPOINT(n.z, n.x));
		SetAngleX(0,  sinf(nn.x));
		SetAngleZ(0, -sinf(nn.y));
	}
}


// Donne la vibration linéaire.

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

// Donne la rotation circulaire suite à un choc.

void CObject::SetCirChoc(D3DVECTOR dir)
{
	if ( m_cirChoc.x != dir.x ||
		 m_cirChoc.y != dir.y ||
		 m_cirChoc.z != dir.z )
	{
		m_cirChoc = dir;
		m_objectPart[0].bRotate = TRUE;
	}
}

D3DVECTOR CObject::RetCirChoc()
{
	return m_cirChoc;
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

	if ( part == 0 && m_physics != 0 )
	{
		m_physics->UpdateCorner();
	}

	if ( part == 0 && !m_bFlat )  // partie principale ?
	{
		rank = m_objectPart[0].object;

		shPos = pos;
		m_terrain->MoveOnFloor(shPos, TRUE);
		m_engine->SetObjectShadowPos(rank, shPos);

		height = 0.0f;
		m_engine->SetObjectShadowHeight(rank, height);

		// Calcul la normale au terrain en 9 points stratégiques,
		// puis effectue une moyenne pondérée (les points au centre
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
	}
}

D3DVECTOR CObject::RetPosition(int part)
{
	return m_objectPart[part].position;
}

// Donne la rotation autour des 3 axes.

void CObject::SetAngle(int part, const D3DVECTOR &angle)
{
	float	a;

	m_objectPart[part].angle = angle;
	m_objectPart[part].bRotate = TRUE;  // il faudra recalculer les matrices

	if ( part == 0 && !m_bFlat )  // partie principale ?
	{
		a = m_objectPart[0].angle.y+m_cirVibration.y+m_cirChoc.y+m_inclinaison.y;
		m_engine->SetObjectShadowAngle(m_objectPart[0].object, a);
	}
}

D3DVECTOR CObject::RetAngle(int part)
{
	return m_objectPart[part].angle;
}

// Donne la rotation autour de l'axe Y.

void CObject::SetAngleY(int part, float angle)
{
	float	a;

	m_objectPart[part].angle.y = angle;
	m_objectPart[part].bRotate = TRUE;  // il faudra recalculer les matrices

	if ( part == 0 && !m_bFlat )  // partie principale ?
	{
		a = m_objectPart[0].angle.y+m_cirVibration.y+m_cirChoc.y+m_inclinaison.y;
		m_engine->SetObjectShadowAngle(m_objectPart[0].object, a);
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

	if ( m_bTrainer )  // entraînement ?
	{
		m_cameraType = CAMERA_FIX;
	}
}

BOOL CObject::RetTrainer()
{
	return m_bTrainer;
}


// Gestion de la particule maîtresse.

void CObject::SetMasterParticule(int part, int parti)
{
	m_objectPart[part].masterParti = parti;
}

int CObject::RetMasterParticule(int part)
{
	return m_objectPart[part].masterParti;
}


// Gestion de la pile transportée.

void CObject::SetPower(CObject* power)
{
	m_power = power;
}

CObject* CObject::RetPower()
{
	return m_power;
}

// Gestion de l'objet transporté.

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

	// Ombre invisible si l'objet est transporté.
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


// Indique si l'objet doit être dessiné par dessous l'interface.

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

// Indique si l'objet doit être dessiné par dessus l'interface.

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


// Crée un véhicule roulant quelconque posé sur le sol.

BOOL CObject::CreateVehicle(D3DVECTOR pos, float angle, float zoom,
							ObjectType type, int model, int subModel,
							BOOL bPlumb, BOOL bTrainer)
{
	char	actualTex[20];
	char	futureTex[20];

	m_type = type;
	m_model = model;
	m_subModel = subModel;

	if ( m_subModel == 1 )
	{
		sprintf(actualTex, "car%.2d.tga", m_model);
		sprintf(futureTex, "car%.2db.tga", m_model);
		m_engine->SetReplaceTex(actualTex, futureTex);
	}
	if ( m_subModel == 2 )
	{
		sprintf(actualTex, "car%.2d.tga", m_model);
		sprintf(futureTex, "car%.2dc.tga", m_model);
		m_engine->SetReplaceTex(actualTex, futureTex);
	}
	if ( m_subModel == 3 )
	{
		sprintf(actualTex, "car%.2d.tga", m_model);
		sprintf(futureTex, "car%.2dd.tga", m_model);
		m_engine->SetReplaceTex(actualTex, futureTex);
	}

	SetTrainer(bTrainer);

	m_physics = new CPhysics(m_iMan, this);
	m_brain   = new CBrain(m_iMan, this);

	m_physics->SetBrain(m_brain);
	m_brain->SetPhysics(m_physics);

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
	if ( !m_motion->Create(pos, angle, type, TRUE) )
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

// Crée l'ombre sous un véhicule sous forme d'une lumière
// négative.

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

	// N'éclaire que les objets du terrain.
	m_light->SetLightIncluType(m_shadowLight, TYPETERRAIN);

	return TRUE;
}

// Retourne le numéro de la lumière d'ombre négative.

int CObject::RetShadowLight()
{
	return m_shadowLight;
}

// Crée la lumière pour les effects d'un véhicule.

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

// Retourne le numéro de la lumière des effets.

int CObject::RetEffectLight()
{
	return m_effectLight;
}

// Crée l'ombre circulaire sous un véhicule.

BOOL CObject::CreateShadowCircle(float radius, float intensity,
								 D3DShadowType type)
{
	float	zoom;

	zoom = RetZoomX(0);

	m_engine->ShadowCreate(m_objectPart[0].object);

	m_engine->SetObjectShadowRadius(m_objectPart[0].object, radius*zoom);
	m_engine->SetObjectShadowIntensity(m_objectPart[0].object, intensity);
	m_engine->SetObjectShadowHeight(m_objectPart[0].object, 0.0f);
	m_engine->SetObjectShadowAngle(m_objectPart[0].object, m_objectPart[0].angle.y);
	m_engine->SetObjectShadowType(m_objectPart[0].object, type);

	return TRUE;
}

// Crée un batiment quelconque posé sur le sol.

BOOL CObject::CreateBuilding(D3DVECTOR pos, float angle, float zoom,
							 float height, ObjectType type, BOOL bPlumb)
{
	CModFile*	pModFile;
	FPOINT		p;
	int			rank;

	if ( m_engine->RetRestCreate() < 20 )  return FALSE;

	pModFile = new CModFile(m_iMan);

	SetType(type);

	rank = m_engine->CreateObject();
	m_engine->SetObjectType(rank, TYPEFIX);  // c'est un objet fixe
	SetObjectRank(0, rank);

	if ( m_type == OBJECT_FACTORY1 )  // 2 tuyaux verticaux ?
	{
		pModFile->ReadModel("objects\\factory1.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);

#if 0
		CreateCrashSphere(D3DVECTOR(  0.0f,  0.0f,   0.0f), 40.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 30.0f,  5.0f,  10.0f),  9.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 30.0f,  5.0f,  30.0f),  9.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 35.0f,  5.0f, -10.0f),  5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 35.0f,  5.0f, -20.0f),  5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 35.0f,  5.0f, -30.0f),  5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 25.0f,  5.0f, -30.0f),  5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 15.0f,  5.0f, -35.0f),  5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-20.0f,  5.0f, -35.0f),  5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-30.0f,  5.0f, -35.0f),  5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 15.0f,  5.0f,  35.0f),  5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-20.0f,  5.0f,  35.0f),  5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-30.0f,  5.0f,  35.0f),  5.0f, SOUND_BOUMm, 0.45f);
#else
		CreateCrashLine(FPOINT(-35.0f, -40.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 40.0f, -40.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 40.0f,  40.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-35.0f,  40.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-35.0f, -40.0f), SOUND_BOUMm, 0.45f);
#endif

		SetGlobalSphere(D3DVECTOR(0.0f, 10.0f, 0.0f), 40.0f);
	}

	if ( m_type == OBJECT_FACTORY2 )  // usine avec 2 cheminées ?
	{
		pModFile->ReadModel("objects\\factory2.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);

#if 0
		CreateCrashSphere(D3DVECTOR(  0.0f,  0.0f,   0.0f), 40.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 26.0f,  3.0f,  26.0f), 14.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 26.0f,  3.0f, -26.0f), 14.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-26.0f,  3.0f,  26.0f), 14.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-26.0f,  3.0f, -26.0f), 14.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 36.0f,  3.0f,  36.0f),  4.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 36.0f,  3.0f, -36.0f),  4.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-36.0f,  3.0f,  36.0f),  4.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-36.0f,  3.0f, -36.0f),  4.0f, SOUND_BOUMm, 0.45f);
#else
		CreateCrashLine(FPOINT(-40.0f, -40.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 40.0f, -40.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 40.0f,  40.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-40.0f,  40.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-40.0f, -40.0f), SOUND_BOUMm, 0.45f);
#endif

		SetGlobalSphere(D3DVECTOR(0.0f, 10.0f, 0.0f), 40.0f);
	}

	if ( m_type == OBJECT_FACTORY3 )  // usine avec toît /|/| ?
	{
		pModFile->ReadModel("objects\\factory3.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);

#if 0
		CreateCrashSphere(D3DVECTOR(  0.0f,  0.0f,   0.0f), 40.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 26.0f,  3.0f,  26.0f), 14.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 26.0f,  3.0f, -26.0f), 14.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-26.0f,  3.0f,  26.0f), 14.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-26.0f,  3.0f, -26.0f), 14.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 36.0f,  3.0f,  36.0f),  4.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 36.0f,  3.0f, -36.0f),  4.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-36.0f,  3.0f,  36.0f),  4.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-36.0f,  3.0f, -36.0f),  4.0f, SOUND_BOUMm, 0.45f);
#else
		CreateCrashLine(FPOINT(-40.0f, -40.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 40.0f, -40.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 40.0f,  40.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-40.0f,  40.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-40.0f, -40.0f), SOUND_BOUMm, 0.45f);
#endif

		SetGlobalSphere(D3DVECTOR(0.0f, 10.0f, 0.0f), 40.0f);
	}

	if ( m_type == OBJECT_FACTORY4 )  // ailettes refroidisseur ?
	{
		pModFile->ReadModel("objects\\factory4.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);

#if 0
		CreateCrashSphere(D3DVECTOR(  0.0f,  0.0f,   0.0f), 40.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 15.0f,  3.0f,  37.0f),  3.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 37.0f,  3.0f,  15.0f),  3.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 15.0f,  3.0f, -37.0f),  3.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 37.0f,  3.0f, -15.0f),  3.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-15.0f,  3.0f,  37.0f),  3.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-37.0f,  3.0f,  15.0f),  3.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-15.0f,  3.0f, -37.0f),  3.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-37.0f,  3.0f, -15.0f),  3.0f, SOUND_BOUMm, 0.45f);
#else
		CreateCrashLine(FPOINT(-17.0f, -40.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 17.0f, -40.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 40.0f, -17.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 40.0f,  17.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 17.0f,  40.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-17.0f,  40.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-40.0f,  17.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-40.0f, -17.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-17.0f, -40.0f), SOUND_BOUMm, 0.45f);
#endif

		SetGlobalSphere(D3DVECTOR(0.0f, 10.0f, 0.0f), 40.0f);
	}

	if ( m_type == OBJECT_FACTORY5 )  // ?
	{
		pModFile->ReadModel("objects\\factory5.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);

#if 0
		CreateCrashSphere(D3DVECTOR(  0.0f,  0.0f,   0.0f), 40.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 35.0f,  3.0f,  15.0f),  5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 35.0f,  3.0f,  25.0f),  5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 15.0f,  3.0f,  35.0f),  5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 25.0f,  3.0f,  35.0f),  5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 35.0f,  3.0f, -15.0f),  5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 35.0f,  3.0f, -25.0f),  5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 15.0f,  3.0f, -35.0f),  5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 25.0f,  3.0f, -35.0f),  5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-35.0f,  3.0f,  15.0f),  5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-35.0f,  3.0f,  25.0f),  5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-15.0f,  3.0f,  35.0f),  5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-25.0f,  3.0f,  35.0f),  5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-35.0f,  3.0f, -15.0f),  5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-35.0f,  3.0f, -25.0f),  5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-15.0f,  3.0f, -35.0f),  5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-25.0f,  3.0f, -35.0f),  5.0f, SOUND_BOUMm, 0.45f);
#else
		CreateCrashLine(FPOINT(-30.0f, -40.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 30.0f, -40.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 40.0f, -30.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 40.0f,  30.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 30.0f,  40.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-30.0f,  40.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-40.0f,  30.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-40.0f, -30.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-30.0f, -40.0f), SOUND_BOUMm, 0.45f);
#endif

		SetGlobalSphere(D3DVECTOR(0.0f, 10.0f, 0.0f), 40.0f);
	}

	if ( m_type == OBJECT_FACTORY6 )  // 2 tuyaux haut ?
	{
		pModFile->ReadModel("objects\\factory6.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);
	}

	if ( m_type == OBJECT_FACTORY7 )  // pont de chargement haut ?
	{
		pModFile->ReadModel("objects\\factory7.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);
	}

	if ( m_type == OBJECT_FACTORY8 )  // départ tuyau 6 ?
	{
		pModFile->ReadModel("objects\\factory8.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);

#if 0
		CreateCrashSphere(D3DVECTOR(  0.0f,  0.0f,   0.0f), 40.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 26.0f,  3.0f,  26.0f), 14.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 26.0f,  3.0f, -26.0f), 14.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-26.0f,  3.0f,  26.0f), 14.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-26.0f,  3.0f, -26.0f), 14.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 36.0f,  3.0f,  36.0f),  4.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 36.0f,  3.0f, -36.0f),  4.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-36.0f,  3.0f,  36.0f),  4.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-36.0f,  3.0f, -36.0f),  4.0f, SOUND_BOUMm, 0.45f);
#else
		CreateCrashLine(FPOINT(-30.0f, -40.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 30.0f, -40.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 40.0f, -30.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 40.0f,  30.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 30.0f,  40.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-30.0f,  40.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-40.0f,  30.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-40.0f, -30.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-30.0f, -40.0f), SOUND_BOUMm, 0.45f);
#endif

		SetGlobalSphere(D3DVECTOR(0.0f, 10.0f, 0.0f), 40.0f);
	}

	if ( m_type == OBJECT_FACTORY9 )  // cilo rond ?
	{
		pModFile->ReadModel("objects\\factory9.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);

		CreateCrashSphere(D3DVECTOR( 19.0f, 3.0f,   5.0f), 1.5f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 19.0f, 3.0f,  -5.0f), 1.5f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-19.0f, 3.0f,   5.0f), 1.5f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-19.0f, 3.0f,  -5.0f), 1.5f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(  5.0f, 3.0f,  19.0f), 1.5f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( -5.0f, 3.0f,  19.0f), 1.5f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(  5.0f, 3.0f, -19.0f), 1.5f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( -5.0f, 3.0f, -19.0f), 1.5f, SOUND_BOUMm, 0.45f);

		CreateShadowCircle(20.0f, 0.5f);
	}

	if ( m_type == OBJECT_FACTORY10 )  // tanker A2 ?
	{
		pModFile->ReadModel("objects\\factory10.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);

		CreateCrashSphere(D3DVECTOR(0.0f, 3.0f, 0.0f), 40.0f, SOUND_BOUMm, 0.45f);
		SetGlobalSphere(D3DVECTOR(0.0f, 10.0f, 0.0f), 40.0f);
	}

	if ( m_type == OBJECT_FACTORY11 )  // tanker C5 ?
	{
		pModFile->ReadModel("objects\\factory11.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);

		CreateCrashSphere(D3DVECTOR(0.0f, 3.0f, 0.0f), 40.0f, SOUND_BOUMm, 0.45f);
		SetGlobalSphere(D3DVECTOR(0.0f, 10.0f, 0.0f), 40.0f);
	}

	if ( m_type == OBJECT_FACTORY12 )  // tanker F4 ?
	{
		pModFile->ReadModel("objects\\factory12.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);

		CreateCrashSphere(D3DVECTOR(0.0f, 3.0f, 0.0f), 40.0f, SOUND_BOUMm, 0.45f);
		SetGlobalSphere(D3DVECTOR(0.0f, 10.0f, 0.0f), 40.0f);
	}

	if ( m_type == OBJECT_FACTORY13 )  // cilo carré ?
	{
		pModFile->ReadModel("objects\\factory13.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);

		CreateCrashSphere(D3DVECTOR( 9.0f, 3.0f,   29.5f), 1.5f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 9.0f, 3.0f,  -29.5f), 1.5f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-9.0f, 3.0f,   29.5f), 1.5f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-9.0f, 3.0f,  -29.5f), 1.5f, SOUND_BOUMm, 0.45f);

		CreateShadowCircle(20.0f, 0.5f);
	}

	if ( m_type == OBJECT_FACTORY14 )  // tapis roulant ?
	{
		pModFile->ReadModel("objects\\factory14.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);

		CreateCrashLine(FPOINT(-18.0f, -4.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(  5.0f, -4.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(  5.0f,  4.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-18.0f,  4.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-18.0f, -4.0f), SOUND_BOUMm, 0.45f);

//?		CreateShadowCircle(18.0f, 1.0f, D3DSHADOWBARRIER2);
	}

	if ( m_type == OBJECT_GRAVEL )  // tas de gravier ?
	{
		pModFile->ReadModel("objects\\gravel.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetZoom(0, zoom);
		SetFloorHeight(0.0f);

//?		CreateCrashSphere(D3DVECTOR(0.0f, 3.0f, 0.0f), 15.0f, SOUND_BOUMv, 0.15f);
		m_terrain->AddBuildingLevel(pos, 6.0f*zoom, 14.0f*zoom, 10.0f*zoom, 0.5f);
	}

	if ( m_type == OBJECT_TUB )  // bac de gravier ?
	{
		pModFile->ReadModel("objects\\tub.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetZoom(0, zoom);
		SetFloorHeight(0.0f);

		CreateCrashLine(FPOINT(-20.0f, -20.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 20.0f, -20.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 20.0f,  20.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-20.0f,  20.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-20.0f, -20.0f), SOUND_BOUMm, 0.45f);
	}

	if ( m_type == OBJECT_ALIEN1 )  // usine ?
	{
		pModFile->ReadModel("objects\\alien1.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);

		CreateCrashLine(FPOINT(-40.0f, -40.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 40.0f, -40.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 40.0f,  40.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-40.0f,  40.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-40.0f, -40.0f), SOUND_BOUMm, 0.45f);

		SetGlobalSphere(D3DVECTOR(0.0f, 10.0f, 0.0f), 40.0f);
	}

	if ( m_type == OBJECT_ALIEN2 )  // tour ?
	{
		pModFile->ReadModel("objects\\alien2.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(1, rank);
		SetObjectParent(1, 0);
		pModFile->ReadModel("objects\\alien21.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(1, D3DVECTOR(0.0f, 40.0f, 0.0f));

		CreateCrashLine(FPOINT(-40.0f, -40.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 40.0f, -40.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 40.0f,  40.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-40.0f,  40.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-40.0f, -40.0f), SOUND_BOUMm, 0.45f);

		SetGlobalSphere(D3DVECTOR(0.0f, 10.0f, 0.0f), 40.0f);
	}

	if ( m_type == OBJECT_ALIEN3 )  // tour haute ?
	{
		pModFile->ReadModel("objects\\alien3.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(1, rank);
		SetObjectParent(1, 0);
		pModFile->ReadModel("objects\\alien21.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(1, D3DVECTOR(0.0f, 50.0f, 0.0f));

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(2, rank);
		SetObjectParent(2, 0);
		pModFile->ReadModel("objects\\alien21.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(2, D3DVECTOR(0.0f, 80.0f, 0.0f));

		CreateCrashLine(FPOINT(-40.0f, -40.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 40.0f, -40.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 40.0f,  40.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-40.0f,  40.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-40.0f, -40.0f), SOUND_BOUMm, 0.45f);

		SetGlobalSphere(D3DVECTOR(0.0f, 10.0f, 0.0f), 40.0f);
	}

	if ( m_type == OBJECT_ALIEN4 )  // habitation ?
	{
		pModFile->ReadModel("objects\\alien4.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);

		CreateCrashLine(FPOINT(-40.0f, -40.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 40.0f, -40.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 40.0f,  40.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-40.0f,  40.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-40.0f, -40.0f), SOUND_BOUMm, 0.45f);

		SetGlobalSphere(D3DVECTOR(0.0f, 10.0f, 0.0f), 40.0f);
	}

	if ( m_type == OBJECT_ALIEN5 )  // marteau ?
	{
		pModFile->ReadModel("objects\\alien5.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(1, rank);
		SetObjectParent(1, 0);
		pModFile->ReadModel("objects\\alien51.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(1, D3DVECTOR(0.0f, 25.0f, 17.0f));

		CreateCrashLine(FPOINT(-40.0f, -40.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 40.0f, -40.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 40.0f,  40.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-40.0f,  40.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-40.0f, -40.0f), SOUND_BOUMm, 0.45f);

		SetGlobalSphere(D3DVECTOR(0.0f, 10.0f, 0.0f), 40.0f);
	}

	if ( m_type == OBJECT_ALIEN6 )  // écraseur ?
	{
		pModFile->ReadModel("objects\\alien6.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(1, rank);
		SetObjectParent(1, 0);
		pModFile->ReadModel("objects\\alien61.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(1, D3DVECTOR(-26.0f, 13.0f, 0.0f));

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(2, rank);
		SetObjectParent(2, 0);
		pModFile->ReadModel("objects\\alien62.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(2, D3DVECTOR(26.0f, 13.0f, 0.0f));

		CreateCrashLine(FPOINT(-40.0f, -40.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 40.0f, -40.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 40.0f,  40.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-40.0f,  40.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-40.0f, -40.0f), SOUND_BOUMm, 0.45f);

		SetGlobalSphere(D3DVECTOR(0.0f, 10.0f, 0.0f), 40.0f);
	}

	if ( m_type == OBJECT_ALIEN7 )  // électrocuteur ?
	{
		pModFile->ReadModel("objects\\alien7.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);

		CreateCrashLine(FPOINT(-40.0f, -40.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 40.0f, -40.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 40.0f,  40.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-40.0f,  40.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-40.0f, -40.0f), SOUND_BOUMm, 0.45f);

		SetGlobalSphere(D3DVECTOR(0.0f, 10.0f, 0.0f), 40.0f);
	}

	if ( m_type == OBJECT_ALIEN8 )  // générateur ?
	{
		pModFile->ReadModel("objects\\alien8.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);

		rank = m_engine->CreateObject();  // bras
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(1, rank);
		SetObjectParent(1, 0);
		pModFile->ReadModel("objects\\alien81.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(1, D3DVECTOR(25.0f, 13.0f, 15.0f));
		SetAngleY(1, 50.0f*PI/180.0f);

		rank = m_engine->CreateObject();  // pince gauche
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(2, rank);
		SetObjectParent(2, 1);
		pModFile->ReadModel("objects\\alien82.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(2, D3DVECTOR(-3.0f, 0.0f, -47.0f));
		SetAngleY(2, 45.0f*PI/180.0f);

		rank = m_engine->CreateObject();  // pince droite
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(3, rank);
		SetObjectParent(3, 1);
		pModFile->ReadModel("objects\\alien83.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(3, D3DVECTOR(3.0f, 0.0f, -47.0f));
		SetAngleY(3, -45.0f*PI/180.0f);

		rank = m_engine->CreateObject();  // bouton rouge
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(4, rank);
		SetObjectParent(4, 0);
		pModFile->ReadModel("objects\\alien84.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(4, D3DVECTOR(-14.0f, 5.0f, 40.0f));

		// bouton rouge :
		CreateCrashSphere(D3DVECTOR(-14.0f, 3.0f, 42.0f), 3.0f, SOUND_BOUMm, 0.44f);

		CreateCrashLine(FPOINT(-40.0f, -40.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 40.0f, -40.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 40.0f,  40.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-40.0f,  40.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-40.0f, -40.0f), SOUND_BOUMm, 0.45f);

		SetGlobalSphere(D3DVECTOR(0.0f, 10.0f, 0.0f), 40.0f);
	}

	if ( m_type == OBJECT_ALIEN9 )  // torture ?
	{
		pModFile->ReadModel("objects\\alien9.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(1, rank);
		SetObjectParent(1, 0);
		pModFile->ReadModel("objects\\alien91.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(1, D3DVECTOR(0.0f, 34.0f, 0.0f));

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(2, rank);
		SetObjectParent(2, 1);
		pModFile->ReadModel("objects\\alien92.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(2, D3DVECTOR(0.0f, -1.0f, 0.0f));

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(3, rank);
		SetObjectParent(3, 2);
		pModFile->ReadModel("objects\\alien93.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(3, D3DVECTOR(0.0f, 1.0f, 0.0f));

		CreateCrashLine(FPOINT(-15.0f, -15.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 15.0f, -15.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 15.0f,  15.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-15.0f,  15.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-15.0f, -15.0f), SOUND_BOUMm, 0.45f);
	}

	if ( m_type == OBJECT_INCA1 )  // temple ?
	{
		pModFile->ReadModel("objects\\inca1.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);

		CreateCrashLine(FPOINT(-40.0f, -40.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 40.0f, -40.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 40.0f,  40.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-40.0f,  40.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-40.0f, -40.0f), SOUND_BOUMm, 0.45f);
	}

	if ( m_type == OBJECT_INCA2 )  // temple ?
	{
		pModFile->ReadModel("objects\\inca2.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);

		CreateCrashLine(FPOINT(-10.0f, -10.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 10.0f, -10.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 10.0f,  10.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-10.0f,  10.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-10.0f, -10.0f), SOUND_BOUMm, 0.45f);
	}

	if ( m_type == OBJECT_INCA3 )  // temple ?
	{
		pModFile->ReadModel("objects\\inca3.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);

		CreateCrashLine(FPOINT(-50.0f, -30.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 50.0f, -30.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 50.0f,  30.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-50.0f,  30.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-50.0f, -30.0f), SOUND_BOUMm, 0.45f);
	}

	if ( m_type == OBJECT_INCA4 )  // temple ?
	{
		pModFile->ReadModel("objects\\inca4.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);

		CreateCrashLine(FPOINT(-40.0f, -10.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 40.0f, -10.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 40.0f,  10.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-40.0f,  10.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-40.0f, -10.0f), SOUND_BOUMm, 0.45f);
	}

	if ( m_type == OBJECT_INCA5 )  // temple ?
	{
		pModFile->ReadModel("objects\\inca5.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);

		CreateCrashLine(FPOINT(-40.0f, -40.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 40.0f, -40.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 40.0f,  40.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-40.0f,  40.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-40.0f, -40.0f), SOUND_BOUMm, 0.45f);
	}

	if ( m_type == OBJECT_INCA6 )  // statue ?
	{
		pModFile->ReadModel("objects\\inca6.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);

		CreateCrashLine(FPOINT(-10.0f, -10.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 10.0f, -10.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 10.0f,  10.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-10.0f,  10.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-10.0f, -10.0f), SOUND_BOUMm, 0.45f);
	}

	if ( m_type == OBJECT_INCA7 )  // temple électrocuteur ?
	{
		pModFile->ReadModel("objects\\inca71.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);

		rank = m_engine->CreateObject();  // bouton rouge
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(1, rank);
		SetObjectParent(1, 0);
		pModFile->ReadModel("objects\\inca72.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(1, D3DVECTOR(-10.0f, 5.0f, 0.0f));

		rank = m_engine->CreateObject();  // pilier
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(2, rank);
		SetObjectParent(2, 0);
		pModFile->ReadModel("objects\\inca73.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(2, D3DVECTOR(0.0f, 10.0f, 0.0f));

		rank = m_engine->CreateObject();  // porte gauche
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(3, rank);
		SetObjectParent(3, 2);
		pModFile->ReadModel("objects\\inca74.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(3, D3DVECTOR(0.0f, 25.0f, 0.0f));

		rank = m_engine->CreateObject();  // porte droite
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(4, rank);
		SetObjectParent(4, 2);
		pModFile->ReadModel("objects\\inca75.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(4, D3DVECTOR(0.0f, 25.0f, 0.0f));
	
		// bouton rouge :
		CreateCrashSphere(D3DVECTOR(-12.0f, 3.0f,  0.0f), 3.0f, SOUND_BOUMm, 0.44f);

		CreateCrashLine(FPOINT(-10.0f, -10.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 10.0f, -10.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 10.0f,  10.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-10.0f,  10.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-10.0f, -10.0f), SOUND_BOUMm, 0.45f);
	}

	if ( m_type == OBJECT_BUILDING1 )  // bâtiment ?
	{
		pModFile->ReadModel("objects\\buildin1.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);

		CreateCrashLine(FPOINT(-40.0f, -40.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 40.0f, -40.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 40.0f,  40.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-40.0f,  40.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-40.0f, -40.0f), SOUND_BOUMm, 0.45f);
	}

	if ( m_type == OBJECT_BUILDING2 )  // bâtiment ?
	{
		pModFile->ReadModel("objects\\buildin2.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);

		CreateCrashLine(FPOINT(-40.0f, -40.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 40.0f, -40.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 40.0f,  40.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-40.0f,  40.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-40.0f, -40.0f), SOUND_BOUMm, 0.45f);
	}

	if ( m_type == OBJECT_BUILDING3 )  // bâtiment ?
	{
		pModFile->ReadModel("objects\\buildin3.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);

		CreateCrashLine(FPOINT(-40.0f, -40.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 40.0f, -40.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 40.0f,  40.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-40.0f,  40.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-40.0f, -40.0f), SOUND_BOUMm, 0.45f);
	}

	if ( m_type == OBJECT_CARCASS1 )  // carcasse ?
	{
		pModFile->ReadModel("objects\\carcass1.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);
		SetZoom(0, 1.4f);

		CreateCrashLine(FPOINT(-6.0f, -3.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 6.0f, -3.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 6.0f,  3.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-6.0f,  3.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-6.0f, -3.0f), SOUND_BOUMm, 0.45f);

		CreateShadowCircle(4.6f, 1.0f, D3DSHADOWCAR07);
		FloorAdjust();
	}

	if ( m_type == OBJECT_CARCASS2 )  // carcasse ?
	{
		pModFile->ReadModel("objects\\carcass2.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);
		SetZoom(0, 1.4f);

		CreateCrashLine(FPOINT(-6.0f, -3.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 6.0f, -3.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 6.0f,  3.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-6.0f,  3.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-6.0f, -3.0f), SOUND_BOUMm, 0.45f);

		CreateShadowCircle(7.0f, 0.2f);
		FloorAdjust();
	}

	if ( m_type == OBJECT_ORGA1 )  // organique ?
	{
		pModFile->ReadModel("objects\\orga1.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);
		SetZoom(0, zoom);

		CreateCrashSphere(D3DVECTOR(-25.0f, 3.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 24.0f, 3.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
	}

	if ( m_type == OBJECT_ORGA2 )  // organique ?
	{
		pModFile->ReadModel("objects\\orga2.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);
		SetZoom(0, zoom);

		CreateCrashSphere(D3DVECTOR(-25.0f, 3.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 28.0f, 3.0f, 0.0f), 4.0f, SOUND_BOUMm, 0.45f);
	}

	if ( m_type == OBJECT_ORGA3 )  // organique ?
	{
		pModFile->ReadModel("objects\\orga3.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);
		SetZoom(0, zoom);

		CreateCrashSphere(D3DVECTOR(-25.0f, 3.0f, 0.0f), 4.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 24.0f, 3.0f, 0.0f), 4.0f, SOUND_BOUMm, 0.45f);
	}

	if ( m_type == OBJECT_TOWER )
	{
		pModFile->ReadModel("objects\\tower1.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(1, rank);
		SetObjectParent(1, 0);
		pModFile->ReadModel("objects\\tower2.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(1, D3DVECTOR(0.0f, 40.0f, 0.0f));

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(2, rank);
		SetObjectParent(2, 1);
		pModFile->ReadModel("objects\\tower3.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(2, D3DVECTOR(0.0f, 9.0f, 0.0f));
		SetAngleZ(2, 0.0f);

		CreateCrashSphere(D3DVECTOR(0.0f, 4.0f, 0.0f), 11.0f, SOUND_BOUMm, 0.45f);
		SetGlobalSphere(D3DVECTOR(0.0f, 7.0f, 0.0f), 11.0f);

		CreateShadowCircle(9.0f, 1.0f);
	}

	if ( m_type == OBJECT_NUCLEAR )
	{
		pModFile->ReadModel("objects\\nuclear1.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);

		CreateCrashSphere(D3DVECTOR( 0.0f,  0.0f, 0.0f), 55.0f, SOUND_BOUMm, 0.45f);
		SetGlobalSphere(D3DVECTOR(0.0f, 10.0f, 0.0f), 60.0f);
	}

	if ( m_type == OBJECT_PARA )
	{
		pModFile->ReadModel("objects\\para.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);

		m_terrain->AddBuildingLevel(pos, 24.5f, 27.4f, 2.2f, 0.5f);

		CreateCrashSphere(D3DVECTOR( 19.5f, 4.5f,  19.5f), 4.5f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-19.5f, 4.5f,  19.5f), 4.5f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 19.5f, 4.5f, -19.5f), 4.5f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-19.5f, 4.5f, -19.5f), 4.5f, SOUND_BOUMm, 0.45f);
		SetGlobalSphere(D3DVECTOR(0.0f, 15.0f, 0.0f), 30.0f);
	}

	if ( m_type == OBJECT_COMPUTER )
	{
		pModFile->ReadModel("objects\\compu1.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);

		rank = m_engine->CreateObject();  // bouton rouge
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(1, rank);
		SetObjectParent(1, 0);
		pModFile->ReadModel("objects\\compu2.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(1, D3DVECTOR(-6.0f, 5.0f, 0.0f));
		SetZoom(1, D3DVECTOR(0.8f, 0.7f, 0.7f));

		rank = m_engine->CreateObject();  // bouton rouge
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(2, rank);
		SetObjectParent(2, 0);
		pModFile->ReadModel("objects\\compu2.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(2, D3DVECTOR(3.0f, 5.0f, -5.2f));
		SetZoom(2, D3DVECTOR(0.8f, 0.7f, 0.7f));
		SetAngleY(2, -120.0f*PI/180.0f);

		rank = m_engine->CreateObject();  // bouton rouge
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(3, rank);
		SetObjectParent(3, 0);
		pModFile->ReadModel("objects\\compu2.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(3, D3DVECTOR(3.0f, 5.0f, 5.2f));
		SetZoom(3, D3DVECTOR(0.8f, 0.7f, 0.7f));
		SetAngleY(3, 120.0f*PI/180.0f);

		// boutons rouges :
		CreateCrashSphere(D3DVECTOR(-9.0f, 3.0f,  0.0f), 2.0f, SOUND_BOUMm, 0.44f);
		CreateCrashSphere(D3DVECTOR( 4.4f, 3.0f,  7.8f), 2.0f, SOUND_BOUMm, 0.43f);
		CreateCrashSphere(D3DVECTOR( 4.4f, 3.0f, -7.8f), 2.0f, SOUND_BOUMm, 0.42f);

		CreateCrashSphere(D3DVECTOR( 0.0f,  2.0f, 0.0f), 9.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 0.0f, 10.0f, 0.0f), 9.0f, SOUND_BOUMm, 0.45f);
		SetGlobalSphere(D3DVECTOR(0.0f, 10.0f, 0.0f), 10.0f);
		CreateShadowCircle(12.0f, 1.0f);
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
		SetAngleZ(1, PI*0.5f);

		CreateCrashSphere(D3DVECTOR(-11.0f, 3.0f, 0.0f), 8.0f, SOUND_BOUMm, 0.45f);
		SetGlobalSphere(D3DVECTOR(-11.0f, 3.0f, 0.0f), 8.0f);
		m_terrain->AddBuildingLevel(pos, 7.0f, 9.0f, 1.0f, 0.5f);
	}

	if ( m_type == OBJECT_SWEET )
	{
		pModFile->ReadModel("objects\\sweet.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);

		CreateCrashSphere(D3DVECTOR(-4.0f, 3.0f, 0.0f), 7.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 4.0f, 3.0f, 0.0f), 7.0f, SOUND_BOUMm, 0.45f);
		SetGlobalSphere(D3DVECTOR(0.0f, 3.0f, 0.0f), 10.0f);
		CreateShadowCircle(12.0f, 1.0f);
	}

	if ( m_type == OBJECT_DOOR1 )
	{
		pModFile->ReadModel("objects\\door10.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(1, rank);
		SetObjectParent(1, 0);
		pModFile->ReadModel("objects\\door11.mod");
		pModFile->CreateEngineObject(rank);

		// Sphères déplacées par MoveCrashSphere.
		CreateCrashSphere(D3DVECTOR(  0.0f, 35.0f,  24.0f),  6.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(  0.0f, 35.0f,  12.0f),  6.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(  0.0f, 35.0f,   0.0f),  6.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(  0.0f, 35.0f, -12.0f),  6.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(  0.0f, 35.0f, -24.0f),  6.0f, SOUND_BOUMm, 0.45f);

#if 0
		CreateCrashSphere(D3DVECTOR(  5.0f,  3.0f,  42.0f), 12.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( -5.0f,  3.0f,  42.0f), 12.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 14.0f,  3.0f,  33.0f),  3.5f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-14.0f,  3.0f,  33.0f),  3.5f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 14.0f,  3.0f,  51.0f),  3.5f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-14.0f,  3.0f,  51.0f),  3.5f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(  0.0f, 36.0f,  38.0f),  9.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(  5.0f,  3.0f, -42.0f), 12.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( -5.0f,  3.0f, -42.0f), 12.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 14.0f,  3.0f, -33.0f),  3.5f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-14.0f,  3.0f, -33.0f),  3.5f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 14.0f,  3.0f, -51.0f),  3.5f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-14.0f,  3.0f, -51.0f),  3.5f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(  0.0f, 36.0f, -38.0f),  9.0f, SOUND_BOUMm, 0.45f);
#else
		CreateCrashLine(FPOINT(-17.0f,  30.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 17.0f,  30.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 17.0f,  55.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-17.0f,  55.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-17.0f,  30.0f), SOUND_BOUMm, 0.45f);

		CreateCrashLine(FPOINT(-17.0f, -55.0f), SOUND_BOUMm, 0.45f, TRUE);
		CreateCrashLine(FPOINT( 17.0f, -55.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 17.0f, -30.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-17.0f, -30.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-17.0f, -55.0f), SOUND_BOUMm, 0.45f);
#endif
	}

	if ( m_type == OBJECT_DOOR2 )
	{
		pModFile->ReadModel("objects\\door20.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);

		// machoire gauche
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(1, rank);
		SetObjectParent(1, 0);
		pModFile->ReadModel("objects\\door21.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(1, D3DVECTOR(0.0f, 17.0f, 20.0f));

		// machoire droite
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(2, rank);
		SetObjectParent(2, 0);
		pModFile->ReadModel("objects\\door21.mod");
		pModFile->Mirror();
		pModFile->CreateEngineObject(rank);
		SetPosition(2, D3DVECTOR(0.0f, 17.0f, -20.0f));

		// grande roue
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(3, rank);
		SetObjectParent(3, 0);
		pModFile->ReadModel("objects\\door23.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(3, D3DVECTOR(21.0f, 9.0f, -38.0f));

		// piston
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(4, rank);
		SetObjectParent(4, 0);
		pModFile->ReadModel("objects\\door24.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(4, D3DVECTOR(26.0f, 28.0f, -38.0f));

		// bielle
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(5, rank);
		SetObjectParent(5, 4);
		pModFile->ReadModel("objects\\door25.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(5, D3DVECTOR(0.0f, 0.0f, 0.0f));

		// petite roue
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(6, rank);
		SetObjectParent(6, 0);
		pModFile->ReadModel("objects\\door26.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(6, D3DVECTOR(21.0f, 17.0f, -42.0f));

		// Sphères déplacées par MoveCrashSphere.
		CreateCrashSphere(D3DVECTOR(-15.0f, 5.0f, -23.0f),  4.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( -8.0f, 5.0f, -23.0f),  4.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(  0.0f, 5.0f, -23.0f),  4.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(  8.0f, 5.0f, -23.0f),  4.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 15.0f, 5.0f, -23.0f),  4.0f, SOUND_BOUMm, 0.45f);
		// Sphères déplacées par MoveCrashSphere.
		CreateCrashSphere(D3DVECTOR(-15.0f, 5.0f,  23.0f),  4.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( -8.0f, 5.0f,  23.0f),  4.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(  0.0f, 5.0f,  23.0f),  4.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(  8.0f, 5.0f,  23.0f),  4.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 15.0f, 5.0f,  23.0f),  4.0f, SOUND_BOUMm, 0.45f);

		// Sphère du mécanisme.
		CreateCrashSphere(D3DVECTOR( 20.0f, 4.0f, -38.0f),  7.0f, SOUND_BOUMm, 0.44f);

		CreateCrashSphere(D3DVECTOR(-12.0f, 5.0f, -38.0f),  9.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(  0.0f, 5.0f, -38.0f),  9.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 12.0f, 5.0f, -38.0f),  9.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(  0.0f, 5.0f, -48.0f), 11.0f, SOUND_BOUMm, 0.45f);

		CreateCrashSphere(D3DVECTOR(-12.0f, 5.0f,  38.0f),  9.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(  0.0f, 5.0f,  38.0f),  9.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 12.0f, 5.0f,  38.0f),  9.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(  0.0f, 5.0f,  48.0f), 11.0f, SOUND_BOUMm, 0.45f);
	}

	if ( m_type == OBJECT_DOOR3 )
	{
		pModFile->ReadModel("objects\\door30.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);

		// piston écraseur
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(1, rank);
		SetObjectParent(1, 0);
		pModFile->ReadModel("objects\\door31.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(1, D3DVECTOR(0.0f, 20.0f, 0.0f));

		// grande roue
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(3, rank);
		SetObjectParent(3, 0);
		pModFile->ReadModel("objects\\door23.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(3, D3DVECTOR(21.0f, 9.0f, -43.0f));

		// piston
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(4, rank);
		SetObjectParent(4, 0);
		pModFile->ReadModel("objects\\door24.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(4, D3DVECTOR(26.0f, 28.0f, -43.0f));

		// bielle
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(5, rank);
		SetObjectParent(5, 4);
		pModFile->ReadModel("objects\\door25.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(5, D3DVECTOR(0.0f, 0.0f, 0.0f));

		// petite roue
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(6, rank);
		SetObjectParent(6, 0);
		pModFile->ReadModel("objects\\door26.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(6, D3DVECTOR(21.0f, 17.0f, -47.0f));

		// Sphères déplacées par MoveCrashSphere.
		CreateCrashSphere(D3DVECTOR( 14.0f, 25.0f,   0.0f),  6.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-14.0f, 25.0f,   0.0f),  6.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(  0.0f, 25.0f,  14.0f),  6.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(  0.0f, 25.0f, -14.0f),  6.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 10.0f, 25.0f,  10.0f),  6.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 10.0f, 25.0f, -10.0f),  6.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-10.0f, 25.0f,  10.0f),  6.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-10.0f, 25.0f, -10.0f),  6.0f, SOUND_BOUMm, 0.45f);

		// Sphère du mécanisme.
		CreateCrashSphere(D3DVECTOR( 20.0f, 4.0f, -43.0f),  7.0f, SOUND_BOUMm, 0.44f);

#if 0
		CreateCrashSphere(D3DVECTOR(  0.0f, 5.0f, -48.0f), 20.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 15.0f, 5.0f, -35.0f),  5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-15.0f, 5.0f, -35.0f),  5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 15.0f, 5.0f, -61.0f),  5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-15.0f, 5.0f, -61.0f),  5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 25.0f, 3.0f, -32.5f),  3.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-25.0f, 3.0f, -32.5f),  3.0f, SOUND_BOUMm, 0.45f);

		CreateCrashSphere(D3DVECTOR(  0.0f, 5.0f,  48.0f), 20.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 15.0f, 5.0f,  35.0f),  5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-15.0f, 5.0f,  35.0f),  5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 15.0f, 5.0f,  61.0f),  5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-15.0f, 5.0f,  61.0f),  5.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 25.0f, 3.0f,  32.5f),  3.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-25.0f, 3.0f,  32.5f),  3.0f, SOUND_BOUMm, 0.45f);
#else
		CreateCrashSphere(D3DVECTOR( 25.0f, 3.0f, -32.5f),  3.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-25.0f, 3.0f, -32.5f),  3.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 25.0f, 3.0f,  32.5f),  3.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-25.0f, 3.0f,  32.5f),  3.0f, SOUND_BOUMm, 0.45f);

		CreateCrashLine(FPOINT(-20.0f,  30.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 20.0f,  30.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 20.0f,  67.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-20.0f,  67.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-20.0f,  30.0f), SOUND_BOUMm, 0.45f);

		CreateCrashLine(FPOINT(-20.0f, -67.0f), SOUND_BOUMm, 0.45f, TRUE);
		CreateCrashLine(FPOINT( 20.0f, -67.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 20.0f, -30.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-20.0f, -30.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-20.0f, -67.0f), SOUND_BOUMm, 0.45f);
#endif
	}

	if ( m_type == OBJECT_DOOR4 )
	{
		pModFile->ReadModel("objects\\door40.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);

		CreateCrashSphere(D3DVECTOR(0.0f, 2.0f,  34.0f), 6.0f, SOUND_CHOCm, 0.45f);
		CreateCrashSphere(D3DVECTOR(0.0f, 2.0f, -34.0f), 6.0f, SOUND_CHOCm, 0.45f);
	}

	if ( m_type == OBJECT_DOCK )
	{
		pModFile->ReadModel("objects\\dock1.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(1, rank);
		SetObjectParent(1, 0);
		pModFile->ReadModel("objects\\dock2.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(1, D3DVECTOR(0.0f, 20.0f, 0.0f));

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(2, rank);
		SetObjectParent(2, 1);
		pModFile->ReadModel("objects\\dock3.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(2, D3DVECTOR(0.0f, 0.0f, 0.0f));

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(3, rank);
		SetObjectParent(3, 2);
		pModFile->ReadModel("objects\\dock4.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(3, D3DVECTOR(0.0f, (16.0f-14.0f)*3.0f/10.0f, 0.0f));

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(4, rank);
		SetObjectParent(4, 2);
		pModFile->ReadModel("objects\\dock5.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(4, D3DVECTOR(0.0f, (16.0f-14.0f)*6.0f/10.0f, 0.0f));

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(5, rank);
		SetObjectParent(5, 2);
		pModFile->ReadModel("objects\\dock6.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(5, D3DVECTOR(0.0f, 16.0f-14.0f, 0.0f));

		m_terrain->AddBuildingLevel(pos, 7.0f, 9.0f, 1.0f, 0.5f);
		m_terrain->AddSlowerZone(pos, 20.0f, 30.0f, 0.2f);

#if 0
		CreateCrashSphere(D3DVECTOR(  0.0f, 6.0f, 27.0f), 15.0f, SOUND_BOUMm, 0.45f);

		CreateCrashSphere(D3DVECTOR(-15.5f, 2.0f,-12.0f),  1.5f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-15.0f, 7.0f,-12.0f),  1.5f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-15.5f, 2.0f, 12.0f),  1.5f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-14.0f, 2.0f, 17.0f),  2.5f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-14.0f, 2.0f, 22.0f),  2.5f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-14.0f, 2.0f, 27.0f),  2.5f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-14.0f, 2.0f, 32.0f),  2.5f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-14.0f, 2.0f, 37.0f),  2.5f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-15.5f, 2.0f, 42.0f),  1.5f, SOUND_BOUMm, 0.45f);

		CreateCrashSphere(D3DVECTOR( 15.5f, 2.0f,-12.0f),  1.5f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 15.0f, 7.0f,-12.0f),  1.5f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 15.5f, 2.0f, 12.0f),  1.5f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 14.0f, 2.0f, 17.0f),  2.5f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 14.0f, 2.0f, 22.0f),  2.5f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 14.0f, 2.0f, 27.0f),  2.5f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 14.0f, 2.0f, 32.0f),  2.5f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 14.0f, 2.0f, 37.0f),  2.5f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 15.5f, 2.0f, 42.0f),  1.5f, SOUND_BOUMm, 0.45f);

		CreateCrashSphere(D3DVECTOR(-10.0f, 2.0f, 14.0f),  2.5f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( -5.0f, 2.0f, 14.0f),  2.5f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(  0.0f, 2.0f, 14.0f),  2.5f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 10.0f, 2.0f, 14.0f),  2.5f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 10.0f, 2.0f, 14.0f),  2.5f, SOUND_BOUMm, 0.45f);

		CreateCrashSphere(D3DVECTOR(-10.0f, 2.0f, 40.0f),  2.5f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( -5.0f, 2.0f, 40.0f),  2.5f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(  0.0f, 2.0f, 40.0f),  2.5f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 10.0f, 2.0f, 40.0f),  2.5f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 10.0f, 2.0f, 40.0f),  2.5f, SOUND_BOUMm, 0.45f);
#else
		CreateCrashSphere(D3DVECTOR(-15.5f, 2.0f,-12.0f),  1.5f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-15.0f, 7.0f,-12.0f),  1.5f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 15.5f, 2.0f,-12.0f),  1.5f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 15.0f, 7.0f,-12.0f),  1.5f, SOUND_BOUMm, 0.45f);

		CreateCrashLine(FPOINT(-15.0f, 12.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 15.0f, 12.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 15.0f, 42.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-15.0f, 42.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-15.0f, 12.0f), SOUND_BOUMm, 0.45f);
#endif
	}

	if ( m_type == OBJECT_REMOTE )
	{
		pModFile->ReadModel("objects\\remote1.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(1, rank);
		SetObjectParent(1, 0);
		pModFile->ReadModel("objects\\remote2.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(1, D3DVECTOR(3.6f, 4.4f, 0.0f));

		m_terrain->AddBuildingLevel(pos, 7.0f, 9.0f, 1.0f, 0.5f);

		CreateCrashSphere(D3DVECTOR( 6.0f, 3.0f,  4.0f), 1.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 6.0f, 3.0f,  0.0f), 1.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 6.0f, 3.0f, -4.0f), 1.0f, SOUND_BOUMm, 0.45f);
	}

	if ( m_type == OBJECT_STAND )
	{
		pModFile->ReadModel("objects\\stand.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);

		m_terrain->AddBuildingLevel(pos, 7.0f, 9.0f, 1.0f, 0.5f);

		CreateCrashLine(FPOINT(-30.0f, -60.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 22.0f, -60.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 22.0f,  60.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-30.0f,  60.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-30.0f, -60.0f), SOUND_BOUMm, 0.45f);
	}

	if ( m_type == OBJECT_GENERATOR )
	{
		pModFile->ReadModel("objects\\generat.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);
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

	if ( m_type == OBJECT_SUPPORT )
	{
		pModFile->ReadModel("objects\\support.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);
	}

	pos = RetPosition(0);
	pos.y += height;
	SetPosition(0, pos);  // pour afficher les ombres tout de suite

	CreateOtherObject(type);
	m_engine->LoadAllTexture();

	delete pModFile;
	return TRUE;
}

// Crée une petite ressource posée sur le sol.

BOOL CObject::CreateResource(D3DVECTOR pos, float angle, float zoom,
							 ObjectType type, BOOL bPlumb)
{
	CModFile*	pModFile;
	char		name[50];
	int			rank;
	float		radius, height, density;

	if ( type != OBJECT_SHOW )
	{
		if ( m_engine->RetRestCreate() < 1 )  return FALSE;
	}

	pModFile = new CModFile(m_iMan);

	SetType(type);

	rank = m_engine->CreateObject();
	m_engine->SetObjectType(rank, TYPEFIX);  // c'est un objet fixe
	SetObjectRank(0, rank);
	m_character.mass = 1000.0f;

	name[0] = 0;
	if ( type == OBJECT_STONE       )  strcpy(name, "objects\\stone.mod");
	if ( type == OBJECT_URANIUM     )  strcpy(name, "objects\\uranium.mod");
	if ( type == OBJECT_METAL       )  strcpy(name, "objects\\metal.mod");
	if ( type == OBJECT_BARREL      )  strcpy(name, "objects\\barrel.mod");
	if ( type == OBJECT_BARRELa     )  strcpy(name, "objects\\barrela.mod");
	if ( type == OBJECT_ATOMIC      )  strcpy(name, "objects\\atomic.mod");
	if ( type == OBJECT_BULLET      )  strcpy(name, "objects\\bullet.mod");
	if ( type == OBJECT_BBOX        )  strcpy(name, "objects\\bbox.mod");
	if ( type == OBJECT_KEYa        )  strcpy(name, "objects\\keya.mod");
	if ( type == OBJECT_KEYb        )  strcpy(name, "objects\\keyb.mod");
	if ( type == OBJECT_KEYc        )  strcpy(name, "objects\\keyc.mod");
	if ( type == OBJECT_KEYd        )  strcpy(name, "objects\\keyd.mod");
	if ( type == OBJECT_TNT         )  strcpy(name, "objects\\tnt.mod");
	if ( type == OBJECT_MINE        )  strcpy(name, "objects\\mine.mod");
	if ( type == OBJECT_POLE        )  strcpy(name, "objects\\pole.mod");
	if ( type == OBJECT_BOMB        )  strcpy(name, "objects\\bomb1.mod");
	if ( type == OBJECT_CONE        )  strcpy(name, "objects\\cone.mod");
	if ( type == OBJECT_PIPES       )  strcpy(name, "objects\\pipes.mod");
	if ( type == OBJECT_WAYPOINT    )  strcpy(name, "objects\\waypoint.mod");
	if ( type == OBJECT_TRAJECT     )  strcpy(name, "objects\\traject.mod");
	if ( type == OBJECT_TARGET      )  strcpy(name, "objects\\xxx.mod");
	if ( type == OBJECT_SHOW        )  strcpy(name, "objects\\show.mod");
	if ( type == OBJECT_WINFIRE     )  strcpy(name, "objects\\winfire.mod");
	if ( type == OBJECT_BAG         )  strcpy(name, "objects\\bag.mod");
	if ( type == OBJECT_MARK        )  strcpy(name, "objects\\mark.mod");
	if ( type == OBJECT_CROWN       )  strcpy(name, "objects\\crown.mod");
	if ( type == OBJECT_ROADSIGN1   )  strcpy(name, "objects\\road1.mod");
	if ( type == OBJECT_ROADSIGN2   )  strcpy(name, "objects\\road2.mod");
	if ( type == OBJECT_ROADSIGN3   )  strcpy(name, "objects\\road3.mod");
	if ( type == OBJECT_ROADSIGN4   )  strcpy(name, "objects\\road4.mod");
	if ( type == OBJECT_ROADSIGN5   )  strcpy(name, "objects\\road5.mod");
	if ( type == OBJECT_ROADSIGN6   )  strcpy(name, "objects\\road6.mod");
	if ( type == OBJECT_ROADSIGN7   )  strcpy(name, "objects\\road7.mod");
	if ( type == OBJECT_ROADSIGN8   )  strcpy(name, "objects\\road8.mod");
	if ( type == OBJECT_ROADSIGN9   )  strcpy(name, "objects\\road9.mod");
	if ( type == OBJECT_ROADSIGN10  )  strcpy(name, "objects\\road10.mod");
	if ( type == OBJECT_ROADSIGN11  )  strcpy(name, "objects\\road11.mod");
	if ( type == OBJECT_ROADSIGN12  )  strcpy(name, "objects\\road12.mod");
	if ( type == OBJECT_ROADSIGN13  )  strcpy(name, "objects\\road13.mod");
	if ( type == OBJECT_ROADSIGN14  )  strcpy(name, "objects\\road14.mod");
	if ( type == OBJECT_ROADSIGN15  )  strcpy(name, "objects\\road15.mod");
	if ( type == OBJECT_ROADSIGN16  )  strcpy(name, "objects\\road16.mod");
	if ( type == OBJECT_ROADSIGN17  )  strcpy(name, "objects\\road17.mod");
	if ( type == OBJECT_ROADSIGN18  )  strcpy(name, "objects\\road18.mod");
	if ( type == OBJECT_ROADSIGN19  )  strcpy(name, "objects\\road19.mod");
	if ( type == OBJECT_ROADSIGN20  )  strcpy(name, "objects\\road20.mod");
	if ( type == OBJECT_ROADSIGN21  )  strcpy(name, "objects\\road21.mod");
	if ( type == OBJECT_ROADSIGN22  )  strcpy(name, "objects\\road22.mod");
	if ( type == OBJECT_ROADSIGN23  )  strcpy(name, "objects\\road23.mod");
	if ( type == OBJECT_ROADSIGN24  )  strcpy(name, "objects\\road24.mod");
	if ( type == OBJECT_ROADSIGN25  )  strcpy(name, "objects\\road25.mod");
	if ( type == OBJECT_ROADSIGN26  )  strcpy(name, "objects\\road26.mod");
	if ( type == OBJECT_ROADSIGN27  )  strcpy(name, "objects\\road27.mod");
	if ( type == OBJECT_ROADSIGN28  )  strcpy(name, "objects\\road28.mod");
	if ( type == OBJECT_ROADSIGN29  )  strcpy(name, "objects\\road29.mod");
	if ( type == OBJECT_ROADSIGN30  )  strcpy(name, "objects\\road30.mod");
	if ( type == OBJECT_PUB11       )  strcpy(name, "objects\\pub11.mod");
	if ( type == OBJECT_PUB12       )  strcpy(name, "objects\\pub12.mod");
	if ( type == OBJECT_PUB13       )  strcpy(name, "objects\\pub13.mod");
	if ( type == OBJECT_PUB14       )  strcpy(name, "objects\\pub14.mod");
	if ( type == OBJECT_PUB21       )  strcpy(name, "objects\\pub21.mod");
	if ( type == OBJECT_PUB22       )  strcpy(name, "objects\\pub22.mod");
	if ( type == OBJECT_PUB23       )  strcpy(name, "objects\\pub23.mod");
	if ( type == OBJECT_PUB24       )  strcpy(name, "objects\\pub24.mod");
	if ( type == OBJECT_PUB31       )  strcpy(name, "objects\\pub31.mod");
	if ( type == OBJECT_PUB32       )  strcpy(name, "objects\\pub32.mod");
	if ( type == OBJECT_PUB33       )  strcpy(name, "objects\\pub33.mod");
	if ( type == OBJECT_PUB34       )  strcpy(name, "objects\\pub34.mod");
	if ( type == OBJECT_PUB41       )  strcpy(name, "objects\\pub41.mod");
	if ( type == OBJECT_PUB42       )  strcpy(name, "objects\\pub42.mod");
	if ( type == OBJECT_PUB43       )  strcpy(name, "objects\\pub43.mod");
	if ( type == OBJECT_PUB44       )  strcpy(name, "objects\\pub44.mod");
	if ( type == OBJECT_PUB51       )  strcpy(name, "objects\\pub51.mod");
	if ( type == OBJECT_PUB52       )  strcpy(name, "objects\\pub52.mod");
	if ( type == OBJECT_PUB53       )  strcpy(name, "objects\\pub53.mod");
	if ( type == OBJECT_PUB54       )  strcpy(name, "objects\\pub54.mod");
	if ( type == OBJECT_PUB61       )  strcpy(name, "objects\\pub61.mod");
	if ( type == OBJECT_PUB62       )  strcpy(name, "objects\\pub62.mod");
	if ( type == OBJECT_PUB63       )  strcpy(name, "objects\\pub63.mod");
	if ( type == OBJECT_PUB64       )  strcpy(name, "objects\\pub64.mod");
	if ( type == OBJECT_PUB71       )  strcpy(name, "objects\\pub71.mod");
	if ( type == OBJECT_PUB72       )  strcpy(name, "objects\\pub72.mod");
	if ( type == OBJECT_PUB73       )  strcpy(name, "objects\\pub73.mod");
	if ( type == OBJECT_PUB74       )  strcpy(name, "objects\\pub74.mod");
	if ( type == OBJECT_PUB81       )  strcpy(name, "objects\\pub81.mod");
	if ( type == OBJECT_PUB82       )  strcpy(name, "objects\\pub82.mod");
	if ( type == OBJECT_PUB83       )  strcpy(name, "objects\\pub83.mod");
	if ( type == OBJECT_PUB84       )  strcpy(name, "objects\\pub84.mod");
	if ( type == OBJECT_PUB91       )  strcpy(name, "objects\\pub91.mod");
	if ( type == OBJECT_PUB92       )  strcpy(name, "objects\\pub92.mod");
	if ( type == OBJECT_PUB93       )  strcpy(name, "objects\\pub93.mod");
	if ( type == OBJECT_PUB94       )  strcpy(name, "objects\\pub94.mod");
	if ( type == OBJECT_TRAXf       )  strcpy(name, "objects\\trax1f.mod");

	if ( (type >= OBJECT_ROADSIGN1  && type <= OBJECT_ROADSIGN30) ||
		 (type >= OBJECT_PUB11      && type <= OBJECT_PUB94     ) )
	{
		angle -= PI/2.0f;  // pour des raisons historiques !
	}

	pModFile->ReadModel(name);
	pModFile->CreateEngineObject(rank);

	SetPosition(0, pos);
	SetAngleY(0, angle);
	SetZoom(0, zoom);

	if ( type == OBJECT_SHOW )  // reste en l'air ?
	{
		delete pModFile;
		return TRUE;
	}

	radius  = 1.5f;
	density = 1.0f;
	height  = 0.0f;

	if ( type == OBJECT_BOMB )
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(1, rank);
		SetObjectParent(1, 0);
		pModFile->ReadModel("objects\\bomb2.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(1, D3DVECTOR(0.0f, 4.2f, 0.0f));

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(2, rank);
		SetObjectParent(2, 1);
		pModFile->ReadModel("objects\\bomb2.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(2, D3DVECTOR(0.0f, 2.0f, 0.0f));

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(3, rank);
		SetObjectParent(3, 2);
		pModFile->ReadModel("objects\\bomb2.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(3, D3DVECTOR(0.0f, 2.0f, 0.0f));

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(4, rank);
		SetObjectParent(4, 3);
		pModFile->ReadModel("objects\\bomb2.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(4, D3DVECTOR(0.0f, 2.0f, 0.0f));
	}

	if ( type == OBJECT_WAYPOINT    )
	{
	}
	else if ( type == OBJECT_TARGET )
	{
		radius = 0.0f;
	}
	else if ( type == OBJECT_BOMB )
	{
		CreateCrashSphere(D3DVECTOR(0.0f, 1.0f, 0.0f), 2.0f, SOUND_BOUMm, 0.45f);
		SetGlobalSphere(D3DVECTOR(0.0f, 1.0f, 0.0f), 2.0f);
		radius = 3.0f;
	}
	else if ( type == OBJECT_MINE )
	{
		CreateCrashSphere(D3DVECTOR(0.0f, -1.0f, 0.0f), 3.0f, SOUND_BOUMm, 0.45f);
		SetGlobalSphere(D3DVECTOR(0.0f, 0.0f, 0.0f), 3.0f);
		radius = 3.0f;
	}
	else if ( type == OBJECT_POLE )
	{
		CreateCrashSphere(D3DVECTOR(0.0f, 0.0f, 0.0f), 3.5f, SOUND_BOUMm, 0.45f);
		SetGlobalSphere(D3DVECTOR(0.0f, 0.0f, 0.0f), 3.5f);
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
	else if ( type == OBJECT_MARK )
	{
		CreateCrashSphere(D3DVECTOR(0.0f, 3.0f, 4.0f), 6.0f, SOUND_BOUMm, 0.45f);
		radius =  10.0f;
		density = 0.6f;
	}
	else if ( type == OBJECT_CROWN )
	{
		D3DVECTOR	z;
		z = RetZoom(0);
		z.y *= 0.8f;
		SetZoom(0, z);
		radius =  12.0f;
		density = 0.8f;
	}
	else if ( type == OBJECT_BARREL  ||
			  type == OBJECT_BARRELa )
	{
		CreateCrashSphere(D3DVECTOR(0.0f, 2.0f, 0.0f), 2.0f, SOUND_BOUMm, 0.45f);
		SetGlobalSphere(D3DVECTOR(0.0f, 2.0f, 0.0f), 2.0f);
		radius  = 3.0f;
		density = 0.6f;
		m_character.mass = 1500.0f;
	}
	else if ( type == OBJECT_CONE )
	{
		CreateCrashSphere(D3DVECTOR(0.0f, 1.0f, 0.0f), 1.0f, SOUND_BOUMv, 0.45f);
		radius  = 1.2f;
		density = 1.0f;
		m_character.mass = 700.0f;
	}
	else if ( type == OBJECT_PIPES )
	{
		CreateCrashLine(FPOINT(-20.0f, -6.0f), SOUND_CHOCm, 0.45f);
		CreateCrashLine(FPOINT( 20.0f, -6.0f), SOUND_CHOCm, 0.45f);
		CreateCrashLine(FPOINT( 20.0f,  6.0f), SOUND_CHOCm, 0.45f);
		CreateCrashLine(FPOINT(-20.0f,  6.0f), SOUND_CHOCm, 0.45f);
		CreateCrashLine(FPOINT(-20.0f, -6.0f), SOUND_CHOCm, 0.45f);
		radius  = 0.0f;
		CreateShadowCircle(20.0f, 1.0f, D3DSHADOWWORM);
	}
	else if ( type >= OBJECT_ROADSIGN1 &&
			  type <= OBJECT_ROADSIGN5 )
	{
		CreateCrashSphere(D3DVECTOR( 4.0f, 3.0f, 0.0f), 2.0f, SOUND_CHOCo, 0.45f);
		CreateCrashSphere(D3DVECTOR( 0.0f, 3.0f, 0.0f), 2.0f, SOUND_CHOCo, 0.45f);
		CreateCrashSphere(D3DVECTOR(-3.0f, 3.0f, 0.0f), 2.0f, SOUND_CHOCo, 0.45f);
		radius  = 5.0f;
		density = 0.3f;
	}
	else if ( type >= OBJECT_ROADSIGN6  &&
			  type <= OBJECT_ROADSIGN21 )
	{
		CreateCrashSphere(D3DVECTOR(0.0f, 3.0f, 0.0f), 2.5f, SOUND_CHOCo, 0.45f);
		radius  = 2.0f;
		density = 0.3f;
		bPlumb  = TRUE;  // toujours droit
	}
	else if ( type >= OBJECT_ROADSIGN22 &&
			  type <= OBJECT_ROADSIGN25 )
	{
		CreateCrashSphere(D3DVECTOR(0.0f, 3.0f,  32.5f), 3.0f, SOUND_CHOCm, 0.45f);
		CreateCrashSphere(D3DVECTOR(0.0f, 3.0f, -32.5f), 3.0f, SOUND_CHOCm, 0.45f);
		radius  = 0.0f;
		bPlumb  = TRUE;  // toujours droit
	}
	else if ( type >= OBJECT_PUB11 &&
			  type <= OBJECT_PUB94 )
	{
		SetZoom(0, 0.5f*zoom);  // pour éclaircir l'image !!!
		CreateCrashLine(FPOINT(-15.0f, -1.0f), SOUND_CHOCo, 0.45f);
		CreateCrashLine(FPOINT( 15.0f, -1.0f), SOUND_CHOCo, 0.45f);
		CreateCrashLine(FPOINT( 15.0f,  1.0f), SOUND_CHOCo, 0.45f);
		CreateCrashLine(FPOINT(-15.0f,  1.0f), SOUND_CHOCo, 0.45f);
		CreateCrashLine(FPOINT(-15.0f, -1.0f), SOUND_CHOCo, 0.45f);
		radius  = 0.0f;
		bPlumb  = TRUE;  // toujours droit
	}
	else if ( type == OBJECT_TRAXf )
	{
		// Crée la chenille droite.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(1, rank);
		SetObjectParent(1, 0);
		pModFile->ReadModel("objects\\trax2.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(1, D3DVECTOR(-1.0f, 3.0f, -4.0f));

		// Crée la chenille gauche.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(2, rank);
		SetObjectParent(2, 0);
		pModFile->ReadModel("objects\\trax3.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(2, D3DVECTOR(-1.0f, 3.0f, 4.0f));

		// Crée la pelle.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(3, rank);
		SetObjectParent(3, 0);
		pModFile->ReadModel("objects\\trax4.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(3, D3DVECTOR(-1.0f, 4.0f, 0.0f));

		// Crée le levier droite.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(4, rank);
		SetObjectParent(4, 0);
		pModFile->ReadModel("objects\\trax5.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(4, D3DVECTOR(-3.5f, 8.0f, -3.0f));

		// Crée le levier gauche.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(5, rank);
		SetObjectParent(5, 0);
		pModFile->ReadModel("objects\\trax5.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(5, D3DVECTOR(-3.5f, 8.0f, 3.0f));

		CreateCrashSphere(D3DVECTOR(0.0f, 0.0f, 0.0f), 11.0f, SOUND_CHOCo, 0.45f);
		SetGlobalSphere(D3DVECTOR(0.0f, 4.0f, 0.0f), 14.0f);
		radius = 10.0f;
	}
	else if ( type == OBJECT_TRAJECT )
	{
		radius = 0.0f;
	}
	else
	{
		CreateCrashSphere(D3DVECTOR(0.0f, 1.0f, 0.0f), 1.0f, SOUND_BOUMm, 0.45f);
		SetGlobalSphere(D3DVECTOR(0.0f, 1.0f, 0.0f), 1.5f);
	}

	if ( radius != 0.0f )
	{
		CreateShadowCircle(radius, density);
	}

	SetFloorHeight(0.0f);
	CreateOtherObject(type);
	m_engine->LoadAllTexture();
	if ( !bPlumb )  FloorAdjust();

	pos = RetPosition(0);
	pos.y += height;
	SetPosition(0, pos);  // pour afficher les ombres tout de suite

	delete pModFile;
	return TRUE;
}

// Crée un robot posé sur le sol.

BOOL CObject::CreateBot(D3DVECTOR pos, float angle, float zoom,
						ObjectType type, BOOL bPlumb)
{
	m_type = type;

	SetZoom(0, zoom);

	if ( type == OBJECT_CARROT  ||
		 type == OBJECT_STARTER ||
		 type == OBJECT_WALKER  ||
		 type == OBJECT_CRAZY   ||
		 type == OBJECT_GUIDE   ||
		 type == OBJECT_EVIL1   ||
		 type == OBJECT_EVIL3   ||
		 type == OBJECT_EVIL4   ||
		 type == OBJECT_EVIL5   )
	{
		m_physics = new CPhysics(m_iMan, this);
		m_brain   = new CBrain(m_iMan, this);

		m_physics->SetBrain(m_brain);
		m_brain->SetPhysics(m_physics);

		m_motion = new CMotionBot(m_iMan, this);
		if ( m_motion == 0 )  return FALSE;

		m_physics->SetMotion(m_motion);
		m_brain->SetMotion(m_motion);
		m_motion->SetPhysics(m_physics);
		m_motion->SetBrain(m_brain);
		if ( !m_motion->Create(pos, angle, type, TRUE) )
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
	}
	else
	{
		m_motion = new CMotionBot(m_iMan, this);
		if ( m_motion == 0 )  return FALSE;
		if ( !m_motion->Create(pos, angle, type, bPlumb) )  return FALSE;
	}

	return TRUE;
}

// Crée une object additionnel pour un véhicule.

BOOL CObject::CreateAdditionnal(D3DVECTOR pos, float angle, float zoom,
								ObjectType type, BOOL bPlumb)
{
	CModFile*	pModFile;
	int			rank;

	if ( type != OBJECT_SHOW )
	{
		if ( m_engine->RetRestCreate() < 1 )  return FALSE;
	}

	pModFile = new CModFile(m_iMan);

	SetType(type);

	rank = m_engine->CreateObject();
	m_engine->SetObjectType(rank, TYPEFIX);  // c'est un objet fixe
	SetObjectRank(0, rank);
	m_character.mass = 1000.0f;

	if ( type == OBJECT_FIRE )
	{
		pModFile->ReadModel("objects\\fire1.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetZoom(0, zoom);

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(1, rank);
		SetObjectParent(1, 0);
		pModFile->ReadModel("objects\\fire2.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(1, D3DVECTOR(0.0f, 4.5f, 0.0f));

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(2, rank);
		SetObjectParent(2, 1);
		pModFile->ReadModel("objects\\fire3.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(2, D3DVECTOR(0.0f, 2.0f, 0.0f));
		SetAngleZ(2, 20.0f*PI/180.0f);
	}

	if ( type == OBJECT_HELICO )
	{
		pModFile->ReadModel("objects\\helico1.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetZoom(0, zoom);

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(1, rank);
		SetObjectParent(1, 0);
		pModFile->ReadModel("objects\\helico2.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(1, D3DVECTOR(0.0f, 5.0f, 0.0f));
	}

	if ( type == OBJECT_COMPASS )
	{
		pModFile->ReadModel("objects\\compass1.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetZoom(0, zoom);

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(1, rank);
		SetObjectParent(1, 0);
		pModFile->ReadModel("objects\\compass2.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(1, D3DVECTOR(0.0f, 5.0f, 0.0f));
	}

	if ( type == OBJECT_BLITZER )
	{
		pModFile->ReadModel("objects\\blitzer.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetZoom(0, zoom);
	}

	if ( type == OBJECT_HOOK )
	{
		pModFile->ReadModel("objects\\hook1.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetZoom(0, zoom);

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(1, rank);
		SetObjectParent(1, 0);
		pModFile->ReadModel("objects\\hook2.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(1, D3DVECTOR(0.0f, 4.6f, 0.0f));
		SetAngleY(1, PI);

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(2, rank);
		SetObjectParent(2, 1);
		pModFile->ReadModel("objects\\hook3.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(2, D3DVECTOR(-8.0f, 0.9f, 0.0f));

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(3, rank);
		SetObjectParent(3, 2);
		pModFile->ReadModel("objects\\hook4.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(3, D3DVECTOR(0.0f, -2.1f, 0.0f));
	}

	if ( type == OBJECT_AQUA )
	{
		pModFile->ReadModel("objects\\aqua1.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetZoom(0, zoom);

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(1, rank);
		SetObjectParent(1, 0);
		pModFile->ReadModel("objects\\aqua2.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(1, D3DVECTOR(0.0f, 0.0f, 0.0f));

		CreateCrashSphere(D3DVECTOR(0.0f, 1.0f, 0.0f), 2.0f, SOUND_BOUMm, 0.45f);
		CreateShadowCircle(2.0f, 0.8f);
	}

	SetFloorHeight(0.0f);
	CreateOtherObject(type);
	m_engine->LoadAllTexture();
	if ( !bPlumb )  FloorAdjust();

	delete pModFile;
	return TRUE;
}

// Crée une barrière posée sur le sol.

BOOL CObject::CreateBarrier(D3DVECTOR pos, float angle, float zoom,
							float height, ObjectType type, BOOL bPlumb)
{
	CModFile*	pModFile;
	int			rank;

	if ( m_engine->RetRestCreate() < 1 )  return FALSE;

	pModFile = new CModFile(m_iMan);

	SetType(type);

	if ( type == OBJECT_BARRIER0 )  // barrière //// jaune-noire 2.5m
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\barrier0.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);

		CreateCrashSphere(D3DVECTOR( 3.5f, 3.0f, 0.0f), 0.7f, SOUND_CHOCo, 0.45f);
		CreateCrashSphere(D3DVECTOR( 0.0f, 3.0f, 0.0f), 0.7f, SOUND_CHOCo, 0.45f);
		CreateCrashSphere(D3DVECTOR(-3.5f, 3.0f, 0.0f), 0.7f, SOUND_CHOCo, 0.45f);
		SetCrashLineHeight(2.0f);
		CreateShadowCircle(8.0f, 0.2f);
	}

	if ( type == OBJECT_BARRIER1 )  // barrière //// jaune-noire 5m
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\barrier1.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);

		CreateCrashLine(FPOINT(-10.0f, -1.0f), SOUND_CHOCo, 0.45f);
		CreateCrashLine(FPOINT( 10.0f, -1.0f), SOUND_CHOCo, 0.45f);
		CreateCrashLine(FPOINT( 10.0f,  1.0f), SOUND_CHOCo, 0.45f);
		CreateCrashLine(FPOINT(-10.0f,  1.0f), SOUND_CHOCo, 0.45f);
		CreateCrashLine(FPOINT(-10.0f, -1.0f), SOUND_CHOCo, 0.45f);
		SetCrashLineHeight(2.0f);
		CreateShadowCircle(8.0f, 1.0f, D3DSHADOWBARRIER1);
	}

	if ( type == OBJECT_BARRIER2 )  // barrière >>>> rouge-grise 10m
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\barrier2.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);

		CreateCrashLine(FPOINT(-20.0f, -3.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 20.0f, -3.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 20.0f,  3.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-20.0f,  3.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-20.0f, -3.0f), SOUND_BOUMm, 0.45f);
		SetCrashLineHeight(3.0f);
	}

	if ( type == OBJECT_BARRIER3 )  // barrière carrée
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\barrier3.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);

		CreateCrashLine(FPOINT(-10.0f, -10.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 10.0f, -10.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 10.0f,  10.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-10.0f,  10.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-10.0f, -10.0f), SOUND_BOUMm, 0.45f);
		SetCrashLineHeight(3.0f);
		CreateShadowCircle(16.0f, 1.0f);
	}

	if ( type == OBJECT_BARRIER4 )  // barrière en bois cassée 5m
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\barrier4.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);

		CreateCrashLine(FPOINT(-10.0f, -1.0f), SOUND_CHOCo, 0.45f);
		CreateCrashLine(FPOINT( 10.0f, -1.0f), SOUND_CHOCo, 0.45f);
		CreateCrashLine(FPOINT( 10.0f,  1.0f), SOUND_CHOCo, 0.45f);
		CreateCrashLine(FPOINT(-10.0f,  1.0f), SOUND_CHOCo, 0.45f);
		CreateCrashLine(FPOINT(-10.0f, -1.0f), SOUND_CHOCo, 0.45f);
		SetCrashLineHeight(2.0f);
		CreateShadowCircle(8.0f, 1.0f, D3DSHADOWBARRIER1);
	}

	if ( type == OBJECT_BARRIER5 )  // barrière en bois droite 5m
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\barrier5.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);

		CreateCrashLine(FPOINT(-10.0f, -1.0f), SOUND_CHOCo, 0.45f);
		CreateCrashLine(FPOINT( 10.0f, -1.0f), SOUND_CHOCo, 0.45f);
		CreateCrashLine(FPOINT( 10.0f,  1.0f), SOUND_CHOCo, 0.45f);
		CreateCrashLine(FPOINT(-10.0f,  1.0f), SOUND_CHOCo, 0.45f);
		CreateCrashLine(FPOINT(-10.0f, -1.0f), SOUND_CHOCo, 0.45f);
		SetCrashLineHeight(2.0f);
		CreateShadowCircle(8.0f, 1.0f, D3DSHADOWBARRIER1);
	}

	if ( type == OBJECT_BARRIER6 )  // barrière métal 5m
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\barrier6.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);

		CreateCrashLine(FPOINT(-10.0f, -1.0f), SOUND_CHOCm, 0.45f);
		CreateCrashLine(FPOINT( 10.0f, -1.0f), SOUND_CHOCm, 0.45f);
		CreateCrashLine(FPOINT( 10.0f,  1.0f), SOUND_CHOCm, 0.45f);
		CreateCrashLine(FPOINT(-10.0f,  1.0f), SOUND_CHOCm, 0.45f);
		CreateCrashLine(FPOINT(-10.0f, -1.0f), SOUND_CHOCm, 0.45f);
		SetCrashLineHeight(2.0f);
		CreateShadowCircle(8.0f, 1.0f, D3DSHADOWBARRIER1);
	}

	if ( type == OBJECT_BARRIER7 )  // barrière métal 10m
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\barrier7.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);

		CreateCrashLine(FPOINT(-20.0f, -1.0f), SOUND_CHOCm, 0.45f);
		CreateCrashLine(FPOINT( 20.0f, -1.0f), SOUND_CHOCm, 0.45f);
		CreateCrashLine(FPOINT( 20.0f,  1.0f), SOUND_CHOCm, 0.45f);
		CreateCrashLine(FPOINT(-20.0f,  1.0f), SOUND_CHOCm, 0.45f);
		CreateCrashLine(FPOINT(-20.0f, -1.0f), SOUND_CHOCm, 0.45f);
		SetCrashLineHeight(2.0f);
		CreateShadowCircle(16.0f, 1.0f, D3DSHADOWBARRIER2);
	}

	if ( type == OBJECT_BARRIER8 )  // barrière métal 14m
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\barrier8.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);

		CreateCrashLine(FPOINT(-28.0f, -1.0f), SOUND_CHOCm, 0.45f);
		CreateCrashLine(FPOINT( 28.0f, -1.0f), SOUND_CHOCm, 0.45f);
		CreateCrashLine(FPOINT( 28.0f,  1.0f), SOUND_CHOCm, 0.45f);
		CreateCrashLine(FPOINT(-28.0f,  1.0f), SOUND_CHOCm, 0.45f);
		CreateCrashLine(FPOINT(-28.0f, -1.0f), SOUND_CHOCm, 0.45f);
		SetCrashLineHeight(2.0f);
		CreateShadowCircle(19.0f, 1.0f, D3DSHADOWBARRIER2);
	}

	if ( type == OBJECT_BARRIER9 )  // barrière rouge-blanche 10m
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\barrier9.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);

		CreateCrashLine(FPOINT(-20.0f, -1.0f), SOUND_CHOCo, 0.45f);
		CreateCrashLine(FPOINT( 20.0f, -1.0f), SOUND_CHOCo, 0.45f);
		CreateCrashLine(FPOINT( 20.0f,  1.0f), SOUND_CHOCo, 0.45f);
		CreateCrashLine(FPOINT(-20.0f,  1.0f), SOUND_CHOCo, 0.45f);
		CreateCrashLine(FPOINT(-20.0f, -1.0f), SOUND_CHOCo, 0.45f);
		SetCrashLineHeight(3.0f);
		CreateShadowCircle(15.0f, 1.0f, D3DSHADOWBARRIER2);
	}

	if ( type == OBJECT_BARRIER10 )  // tuyau extrémité 10m
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\barrier10.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);

		CreateCrashLine(FPOINT(-20.0f, -1.0f), SOUND_CHOCm, 0.45f);
		CreateCrashLine(FPOINT( 20.0f, -1.0f), SOUND_CHOCm, 0.45f);
		CreateCrashLine(FPOINT( 20.0f,  1.0f), SOUND_CHOCm, 0.45f);
		CreateCrashLine(FPOINT(-20.0f,  1.0f), SOUND_CHOCm, 0.45f);
		CreateCrashLine(FPOINT(-20.0f, -1.0f), SOUND_CHOCm, 0.45f);
		SetCrashLineHeight(3.0f);
		CreateShadowCircle(15.0f, 1.0f, D3DSHADOWBARRIER2);
	}

	if ( type == OBJECT_BARRIER11 )  // tuyau milieu 10m
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\barrier11.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);

		CreateCrashSphere(D3DVECTOR(-20.0f, 3.0f, 0.0f), 3.5f, SOUND_CHOCm, 0.45f);
		CreateCrashLine(FPOINT(-20.0f, -1.0f), SOUND_CHOCm, 0.45f);
		CreateCrashLine(FPOINT( 20.0f, -1.0f), SOUND_CHOCm, 0.45f);
		CreateCrashLine(FPOINT( 20.0f,  1.0f), SOUND_CHOCm, 0.45f);
		CreateCrashLine(FPOINT(-20.0f,  1.0f), SOUND_CHOCm, 0.45f);
		CreateCrashLine(FPOINT(-20.0f, -1.0f), SOUND_CHOCm, 0.45f);
		SetCrashLineHeight(3.0f);
		CreateShadowCircle(15.0f, 1.0f, D3DSHADOWBARRIER2);
	}

	if ( type == OBJECT_BARRIER12 )  // tuyau milieu avec robinet 10m
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\barrier12.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);

		CreateCrashSphere(D3DVECTOR(-20.0f, 3.0f, 0.0f), 3.5f, SOUND_CHOCm, 0.45f);
		CreateCrashLine(FPOINT(-20.0f, -1.0f), SOUND_CHOCm, 0.45f);
		CreateCrashLine(FPOINT( 20.0f, -1.0f), SOUND_CHOCm, 0.45f);
		CreateCrashLine(FPOINT( 20.0f,  1.0f), SOUND_CHOCm, 0.45f);
		CreateCrashLine(FPOINT(-20.0f,  1.0f), SOUND_CHOCm, 0.45f);
		CreateCrashLine(FPOINT(-20.0f, -1.0f), SOUND_CHOCm, 0.45f);
		SetCrashLineHeight(3.0f);
		CreateShadowCircle(15.0f, 1.0f, D3DSHADOWBARRIER2);
	}

	if ( type == OBJECT_BARRIER13 )  // tuyau aérien 10m
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\barrier13.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);

		CreateCrashSphere(D3DVECTOR(-20.0f, 3.0f, 0.0f), 3.5f, SOUND_CHOCm, 0.45f);
		CreateShadowCircle(15.0f, 1.0f, D3DSHADOWBARRIER2);
	}

	if ( type == OBJECT_BARRIER14 )  // mur 10m
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\barrier14.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);

		CreateCrashLine(FPOINT(-20.0f, -1.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 20.0f, -1.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 20.0f,  1.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-20.0f,  1.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-20.0f, -1.0f), SOUND_BOUMm, 0.45f);
		SetGlobalSphere(D3DVECTOR(0.0f, 3.0f, 0.0f), 21.0f);
		bPlumb = TRUE;
	}

	if ( type == OBJECT_BARRIER15 )  // mur haut 10m
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\barrier15.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);

		CreateCrashLine(FPOINT(-20.0f, -1.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 20.0f, -1.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 20.0f,  1.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-20.0f,  1.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-20.0f, -1.0f), SOUND_BOUMm, 0.45f);
		SetGlobalSphere(D3DVECTOR(0.0f, 3.0f, 0.0f), 21.0f);
		bPlumb = TRUE;
	}

	if ( type == OBJECT_BARRIER16 )  // mur  14m
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\barrier16.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);

		CreateCrashLine(FPOINT(-28.0f, -1.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 28.0f, -1.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 28.0f,  1.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-28.0f,  1.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-28.0f, -1.0f), SOUND_BOUMm, 0.45f);
		SetGlobalSphere(D3DVECTOR(0.0f, 3.0f, 0.0f), 30.0f);
		bPlumb = TRUE;
	}

	if ( type == OBJECT_BARRIER17 )  // barrière panneaux >
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\barrier17.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);

		CreateCrashLine(FPOINT(-20.0f, -2.5f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 20.0f, -2.5f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 20.0f,  2.5f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-20.0f,  2.5f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-20.0f, -2.5f), SOUND_BOUMm, 0.45f);
		SetCrashLineHeight(3.0f);
	}
	if ( type == OBJECT_BARRIER18 )  // barrière panneaux <
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\barrier17.mod");
		pModFile->Mirror();
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);

		CreateCrashLine(FPOINT(-20.0f, -2.5f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 20.0f, -2.5f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 20.0f,  2.5f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-20.0f,  2.5f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-20.0f, -2.5f), SOUND_BOUMm, 0.45f);
		SetCrashLineHeight(3.0f);
	}

	if ( type == OBJECT_BARRIER19 )  // poteau >
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\barrier19.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);

		CreateCrashSphere(D3DVECTOR(0.0f, 3.0f, 0.0f), 1.2f, SOUND_CHOCo, 0.45f);
		SetCrashLineHeight(3.0f);
		CreateShadowCircle(2.5f, 0.5f);
	}

	pos = RetPosition(0);
	SetPosition(0, pos);  // pour afficher les ombres tout de suite

	SetFloorHeight(0.0f);
	CreateOtherObject(type);
	if ( !bPlumb )
	{
		FloorAdjust();
		if ( type != OBJECT_BARRIER3  &&
			 type != OBJECT_BARRIER14 )
		{
			SetAngleX(0, 0.0f);
		}
	}

	pos = RetPosition(0);
	pos.y += height;
	SetPosition(0, pos);

	CrashLineFusion();

	delete pModFile;
	return TRUE;
}

// Crée une caisse posée sur le sol.

BOOL CObject::CreateBox(D3DVECTOR pos, float angle, float zoom,
						float height, ObjectType type, BOOL bPlumb)
{
	CModFile*	pModFile;
	int			rank;
	char		text[100];

	if ( m_engine->RetRestCreate() < 1 )  return FALSE;

	pModFile = new CModFile(m_iMan);

	SetType(type);

	if ( type == OBJECT_BOX1 )
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		sprintf(text, "objects\\box1%d.mod", rand()%6);
		pModFile->ReadModel(text);
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		m_character.height = 4.0f;

#if 0
		CreateCrashSphere(D3DVECTOR(0.0f, 0.0f, 0.0f), 4.0f, SOUND_BOUMm, 0.45f);
#else
		CreateCrashLine(FPOINT(-4.0f, -4.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 4.0f, -4.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 4.0f,  4.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-4.0f,  4.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-4.0f, -4.0f), SOUND_BOUMm, 0.45f);
#endif
		CreateShadowCircle(5.0f, 1.0f);
	}

	if ( type == OBJECT_BOX2 )
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		sprintf(text, "objects\\box1%d.mod", rand()%6);
		pModFile->ReadModel(text);
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		m_character.height = 4.0f;

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(1, rank);
		SetObjectParent(1, 0);
		sprintf(text, "objects\\box1%d.mod", rand()%6);
		pModFile->ReadModel(text);
		pModFile->CreateEngineObject(rank);
		SetPosition(1, D3DVECTOR((Rand()-0.5f)*4.0f, 8.0f, (Rand()-0.5f)*4.0f));
		SetAngleY(1, Rand()*PI);

#if 0
		CreateCrashSphere(D3DVECTOR(0.0f, 0.0f, 0.0f), 4.0f, SOUND_BOUMm, 0.45f);
#else
		CreateCrashLine(FPOINT(-4.0f, -4.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 4.0f, -4.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 4.0f,  4.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-4.0f,  4.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-4.0f, -4.0f), SOUND_BOUMm, 0.45f);
#endif
		CreateShadowCircle(5.0f, 1.0f);
		m_character.mass = 4000.0f;
	}

	if ( type == OBJECT_BOX3 )
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		sprintf(text, "objects\\box1%d.mod", rand()%6);
		pModFile->ReadModel(text);
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		m_character.height = 4.0f;

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(1, rank);
		SetObjectParent(1, 0);
		sprintf(text, "objects\\box1%d.mod", rand()%6);
		pModFile->ReadModel(text);
		pModFile->CreateEngineObject(rank);
		SetPosition(1, D3DVECTOR((Rand()-0.5f)*3.0f, 8.0f, (Rand()-0.5f)*3.0f));
		SetAngleY(1, Rand()*PI);

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(2, rank);
		SetObjectParent(2, 1);
		sprintf(text, "objects\\box1%d.mod", rand()%6);
		pModFile->ReadModel(text);
		pModFile->CreateEngineObject(rank);
		SetPosition(2, D3DVECTOR((Rand()-0.5f)*2.0f, 8.0f, (Rand()-0.5f)*2.0f));
		SetAngleY(2, Rand()*PI);

#if 0
		CreateCrashSphere(D3DVECTOR(0.0f, 0.0f, 0.0f), 4.0f, SOUND_BOUMm, 0.45f);
#else
		CreateCrashLine(FPOINT(-4.0f, -4.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 4.0f, -4.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 4.0f,  4.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-4.0f,  4.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-4.0f, -4.0f), SOUND_BOUMm, 0.45f);
#endif
		CreateShadowCircle(5.0f, 1.0f);
		m_character.mass = 6000.0f;
	}

	if ( type == OBJECT_BOX4 )
	{
		m_physics = new CPhysics(m_iMan, this);
		m_physics->SetType(TYPE_MASS);

		m_character.mass = 2000.0f;

//?		m_physics->SetLinMotion(MO_STOACCEL, D3DVECTOR(20.0f, 50.0f, 20.0f));
		m_physics->SetLinMotion(MO_STOACCEL, D3DVECTOR( 5.0f, 50.0f,  5.0f));
//?		m_physics->SetLinMotionY(MO_MOTSPEED, -200.0f);
		m_physics->SetLinMotion(MO_TERSLIDE, D3DVECTOR( 1.0f,  1.0f,  1.0f));
		m_physics->SetLinMotion(MO_TERFORCE, D3DVECTOR(50.0f, 50.0f, 50.0f));
		m_physics->SetCirMotion(MO_MOTACCEL, D3DVECTOR( 1.0f,  1.0f,  1.0f));

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		sprintf(text, "objects\\box1%d.mod", rand()%6);
		pModFile->ReadModel(text);
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		m_character.height = 4.0f;

#if 0
		CreateCrashSphere(D3DVECTOR(0.0f, 0.0f, 0.0f), 4.0f, SOUND_BOUMm, 0.45f);
#else
		CreateCrashLine(FPOINT(-4.0f, -4.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 4.0f, -4.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 4.0f,  4.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-4.0f,  4.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-4.0f, -4.0f), SOUND_BOUMm, 0.45f);
#endif
		CreateShadowCircle(5.0f, 1.0f);
	}

	if ( type == OBJECT_BOX5 )
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\box5.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);

		CreateCrashLine(FPOINT(-4.0f, -4.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 4.0f, -4.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 4.0f,  4.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-4.0f,  4.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-4.0f, -4.0f), SOUND_BOUMm, 0.45f);
		CreateShadowCircle(5.0f, 1.0f);
	}

	if ( type == OBJECT_BOX6 )
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\box6.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);

		CreateCrashLine(FPOINT(-4.0f, -4.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 4.0f, -4.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 4.0f,  4.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-4.0f,  4.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-4.0f, -4.0f), SOUND_BOUMm, 0.45f);
		CreateShadowCircle(5.0f, 1.0f);
	}

	if ( type == OBJECT_TOYS1 )
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\toys1.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);

		CreateCrashSphere(D3DVECTOR(0.0f, 2.0f, 0.0f), 2.2f, SOUND_CHOCo, 0.45f);
		CreateShadowCircle(3.0f, 1.0f);
	}

	if ( type == OBJECT_TOYS2 )
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\toys2.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);

		CreateCrashSphere(D3DVECTOR(0.0f, 2.0f, 0.0f), 2.2f, SOUND_CHOCo, 0.45f);
		CreateShadowCircle(3.0f, 1.0f);
	}

	if ( type == OBJECT_TOYS3 )
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\toys3.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);

		CreateCrashSphere(D3DVECTOR(0.0f, 2.0f, 0.0f), 2.2f, SOUND_CHOCo, 0.45f);
		CreateShadowCircle(3.0f, 1.0f);
	}

	if ( type == OBJECT_TOYS4 )
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\toys4.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);

		CreateCrashSphere(D3DVECTOR(0.0f, 2.0f, 0.0f), 2.2f, SOUND_CHOCo, 0.45f);
		CreateShadowCircle(3.0f, 1.0f);
	}

	if ( type == OBJECT_TOYS5 )
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\toys5.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);

		CreateCrashSphere(D3DVECTOR(0.0f, 2.0f, 0.0f), 2.2f, SOUND_CHOCo, 0.45f);
		CreateShadowCircle(3.0f, 1.0f);
	}

	pos = RetPosition(0);
	SetPosition(0, pos);  // pour afficher les ombres tout de suite

	SetFloorHeight(0.0f);
	CreateOtherObject(type);
	if ( !bPlumb )  FloorAdjust();

	pos = RetPosition(0);
	pos.y += height;
	SetPosition(0, pos);

	delete pModFile;
	return TRUE;
}

// Crée une pierre posée sur le sol.

BOOL CObject::CreateStone(D3DVECTOR pos, float angle, float zoom,
						float height, ObjectType type, BOOL bPlumb)
{
	CModFile*	pModFile;
	int			rank;

	if ( m_engine->RetRestCreate() < 1 )  return FALSE;

	pModFile = new CModFile(m_iMan);

	SetType(type);

	if ( type == OBJECT_STONE1 )  // cube 1
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\stone1.mod");
		pModFile->CreateEngineObject(rank);
		SetZoom(0, zoom);
		SetPosition(0, pos);
		SetAngleY(0, angle);

#if 1
		CreateCrashSphere(D3DVECTOR(0.0f, 3.0f, 0.0f), 6.0f, SOUND_BOUMm, 0.45f);
#else
		CreateCrashLine(FPOINT(-6.0f, -6.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 6.0f, -6.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 6.0f,  6.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-6.0f,  6.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-6.0f, -6.0f), SOUND_BOUMm, 0.45f);
#endif
		CreateShadowCircle(8.0f, 1.0f);
	}

	if ( type == OBJECT_STONE2 )  // cube 2
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\stone2.mod");
		pModFile->CreateEngineObject(rank);
		SetZoom(0, zoom);
		SetPosition(0, pos);
		SetAngleY(0, angle);

#if 1
		CreateCrashSphere(D3DVECTOR(0.0f, 3.0f, 0.0f), 8.0f, SOUND_BOUMm, 0.45f);
#else
		CreateCrashLine(FPOINT(-8.0f, -8.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 8.0f, -8.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 8.0f,  8.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-8.0f,  8.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-8.0f, -8.0f), SOUND_BOUMm, 0.45f);
#endif
		CreateShadowCircle(10.0f, 1.0f);
	}

	if ( type == OBJECT_STONE3 )  // cube 3
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\stone3.mod");
		pModFile->CreateEngineObject(rank);
		SetZoom(0, zoom);
		SetPosition(0, pos);
		SetAngleY(0, angle);

#if 1
		CreateCrashSphere(D3DVECTOR(0.0f, 3.0f, 0.0f), 10.0f, SOUND_BOUMm, 0.45f);
#else
		CreateCrashLine(FPOINT(-9.5f, -9.5f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 9.5f, -9.5f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 9.5f,  9.5f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-9.5f,  9.5f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-9.5f, -9.5f), SOUND_BOUMm, 0.45f);
#endif
		CreateShadowCircle(12.0f, 1.0f);
	}

	if ( type == OBJECT_STONE4 )  // cube 4
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\stone4.mod");
		pModFile->CreateEngineObject(rank);
		SetZoom(0, zoom);
		SetPosition(0, pos);
		SetAngleY(0, angle);

#if 1
		CreateCrashSphere(D3DVECTOR(0.0f, 3.0f, 0.0f), 15.0f, SOUND_BOUMm, 0.45f);
#else
		CreateCrashLine(FPOINT(-14.0f, -14.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 14.0f, -14.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 14.0f,  14.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-14.0f,  14.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-14.0f, -14.0f), SOUND_BOUMm, 0.45f);
#endif
		CreateShadowCircle(18.0f, 1.0f);
	}

	if ( type == OBJECT_STONE5 )  // pilier h=30
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\stone5.mod");
		pModFile->CreateEngineObject(rank);
		SetZoom(0, zoom);
		SetPosition(0, pos);
		SetAngleY(0, angle);

#if 1
		CreateCrashSphere(D3DVECTOR(0.0f, 3.0f, 0.0f), 8.0f, SOUND_BOUMm, 0.45f);
#else
		CreateCrashLine(FPOINT(-8.0f, -8.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 8.0f, -8.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 8.0f,  8.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-8.0f,  8.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-8.0f, -8.0f), SOUND_BOUMm, 0.45f);
#endif
		CreateShadowCircle(10.0f, 1.0f);
	}

	if ( type == OBJECT_STONE6 )  // dalle haute pour piliers espacés d=60
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\stone6.mod");
		pModFile->CreateEngineObject(rank);
		SetZoom(0, zoom);
		SetPosition(0, pos);
		SetAngleY(0, angle);
	}

	if ( type == OBJECT_CROSS1 )
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\cross1.mod");
		pModFile->CreateEngineObject(rank);
		SetZoom(0, zoom);
		SetPosition(0, pos);
		SetAngleY(0, angle);

		CreateCrashSphere(D3DVECTOR( 33.5f, 3.0f,  33.5f), 3.5f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 33.5f, 3.0f, -33.5f), 3.5f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-33.5f, 3.0f,  33.5f), 3.5f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-33.5f, 3.0f, -33.5f), 3.5f, SOUND_BOUMm, 0.45f);
	}

	SetPosition(0, pos);  // pour afficher les ombres tout de suite

	SetFloorHeight(0.0f);
	CreateOtherObject(type);
	if ( !bPlumb )  FloorAdjust();

	pos = RetPosition(0);
	pos.y += height;
	SetPosition(0, pos);

	delete pModFile;
	return TRUE;
}

// Crée une pièce mécanique posée sur le sol.

BOOL CObject::CreatePiece(D3DVECTOR pos, float angle, float zoom,
						float height, ObjectType type, BOOL bPlumb)
{
	CModFile*	pModFile;
	int			rank;
	char		name[50];

	if ( m_engine->RetRestCreate() < 1 )  return FALSE;

	pModFile = new CModFile(m_iMan);

	SetType(type);

	sprintf(name, "objects\\piece%d.mod", type-OBJECT_PIECE0);
	rank = m_engine->CreateObject();
	m_engine->SetObjectType(rank, TYPEFIX);
	SetObjectRank(0, rank);
	pModFile->ReadModel(name);
	pModFile->CreateEngineObject(rank);
	SetZoom(0, zoom);
	SetPosition(0, pos);
	SetAngleY(0, angle);

	CreateShadowCircle(1.0f, 0.5f);

	SetFloorHeight(0.0f);
	CreateOtherObject(type);
	if ( !bPlumb )  FloorAdjust();

	pos = RetPosition(0);
	pos.y += height;
	SetPosition(0, pos);

	delete pModFile;
	return TRUE;
}

// Crée une plante posée sur le sol.

BOOL CObject::CreatePlant(D3DVECTOR pos, float angle, float zoom,
						  float height, ObjectType type, BOOL bPlumb)
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

//?		CreateCrashSphere(D3DVECTOR(0.0f, 0.0f, 0.0f), 4.0f, SOUND_BOUM, 0.10f);
//?		SetGlobalSphere(D3DVECTOR(0.0f, 3.0f, 0.0f), 6.0f);
		SetJotlerSphere(D3DVECTOR(0.0f, 0.0f, 0.0f), 8.0f);

		CreateShadowCircle(8.0f, 0.5f);
	}

	if ( type == OBJECT_PLANT5 ||
		 type == OBJECT_PLANT6 ||
		 type == OBJECT_PLANT7 )  // trèfle ?
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

//?		CreateCrashSphere(D3DVECTOR(0.0f, 12.0f, 0.0f), 5.0f, SOUND_BOUM, 0.10f);
//?		SetGlobalSphere(D3DVECTOR(0.0f, 6.0f, 0.0f), 6.0f);
		SetJotlerSphere(D3DVECTOR(0.0f, 4.0f, 0.0f), 8.0f);

		CreateShadowCircle(8.0f, 0.3f);
	}

	if ( type == OBJECT_PLANT15 ||
		 type == OBJECT_PLANT16 ||
		 type == OBJECT_PLANT17 ||
		 type == OBJECT_PLANT18 ||
		 type == OBJECT_PLANT19 )  // fougère ?
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
//?			CreateCrashSphere(D3DVECTOR(0.0f, 0.0f, 0.0f), 4.0f, SOUND_BOUM, 0.10f);
//?			SetGlobalSphere(D3DVECTOR(0.0f, 3.0f, 0.0f), 6.0f);
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

		CreateCrashSphere(D3DVECTOR( 0.0f,  3.0f, 2.0f), 3.0f, SOUND_CHOCa, 0.45f);
		CreateCrashSphere(D3DVECTOR(-1.0f, 10.0f, 1.0f), 2.0f, SOUND_CHOCa, 0.45f);
		CreateCrashSphere(D3DVECTOR( 0.0f, 17.0f, 0.0f), 2.0f, SOUND_CHOCa, 0.45f);
		CreateCrashSphere(D3DVECTOR( 1.0f, 27.0f, 0.0f), 2.0f, SOUND_CHOCa, 0.45f);

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

		CreateCrashSphere(D3DVECTOR( 0.0f,  3.0f, 2.0f), 3.0f, SOUND_CHOCa, 0.45f);
		CreateCrashSphere(D3DVECTOR(-2.0f, 11.0f, 1.0f), 2.0f, SOUND_CHOCa, 0.45f);
		CreateCrashSphere(D3DVECTOR(-2.0f, 19.0f, 2.0f), 2.0f, SOUND_CHOCa, 0.45f);
		CreateCrashSphere(D3DVECTOR( 2.0f, 26.0f, 0.0f), 2.0f, SOUND_CHOCa, 0.45f);
		CreateCrashSphere(D3DVECTOR( 2.0f, 34.0f,-2.0f), 2.0f, SOUND_CHOCa, 0.45f);

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

		CreateCrashSphere(D3DVECTOR( 0.0f,  3.0f, 1.0f), 3.0f, SOUND_CHOCa, 0.45f);
		CreateCrashSphere(D3DVECTOR(-2.0f, 10.0f, 1.0f), 2.0f, SOUND_CHOCa, 0.45f);
		CreateCrashSphere(D3DVECTOR(-2.0f, 19.0f, 2.0f), 2.0f, SOUND_CHOCa, 0.45f);
		CreateCrashSphere(D3DVECTOR( 2.0f, 25.0f, 0.0f), 2.0f, SOUND_CHOCa, 0.45f);
		CreateCrashSphere(D3DVECTOR( 3.0f, 32.0f,-2.0f), 2.0f, SOUND_CHOCa, 0.45f);

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

		CreateCrashSphere(D3DVECTOR(-2.0f,  3.0f, 2.0f), 3.0f, SOUND_CHOCa, 0.45f);
		CreateCrashSphere(D3DVECTOR(-3.0f,  9.0f, 1.0f), 2.0f, SOUND_CHOCa, 0.45f);
		CreateCrashSphere(D3DVECTOR( 0.0f, 18.0f, 0.0f), 2.0f, SOUND_CHOCa, 0.45f);
		CreateCrashSphere(D3DVECTOR( 0.0f, 27.0f, 7.0f), 2.0f, SOUND_CHOCa, 0.45f);

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

		CreateCrashSphere(D3DVECTOR(0.0f, 10.0f, 0.0f), 10.0f, SOUND_CHOCa, 0.45f);
		CreateCrashSphere(D3DVECTOR(0.0f, 21.0f, 0.0f),  8.0f, SOUND_CHOCa, 0.45f);
		CreateCrashSphere(D3DVECTOR(0.0f, 32.0f, 0.0f),  7.0f, SOUND_CHOCa, 0.45f);

		CreateShadowCircle(8.0f, 0.5f);
	}

	SetZoom(0, zoom);

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

// Crée un champignon posé sur le sol.

BOOL CObject::CreateMushroom(D3DVECTOR pos, float angle, float zoom,
							 float height, ObjectType type, BOOL bPlumb)
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

	SetZoom(0, zoom);

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

// Crée un quartz posé sur le sol.

BOOL CObject::CreateQuartz(D3DVECTOR pos, float angle, float zoom,
						   float height, ObjectType type, BOOL bPlumb)
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

	SetZoom(0, zoom);

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
	m_particule->CreateParticule(pos, pos, FPOINT(2.0f, 2.0f), PARTIQUARTZ, 0.7f+Rand()*0.7f, radius);
	m_particule->CreateParticule(pos, pos, FPOINT(2.0f, 2.0f), PARTIQUARTZ, 0.7f+Rand()*0.7f, radius);

	delete pModFile;
	return TRUE;
}

// Crée une racine posée sur le sol.

BOOL CObject::CreateRoot(D3DVECTOR pos, float angle, float zoom,
						 float height, ObjectType type, BOOL bPlumb)
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
		SetZoom(0, 2.0f*zoom);

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
		SetZoom(0, 2.0f*zoom);

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
		SetZoom(0, 2.0f*zoom);

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
		SetZoom(0, 2.0f*zoom);

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
		SetZoom(0, 2.0f*zoom);

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
		SetZoom(0, 2.0f*zoom);

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

// Crée une petite maison.

BOOL CObject::CreateHome(D3DVECTOR pos, float angle, float zoom,
						 float height, ObjectType type, BOOL bPlumb)
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

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(1, rank);
		SetObjectParent(1, 0);
		pModFile->ReadModel("objects\\home102.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(1, D3DVECTOR(8.5f, 14.0f, 8.5f));
		SetAngleY(1, PI*1.75f);

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(2, rank);
		SetObjectParent(2, 0);
		pModFile->ReadModel("objects\\home102.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(2, D3DVECTOR(-8.5f, 14.0f, 8.5f));
		SetAngleY(2, PI*1.25f);

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(3, rank);
		SetObjectParent(3, 0);
		pModFile->ReadModel("objects\\home102.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(3, D3DVECTOR(8.5f, 14.0f, -8.5f));
		SetAngleY(3, PI*0.25f);

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(4, rank);
		SetObjectParent(4, 0);
		pModFile->ReadModel("objects\\home102.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(4, D3DVECTOR(-8.5f, 14.0f, -8.5f));
		SetAngleY(4, PI*0.75f);

#if 0
		CreateCrashSphere(D3DVECTOR( 0.0f, 2.0f,  0.0f), 10.0f, SOUND_BOUMm, 0.25f);
		CreateCrashSphere(D3DVECTOR(-6.0f, 2.0f,  6.0f),  4.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-6.0f, 2.0f,  0.0f),  4.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-6.0f, 2.0f, -6.0f),  4.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 0.0f, 2.0f,  6.0f),  4.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 0.0f, 2.0f,  0.0f),  4.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 0.0f, 2.0f, -6.0f),  4.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 6.0f, 2.0f,  6.0f),  4.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 6.0f, 2.0f,  0.0f),  4.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 6.0f, 2.0f, -6.0f),  4.0f, SOUND_BOUMm, 0.45f);
#else
		CreateCrashLine(FPOINT(-10.0f, -10.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 10.0f, -10.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 10.0f,  10.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-10.0f,  10.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-10.0f, -10.0f), SOUND_BOUMm, 0.45f);
#endif
		CreateShadowCircle(16.0f, 0.5f);
	}

	if ( type == OBJECT_HOME2 )
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\home2.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);

#if 0
		CreateCrashSphere(D3DVECTOR( 0.0f, 2.0f,  0.0f), 10.0f, SOUND_BOUMm, 0.25f);
		CreateCrashSphere(D3DVECTOR(-6.0f, 2.0f,  6.0f),  4.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-6.0f, 2.0f,  0.0f),  4.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-6.0f, 2.0f, -6.0f),  4.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 0.0f, 2.0f,  6.0f),  4.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 0.0f, 2.0f,  0.0f),  4.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 0.0f, 2.0f, -6.0f),  4.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 6.0f, 2.0f,  6.0f),  4.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 6.0f, 2.0f,  0.0f),  4.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 6.0f, 2.0f, -6.0f),  4.0f, SOUND_BOUMm, 0.45f);
#else
		CreateCrashLine(FPOINT(-10.0f, -10.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 10.0f, -10.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 10.0f,  10.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-10.0f,  10.0f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-10.0f, -10.0f), SOUND_BOUMm, 0.45f);
#endif
		CreateShadowCircle(16.0f, 0.5f);
	}

	if ( type == OBJECT_HOME3 )
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEFIX);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\home3.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);

#if 0
		CreateCrashSphere(D3DVECTOR( 0.0f, 2.0f,  0.0f), 10.0f, SOUND_BOUMm, 0.25f);
		CreateCrashSphere(D3DVECTOR(-6.0f, 2.0f,  6.0f),  4.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-6.0f, 2.0f,  0.0f),  4.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR(-6.0f, 2.0f, -6.0f),  4.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 0.0f, 2.0f,  6.0f),  4.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 0.0f, 2.0f,  0.0f),  4.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 0.0f, 2.0f, -6.0f),  4.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 6.0f, 2.0f,  6.0f),  4.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 6.0f, 2.0f,  0.0f),  4.0f, SOUND_BOUMm, 0.45f);
		CreateCrashSphere(D3DVECTOR( 6.0f, 2.0f, -6.0f),  4.0f, SOUND_BOUMm, 0.45f);
#else
		CreateCrashLine(FPOINT(-9.5f, -9.5f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 9.5f, -9.5f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT( 9.5f,  9.5f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-9.5f,  9.5f), SOUND_BOUMm, 0.45f);
		CreateCrashLine(FPOINT(-9.5f, -9.5f), SOUND_BOUMm, 0.45f);
#endif
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

// Crée une ruine posée sur le sol.

BOOL CObject::CreateRuin(D3DVECTOR pos, float angle, float zoom,
						 float height, ObjectType type, BOOL bPlumb)
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

	if ( type == OBJECT_RUINmobilew1 )  // véhicule à roues ?
	{
		// Crée la roue arrière-droite.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(6, rank);
		SetObjectParent(6, 0);

		pModFile->ReadModel("objects\\ruin1w.mod");
		pModFile->CreateEngineObject(rank);

		SetPosition(6, D3DVECTOR(-3.0f, 1.8f, -4.0f));
		SetAngleX(6, -PI/2.0f);

		// Crée la roue arrière-gauche.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(7, rank);
		SetObjectParent(7, 0);

		pModFile->ReadModel("objects\\ruin1w.mod");
		pModFile->CreateEngineObject(rank);

		SetPosition(7, D3DVECTOR(-3.0f, 1.0f, 3.0f));
		SetAngleY(7, PI-0.3f);
		SetAngleX(7, -0.3f);

		// Crée la roue avant-droite.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(8, rank);
		SetObjectParent(8, 0);

		pModFile->ReadModel("objects\\ruin1w.mod");
		pModFile->CreateEngineObject(rank);

		SetPosition(8, D3DVECTOR(2.0f, 1.6f, -3.0f));
		SetAngleY(8, 0.3f);

		// Crée la roue avant-gauche.
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

	if ( type == OBJECT_RUINmobilew2 )  // véhicule à roues ?
	{
		// Crée la roue arrière-gauche.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(7, rank);
		SetObjectParent(7, 0);

		pModFile->ReadModel("objects\\ruin1w.mod");
		pModFile->CreateEngineObject(rank);

		SetPosition(7, D3DVECTOR(-3.0f, 1.0f, 3.0f));
		SetAngleY(7, PI+0.3f);
		SetAngleX(7, 0.4f);

		// Crée la roue avant-gauche.
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

	if ( type == OBJECT_RUINmobilet1 )  // véhicule à chenilles ?
	{
		// Crée le canon.
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

	if ( type == OBJECT_RUINmobilet2 )  // véhicule à chenilles ?
	{
		CreateCrashSphere(D3DVECTOR(0.0f, 2.8f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
//?		SetGlobalSphere(D3DVECTOR(0.0f, 5.0f, 0.0f), 10.0f);

		CreateShadowCircle(5.0f, 1.0f);
	}

	if ( type == OBJECT_RUINmobiler1 )  // véhicule roller ?
	{
		CreateCrashSphere(D3DVECTOR(1.0f, 2.8f, -1.0f), 5.0f, SOUND_BOUMm, 0.45f);
		SetGlobalSphere(D3DVECTOR(1.0f, 5.0f, -1.0f), 10.0f);

		CreateShadowCircle(5.0f, 1.0f);
	}

	if ( type == OBJECT_RUINmobiler2 )  // véhicule roller ?
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
		if ( !bPlumb )  FloorAdjust();
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

// Crée tous les sous-objets permettant de gérer cet objet.

void CObject::CreateOtherObject(ObjectType type)
{
	if ( type == OBJECT_TOWER )
	{
		m_auto = new CAutoTower(m_iMan, this);
	}
	if ( type == OBJECT_DOOR1 ||
		 type == OBJECT_DOOR2 ||
		 type == OBJECT_DOOR3 ||
		 type == OBJECT_DOOR4 )
	{
		m_auto = new CAutoDoor(m_iMan, this);
	}
	if ( type == OBJECT_ALIEN2 ||
		 type == OBJECT_ALIEN3 ||
		 type == OBJECT_ALIEN5 ||
		 type == OBJECT_ALIEN6 ||
		 type == OBJECT_ALIEN7 ||
		 type == OBJECT_ALIEN8 ||
		 type == OBJECT_ALIEN9 )
	{
		m_auto = new CAutoAlien(m_iMan, this);
	}
	if ( type == OBJECT_DOCK )
	{
		m_auto = new CAutoDock(m_iMan, this);
	}
	if ( type == OBJECT_REMOTE )
	{
		m_auto = new CAutoRemote(m_iMan, this);
	}
	if ( type == OBJECT_STAND )
	{
		m_auto = new CAutoStand(m_iMan, this);
	}
	if ( type == OBJECT_GENERATOR )
	{
		m_auto = new CAutoGenerator(m_iMan, this);
	}
	if ( type == OBJECT_COMPUTER )
	{
		m_auto = new CAutoComputer(m_iMan, this);
	}
	if ( type == OBJECT_REPAIR )
	{
		m_auto = new CAutoRepair(m_iMan, this);
	}
	if ( type == OBJECT_FIRE )
	{
		m_auto = new CAutoFire(m_iMan, this);
	}
	if ( type == OBJECT_HELICO )
	{
		m_auto = new CAutoHelico(m_iMan, this);
	}
	if ( type == OBJECT_COMPASS )
	{
		m_auto = new CAutoCompass(m_iMan, this);
	}
	if ( type == OBJECT_BLITZER )
	{
		m_auto = new CAutoBlitzer(m_iMan, this);
	}
	if ( type == OBJECT_INCA7 )
	{
		m_auto = new CAutoInca(m_iMan, this);
	}
	if ( type == OBJECT_HOOK )
	{
		m_auto = new CAutoHook(m_iMan, this);
	}
	if ( type == OBJECT_BARREL    ||
		 type == OBJECT_BARRELa   ||
		 type == OBJECT_CARCASS1  ||
		 type == OBJECT_CARCASS2  ||
		 type == OBJECT_CARCASS3  ||
		 type == OBJECT_CARCASS4  ||
		 type == OBJECT_CARCASS5  ||
		 type == OBJECT_CARCASS6  ||
		 type == OBJECT_CARCASS7  ||
		 type == OBJECT_CARCASS8  ||
		 type == OBJECT_CARCASS9  ||
		 type == OBJECT_CARCASS10 )
	{
		m_auto = new CAutoBarrel(m_iMan, this);
	}
	if ( type == OBJECT_BOMB )
	{
		m_auto = new CAutoBomb(m_iMan, this);
	}
	if ( type == OBJECT_HOME1 ||
		 type == OBJECT_HOME2 ||
		 type == OBJECT_HOME3 ||
		 type == OBJECT_HOME4 ||
		 type == OBJECT_HOME5 )
	{
		m_auto = new CAutoHome(m_iMan, this);
	}
	if ( type == OBJECT_ROOT5 )
	{
		m_auto = new CAutoRoot(m_iMan, this);
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

// Démarre un programme.

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
// Retourne TRUE si la matrice a changé.
// Les rotations ont lieu dans l'ordre Y, Z et X.

BOOL CObject::UpdateTransformObject(int part, BOOL bForceUpdate)
{
	D3DVECTOR	position, angle, eye;
	BOOL		bModif = FALSE;
	int			parent;

	if ( m_truck != 0 )  // transporté par camion ?
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
		angle    += m_cirVibration+m_cirChoc+m_inclinaison;
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

		if ( part == 0 && m_truck != 0 )  // transporté par un camion ?
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

// Met à jour toutes les matrices pour transformer l'objet père
// et tous ses fils.
// On suppose un maximum de 4 degrés de liberté. Cela convient,
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
		for ( parent1=0 ; parent1<m_totalPart ; parent1++ )
		{
			if ( !m_objectPart[parent1].bUsed )  continue;
			if ( m_objectPart[parent1].parentPart != -1 )  continue;

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
	}

	return TRUE;
}


// Met toute la descendance à plat (il n'y a plus que des pères).
// Ceci permet de faire partir les débris indépendamment les uns
// des autres dans tous les sens.

void CObject::FlatParent()
{
	int		i;

	for ( i=0 ; i<m_totalPart ; i++ )
	{
		if ( m_objectPart[i].bUsed )
		{
			FlatParent(i);
		}
	}

	m_bFlat = TRUE;
}

// Met un sous-objet à plat (il devient père).
// Ceci permet de faire partir le sous-objet sous forme d'un débris
// indépendamment du reste de l'objet.

BOOL CObject::FlatParent(int part)
{
	int		i = part;

	if ( m_objectPart[i].parentPart == -1 )  return FALSE;

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

	return TRUE;
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
		if ( m_bRecorderPlay && m_recorder != 0 )  // joue l'objet ?
		{
			RecorderEvent	re;
			D3DVECTOR		pos;
			float			time;

			time = m_main->RetRecordTime();
			m_recorder->Get(time, re);
			pos = re.position;

			if ( !m_main->IsRecordTime() && !m_engine->RetPause() )
			{
				pos.y -= 100.0f;  // voiture cachée
			}

			SetPosition(0, pos);
			SetAngle(0, re.angle);
		}
		else
		{
			if ( !m_physics->EventProcess(event) )  // objet détruit ?
			{
				if ( RetSelect() )
				{
					if ( !m_bDead )  m_camera->SetType(CAMERA_EXPLO);
					m_main->DeselectAll();
				}
				m_main->StopStartCounter();  // moteur explosé
				return FALSE;
			}

			if ( m_bRecorderRecord && m_recorder != 0 )  // enregistre l'objet ?
			{
				RecorderEvent	re;
				float			time;

				time = m_main->RetRecordTime();
				re.position  = RetPosition(0);
				re.position += RetLinVibration();
				re.angle     = RetAngle(0);
				re.angle    += RetInclinaison();
				re.angle    += RetCirVibration();
				m_recorder->Put(time, re);
			}
		}
	}

	if ( m_auto != 0 )
	{
		if ( !m_auto->EventProcess(event) )
		{
			return TRUE;  // bombe détruite
		}

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

	PartiFrame(event.rTime);

	UpdateMapping();
	UpdateTransformObject();
	UpdateSelectParticule();

	return TRUE;
}

// Met à jour le mapping de l'objet.

void CObject::UpdateMapping()
{
}


// Gestion des particules maîtresses.

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

		// Chaque morceau tournoie différemment.
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


// Modifie le point de vue pour voir comme si on était
// dans le véhicule, ou derrière le véhicule.

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
	else if ( m_type == OBJECT_CAR )
	{
		eye = m_character.camera;
	}
	else
	{
		eye.x =  0.7f;  // entre les supports
		eye.y =  4.8f;
		eye.z =  0.0f;
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

	// Penche la caméra dans les virages.
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


// Gestion des caractéristiques.

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
		 m_type == OBJECT_MINE     ||
		 m_type == OBJECT_POLE     ||
		 m_type == OBJECT_BOMB     ||
		 m_type == OBJECT_WAYPOINT ||
		 m_type == OBJECT_BARREL   ||
		 m_type == OBJECT_BARRELa  ||
		 m_type == OBJECT_ATOMIC   ||
		(m_type >= OBJECT_ROADSIGN1 && m_type <= OBJECT_ROADSIGN30) )
	{
		return 0.0f;
	}
	return m_shield;
}


// Gestion du bouclier de résistance au feu.

void CObject::SetBurnShield(float level)
{
	m_burnShield = level;
}

float CObject::RetBurnShield()
{
	return m_burnShield;
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
			m_engine->SetObjectTransparency(m_objectPart[i].object, value);
		}
	}
}

float CObject::RetTransparency()
{
	return m_transparency;
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

	if ( m_auto != 0 )  return FALSE;

	m_auto = new CAutoJostle(m_iMan, this);
	pa = (CAutoJostle*)m_auto;
	pa->Start(0, force);
	return TRUE;
}


// Gestion du mode de la caméra.

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



// Gestion de la mise en évidence de l'objet.

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

		m_engine->SetHiliteRank(list);  // donne la liste des parties sélectionnées
	}
}

BOOL CObject::RetHilite()
{
	return m_bHilite;
}


// Indique si l'objet est sélecionné ou non.

void CObject::SetSelect(BOOL bMode, BOOL bDisplayError)
{
	Error		err;

	if ( m_main->RetFixScene() )
	{
		CreateSelectParticule();  // crée/supprime les particules
		return;
	}

	m_bSelect = bMode;

	if ( m_physics != 0 )
	{
		m_physics->CreateInterface(m_bSelect);
	}

	CreateSelectParticule();  // crée/supprime les particules

	if ( !m_bSelect )
	{
		return;  // fini si pas sélectionné
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
		m_displayText->DisplayError(err);
	}
}

// Indique si l'objet est sélectionné ou non.

BOOL CObject::RetSelect(BOOL bReal)
{
	if ( !bReal && m_main->RetFixScene() )  return FALSE;
	return m_bSelect;
}


// Indique si l'objet est sélecionnable ou non.

void CObject::SetSelectable(BOOL bMode)
{
	m_bSelectable = bMode;
}

// Indique si l'objet est sélecionnable ou non.

BOOL CObject::RetSelectable()
{
	return m_bSelectable;
}


// Gestion de l'activité d'un objet.

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


// Gestion de la visibilité d'un objet.
// L'objet n'est pas caché visuellement ni inactif, mais ignoré
// des détections ! Par exemple: ver sous terre.

void CObject::SetVisible(BOOL bVisible)
{
	m_bVisible = bVisible;
}

BOOL CObject::RetVisible()
{
	return m_bVisible;
}


// Gestion du mode de fonctionnement d'un objet. Un objet
// inactif est identique à un objet détruit, inexistant.
// Ce mode est utilisé pour les objets "resetables" lors
// d'entraînement, pour simuler une destruction.

void CObject::SetEnable(BOOL bEnable)
{
	m_bEnable = bEnable;
}

BOOL CObject::RetEnable()
{
	return m_bEnable;
}


// Gestion du mode fantome;

void CObject::SetGhost(BOOL bGhost)
{
	m_bGhost = bGhost;
}

BOOL CObject::RetGhost()
{
	return m_bGhost;
}


// Gestion du mode "à terre", lorsqu'un objet a été renversé
// par PT_ACROBATIC;

void CObject::SetGround(BOOL bGround)
{
	m_bGround = bGround;
}

BOOL CObject::RetGround()
{
	return m_bGround;
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


// Gestion du paramètre libre.

void CObject::SetParam(float value)
{
	m_param = value;
}

float CObject::RetParam()
{
	return m_param;
}


// Gestion du mode "bloqué" d'un objet.
// Par exemple, un cube de titanium est bloqué pendant qu'il est utilisé
// pour fabriquer qq chose, ou un véhicule est bloqué tant que sa
// construction n'est pas terminée.

void CObject::SetLock(BOOL bLock)
{
	m_bLock = bLock;
}

BOOL CObject::RetLock()
{
	return m_bLock;
}

// Gestion du mode "bloqué" pendant le compte à rebour (3, 2, 1, GO).

void CObject::SetStarting(BOOL bStarting)
{
	m_bStarting = bStarting;
}

BOOL CObject::RetStarting()
{
	return m_bStarting;
}

// Gestion du mode "en cours d'explosion" d'un objet.
// Un objet dans ce mode n'est pas sauvegardé.

void CObject::SetExplo(BOOL bExplo)
{
	m_bExplo = bExplo;
}

BOOL CObject::RetExplo()
{
	return m_bExplo;
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
		m_brain->StopProgram();  // stoppe la tâche en cours
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
	return !m_bLock && !m_bBurn && !m_bFlat && m_bVisible && m_bEnable && !m_bGhost;
}



// Gestion du compteur du nombre de passages.

void CObject::SetPassCounter(int counter)
{
	m_passCounter = counter;
}

int CObject::RetPassCounter()
{
	return m_passCounter;
}

// Gestion de l'ordre de passages.

void CObject::SetRankCounter(int rank)
{
	m_rankCounter = rank;
}

int CObject::RetRankCounter()
{
	return m_rankCounter;
}



// Indique si un programme est en cours d'exécution.

BOOL CObject::IsProgram()
{
	if ( m_brain == 0 )  return FALSE;
	return m_brain->IsProgram();
}


// Crée ou supprime les particules associées à l'objet.

void CObject::CreateSelectParticule()
{
	D3DVECTOR	pos, speed;
	FPOINT		dim;
	int			i;

	// Supprime les particules précédentes.
	for ( i=0 ; i<10 ; i++ )
	{
		if ( m_partiSel[i] != -1 )
		{
			m_particule->DeleteParticule(m_partiSel[i]);
			m_partiSel[i] = -1;
		}
	}

	if ( m_bSelect || IsProgram() || m_main->RetFixScene() )
	{
		// Crée les particules lens pour les phares.
		if ( m_type == OBJECT_CAR      ||
			 m_type == OBJECT_MOBILEtg ||
			 m_type == OBJECT_MOBILEfb ||
			 m_type == OBJECT_MOBILEob ||
			 m_type == OBJECT_TRAX     ||
			 m_type == OBJECT_UFO      )  // véhicule ?
		{
			pos = D3DVECTOR(0.0f, 0.0f, 0.0f);
			speed = D3DVECTOR(0.0f, 0.0f, 0.0f);
			dim.x = 0.0f;
			dim.y = 0.0f;
			m_partiSel[0] = m_particule->CreateParticule(pos, speed, dim, PARTISELY, 1.0f, 0.0f);
			m_partiSel[1] = m_particule->CreateParticule(pos, speed, dim, PARTISELY, 1.0f, 0.0f);
			m_partiSel[2] = m_particule->CreateParticule(pos, speed, dim, PARTISELR, 1.0f, 0.0f);
			m_partiSel[3] = m_particule->CreateParticule(pos, speed, dim, PARTISELR, 1.0f, 0.0f);
			m_partiSel[4] = m_particule->CreateParticule(pos, speed, dim, PARTISELY, 1.0f, 0.0f);
			m_partiSel[5] = m_particule->CreateParticule(pos, speed, dim, PARTISELY, 1.0f, 0.0f);
			m_partiSel[6] = m_particule->CreateParticule(pos, speed, dim, PARTISELR, 1.0f, 0.0f);
			UpdateSelectParticule();
		}
	}
}

// Met à jour les particules associées à l'objet.

void CObject::UpdateSelectParticule()
{
	D3DVECTOR	pos[10];
	FPOINT		dim[10], pp;
	float		zoom[10];
	float		angle;
	int			i;

	if ( !m_bSelect && !IsProgram() && !m_main->RetFixScene() )  return;

	dim[0].x = 1.0f;
	dim[1].x = 1.0f;
	dim[2].x = 1.2f;
	dim[3].x = 1.2f;
	dim[4].x = 1.2f;
	dim[5].x = 1.2f;
	dim[6].x = 0.2f;

	// Lens avants jaunes.
	if ( m_type == OBJECT_MOBILEtg )  // cible ?
	{
		pos[0] = D3DVECTOR(3.4f, 6.5f,  2.0f);
		pos[1] = D3DVECTOR(3.4f, 6.5f, -2.0f);
	}
	else if ( m_type == OBJECT_CAR )
	{
		pos[0] = m_character.lightFL;
		pos[1] = m_character.lightFR;
	}
	else if ( m_type == OBJECT_TRAX )
	{
		pos[0] = D3DVECTOR(6.2f, 10.0f,  2.5f);
		pos[1] = D3DVECTOR(6.2f, 10.0f, -2.5f);
	}
	else if ( m_type == OBJECT_UFO )
	{
		angle = m_aTime*2.0f;
		angle *= 180.0f/PI/22.5f;
		angle = (float)((int)angle);
		angle /= 180.0f/PI/22.5f;
		pp = RotatePoint(angle, 6.0f);
		pos[0] = D3DVECTOR(pp.x, -0.5f, pp.y);
		pp = RotatePoint(-angle, 6.0f);
		pos[1] = D3DVECTOR(pp.x, -0.5f, pp.y);
	}
	else
	{
		pos[0] = D3DVECTOR(4.2f, 2.5f,  1.5f);
		pos[1] = D3DVECTOR(4.2f, 2.5f, -1.5f);
	}

	// Lens arrières rouges+blanche.
	if ( m_type == OBJECT_CAR )
	{
		pos[2] = m_character.lightSL;
		pos[3] = m_character.lightSR;
		pos[4] = m_character.lightRL;
		pos[5] = m_character.lightRR;
	}
	if ( m_type == OBJECT_MOBILEtg )  // cible ?
	{
		pos[2] = D3DVECTOR(-2.4f, 6.5f,  2.0f);
		pos[3] = D3DVECTOR(-2.4f, 6.5f, -2.0f);
		pos[4] = D3DVECTOR(-2.4f, 6.0f,  2.0f);
		pos[5] = D3DVECTOR(-2.4f, 6.0f, -2.0f);
	}

	// Lens sur l'antenne.
	pos[6] = m_character.antenna;

	angle = RetAngleY(0)/PI;

	zoom[0] = 1.0f;
	zoom[1] = 1.0f;
	zoom[2] = 1.0f;
	zoom[3] = 1.0f;
	zoom[4] = 1.0f;
	zoom[5] = 1.0f;
	zoom[6] = 1.0f;

	if ( IsProgram() &&  // programme en cours ?
		 Mod(m_aTime, 0.7f) < 0.3f )
	{
		zoom[0] = 0.0f;  // clignotte
		zoom[1] = 0.0f;
		zoom[2] = 0.0f;
		zoom[3] = 0.0f;
		zoom[4] = 0.0f;
		zoom[5] = 0.0f;
	}

	if ( m_type == OBJECT_CAR )
	{
		if ( m_physics != 0 )
		{
			for ( i=0 ; i<6 ; i++ )
			{
				if ( !m_physics->RetLight(i) )  zoom[i] = 0.0f;  // éteint
				if ( m_main->RetFixScene() && (i==2 || i==3) )  zoom[i] = 0.5f;
			}
		}

		if ( Mod(m_aTime, 0.3f) < 0.15f )  zoom[6] = 0.0f;
		if ( m_bDead )  zoom[6] = 0.0f;
	}

	if ( m_type == OBJECT_TRAX )
	{
		zoom[0] = 4.0f;
		zoom[1] = 4.0f;
		zoom[2] = 0.0f;
		zoom[3] = 0.0f;
		zoom[4] = 0.0f;
		zoom[5] = 0.0f;
		zoom[6] = 0.0f;
	}

	if ( m_type == OBJECT_UFO )
	{
		zoom[0] = 2.0f;
		zoom[1] = 2.0f;
		zoom[2] = 0.0f;
		zoom[3] = 0.0f;
		zoom[4] = 0.0f;
		zoom[5] = 0.0f;
		zoom[6] = 0.0f;
	}

	// Met à jour tous les lens.
	for ( i=0 ; i<7 ; i++ )
	{
		pos[i] = Transform(m_objectPart[0].matWorld, pos[i]);
		dim[i].y = dim[i].x;
		m_particule->SetParam(m_partiSel[i], pos[i], dim[i], zoom[i], angle, 1.0f);
	}
}


// Donne le pointeur au script en cours d'exécution.

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

// Retourne la physique associée à l'objet.

CPhysics* CObject::RetPhysics()
{
	return m_physics;
}

// Retourne le cerveau associé à l'objet.

CBrain* CObject::RetBrain()
{
	return m_brain;
}

// Retourne le mouvement associé à l'objet.

CMotion* CObject::RetMotion()
{
	return m_motion;
}

// Retourne l'automate associé à l'objet.

CAuto* CObject::RetAuto()
{
	return m_auto;
}

void CObject::SetAuto(CAuto* automat)
{
	m_auto = automat;
}



// Gestion du rang dans le fichier de définition.

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


// Gestion du mode d'enregistrement.

void CObject::SetRecorderRecordMode(BOOL bRecord)
{
	m_bRecorderRecord = bRecord;
}

BOOL CObject::RetRecorderRecordMode()
{
	return m_bRecorderRecord;
}

void CObject::SetRecorderPlayMode(BOOL bPlay)
{
	m_bRecorderPlay = bPlay;
}

BOOL CObject::RetRecorderPlayMode()
{
	return m_bRecorderPlay;
}

void CObject::SetRecorder(CRecorder* recorder)
{
	m_recorder = recorder;
}

CRecorder* CObject::RetRecorder()
{
	return m_recorder;
}


