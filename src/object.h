// object.h

#ifndef _OBJECT_H_
#define	_OBJECT_H_


class CInstanceManager;
class CD3DEngine;
class CLight;
class CTerrain;
class CWater;
class CCamera;
class CParticule;
class CMotion;
class CTaskList;
class CAuto;
class CDisplayText;
class CRobotMain;
class CMainUndo;
class CBotVar;
class CScript;

enum CameraType;
enum Sound;
enum D3DShadowType;
enum TaskOrder;
enum LockZone;



// Le père de toutes les parties doit toujours être la partie
// numéro zéro !

#define OBJECTMAXPART		40
#define OBJECTMAXCMDLINE	20

#define ADD_HAT				0
#define ADD_GLASS			1
#define ADD_GLOVE			2
#define ADD_SHOE			3
#define ADD_BAG				4



enum ObjectType
{
	OBJECT_NULL			= 0,	// objet détruit
	OBJECT_FIX			= 1,	// décor fixe
	OBJECT_START		= 32,	// départ
	OBJECT_END			= 33,	// arrivée
	OBJECT_DOOR1		= 40,	// porte 1
	OBJECT_DOOR2		= 41,	// porte 2
	OBJECT_DOOR3		= 42,	// porte 3
	OBJECT_DOOR4		= 43,	// porte 4
	OBJECT_DOOR5		= 44,	// porte 5
	OBJECT_DOCK			= 50,	// transbordeur
	OBJECT_FRET		    = 150,	// transportable
	OBJECT_STONE	    = 151,	// pierre
	OBJECT_URANIUM	    = 152,	// uranium
	OBJECT_METAL	    = 153,	// métal
	OBJECT_BARREL	    = 154,	// tonneau
	OBJECT_BARRELa	    = 155,	// tonneau radioactif
	OBJECT_ATOMIC	    = 156,	// pile atomique
	OBJECT_BULLET	    = 157,	// boulet
	OBJECT_BBOX		    = 158,	// black-box
	OBJECT_TNT			= 159,	// caisse de TNT
	OBJECT_MINE			= 160,	// bombe fixe
	OBJECT_FIOLE		= 161,	// potion magique
	OBJECT_LIFT			= 162,	// ascenseur de départ
	OBJECT_GOAL			= 163,	// ballon d'arrivée
	OBJECT_COLUMN1		= 164,	// pilier
	OBJECT_COLUMN2		= 165,	// pilier
	OBJECT_COLUMN3		= 166,	// pilier
	OBJECT_COLUMN4		= 167,	// pilier
	OBJECT_GLASS1		= 168,	// vitre
	OBJECT_CATAPULT		= 169,	// catapulte
	OBJECT_GLASS2		= 170,	// vitre
	OBJECT_GLU			= 171,	// potion de glu
	OBJECT_BIRD			= 180,	// oiseau
	OBJECT_PTERO		= 181,	// pterodactyl
	OBJECT_FISH			= 182,	// poisson
	OBJECT_SNAKE		= 183,	// serpent
	OBJECT_MAX1X		= 184,	// un seul passage
	OBJECT_SUBM			= 185,	// sous-marin
	OBJECT_JET			= 186,	// avion
	OBJECT_WINFIRE		= 191,	// feu d'artifice
	OBJECT_SHOW			= 192,	// montre un lieu
	OBJECT_MARK			= 196,	// cible pour bot2
	OBJECT_PLANT0		= 200,	// plante 0
	OBJECT_PLANT1		= 201,	// plante 1
	OBJECT_PLANT2		= 202,	// plante 2
	OBJECT_PLANT3		= 203,	// plante 3
	OBJECT_PLANT4		= 204,	// plante 4
	OBJECT_PLANT5		= 205,	// plante 5
	OBJECT_PLANT6		= 206,	// plante 6
	OBJECT_PLANT7		= 207,	// plante 7
	OBJECT_PLANT8		= 208,	// plante 8
	OBJECT_PLANT9		= 209,	// plante 9
	OBJECT_PLANT10		= 210,	// plante 10
	OBJECT_PLANT11		= 211,	// plante 11
	OBJECT_PLANT12		= 212,	// plante 12
	OBJECT_PLANT13		= 213,	// plante 13
	OBJECT_PLANT14		= 214,	// plante 14
	OBJECT_PLANT15		= 215,	// plante 15
	OBJECT_PLANT16		= 216,	// plante 16
	OBJECT_PLANT17		= 217,	// plante 17
	OBJECT_PLANT18		= 218,	// plante 18
	OBJECT_PLANT19		= 219,	// plante 19
	OBJECT_TREE0		= 220,	// arbre 0
	OBJECT_TREE1		= 221,	// arbre 1
	OBJECT_TREE2		= 222,	// arbre 2
	OBJECT_TREE3		= 223,	// arbre 3
	OBJECT_TREE4		= 224,	// arbre 4
	OBJECT_TREE5		= 225,	// arbre 5
	OBJECT_TREE6		= 226,	// arbre 6
	OBJECT_TREE7		= 227,	// arbre 7
	OBJECT_TREE8		= 228,	// arbre 8
	OBJECT_TREE9		= 229,	// arbre 9
	OBJECT_TRAX         = 300,	// trax pousseur
	OBJECT_PERFO        = 301,	// trax perforateur
	OBJECT_GUN	        = 302,	// trax canon
	OBJECT_MOBILEtg	    = 303,	// cible d'exercice
	OBJECT_MOBILEfb	    = 304,	// fireball
	OBJECT_MOBILEob	    = 305,	// orgaball
	OBJECT_BLUPI		= 310,	// blupi
	OBJECT_BOT1			= 320,	// robot 1
	OBJECT_BOT2			= 321,	// robot 2
	OBJECT_BOT3			= 322,	// robot 3
	OBJECT_BOT4			= 323,	// robot 4
	OBJECT_BOT5			= 324,	// robot 5
	OBJECT_CARROT	    = 340,	// carotte à suivre
	OBJECT_WALKER	    = 342,	// marcheur
	OBJECT_CRAZY	    = 343,	// fou
	OBJECT_WAYPOINT 	= 350,	// chemin
	OBJECT_SCRAP0		= 360,	// déchet flottant
	OBJECT_SCRAP1		= 361,	// déchet flottant
	OBJECT_SCRAP2		= 362,	// déchet flottant
	OBJECT_SCRAP3		= 363,	// déchet flottant
	OBJECT_SCRAP4		= 364,	// déchet flottant
	OBJECT_SCRAP5		= 365,	// déchet flottant
	OBJECT_SCRAP6		= 366,	// déchet flottant
	OBJECT_SCRAP7		= 367,	// déchet flottant
	OBJECT_SCRAP8		= 368,	// déchet flottant
	OBJECT_SCRAP9		= 369,	// déchet flottant
	OBJECT_BARRIER0		= 400,	// barrière
	OBJECT_BARRIER1		= 401,	// barrière
	OBJECT_BARRIER2		= 402,	// barrière
	OBJECT_BARRIER3		= 403,	// barrière
	OBJECT_BARRIER4		= 404,	// barrière
	OBJECT_BARRIER5		= 405,	// barrière
	OBJECT_BARRIER6		= 406,	// barrière
	OBJECT_BARRIER7		= 407,	// barrière
	OBJECT_BARRIER8		= 408,	// barrière
	OBJECT_BARRIER9		= 409,	// barrière
	OBJECT_BARRIER10	= 410,	// barrière
	OBJECT_BARRIER11	= 411,	// barrière
	OBJECT_BARRIER12	= 412,	// barrière
	OBJECT_BARRIER13	= 413,	// barrière
	OBJECT_BARRIER14	= 414,	// barrière
	OBJECT_BARRIER15	= 415,	// barrière
	OBJECT_BARRIER16	= 416,	// barrière
	OBJECT_BARRIER17	= 417,	// barrière
	OBJECT_BARRIER18	= 418,	// barrière
	OBJECT_BARRIER19	= 419,	// barrière
	OBJECT_BARRIER20	= 420,	// barrière
	OBJECT_BARRIER21	= 421,	// barrière
	OBJECT_BARRIER22	= 422,	// barrière
	OBJECT_BARRIER23	= 423,	// barrière
	OBJECT_BARRIER24	= 424,	// barrière
	OBJECT_BARRIER25	= 425,	// barrière
	OBJECT_BARRIER26	= 426,	// barrière
	OBJECT_BARRIER27	= 427,	// barrière
	OBJECT_BARRIER28	= 428,	// barrière
	OBJECT_BARRIER29	= 429,	// barrière
	OBJECT_BARRIER30	= 430,	// barrière
	OBJECT_BARRIER31	= 431,	// barrière
	OBJECT_BARRIER32	= 432,	// barrière
	OBJECT_BARRIER33	= 433,	// barrière
	OBJECT_BARRIER34	= 434,	// barrière
	OBJECT_BARRIER35	= 435,	// barrière
	OBJECT_BARRIER36	= 436,	// barrière
	OBJECT_BARRIER37	= 437,	// barrière
	OBJECT_BARRIER38	= 438,	// barrière
	OBJECT_BARRIER39	= 439,	// barrière
	OBJECT_BARRIER40	= 440,	// barrière
	OBJECT_BARRIER41	= 441,	// barrière
	OBJECT_BARRIER42	= 442,	// barrière
	OBJECT_BARRIER43	= 443,	// barrière
	OBJECT_BARRIER44	= 444,	// barrière
	OBJECT_BARRIER45	= 445,	// barrière
	OBJECT_BARRIER46	= 446,	// barrière
	OBJECT_BARRIER47	= 447,	// barrière
	OBJECT_BARRIER48	= 448,	// barrière
	OBJECT_BARRIER49	= 449,	// barrière
	OBJECT_BARRIER50	= 450,	// barrière
	OBJECT_BARRIER51	= 451,	// barrière
	OBJECT_BARRIER52	= 452,	// barrière
	OBJECT_BARRIER53	= 453,	// barrière
	OBJECT_BARRIER54	= 454,	// barrière
	OBJECT_BARRIER55	= 455,	// barrière
	OBJECT_BARRIER56	= 456,	// barrière
	OBJECT_BARRIER57	= 457,	// barrière
	OBJECT_BARRIER58	= 458,	// barrière
	OBJECT_BARRIER59	= 459,	// barrière
	OBJECT_BARRIER60	= 460,	// barrière
	OBJECT_BARRIER61	= 461,	// barrière
	OBJECT_BARRIER62	= 462,	// barrière
	OBJECT_BARRIER63	= 463,	// barrière
	OBJECT_BARRIER64	= 464,	// barrière
	OBJECT_BARRIER65	= 465,	// barrière
	OBJECT_BARRIER66	= 466,	// barrière
	OBJECT_BARRIER67	= 467,	// barrière
	OBJECT_BARRIER68	= 468,	// barrière
	OBJECT_BARRIER69	= 469,	// barrière
	OBJECT_BARRIER70	= 470,	// barrière
	OBJECT_BARRIER71	= 471,	// barrière
	OBJECT_BARRIER72	= 472,	// barrière
	OBJECT_BARRIER73	= 473,	// barrière
	OBJECT_BARRIER74	= 474,	// barrière
	OBJECT_BARRIER75	= 475,	// barrière
	OBJECT_BARRIER76	= 476,	// barrière
	OBJECT_BARRIER77	= 477,	// barrière
	OBJECT_BARRIER78	= 478,	// barrière
	OBJECT_BARRIER79	= 479,	// barrière
	OBJECT_BARRIER80	= 480,	// barrière
	OBJECT_BARRIER81	= 481,	// barrière
	OBJECT_BARRIER82	= 482,	// barrière
	OBJECT_BARRIER83	= 483,	// barrière
	OBJECT_BARRIER84	= 484,	// barrière
	OBJECT_BARRIER85	= 485,	// barrière
	OBJECT_BARRIER86	= 486,	// barrière
	OBJECT_BARRIER87	= 487,	// barrière
	OBJECT_BARRIER88	= 488,	// barrière
	OBJECT_BARRIER89	= 489,	// barrière
	OBJECT_BARRIER90	= 490,	// barrière
	OBJECT_BARRIER91	= 491,	// barrière
	OBJECT_BARRIER92	= 492,	// barrière
	OBJECT_BARRIER93	= 493,	// barrière
	OBJECT_BARRIER94	= 494,	// barrière
	OBJECT_BARRIER95	= 495,	// barrière
	OBJECT_BARRIER96	= 496,	// barrière
	OBJECT_BARRIER97	= 497,	// barrière
	OBJECT_BARRIER98	= 498,	// barrière
	OBJECT_BARRIER99	= 499,	// barrière
	OBJECT_BOX1			= 500,	// caisse
	OBJECT_BOX2			= 501,	// caisse
	OBJECT_BOX3			= 502,	// caisse
	OBJECT_BOX4			= 503,	// caisse
	OBJECT_BOX5			= 504,	// caisse
	OBJECT_BOX6			= 505,	// caisse
	OBJECT_BOX7			= 506,	// sphère
	OBJECT_BOX8			= 507,	// colonne
	OBJECT_BOX9			= 508,	// caisse light
	OBJECT_BOX10		= 509,	// bombe
	OBJECT_BOX11		= 510,	// caisse x-x
	OBJECT_BOX12		= 511,	// caisse z-z
	OBJECT_BOX13		= 512,	// caisse à coussins d'air (cca)
	OBJECT_BOX14		= 513,	// 
	OBJECT_BOX15		= 514,	// 
	OBJECT_BOX16		= 515,	// 
	OBJECT_BOX17		= 516,	// 
	OBJECT_BOX18		= 517,	// 
	OBJECT_BOX19		= 518,	// 
	OBJECT_BOX20		= 519,	// 
	OBJECT_STONE1		= 530,	// pierre
	OBJECT_STONE2		= 531,	// pierre
	OBJECT_STONE3		= 532,	// pierre
	OBJECT_STONE4		= 533,	// pierre
	OBJECT_STONE5		= 534,	// pierre
	OBJECT_STONE6		= 535,	// pierre
	OBJECT_STONE7		= 536,	// pierre
	OBJECT_STONE8		= 537,	// pierre
	OBJECT_STONE9		= 538,	// pierre
	OBJECT_STONE10		= 539,	// pierre
	OBJECT_KEY1			= 550,	// clé
	OBJECT_KEY2			= 551,	// clé
	OBJECT_KEY3			= 552,	// clé
	OBJECT_KEY4			= 553,	// clé
	OBJECT_KEY5			= 554,	// clé
	OBJECT_GROUND0		= 600,	// sol spécial
	OBJECT_GROUND1		= 601,	// sol spécial
	OBJECT_GROUND2		= 602,	// sol spécial
	OBJECT_GROUND3		= 603,	// sol spécial
	OBJECT_GROUND4		= 604,	// sol spécial
	OBJECT_GROUND5		= 605,	// sol spécial
	OBJECT_GROUND6		= 606,	// sol spécial
	OBJECT_GROUND7		= 607,	// sol spécial
	OBJECT_GROUND8		= 608,	// sol spécial
	OBJECT_GROUND9		= 609,	// sol spécial
	OBJECT_GROUND10		= 610,	// sol spécial
	OBJECT_GROUND11		= 611,	// sol spécial
	OBJECT_GROUND12		= 612,	// sol spécial
	OBJECT_GROUND13		= 613,	// sol spécial
	OBJECT_GROUND14		= 614,	// sol spécial
	OBJECT_GROUND15		= 615,	// sol spécial
	OBJECT_GROUND16		= 616,	// sol spécial
	OBJECT_GROUND17		= 617,	// sol spécial
	OBJECT_GROUND18		= 618,	// sol spécial
	OBJECT_GROUND19		= 619,	// sol spécial
	OBJECT_RUINmobilew1 = 700,	// ruine 1
	OBJECT_RUINmobilew2 = 701,	// ruine 1
	OBJECT_RUINmobilet1 = 702,	// ruine 2
	OBJECT_RUINmobilet2 = 703,	// ruine 2
	OBJECT_RUINmobiler1 = 704,	// ruine 3
	OBJECT_RUINmobiler2 = 705,	// ruine 3
	OBJECT_RUINfactory  = 706,	// ruine 4
	OBJECT_RUINdoor     = 707,	// ruine 5
	OBJECT_RUINsupport  = 708,	// ruine 6
	OBJECT_RUINradar    = 709,	// ruine 7
	OBJECT_RUINconvert  = 710,	// ruine 8
	OBJECT_RUINbase	    = 711,	// ruine 9
	OBJECT_RUINhead	    = 712,	// ruine 10
	OBJECT_QUARTZ0		= 800,	// quartz 0
	OBJECT_QUARTZ1		= 801,	// quartz 1
	OBJECT_QUARTZ2		= 802,	// quartz 2
	OBJECT_QUARTZ3		= 803,	// quartz 3
	OBJECT_QUARTZ4		= 804,	// quartz 4
	OBJECT_QUARTZ5		= 805,	// quartz 5
	OBJECT_QUARTZ6		= 806,	// quartz 6
	OBJECT_QUARTZ7		= 807,	// quartz 7
	OBJECT_QUARTZ8		= 808,	// quartz 8
	OBJECT_QUARTZ9		= 809,	// quartz 9
	OBJECT_ROOT0		= 810,	// racine 0
	OBJECT_ROOT1		= 811,	// racine 1
	OBJECT_ROOT2		= 812,	// racine 2
	OBJECT_ROOT3		= 813,	// racine 3
	OBJECT_ROOT4		= 814,	// racine 4
	OBJECT_ROOT5		= 815,	// racine 5
	OBJECT_ROOT6		= 816,	// racine 6
	OBJECT_ROOT7		= 817,	// racine 7
	OBJECT_ROOT8		= 818,	// racine 8
	OBJECT_ROOT9		= 819,	// racine 9
	OBJECT_SEAWEED0		= 820,	// algue 0
	OBJECT_SEAWEED1		= 821,	// algue 1
	OBJECT_SEAWEED2		= 822,	// algue 2
	OBJECT_SEAWEED3		= 823,	// algue 3
	OBJECT_SEAWEED4		= 824,	// algue 4
	OBJECT_SEAWEED5		= 825,	// algue 5
	OBJECT_SEAWEED6		= 826,	// algue 6
	OBJECT_SEAWEED7		= 827,	// algue 7
	OBJECT_SEAWEED8		= 828,	// algue 8
	OBJECT_SEAWEED9		= 829,	// algue 9
	OBJECT_MUSHROOM0	= 830,	// champignon 0
	OBJECT_MUSHROOM1	= 831,	// champignon 1
	OBJECT_MUSHROOM2	= 832,	// champignon 2
	OBJECT_MUSHROOM3	= 833,	// champignon 3
	OBJECT_MUSHROOM4	= 834,	// champignon 4
	OBJECT_MUSHROOM5	= 835,	// champignon 5
	OBJECT_MUSHROOM6	= 836,	// champignon 6
	OBJECT_MUSHROOM7	= 837,	// champignon 7
	OBJECT_MUSHROOM8	= 838,	// champignon 8
	OBJECT_MUSHROOM9	= 839,	// champignon 9
	OBJECT_BUILDING1	= 940,	// bâtiment
	OBJECT_BUILDING2	= 941,	// bâtiment
	OBJECT_BUILDING3	= 942,	// bâtiment
	OBJECT_BUILDING4	= 943,	// bâtiment
	OBJECT_BUILDING5	= 944,	// bâtiment
	OBJECT_BUILDING6	= 945,	// bâtiment
	OBJECT_BUILDING7	= 946,	// bâtiment
	OBJECT_BUILDING8	= 947,	// bâtiment
	OBJECT_BUILDING9	= 948,	// bâtiment
	OBJECT_BUILDING10	= 949,	// bâtiment
	OBJECT_CARCASS1	    = 950,	// carcasse
	OBJECT_CARCASS2	    = 951,	// carcasse
	OBJECT_CARCASS3	    = 952,	// carcasse
	OBJECT_CARCASS4	    = 953,	// carcasse
	OBJECT_CARCASS5	    = 954,	// carcasse
	OBJECT_CARCASS6	    = 955,	// carcasse
	OBJECT_CARCASS7	    = 956,	// carcasse
	OBJECT_CARCASS8	    = 957,	// carcasse
	OBJECT_CARCASS9	    = 958,	// carcasse
	OBJECT_CARCASS10	= 959,	// carcasse
	OBJECT_ORGA1	    = 960,	// organique
	OBJECT_ORGA2	    = 961,	// organique
	OBJECT_ORGA3	    = 962,	// organique
	OBJECT_ORGA4	    = 963,	// organique
	OBJECT_ORGA5	    = 964,	// organique
	OBJECT_ORGA6	    = 965,	// organique
	OBJECT_ORGA7	    = 966,	// organique
	OBJECT_ORGA8	    = 967,	// organique
	OBJECT_ORGA9	    = 968,	// organique
	OBJECT_ORGA10		= 969,	// organique
	OBJECT_MAX			= 1000,
};

typedef struct
{
	char		bUsed;
	char		bHide;
	int			objRank;		// numéro de l'objet dans CD3DEngine
	int			parentPart;		// numéro de la partie père
	int			masterParti;	// canal de la particule maître
	D3DVECTOR	position;
	D3DVECTOR	angle;
	D3DVECTOR	zoom;
	char		bVarTex;
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
	float		height;			// hauteur normale au-dessus du sol
	D3DVECTOR	camera;			// position caméra en mode CAMERA_ONBOARD
	float		mass;			// poid du véhicule
}
Character;

enum ExploType
{
	EXPLO_BOUM		= 1,
	EXPLO_BURN		= 2,
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
	BOOL		ExploObject(ExploType type, float force, D3DVECTOR impact=D3DVECTOR(NAN,NAN,NAN));
	BOOL		DetachPart(int part, D3DVECTOR speed=D3DVECTOR(0.0f, 0.0f, 0.0f));

	BOOL		EventProcess(const Event &event);
	void		ShowActions(BOOL bShow, int part, float time);
	BOOL		IsAction(int part);
	void		UpdateMapping();

	int			CreatePart();
	void		DeletePart(int part);
	void		SetObjectRank(int part, int objRank);
	int			RetObjectRank(int part);
	void		SetObjectParent(int part, int parent);
	void		SetHide(int part, BOOL bHide);
	BOOL		RetHide(int part);
	void		SetType(ObjectType type);
	ObjectType	RetType();
	char*		RetName();
	void		SetOption(int option);
	int			RetOption();
	void		SetAdditional(int type, int add);
	int			RetAdditional(int type);
	void		NoDetect();
	void		SetVarTex(int part);
	void		SetVarTex(int part, BOOL bVar);

	void		SetID(int id);
	int			RetID();

	void		SetDrawWorld(BOOL bDraw);
	void		SetDrawFront(BOOL bDraw);

	BOOL		CreateObject(D3DVECTOR pos, float angle, float zoom, float height, ObjectType type, int option=0, int addHat=0, int addGlass=0, int addGlove=0, int addShoe=0, int addBag=0);

	void		CreateLockZone(int dx, int dz, LockZone type);

	void		SetFloorHeight(float height);
	void		FloorAdjust();

	void		SetLinVibration(D3DVECTOR dir);
	D3DVECTOR	RetLinVibration();
	void		SetCirVibration(D3DVECTOR dir);
	D3DVECTOR	RetCirVibration();
	void		SetCirChoc(D3DVECTOR dir);
	D3DVECTOR	RetCirChoc();
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

	void		SetShield(float level);
	float		RetShield();

	void		SetBurnShield(float level);
	float		RetBurnShield();

	void		SetStrong(float level);
	float		RetStrong();
	void		SetFuturStrong(float level);
	float		RetFuturStrong();

	void		SetTransparency(float value);
	float		RetTransparency();

	void		SetGadget(BOOL bMode);
	BOOL		RetGadget();

	void		SetFixed(BOOL bFixed);
	BOOL		RetFixed();

	void		SetClip(BOOL bClip);
	BOOL		RetClip();

	BOOL		JostleObject(float force);

	void		SetCameraType(CameraType type);
	CameraType	RetCameraType();
	void		SetCameraDist(float dist);
	float		RetCameraDist();
	void		SetCameraLock(BOOL bLock);
	BOOL		RetCameraLock();

	void		SetFlash(int part);

	void		SetHilite(BOOL bMode, BOOL bSelectable);
	BOOL		RetHilite();

	void		SetSelect(BOOL bMode, BOOL bDisplayError=TRUE);
	BOOL		RetSelect(BOOL bReal=FALSE);

	void		SetSelectable(BOOL bMode);
	BOOL		RetSelectable();

	void		SetEnable(BOOL bEnable);
	BOOL		RetEnable();

	void		SetGhost(BOOL bGhost);
	BOOL		RetGhost();

	void		SetGround(BOOL bGround);
	BOOL		RetGround();

	void		SetMagnifyDamage(float factor);
	float		RetMagnifyDamage();

	void		SetParam(float value);
	float		RetParam();

	void		SetExplo(BOOL bExplo);
	BOOL		RetExplo();
	void		SetNoUndoable(BOOL bNoUndo);
	BOOL		RetNoUndoable();
	void		SetLock(BOOL bLock);
	BOOL		RetLock();
	void		SetBurn(BOOL bBurn);
	BOOL		RetBurn();
	void		SetDead(BOOL bDead);
	BOOL		RetDead();
	BOOL		RetRuin();
	BOOL		RetActif();

	float		RetHeight();

	void		SetRunScript(CScript* script);
	CScript*	RetRunScript();
	CBotVar*	RetBotVar();
	CMotion*	RetMotion();
	CTaskList*	RetTaskList();
	CAuto*		RetAuto();
	void		SetAuto(CAuto* automat);
	BOOL		StartTaskList(TaskOrder order, D3DVECTOR pos, CObject *target, int part, float param);

	BOOL		GetTooltipName(int part, char* name);

	BOOL		CreateShadow(float radius, float intensity, D3DShadowType type=D3DSHADOWNORM, BOOL bTerrainHole=FALSE, float sunFactor=0.0f);
	void		AdjustShadow(D3DVECTOR pos);
	void		SetAdjustShadow(BOOL bAdjust);
	BOOL		RetAdjustShadow();

	void		FlatParent();
	BOOL		FlatParent(int part);
	BOOL		TerrainEmbedded();

	void		WriteSituation();
	void		ReadSituation();

protected:
	BOOL		CreateVehicle(D3DVECTOR pos, float angle, float zoom, ObjectType type);
	BOOL		CreateBuilding(D3DVECTOR pos, float angle, float zoom, float height, ObjectType type);
	BOOL		CreateResource(D3DVECTOR pos, float angle, float zoom, ObjectType type);
	BOOL		CreateBlupi(D3DVECTOR pos, float angle, float zoom, ObjectType type);
	BOOL		CreateBot(D3DVECTOR pos, float angle, float zoom, ObjectType type);
	BOOL		CreateAnimal(D3DVECTOR pos, float angle, float zoom, ObjectType type);
	BOOL		CreateSpecial(D3DVECTOR pos, float angle, float zoom, ObjectType type);
	BOOL		CreateBarrier(D3DVECTOR pos, float angle, float zoom, float height, ObjectType type);
	BOOL		CreateBox(D3DVECTOR pos, float angle, float zoom, float height, ObjectType type);
	BOOL		CreateGround(D3DVECTOR pos, float angle, ObjectType type);
	BOOL		CreateStone(D3DVECTOR pos, float angle, float zoom, float height, ObjectType type);
	BOOL		CreatePlant(D3DVECTOR pos, float angle, float zoom, float height, ObjectType type);
	BOOL		CreateMushroom(D3DVECTOR pos, float angle, float zoom, float height, ObjectType type);
	BOOL		CreateQuartz(D3DVECTOR pos, float angle, float zoom, float height, ObjectType type);
	BOOL		CreateRoot(D3DVECTOR pos, float angle, float zoom, float height, ObjectType type);
	BOOL		CreateRuin(D3DVECTOR pos, float angle, float zoom, float height, ObjectType type);

	BOOL		EventFrame(const Event &event);
	void		PartiFrame(float rTime);
	void		CreateAuto(ObjectType type);
	void		InitPart(int part);
	void		UpdateTotalPart();
	int			SearchDescendant(int parent, int n);
	BOOL		UpdateTransformObject(int part, BOOL bForceUpdate);
	BOOL		UpdateTransformObject();

protected:
	CInstanceManager* m_iMan;
	CD3DEngine*		m_engine;
	CLight*			m_light;
	CTerrain*		m_terrain;
	CWater*			m_water;
	CCamera*		m_camera;
	CParticule*		m_particule;
	CMotion*		m_motion;
	CTaskList*		m_taskList;
	CAuto*			m_auto;
	CDisplayText*	m_displayText;
	CRobotMain*		m_main;
	CMainUndo*		m_undo;
	CSound*			m_sound;
	CBotVar*		m_botVar;
	CScript*		m_runScript;

	ObjectType	m_type;				// OBJECT_*
	int			m_id;				// identificateur unique
	Character	m_character;		// caractéristiques
	int			m_option;			// option
	int			m_additional[10];	// accessoire
	float		m_shadowHeight;		// hauteur de la lumière de l'ombre
	D3DVECTOR	m_linVibration;		// vibration linéaire
	D3DVECTOR	m_cirVibration;		// vibration circulaire
	D3DVECTOR	m_cirChoc;			// rotation suite à un choc
	D3DVECTOR	m_inclinaison;		// inclinaison
	CObject*	m_power;			// pile utilisée par le véhicule
	CObject*	m_fret;				// objet transporté
	CObject*	m_truck;			// objet portant celui-ci
	int			m_truckLink;		// partie
	float		m_shield;			// bouclier
	float		m_burnShield;		// bouclier de résistance au feu
	float		m_strong;			// force (0..1)
	float		m_futurStrong;		// force (0..1)
	float		m_transparency;		// transparence (0..1)
	float		m_aTime;
	float		m_shotTime;			// temps depuis dernier coup
	float		m_lastParticule;
	BOOL		m_bHilite;
	BOOL		m_bSelect;			// objet sélectionné
	BOOL		m_bSelectable;		// objet sélectionnable
	BOOL		m_bEnable;			// objet mort
	BOOL		m_bGadget;			// objet non indispensable
	BOOL		m_bLock;
	BOOL		m_bExplo;
	BOOL		m_bNoUndoable;
	BOOL		m_bBurn;
	BOOL		m_bDead;
	BOOL		m_bFlat;
	BOOL		m_bTrainer;			// véhicule d'entraînement (sans télécommande)
	BOOL		m_bFixed;
	BOOL		m_bClip;
	BOOL		m_bGhost;
	BOOL		m_bGround;
	CameraType	m_cameraType;
	float		m_cameraDist;
	BOOL		m_bCameraLock;
	float		m_magnifyDamage;
	float		m_param;
	float		m_flashTime;
	float		m_flashDelay;
	int			m_flashPart;
	int			m_generation;

	BOOL		m_bAdjustShadow;
	BOOL		m_bTerrainHole;
	D3DVECTOR	m_lastPosTerrain;	// centre pour l'ombre

	int			m_arrowMode;
	float		m_arrowPos;

	int			m_totalPart;
	ObjectPart	m_objectPart[OBJECTMAXPART];

	float		m_cmdLine[OBJECTMAXCMDLINE];
};


#endif //_OBJECT_H_
