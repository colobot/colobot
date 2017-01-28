// motionblupi.h

#ifndef _MOTIONBLUPI_H_
#define	_MOTIONBLUPI_H_


class CInstanceManager;
class CEngine;
class CLight;
class CParticule;
class CTerrain;
class CCamera;
class CObject;


#define MBLUPI_WAIT		0		// attend au repos
#define MBLUPI_TURN		1		// tourne
#define MBLUPI_ERROR	2		// opération impossible
#define MBLUPI_ERROR2	3		// 
#define MBLUPI_HELLO	4		// bonjour avec le bras
#define MBLUPI_HELLO2	5		// 
#define MBLUPI_AMAZE	6		// ohhhhh... (étonné)
#define MBLUPI_AMAZE2	7		// 
#define MBLUPI_PFIOU	8		// pfiouuu...
#define MBLUPI_PFIOU2	9		// 
#define MBLUPI_PFIOU3	10		// 
#define MBLUPI_RUSH		11		// élan pour pousser
#define MBLUPI_ROLL		12		// pousse une sphère pour la rouler
#define MBLUPI_ROLL2	13		// 
#define MBLUPI_TRAX		14		// presse un bouton du trax
#define MBLUPI_TRAX2	15		// 
#define MBLUPI_TAPTAP	16		// tape du pied
#define MBLUPI_TAPTAP2	17		// 
#define MBLUPI_DRINK1	18		// prend fiole
#define MBLUPI_DRINK2	19		// boit fiole
#define MBLUPI_YOUPIE	20		// content
#define MBLUPI_YOUPIE2	21		// 
#define MBLUPI_GOAL1	22		// attrape le ballon
#define MBLUPI_GOAL2	23		// suspendu au ballon
#define MBLUPI_STOP		24		// stoppe un robot
#define MBLUPI_TRUCK	25		// transporté par dock
#define MBLUPI_OUPS		26		// perd l'équilibre
#define MBLUPI_WALK1	27		// marche 1
#define MBLUPI_WALK2	28		// marche 2
#define MBLUPI_WALK3	29		// marche 3
#define MBLUPI_WALK4	30		// marche 4
#define MBLUPI_PUSH1	31		// pousse 1
#define MBLUPI_PUSH2	32		// pousse 2
#define MBLUPI_PUSH3	33		// pousse 3
#define MBLUPI_PUSH4	34		// pousse 4
#define MBLUPI_MAX		35

#define MBLUPI_EYE		100		// yeux..
#define MBLUPI_OPEN		0		// ..normaux
#define MBLUPI_CLOSE	1		// ..fermés

#define MBLUPI_MOUTH	110		// bouche..
#define MBLUPI_HAPPY	0		// ..sourire \-/
#define MBLUPI_SAD		1		// ..triste  /-\ 
#define MBLUPI_ANGRY	2		// ..fâché    o

#define MBLUPI_WALK		200		// marche normalement
#define MBLUPI_PUSH		201		// pousse une caisse



class CMotionBlupi : public CMotion
{
public:
	CMotionBlupi(CInstanceManager* iMan, CObject* object);
	~CMotionBlupi();

	void		DeleteObject(BOOL bAll=FALSE);
	BOOL		Create(D3DVECTOR pos, float angle, ObjectType type);
	BOOL		EventProcess(const Event &event);
	Error		SetAction(int action, float time=0.2f);

	float		RetLinSpeed();
	float		RetCirSpeed();
	float		RetLinStopLength();

protected:
	void		CreatePhysics();
	BOOL		EventFrame(const Event &event);
	void		LimitArticulations(const D3DVECTOR &cirVib);
	void		StrongParticule();
	void		GluParticule();
	CObject*	SearchBot(D3DVECTOR center, float radius);
	void		UpdateFaceMappingEye(int face);
	void		UpdateFaceMappingMouth(int face);
	void		StepSound();
	void		BrainFrameDemo();

protected:
	int			m_option;
	int			m_additional[10];
	int			m_faceEye;
	int			m_faceMouth;
	int			m_walk;
	float		m_aTime;
	float		m_bTime;
	short		m_armAngles[3*20*MBLUPI_MAX];
	int			m_armPartIndex;
	float		m_walkTime;
	BOOL		m_bFirstSound;
	float		m_closeTime;
	float		m_lastParticuleStrong;
	D3DVECTOR	m_lastPos;
	float		m_gimmickTime;
	float		m_startAngle;
	float		m_goalAngle;
};


#endif //_MOTIONBLUPI_H_
