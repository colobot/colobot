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
class CPhysics;
class CBrain;
class CMotion;
class CAuto;
class CDisplayText;
class CRobotMain;
class CBotVar;
class CScript;
class CRecorder;

enum CameraType;
enum Sound;
enum D3DShadowType;



// Le père de toutes les parties doit toujours être la partie
// numéro zéro !

#define OBJECTMAXPART		40
#define MAXCRASHSPHERE		40
#define MAXCRASHLINE		20
#define OBJECTMAXCMDLINE	20

enum ObjectType
{
	OBJECT_NULL			= 0,	// objet détruit
	OBJECT_FIX			= 1,	// décor fixe
	OBJECT_PIECE		= 2,	// pièce de voiture
	OBJECT_FACTORY1		= 10,	// usine fixe
	OBJECT_FACTORY2		= 11,	// usine fixe
	OBJECT_FACTORY3		= 12,	// usine fixe
	OBJECT_FACTORY4		= 13,	// usine fixe
	OBJECT_FACTORY5		= 14,	// usine fixe
	OBJECT_FACTORY6		= 15,	// usine fixe
	OBJECT_FACTORY7		= 16,	// usine fixe
	OBJECT_FACTORY8		= 17,	// usine fixe
	OBJECT_FACTORY9		= 18,	// usine fixe
	OBJECT_FACTORY10	= 19,	// usine fixe
	OBJECT_FACTORY11	= 20,	// usine fixe
	OBJECT_FACTORY12	= 21,	// usine fixe
	OBJECT_FACTORY13	= 22,	// usine fixe
	OBJECT_FACTORY14	= 23,	// usine fixe
	OBJECT_FACTORY15	= 24,	// usine fixe
	OBJECT_FACTORY16	= 25,	// usine fixe
	OBJECT_FACTORY17	= 26,	// usine fixe
	OBJECT_FACTORY18	= 27,	// usine fixe
	OBJECT_FACTORY19	= 28,	// usine fixe
	OBJECT_FACTORY20	= 29,	// usine fixe
	OBJECT_TOWER		= 30,	// tour de défense
	OBJECT_NUCLEAR		= 31,	// centrale nucléaire
	OBJECT_START		= 32,	// départ
	OBJECT_END			= 33,	// arrivée
	OBJECT_PARA			= 34,	// paratonnerre
	OBJECT_SUPPORT		= 35,	// présentoir
	OBJECT_DOCK			= 36,	// portique
	OBJECT_REMOTE		= 37,	// télécommande
	OBJECT_STAND		= 38,	// tribune
	OBJECT_GENERATOR	= 39,	// générateur
	OBJECT_DOOR1		= 40,	// porte 1
	OBJECT_DOOR2		= 41,	// porte 2
	OBJECT_DOOR3		= 42,	// porte 3
	OBJECT_DOOR4		= 43,	// porte 4
	OBJECT_DOOR5		= 44,	// porte 5
	OBJECT_COMPUTER		= 45,	// ordi
	OBJECT_REPAIR		= 46,	// center de réparation
	OBJECT_SWEET		= 47,	// maisonette
	OBJECT_ROADSIGN1	= 60,	// signal 1
	OBJECT_ROADSIGN2	= 61,	// signal 2
	OBJECT_ROADSIGN3	= 62,	// signal 3
	OBJECT_ROADSIGN4	= 63,	// signal 4
	OBJECT_ROADSIGN5	= 64,	// signal 5
	OBJECT_ROADSIGN6	= 65,	// signal 6
	OBJECT_ROADSIGN7	= 66,	// signal 7
	OBJECT_ROADSIGN8	= 67,	// signal 8
	OBJECT_ROADSIGN9	= 68,	// signal 9
	OBJECT_ROADSIGN10	= 69,	// signal 10
	OBJECT_ROADSIGN11	= 70,	// signal 11
	OBJECT_ROADSIGN12	= 71,	// signal 12
	OBJECT_ROADSIGN13	= 72,	// signal 13
	OBJECT_ROADSIGN14	= 73,	// signal 14
	OBJECT_ROADSIGN15	= 74,	// signal 15
	OBJECT_ROADSIGN16	= 75,	// signal 16
	OBJECT_ROADSIGN17	= 76,	// signal 17
	OBJECT_ROADSIGN18	= 77,	// signal 18
	OBJECT_ROADSIGN19	= 78,	// signal 19
	OBJECT_ROADSIGN20	= 79,	// signal 20
	OBJECT_ROADSIGN21	= 80,	// signal 21
	OBJECT_ROADSIGN22	= 81,	// signal 22
	OBJECT_ROADSIGN23	= 82,	// signal 23
	OBJECT_ROADSIGN24	= 83,	// signal 24
	OBJECT_ROADSIGN25	= 84,	// signal 25
	OBJECT_ROADSIGN26	= 85,	// signal 26
	OBJECT_ROADSIGN27	= 86,	// signal 27
	OBJECT_ROADSIGN28	= 87,	// signal 28
	OBJECT_ROADSIGN29	= 88,	// signal 29
	OBJECT_ROADSIGN30	= 89,	// signal 30
	OBJECT_PUB11		= 100,	// pub
	OBJECT_PUB12		= 101,	// pub
	OBJECT_PUB13		= 102,	// pub
	OBJECT_PUB14		= 103,	// pub
	OBJECT_PUB21		= 104,	// pub
	OBJECT_PUB22		= 105,	// pub
	OBJECT_PUB23		= 106,	// pub
	OBJECT_PUB24		= 107,	// pub
	OBJECT_PUB31		= 108,	// pub
	OBJECT_PUB32		= 109,	// pub
	OBJECT_PUB33		= 110,	// pub
	OBJECT_PUB34		= 111,	// pub
	OBJECT_PUB41		= 112,	// pub
	OBJECT_PUB42		= 113,	// pub
	OBJECT_PUB43		= 114,	// pub
	OBJECT_PUB44		= 115,	// pub
	OBJECT_PUB51		= 116,	// pub
	OBJECT_PUB52		= 117,	// pub
	OBJECT_PUB53		= 118,	// pub
	OBJECT_PUB54		= 119,	// pub
	OBJECT_PUB61		= 120,	// pub
	OBJECT_PUB62		= 121,	// pub
	OBJECT_PUB63		= 122,	// pub
	OBJECT_PUB64		= 123,	// pub
	OBJECT_PUB71		= 124,	// pub
	OBJECT_PUB72		= 125,	// pub
	OBJECT_PUB73		= 126,	// pub
	OBJECT_PUB74		= 127,	// pub
	OBJECT_PUB81		= 128,	// pub
	OBJECT_PUB82		= 129,	// pub
	OBJECT_PUB83		= 130,	// pub
	OBJECT_PUB84		= 131,	// pub
	OBJECT_PUB91		= 132,	// pub
	OBJECT_PUB92		= 133,	// pub
	OBJECT_PUB93		= 134,	// pub
	OBJECT_PUB94		= 135,	// pub
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
	OBJECT_POLE			= 161,	// poteau
	OBJECT_CONE			= 162,	// cône
	OBJECT_AQUA			= 163,	// aquarium
	OBJECT_PIPES		= 164,	// tuyaux
	OBJECT_GRAVEL		= 165,	// tas de gravier
	OBJECT_TUB			= 166,	// bac de gravier
	OBJECT_FIRE			= 170,	// lance d'incendie
	OBJECT_HELICO		= 171,	// hélicoptère
	OBJECT_COMPASS		= 172,	// boussole
	OBJECT_BLITZER		= 173,	// électrocuteur
	OBJECT_HOOK			= 174,	// crochet
	OBJECT_TOYS1		= 180,	// jouet 1
	OBJECT_TOYS2		= 181,	// jouet 2
	OBJECT_TOYS3		= 182,	// jouet 3
	OBJECT_TOYS4		= 183,	// jouet 4
	OBJECT_TOYS5		= 184,	// jouet 5
	OBJECT_BOMB			= 190,	// bombe transportable
	OBJECT_WINFIRE		= 191,	// feu d'artifice
	OBJECT_SHOW			= 192,	// montre un lieu
	OBJECT_BAG			= 193,	// sac de survie
	OBJECT_CROSS1		= 195,	// croisement
	OBJECT_MARK			= 196,	// cible pour bot2
	OBJECT_CROWN		= 197,	// couronne
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
	OBJECT_CAR			= 250,	// voiture
	OBJECT_MOBILEtg	    = 300,	// cible d'exercice
	OBJECT_MOBILEfb	    = 301,	// fireball
	OBJECT_MOBILEob	    = 302,	// orgaball
	OBJECT_TRAX		    = 303,	// trax
	OBJECT_TRAXf	    = 304,	// trax fixe
	OBJECT_UFO		    = 305,	// ufo
	OBJECT_BOT1			= 320,	// robot 1
	OBJECT_BOT2			= 321,	// robot 2
	OBJECT_BOT3			= 322,	// robot 3
	OBJECT_BOT4			= 323,	// robot 4
	OBJECT_BOT5			= 324,	// robot 5
	OBJECT_EVIL1		= 330,	// méchant 1
	OBJECT_EVIL2		= 331,	// méchant 2
	OBJECT_EVIL3		= 332,	// méchant 3
	OBJECT_EVIL4		= 333,	// méchant 4
	OBJECT_EVIL5		= 334,	// méchant 5
	OBJECT_CARROT	    = 340,	// carotte à suivre
	OBJECT_STARTER	    = 341,	// starter
	OBJECT_WALKER	    = 342,	// marcheur
	OBJECT_CRAZY	    = 343,	// fou
	OBJECT_GUIDE	    = 344,	// guide
	OBJECT_WAYPOINT 	= 350,	// chemin
	OBJECT_TRAJECT	 	= 351,	// trajectoire
	OBJECT_TARGET		= 352,	// cible
	OBJECT_FLAGb		= 360,	// drapeau bleu
	OBJECT_FLAGr		= 361,	// drapeau rouge
	OBJECT_FLAGg		= 362,	// drapeau vert
	OBJECT_FLAGy		= 363,	// drapeau jaune
	OBJECT_FLAGv		= 364,	// drapeau violet
	OBJECT_KEYa			= 370,	// clé a
	OBJECT_KEYb			= 371,	// clé b
	OBJECT_KEYc			= 372,	// clé c
	OBJECT_KEYd			= 373,	// clé d
	OBJECT_HUMAN	    = 400,	// homme
	OBJECT_TECH		    = 402,	// technicien
	OBJECT_BARRIER0		= 500,	// barrière
	OBJECT_BARRIER1		= 501,	// barrière
	OBJECT_BARRIER2		= 502,	// barrière
	OBJECT_BARRIER3		= 503,	// barrière
	OBJECT_BARRIER4		= 504,	// barrière
	OBJECT_BARRIER5		= 505,	// barrière
	OBJECT_BARRIER6		= 506,	// barrière
	OBJECT_BARRIER7		= 507,	// barrière
	OBJECT_BARRIER8		= 508,	// barrière
	OBJECT_BARRIER9		= 509,	// barrière
	OBJECT_BARRIER10	= 510,	// barrière
	OBJECT_BARRIER11	= 511,	// barrière
	OBJECT_BARRIER12	= 512,	// barrière
	OBJECT_BARRIER13	= 513,	// barrière
	OBJECT_BARRIER14	= 514,	// barrière
	OBJECT_BARRIER15	= 515,	// barrière
	OBJECT_BARRIER16	= 516,	// barrière
	OBJECT_BARRIER17	= 517,	// barrière
	OBJECT_BARRIER18	= 518,	// barrière
	OBJECT_BARRIER19	= 519,	// barrière
	OBJECT_BOX1			= 520,	// caisse
	OBJECT_BOX2			= 521,	// caisse
	OBJECT_BOX3			= 522,	// caisse
	OBJECT_BOX4			= 523,	// caisse
	OBJECT_BOX5			= 524,	// caisse
	OBJECT_BOX6			= 525,	// caisse
	OBJECT_BOX7			= 526,	// caisse
	OBJECT_BOX8			= 527,	// caisse
	OBJECT_BOX9			= 528,	// caisse
	OBJECT_BOX10		= 529,	// caisse
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
	OBJECT_PIECE0		= 540,	// pièce
	OBJECT_PIECE1		= 541,	// pièce
	OBJECT_PIECE2		= 542,	// pièce
	OBJECT_PIECE3		= 543,	// pièce
	OBJECT_PIECE4		= 544,	// pièce
	OBJECT_PIECE5		= 545,	// pièce
	OBJECT_PIECE6		= 546,	// pièce
	OBJECT_PIECE7		= 547,	// pièce
	OBJECT_PIECE8		= 548,	// pièce
	OBJECT_PIECE9		= 549,	// pièce
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
	OBJECT_HOME1		= 910,	// maison 1
	OBJECT_HOME2		= 911,	// maison 2
	OBJECT_HOME3		= 912,	// maison 3
	OBJECT_HOME4		= 913,	// maison 4
	OBJECT_HOME5		= 914,	// maison 5
	OBJECT_ALIEN1	    = 920,	// usine alien
	OBJECT_ALIEN2	    = 921,	// usine alien
	OBJECT_ALIEN3	    = 922,	// usine alien
	OBJECT_ALIEN4	    = 923,	// usine alien
	OBJECT_ALIEN5	    = 924,	// usine alien
	OBJECT_ALIEN6	    = 925,	// usine alien
	OBJECT_ALIEN7	    = 926,	// usine alien
	OBJECT_ALIEN8	    = 927,	// usine alien
	OBJECT_ALIEN9	    = 928,	// usine alien
	OBJECT_ALIEN10		= 929,	// usine alien
	OBJECT_INCA1	    = 930,	// temple
	OBJECT_INCA2	    = 931,	// temple
	OBJECT_INCA3	    = 932,	// temple
	OBJECT_INCA4	    = 933,	// temple
	OBJECT_INCA5	    = 934,	// temple
	OBJECT_INCA6	    = 935,	// temple
	OBJECT_INCA7	    = 936,	// temple
	OBJECT_INCA8	    = 937,	// temple
	OBJECT_INCA9	    = 938,	// temple
	OBJECT_INCA10		= 939,	// temple
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
	int			object;			// numéro de l'objet dans CD3DEngine
	int			parentPart;		// numéro de la partie père
	int			masterParti;	// canal de la particule maître
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
	D3DVECTOR	wheelFrontPos;	// position roue avant gauche
	D3DVECTOR	wheelBackPos;	// position roue arrière gauche
	float		wheelFrontDim;	// rayon roues avants
	float		wheelBackDim;	// rayon roues arrières
	float		wheelFrontWidth;// largeur des pneus avants
	float		wheelBackWidth;	// largeur des pneus arrières
	float		crashFront;		// distance jusqu'à l'avant pour collisions
	float		crashBack;		// distance jusqu'à l'arrière pour collisions
	float		crashWidth;		// distance latérale pour collisions
	float		height;			// hauteur normale au-dessus du sol
	float		suspDetect;		// détection suspension
	float		suspHeight;		// hauteur suspension
	float		suspFrequency;	// fréquence suspension
	float		suspAbsorber;	// amortisseur suspension
	float		rolling;		// tandance au roulis dans les virages
	float		nicking;		// tandance au nick avant/arrière
	float		maxRolling;		// angle max de roulis
	float		maxNicking;		// angle max de nick
	float		overProp;		// tandance à survirer si propulsion
	float		overFactor;		// tandance au survirage dans les courbes
	float		overAngle;		// angle max de survirage
	float		overMul;		// accentiation de l'effet de survirage
	float		brakeDir;		// efficacité direction pendant freinage
	D3DVECTOR	posFret;		// position pour le fret
	D3DVECTOR	angleFret;		// angle pour le fret
	D3DVECTOR	antenna;		// pointe de l'antenne
	D3DVECTOR	lightFL;		// phase avant gauche
	D3DVECTOR	lightFR;		// phase avant droite
	D3DVECTOR	lightSL;		// phase stop gauche
	D3DVECTOR	lightSR;		// phase stop droite
	D3DVECTOR	lightRL;		// phase recule gauche
	D3DVECTOR	lightRR;		// phase recule droite
	D3DVECTOR	camera;			// position caméra en mode CAMERA_ONBOARD
	float		hookDist;		// distance pour porter avec le crochet
	float		mass;			// poid du véhicule
	float		turnSlide;		// glissement si virage rapide
	float		gripLimit;		// limite d'adérance latérale
	float		gripSlide;		// tenue de route latérale
	float		accelLow;		// facteur d'accélération si vitesse nulle
	float		accelHigh;		// facteur d'accélération si vitesse max
	float		accelSmooth;	// progresion de l'accélération
	int			motorSound;		// son pour le moteur
}
Character;

enum ExploType
{
	EXPLO_BOUM		= 1,
	EXPLO_BURN		= 2,
	EXPLO_WATER		= 3,
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
	BOOL		ExploPart(int total, float force);
	BOOL		DetachPart(int part, D3DVECTOR speed=D3DVECTOR(0.0f, 0.0f, 0.0f));
	BOOL		DetachPiece(int part, int param, D3DVECTOR speed=D3DVECTOR(0.0f, 0.0f, 0.0f));
	BOOL		ExploPiece(int part);

	BOOL		EventProcess(const Event &event);
	void		UpdateMapping();

	int			CreatePart();
	void		DeletePart(int part);
	void		SetObjectRank(int part, int objRank);
	int			RetObjectRank(int part);
	void		SetObjectParent(int part, int parent);
	void		SetType(ObjectType type);
	ObjectType	RetType();
	void		SetModel(int model);
	int			RetModel();
	void		SetSubModel(int subModel);
	int			RetSubModel();
	char*		RetName();
	void		SetOption(int option);
	int			RetOption();

	void		SetID(int id);
	int			RetID();

	void		SetDrawWorld(BOOL bDraw);
	void		SetDrawFront(BOOL bDraw);

	BOOL		CreateVehicle(D3DVECTOR pos, float angle, float zoom, ObjectType type, int model, int subModel, BOOL bPlumb, BOOL bTrainer);
	BOOL		CreateBuilding(D3DVECTOR pos, float angle, float zoom, float height, ObjectType type, BOOL bPlumb);
	BOOL		CreateResource(D3DVECTOR pos, float angle, float zoom, ObjectType type, BOOL bPlumb);
	BOOL		CreateBot(D3DVECTOR pos, float angle, float zoom, ObjectType type, BOOL bPlumb);
	BOOL		CreateAdditionnal(D3DVECTOR pos, float angle, float zoom, ObjectType type, BOOL bPlumb);
	BOOL		CreateBarrier(D3DVECTOR pos, float angle, float zoom, float height, ObjectType type, BOOL bPlumb);
	BOOL		CreateBox(D3DVECTOR pos, float angle, float zoom, float height, ObjectType type, BOOL bPlumb);
	BOOL		CreateStone(D3DVECTOR pos, float angle, float zoom, float height, ObjectType type, BOOL bPlumb);
	BOOL		CreatePiece(D3DVECTOR pos, float angle, float zoom, float height, ObjectType type, BOOL bPlumb);
	BOOL		CreatePlant(D3DVECTOR pos, float angle, float zoom, float height, ObjectType type, BOOL bPlumb);
	BOOL		CreateMushroom(D3DVECTOR pos, float angle, float zoom, float height, ObjectType type, BOOL bPlumb);
	BOOL		CreateQuartz(D3DVECTOR pos, float angle, float zoom, float height, ObjectType type, BOOL bPlumb);
	BOOL		CreateRoot(D3DVECTOR pos, float angle, float zoom, float height, ObjectType type, BOOL bPlumb);
	BOOL		CreateHome(D3DVECTOR pos, float angle, float zoom, float height, ObjectType type, BOOL bPlumb);
	BOOL		CreateRuin(D3DVECTOR pos, float angle, float zoom, float height, ObjectType type, BOOL bPlumb);

	BOOL		ReadProgram(int rank, char* filename);
	BOOL		WriteProgram(int rank, char* filename);
	BOOL		RunProgram(int rank);

	int			RetShadowLight();
	int			RetEffectLight();

	void		FlushCrashShere();
	int			CreateCrashSphere(D3DVECTOR pos, float radius, Sound sound, float hardness=0.45f);
	void		MoveCrashSphere(int rank, D3DVECTOR pos, float radius);
	int			RetCrashSphereTotal();
	BOOL		GetCrashSphere(int rank, D3DVECTOR &pos, float &radius);
	float		RetCrashSphereHardness(int rank);
	Sound		RetCrashSphereSound(int rank);
	void		DeleteCrashSphere(int rank);
	void		SetGlobalSphere(D3DVECTOR pos, float radius);
	void		GetGlobalSphere(D3DVECTOR &pos, float &radius);
	void		SetJotlerSphere(D3DVECTOR pos, float radius);
	void		GetJotlerSphere(D3DVECTOR &pos, float &radius);

	void		FlushCrashLine();
	void		SetCrashLineHeight(float h);
	float		RetCrashLineHeight();
	int			CreateCrashLine(FPOINT pos, Sound sound, float hardness=0.45f, BOOL bNew=FALSE);
	void		UpdateBBoxCrashLine();
	int			RetCrashLineTotal();
	BOOL		GetCrashLine(int rank, FPOINT &pos, BOOL &bNew);
	float		RetCrashLineHardness(int rank);
	Sound		RetCrashLineSound(int rank);
	void		RetCrashLineBBox(FPOINT &min, FPOINT &max);
	BOOL		IsCrashLineFusion();
	void		CrashLineFusion();
	BOOL		IsOccludeCamera(const D3DVECTOR &eye, const D3DVECTOR &look);

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
	void		SetLock(BOOL bLock);
	BOOL		RetLock();
	void		SetStarting(BOOL bStarting);
	BOOL		RetStarting();
	void		SetBurn(BOOL bBurn);
	BOOL		RetBurn();
	void		SetDead(BOOL bDead);
	BOOL		RetDead();
	BOOL		RetRuin();
	BOOL		RetActif();

	void		SetPassCounter(int counter);
	int			RetPassCounter();
	void		SetRankCounter(int rank);
	int			RetRankCounter();

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

	BOOL		CreateShadowCircle(float radius, float intensity, D3DShadowType type=D3DSHADOWNORM);
	BOOL		CreateShadowLight(float height, D3DCOLORVALUE color);
	BOOL		CreateEffectLight(float height, D3DCOLORVALUE color);

	void		FlatParent();
	BOOL		FlatParent(int part);

	void		SetRecorderRecordMode(BOOL bRecord);
	BOOL		RetRecorderRecordMode();
	void		SetRecorderPlayMode(BOOL bPlay);
	BOOL		RetRecorderPlayMode();
	void		SetRecorder(CRecorder* recorder);
	CRecorder*	RetRecorder();

protected:
	BOOL		EventFrame(const Event &event);
	void		PartiFrame(float rTime);
	void		CreateOtherObject(ObjectType type);
	void		InitPart(int part);
	void		UpdateTotalPart();
	int			SearchDescendant(int parent, int n);
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
	int			m_model;			// modèle de voiture
	int			m_subModel;			// modèle de la peinture
	int			m_id;				// identificateur unique
	Character	m_character;		// caractéristiques
	int			m_option;			// option
	int			m_shadowLight;		// numéro de la lumière de l'ombre
	float		m_shadowHeight;		// hauteur de la lumière de l'ombre
	int			m_effectLight;		// numéro de la lumière des effets
	float		m_effectHeight;		// hauteur de la lumière des effets
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
	float		m_transparency;		// transparence (0..1)
	float		m_aTime;
	float		m_shotTime;			// temps depuis dernier coup
	float		m_lastParticule;
	BOOL		m_bHilite;
	BOOL		m_bSelect;			// objet sélectionné
	BOOL		m_bSelectable;		// objet sélectionnable
	BOOL		m_bVisible;			// objet actif mais indétectable
	BOOL		m_bEnable;			// objet mort
	BOOL		m_bGadget;			// objet non indispensable
	BOOL		m_bLock;
	BOOL		m_bStarting;
	BOOL		m_bExplo;
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
	int			m_defRank;
	float		m_magnifyDamage;
	float		m_param;

	int			m_crashSphereUsed;	// nb de sphères utilisées
	D3DVECTOR	m_crashSpherePos[MAXCRASHSPHERE];
	float		m_crashSphereRadius[MAXCRASHSPHERE];
	float		m_crashSphereHardness[MAXCRASHSPHERE];
	Sound		m_crashSphereSound[MAXCRASHSPHERE];
	D3DVECTOR	m_globalSpherePos;
	float		m_globalSphereRadius;
	D3DVECTOR	m_jotlerSpherePos;
	float		m_jotlerSphereRadius;

	int			m_crashLineUsed;	// nb de lignes utilisées
	FPOINT		m_crashLinePos[MAXCRASHLINE];
	char		m_crashLineNew[MAXCRASHLINE];
	float		m_crashLineHardness[MAXCRASHLINE];
	Sound		m_crashLineSound[MAXCRASHLINE];
	FPOINT		m_crashLineMin;
	FPOINT		m_crashLineMax;
	float		m_crashLineHeight;

	int			m_totalPart;
	ObjectPart	m_objectPart[OBJECTMAXPART];

	int			m_partiSel[10];

	int			m_passCounter;
	int			m_rankCounter;

	float		m_cmdLine[OBJECTMAXCMDLINE];

	BOOL		m_bRecorderRecord;
	BOOL		m_bRecorderPlay;
	CRecorder*	m_recorder;
};


#endif //_OBJECT_H_
