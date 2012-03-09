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
// * along with this program. If not, see .

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
	PARTIMOTOR		= 3,		// gaz d'�chappement du moteur
	PARTIGLINT		= 4,		// reflet
	PARTIBLITZ		= 5,		// �clair recharge batterie
	PARTICRASH		= 6,		// poussi�re apr�s chute
	PARTIGAS		= 7,		// gaz du r�acteur
	PARTIFIRE		= 9,		// boule de feu qui r�tricit
	PARTIFIREZ		= 10,		// boule de feu qui grandit
	PARTIBLUE		= 11,		// boule bleu
	PARTISELY		= 12,		// s�lection jaune
	PARTISELR		= 13,		// s�lection rouge
	PARTIGUN1		= 18,		// balle 1 (fireball)
	PARTIGUN2		= 19,		// balle 2 (fourmi)
	PARTIGUN3		= 20,		// balle 3 (araign�e)
	PARTIGUN4		= 21,		// balle 4 (orgaball)
	PARTIFRAG		= 22,		// fragment triangulaire
	PARTIQUEUE		= 23,		// queue enflamm�e
	PARTIORGANIC1	= 24,		// boule organique m�re
	PARTIORGANIC2	= 25,		// boule organique fille
	PARTISMOKE1		= 26,		// fum�e noire
	PARTISMOKE2		= 27,		// fum�e noire
	PARTISMOKE3		= 28,		// fum�e noire
	PARTISMOKE4		= 29,		// fum�e noire
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
	PARTIEJECT		= 50,		// �jection du r�acteur
	PARTISCRAPS		= 51,		// d�chets du r�acteur
	PARTITOTO		= 52,		// r�acteur de toto
	PARTIERROR		= 53,		// toto dit non
	PARTIWARNING	= 54,		// toto dit bof
	PARTIINFO		= 54,		// toto dit oui
	PARTIQUARTZ		= 55,		// reflet quartz
	PARTISPHERE0	= 56,		// sph�re d'explosion
	PARTISPHERE1	= 57,		// sph�re d'�nergie
	PARTISPHERE2	= 58,		// sph�re d'analyse
	PARTISPHERE3	= 59,		// sph�re de bouclier
	PARTISPHERE4	= 60,		// sph�re d'information (emette)
	PARTISPHERE5	= 61,		// sph�re v�g�tale (gravity root)
	PARTISPHERE6	= 62,		// sph�re d'information (receive)
	PARTISPHERE7	= 63,		// sph�re
	PARTISPHERE8	= 64,		// sph�re
	PARTISPHERE9	= 65,		// sph�re
	PARTIGUNDEL		= 66,		// destruction balle par bouclier
	PARTIPART		= 67,		// partie d'objet
	PARTITRACK1		= 68,		// tra�n�e 1
	PARTITRACK2		= 69,		// tra�n�e 2
	PARTITRACK3		= 70,		// tra�n�e 3
	PARTITRACK4		= 71,		// tra�n�e 4
	PARTITRACK5		= 72,		// tra�n�e 5
	PARTITRACK6		= 73,		// tra�n�e 6
	PARTITRACK7		= 74,		// tra�n�e 7
	PARTITRACK8		= 75,		// tra�n�e 8
	PARTITRACK9		= 76,		// tra�n�e 9
	PARTITRACK10	= 77,		// tra�n�e 10
	PARTITRACK11	= 78,		// tra�n�e 11
	PARTITRACK12	= 79,		// tra�n�e 12
	PARTITRACK13	= 80,		// tra�n�e 13
	PARTITRACK14	= 81,		// tra�n�e 14
	PARTITRACK15	= 82,		// tra�n�e 15
	PARTITRACK16	= 83,		// tra�n�e 16
	PARTITRACK17	= 84,		// tra�n�e 17
	PARTITRACK18	= 85,		// tra�n�e 18
	PARTITRACK19	= 86,		// tra�n�e 19
	PARTITRACK20	= 87,		// tra�n�e 20
	PARTIGLINTb		= 88,		// reflet bleu
	PARTIGLINTr		= 89,		// reflet rouge
	PARTILENS1		= 90,		// �clat 1 (orange)
	PARTILENS2		= 91,		// �clat 2 (jaune)
	PARTILENS3		= 92,		// �clat 3 (rouge)
	PARTILENS4		= 93,		// �clat 4 (violet)
	PARTICONTROL	= 94,		// reflet sur bouton
	PARTISHOW		= 95,		// montre un lieu
	PARTICHOC		= 96,		// onde de choc
	PARTIGFLAT		= 97,		// montre si le sol est plat
	PARTIRECOVER	= 98,		// boule bleu pour recycleur
	PARTIROOT		= 100,		// fum�e gravity root
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
	char			bUsed;		// TRUE -> particule utilis�e
	char			bRay;		// TRUE -> rayon avec but
	unsigned short	uniqueStamp;// marque unique
	short			sheet;		// feuille (0..n)
	ParticuleType	type;		// type PARTI*
	ParticulePhase	phase;		// phase PARPH*
	float			mass;		// masse de la particule (pour les rebonds)
	float			weight;		// poids de la particule (pour le bruit)
	float			duration;	// dur�e de vie
	D3DVECTOR		pos;		// position absolue (relative si objet li�)
	D3DVECTOR		goal;		// position but (si bRay)
	D3DVECTOR		speed;		// vitesses de d�placement
	float			windSensitivity;
	short			bounce;		// nb de rebonds
	FPOINT			dim;		// dimensions du rectangle
	float			zoom;		// zoom (0..1)
	float			angle;		// angle de rotation
	float			intensity;	// intensit�
	FPOINT			texSup;		// coordonn�e texture sup�rieure
	FPOINT			texInf;		// cooddonn�e texture inf�rieure
	float			time;		// �ge de la particule (0..n)
	float			phaseTime;	// �ge au d�but de la phase
	float			testTime;	// temps depuis dernier test
	CObject*		objLink;	// objet p�re (pour r�acteur par exemple)
	CObject*		objFather;	// objet p�re (pour r�acteur par exemple)
	short			objRank;	// rang de l'objet, ou -1
	short			trackRank;	// rang de la tra�n�e
}
Particule;

typedef struct
{
	char			bUsed;		// TRUE -> tra�n�e utilis�e
	char			bDrawParticule;
	float			step;		// dur�e d'un pas
	float			last;		// progression dernier pas m�moris�
	float			intensity;	// intensit� au d�part (0..1)
	float			width;		// largeur queue
	int				used;		// nb de positions dans "pos"
	int				head;		// index t�te d'�criture
	D3DVECTOR		pos[MAXTRACKLEN];
	float			len[MAXTRACKLEN];
}
Track;

typedef struct
{
	ParticuleType	type;		// type PARTI*
	D3DVECTOR		pos[4];		// positions rectangle
	float			startTime;	// d�but de vie
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
