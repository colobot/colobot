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
	PT_FRAGT	= 10,		// fragmentation objet technique
	PT_FRAGO	= 11,		// fragmentation objet organique
	PT_FRAGW	= 12,		// fragmentation objet sous l'eau
	PT_FRAGA	= 13,		// fragmentation objet atomique (bleu)
	PT_EXPLOT	= 20,		// explosion objet technique
	PT_EXPLOO	= 21,		// explosion objet organique
	PT_EXPLOW	= 22,		// explosion objet sous l'eau
	PT_EXPLOS	= 23,		// explosion seule
	PT_EXPLOP	= 24,		// explosion partie voiture
	PT_SHOTT	= 40,		// coup objet technique
	PT_SHOTH	= 41,		// coup homme
	PT_SHOTM	= 42,		// coup pondeuse
	PT_SHOTW	= 43,		// coup sous l'eau
	PT_BURNT	= 60,		// brûle objet technique
	PT_BURNO	= 61,		// brûle objet organique
	PT_BURNS	= 62,		// brûle objet seul
	PT_EJECT	= 70,		// objet éjecté
	PT_FINDING	= 71,		// objet découvert
	PT_PAINTING	= 72,		// voiture peinte
	PT_ACROBATIC= 73,		// robot voltigeant suite à un choc
	PT_PIECE	= 74,		// pièce de véhicule perdue
	PT_SABOTAGE	= 75,		// sabotage en cours
	PT_WPCHECK	= 80,		// indicateur atteint
	PT_WPVIBRA	= 81,		// indicateur touché
	PT_FLCREATE	= 82,		// drapeau créé
	PT_FLDELETE	= 83,		// drapeau détruit
	PT_WIN		= 90,		// feu d'artifice
	PT_LOST		= 91,		// fumée noire
	PT_DEADG	= 92,		// mort par balle
	PT_DEADW	= 93,		// mort noyé
	PT_SPIDER	= 94,		// explosion araignée
	PT_EGG		= 95,		// casse l'oeuf
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
	void		SetImpact(D3DVECTOR impact);
	BOOL		Create(PyroType type, CObject* pObj, float force=1.0f, int param=0);
	BOOL		EventProcess(const Event &event);
	Error		IsEnded();
	void		CutObjectLink(CObject* pObj);

protected:
	void		DisplayError(PyroType type, CObject* pObj);
	BOOL		CreateLight(D3DVECTOR pos, float height);
	void		DeleteObject(BOOL bPrimary, BOOL bSecondary);

	void		CreateTriangle(CObject* pObj, ObjectType oType, int part);

	void		ExploStart(ObjectType oType);
	void		ExploTerminate();

	void		OrgaStart();
	void		OrgaProgress();
	void		OrgaTerminate();

	void		BurnStart();
	void		BurnAddPart(int part, D3DVECTOR pos, D3DVECTOR angle);
	void		BurnProgress();
	BOOL		BurnIsKeepPart(int part);
	void		BurnTerminate();

	void		AcrobaticStart(D3DVECTOR min, D3DVECTOR max);
	void		AcrobaticTransform(ObjectType type, D3DVECTOR cdg, D3DVECTOR angle, D3DVECTOR min, D3DVECTOR max);
	void		AcrobaticGroundAdapt(ObjectType type, D3DVECTOR &angle, D3DVECTOR &pos);
	void		AcrobaticProgress();
	void		AcrobaticTerminate();

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

	D3DVECTOR		m_impact;
	D3DVECTOR		m_posStart;
	D3DVECTOR		m_angleStart;
	D3DVECTOR		m_angleGoal;
	D3DVECTOR		m_pos;			// centre de l'effet
	D3DVECTOR		m_posTracks;	// départ des traînées
	BOOL			m_bTracks;		// traînées incandescentes ?
	BOOL			m_bSphere;		// sphère translucide
	BOOL			m_bChoc;		// onde de choc circulaire
	PyroType		m_type;
	float			m_force;
	float			m_size;
	float			m_height;
	float			m_heightSuppl;
	float			m_progress;
	float			m_speed;
	float			m_time;
	float			m_lastParticule;
	float			m_lastParticuleSmoke;
	int				m_param;
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

	int				m_crashSphereUsed;	// nb de sphères utilisées
	D3DVECTOR		m_crashSpherePos[50];
	float			m_crashSphereRadius[50];
};


#endif //_PYRO_H_
