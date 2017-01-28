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
#include "language.h"
#include "global.h"
#include "event.h"
#include "misc.h"
#include "iman.h"
#include "restext.h"
#include "math3d.h"
#include "robotmain.h"
#include "mainundo.h"
#include "light.h"
#include "terrain.h"
#include "water.h"
#include "blitz.h"
#include "camera.h"
#include "particule.h"
#include "motion.h"
#include "motionblupi.h"
#include "motionbot.h"
#include "motiontrax.h"
#include "motionperfo.h"
#include "motiongun.h"
#include "motionbird.h"
#include "motionptero.h"
#include "motionfish.h"
#include "motionsnake.h"
#include "motionsubm.h"
#include "motionjet.h"
#include "tasklist.h"
#include "modfile.h"
#include "auto.h"
#include "autojostle.h"
#include "autodoor.h"
#include "autodock.h"
#include "autocatapult.h"
#include "autofiole.h"
#include "autolift.h"
#include "autogoal.h"
#include "automax1x.h"
#include "autoflash.h"
#include "autoscrap.h"
#include "automeca.h"
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

#define BOX_ADOWN		0.0f

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
	CBotVar		*pVar, *pSub;
	ObjectType	type;
	D3DVECTOR	pos;
	float		value;

	if ( object == 0 )  return;

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
//?	pVar->SetValFloat(360.0f-Mod(pos.y*180.0f/PI, 360.0f));
	pVar->SetValFloat(360.0f-NormAngle(pos.y)*180.0f/PI);
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
	value = 0.0f;
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
	m_undo        = (CMainUndo*)m_iMan->SearchInstance(CLASS_UNDO);
	m_sound       = (CSound*)m_iMan->SearchInstance(CLASS_SOUND);
	m_motion      = 0;
	m_taskList    = 0;
	m_auto        = 0;
	m_runScript   = 0;

	m_type = OBJECT_FIX;
	m_id = ++g_id;
	m_option = 0;
	m_linVibration   = D3DVECTOR(0.0f, 0.0f, 0.0f);
	m_cirVibration   = D3DVECTOR(0.0f, 0.0f, 0.0f);
	m_cirChoc        = D3DVECTOR(0.0f, 0.0f, 0.0f);
	m_inclinaison    = D3DVECTOR(0.0f, 0.0f, 0.0f);
	m_lastPosTerrain = D3DVECTOR(NAN, NAN, NAN);
	m_bAdjustShadow  = TRUE;
	m_bTerrainHole   = FALSE;
	m_lastParticule  = 0.0f;

	for ( i=0 ; i<10 ; i++ )
	{
		m_additional[i] = 0;
	}

	m_power = 0;
	m_fret  = 0;
	m_truck = 0;
	m_truckLink  = 0;
	m_shield     = 1.0f;
	m_burnShield = 1.0f;
	m_strong = 0.0f;
	m_futurStrong = 0.0f;
	m_transparency = 0.0f;
	m_bHilite = FALSE;
	m_bSelect = FALSE;
	m_bSelectable = TRUE;
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
	m_bExplo    = FALSE;
	m_bNoUndoable = FALSE;
	m_bBurn     = FALSE;
	m_bDead     = FALSE;
	m_bFlat     = FALSE;
	m_magnifyDamage = 1.0f;
	m_param = 0.0f;
	m_flashTime = 0.0f;
	m_flashDelay = 0.0f;
	m_flashPart = 0;
	m_generation = -1;

	m_arrowMode = 0;  // flèches n'importe comment
	m_arrowPos = 8.0f;

	ZeroMemory(&m_character, sizeof(Character));
	m_character.mass = 1000.0f;

	m_cameraType = CAMERA_BACK;
	m_cameraDist = 50.0f;
	m_bCameraLock = FALSE;

	for ( i=0 ; i<OBJECTMAXPART ; i++ )
	{
		m_objectPart[i].bUsed = FALSE;
	}
	m_totalPart = 0;

	for ( i=0 ; i<OBJECTMAXCMDLINE ; i++ )
	{
		m_cmdLine[i] = NAN;
	}

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

	delete m_motion;
	delete m_taskList;
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

	if ( m_fret != 0 )
	{
		m_fret->SetTruck(0);
	}
	if ( m_truck != 0 )
	{
		m_truck->SetFret(0);
	}

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
	}

	m_type = OBJECT_NULL;  // objet invalide jusqu'à destruction complète

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
			m_engine->DeleteObject(m_objectPart[i].objRank);

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
	if ( m_motion != 0 )
	{
		m_motion->DeleteObject();
		delete m_motion;
		m_motion = 0;
	}

	if ( m_taskList != 0 )
	{
		delete m_taskList;
		m_taskList = 0;
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

	if ( type == EXPLO_BURN )
	{
		if ( m_type == OBJECT_MOBILEtg ||
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

	shield = RetShield();
	shield -= loss;
	if ( shield < 0.0f )  shield = 0.0f;  // mort
	SetShield(shield);

	if ( shield > 0.0f )  // pas encore mort ?
	{
		pyroType = PT_SHOTT;
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
			else
			{
				pyroType = PT_BURNT;
				SetBurn(TRUE);
			}
		}
		else	// explosion ?
		{
			if ( m_type == OBJECT_BULLET )
			{
				pyroType = PT_FRAGO;
			}
			else if ( m_type == OBJECT_DOOR1    ||
					  m_type == OBJECT_DOOR2    ||
					  m_type == OBJECT_DOOR3    ||
					  m_type == OBJECT_DOOR4    ||
					  m_type == OBJECT_DOOR5    ||
					  m_type == OBJECT_DOCK     ||
					  m_type == OBJECT_CATAPULT ||
					  m_type == OBJECT_START    ||
					  m_type == OBJECT_END      )  // batiment ?
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

	if ( shield > 0.0f )  return FALSE;  // pas encore mort

	if ( RetSelect() )
	{
		SetSelect(FALSE);  // désélectionne l'objet
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
			 m_type == OBJECT_TNT     )  // (*)
		{
			m_botVar->SetUserPtr(OBJECTDELETED);
		}
	}

	return TRUE;
}

// (*)	Si un robot ou le cosmonaute meurt, l'objet doit continuer
//		d'exister, pour que les programmes des fourmis continuent
//		de fonctionner comme si de rien était !


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
		speedx = 0.0f;
		speedy = 0.0f;

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


// Initialise une nouvelle partie.

void CObject::InitPart(int part)
{
	m_objectPart[part].bUsed      = TRUE;
	m_objectPart[part].bHide      = FALSE;
	m_objectPart[part].objRank    = -1;
	m_objectPart[part].parentPart = -1;

	m_objectPart[part].position   = D3DVECTOR(0.0f, 0.0f, 0.0f);
	m_objectPart[part].angle.y    = 0.0f;
	m_objectPart[part].angle.x    = 0.0f;
	m_objectPart[part].angle.z    = 0.0f;
	m_objectPart[part].zoom       = D3DVECTOR(1.0f, 1.0f, 1.0f);

	m_objectPart[part].bVarTex    = FALSE;
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
	m_engine->DeleteObject(m_objectPart[part].objRank);
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
	m_objectPart[part].objRank = objRank;
}

// Retourne le numéro d'une partie.

int CObject::RetObjectRank(int part)
{
	if ( !m_objectPart[part].bUsed )  return -1;
	return m_objectPart[part].objRank;
}

// Spécifie quel est le parent d'ue partie.
// Rappel: la partie 0 est toujours le père de tous et donc la
// partie principale (par exemple le chassis d'une voiture).

void CObject::SetObjectParent(int part, int parent)
{
	m_objectPart[part].parentPart = parent;
}

// Spécifie si une partie est visible ou non.

void CObject::SetHide(int part, BOOL bHide)
{
	int		i;

	if ( part == -1 )
	{
		for ( i=0 ; i<m_totalPart ; i++ )
		{
			if ( !m_objectPart[i].bUsed )  continue;

			m_objectPart[i].bHide = bHide;
			m_engine->SetObjectHide(m_objectPart[i].objRank, bHide);
		}
	}
	else
	{
		if ( m_objectPart[part].bUsed )
		{
			m_objectPart[part].bHide = bHide;
			m_engine->SetObjectHide(m_objectPart[part].objRank, bHide);
		}
	}
}

BOOL CObject::RetHide(int part)
{
	return m_objectPart[part].bHide;
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

// Choix de l'option à utiliser.

void CObject::SetOption(int option)
{
	m_option = option;
}

int CObject::RetOption()
{
	return m_option;
}

// Choix de l'accessaoire.

void CObject::SetAdditional(int type, int add)
{
	m_additional[type] = add;
}

int CObject::RetAdditional(int type)
{
	return m_additional[type];
}


// Rend un objet indétectable.

void CObject::NoDetect()
{
	int		i, objRank;

	for ( i=0 ; i<OBJECTMAXPART ; i++ )
	{
		if ( m_objectPart[i].bUsed )
		{
			objRank = m_objectPart[i].objRank;
			m_engine->SetObjectDetect(objRank, FALSE);  // indétectable
		}
	}
}

// Choix de la variante de texture d'une partie.

void CObject::SetVarTex(int part)
{
	int		i;

	for ( i=0 ; i<OBJECTMAXPART ; i++ )
	{
		SetVarTex(i, FALSE);
	}

	if ( part != -1 )
	{
		SetVarTex(part, TRUE);
	}
}

// Choix de la variante de texture d'une partie.
// Permet de faire apparaître en jaune les boutons survolés.

void CObject::SetVarTex(int part, BOOL bVar)
{
	float			sign, offset;
	int				rank;
	char			texName1[50];

	if ( m_objectPart[part].bVarTex == bVar )  return;

	m_objectPart[part].bVarTex = bVar;
	sign = bVar?1.0f:-1.0f;

	offset = NAN;

	if ( (m_type >= OBJECT_BOX1 && m_type <= OBJECT_BOX20) ||
		 (m_type >= OBJECT_KEY1 && m_type <= OBJECT_KEY5 ) )
	{
		strcpy(texName1, "box.tga");
		if ( part == 1 )  offset = 10.0f/256.0f;
		if ( part == 2 )  offset = 42.0f/256.0f;
		if ( part == 3 )  offset = 10.0f/256.0f;
		if ( part == 4 )  offset = 42.0f/256.0f;
		if ( part == 5 )  offset = 18.0f/256.0f;
		if ( part == 6 )  offset = 32.0f/256.0f;
		if ( part == 7 )  offset = 18.0f/256.0f;
		if ( part == 8 )  offset = 32.0f/256.0f;
	}

	if ( m_type == OBJECT_DOCK )
	{
		strcpy(texName1, "object2.tga");
		if ( part == 4 )  offset = 32.0f/256.0f;
		if ( part == 5 )  offset = 32.0f/256.0f;
		if ( part == 6 )  offset = 37.0f/256.0f;
		if ( part == 7 )  offset = 16.0f/256.0f;
		if ( part == 8 )  offset = 16.0f/256.0f;
	}

	if ( m_type == OBJECT_CATAPULT )
	{
		strcpy(texName1, "catapult.tga");
		if ( part == 1 )  offset = 32.0f/256.0f;
	}

	if ( m_type == OBJECT_TRAX  ||
		 m_type == OBJECT_PERFO ||
		 m_type == OBJECT_GUN   )
	{
		strcpy(texName1, "trax.tga");
		if ( part == 3 )  offset = 64.0f/256.0f;
		if ( part == 4 )  offset = 64.0f/256.0f;
		if ( part == 5 )  offset = 64.0f/256.0f;
	}

	if ( offset == NAN )  return;

	rank = m_objectPart[part].objRank;
	offset *= sign;
	m_engine->AddTextureMapping(rank, texName1, "",
								0.0f, 1000000.0f, D3DMAPPINGX, offset);
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


// Bloque une zone correspondant à un obstacle fixe.

void CObject::CreateLockZone(int dx, int dz, LockZone type)
{
	D3DMATRIX*	mat;
	D3DVECTOR	pos;

	mat = RetWorldMatrix(0);
	pos = Transform(*mat, D3DVECTOR(8.0f*dx, 0.0f, 8.0f*dz));

	if ( type == LZ_TUNNELh ||
		 type == LZ_TUNNELv )
	{
		m_terrain->SetLockZone(pos, type, TRUE);
	}
	else
	{
		m_terrain->SetLockZone(pos, type, FALSE);
	}
}


// Positionne un objet à une certaine hauteur par-rapport au sol.

void CObject::SetFloorHeight(float height)
{
	D3DVECTOR	pos;

	if ( m_type != OBJECT_BARRIER29 &&  // tuyaux ?
		 m_type != OBJECT_BARRIER39 &&  // tuyaux ?
		 m_type != OBJECT_BARRIER48 &&  // tuyaux ?
		 m_type != OBJECT_BARRIER49 &&  // tuyaux ?
		 m_type != OBJECT_BARRIER67 )   // tuyaux ?
	{
		pos = m_objectPart[0].position;
		m_terrain->MoveOnFloor(pos);

		m_objectPart[0].position.y = pos.y+height+m_character.height;
		m_objectPart[0].bTranslate = TRUE;  // il faudra recalculer les matrices
	}
	else
	{
		if ( !m_main->RetEdit() )  // détectable si édition !
		{
			NoDetect();
		}
	}

	// Tous les objets en dessous du sol (y<0) sont rendus
	// indétectable (c'est des éléments du décor morts), sauf
	// blupi qui arrive avec le lift (donc y<0).
	if ( pos.y < 0.0f &&  // objet en dessous du sol ?
		 m_type != OBJECT_BLUPI &&
		 m_type != OBJECT_MAX1X )
	{
		if ( !m_main->RetEdit() )  // détectable si édition !
		{
			NoDetect();
		}
	}

	// Les caisses en dessous du sol (y<0) sont transformées
	// en sol (comme lorsqu'elles tombent dans un trou).
	if ( pos.y < 0.0f &&  // objet en dessous du sol ?
		 (m_type == OBJECT_BOX1 ||
		  m_type == OBJECT_BOX2 ||
		  m_type == OBJECT_BOX3 ||
		  m_type == OBJECT_BOX4 ||
		  m_type == OBJECT_BOX5 ||
		  m_type == OBJECT_BOX6 ||
		  m_type == OBJECT_BOX9 ) )
	{
		m_terrain->SetResource(pos, TR_BOX);
		SetLock(TRUE);
		SetDead(TRUE);
		TerrainEmbedded();  // incruste dans le terrain
	}
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
	D3DVECTOR	shPos;
	float		height;
	int			rank;

	m_objectPart[part].position = pos;
	m_objectPart[part].bTranslate = TRUE;  // il faudra recalculer les matrices

	if ( part == 0 && !m_bFlat && m_truck == 0 )  // partie principale ?
	{
		rank = m_objectPart[0].objRank;

		shPos = pos;

		if ( m_type == OBJECT_DOCK )
		{
			shPos.y -= m_character.height;
		}
		else
		{
			if ( m_bAdjustShadow )
			{
				m_terrain->MoveOnFloor(shPos);
			}
			else
			{
				shPos.y = 0.0f;
			}
		}

		m_engine->SetObjectShadowPos(rank, shPos);

		height = pos.y-shPos.y;
		m_engine->SetObjectShadowHeight(rank, height);

		if ( m_bTerrainHole && m_bAdjustShadow )
		{
			shPos = Grid(shPos, 8.0f);
			if ( shPos.x != m_lastPosTerrain.x ||
				 shPos.z != m_lastPosTerrain.z )
			{
				AdjustShadow(shPos);
			}
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
		m_engine->SetObjectShadowAngle(m_objectPart[0].objRank, a);
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
		m_engine->SetObjectShadowAngle(m_objectPart[0].objRank, a);
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
	m_engine->SetObjectShadowHide(m_objectPart[0].objRank, (m_truck != 0));
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
			m_engine->SetDrawWorld(m_objectPart[i].objRank, bDraw);
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
			m_engine->SetDrawFront(m_objectPart[i].objRank, bDraw);
		}
	}
}


// Crée un objet quelconque.

BOOL CObject::CreateObject(D3DVECTOR pos, float angle, float zoom,
						   float height, ObjectType type, int option,
						   int addHat, int addGlass,
						   int addGlove, int addShoe, int addBag)
{
	SetOption(option);
	SetAdditional(ADD_HAT,   addHat);
	SetAdditional(ADD_GLASS, addGlass);
	SetAdditional(ADD_GLOVE, addGlove);
	SetAdditional(ADD_SHOE,  addShoe);
	SetAdditional(ADD_BAG,   addBag);

	if ( (type >= OBJECT_BUILDING1 && type <= OBJECT_BUILDING10) ||
		 (type >= OBJECT_CARCASS1  && type <= OBJECT_CARCASS10 ) ||
		 (type >= OBJECT_ORGA1     && type <= OBJECT_ORGA10    ) ||
		 type == OBJECT_DOOR1     ||
		 type == OBJECT_DOOR2     ||
		 type == OBJECT_DOOR3     ||
		 type == OBJECT_DOOR4     ||
		 type == OBJECT_DOOR5     ||
		 type == OBJECT_DOCK      ||
		 type == OBJECT_CATAPULT  ||
		 type == OBJECT_START     ||
		 type == OBJECT_END       )
	{
		CreateBuilding(pos, angle, zoom, height, type);

		if ( m_auto != 0 )
		{
			m_auto->Init();
		}
	}
	else
	if ( type == OBJECT_FRET        ||
		 type == OBJECT_STONE       ||
		 type == OBJECT_URANIUM     ||
		 type == OBJECT_METAL       ||
		 type == OBJECT_BARREL      ||
		 type == OBJECT_BARRELa     ||
		 type == OBJECT_ATOMIC      ||
		 type == OBJECT_BULLET      ||
		 type == OBJECT_BBOX        ||
		 type == OBJECT_TNT         ||
		 type == OBJECT_MINE        ||
		 type == OBJECT_WAYPOINT    ||
		 type == OBJECT_SHOW        ||
		 type == OBJECT_WINFIRE     ||
		 type == OBJECT_MARK        ||
		 type == OBJECT_FIOLE       ||
		 type == OBJECT_GLU         ||
		 type == OBJECT_LIFT        ||
		 type == OBJECT_GOAL        ||
		 type == OBJECT_COLUMN1     ||
		 type == OBJECT_COLUMN2     ||
		 type == OBJECT_COLUMN3     ||
		 type == OBJECT_COLUMN4     ||
		 type == OBJECT_GLASS1      ||
		 type == OBJECT_GLASS2      ||
		 type == OBJECT_SCRAP0      ||
		 type == OBJECT_SCRAP1      ||
		 type == OBJECT_SCRAP2      ||
		 type == OBJECT_SCRAP3      ||
		 type == OBJECT_SCRAP4      ||
		 type == OBJECT_SCRAP5      ||
		 type == OBJECT_SCRAP6      ||
		 type == OBJECT_SCRAP7      ||
		 type == OBJECT_SCRAP8      ||
		 type == OBJECT_SCRAP9      )
	{
		CreateResource(pos, angle, zoom, type);
	}
	else
	if ( type == OBJECT_BLUPI )
	{
		SetOption(option);
		SetAdditional(ADD_HAT,   addHat);
		SetAdditional(ADD_GLASS, addGlass);
		SetAdditional(ADD_GLOVE, addGlove);
		SetAdditional(ADD_SHOE,  addShoe);
		SetAdditional(ADD_BAG,   addBag);
		CreateBlupi(pos, angle, zoom, type);
	}
	else
	if ( type == OBJECT_BLUPI   ||
		 type == OBJECT_BOT1    ||
		 type == OBJECT_BOT2    ||
		 type == OBJECT_BOT3    ||
		 type == OBJECT_BOT4    ||
		 type == OBJECT_BOT5    ||
		 type == OBJECT_CARROT  ||
		 type == OBJECT_WALKER  ||
		 type == OBJECT_CRAZY   )
	{
		SetOption(option);
		CreateBot(pos, angle, zoom, type);
	}
	else
	if ( type == OBJECT_BIRD  ||
		 type == OBJECT_PTERO ||
		 type == OBJECT_FISH  ||
		 type == OBJECT_SNAKE ||
		 type == OBJECT_SUBM  ||
		 type == OBJECT_JET   )
	{
		SetOption(option);
		CreateAnimal(pos, angle, zoom, type);
	}
	else
	if ( type == OBJECT_MAX1X )
	{
		SetOption(option);
		CreateSpecial(pos, angle, zoom, type);
	}
	else
	if ( type >= OBJECT_BARRIER0  &&
		 type <= OBJECT_BARRIER99 )
	{
		CreateBarrier(pos, angle, zoom, height, type);
	}
	else
	if ( (type >= OBJECT_BOX1  &&
		  type <= OBJECT_BOX20 ) ||
		 (type >= OBJECT_KEY1  &&
		  type <= OBJECT_KEY5  ) )
	{
		CreateBox(pos, angle, zoom, height, type);
	}
	else
	if ( type >= OBJECT_GROUND0  &&
		 type <= OBJECT_GROUND19 )
	{
		SetOption(option);
		CreateGround(pos, angle, type);
	}
	else
	if ( type == OBJECT_STONE1  ||
		 type == OBJECT_STONE2  ||
		 type == OBJECT_STONE3  ||
		 type == OBJECT_STONE4  ||
		 type == OBJECT_STONE5  ||
		 type == OBJECT_STONE6  ||
		 type == OBJECT_STONE7  ||
		 type == OBJECT_STONE8  ||
		 type == OBJECT_STONE9  ||
		 type == OBJECT_STONE10 )
	{
		CreateStone(pos, angle, zoom, height, type);
	}
	else
	if ( type == OBJECT_PLANT0  ||
		 type == OBJECT_PLANT1  ||
		 type == OBJECT_PLANT2  ||
		 type == OBJECT_PLANT3  ||
		 type == OBJECT_PLANT4  ||
		 type == OBJECT_PLANT5  ||
		 type == OBJECT_PLANT6  ||
		 type == OBJECT_PLANT7  ||
		 type == OBJECT_PLANT8  ||
		 type == OBJECT_PLANT9  ||
		 type == OBJECT_PLANT10 ||
		 type == OBJECT_PLANT11 ||
		 type == OBJECT_PLANT12 ||
		 type == OBJECT_PLANT13 ||
		 type == OBJECT_PLANT14 ||
		 type == OBJECT_PLANT15 ||
		 type == OBJECT_PLANT16 ||
		 type == OBJECT_PLANT17 ||
		 type == OBJECT_PLANT18 ||
		 type == OBJECT_PLANT19 ||
		 type == OBJECT_TREE0   ||
		 type == OBJECT_TREE1   ||
		 type == OBJECT_TREE2   ||
		 type == OBJECT_TREE3   ||
		 type == OBJECT_TREE4   ||
		 type == OBJECT_TREE5   ||
		 type == OBJECT_TREE6   ||
		 type == OBJECT_TREE7   ||
		 type == OBJECT_TREE8   ||
		 type == OBJECT_TREE9   )
	{
		CreatePlant(pos, angle, zoom, height, type);
	}
	else
	if ( type == OBJECT_MUSHROOM0 ||
		 type == OBJECT_MUSHROOM1 ||
		 type == OBJECT_MUSHROOM2 ||
		 type == OBJECT_MUSHROOM3 ||
		 type == OBJECT_MUSHROOM4 ||
		 type == OBJECT_MUSHROOM5 ||
		 type == OBJECT_MUSHROOM6 ||
		 type == OBJECT_MUSHROOM7 ||
		 type == OBJECT_MUSHROOM8 ||
		 type == OBJECT_MUSHROOM9 )
	{
		CreateMushroom(pos, angle, zoom, height, type);
	}
	else
	if ( type == OBJECT_QUARTZ0 ||
		 type == OBJECT_QUARTZ1 ||
		 type == OBJECT_QUARTZ2 ||
		 type == OBJECT_QUARTZ3 ||
		 type == OBJECT_QUARTZ4 ||
		 type == OBJECT_QUARTZ5 ||
		 type == OBJECT_QUARTZ6 ||
		 type == OBJECT_QUARTZ7 ||
		 type == OBJECT_QUARTZ8 ||
		 type == OBJECT_QUARTZ9 )
	{
		CreateQuartz(pos, angle, zoom, height, type);
	}
	else
	if ( type == OBJECT_ROOT0 ||
		 type == OBJECT_ROOT1 ||
		 type == OBJECT_ROOT2 ||
		 type == OBJECT_ROOT3 ||
		 type == OBJECT_ROOT4 ||
		 type == OBJECT_ROOT5 ||
		 type == OBJECT_ROOT6 ||
		 type == OBJECT_ROOT7 ||
		 type == OBJECT_ROOT8 ||
		 type == OBJECT_ROOT9 )
	{
		CreateRoot(pos, angle, zoom, height, type);
	}
	else
	if ( type == OBJECT_RUINmobilew1 ||
		 type == OBJECT_RUINmobilew2 ||
		 type == OBJECT_RUINmobilet1 ||
		 type == OBJECT_RUINmobilet2 ||
		 type == OBJECT_RUINmobiler1 ||
		 type == OBJECT_RUINmobiler2 ||
		 type == OBJECT_RUINfactory  ||
		 type == OBJECT_RUINdoor     ||
		 type == OBJECT_RUINsupport  ||
		 type == OBJECT_RUINradar    ||
		 type == OBJECT_RUINconvert  ||
		 type == OBJECT_RUINbase     ||
		 type == OBJECT_RUINhead     )
	{
		CreateRuin(pos, angle, zoom, height, type);
	}
	else
	if ( type == OBJECT_TRAX  ||
		 type == OBJECT_PERFO ||
		 type == OBJECT_GUN   )
	{
		SetOption(option);
		CreateVehicle(pos, angle, zoom, type);
	}

	return TRUE;
}


// Crée un véhicule roulant quelconque posé sur le sol.

BOOL CObject::CreateVehicle(D3DVECTOR pos, float angle, float zoom,
							ObjectType type)
{
	m_type = type;

	SetZoom(0, zoom);

	if ( type == OBJECT_TRAX )
	{
		m_motion = new CMotionTrax(m_iMan, this);
		if ( m_motion == 0 )  return FALSE;
	}
	if ( type == OBJECT_PERFO )
	{
		m_motion = new CMotionPerfo(m_iMan, this);
		if ( m_motion == 0 )  return FALSE;
	}
	if ( type == OBJECT_GUN )
	{
		m_motion = new CMotionGun(m_iMan, this);
		if ( m_motion == 0 )  return FALSE;
	}

	m_taskList = new CTaskList(m_iMan, this);
	if ( m_taskList == 0 )  return FALSE;

	if ( !m_motion->Create(pos, angle, type) )
	{
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

// Crée l'ombre sous un objet.

BOOL CObject::CreateShadow(float radius, float intensity,
						   D3DShadowType type, BOOL bTerrainHole,
						   float sunFactor)
{
	D3DVECTOR	pos;
	float		zoom, angle;
	char		bHole[9];

	m_bTerrainHole = bTerrainHole;

	zoom = RetZoomX(0);

	angle = m_objectPart[0].angle.y;
	if ( m_type >= OBJECT_BARRIER92 &&
		 m_type <= OBJECT_BARRIER99 )
	{
		angle = Mod(angle, PI);
	}

	m_engine->ShadowCreate(m_objectPart[0].objRank);
	m_engine->SetObjectShadowRadius(m_objectPart[0].objRank, radius*zoom);
	m_engine->SetObjectShadowIntensity(m_objectPart[0].objRank, intensity);
	m_engine->SetObjectShadowHeight(m_objectPart[0].objRank, 0.0f);
	m_engine->SetObjectShadowSunFactor(m_objectPart[0].objRank, sunFactor);
	m_engine->SetObjectShadowAngle(m_objectPart[0].objRank, angle);
	m_engine->SetObjectShadowType(m_objectPart[0].objRank, type);

	if ( m_bTerrainHole )
	{
		pos = Grid(RetPosition(0), 8.0f);
		AdjustShadow(pos);
	}
	else
	{
		bHole[0] = 0;
		bHole[1] = 0;
		bHole[2] = 0;
		bHole[3] = 0;
		bHole[4] = 0;
		bHole[5] = 0;
		bHole[6] = 0;
		bHole[7] = 0;
		bHole[8] = 0;
		m_engine->SetObjectShadowHole(m_objectPart[0].objRank, RetPosition(0), bHole);
	}

	return TRUE;
}

// Ajuste l'ombre sous un objet, en fonction des trous dans le terrain.

void CObject::AdjustShadow(D3DVECTOR pos)
{
	char		bHole[9];

	m_lastPosTerrain = pos;

	pos = m_lastPosTerrain;
	pos.x -= 8.0f;
	pos.z += 8.0f;
	bHole[0] = !m_terrain->IsSolid(pos);

	pos = m_lastPosTerrain;
	pos.z += 8.0f;
	bHole[1] = !m_terrain->IsSolid(pos);

	pos = m_lastPosTerrain;
	pos.x += 8.0f;
	pos.z += 8.0f;
	bHole[2] = !m_terrain->IsSolid(pos);

	pos = m_lastPosTerrain;
	pos.x -= 8.0f;
	bHole[3] = !m_terrain->IsSolid(pos);

	pos = m_lastPosTerrain;
	bHole[4] = !m_terrain->IsSolid(pos);

	pos = m_lastPosTerrain;
	pos.x += 8.0f;
	bHole[5] = !m_terrain->IsSolid(pos);

	pos = m_lastPosTerrain;
	pos.x -= 8.0f;
	pos.z -= 8.0f;
	bHole[6] = !m_terrain->IsSolid(pos);

	pos = m_lastPosTerrain;
	pos.z -= 8.0f;
	bHole[7] = !m_terrain->IsSolid(pos);

	pos = m_lastPosTerrain;
	pos.x += 8.0f;
	pos.z -= 8.0f;
	bHole[8] = !m_terrain->IsSolid(pos);

	m_engine->SetObjectShadowHole(m_objectPart[0].objRank, m_lastPosTerrain, bHole);
}

// Spécifie si les ombres doivent être ajustées au terrain lorsque
// l'objet bouge.

void CObject::SetAdjustShadow(BOOL bAdjust)
{
	m_bAdjustShadow = bAdjust;
}

BOOL CObject::RetAdjustShadow()
{
	return m_bAdjustShadow;
}

// Crée un batiment quelconque posé sur le sol.

BOOL CObject::CreateBuilding(D3DVECTOR pos, float angle, float zoom,
							 float height, ObjectType type)
{
	CModFile*	pModFile;
	FPOINT		p;
	int			rank;

	if ( m_engine->RetRestCreate() < 20 )  return FALSE;

	pModFile = new CModFile(m_iMan);

	SetType(type);

	rank = m_engine->CreateObject();
	m_engine->SetObjectType(rank, TYPEOBJECT);  // c'est un objet fixe
	SetObjectRank(0, rank);

	if ( m_type == OBJECT_BUILDING1 )  // bâtiment ?
	{
		pModFile->ReadModel("objects\\buildin1.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);
	}

	if ( m_type == OBJECT_BUILDING2 )  // bâtiment ?
	{
		pModFile->ReadModel("objects\\buildin2.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);
	}

	if ( m_type == OBJECT_BUILDING3 )  // bâtiment ?
	{
		pModFile->ReadModel("objects\\buildin3.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);
	}

	if ( m_type == OBJECT_CARCASS1 )  // carcasse ?
	{
		pModFile->ReadModel("objects\\carcass1.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);
		SetZoom(0, 1.4f);

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

		CreateShadow(7.0f, 0.2f);
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
	}

	if ( m_type == OBJECT_ORGA2 )  // organique ?
	{
		pModFile->ReadModel("objects\\orga2.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);
		SetZoom(0, zoom);
	}

	if ( m_type == OBJECT_ORGA3 )  // organique ?
	{
		pModFile->ReadModel("objects\\orga3.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);
		SetZoom(0, zoom);
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

		CreateLockZone( 0, 0, LZ_FIX);
		CreateLockZone(-1, 0, LZ_FIX);
		CreateLockZone( 1, 0, LZ_FIX);

		CreateShadow(12.0f, 0.5f, D3DSHADOWNORM, TRUE, 2.0f);
	}

	if ( m_type == OBJECT_DOOR3 )
	{
		pModFile->ReadModel("objects\\door30.mod");
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

		CreateLockZone( 0, 0, LZ_FIX);
		CreateLockZone(-1, 0, LZ_FIX);
		CreateLockZone( 1, 0, LZ_FIX);

		CreateShadow(12.0f, 0.5f, D3DSHADOWNORM, TRUE, 2.0f);
	}

	if ( m_type == OBJECT_DOOR2 )
	{
		pModFile->ReadModel("objects\\door20.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);

		// porte gauche
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(1, rank);
		SetObjectParent(1, 0);
		pModFile->ReadModel("objects\\door21.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(1, D3DVECTOR(-5.0f, 0.0f, -1.0f));

		// porte droite
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(2, rank);
		SetObjectParent(2, 0);
		pModFile->ReadModel("objects\\door22.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(2, D3DVECTOR(5.0f, 0.0f, -1.0f));

		CreateLockZone( 0, 0, LZ_FIX);
		CreateLockZone(-1, 0, LZ_FIX);
		CreateLockZone( 1, 0, LZ_FIX);

		CreateShadow(12.0f, 0.5f, D3DSHADOWNORM, TRUE, 2.0f);
	}

	if ( m_type == OBJECT_DOOR4 )
	{
		pModFile->ReadModel("objects\\door40.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);

		// porte gauche
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(1, rank);
		SetObjectParent(1, 0);
		pModFile->ReadModel("objects\\door21.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(1, D3DVECTOR(-5.0f, 0.0f, -1.0f));

		// porte droite
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(2, rank);
		SetObjectParent(2, 0);
		pModFile->ReadModel("objects\\door22.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(2, D3DVECTOR(5.0f, 0.0f, -1.0f));

		CreateLockZone( 0, 0, LZ_FIX);
		CreateLockZone(-1, 0, LZ_FIX);
		CreateLockZone( 1, 0, LZ_FIX);

		CreateShadow(12.0f, 0.5f, D3DSHADOWNORM, TRUE, 2.0f);
	}

	if ( m_type == OBJECT_DOCK )
	{
		pModFile->ReadModel("objects\\dock0.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);

		// Portique
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(1, rank);
		SetObjectParent(1, 0);
		pModFile->ReadModel("objects\\dock1.mod");
		pModFile->CreateEngineObject(rank);

		// Charriot
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(2, rank);
		SetObjectParent(2, 1);
		pModFile->ReadModel("objects\\dock2.mod");
		pModFile->CreateEngineObject(rank);

		// Piston
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(3, rank);
		SetObjectParent(3, 2);
		pModFile->ReadModel("objects\\dock3.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(3, D3DVECTOR(0.0f, 10.0f, 0.0f));

		// ^
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(4, rank);
		SetObjectParent(4, 1);
		pModFile->ReadModel("objects\\dock4.mod");
		pModFile->CreateEngineObject(rank);

		// v
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(5, rank);
		SetObjectParent(5, 1);
		pModFile->ReadModel("objects\\dock5.mod");
		pModFile->CreateEngineObject(rank);

		// piston
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(6, rank);
		SetObjectParent(6, 1);
		pModFile->ReadModel("objects\\dock6.mod");
		pModFile->CreateEngineObject(rank);

		// <
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(7, rank);
		SetObjectParent(7, 1);
		pModFile->ReadModel("objects\\dock7.mod");
		pModFile->CreateEngineObject(rank);

		// >
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(8, rank);
		SetObjectParent(8, 1);
		pModFile->ReadModel("objects\\dock8.mod");
		pModFile->CreateEngineObject(rank);

		// reste
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(9, rank);
		SetObjectParent(9, 1);
		pModFile->ReadModel("objects\\dock9.mod");
		pModFile->CreateEngineObject(rank);

		CreateLockZone(-4, -3, LZ_FIX);
		CreateLockZone(-4, -2, LZ_FIX);
		CreateLockZone(-4, -1, LZ_FIX);
		CreateLockZone(-4,  0, LZ_FIX);
		CreateLockZone(-4,  1, LZ_FIX);
		CreateLockZone(-4,  2, LZ_FIX);
		CreateLockZone(-4,  3, LZ_FIX);

		CreateLockZone( 4, -3, LZ_FIX);
		CreateLockZone( 4, -2, LZ_FIX);
		CreateLockZone( 4, -1, LZ_FIX);
		CreateLockZone( 4,  0, LZ_FIX);
		CreateLockZone( 4,  1, LZ_FIX);
		CreateLockZone( 4,  2, LZ_FIX);
		CreateLockZone( 4,  3, LZ_FIX);

		CreateShadow(5.0f, 1.0f, D3DSHADOWDOCK, FALSE, 0.0f);  // cible

		m_arrowPos = 0.0f;
	}

	if ( m_type == OBJECT_CATAPULT )
	{
		pModFile->ReadModel("objects\\cata0.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(1, rank);
		SetObjectParent(1, 0);
		pModFile->ReadModel("objects\\cata1.mod");  // bouton
		pModFile->CreateEngineObject(rank);

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(2, rank);
		SetObjectParent(2, 0);
		pModFile->ReadModel("objects\\cata2.mod");  // marteau
		pModFile->CreateEngineObject(rank);
		SetPosition(2, D3DVECTOR(0.0f, 14.0f, 0.0f));

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(3, rank);
		SetObjectParent(3, 0);
		pModFile->ReadModel("objects\\cata3.mod");  // moteur
		pModFile->CreateEngineObject(rank);
		SetPosition(3, D3DVECTOR(0.0f, 8.0f, 0.0f));

		CreateLockZone(0, -1, LZ_FIX);
		CreateLockZone(0,  1, LZ_FIX);

		m_arrowPos = 0.0f;
	}

	if ( m_type == OBJECT_START )
	{
		pModFile->ReadModel("objects\\start.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);
	}

	if ( m_type == OBJECT_END )
	{
		pModFile->ReadModel("objects\\end.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);
	}

	pos = RetPosition(0);
	pos.y = 0.0f;
	pos.y += height;
	SetPosition(0, pos);  // pour afficher les ombres tout de suite

	CreateAuto(type);

	delete pModFile;
	return TRUE;
}

// Crée une petite ressource posée sur le sol.

BOOL CObject::CreateResource(D3DVECTOR pos, float angle, float zoom,
							 ObjectType type)
{
	CModFile*		pModFile;
	D3DTypeObj		typeObj;
	D3DShadowType	sType;
	char			name[50];
	int				rank, i;
	float			radius, height, density;

	if ( type != OBJECT_SHOW )
	{
		if ( m_engine->RetRestCreate() < 1 )  return FALSE;
	}

	pModFile = new CModFile(m_iMan);

	SetType(type);

	typeObj = TYPEOBJECT;
	if ( type == OBJECT_LIFT )
	{
		typeObj = TYPETERRAIN;
	}
	if ( type == OBJECT_FIOLE  ||
		 type == OBJECT_GLU    ||
		 type == OBJECT_GOAL   ||
		 type == OBJECT_MINE   ||
		 type == OBJECT_GLASS1 )
	{
		typeObj = TYPEMETAL;
	}

	rank = m_engine->CreateObject();
	m_engine->SetObjectType(rank, typeObj);  // c'est un objet fixe
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
	if ( type == OBJECT_TNT         )  strcpy(name, "objects\\tnt.mod");
	if ( type == OBJECT_MINE        )  strcpy(name, "objects\\mine.mod");
	if ( type == OBJECT_WAYPOINT    )  strcpy(name, "objects\\waypoint.mod");
	if ( type == OBJECT_SHOW        )  strcpy(name, "objects\\show.mod");
	if ( type == OBJECT_WINFIRE     )  strcpy(name, "objects\\winfire.mod");
	if ( type == OBJECT_MARK        )  strcpy(name, "objects\\mark.mod");
	if ( type == OBJECT_FIOLE       )  strcpy(name, "objects\\fiole.mod");
	if ( type == OBJECT_GLU         )  strcpy(name, "objects\\glu.mod");
	if ( type == OBJECT_LIFT        )  strcpy(name, "objects\\lift.mod");
	if ( type == OBJECT_GOAL        )  strcpy(name, "objects\\goal.mod");
	if ( type == OBJECT_COLUMN1     )  strcpy(name, "objects\\column1a.mod");
	if ( type == OBJECT_COLUMN2     )  strcpy(name, "objects\\column2a.mod");
	if ( type == OBJECT_COLUMN3     )  strcpy(name, "objects\\column3a.mod");
	if ( type == OBJECT_COLUMN4     )  strcpy(name, "objects\\column4a.mod");
	if ( type == OBJECT_GLASS1      )  strcpy(name, "objects\\glass1.mod");
	if ( type == OBJECT_GLASS2      )  strcpy(name, "objects\\glass2.mod");
	if ( type == OBJECT_SCRAP0      )  strcpy(name, "objects\\scrap0.mod");
	if ( type == OBJECT_SCRAP1      )  strcpy(name, "objects\\scrap1.mod");
	if ( type == OBJECT_SCRAP2      )  strcpy(name, "objects\\scrap2.mod");
	if ( type == OBJECT_SCRAP3      )  strcpy(name, "objects\\scrap3.mod");
	if ( type == OBJECT_SCRAP4      )  strcpy(name, "objects\\scrap4.mod");
	if ( type == OBJECT_SCRAP5      )  strcpy(name, "objects\\scrap5.mod");
	if ( type == OBJECT_SCRAP6      )  strcpy(name, "objects\\scrap6.mod");
	if ( type == OBJECT_SCRAP7      )  strcpy(name, "objects\\scrap7.mod");
	if ( type == OBJECT_SCRAP8      )  strcpy(name, "objects\\scrap8.mod");
	if ( type == OBJECT_SCRAP9      )  strcpy(name, "objects\\scrap9.mod");

	if ( type == OBJECT_GLASS1 ||
		 type == OBJECT_GLASS2 )
	{
		angle = Mod(angle, PI);
	}

	pModFile->ReadModel(name);
	pModFile->CreateEngineObject(rank);
	SetPosition(0, pos);
	SetAngleY(0, angle);
	SetZoom(0, zoom);

	if ( type == OBJECT_FIOLE )
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(1, rank);
		SetObjectParent(1, 0);
		pModFile->ReadModel("objects\\fiolea.mod");
		pModFile->CreateEngineObject(rank);
		m_arrowPos = 0.0f;
	}

	if ( type == OBJECT_GLU )
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(1, rank);
		SetObjectParent(1, 0);
		pModFile->ReadModel("objects\\glua.mod");
		pModFile->CreateEngineObject(rank);
		m_arrowPos = 0.0f;
	}

	if ( type == OBJECT_LIFT )
	{
		CreateAuto(type);
		delete pModFile;
		return TRUE;
	}

	if ( type == OBJECT_GOAL )
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(1, rank);
		SetObjectParent(1, 0);
		pModFile->ReadModel("objects\\goala.mod");
		pModFile->CreateEngineObject(rank);
		m_arrowPos = 0.0f;
	}

	if ( type == OBJECT_GLASS1 )
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(1, rank);
		SetObjectParent(1, 0);
		pModFile->ReadModel("objects\\glass11.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(1, D3DVECTOR(0.0f, 10.5f, 0.0f));
	}
	if ( type == OBJECT_GLASS2 )
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(1, rank);
		SetObjectParent(1, 0);
		pModFile->ReadModel("objects\\glass21.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(1, D3DVECTOR(0.0f, 10.5f, 0.0f));
	}

	if ( type >= OBJECT_COLUMN1 &&
		 type <= OBJECT_COLUMN4 )
	{
		for ( i=0 ; i<3 ; i++ )
		{
			pos.y -= 200.0f;
			rank = m_engine->CreateObject();
			m_engine->SetObjectType(rank, TYPEDESCENDANT);
			SetObjectRank(1+i, rank);
			SetObjectParent(1+i, 0);
			sprintf(name, "objects\\column%db.mod", type-OBJECT_COLUMN1+1);
			pModFile->ReadModel(name);
			pModFile->CreateEngineObject(rank);
			SetPosition(1+i, pos);
		}
		NoDetect();
		delete pModFile;
		return TRUE;
	}

	if ( type == OBJECT_SHOW )  // reste en l'air ?
	{
		delete pModFile;
		return TRUE;
	}

	radius  = 1.5f;
	density = 1.0f;
	height  = 0.0f;
	sType   = D3DSHADOWNORM;

	if ( type == OBJECT_WAYPOINT    )
	{
	}
	else if ( type == OBJECT_MINE )
	{
		CreateLockZone(0, 0, LZ_MINE);
		radius = 3.0f;
	}
	else if ( type == OBJECT_MARK )
	{
		radius =  10.0f;
		density = 0.6f;
	}
	else if ( type == OBJECT_BARREL  ||
			  type == OBJECT_BARRELa )
	{
		CreateLockZone(0, 0, LZ_FIX);
		radius  = 3.0f;
		density = 0.6f;
		m_character.mass = 1500.0f;
	}
	else if ( type == OBJECT_FIOLE ||
			  type == OBJECT_GLU   )
	{
		CreateLockZone(0, 0, LZ_FIOLE);
	}
	else if ( type == OBJECT_GLASS1 ||
			  type == OBJECT_GLASS2 )
	{
		CreateLockZone(0, 0, LZ_GLASS);
		radius  = 4.0f;
		density = 1.0f;
		sType   = D3DSHADOWGLASS;
	}
	else if ( type == OBJECT_LIFT  ||
			  type == OBJECT_GOAL  )
	{
		radius  = 0.0f;
		density = 0.0f;
	}
	else if ( type >= OBJECT_SCRAP0 &&
			  type <= OBJECT_SCRAP9 )
	{
		radius  = 0.0f;
		density = 0.0f;
	}
	else
	{
		CreateLockZone(0, 0, LZ_FIX);
	}

	if ( radius != 0.0f )
	{
		CreateShadow(radius, density, sType, TRUE, 1.0f);
	}

	SetFloorHeight(0.0f);
	CreateAuto(type);

	pos = RetPosition(0);
	pos.y += height;
	SetPosition(0, pos);  // pour afficher les ombres tout de suite

	delete pModFile;
	return TRUE;
}

// Crée un blupi posé sur le sol.

BOOL CObject::CreateBlupi(D3DVECTOR pos, float angle, float zoom,
						  ObjectType type)
{
	CAuto*		pAuto;

	m_type = type;

	SetZoom(0, zoom);

	m_motion = new CMotionBlupi(m_iMan, this);
	if ( m_motion == 0 )  return FALSE;

	m_taskList = new CTaskList(m_iMan, this);
	if ( m_taskList == 0 )  return FALSE;

	if ( !m_motion->Create(pos, angle, type) )
	{
		if ( m_motion != 0 )
		{
			m_motion->DeleteObject();
			delete m_motion;
			m_motion = 0;
		}
		return FALSE;
	}

	if ( m_option == 1 )
	{
		CObject*	lift;

		lift = new CObject(m_iMan);
		lift->CreateResource(pos, 0.0f, 1.0f, OBJECT_LIFT);

		if ( m_main->RetEdit() )
		{
			pAuto = lift->RetAuto();
			if ( pAuto != 0 )
			{
				pAuto->Start(1);
			}
		}
	}

	return TRUE;
}

// Crée un robot posé sur le sol.

BOOL CObject::CreateBot(D3DVECTOR pos, float angle, float zoom,
						ObjectType type)
{
	m_type = type;

	SetZoom(0, zoom);

	if ( type == OBJECT_BOT1    ||
		 type == OBJECT_BOT2    ||
		 type == OBJECT_BOT3    ||
		 type == OBJECT_BOT4    ||
		 type == OBJECT_BOT5    ||
		 type == OBJECT_CARROT  ||
		 type == OBJECT_WALKER  ||
		 type == OBJECT_CRAZY   )
	{
		m_motion = new CMotionBot(m_iMan, this);
		if ( m_motion == 0 )  return FALSE;

		m_taskList = new CTaskList(m_iMan, this);
		if ( m_taskList == 0 )  return FALSE;

		if ( !m_motion->Create(pos, angle, type) )
		{
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
		if ( !m_motion->Create(pos, angle, type) )  return FALSE;
	}

	return TRUE;
}

// Crée un animal.

BOOL CObject::CreateAnimal(D3DVECTOR pos, float angle, float zoom,
						   ObjectType type)
{
	m_type = type;

	SetZoom(0, zoom);

	if ( m_type == OBJECT_BIRD )
	{
		m_motion = new CMotionBird(m_iMan, this);
		if ( m_motion == 0 )  return FALSE;
		if ( !m_motion->Create(pos, angle, type) )  return FALSE;
	}
	if ( m_type == OBJECT_PTERO )
	{
		m_motion = new CMotionPtero(m_iMan, this);
		if ( m_motion == 0 )  return FALSE;
		if ( !m_motion->Create(pos, angle, type) )  return FALSE;
	}
	if ( m_type == OBJECT_FISH )
	{
		m_motion = new CMotionFish(m_iMan, this);
		if ( m_motion == 0 )  return FALSE;
		if ( !m_motion->Create(pos, angle, type) )  return FALSE;
	}
	if ( m_type == OBJECT_SNAKE )
	{
		m_motion = new CMotionSnake(m_iMan, this);
		if ( m_motion == 0 )  return FALSE;
		if ( !m_motion->Create(pos, angle, type) )  return FALSE;
	}
	if ( m_type == OBJECT_SUBM )
	{
		m_motion = new CMotionSubm(m_iMan, this);
		if ( m_motion == 0 )  return FALSE;
		if ( !m_motion->Create(pos, angle, type) )  return FALSE;
	}
	if ( m_type == OBJECT_JET )
	{
		m_motion = new CMotionJet(m_iMan, this);
		if ( m_motion == 0 )  return FALSE;
		if ( !m_motion->Create(pos, angle, type) )  return FALSE;
	}

	NoDetect();
	return TRUE;
}

// Crée un bidule spécial.

BOOL CObject::CreateSpecial(D3DVECTOR pos, float angle, float zoom,
							ObjectType type)
{
	CModFile*	pModFile;
	FPOINT		p;
	int			rank;

	if ( m_engine->RetRestCreate() < 10 )  return FALSE;

	pModFile = new CModFile(m_iMan);

	SetType(type);

	rank = m_engine->CreateObject();
	m_engine->SetObjectType(rank, TYPEOBJECT);  // c'est un objet fixe
	SetObjectRank(0, rank);

	if ( m_type == OBJECT_MAX1X )
	{
		pModFile->ReadModel("objects\\max1x0.mod");  // 2 portes fermées
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetFloorHeight(0.0f);

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(1, rank);
		SetObjectParent(1, 0);
		pModFile->ReadModel("objects\\max1x1.mod");  // porte gauche
		pModFile->CreateEngineObject(rank);
		SetPosition(1, D3DVECTOR(-4.0f, 0.0f, 0.0f));
		SetHide(1, TRUE);

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(2, rank);
		SetObjectParent(2, 0);
		pModFile->ReadModel("objects\\max1x2.mod");  // porte droite
		pModFile->CreateEngineObject(rank);
		SetPosition(2, D3DVECTOR(4.0f, 0.0f, 0.0f));
		SetHide(2, TRUE);

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(3, rank);
		SetObjectParent(3, 0);
		pModFile->ReadModel("objects\\max1x3.mod");  // support 1
		pModFile->CreateEngineObject(rank);
		SetPosition(3, D3DVECTOR(0.0f, -12.5f, 0.0f));
		SetHide(3, TRUE);

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(4, rank);
		SetObjectParent(4, 3);
		pModFile->ReadModel("objects\\max1x3.mod");  // support 2
		pModFile->CreateEngineObject(rank);
		SetPosition(4, D3DVECTOR(0.0f, 1.7f, 0.0f));
		SetHide(4, TRUE);

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(5, rank);
		SetObjectParent(5, 4);
		pModFile->ReadModel("objects\\max1x3.mod");  // support 3
		pModFile->CreateEngineObject(rank);
		SetPosition(5, D3DVECTOR(0.0f, 1.7f, 0.0f));
		SetHide(5, TRUE);

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(6, rank);
		SetObjectParent(6, 5);
		pModFile->ReadModel("objects\\max1x4.mod");  // clown
		pModFile->CreateEngineObject(rank);
		SetPosition(6, D3DVECTOR(0.0f, 1.7f, 0.0f));
		SetHide(6, TRUE);

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(7, rank);
		SetObjectParent(7, 0);
		pModFile->ReadModel("objects\\max1x5.mod");  // fond
		pModFile->CreateEngineObject(rank);
		SetPosition(7, D3DVECTOR(0.0f, 0.0f, 0.0f));
		SetHide(7, TRUE);

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(8, rank);
		SetObjectParent(8, 0);
		pModFile->ReadModel("objects\\max1x6.mod");  // parois
		pModFile->CreateEngineObject(rank);
		SetPosition(8, D3DVECTOR(0.0f, 0.0f, 0.0f));
		m_engine->SetObjectDetect(rank, FALSE);

		CreateLockZone(0, 0, LZ_MAX1X);
	}

	pos = RetPosition(0);
	pos.y = 0.0f;
	SetPosition(0, pos);  // pour afficher les ombres tout de suite

	CreateAuto(type);

	delete pModFile;
	return TRUE;
}

// Crée une barrière posée sur le sol.

BOOL CObject::CreateBarrier(D3DVECTOR pos, float angle, float zoom,
							float height, ObjectType type)
{
	CModFile*		pModFile;
	float			radius, intensity, sunFactor;
	D3DShadowType	sType;
	LockZone		lz;
	int				rank;
	char			text[100];

	if ( m_engine->RetRestCreate() < 1 )  return FALSE;

	pModFile = new CModFile(m_iMan);

	SetType(type);

	radius = 5.0f;
	intensity = 1.0f;
	sunFactor = 1.0f;
	sType = D3DSHADOWSQUARE;

	if ( type >= OBJECT_BARRIER0  &&
		 type <= OBJECT_BARRIER99 )
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEOBJECT);
		SetObjectRank(0, rank);
		sprintf(text, "objects\\barrier%d.mod", type-OBJECT_BARRIER0);
		pModFile->ReadModel(text);
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);

		if ( type == OBJECT_BARRIER40 )  // gros engrenage ?
		{
			rank = m_engine->CreateObject();
			m_engine->SetObjectType(rank, TYPEDESCENDANT);
			SetObjectRank(1, rank);
			SetObjectParent(1, 0);
			pModFile->ReadModel("objects\\barrier401.mod");
			pModFile->CreateEngineObject(rank);
			SetPosition(1, D3DVECTOR(0.0f, 6.0f, 0.0f));
		}

		if ( type == OBJECT_BARRIER41 )  // gros engrenage ?
		{
			rank = m_engine->CreateObject();
			m_engine->SetObjectType(rank, TYPEDESCENDANT);
			SetObjectRank(1, rank);
			SetObjectParent(1, 0);
			pModFile->ReadModel("objects\\barrier411.mod");
			pModFile->CreateEngineObject(rank);
			SetPosition(1, D3DVECTOR(0.0f, 6.0f, 0.0f));

			rank = m_engine->CreateObject();
			m_engine->SetObjectType(rank, TYPEDESCENDANT);
			SetObjectRank(2, rank);
			SetObjectParent(2, 0);
			pModFile->ReadModel("objects\\barrier412.mod");
			pModFile->CreateEngineObject(rank);
			SetPosition(2, D3DVECTOR(0.0f, 11.0f, 0.0f));
		}

		if ( type == OBJECT_BARRIER42 )  // gros axe ?
		{
			rank = m_engine->CreateObject();
			m_engine->SetObjectType(rank, TYPEDESCENDANT);
			SetObjectRank(1, rank);
			SetObjectParent(1, 0);
			pModFile->ReadModel("objects\\barrier421.mod");
			pModFile->CreateEngineObject(rank);
			SetPosition(1, D3DVECTOR(0.0f, 6.0f, 0.0f));
		}

		if ( type == OBJECT_BARRIER46 )  // chateau avec drapeau ?
		{
			rank = m_engine->CreateObject();
			m_engine->SetObjectType(rank, TYPEDESCENDANT);
			SetObjectRank(1, rank);
			SetObjectParent(1, 0);
			pModFile->ReadModel("objects\\barrier461.mod");
			pModFile->CreateEngineObject(rank);
			SetPosition(1, D3DVECTOR(0.0f, 19.0f, 0.0f));

			rank = m_engine->CreateObject();
			m_engine->SetObjectType(rank, TYPEDESCENDANT);
			SetObjectRank(2, rank);
			SetObjectParent(2, 1);
			pModFile->ReadModel("objects\\barrier462.mod");
			pModFile->CreateEngineObject(rank);
			SetPosition(2, D3DVECTOR(2.0f, 0.0f, 0.0f));

			rank = m_engine->CreateObject();
			m_engine->SetObjectType(rank, TYPEDESCENDANT);
			SetObjectRank(3, rank);
			SetObjectParent(3, 2);
			pModFile->ReadModel("objects\\barrier463.mod");
			pModFile->CreateEngineObject(rank);
			SetPosition(3, D3DVECTOR(2.0f, 0.0f, 0.0f));

			rank = m_engine->CreateObject();
			m_engine->SetObjectType(rank, TYPEDESCENDANT);
			SetObjectRank(4, rank);
			SetObjectParent(4, 3);
			pModFile->ReadModel("objects\\barrier464.mod");
			pModFile->CreateEngineObject(rank);
			SetPosition(4, D3DVECTOR(2.0f, 0.0f, 0.0f));
		}

		if ( type == OBJECT_BARRIER58 )  // futura étuve ?
		{
			rank = m_engine->CreateObject();
			m_engine->SetObjectType(rank, TYPEDESCENDANT);
			SetObjectRank(1, rank);
			SetObjectParent(1, 0);
			pModFile->ReadModel("objects\\barrier581.mod");
			pModFile->CreateEngineObject(rank);
			SetPosition(1, D3DVECTOR(0.0f, 6.0f, 0.0f));
		}

		if ( type == OBJECT_BARRIER62 )  // futura canon ?
		{
			rank = m_engine->CreateObject();
			m_engine->SetObjectType(rank, TYPEDESCENDANT);
			SetObjectRank(1, rank);
			SetObjectParent(1, 0);
			pModFile->ReadModel("objects\\barrier621.mod");
			pModFile->CreateEngineObject(rank);
			SetPosition(1, D3DVECTOR(0.0f, 3.0f, 0.0f));

			rank = m_engine->CreateObject();
			m_engine->SetObjectType(rank, TYPEDESCENDANT);
			SetObjectRank(2, rank);
			SetObjectParent(2, 1);
			pModFile->ReadModel("objects\\barrier622.mod");
			pModFile->CreateEngineObject(rank);
			SetPosition(2, D3DVECTOR(0.0f, 5.0f, 0.0f));
		}

		if ( type == OBJECT_BARRIER77 )  // labo parabole ?
		{
			rank = m_engine->CreateObject();
			m_engine->SetObjectType(rank, TYPEDESCENDANT);
			SetObjectRank(1, rank);
			SetObjectParent(1, 0);
			pModFile->ReadModel("objects\\barrier771.mod");
			pModFile->CreateEngineObject(rank);
			SetPosition(1, D3DVECTOR(0.0f, 7.0f, 0.0f));

			rank = m_engine->CreateObject();
			m_engine->SetObjectType(rank, TYPEDESCENDANT);
			SetObjectRank(2, rank);
			SetObjectParent(2, 1);
			pModFile->ReadModel("objects\\barrier772.mod");
			pModFile->CreateEngineObject(rank);
			SetPosition(2, D3DVECTOR(0.0f, 4.0f, -0.7f));
		}

		if ( type == OBJECT_BARRIER1 )  // barrière unreal ?
		{
			radius = 2.0f;
			intensity = 0.4f;
		}
		if ( type == OBJECT_BARRIER2 )
		{
			radius = 4.0f;
		}
		if ( type >= OBJECT_BARRIER18 &&
			 type <= OBJECT_BARRIER22 )  // cinéma ?
		{
			intensity = 0.2f;
		}
		if ( type >= OBJECT_BARRIER50 &&
			 type <= OBJECT_BARRIER57 )  // rochers ?
		{
			intensity = 0.3f;
		}
		if ( type == OBJECT_BARRIER65 )  // futura barrière ?
		{
			radius = 4.0f;
			intensity = 0.1f;
		}
		if ( type == OBJECT_BARRIER66 )  // trésor ?
		{
			radius = 4.0f;
			intensity = 0.5f;
		}
		if ( type >= OBJECT_BARRIER92 &&
			 type <= OBJECT_BARRIER99 )  // palissade ?
		{
			radius  = 4.0f;
			intensity = 1.0f;
			sType = D3DSHADOWPALISSADE;
		}

		if ( type == OBJECT_BARRIER0  )  sunFactor = 1.5f;
		if ( type == OBJECT_BARRIER1  )  sunFactor = 0.5f;
		if ( type == OBJECT_BARRIER2  )  sunFactor = 0.5f;
		if ( type == OBJECT_BARRIER3  )  sunFactor = 0.5f;
		if ( type == OBJECT_BARRIER4  )  sunFactor = 1.5f;
		if ( type == OBJECT_BARRIER5  )  sunFactor = 1.5f;
		if ( type == OBJECT_BARRIER6  )  sunFactor = 2.0f;
		if ( type == OBJECT_BARRIER7  )  sunFactor = 0.3f;
		if ( type == OBJECT_BARRIER8  )  sunFactor = 0.6f;
		if ( type == OBJECT_BARRIER9  )  sunFactor = 0.6f;
		if ( type == OBJECT_BARRIER10 )  sunFactor = 1.0f;
		if ( type == OBJECT_BARRIER11 )  sunFactor = 1.0f;
		if ( type == OBJECT_BARRIER12 )  sunFactor = 2.0f;
		if ( type == OBJECT_BARRIER13 )  sunFactor = 0.6f;
		if ( type == OBJECT_BARRIER14 )  sunFactor = 0.9f;
		if ( type == OBJECT_BARRIER15 )  sunFactor = 0.6f;
		if ( type == OBJECT_BARRIER16 )  sunFactor = 0.7f;
		if ( type == OBJECT_BARRIER17 )  sunFactor = 0.3f;
		if ( type == OBJECT_BARRIER18 )  sunFactor = 1.5f;
		if ( type == OBJECT_BARRIER19 )  sunFactor = 1.0f;
		if ( type == OBJECT_BARRIER20 )  sunFactor = 1.1f;
		if ( type == OBJECT_BARRIER21 )  sunFactor = 2.0f;
		if ( type == OBJECT_BARRIER22 )  sunFactor = 1.7f;
		if ( type == OBJECT_BARRIER23 )  sunFactor = 1.3f;
		if ( type == OBJECT_BARRIER24 )  sunFactor = 1.3f;
		if ( type == OBJECT_BARRIER25 )  sunFactor = 1.3f;
		if ( type == OBJECT_BARRIER26 )  sunFactor = 1.6f;
		if ( type == OBJECT_BARRIER27 )  sunFactor = 0.3f;
		if ( type == OBJECT_BARRIER28 )  sunFactor = 1.3f;
		if ( type == OBJECT_BARRIER29 )  sunFactor = 0.0f;
		if ( type == OBJECT_BARRIER30 )  sunFactor = 1.2f;
		if ( type == OBJECT_BARRIER31 )  sunFactor = 0.8f;
		if ( type == OBJECT_BARRIER32 )  sunFactor = 0.6f;
		if ( type == OBJECT_BARRIER33 )  sunFactor = 0.8f;
		if ( type == OBJECT_BARRIER34 )  sunFactor = 1.4f;
		if ( type == OBJECT_BARRIER35 )  sunFactor = 0.7f;
		if ( type == OBJECT_BARRIER36 )  sunFactor = 0.8f;
		if ( type == OBJECT_BARRIER37 )  sunFactor = 1.0f;
		if ( type == OBJECT_BARRIER38 )  sunFactor = 1.0f;
		if ( type == OBJECT_BARRIER39 )  sunFactor = 0.0f;
		if ( type == OBJECT_BARRIER40 )  sunFactor = 1.0f;
		if ( type == OBJECT_BARRIER41 )  sunFactor = 1.3f;
		if ( type == OBJECT_BARRIER42 )  sunFactor = 0.7f;
		if ( type == OBJECT_BARRIER43 )  sunFactor = 1.0f;
		if ( type == OBJECT_BARRIER44 )  sunFactor = 2.0f;
		if ( type == OBJECT_BARRIER45 )  sunFactor = 2.1f;
		if ( type == OBJECT_BARRIER46 )  sunFactor = 2.0f;
		if ( type == OBJECT_BARRIER47 )  sunFactor = 2.3f;
		if ( type == OBJECT_BARRIER48 )  sunFactor = 0.0f;
		if ( type == OBJECT_BARRIER49 )  sunFactor = 0.0f;
		if ( type == OBJECT_BARRIER50 )  sunFactor = 0.8f;
		if ( type == OBJECT_BARRIER51 )  sunFactor = 1.3f;
		if ( type == OBJECT_BARRIER52 )  sunFactor = 0.4f;
		if ( type == OBJECT_BARRIER53 )  sunFactor = 0.6f;
		if ( type == OBJECT_BARRIER54 )  sunFactor = 0.5f;
		if ( type == OBJECT_BARRIER55 )  sunFactor = 0.5f;
		if ( type == OBJECT_BARRIER56 )  sunFactor = 0.5f;
		if ( type == OBJECT_BARRIER57 )  sunFactor = 1.3f;
		if ( type == OBJECT_BARRIER58 )  sunFactor = 1.0f;
		if ( type == OBJECT_BARRIER59 )  sunFactor = 1.0f;
		if ( type == OBJECT_BARRIER60 )  sunFactor = 1.0f;
		if ( type == OBJECT_BARRIER61 )  sunFactor = 1.0f;
		if ( type == OBJECT_BARRIER62 )  sunFactor = 1.0f;
		if ( type == OBJECT_BARRIER63 )  sunFactor = 1.0f;
		if ( type == OBJECT_BARRIER64 )  sunFactor = 1.0f;
		if ( type == OBJECT_BARRIER65 )  sunFactor = 0.3f;
		if ( type == OBJECT_BARRIER66 )  sunFactor = 0.5f;
		if ( type == OBJECT_BARRIER67 )  sunFactor = 0.0f;
		if ( type == OBJECT_BARRIER68 )  sunFactor = 1.3f;
		if ( type == OBJECT_BARRIER69 )  sunFactor = 0.7f;
		if ( type == OBJECT_BARRIER70 )  sunFactor = 0.3f;
		if ( type == OBJECT_BARRIER71 )  sunFactor = 1.0f;
		if ( type == OBJECT_BARRIER72 )  sunFactor = 0.5f;
		if ( type == OBJECT_BARRIER73 )  sunFactor = 1.0f;
		if ( type == OBJECT_BARRIER74 )  sunFactor = 1.0f;
		if ( type == OBJECT_BARRIER75 )  sunFactor = 1.2f;
		if ( type == OBJECT_BARRIER76 )  sunFactor = 1.1f;
		if ( type == OBJECT_BARRIER77 )  sunFactor = 0.9f;
		if ( type == OBJECT_BARRIER78 )  sunFactor = 0.4f;
		if ( type == OBJECT_BARRIER79 )  sunFactor = 0.5f;
		if ( type == OBJECT_BARRIER80 )  sunFactor = 1.1f;
		if ( type == OBJECT_BARRIER81 )  sunFactor = 1.0f;
		if ( type == OBJECT_BARRIER82 )  sunFactor = 0.4f;
		if ( type == OBJECT_BARRIER83 )  sunFactor = 0.5f;
		if ( type == OBJECT_BARRIER84 )  sunFactor = 1.1f;
		if ( type == OBJECT_BARRIER85 )  sunFactor = 1.0f;
		if ( type == OBJECT_BARRIER86 )  sunFactor = 0.4f;
		if ( type == OBJECT_BARRIER87 )  sunFactor = 0.8f;
		if ( type == OBJECT_BARRIER88 )  sunFactor = 0.6f;
		if ( type == OBJECT_BARRIER89 )  sunFactor = 1.0f;
		if ( type == OBJECT_BARRIER90 )  sunFactor = 0.6f;
		if ( type == OBJECT_BARRIER91 )  sunFactor = 0.6f;
		if ( type == OBJECT_BARRIER92 )  sunFactor = 0.01f;
		if ( type == OBJECT_BARRIER93 )  sunFactor = 0.01f;
		if ( type == OBJECT_BARRIER94 )  sunFactor = 0.01f;
		if ( type == OBJECT_BARRIER95 )  sunFactor = 0.01f;
		if ( type == OBJECT_BARRIER96 )  sunFactor = 0.01f;
		if ( type == OBJECT_BARRIER97 )  sunFactor = 0.01f;
		if ( type == OBJECT_BARRIER98 )  sunFactor = 0.01f;
		if ( type == OBJECT_BARRIER99 )  sunFactor = 0.01f;

		radius *= 0.8f+((sunFactor-0.5f)/1.5f)*0.4f;
	}

	lz = LZ_FIX;
	if ( type == OBJECT_BARRIER24 ||  // tunnel ?
		 type == OBJECT_BARRIER57 ||  // tunnel ?
		 type == OBJECT_BARRIER65 )   // barrière ?
	{
		angle = Mod(angle, PI);
		if ( angle < PI*0.25f || angle > PI*0.75f )
		{
			lz = LZ_TUNNELh;
		}
		else
		{
			lz = LZ_TUNNELv;
		}
	}
#if 1
	if ( type == OBJECT_BARRIER29 ||  // tuyaux ?
		 type == OBJECT_BARRIER39 ||  // tuyaux ?
		 type == OBJECT_BARRIER48 ||  // tuyaux ?
		 type == OBJECT_BARRIER49 ||  // tuyaux ?
		 type == OBJECT_BARRIER67 )   // tuyaux ?
	{
		lz = LZ_PIPE;
	}
#endif
	CreateLockZone(0, 0, lz);

	if ( sunFactor != 0.0f )
	{
		CreateShadow(radius, intensity, sType, TRUE, sunFactor);
	}

	pos = RetPosition(0);
	SetPosition(0, pos);  // pour afficher les ombres tout de suite

	SetFloorHeight(0.0f);
	CreateAuto(type);

	pos = RetPosition(0);
	pos.y += height;
	SetPosition(0, pos);

	delete pModFile;
	return TRUE;
}

// Crée une caisse posée sur le sol.

BOOL CObject::CreateBox(D3DVECTOR pos, float angle, float zoom,
						float height, ObjectType type)
{
	CModFile*	pModFile;
	float		radius, sunFactor;
	int			rank;
	char		text[100];
	BOOL		bSquare;

	if ( m_engine->RetRestCreate() < 1 )  return FALSE;

	pModFile = new CModFile(m_iMan);

	SetType(type);
	radius = 4.5f;
	sunFactor = 1.0f;
	bSquare = TRUE;

	if ( type >= OBJECT_BOX1  &&
		 type <= OBJECT_BOX20 )
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEOBJECT);
		SetObjectRank(0, rank);
		sprintf(text, "objects\\box%d.mod", type-OBJECT_BOX1+1);
		pModFile->ReadModel(text);
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);

		m_arrowPos = 8.0f;
		if ( type == OBJECT_BOX8 )  // colonne ?
		{
			m_arrowPos = 5.0f;
			sunFactor = 2.0f;
		}
	}

	if ( type == OBJECT_BOX7 )  // sphère ?
	{
		m_taskList = new CTaskList(m_iMan, this);
		if ( m_taskList == 0 )  return FALSE;

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(9, rank);
		SetObjectParent(9, 0);
		pModFile->ReadModel("objects\\box7b.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(9, D3DVECTOR(0.0f, 4.0f, 0.0f));

		m_arrowPos = 6.5f;
		bSquare = FALSE;
		radius = 4.0f;
	}

	if ( type == OBJECT_BOX10 )  // bombe ?
	{
		m_taskList = new CTaskList(m_iMan, this);
		if ( m_taskList == 0 )  return FALSE;

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(9, rank);
		SetObjectParent(9, 0);
		pModFile->ReadModel("objects\\box10b.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(9, D3DVECTOR(0.0f, 4.0f, 0.0f));

		m_arrowPos = 5.8f;
		bSquare = FALSE;
		radius = 4.0f;
	}

	if ( type == OBJECT_BOX11 )  // caisse x-x ?
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(10, rank);
		SetObjectParent(10, 0);
		pModFile->ReadModel("objects\\box11w.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(10, D3DVECTOR(2.5f, 1.4f, 3.0f));

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(11, rank);
		SetObjectParent(11, 0);
		pModFile->ReadModel("objects\\box11w.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(11, D3DVECTOR(-2.5f, 1.4f, 3.0f));

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(12, rank);
		SetObjectParent(12, 0);
		pModFile->ReadModel("objects\\box11w.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(12, D3DVECTOR(2.5f, 1.4f, -3.0f));

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(13, rank);
		SetObjectParent(13, 0);
		pModFile->ReadModel("objects\\box11w.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(13, D3DVECTOR(-2.5f, 1.4f, -3.0f));
	}

	if ( type == OBJECT_BOX12 )  // caisse z-z ?
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(10, rank);
		SetObjectParent(10, 0);
		pModFile->ReadModel("objects\\box12w.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(10, D3DVECTOR(3.0f, 1.4f, 2.5f));

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(11, rank);
		SetObjectParent(11, 0);
		pModFile->ReadModel("objects\\box12w.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(11, D3DVECTOR(-3.0f, 1.4f, 2.5f));

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(12, rank);
		SetObjectParent(12, 0);
		pModFile->ReadModel("objects\\box12w.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(12, D3DVECTOR(3.0f, 1.4f, -2.5f));

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(13, rank);
		SetObjectParent(13, 0);
		pModFile->ReadModel("objects\\box12w.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(13, D3DVECTOR(-3.0f, 1.4f, -2.5f));
	}

	if ( type == OBJECT_BOX13 )  // cca ?
	{
		m_taskList = new CTaskList(m_iMan, this);
		if ( m_taskList == 0 )  return FALSE;
	}

	if ( type >= OBJECT_KEY1 &&
		 type <= OBJECT_KEY5 )
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEOBJECT);
		SetObjectRank(0, rank);
		sprintf(text, "objects\\key%d.mod", type-OBJECT_KEY1+1);
		pModFile->ReadModel(text);
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);

		m_arrowPos = 4.0f;
		bSquare = FALSE;
	}

	if ( type != OBJECT_BOX12 )  // pas caisse z-z ?
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(1, rank);
		SetObjectParent(1, 0);
		pModFile->ReadModel("objects\\boxa.mod");  // ouest
		pModFile->CreateEngineObject(rank);

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(3, rank);
		SetObjectParent(3, 0);
		pModFile->ReadModel("objects\\boxc.mod");  // est
		pModFile->CreateEngineObject(rank);
	}

	if ( type != OBJECT_BOX11 )  // pas caisse x-x ?
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(2, rank);
		SetObjectParent(2, 0);
		pModFile->ReadModel("objects\\boxb.mod");  // nord
		pModFile->CreateEngineObject(rank);

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(4, rank);
		SetObjectParent(4, 0);
		pModFile->ReadModel("objects\\boxd.mod");  // sud
		pModFile->CreateEngineObject(rank);
	}

	if ( type != OBJECT_BOX7  &&  // pas sphère ?
		 type != OBJECT_BOX8  &&  // pas colonne ?
		 type != OBJECT_BOX10 &&  // pas bombe ?
		 type != OBJECT_BOX12 &&  // pas caisse z-z ?
		 type != OBJECT_BOX13 &&  // pas cca ?
		 m_main->RetHandleMove() > 0 )
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(5, rank);
		SetObjectParent(5, 0);
		pModFile->ReadModel("objects\\boxe.mod");  // ouest
		pModFile->CreateEngineObject(rank);

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(7, rank);
		SetObjectParent(7, 0);
		pModFile->ReadModel("objects\\boxg.mod");  // est
		pModFile->CreateEngineObject(rank);
	}

	if ( type != OBJECT_BOX7  &&  // pas sphère ?
		 type != OBJECT_BOX8  &&  // pas colonne ?
		 type != OBJECT_BOX10 &&  // pas bombe ?
		 type != OBJECT_BOX11 &&  // pas caisse x-x ?
		 type != OBJECT_BOX13 &&  // pas cca ?
		 m_main->RetHandleMove() > 0 )
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(6, rank);
		SetObjectParent(6, 0);
		pModFile->ReadModel("objects\\boxf.mod");  // nord
		pModFile->CreateEngineObject(rank);

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(8, rank);
		SetObjectParent(8, 0);
		pModFile->ReadModel("objects\\boxh.mod");  // sud
		pModFile->CreateEngineObject(rank);
	}

	CreateLockZone(0, 0, bSquare?LZ_BOX:LZ_BOXo);
	CreateShadow(radius, 1.0f, bSquare?D3DSHADOWSQUARE:D3DSHADOWNORM, TRUE, sunFactor);

	pos = RetPosition(0);
	SetPosition(0, pos);  // pour afficher les ombres tout de suite

	SetFloorHeight(0.0f);
	CreateAuto(type);

	pos = RetPosition(0);
	pos.y += height;
	SetPosition(0, pos);

	delete pModFile;
	return TRUE;
}

// Crée un sol spécial à niveau.

BOOL CObject::CreateGround(D3DVECTOR pos, float angle, ObjectType type)
{
	CModFile*	pModFile;
	int			rank;
	char		text[100];

	if ( m_engine->RetRestCreate() < 1 )  return FALSE;

	pModFile = new CModFile(m_iMan);
	SetType(type);

	if ( type >= OBJECT_GROUND0  &&
		 type <= OBJECT_GROUND19 )
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPETERRAIN);
		SetObjectRank(0, rank);
		sprintf(text, "objects\\ground%d.mod", type-OBJECT_GROUND0);
		pModFile->ReadModel(text);
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
	}

	if ( type == OBJECT_GROUND2 )
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		SetObjectRank(1, rank);
		SetObjectParent(1, 0);
		pModFile->ReadModel("objects\\ground2b.mod");
		pModFile->CreateEngineObject(rank);

		m_engine->SetObjectHide(RetObjectRank(1), m_option==0);
	}

	m_terrain->SetResource(pos, TR_BOX);

	delete pModFile;
	return TRUE;
}

// Crée une pierre posée sur le sol.

BOOL CObject::CreateStone(D3DVECTOR pos, float angle, float zoom,
						float height, ObjectType type)
{
	CModFile*	pModFile;
	int			rank;

	if ( m_engine->RetRestCreate() < 1 )  return FALSE;

	pModFile = new CModFile(m_iMan);

	SetType(type);

	if ( type == OBJECT_STONE1 )  // cube 1
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEOBJECT);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\stone1.mod");
		pModFile->CreateEngineObject(rank);
		SetZoom(0, zoom);
		SetPosition(0, pos);
		SetAngleY(0, angle);

		CreateLockZone(0, 0, LZ_FIX);
		CreateShadow(8.0f, 1.0f);
	}

	if ( type == OBJECT_STONE2 )  // cube 2
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEOBJECT);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\stone2.mod");
		pModFile->CreateEngineObject(rank);
		SetZoom(0, zoom);
		SetPosition(0, pos);
		SetAngleY(0, angle);

		CreateLockZone(0, 0, LZ_FIX);
		CreateShadow(10.0f, 1.0f);
	}

	if ( type == OBJECT_STONE3 )  // cube 3
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEOBJECT);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\stone3.mod");
		pModFile->CreateEngineObject(rank);
		SetZoom(0, zoom);
		SetPosition(0, pos);
		SetAngleY(0, angle);

		CreateLockZone(0, 0, LZ_FIX);
		CreateShadow(12.0f, 1.0f);
	}

	if ( type == OBJECT_STONE4 )  // cube 4
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEOBJECT);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\stone4.mod");
		pModFile->CreateEngineObject(rank);
		SetZoom(0, zoom);
		SetPosition(0, pos);
		SetAngleY(0, angle);

		CreateLockZone(0, 0, LZ_FIX);
		CreateShadow(18.0f, 1.0f);
	}

	if ( type == OBJECT_STONE5 )  // pilier h=30
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEOBJECT);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\stone5.mod");
		pModFile->CreateEngineObject(rank);
		SetZoom(0, zoom);
		SetPosition(0, pos);
		SetAngleY(0, angle);

		CreateLockZone(0, 0, LZ_FIX);
		CreateShadow(10.0f, 1.0f);
	}

	if ( type == OBJECT_STONE6 )  // dalle haute pour piliers espacés d=60
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEOBJECT);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\stone6.mod");
		pModFile->CreateEngineObject(rank);
		SetZoom(0, zoom);
		SetPosition(0, pos);
		SetAngleY(0, angle);
	}

	SetPosition(0, pos);  // pour afficher les ombres tout de suite

	SetFloorHeight(0.0f);
	CreateAuto(type);

	pos = RetPosition(0);
	pos.y += height;
	SetPosition(0, pos);

	delete pModFile;
	return TRUE;
}

// Crée une plante posée sur le sol.

BOOL CObject::CreatePlant(D3DVECTOR pos, float angle, float zoom,
						  float height, ObjectType type)
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
		m_engine->SetObjectType(rank, TYPEOBJECT);
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

		CreateLockZone(0, 0, LZ_FIX);
		CreateShadow(8.0f*zoom, 0.5f);
	}

	if ( type == OBJECT_PLANT5 ||
		 type == OBJECT_PLANT6 ||
		 type == OBJECT_PLANT7 ||
		 type == OBJECT_PLANT8 ||
		 type == OBJECT_PLANT9 )  // plante tombante ?
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEOBJECT);
		SetObjectRank(0, rank);
		if ( type == OBJECT_PLANT5 )  pModFile->ReadModel("objects\\plant5.mod");
		if ( type == OBJECT_PLANT6 )  pModFile->ReadModel("objects\\plant6.mod");
		if ( type == OBJECT_PLANT7 )  pModFile->ReadModel("objects\\plant7.mod");
		if ( type == OBJECT_PLANT8 )  pModFile->ReadModel("objects\\plant8.mod");
		if ( type == OBJECT_PLANT9 )  pModFile->ReadModel("objects\\plant9.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
	}

	if ( type == OBJECT_PLANT10 ||
		 type == OBJECT_PLANT11 ||
		 type == OBJECT_PLANT12 ||
		 type == OBJECT_PLANT13 ||
		 type == OBJECT_PLANT14 )  // plante grasse ?
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEOBJECT);
		SetObjectRank(0, rank);
		if ( type == OBJECT_PLANT10 )  pModFile->ReadModel("objects\\plant10.mod");
		if ( type == OBJECT_PLANT11 )  pModFile->ReadModel("objects\\plant11.mod");
		if ( type == OBJECT_PLANT12 )  pModFile->ReadModel("objects\\plant12.mod");
		if ( type == OBJECT_PLANT13 )  pModFile->ReadModel("objects\\plant13.mod");
		if ( type == OBJECT_PLANT14 )  pModFile->ReadModel("objects\\plant14.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);

		CreateLockZone(0, 0, LZ_FIX);
		CreateShadow(8.0f*zoom, 0.3f);
	}

	if ( type == OBJECT_PLANT15 ||
		 type == OBJECT_PLANT16 ||
		 type == OBJECT_PLANT17 ||
		 type == OBJECT_PLANT18 ||
		 type == OBJECT_PLANT19 )  // fougère ?
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEOBJECT);
		SetObjectRank(0, rank);
		if ( type == OBJECT_PLANT15 )  pModFile->ReadModel("objects\\plant15.mod");
		if ( type == OBJECT_PLANT16 )  pModFile->ReadModel("objects\\plant16.mod");
		if ( type == OBJECT_PLANT17 )  pModFile->ReadModel("objects\\plant17.mod");
		if ( type == OBJECT_PLANT18 )  pModFile->ReadModel("objects\\plant18.mod");
		if ( type == OBJECT_PLANT19 )  pModFile->ReadModel("objects\\plant19.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);

		CreateLockZone(0, 0, LZ_FIX);
		CreateShadow(8.0f*zoom, 0.5f);
	}

	if ( type == OBJECT_TREE0 )  // arbre ?
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEOBJECT);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\tree0.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);

		CreateLockZone(0, 0, LZ_FIX);
		CreateShadow(8.0f*zoom, 0.5f);
	}

	if ( type == OBJECT_TREE1 )  // arbre ?
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEOBJECT);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\tree1.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);

		CreateLockZone(0, 0, LZ_FIX);
		CreateShadow(8.0f*zoom, 0.5f);
	}

	if ( type == OBJECT_TREE2 )  // arbre ?
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEOBJECT);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\tree2.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);

		CreateLockZone(0, 0, LZ_FIX);
		CreateShadow(8.0f*zoom, 0.5f);
	}

	if ( type == OBJECT_TREE3 )  // arbre ?
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEOBJECT);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\tree3.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);

		CreateLockZone(0, 0, LZ_FIX);
		CreateShadow(8.0f*zoom, 0.5f);
	}

	if ( type == OBJECT_TREE4 )  // palmier ?
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEOBJECT);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\tree4.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);

		CreateLockZone(0, 0, LZ_FIX);
		CreateShadow(8.0f*zoom, 0.5f);
	}

	if ( type == OBJECT_TREE5 )  // palmier ?
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEOBJECT);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\tree5.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);

		CreateLockZone(0, 0, LZ_FIX);
		CreateShadow(8.0f*zoom, 0.5f);
	}

	if ( type == OBJECT_TREE6 )  // palmier ?
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEOBJECT);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\tree6.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);

		CreateLockZone(0, 0, LZ_FIX);
		CreateShadow(8.0f*zoom, 0.5f);
	}

	if ( type == OBJECT_TREE7 )  // palmier ?
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEOBJECT);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\tree7.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);

		CreateLockZone(0, 0, LZ_FIX);
		CreateShadow(8.0f*zoom, 0.5f);
	}

	SetZoom(0, zoom);

	pos = RetPosition(0);
	SetPosition(0, pos);  // pour afficher les ombres tout de suite

	SetFloorHeight(0.0f);
	CreateAuto(type);

	pos = RetPosition(0);
	pos.y += height;
	SetPosition(0, pos);

	delete pModFile;
	return TRUE;
}

// Crée un champignon posé sur le sol.

BOOL CObject::CreateMushroom(D3DVECTOR pos, float angle, float zoom,
							 float height, ObjectType type)
{
	CModFile*	pModFile;
	int			rank;

	if ( m_engine->RetRestCreate() < 1 )  return FALSE;

	pModFile = new CModFile(m_iMan);

	SetType(type);

	if ( type == OBJECT_MUSHROOM1 )
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEOBJECT);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\mush1.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);

		CreateLockZone(0, 0, LZ_FIX);

		CreateShadow(6.0f, 0.5f);
	}

	if ( type == OBJECT_MUSHROOM2 )
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEOBJECT);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\mush2.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);

		CreateLockZone(0, 0, LZ_FIX);

		CreateShadow(5.0f, 0.5f);
	}

	SetZoom(0, zoom);

	pos = RetPosition(0);
	SetPosition(0, pos);  // pour afficher les ombres tout de suite

	SetFloorHeight(0.0f);
	CreateAuto(type);

	pos = RetPosition(0);
	pos.y += height;
	SetPosition(0, pos);

	delete pModFile;
	return TRUE;
}

// Crée un quartz posé sur le sol.

BOOL CObject::CreateQuartz(D3DVECTOR pos, float angle, float zoom,
						   float height, ObjectType type)
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

		CreateLockZone(0, 0, LZ_FIX);

		CreateShadow(4.0f, 0.5f);
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

		CreateLockZone(0, 0, LZ_FIX);

		CreateShadow(5.0f, 0.5f);
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

		CreateLockZone(0, 0, LZ_FIX);

		CreateShadow(6.0f, 0.5f);
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

		CreateLockZone(0, 0, LZ_FIX);

		CreateShadow(10.0f, 0.5f);
	}

	SetZoom(0, zoom);

	pos = RetPosition(0);
	SetPosition(0, pos);  // pour afficher les ombres tout de suite

	SetFloorHeight(0.0f);
	CreateAuto(type);

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
						 float height, ObjectType type)
{
	CModFile*	pModFile;
	int			rank;

	if ( m_engine->RetRestCreate() < 1 )  return FALSE;

	pModFile = new CModFile(m_iMan);

	SetType(type);

	if ( type == OBJECT_ROOT0 )
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEOBJECT);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\root0.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetZoom(0, 2.0f*zoom);

		CreateLockZone(0, 0, LZ_FIX);

		CreateShadow(16.0f, 0.5f);
	}
	if ( type == OBJECT_ROOT1 )
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEOBJECT);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\root1.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetZoom(0, 2.0f*zoom);

		CreateLockZone(0, 0, LZ_FIX);

		CreateShadow(16.0f, 0.5f);
	}
	if ( type == OBJECT_ROOT2 )
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEOBJECT);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\root2.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetZoom(0, 2.0f*zoom);

		CreateLockZone(0, 0, LZ_FIX);

		CreateShadow(16.0f, 0.5f);
	}
	if ( type == OBJECT_ROOT3 )
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEOBJECT);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\root3.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetZoom(0, 2.0f*zoom);

		CreateLockZone(0, 0, LZ_FIX);

		CreateShadow(22.0f, 0.5f);
	}
	if ( type == OBJECT_ROOT4 )
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEOBJECT);
		SetObjectRank(0, rank);
		pModFile->ReadModel("objects\\root4.mod");
		pModFile->CreateEngineObject(rank);
		SetPosition(0, pos);
		SetAngleY(0, angle);
		SetZoom(0, 2.0f*zoom);

		CreateLockZone(0, 0, LZ_FIX);

		CreateShadow(30.0f, 0.5f);
	}
	if ( type == OBJECT_ROOT5 )  // gravity root ?
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEOBJECT);
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

		CreateLockZone(0, 0, LZ_FIX);

		CreateShadow(30.0f, 0.5f);
	}

	pos = RetPosition(0);
	SetPosition(0, pos);  // pour afficher les ombres tout de suite

	SetFloorHeight(0.0f);
	CreateAuto(type);

	pos = RetPosition(0);
	pos.y += height;
	SetPosition(0, pos);

	delete pModFile;
	return TRUE;
}

// Crée une ruine posée sur le sol.

BOOL CObject::CreateRuin(D3DVECTOR pos, float angle, float zoom,
						 float height, ObjectType type)
{
	CModFile*	pModFile;
	char		name[50];
	int			rank;

	if ( m_engine->RetRestCreate() < 1+4 )  return FALSE;

	pModFile = new CModFile(m_iMan);

	SetType(type);

	rank = m_engine->CreateObject();
	m_engine->SetObjectType(rank, TYPEOBJECT);  // c'est un objet fixe
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

		CreateLockZone(0, 0, LZ_FIX);

		CreateShadow(4.0f, 1.0f);
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

		CreateLockZone(0, 0, LZ_FIX);

		CreateShadow(4.0f, 1.0f);
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

		CreateLockZone(0, 0, LZ_FIX);

		CreateShadow(5.0f, 1.0f);
	}

	if ( type == OBJECT_RUINmobilet2 )  // véhicule à chenilles ?
	{
		CreateLockZone(0, 0, LZ_FIX);

		CreateShadow(5.0f, 1.0f);
	}

	if ( type == OBJECT_RUINmobiler1 )  // véhicule roller ?
	{
		CreateLockZone(0, 0, LZ_FIX);

		CreateShadow(5.0f, 1.0f);
	}

	if ( type == OBJECT_RUINmobiler2 )  // véhicule roller ?
	{
		CreateLockZone(0, 0, LZ_FIX);

		CreateShadow(6.0f, 1.0f);
	}

	if ( type == OBJECT_RUINfactory )  // factory ?
	{
		CreateLockZone(0, 0, LZ_FIX);

		CreateShadow(20.0f, 0.7f);
	}

	if ( type == OBJECT_RUINdoor )  // porte convert ?
	{
		CreateLockZone(0, 0, LZ_FIX);

		CreateShadow(6.0f, 1.0f);
	}

	if ( type == OBJECT_RUINsupport )  // porte radar ?
	{
		CreateLockZone(0, 0, LZ_FIX);

		CreateShadow(3.0f, 1.0f);
	}

	if ( type == OBJECT_RUINradar )  // base radar ?
	{
		CreateLockZone(0, 0, LZ_FIX);

		CreateShadow(6.0f, 1.0f);
	}

	if ( type == OBJECT_RUINconvert )  // convert ?
	{
		CreateLockZone(0, 0, LZ_FIX);
	}

	if ( type == OBJECT_RUINbase )  // base ?
	{
		CreateLockZone(0, 0, LZ_FIX);

		CreateShadow(40.0f, 1.0f);
	}

	if ( type == OBJECT_RUINhead )  // coiffe base ?
	{
		CreateLockZone(0, 0, LZ_FIX);

		CreateShadow(30.0f, 1.0f);
	}

	pos = RetPosition(0);
	SetPosition(0, pos);  // pour afficher les ombres tout de suite

	SetFloorHeight(0.0f);
	CreateAuto(type);

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

void CObject::CreateAuto(ObjectType type)
{
	if ( type == OBJECT_DOOR1 ||
		 type == OBJECT_DOOR2 ||
		 type == OBJECT_DOOR3 ||
		 type == OBJECT_DOOR4 )
	{
		m_auto = new CAutoDoor(m_iMan, this);
	}
	if ( type == OBJECT_DOCK )
	{
		m_auto = new CAutoDock(m_iMan, this);
	}
	if ( type == OBJECT_CATAPULT )
	{
		m_auto = new CAutoCatapult(m_iMan, this);
	}
	if ( type == OBJECT_FIOLE ||
		 type == OBJECT_GLU   )
	{
		m_auto = new CAutoFiole(m_iMan, this);
	}
	if ( type == OBJECT_LIFT )
	{
		m_auto = new CAutoLift(m_iMan, this);
	}
	if ( type == OBJECT_GOAL )
	{
		m_auto = new CAutoGoal(m_iMan, this);
	}
	if ( type == OBJECT_MAX1X )
	{
		m_auto = new CAutoMax1x(m_iMan, this);
	}
	if ( type == OBJECT_BARRIER27 ||  // antenne ?
		 type == OBJECT_BARRIER28 ||  // antenne ?
		 type == OBJECT_BARRIER43 )   // pupitre ?
	{
		m_auto = new CAutoFlash(m_iMan, this);
	}
	if ( type >= OBJECT_SCRAP0 &&
		 type <= OBJECT_SCRAP9 )
	{
		m_auto = new CAutoScrap(m_iMan, this);
	}
	if ( type == OBJECT_BARRIER40 ||
		 type == OBJECT_BARRIER41 ||
		 type == OBJECT_BARRIER42 ||
		 type == OBJECT_BARRIER46 ||  // chateau avec drapeau ?
		 type == OBJECT_BARRIER58 ||  // futura étuve ?
		 type == OBJECT_BARRIER59 ||  // futura coupleur ?
		 type == OBJECT_BARRIER62 ||  // futura canon ?
		 type == OBJECT_BARRIER63 ||  // futura antenne ?
		 type == OBJECT_BARRIER64 ||  // futura creuset ?
		 type == OBJECT_BARRIER66 ||  // trésor ?
		 type == OBJECT_BARRIER74 ||  // labo étuve ?
		 type == OBJECT_BARRIER75 ||  // labo étuve ?
		 type == OBJECT_BARRIER76 ||  // labo coupleur ?
		 type == OBJECT_BARRIER77 )   // labo parabole ?
	{
		m_auto = new CAutoMeca(m_iMan, this);
	}
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
		m_engine->SetObjectTransform(m_objectPart[part].objRank,
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

// Incruste un objet dans le terrain, lorsqu'une caisse tombe
// dans un trou.

BOOL CObject::TerrainEmbedded()
{
	int		i;

	m_terrain->SetLockZone(RetPosition(0), LZ_FREE);
	m_engine->SetObjectType(m_objectPart[0].objRank, TYPETERRAIN);  // c'est un terrain
	m_engine->ShadowDelete(m_objectPart[0].objRank);

	for ( i=1 ; i<OBJECTMAXPART ; i++ )
	{
		if ( m_objectPart[i].bUsed )
		{
			if ( m_type == OBJECT_BOX11 ||
				 m_type == OBJECT_BOX12 )
			{
				if ( i >= 10 && i <= 13 )  continue;  // roue ?
			}

			m_objectPart[i].bUsed = FALSE;
			m_engine->DeleteObject(m_objectPart[i].objRank);

			if ( m_objectPart[i].masterParti != -1 )
			{
				m_particule->DeleteParticule(m_objectPart[i].masterParti);
				m_objectPart[i].masterParti = -1;
			}
		}
	}

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
	int		generation;

	if ( m_type != OBJECT_SHOW && m_engine->RetPause() )  return TRUE;

	m_aTime += event.rTime;
	m_shotTime += event.rTime;

	PartiFrame(event.rTime);

	UpdateMapping();
	UpdateTransformObject();

	if ( m_bTerrainHole && m_bAdjustShadow )
	{
		generation = m_terrain->RetGeneration();
		if ( generation != m_generation )  // modification du relief ?
		{
			AdjustShadow(m_lastPosTerrain);  // refait les ombres
			m_generation = generation;
		}
	}

	if ( m_taskList != 0 )
	{
		m_taskList->EventFrame(event);
	}

	if ( m_flashTime == 0.0f )
	{
		if ( m_bHilite )  // caisse survolée par la souris ?
		{
			if ( m_arrowMode != 1 )  // flèche pas visibles ?
			{
				m_arrowMode = 1;  // flèches visibles
				ShowActions(TRUE, -1, 0.0f);
//?				m_flashDelay = 0.2f;
				m_flashDelay = 0.0f;
			}
		}
		else	// caisse non survolée ?
		{
			if ( m_arrowMode != -1 )  // flèche pas cachées ?
			{
				if ( m_flashDelay > 0.0f )
				{
					m_flashDelay -= event.rTime;
					if ( m_flashDelay < 0.0f )  m_flashDelay = 0.0f;
				}

				if ( m_flashDelay == 0.0f )
				{
					m_arrowMode = -1;  // flèches cachées
					ShowActions(FALSE, -1, 0.0f);
				}
			}
		}
	}
	else
	{
		m_flashTime -= event.rTime*2.0f;
		if ( m_flashTime > 0.0f )
		{
			m_arrowMode = 0;  // flèches n'importe comment
			ShowActions(TRUE, m_flashPart, m_flashTime);
		}
		else
		{
			m_flashTime = 0.0f;
		}
	}

	return TRUE;
}

// Met à jour le mapping de l'objet.

void CObject::UpdateMapping()
{
}


// Indique si une partie est un bouton-action.

BOOL CObject::IsAction(int part)
{
	if ( (m_type >= OBJECT_BOX1 && m_type <= OBJECT_BOX20) ||
		 (m_type >= OBJECT_KEY1 && m_type <= OBJECT_KEY5 ) )
	{
		return ( part >= 1 && part <= 8 );
	}

	if ( m_type == OBJECT_DOCK )
	{
		return ( part >= 4 && part <= 8 );
	}

	if ( m_type == OBJECT_CATAPULT )
	{
		return ( part == 1 );
	}

	if ( m_type == OBJECT_FIOLE ||
		 m_type == OBJECT_GLU   )
	{
		return ( part == 1 );
	}

	if ( m_type == OBJECT_GOAL )
	{
		return ( part == 1 );
	}

	if ( m_type == OBJECT_TRAX  ||
		 m_type == OBJECT_PERFO ||
		 m_type == OBJECT_GUN   )
	{
		return ( part >= 3 && part <= 5 );
	}

	return FALSE;
}

// Montre ou cache les boutons-actions de l'objet.
//	bShow=TRUE,  part=-1, time=0 -> montre tous les boutons
//	bShow=FALSE, part=-1, time=0 -> cache tous les boutons
//	bShow=TRUE,  part= n, time=n -> montre un seul bouton mobile

void CObject::ShowActions(BOOL bShow, int part, float time)
{
	CObject*	pSel;
	D3DVECTOR	pos;

	pSel = m_main->RetSelect();

	if ( (m_type >= OBJECT_BOX1 && m_type <= OBJECT_BOX20) ||
		 (m_type >= OBJECT_KEY1 && m_type <= OBJECT_KEY5 ) )
	{
		if ( bShow && (part == -1 || part == 1) )  // ouest ?
		{
			pos.x = -sinf(time*PI*8.0f)*0.5f;
			pos.y = m_arrowPos;
			pos.z = 0.0f;
			SetPosition(1, pos);
			SetHide(1, FALSE);
		}
		else
		{
			SetHide(1, TRUE);
		}

		if ( bShow && (part == -1 || part == 2) )  // nord ?
		{
			pos.x = 0.0f;
			pos.y = m_arrowPos;
			pos.z = sinf(time*PI*8.0f)*0.5f;
			SetPosition(2, pos);
			SetHide(2, FALSE);
		}
		else
		{
			SetHide(2, TRUE);
		}

		if ( bShow && (part == -1 || part == 3) )  // est ?
		{
			pos.x = sinf(time*PI*8.0f)*0.5f;
			pos.y = m_arrowPos;
			pos.z = 0.0f;
			SetPosition(3, pos);
			SetHide(3, FALSE);
		}
		else
		{
			SetHide(3, TRUE);
		}

		if ( bShow && (part == -1 || part == 4) )  // sud ?
		{
			pos.x = 0.0f;
			pos.y = m_arrowPos;
			pos.z = -sinf(time*PI*8.0f)*0.5f;
			SetPosition(4, pos);
			SetHide(4, FALSE);
		}
		else
		{
			SetHide(4, TRUE);
		}

		if ( pSel != 0 && pSel->RetFuturStrong() != 0.0f )  // glu/potion ?
		{
			bShow = FALSE;  // pas de poignées "rep"
		}
		if ( m_main->RetSuperShift19() != 0 )
		{
			bShow = FALSE;  // pas de poignées "rep"
		}

		if ( bShow && (part == -1 || part == 5) )  // ouest-rep ?
		{
			pos.x = -sinf(time*PI*8.0f)*0.5f;
			pos.y = m_arrowPos;
			pos.z = 0.0f;
			SetPosition(5, pos);
			SetHide(5, FALSE);
		}
		else
		{
			SetHide(5, TRUE);
		}

		if ( bShow && (part == -1 || part == 6) )  // nord-rep ?
		{
			pos.x = 0.0f;
			pos.y = m_arrowPos;
			pos.z = sinf(time*PI*8.0f)*0.5f;
			SetPosition(6, pos);
			SetHide(6, FALSE);
		}
		else
		{
			SetHide(6, TRUE);
		}

		if ( bShow && (part == -1 || part == 7) )  // est-rep ?
		{
			pos.x = sinf(time*PI*8.0f)*0.5f;
			pos.y = m_arrowPos;
			pos.z = 0.0f;
			SetPosition(7, pos);
			SetHide(7, FALSE);
		}
		else
		{
			SetHide(7, TRUE);
		}

		if ( bShow && (part == -1 || part == 8) )  // sud-rep ?
		{
			pos.x = 0.0f;
			pos.y = m_arrowPos;
			pos.z = -sinf(time*PI*8.0f)*0.5f;
			SetPosition(8, pos);
			SetHide(8, FALSE);
		}
		else
		{
			SetHide(8, TRUE);
		}
	}

	if ( m_type == OBJECT_DOCK )
	{
		if ( bShow && (part == -1 || part == 4) )  // ^
		{
			pos.x =  sinf(time*PI*8.0f)*0.5f;
			pos.y = -sinf(time*PI*8.0f)*0.5f+m_arrowPos;
			pos.z = 0.0f;
			SetPosition(4, pos);
			SetHide(4, FALSE);
		}
		else
		{
			SetHide(4, TRUE);
		}

		if ( bShow && (part == -1 || part == 5) )  // v
		{
			pos.x = -sinf(time*PI*8.0f)*0.5f;
			pos.y =  sinf(time*PI*8.0f)*0.5f+m_arrowPos;
			pos.z = 0.0f;
			SetPosition(5, pos);
			SetHide(5, FALSE);
		}
		else
		{
			SetHide(5, TRUE);
		}

		if ( bShow && (part == -1 || part == 6) )  // piston
		{
			pos.x = -sinf(time*PI*8.0f)*0.5f;
			pos.y =  sinf(time*PI*8.0f)*0.5f+m_arrowPos;
			pos.z = 0.0f;
			SetPosition(6, pos);
			SetHide(6, FALSE);
		}
		else
		{
			SetHide(6, TRUE);
		}

		if ( bShow && (part == -1 || part == 7) )  // <
		{
			pos.x = 0.0f;
			pos.y = m_arrowPos;
			pos.z = sinf(time*PI*8.0f);
			SetPosition(7, pos);
			SetHide(7, FALSE);
		}
		else
		{
			SetHide(7, TRUE);
		}

		if ( bShow && (part == -1 || part == 8) )  // >
		{
			pos.x = 0.0f;
			pos.y = m_arrowPos;
			pos.z = -sinf(time*PI*8.0f);
			SetPosition(8, pos);
			SetHide(8, FALSE);
		}
		else
		{
			SetHide(8, TRUE);
		}
	}

	if ( m_type == OBJECT_CATAPULT )
	{
		if ( bShow && (part == -1 || part == 1) )  // go
		{
			pos.x = 0.0f;
			pos.y = -sinf(time*PI*8.0f)*0.3f;
			pos.z =  sinf(time*PI*8.0f)*0.3f;
			SetPosition(1, pos);
			SetHide(1, FALSE);
		}
		else
		{
			SetHide(1, TRUE);
		}
	}

	if ( m_type == OBJECT_FIOLE ||
		 m_type == OBJECT_GLU   )
	{
		if ( bShow && (part == -1 || part == 1) )
		{
			pos.x = 0.0f;
			pos.y = m_arrowPos+sinf(time*PI*8.0f)*0.2f;
			pos.z = 0.0f;
			SetPosition(1, pos);
			SetHide(1, FALSE);
		}
		else
		{
			SetHide(1, TRUE);
		}
	}

	if ( m_type == OBJECT_GOAL )
	{
		if ( bShow && (part == -1 || part == 1) )
		{
			pos.x = 0.0f;
			pos.y = m_arrowPos+sinf(time*PI*8.0f)*0.2f;
			pos.z = 0.0f;
			SetPosition(1, pos);
			SetHide(1, FALSE);
		}
		else
		{
			SetHide(1, TRUE);
		}
	}

	if ( m_type == OBJECT_TRAX  ||
		 m_type == OBJECT_PERFO ||
		 m_type == OBJECT_GUN )
	{
		if ( bShow && (part == -1 || part == 3) )  // avance ?
		{
			pos.x = -sinf(time*PI*8.0f)*0.2f;
			pos.y =  sinf(time*PI*8.0f)*0.2f;
			pos.z = 0.0f;
			SetPosition(3, pos);
			SetHide(3, FALSE);
		}
		else
		{
			SetHide(3, TRUE);
		}

		if ( bShow && (part == -1 || part == 4) )  // gauche (tourne droite) ?
		{
			pos.x = 0.0f;
			pos.y = sinf(time*PI*8.0f)*0.2f;
			pos.z = sinf(time*PI*8.0f)*0.2f;
			SetPosition(4, pos);
			SetHide(4, FALSE);
		}
		else
		{
			SetHide(4, TRUE);
		}

		if ( bShow && (part == -1 || part == 5) )  // droite (tourne gauche) ?
		{
			pos.x = 0.0f;
			pos.y = sinf(time*PI*8.0f)*0.2f;
			pos.z = -sinf(time*PI*8.0f)*0.2f;
			SetPosition(5, pos);
			SetHide(5, FALSE);
		}
		else
		{
			SetHide(5, TRUE);
		}
	}
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
		switch( (i+m_id)%5 )
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
	int		part;

	UpdateTransformObject();

	part = 0;
	eye.x =  0.7f;  // entre les supports
	eye.y =  4.8f;
	eye.z =  0.0f;

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
		 m_type == OBJECT_TNT      ||
		 m_type == OBJECT_MINE     ||
		 m_type == OBJECT_WAYPOINT ||
		 m_type == OBJECT_BARREL   ||
		 m_type == OBJECT_BARRELa  ||
		 m_type == OBJECT_ATOMIC   ||
		 m_type == OBJECT_FIOLE    ||
		 m_type == OBJECT_GLU      ||
		 m_type == OBJECT_GOAL     ||
		 m_type == OBJECT_GLASS1   ||
		 m_type == OBJECT_GLASS2   )
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


// Gestion de la force.

void CObject::SetStrong(float level)
{
	m_strong = level;
}

float CObject::RetStrong()
{
	if ( m_type == OBJECT_TRAX )  return 1.0f; // toujours puissant
	return m_strong;
}

void CObject::SetFuturStrong(float level)
{
	m_futurStrong = level;
}

float CObject::RetFuturStrong()
{
	return m_futurStrong;
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
			m_engine->SetObjectTransparency(m_objectPart[i].objRank, value);
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

	if ( m_type == OBJECT_GOAL )
	{
		m_auto->SetAction(1);
		return TRUE;
	}

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



// Démarre le flash de l'objet.

void CObject::SetFlash(int part)
{
//?	m_flashTime = 0.5f;
	m_flashTime = 1.0f/(PI*1.0f);
	m_flashPart = part;
}

// Gestion de la mise en évidence de l'objet.

void CObject::SetHilite(BOOL bMode, BOOL bSelectable)
{
	int			list[OBJECTMAXPART+1];
	int			i, j;

	m_bHilite = bMode;
	m_arrowMode = 0;  // flèches n'importe comment

	if ( m_bHilite && bSelectable )
	{
		j = 0;
		for ( i=0 ; i<m_totalPart ; i++ )
		{
			if ( m_objectPart[i].bUsed )
			{
				list[j++] = m_objectPart[i].objRank;
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
		return;
	}

	m_bSelect = bMode;

	m_engine->SetObjectShadowSelect(m_objectPart[0].objRank, m_bSelect);

	if ( !m_bSelect )
	{
		return;  // fini si pas sélectionné
	}

	err = ERR_OK;
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

	if ( m_bLock )  m_bHilite = FALSE;
}

BOOL CObject::RetLock()
{
	return m_bLock;
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


// Gestion du mode "undo impossible maintenant" d'un objet.
// Un objet dans ce mode n'est pas sauvegardé.

void CObject::SetNoUndoable(BOOL bExplo)
{
	m_bNoUndoable = bExplo;
}

BOOL CObject::RetNoUndoable()
{
	return m_bNoUndoable;
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
	return !m_bLock && !m_bBurn && !m_bFlat && m_bEnable && !m_bGhost;
}


// Retourne la hauteur d'un objet.

float CObject::RetHeight()
{
	if ( m_type == OBJECT_BLUPI )  return 6.5f;
	if ( m_type == OBJECT_BOX8 )  return 16.0f;  // colonne ?
	if ( m_type >= OBJECT_KEY1 && m_type <= OBJECT_KEY5 )  return 7.0f;

	return 8.0f;  // hauteur standard
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

// Retourne le mouvement associé à l'objet.

CMotion* CObject::RetMotion()
{
	return m_motion;
}

// Retourne la liste de tâches accociée à l'objet.

CTaskList* CObject::RetTaskList()
{
	return m_taskList;
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


// Ajoute une tâche dans la liste.

BOOL CObject::StartTaskList(TaskOrder order, D3DVECTOR pos,
							CObject *target, int part, float param)
{
	TaskOrder	cOrder;
	D3DVECTOR	cPos;
	float		cParam;
	int			cId, cPart, id;

	if ( m_taskList == 0 )  return FALSE;

	if ( target == 0 )
	{
		id = 0;
	}
	else
	{
		id = target->RetID();
	}

	if ( order == TO_GOTODRINK && target != 0 )
	{
		if ( target->RetType() == OBJECT_FIOLE )  m_futurStrong =  1.0f;
		if ( target->RetType() == OBJECT_GLU   )  m_futurStrong = -1.0f;
	}
	if ( order == TO_GOTOPUSH )
	{
		m_futurStrong = 0.0f;
	}

	// Si l'ordre est de pousser une caisse, regarde si c'est la
	// même caisse et le même bouton, pour transformer l'ordre
	// "pousse n fois" en "pousse n+1 fois".
	if ( order == TO_GOTOPUSH )
	{
		if ( m_taskList->SubHead(cOrder, cPos, cId, cPart, cParam) )
		{
			if ( (cOrder == TO_GOTOPUSH ||
				  cOrder == TO_PUSH     ) &&
				 cId == id && cPart == part )
			{
				return m_taskList->AddHead(cOrder, pos, id, part, cParam+param);
			}

			m_taskList->AddHead(cOrder, cPos, cId, cPart, cParam);
		}
	}

	return m_taskList->AddHead(order, pos, id, part, param);
}


// Cherche une ressource de type "Pousser 3 fois".

void GetResource19(int res, int sh19, char *buffer)
{
	char	text[50];

#if _ENGLISH
	if ( sh19 <= 1 )
	{
		GetResource(RES_TEXT, res, buffer);  // RT_ACTION_PUSH1
	}
	else if ( sh19 == 2 )
	{
		GetResource(RES_TEXT, res+1, buffer);  // RT_ACTION_PUSH2
	}
	else
	{
		GetResource(RES_TEXT, res+2, text);  // RT_ACTION_PUSHx
		sprintf(buffer, text, sh19);
	}
#else
	if ( sh19 <= 1 )
	{
		GetResource(RES_TEXT, res, buffer);  // RT_ACTION_PUSH1
	}
	else
	{
		GetResource(RES_TEXT, res+2, text);  // RT_ACTION_PUSHx
		sprintf(buffer, text, sh19);
	}
#endif
}

// Donne le nom de l'objet pour le tooltip.

BOOL CObject::GetTooltipName(int part, char* name)
{
	CObject*	pSel;
	float		strong;
	int			sh19;

	pSel = m_main->RetSelect();
	sh19 = m_main->RetSuperShift19();

	if ( m_type == OBJECT_BOX7  ||  // sphère ?
		 m_type == OBJECT_BOX10 )   // bombe ?
	{
		if ( part >= 1 && part <= 4 )
		{
			GetResource(RES_TEXT, RT_ACTION_ROLL, name);
			return TRUE;
		}
	}

	if ( (m_type >= OBJECT_BOX1 && m_type <= OBJECT_BOX20) ||
		 (m_type >= OBJECT_KEY1 && m_type <= OBJECT_KEY5 ) )
	{
		if ( part >= 1 && part <= 4 )
		{
			if ( pSel == 0 )
			{
				strong = 0.0f;
			}
			else
			{
				strong = pSel->RetFuturStrong();
			}

			if ( strong < 0.0f )  // glu ?
			{
				GetResource(RES_TEXT, RT_ACTION_PULL1, name);
			}
			else
			{
				if ( sh19 <= 1 || strong != 0.0f )
				{
					GetResource(RES_TEXT, RT_ACTION_PUSH1, name);
				}
				else
				{
					GetResource19(RT_ACTION_PUSH1, sh19, name);
				}
			}
			return TRUE;
		}
		if ( part >= 5 && part <= 8 )
		{
			GetResource(RES_TEXT, RT_ACTION_PUSHn, name);
			return TRUE;
		}
	}

	if ( m_type == OBJECT_DOCK )
	{
		if ( part == 6 )
		{
			GetResource(RES_TEXT, RT_ACTION_DOCKg, name);
			return TRUE;
		}
		if ( part >= 4 && part <= 8 )
		{
			GetResource19(RT_ACTION_DOCKm, sh19, name);
			return TRUE;
		}
	}

	if ( m_type == OBJECT_CATAPULT )
	{
		if ( part == 1 )
		{
			GetResource(RES_TEXT, RT_ACTION_CATAPULT, name);
			return TRUE;
		}
	}

	if ( m_type == OBJECT_TRAX  ||
		 m_type == OBJECT_PERFO )
	{
		if ( part == 3 )
		{
			GetResource19(RT_ACTION_TRAXa, sh19, name);
			return TRUE;
		}
		if ( part == 4 )
		{
			GetResource19(RT_ACTION_TRAXl, sh19, name);
			return TRUE;
		}
		if ( part == 5 )
		{
			GetResource19(RT_ACTION_TRAXr, sh19, name);
			return TRUE;
		}
	}

	if ( m_type == OBJECT_GUN )
	{
		if ( part == 3 )
		{
			GetResource19(RT_ACTION_GUNa, sh19, name);
			return TRUE;
		}
		if ( part == 4 )
		{
			GetResource19(RT_ACTION_GUNl, sh19, name);
			return TRUE;
		}
		if ( part == 5 )
		{
			GetResource19(RT_ACTION_GUNr, sh19, name);
			return TRUE;
		}
	}

	GetResource(RES_OBJECT, m_type, name);
	return ( name[0] != 0 );
}


// Ecrit la situation de l'objet.

void CObject::WriteSituation()
{
	D3DVECTOR	pos;
	float		angle;
	int			i;
	char		op[10];

	m_undo->WriteTokenInt("id", m_id);
	m_undo->WriteTokenInt("type", m_type);
	m_undo->WriteTokenInt("lock", m_bLock);
	m_undo->WriteTokenInt("dead", m_bDead);

	pos = RetPosition(0);
	m_undo->WriteTokenPos("pos", pos);

	angle = RetAngleY(0);
	m_undo->WriteTokenFloat("ay", angle);

	for ( i=0 ; i<10 ; i++ )
	{
		if ( m_additional[i] != 0 )
		{
			sprintf(op, "add%d", i);
			m_undo->WriteTokenInt(op, m_additional[i]);
		}
	}

	if ( m_strong != 0.0f )
	{
		m_undo->WriteTokenFloat("strong", m_strong);
	}

	if ( m_truckLink != 0 )
	{
		m_undo->WriteTokenInt("truckLink", m_truckLink);
	}

	if ( m_truck != 0 )
	{
		m_undo->WriteTokenInt("truck", m_truck->RetID());
	}

	if ( m_fret != 0 )
	{
		m_undo->WriteTokenInt("fret", m_fret->RetID());
	}

	if ( m_auto != 0 )
	{
		m_auto->WriteSituation();
	}

	if ( m_type == OBJECT_CRAZY )
	{
		if ( m_motion != 0 )
		{
			m_motion->WriteSituation();
		}
	}
}

// Lit la situation de l'objet.

void CObject::ReadSituation()
{
	CObject*	pObj;
	D3DVECTOR	nPos, iPos, pos, speed;
	FPOINT		dim;
	float		angle;
	int			i;

	if ( m_undo->ReadTokenInt("type", i) )
	{
		m_type = (ObjectType)i;
	}

	if ( m_undo->ReadTokenPos("pos", nPos) )
	{
		iPos = RetPosition(0);
		if ( nPos.x != iPos.x ||
			 nPos.y != iPos.y ||
			 nPos.z != iPos.z )  // position changée ?
		{
			SetPosition(0, nPos);

			for ( i=0 ; i<=20 ; i++ )
			{
				pos = nPos + (iPos-nPos)*(i/20.0f);
				pos.x += (Rand()-0.5f)*4.0f;
				pos.z += (Rand()-0.5f)*4.0f;
				speed.x = 0.0f;
				speed.z = 0.0f;
				speed.y = 2.0f;
				dim.x = 3.0f;
				dim.y = dim.x;
				m_particule->CreateParticule(pos, speed, dim, (ParticuleType)(PARTILENS1+rand()%4), 1.0f);
			}
		}
	}

	if ( m_undo->ReadTokenFloat("ay", angle) )
	{
		SetAngleY(0, angle);
	}

	m_strong = 0.0f;
	m_undo->ReadTokenFloat("strong", m_strong);
	m_futurStrong = m_strong;

	m_truckLink = 0;
	if ( m_undo->ReadTokenInt("truckLink", i) )
	{
		m_truckLink = i;
	}

	pObj = 0;
	if ( m_undo->ReadTokenInt("truck", i) )
	{
		pObj = m_undo->SearchObjectID(i);
	}
	SetTruck(pObj);

	pObj = 0;
	if ( m_undo->ReadTokenInt("fret", i) )
	{
		pObj = m_undo->SearchObjectID(i);
	}
	SetFret(pObj);

	if ( m_auto != 0 )
	{
		m_auto->ReadSituation();
	}

	if ( m_type == OBJECT_BLUPI )
	{
		if ( m_motion != 0 )
		{
			if ( m_truck == 0 )
			{
				m_motion->SetAction(MBLUPI_WAIT);
				m_motion->SetAction(MBLUPI_WALK);
				m_motion->SetAction(MBLUPI_MOUTH);
			}
			else
			{
				m_motion->SetAction(MBLUPI_TRUCK);
				m_motion->SetAction(MBLUPI_WALK);
				m_motion->SetAction(MBLUPI_MOUTH);
			}
		}
	}

	if ( m_type == OBJECT_CRAZY )
	{
		if ( m_motion != 0 )
		{
			m_motion->ReadSituation();
		}
	}
}

