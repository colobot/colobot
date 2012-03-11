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

#ifndef _OBJECT_H_
#define	_OBJECT_H_


class CInstanceManager;
class CD3DEngine;
class CLight;
class CTerrain;
class CWater;
class CCamera;
class CParticule;
class CPhysics;
class CBrain;
class CMotion;
class CAuto;
class CDisplayText;
class CRobotMain;
class CBotVar;
class CScript;

enum CameraType;
enum Sound;
enum D3DShadowType;



// Le p�re de toutes les parties doit toujours �tre la partie
// num�ro z�ro !

#define OBJECTMAXPART		40
#define MAXCRASHSPHERE		40
#define OBJECTMAXDESELLIST	10
#define OBJECTMAXINFO		10
#define OBJECTMAXCMDLINE	20

enum ObjectType
{
	OBJECT_NULL		    = 0,	// objet d�truit
	OBJECT_FIX		    = 1,	// d�cor fixe
	OBJECT_PORTICO	    = 2,	// portique
	OBJECT_BASE		    = 3,	// grande base principale
	OBJECT_DERRICK	    = 4,	// derrick fixe
	OBJECT_FACTORY	    = 5,	// usine fixe
	OBJECT_STATION	    = 6,	// station de recharge
	OBJECT_CONVERT	    = 7,	// station de transformation
	OBJECT_REPAIR	    = 8,	// r�paration
	OBJECT_TOWER	    = 9,	// tour de d�fense
	OBJECT_NEST		    = 10,	// nid
	OBJECT_RESEARCH     = 11,	// centre de recherches
	OBJECT_RADAR	    = 12,	// radar
	OBJECT_ENERGY	    = 13,	// centrale d'�nergie
	OBJECT_LABO		    = 14,	// laboratoire d'analyse pour insectes
	OBJECT_NUCLEAR		= 15,	// centrale nucl�aire
	OBJECT_START		= 16,	// d�part
	OBJECT_END			= 17,	// arriv�e
	OBJECT_INFO		    = 18,	// borne d'information
	OBJECT_PARA			= 19,	// paratonnerre
	OBJECT_TARGET1		= 20,	// portique cible
	OBJECT_TARGET2		= 21,	// centre cible
	OBJECT_SAFE			= 22,	// coffre fort
	OBJECT_HUSTON		= 23,	// centre de contr�le
	OBJECT_DESTROYER	= 24,	// destructeur
	OBJECT_FRET		    = 30,	// transportable
	OBJECT_STONE	    = 31,	// pierre
	OBJECT_URANIUM	    = 32,	// uranium
	OBJECT_METAL	    = 33,	// m�tal
	OBJECT_POWER	    = 34,	// pile normale
	OBJECT_ATOMIC	    = 35,	// pile atomique
	OBJECT_BULLET	    = 36,	// boulet
	OBJECT_BBOX		    = 37,	// black-box
	OBJECT_TNT			= 38,	// caisse de TNT
	OBJECT_SCRAP1		= 40,	// d�chet m�tallique
	OBJECT_SCRAP2		= 41,	// d�chet m�tallique
	OBJECT_SCRAP3		= 42,	// d�chet m�tallique
	OBJECT_SCRAP4		= 43,	// d�chet plastique
	OBJECT_SCRAP5		= 44,	// d�chet plastique
	OBJECT_MARKPOWER	= 50,	// marque pile en sous-sol
	OBJECT_MARKSTONE	= 51,	// marque minerai en sous-sol
	OBJECT_MARKURANIUM  = 52,	// marque uranium en sous-sol
	OBJECT_MARKKEYa		= 53,	// marque cl� en sous-sol
	OBJECT_MARKKEYb		= 54,	// marque cl� en sous-sol
	OBJECT_MARKKEYc		= 55,	// marque cl� en sous-sol
	OBJECT_MARKKEYd		= 56,	// marque cl� en sous-sol
	OBJECT_BOMB			= 60,	// bombe
	OBJECT_WINFIRE		= 61,	// feu d'artifice
	OBJECT_SHOW			= 62,	// montre un lieu
	OBJECT_BAG			= 63,	// sac de survie
	OBJECT_PLANT0		= 70,	// plante 0
	OBJECT_PLANT1		= 71,	// plante 1
	OBJECT_PLANT2		= 72,	// plante 2
	OBJECT_PLANT3		= 73,	// plante 3
	OBJECT_PLANT4		= 74,	// plante 4
	OBJECT_PLANT5		= 75,	// plante 5
	OBJECT_PLANT6		= 76,	// plante 6
	OBJECT_PLANT7		= 77,	// plante 7
	OBJECT_PLANT8		= 78,	// plante 8
	OBJECT_PLANT9		= 79,	// plante 9
	OBJECT_PLANT10		= 80,	// plante 10
	OBJECT_PLANT11		= 81,	// plante 11
	OBJECT_PLANT12		= 82,	// plante 12
	OBJECT_PLANT13		= 83,	// plante 13
	OBJECT_PLANT14		= 84,	// plante 14
	OBJECT_PLANT15		= 85,	// plante 15
	OBJECT_PLANT16		= 86,	// plante 16
	OBJECT_PLANT17		= 87,	// plante 17
	OBJECT_PLANT18		= 88,	// plante 18
	OBJECT_PLANT19		= 89,	// plante 19
	OBJECT_TREE0		= 90,	// arbre 0
	OBJECT_TREE1		= 91,	// arbre 1
	OBJECT_TREE2		= 92,	// arbre 2
	OBJECT_TREE3		= 93,	// arbre 3
	OBJECT_TREE4		= 94,	// arbre 4
	OBJECT_TREE5		= 95,	// arbre 5
	OBJECT_TREE6		= 96,	// arbre 6
	OBJECT_TREE7		= 97,	// arbre 7
	OBJECT_TREE8		= 98,	// arbre 8
	OBJECT_TREE9		= 99,	// arbre 9
	OBJECT_MOBILEwt	    = 100,	// wheel-trainer
	OBJECT_MOBILEtt	    = 101,	// track-trainer
	OBJECT_MOBILEft	    = 102,	// fly-trainer
	OBJECT_MOBILEit	    = 103,	// insect-trainer
	OBJECT_MOBILEwa	    = 110,	// wheel-arm
	OBJECT_MOBILEta	    = 111,	// track-arm
	OBJECT_MOBILEfa	    = 112,	// fly-arm
	OBJECT_MOBILEia	    = 113,	// insect-arm
	OBJECT_MOBILEwc	    = 120,	// wheel-cannon
	OBJECT_MOBILEtc	    = 121,	// track-cannon
	OBJECT_MOBILEfc	    = 122,	// fly-cannon
	OBJECT_MOBILEic	    = 123,	// insect-cannon
	OBJECT_MOBILEwi	    = 130,	// wheel-insect-cannon
	OBJECT_MOBILEti	    = 131,	// track-insect-cannon
	OBJECT_MOBILEfi	    = 132,	// fly-insect-cannon
	OBJECT_MOBILEii	    = 133,	// insect-insect-cannon
	OBJECT_MOBILEws	    = 140,	// wheel-search
	OBJECT_MOBILEts	    = 141,	// track-search
	OBJECT_MOBILEfs	    = 142,	// fly-search
	OBJECT_MOBILEis	    = 143,	// insect-search
	OBJECT_MOBILErt	    = 200,	// roller-terraform
	OBJECT_MOBILErc	    = 201,	// roller-canon
	OBJECT_MOBILErr	    = 202,	// roller-recover
	OBJECT_MOBILErs	    = 203,	// roller-shield
	OBJECT_MOBILEsa	    = 210,	// sous-marin
	OBJECT_MOBILEtg	    = 211,	// cible d'exercice
	OBJECT_MOBILEdr	    = 212,	// robot de dessin
	OBJECT_WAYPOINT 	= 250,	// chemin
	OBJECT_FLAGb		= 260,	// drapeau bleu
	OBJECT_FLAGr		= 261,	// drapeau rouge
	OBJECT_FLAGg		= 262,	// drapeau vert
	OBJECT_FLAGy		= 263,	// drapeau jaune
	OBJECT_FLAGv		= 264,	// drapeau violet
	OBJECT_KEYa			= 270,	// cl� a
	OBJECT_KEYb			= 271,	// cl� b
	OBJECT_KEYc			= 272,	// cl� c
	OBJECT_KEYd			= 273,	// cl� d
	OBJECT_HUMAN	    = 300,	// homme
	OBJECT_TOTO		    = 301,	// toto
	OBJECT_TECH		    = 302,	// technicien
	OBJECT_BARRIER0		= 400,	// barri�re
	OBJECT_BARRIER1		= 401,	// barri�re
	OBJECT_BARRIER2		= 402,	// barri�re
	OBJECT_BARRIER3		= 403,	// barri�re
	OBJECT_BARRIER4		= 404,	// barri�re
	OBJECT_MOTHER	    = 500,	// m�re pondeuse
	OBJECT_EGG		    = 501,	// oeuf
	OBJECT_ANT		    = 502,	// fourmi
	OBJECT_SPIDER	    = 503,	// araign�e
	OBJECT_BEE		    = 504,	// abeille
	OBJECT_WORM		    = 505,	// ver
	OBJECT_RUINmobilew1 = 600,	// ruine 1
	OBJECT_RUINmobilew2 = 601,	// ruine 1
	OBJECT_RUINmobilet1 = 602,	// ruine 2
	OBJECT_RUINmobilet2 = 603,	// ruine 2
	OBJECT_RUINmobiler1 = 604,	// ruine 3
	OBJECT_RUINmobiler2 = 605,	// ruine 3
	OBJECT_RUINfactory  = 606,	// ruine 4
	OBJECT_RUINdoor     = 607,	// ruine 5
	OBJECT_RUINsupport  = 608,	// ruine 6
	OBJECT_RUINradar    = 609,	// ruine 7
	OBJECT_RUINconvert  = 610,	// ruine 8
	OBJECT_RUINbase	    = 611,	// ruine 9
	OBJECT_RUINhead	    = 612,	// ruine 10
	OBJECT_TEEN0		= 620,	// jouet
	OBJECT_TEEN1		= 621,	// jouet
	OBJECT_TEEN2		= 622,	// jouet
	OBJECT_TEEN3		= 623,	// jouet
	OBJECT_TEEN4		= 624,	// jouet
	OBJECT_TEEN5		= 625,	// jouet
	OBJECT_TEEN6		= 626,	// jouet
	OBJECT_TEEN7		= 627,	// jouet
	OBJECT_TEEN8		= 628,	// jouet
	OBJECT_TEEN9		= 629,	// jouet
	OBJECT_TEEN10	    = 630,	// jouet
	OBJECT_TEEN11	    = 631,	// jouet
	OBJECT_TEEN12	    = 632,	// jouet
	OBJECT_TEEN13	    = 633,	// jouet
	OBJECT_TEEN14	    = 634,	// jouet
	OBJECT_TEEN15	    = 635,	// jouet
	OBJECT_TEEN16	    = 636,	// jouet
	OBJECT_TEEN17	    = 637,	// jouet
	OBJECT_TEEN18	    = 638,	// jouet
	OBJECT_TEEN19	    = 639,	// jouet
	OBJECT_TEEN20	    = 640,	// jouet
	OBJECT_TEEN21	    = 641,	// jouet
	OBJECT_TEEN22	    = 642,	// jouet
	OBJECT_TEEN23	    = 643,	// jouet
	OBJECT_TEEN24	    = 644,	// jouet
	OBJECT_TEEN25	    = 645,	// jouet
	OBJECT_TEEN26	    = 646,	// jouet
	OBJECT_TEEN27	    = 647,	// jouet
	OBJECT_TEEN28	    = 648,	// jouet
	OBJECT_TEEN29	    = 649,	// jouet
	OBJECT_TEEN30	    = 650,	// jouet
	OBJECT_TEEN31	    = 651,	// jouet
	OBJECT_TEEN32	    = 652,	// jouet
	OBJECT_TEEN33	    = 653,	// jouet
	OBJECT_TEEN34	    = 654,	// jouet
	OBJECT_TEEN35	    = 655,	// jouet
	OBJECT_TEEN36	    = 656,	// jouet
	OBJECT_TEEN37	    = 657,	// jouet
	OBJECT_TEEN38	    = 658,	// jouet
	OBJECT_TEEN39	    = 659,	// jouet
	OBJECT_TEEN40	    = 660,	// jouet
	OBJECT_TEEN41	    = 661,	// jouet
	OBJECT_TEEN42	    = 662,	// jouet
	OBJECT_TEEN43	    = 663,	// jouet
	OBJECT_TEEN44	    = 664,	// jouet
	OBJECT_TEEN45	    = 665,	// jouet
	OBJECT_TEEN46	    = 666,	// jouet
	OBJECT_TEEN47	    = 667,	// jouet
	OBJECT_TEEN48	    = 668,	// jouet
	OBJECT_TEEN49	    = 669,	// jouet
	OBJECT_QUARTZ0		= 700,	// quartz 0
	OBJECT_QUARTZ1		= 701,	// quartz 1
	OBJECT_QUARTZ2		= 702,	// quartz 2
	OBJECT_QUARTZ3		= 703,	// quartz 3
	OBJECT_QUARTZ4		= 704,	// quartz 4
	OBJECT_QUARTZ5		= 705,	// quartz 5
	OBJECT_QUARTZ6		= 706,	// quartz 6
	OBJECT_QUARTZ7		= 707,	// quartz 7
	OBJECT_QUARTZ8		= 708,	// quartz 8
	OBJECT_QUARTZ9		= 709,	// quartz 9
	OBJECT_ROOT0		= 710,	// racine 0
	OBJECT_ROOT1		= 711,	// racine 1
	OBJECT_ROOT2		= 712,	// racine 2
	OBJECT_ROOT3		= 713,	// racine 3
	OBJECT_ROOT4		= 714,	// racine 4
	OBJECT_ROOT5		= 715,	// racine 5
	OBJECT_ROOT6		= 716,	// racine 6
	OBJECT_ROOT7		= 717,	// racine 7
	OBJECT_ROOT8		= 718,	// racine 8
	OBJECT_ROOT9		= 719,	// racine 9
	OBJECT_SEAWEED0		= 720,	// algue 0
	OBJECT_SEAWEED1		= 721,	// algue 1
	OBJECT_SEAWEED2		= 722,	// algue 2
	OBJECT_SEAWEED3		= 723,	// algue 3
	OBJECT_SEAWEED4		= 724,	// algue 4
	OBJECT_SEAWEED5		= 725,	// algue 5
	OBJECT_SEAWEED6		= 726,	// algue 6
	OBJECT_SEAWEED7		= 727,	// algue 7
	OBJECT_SEAWEED8		= 728,	// algue 8
	OBJECT_SEAWEED9		= 729,	// algue 9
	OBJECT_MUSHROOM0	= 730,	// champignon 0
	OBJECT_MUSHROOM1	= 731,	// champignon 1
	OBJECT_MUSHROOM2	= 732,	// champignon 2
	OBJECT_MUSHROOM3	= 733,	// champignon 3
	OBJECT_MUSHROOM4	= 734,	// champignon 4
	OBJECT_MUSHROOM5	= 735,	// champignon 5
	OBJECT_MUSHROOM6	= 736,	// champignon 6
	OBJECT_MUSHROOM7	= 737,	// champignon 7
	OBJECT_MUSHROOM8	= 738,	// champignon 8
	OBJECT_MUSHROOM9	= 739,	// champignon 9
	OBJECT_APOLLO1		= 900,	// apollo lem
	OBJECT_APOLLO2		= 901,	// apollo jeep
	OBJECT_APOLLO3		= 902,	// apollo flag
	OBJECT_APOLLO4		= 903,	// apollo module
	OBJECT_APOLLO5		= 904,	// apollo antenna
	OBJECT_HOME1		= 910,	// maison 1
	OBJECT_MAX			= 1000,
};

enum ObjectMaterial
{
	OM_METAL			= 0,	// m�tal
	OM_PLASTIC			= 1,	// plastique
	OM_HUMAN			= 2,	// cosmonaute
	OM_ANIMAL			= 3,	// insecte
	OM_VEGETAL			= 4,	// plante
	OM_MINERAL			= 5,	// pierre
};

typedef struct
{
	char		bUsed;
	int			object;			// num�ro de l'objet dans CD3DEngine
	int			parentPart;		// num�ro de la partie p�re
	int			masterParti;	// canal de la particule ma�tre
	D3DVECTOR	position;
	D3DVECTOR	angle;
	D3DVECTOR	zoom;
	char		bTranslate;
	char		bRotate;
	char		bZoom;
	D3DMATRIX	matTranslate;
	D3DMATRIX	matRotate;
	D3DMATRIX	matTransform;
	D3DMATRIX	matWorld;
}
ObjectPart;

typedef struct
{
	float		wheelFront;		// position X des roues avant
	float		wheelBack;		// position X des roues arri�res
	float		wheelLeft;		// position Z des roues gauches
	float		wheelRight;		// position Z des roues droites
	float		height;			// hauteur normale au-dessus du sol
	D3DVECTOR	posPower;		// position de la pile
}
Character;

typedef struct
{
	char		name[20];		// nom de l'information
	float		value;			// valeur de l'information
}
Info;

enum ExploType
{
	EXPLO_BOUM		= 1,
	EXPLO_BURN		= 2,
	EXPLO_WATER		= 3,
};

enum ResetCap
{
	RESET_NONE		= 0,
	RESET_MOVE		= 1,
	RESET_DELETE	= 2,
};

enum RadarFilter
{
	FILTER_NONE			= 0,
	FILTER_ONLYLANDING	= 1,
	FILTER_ONLYFLYING	= 2,
};




class CObject
{
public:
	CObject(CInstanceManager* iMan);
	~CObject();

	void		DeleteObject(BOOL bAll=FALSE);
	void		Simplify();
	BOOL		ExploObject(ExploType type, float force, float decay=1.0f);

	BOOL		EventProcess(const Event &event);
	void		UpdateMapping();

	int			CreatePart();
	void		DeletePart(int part);
	void		SetObjectRank(int part, int objRank);
	int			RetObjectRank(int part);
	void		SetObjectParent(int part, int parent);
	void		SetType(ObjectType type);
	ObjectType	RetType();
	char*		RetName();
	void		SetOption(int option);
	int			RetOption();

	void		SetID(int id);
	int			RetID();

	BOOL		Write(char *line);
	BOOL		Read(char *line);

	void		SetDrawWorld(BOOL bDraw);
	void		SetDrawFront(BOOL bDraw);

	BOOL		CreateVehicle(D3DVECTOR pos, float angle, ObjectType type, float power, BOOL bTrainer, BOOL bToy);
	BOOL		CreateInsect(D3DVECTOR pos, float angle, ObjectType type);
	BOOL		CreateBuilding(D3DVECTOR pos, float angle, float height, ObjectType type, float power=1.0f);
	BOOL		CreateResource(D3DVECTOR pos, float angle, ObjectType type, float power=1.0f);
	BOOL		CreateFlag(D3DVECTOR pos, float angle, ObjectType type);
	BOOL		CreateBarrier(D3DVECTOR pos, float angle, float height, ObjectType type);
	BOOL		CreatePlant(D3DVECTOR pos, float angle, float height, ObjectType type);
	BOOL		CreateMushroom(D3DVECTOR pos, float angle, float height, ObjectType type);
	BOOL		CreateTeen(D3DVECTOR pos, float angle, float zoom, float height, ObjectType type);
	BOOL		CreateQuartz(D3DVECTOR pos, float angle, float height, ObjectType type);
	BOOL		CreateRoot(D3DVECTOR pos, float angle, float height, ObjectType type);
	BOOL		CreateHome(D3DVECTOR pos, float angle, float height, ObjectType type);
	BOOL		CreateRuin(D3DVECTOR pos, float angle, float height, ObjectType type);
	BOOL		CreateApollo(D3DVECTOR pos, float angle, ObjectType type);

	BOOL		ReadProgram(int rank, char* filename);
	BOOL		WriteProgram(int rank, char* filename);
	BOOL		RunProgram(int rank);

	int			RetShadowLight();
	int			RetEffectLight();

	void		FlushCrashShere();
	int			CreateCrashSphere(D3DVECTOR pos, float radius, Sound sound, float hardness=0.45f);
	int			RetCrashSphereTotal();
	BOOL		GetCrashSphere(int rank, D3DVECTOR &pos, float &radius);
	float		RetCrashSphereHardness(int rank);
	Sound		RetCrashSphereSound(int rank);
	void		DeleteCrashSphere(int rank);
	void		SetGlobalSphere(D3DVECTOR pos, float radius);
	void		GetGlobalSphere(D3DVECTOR &pos, float &radius);
	void		SetJotlerSphere(D3DVECTOR pos, float radius);
	void		GetJotlerSphere(D3DVECTOR &pos, float &radius);
	void		SetShieldRadius(float radius);
	float		RetShieldRadius();

	void		SetFloorHeight(float height);
	void		FloorAdjust();

	void		SetLinVibration(D3DVECTOR dir);
	D3DVECTOR	RetLinVibration();
	void		SetCirVibration(D3DVECTOR dir);
	D3DVECTOR	RetCirVibration();
	void		SetInclinaison(D3DVECTOR dir);
	D3DVECTOR	RetInclinaison();

	void		SetPosition(int part, const D3DVECTOR &pos);
	D3DVECTOR	RetPosition(int part);
	void		SetAngle(int part, const D3DVECTOR &angle);
	D3DVECTOR	RetAngle(int part);
	void		SetAngleY(int part, float angle);
	void		SetAngleX(int part, float angle);
	void		SetAngleZ(int part, float angle);
	float		RetAngleY(int part);
	float		RetAngleX(int part);
	float		RetAngleZ(int part);
	void		SetZoom(int part, float zoom);
	void		SetZoom(int part, D3DVECTOR zoom);
	D3DVECTOR	RetZoom(int part);
	void		SetZoomX(int part, float zoom);
	float		RetZoomX(int part);
	void		SetZoomY(int part, float zoom);
	float		RetZoomY(int part);
	void		SetZoomZ(int part, float zoom);
	float		RetZoomZ(int part);

	float		RetWaterLevel();

	void		SetTrainer(BOOL bEnable);
	BOOL		RetTrainer();

	void		SetToy(BOOL bEnable);
	BOOL		RetToy();

	void		SetManual(BOOL bManual);
	BOOL		RetManual();

	void		SetResetCap(ResetCap cap);
	ResetCap	RetResetCap();
	void		SetResetBusy(BOOL bBusy);
	BOOL		RetResetBusy();
	void		SetResetPosition(const D3DVECTOR &pos);
	D3DVECTOR	RetResetPosition();
	void		SetResetAngle(const D3DVECTOR &angle);
	D3DVECTOR	RetResetAngle();
	void		SetResetRun(int run);
	int			RetResetRun();

	void		SetMasterParticule(int part, int parti);
	int			RetMasterParticule(int part);

	void		SetPower(CObject* power);
	CObject*	RetPower();
	void		SetFret(CObject* fret);
	CObject*	RetFret();
	void		SetTruck(CObject* truck);
	CObject*	RetTruck();
	void		SetTruckPart(int part);
	int			RetTruckPart();

	void		InfoFlush();
	void		DeleteInfo(int rank);
	void		SetInfo(int rank, Info info);
	Info		RetInfo(int rank);
	int			RetInfoTotal();
	void		SetInfoReturn(float value);
	float		RetInfoReturn();
	void		SetInfoUpdate(BOOL bUpdate);
	BOOL		RetInfoUpdate();

	BOOL		SetCmdLine(int rank, float value);
	float		RetCmdLine(int rank);

	D3DMATRIX*	RetRotateMatrix(int part);
	D3DMATRIX*	RetTranslateMatrix(int part);
	D3DMATRIX*	RetTransformMatrix(int part);
	D3DMATRIX*	RetWorldMatrix(int part);

	void		SetViewFromHere(D3DVECTOR &eye, float &dirH, float &dirV, D3DVECTOR	&lookat, D3DVECTOR &upVec, CameraType type);

	void		SetCharacter(Character* character);
	void		GetCharacter(Character* character);
	Character*	RetCharacter();

	float		RetAbsTime();

	void		SetEnergy(float level);
	float		RetEnergy();

	void		SetCapacity(float capacity);
	float		RetCapacity();

	void		SetShield(float level);
	float		RetShield();

	void		SetRange(float delay);
	float		RetRange();

	void		SetTransparency(float value);
	float		RetTransparency();

	ObjectMaterial RetMaterial();

	void		SetGadget(BOOL bMode);
	BOOL		RetGadget();

	void		SetFixed(BOOL bFixed);
	BOOL		RetFixed();

	void		SetClip(BOOL bClip);
	BOOL		RetClip();

	BOOL		JostleObject(float force);

	void		StartDetectEffect(CObject *target, BOOL bFound);

	void		SetVirusMode(BOOL bEnable);
	BOOL		RetVirusMode();
	float		RetVirusTime();

	void		SetCameraType(CameraType type);
	CameraType	RetCameraType();
	void		SetCameraDist(float dist);
	float		RetCameraDist();
	void		SetCameraLock(BOOL bLock);
	BOOL		RetCameraLock();

	void		SetHilite(BOOL bMode);
	BOOL		RetHilite();

	void		SetSelect(BOOL bMode, BOOL bDisplayError=TRUE);
	BOOL		RetSelect(BOOL bReal=FALSE);

	void		SetSelectable(BOOL bMode);
	BOOL		RetSelectable();

	void		SetActivity(BOOL bMode);
	BOOL		RetActivity();

	void		SetVisible(BOOL bVisible);
	BOOL		RetVisible();

	void		SetEnable(BOOL bEnable);
	BOOL		RetEnable();

	void		SetCheckToken(BOOL bMode);
	BOOL		RetCheckToken();

	void		SetProxyActivate(BOOL bActivate);
	BOOL		RetProxyActivate();
	void		SetProxyDistance(float distance);
	float		RetProxyDistance();

	void		SetMagnifyDamage(float factor);
	float		RetMagnifyDamage();

	void		SetParam(float value);
	float		RetParam();

	void		SetExplo(BOOL bExplo);
	BOOL		RetExplo();
	void		SetLock(BOOL bLock);
	BOOL		RetLock();
	void		SetCargo(BOOL bCargo);
	BOOL		RetCargo();
	void		SetBurn(BOOL bBurn);
	BOOL		RetBurn();
	void		SetDead(BOOL bDead);
	BOOL		RetDead();
	BOOL		RetRuin();
	BOOL		RetActif();

	void		SetGunGoalV(float gunGoal);
	void		SetGunGoalH(float gunGoal);
	float		RetGunGoalV();
	float		RetGunGoalH();

	BOOL		StartShowLimit();
	void		StopShowLimit();

	BOOL		IsProgram();
	void		CreateSelectParticule();

	void		SetRunScript(CScript* script);
	CScript*	RetRunScript();
	CBotVar*	RetBotVar();
	CPhysics*	RetPhysics();
	CBrain*		RetBrain();
	CMotion*	RetMotion();
	CAuto*		RetAuto();
	void		SetAuto(CAuto* automat);

	void		SetDefRank(int rank);
	int			RetDefRank();

	BOOL		GetTooltipName(char* name);

	void		AddDeselList(CObject* pObj);
	CObject*	SubDeselList();
	void		DeleteDeselList(CObject* pObj);

	BOOL		CreateShadowCircle(float radius, float intensity, D3DShadowType type=D3DSHADOWNORM);
	BOOL		CreateShadowLight(float height, D3DCOLORVALUE color);
	BOOL		CreateEffectLight(float height, D3DCOLORVALUE color);

	void		FlatParent();

	BOOL		RetTraceDown();
	void		SetTraceDown(BOOL bDown);
	int			RetTraceColor();
	void		SetTraceColor(int color);
	float		RetTraceWidth();
	void		SetTraceWidth(float width);

protected:
	BOOL		EventFrame(const Event &event);
	void		VirusFrame(float rTime);
	void		PartiFrame(float rTime);
	void		CreateOtherObject(ObjectType type);
	void		InitPart(int part);
	void		UpdateTotalPart();
	int			SearchDescendant(int parent, int n);
	void		UpdateEnergyMapping();
	BOOL		UpdateTransformObject(int part, BOOL bForceUpdate);
	BOOL		UpdateTransformObject();
	void		UpdateSelectParticule();

protected:
	CInstanceManager* m_iMan;
	CD3DEngine*		m_engine;
	CLight*			m_light;
	CTerrain*		m_terrain;
	CWater*			m_water;
	CCamera*		m_camera;
	CParticule*		m_particule;
	CPhysics*		m_physics;
	CBrain*			m_brain;
	CMotion*		m_motion;
	CAuto*			m_auto;
	CDisplayText*	m_displayText;
	CRobotMain*		m_main;
	CSound*			m_sound;
	CBotVar*		m_botVar;
	CScript*		m_runScript;

	ObjectType	m_type;				// OBJECT_*
	int			m_id;				// identificateur unique
	char		m_name[50];			// nom de l'objet
	Character	m_character;		// caract�ristiques
	int			m_option;			// option
	int			m_partiReactor;		// num�ro de la particule du r�acteur
	int			m_shadowLight;		// num�ro de la lumi�re de l'ombre
	float		m_shadowHeight;		// hauteur de la lumi�re de l'ombre
	int			m_effectLight;		// num�ro de la lumi�re des effets
	float		m_effectHeight;		// hauteur de la lumi�re des effets
	D3DVECTOR	m_linVibration;		// vibration lin�aire
	D3DVECTOR	m_cirVibration;		// vibration circulaire
	D3DVECTOR	m_inclinaison;		// inclinaison
	CObject*	m_power;			// pile utilis�e par le v�hicule
	CObject*	m_fret;				// objet transport�
	CObject*	m_truck;			// objet portant celui-ci
	int			m_truckLink;		// partie
	float		m_energy;			// �nergie contenue (si pile)
	float		m_lastEnergy;
	float		m_capacity;			// capacit� (si pile)
	float		m_shield;			// bouclier
	float		m_range;			// autonomie de vol
	float		m_transparency;		// transparence (0..1)
	int			m_material;			// mati�re (0..n)
	float		m_aTime;
	float		m_shotTime;			// temps depuis dernier coup
	BOOL		m_bVirusMode;		// virus enclanch�/d�clanch�
	float		m_virusTime;		// temps de vie du virus
	float		m_lastVirusParticule;
	float		m_lastParticule;
	BOOL		m_bHilite;
	BOOL		m_bSelect;			// objet s�lectionn�
	BOOL		m_bSelectable;		// objet s�lectionnable
	BOOL		m_bCheckToken;		// objet avec tokens v�rifi�s
	BOOL		m_bVisible;			// objet actif mais ind�tectable
	BOOL		m_bEnable;			// objet mort
	BOOL		m_bProxyActivate;	// objet activ� si proche
	BOOL		m_bGadget;			// objet non indispensable
	BOOL		m_bLock;
	BOOL		m_bExplo;
	BOOL		m_bCargo;
	BOOL		m_bBurn;
	BOOL		m_bDead;
	BOOL		m_bFlat;
	BOOL		m_bTrainer;			// v�hicule d'entra�nement (sans t�l�commande)
	BOOL		m_bToy;				// jouet � cl�
	BOOL		m_bManual;			// commandes manuelles (Scribbler)
	BOOL		m_bFixed;
	BOOL		m_bClip;
	BOOL		m_bShowLimit;
	float		m_showLimitRadius;
	float		m_gunGoalV;
	float		m_gunGoalH;
	CameraType	m_cameraType;
	float		m_cameraDist;
	BOOL		m_bCameraLock;
	int			m_defRank;
	float		m_magnifyDamage;
	float		m_proxyDistance;
	float		m_param;

	int			m_crashSphereUsed;	// nb de sph�res utilis�es
	D3DVECTOR	m_crashSpherePos[MAXCRASHSPHERE];
	float		m_crashSphereRadius[MAXCRASHSPHERE];
	float		m_crashSphereHardness[MAXCRASHSPHERE];
	Sound		m_crashSphereSound[MAXCRASHSPHERE];
	D3DVECTOR	m_globalSpherePos;
	float		m_globalSphereRadius;
	D3DVECTOR	m_jotlerSpherePos;
	float		m_jotlerSphereRadius;
	float		m_shieldRadius;

	int			m_totalPart;
	ObjectPart	m_objectPart[OBJECTMAXPART];

	int			m_totalDesectList;
	CObject*	m_objectDeselectList[OBJECTMAXDESELLIST];

	int			m_partiSel[4];

	ResetCap	m_resetCap;
	BOOL		m_bResetBusy;
	D3DVECTOR	m_resetPosition;
	D3DVECTOR	m_resetAngle;
	int			m_resetRun;

	int			m_infoTotal;
	Info		m_info[OBJECTMAXINFO];
	float		m_infoReturn;
	BOOL		m_bInfoUpdate;

	float		m_cmdLine[OBJECTMAXCMDLINE];
};


#endif //_OBJECT_H_
