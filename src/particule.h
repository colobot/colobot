// particule.h

#ifndef _PARTICULE_H_
#define	_PARTICULE_H_


#include "D3DEngine.h"


class CInstanceManager;
class CD3DEngine;
class CRobotMain;
class CTerrain;
class CWater;
class CObject;
class CSound;

enum Sound;


#define MAXPARTICULE	500
#define MAXPARTITYPE	5
#define MAXTRACK		100
#define MAXTRACKLEN		10
#define MAXPARTIFOG		100
#define MAXWHEELTRACE	1000

#define SH_WORLD		0		// particule dans le monde sous l'interface
#define SH_FRONT		1		// particule dans le monde sur l'interface
#define SH_INTERFACE	2		// particule dans l'interface
#define SH_MAX			3

// type == 0	->	triangles
// type == 1	->	effect00 (fond noir)
// type == 2	->	effect01 (fond noir)
// type == 3	->	effect02 (fond noir)
// type == 4	->	text     (fond blanc)


enum ParticuleType
{
	PARTIEXPLOT		= 1,		// explosion technique
	PARTIEXPLOO		= 2,		// explosion organique
	PARTIMOTOR		= 3,		// gaz d'échappement du moteur
	PARTIGLINT		= 4,		// reflet
	PARTIBLITZ		= 5,		// éclair recharge batterie
	PARTICRASH		= 6,		// poussière après chute
	PARTIGAS		= 7,		// gaz du réacteur
	PARTIFIRE		= 9,		// boule de feu qui rétricit
	PARTIFIREZ		= 10,		// boule de feu qui grandit
	PARTIBLUE		= 11,		// boule bleu
	PARTISELY		= 12,		// sélection jaune
	PARTISELR		= 13,		// sélection rouge
	PARTIGUN1		= 18,		// balle 1 (fireball)
	PARTIGUN2		= 19,		// balle 2 (fourmi)
	PARTIGUN3		= 20,		// balle 3 (araignée)
	PARTIGUN4		= 21,		// balle 4 (orgaball)
	PARTIFRAG		= 22,		// fragment triangulaire
	PARTIQUEUE		= 23,		// queue enflammée
	PARTIORGANIC1	= 24,		// boule organique mère
	PARTIORGANIC2	= 25,		// boule organique fille
	PARTISMOKE1		= 26,		// fumée noire
	PARTISMOKE2		= 27,		// fumée noire
	PARTISMOKE3		= 28,		// fumée noire
	PARTISMOKE4		= 29,		// fumée noire
	PARTIBLOOD		= 30,		// sang homme
	PARTIBLOODM		= 31,		// sang pondeuse
	PARTIVAPOR		= 32,		// vapeur
	PARTIVIRUS1		= 33,		// virus 1
	PARTIVIRUS2		= 34,		// virus 2
	PARTIVIRUS3		= 35,		// virus 3
	PARTIVIRUS4		= 36,		// virus 4
	PARTIVIRUS5		= 37,		// virus 5
	PARTIVIRUS6		= 38,		// virus 6
	PARTIVIRUS7		= 39,		// virus 7
	PARTIVIRUS8		= 40,		// virus 8
	PARTIVIRUS9		= 41,		// virus 9
	PARTIVIRUS10	= 42,		// virus 10
	PARTIRAY1		= 43,		// rayon 1 (tour)
	PARTIRAY2		= 44,		// rayon 2 (electric arc)
	PARTIRAY3		= 45,		// rayon 3
	PARTIRAY4		= 46,		// rayon 4
	PARTIFLAME		= 47,		// flamme
	PARTIBUBBLE		= 48,		// bubble
	PARTIFLIC		= 49,		// rond dans l'eau
	PARTIEJECT		= 50,		// éjection du réacteur
	PARTISCRAPS		= 51,		// déchets du réacteur
	PARTITOTO		= 52,		// réacteur de toto
	PARTIERROR		= 53,		// toto dit non
	PARTIWARNING	= 54,		// toto dit bof
	PARTIINFO		= 54,		// toto dit oui
	PARTIQUARTZ		= 55,		// reflet quartz
	PARTISPHERE0	= 56,		// sphère d'explosion
	PARTISPHERE1	= 57,		// sphère d'énergie
	PARTISPHERE2	= 58,		// sphère d'analyse
	PARTISPHERE3	= 59,		// sphère de bouclier
	PARTISPHERE4	= 60,		// sphère d'information (emette)
	PARTISPHERE5	= 61,		// sphère végétale (gravity root)
	PARTISPHERE6	= 62,		// sphère d'information (receive)
	PARTISPHERE7	= 63,		// sphère
	PARTISPHERE8	= 64,		// sphère
	PARTISPHERE9	= 65,		// sphère
	PARTIGUNDEL		= 66,		// destruction balle par bouclier
	PARTIPART		= 67,		// partie d'objet
	PARTITRACK1		= 68,		// traînée 1
	PARTITRACK2		= 69,		// traînée 2
	PARTITRACK3		= 70,		// traînée 3
	PARTITRACK4		= 71,		// traînée 4
	PARTITRACK5		= 72,		// traînée 5
	PARTITRACK6		= 73,		// traînée 6
	PARTITRACK7		= 74,		// traînée 7
	PARTITRACK8		= 75,		// traînée 8
	PARTITRACK9		= 76,		// traînée 9
	PARTITRACK10	= 77,		// traînée 10
	PARTITRACK11	= 78,		// traînée 11
	PARTITRACK12	= 79,		// traînée 12
	PARTITRACK13	= 80,		// traînée 13
	PARTITRACK14	= 81,		// traînée 14
	PARTITRACK15	= 82,		// traînée 15
	PARTITRACK16	= 83,		// traînée 16
	PARTITRACK17	= 84,		// traînée 17
	PARTITRACK18	= 85,		// traînée 18
	PARTITRACK19	= 86,		// traînée 19
	PARTITRACK20	= 87,		// traînée 20
	PARTIGLINTb		= 88,		// reflet bleu
	PARTIGLINTr		= 89,		// reflet rouge
	PARTILENS1		= 90,		// éclat 1 (orange)
	PARTILENS2		= 91,		// éclat 2 (jaune)
	PARTILENS3		= 92,		// éclat 3 (rouge)
	PARTILENS4		= 93,		// éclat 4 (violet)
	PARTICONTROL	= 94,		// reflet sur bouton
	PARTISHOW		= 95,		// montre un lieu
	PARTICHOC		= 96,		// onde de choc
	PARTIGFLAT		= 97,		// montre si le sol est plat
	PARTIRECOVER	= 98,		// boule bleu pour recycleur
	PARTIROOT		= 100,		// fumée gravity root
	PARTIPLOUF0		= 101,		// plouf
	PARTIPLOUF1		= 102,		// plouf
	PARTIPLOUF2		= 103,		// plouf
	PARTIPLOUF3		= 104,		// plouf
	PARTIPLOUF4		= 105,		// plouf
	PARTIDROP		= 106,		// goutte
	PARTIFOG0		= 107,		// brouillard 0
	PARTIFOG1		= 108,		// brouillard 1
	PARTIFOG2		= 109,		// brouillard 2
	PARTIFOG3		= 110,		// brouillard 3
	PARTIFOG4		= 111,		// brouillard 4
	PARTIFOG5		= 112,		// brouillard 5
	PARTIFOG6		= 113,		// brouillard 6
	PARTIFOG7		= 114,		// brouillard 7
	PARTIFOG8		= 115,		// brouillard 8
	PARTIFOG9		= 116,		// brouillard 9
	PARTILIMIT1		= 117,		// montre les limites 1
	PARTILIMIT2		= 118,		// montre les limites 2
	PARTILIMIT3		= 119,		// montre les limites 3
	PARTILIMIT4		= 120,		// montre les limites 4
	PARTIWATER		= 121,		// goutte d'eau
	PARTIEXPLOG1	= 122,		// explosion balle 1
	PARTIEXPLOG2	= 123,		// explosion balle 2
	PARTIBASE		= 124,		// gaz du vaisseau spatial
	PARTITRACE0		= 140,		// trace
	PARTITRACE1		= 141,		// trace
	PARTITRACE2		= 142,		// trace
	PARTITRACE3		= 143,		// trace
	PARTITRACE4		= 144,		// trace
	PARTITRACE5		= 145,		// trace
	PARTITRACE6		= 146,		// trace
	PARTITRACE7		= 147,		// trace
	PARTITRACE8		= 148,		// trace
	PARTITRACE9		= 149,		// trace
	PARTITRACE10	= 150,		// trace
	PARTITRACE11	= 151,		// trace
	PARTITRACE12	= 152,		// trace
	PARTITRACE13	= 153,		// trace
	PARTITRACE14	= 154,		// trace
	PARTITRACE15	= 155,		// trace
	PARTITRACE16	= 156,		// trace
	PARTITRACE17	= 157,		// trace
	PARTITRACE18	= 158,		// trace
	PARTITRACE19	= 159,		// trace
};

enum ParticulePhase
{
	PARPHSTART		= 0,
	PARPHEND		= 1,
};

typedef struct
{
	char			bUsed;		// TRUE -> particule utilisée
	char			bRay;		// TRUE -> rayon avec but
	unsigned short	uniqueStamp;// marque unique
	short			sheet;		// feuille (0..n)
	ParticuleType	type;		// type PARTI*
	ParticulePhase	phase;		// phase PARPH*
	float			mass;		// masse de la particule (pour les rebonds)
	float			weight;		// poids de la particule (pour le bruit)
	float			duration;	// durée de vie
	D3DVECTOR		pos;		// position absolue (relative si objet lié)
	D3DVECTOR		goal;		// position but (si bRay)
	D3DVECTOR		speed;		// vitesses de déplacement
	float			windSensitivity;
	short			bounce;		// nb de rebonds
	FPOINT			dim;		// dimensions du rectangle
	float			zoom;		// zoom (0..1)
	float			angle;		// angle de rotation
	float			intensity;	// intensité
	FPOINT			texSup;		// coordonnée texture supérieure
	FPOINT			texInf;		// cooddonnée texture inférieure
	float			time;		// âge de la particule (0..n)
	float			phaseTime;	// âge au début de la phase
	float			testTime;	// temps depuis dernier test
	CObject*		objLink;	// objet père (pour réacteur par exemple)
	CObject*		objFather;	// objet père (pour réacteur par exemple)
	short			objRank;	// rang de l'objet, ou -1
	short			trackRank;	// rang de la traînée
}
Particule;

typedef struct
{
	char			bUsed;		// TRUE -> traînée utilisée
	char			bDrawParticule;
	float			step;		// durée d'un pas
	float			last;		// progression dernier pas mémorisé
	float			intensity;	// intensité au départ (0..1)
	float			width;		// largeur queue
	int				used;		// nb de positions dans "pos"
	int				head;		// index tête d'écriture
	D3DVECTOR		pos[MAXTRACKLEN];
	float			len[MAXTRACKLEN];
}
Track;

typedef struct
{
	ParticuleType	type;		// type PARTI*
	D3DVECTOR		pos[4];		// positions rectangle
	float			startTime;	// début de vie
}
WheelTrace;



class CParticule
{
public:
	CParticule(CInstanceManager* iMan, CD3DEngine* engine);
	~CParticule();

	void		SetD3DDevice(LPDIRECT3DDEVICE7 device);

	void		FlushParticule();
	void		FlushParticule(int sheet);
	int			CreateParticule(D3DVECTOR pos, D3DVECTOR speed, FPOINT dim, ParticuleType type, float duration=1.0f, float mass=0.0f, float windSensitivity=1.0f, int sheet=0);
	int			CreateFrag(D3DVECTOR pos, D3DVECTOR speed, D3DTriangle *triangle, ParticuleType type, float duration=1.0f, float mass=0.0f, float windSensitivity=1.0f, int sheet=0);
	int			CreatePart(D3DVECTOR pos, D3DVECTOR speed, ParticuleType type, float duration=1.0f, float mass=0.0f, float weight=0.0f, float windSensitivity=1.0f, int sheet=0);
	int			CreateRay(D3DVECTOR pos, D3DVECTOR goal, ParticuleType type, FPOINT dim, float duration=1.0f, int sheet=0);
	int			CreateTrack(D3DVECTOR pos, D3DVECTOR speed, FPOINT dim, ParticuleType type, float duration=1.0f, float mass=0.0f, float length=10.0f, float width=1.0f);
	void		CreateWheelTrace(const D3DVECTOR &p1, const D3DVECTOR &p2, const D3DVECTOR &p3, const D3DVECTOR &p4, ParticuleType type);
	void		DeleteParticule(ParticuleType type);
	void		DeleteParticule(int channel);
	void		SetObjectLink(int channel, CObject *object);
	void		SetObjectFather(int channel, CObject *object);
	void		SetPosition(int channel, D3DVECTOR pos);
	void		SetDimension(int channel, FPOINT dim);
	void		SetZoom(int channel, float zoom);
	void		SetAngle(int channel, float angle);
	void		SetIntensity(int channel, float intensity);
	void		SetParam(int channel, D3DVECTOR pos, FPOINT dim, float zoom, float angle, float intensity);
	void		SetPhase(int channel, ParticulePhase phase, float duration);
	BOOL		GetPosition(int channel, D3DVECTOR &pos);

	D3DCOLORVALUE RetFogColor(D3DVECTOR pos);

	void		SetFrameUpdate(int sheet, BOOL bUpdate);
	void		FrameParticule(float rTime);
	void		DrawParticule(int sheet);

	BOOL		WriteWheelTrace(char *filename, int width, int height, D3DVECTOR dl, D3DVECTOR ur);

protected:
	void		DeleteRank(int rank);
	BOOL		CheckChannel(int &channel);
	void		DrawParticuleTriangle(int i);
	void		DrawParticuleNorm(int i);
	void		DrawParticuleFlat(int i);
	void		DrawParticuleFog(int i);
	void		DrawParticuleRay(int i);
	void		DrawParticuleSphere(int i);
	void		DrawParticuleCylinder(int i);
	void		DrawParticuleWheel(int i);
	CObject*	SearchObjectGun(D3DVECTOR old, D3DVECTOR pos, ParticuleType type, CObject *father);
	CObject*	SearchObjectRay(D3DVECTOR pos, D3DVECTOR goal, ParticuleType type, CObject *father);
	void		Play(Sound sound, D3DVECTOR pos, float amplitude);
	BOOL		TrackMove(int i, D3DVECTOR pos, float progress);
	void		TrackDraw(int i, ParticuleType type);

protected:
	CInstanceManager*	m_iMan;
	CD3DEngine*			m_engine;
	LPDIRECT3DDEVICE7	m_pD3DDevice;
	CRobotMain*			m_main;
	CTerrain*			m_terrain;
	CWater*				m_water;
	CSound*				m_sound;

	Particule	m_particule[MAXPARTICULE*MAXPARTITYPE];
	D3DTriangle	m_triangle[MAXPARTICULE];  // triangle si PartiType == 0
	Track		m_track[MAXTRACK];
	int			m_wheelTraceTotal;
	int			m_wheelTraceIndex;
	WheelTrace	m_wheelTrace[MAXWHEELTRACE];
	int			m_totalInterface[MAXPARTITYPE][SH_MAX];
	BOOL		m_bFrameUpdate[SH_MAX];
	int			m_fogTotal;
	int			m_fog[MAXPARTIFOG];
	int			m_uniqueStamp;
	int			m_exploGunCounter;
	float		m_lastTimeGunDel;
	float		m_absTime;
};


#endif //_PARTICULE_H_
