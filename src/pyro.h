// pyro.h

#ifndef _PYRO_H_
#define	_PYRO_H_


class CInstanceManager;
class CD3DEngine;
class CTerrain;
class CCamera;
class CParticule;
class CLight;
class CObject;
class CDisplayText;
class CRobotMain;
class CSound;



enum PyroType
{
	PT_NULL		= 0,
	PT_FRAGT	= 1,		// fragmentation objet technique
	PT_FRAGO	= 2,		// fragmentation objet organique
	PT_FRAGW	= 4,		// fragmentation objet sous l'eau
	PT_EXPLOT	= 5,		// explosion objet technique
	PT_EXPLOO	= 6,		// explosion objet organique
	PT_EXPLOW	= 8,		// explosion objet sous l'eau
	PT_SHOTT	= 9,		// coup objet technique
	PT_SHOTH	= 10,		// coup homme
	PT_SHOTM	= 11,		// coup pondeuse
	PT_SHOTW	= 12,		// coup sous l'eau
	PT_EGG		= 13,		// casse l'oeuf
	PT_BURNT	= 14,		// brûle objet technique
	PT_BURNO	= 15,		// brûle objet organique
	PT_SPIDER	= 16,		// explosion araignée
	PT_FALL		= 17,		// fret qui tombe
	PT_WPCHECK	= 18,		// indicateur atteint
	PT_FLCREATE	= 19,		// drapeau créé
	PT_FLDELETE	= 20,		// drapeau détruit
	PT_RESET	= 21,		// reset position de l'objet
	PT_WIN		= 22,		// feu d'artifice
	PT_LOST		= 23,		// fumée noire
	PT_DEADG	= 24,		// mort par balle
	PT_DEADW	= 25,		// mort noyé
	PT_FINDING	= 26,		// objet découvert
};


enum ObjectType;
enum Error;


typedef struct
{
	int				part;
	D3DVECTOR		initialPos;
	D3DVECTOR		finalPos;
	D3DVECTOR		initialAngle;
	D3DVECTOR		finalAngle;
}
PyroBurnPart;

typedef struct
{
	float			progress;
	float			intensity;
	D3DCOLORVALUE	color;
}
PyroLightOper;



class CPyro
{
public:
	CPyro(CInstanceManager* iMan);
	~CPyro();

	void		DeleteObject(BOOL bAll=FALSE);
	BOOL		Create(PyroType type, CObject* pObj, float force=1.0f);
	BOOL		EventProcess(const Event &event);
	Error		IsEnded();
	void		CutObjectLink(CObject* pObj);

protected:
	void		DisplayError(PyroType type, CObject* pObj);
	BOOL		CreateLight(D3DVECTOR pos, float height);
	void		DeleteObject(BOOL bPrimary, BOOL bSecondary);

	void		CreateTriangle(CObject* pObj, ObjectType oType, int part);

	void		ExploStart();
	void		ExploTerminate();

	void		BurnStart();
	void		BurnAddPart(int part, D3DVECTOR pos, D3DVECTOR angle);
	void		BurnProgress();
	BOOL		BurnIsKeepPart(int part);
	void		BurnTerminate();

	void		FallStart();
	CObject*	FallSearchBeeExplo();
	void		FallProgress(float rTime);
	Error		FallIsEnded();

	void		LightOperFlush();
	void		LightOperAdd(float progress, float intensity, float r, float g, float b);
	void		LightOperFrame(float rTime);

protected:
	CInstanceManager* m_iMan;
	CD3DEngine*		m_engine;
	CTerrain*		m_terrain;
	CCamera*		m_camera;
	CParticule*		m_particule;
	CLight*			m_light;
	CObject*		m_object;
	CDisplayText*	m_displayText;
	CRobotMain*		m_main;
	CSound*			m_sound;

	D3DVECTOR		m_pos;			// centre de l'effet
	D3DVECTOR		m_posPower;		// centre de la pile
	BOOL			m_bPower;		// pile existe ?
	PyroType		m_type;
	float			m_force;
	float			m_size;
	float			m_progress;
	float			m_speed;
	float			m_time;
	float			m_lastParticule;
	float			m_lastParticuleSmoke;
	int				m_soundChannel;

	int				m_lightRank;
	int				m_lightOperTotal;
	PyroLightOper	m_lightOper[10];
	float			m_lightHeight;

	ObjectType		m_burnType;
	int				m_burnPartTotal;
	PyroBurnPart	m_burnPart[10];
	int				m_burnKeepPart[10];
	float			m_burnFall;

	float			m_fallFloor;
	float			m_fallSpeed;
	float			m_fallBulletTime;
	BOOL			m_bFallEnding;

	int				m_crashSphereUsed;	// nb de sphères utilisées
	D3DVECTOR		m_crashSpherePos[50];
	float			m_crashSphereRadius[50];
};


#endif //_PYRO_H_
