// motionvehicle.h

#ifndef _MOTIONVEHICLE_H_
#define	_MOTIONVEHICLE_H_


class CInstanceManager;
class CEngine;
class CLight;
class CParticule;
class CTerrain;
class CCamera;
class CBrain;
class CPhysics;
class CObject;
class CModFile;


#define MV_OPEN		0
#define MV_CLOSE	1
#define MV_FLATY	2
#define MV_FLATZ	3
#define MV_DEFLAT	4
#define MV_LOADBOT	5

#define MAX_PART	50


typedef struct
{
	short		part;		// numéro de la pièce
	short		order;		// ordre pour destruction
	float		x,y,z;		// position
	short		light[4];	// phares attachés à la pièce
	short		top;		// 1 -> partie du toît
	short		file;		// numéro pp dans "carmmpp.mod"
}
RemovePart;

typedef struct
{
	char		bDeleted;	// pièce détruite ?
	short		part;		// numéro de la pièce
	short		order;		// ordre pour destruction
	short		light[4];	// phares sur cette partie
	short		top;		// fait partie du toît ?
	short		twistState;	// avancement du délabrement
	D3DVECTOR	twistAngle;	// angle max si délabrement max
}
StatusPart;

typedef struct
{
	char		bDeleted;	// pièce détruite
	short		twistState;	// avancement du délabrement
	D3DVECTOR	twistAngle;	// angle max si délabrement max
}
StatePart;

typedef struct
{
	StatePart	state[MAX_PART];
}
StateVehicle;




class CMotionVehicle : public CMotion
{
public:
	CMotionVehicle(CInstanceManager* iMan, CObject* object);
	~CMotionVehicle();

	void		DeleteObject(BOOL bAll=FALSE);
	BOOL		Create(D3DVECTOR pos, float angle, ObjectType type, BOOL bPlumb);
	BOOL		EventProcess(const Event &event);
	Error		SetAction(int action, float time=0.2f);

	void		TwistInit();
	void		TwistPart(D3DVECTOR impact, float force);
	int			RetRemovePart(int &param);
	BOOL		RetLight(int rank);
	BOOL		ExistPart(TypePart part);
	int			RetTotalPart();
	int			RetUsedPart();

	void		SetWheelType(WheelType type);
	WheelType	RetWheelType();

	int			RetStateLength();
	void		GetStateBuffer(char *buffer);

protected:
	void		CreateWheel(CModFile* pModFile, char *name);
	void		CreatePart(CModFile* pModFile, RemovePart* table, float zoom=1.0f);
	void		CreatePhysics();
	BOOL		EventFrame(const Event &event);
	void		OpenClosePart(float progress);
	void		UpdateWheelMapping(float speed, BOOL bFront);
	void		UpdateGlassMapping(int part, float min, float max, float dir, D3DMaping mode, BOOL bBlack=FALSE);
	void		UpdateTrackMapping(float left, float right);
	BOOL		CrashVehicle(D3DVECTOR zoom);
	BOOL		IsUnderWater();
	void		TwistInit(StateVehicle *sv);

	void		ActionLoadBot(float rTime);
	CObject*	SearchObject(ObjectType type, D3DVECTOR center, float radius);
	D3DVECTOR	RetVehiclePoint(CObject *pObj);
	void		StartObjectAction(CObject *pObj, int action);

protected:
	int			m_model;
	int			m_option;
	float		m_posTrackLeft;
	float		m_posTrackRight;
	StatusPart	m_part[MAX_PART];
	int			m_partTotal;
	int			m_partUsed;
	int			m_topRest;		// nb de pièces du toît restantes
	BOOL		m_bLight[6];	// présence des phares
	BOOL		m_bWater;
	BOOL		m_bHelico;
	D3DVECTOR	m_wheelLastPos;
	D3DVECTOR	m_wheelLastAngle;
	float		m_flatTime;
	float		m_lastWheelSpeed[2];
	char		m_nameObjWheel[30];
	char		m_nameObjWBurn[30];
	char		m_nameTexWheel[20];
								// action LoadBot :
	BOOL		m_lbAction;
	BOOL		m_lbStart;
	float		m_lbProgress;
	float		m_lbSpeed;
	float		m_lbTime;
	CameraType	m_lbCamera;
	float		m_lbBackDist;
	CObject*	m_lbWalker;
	D3DVECTOR	m_lbStartPos;
	D3DVECTOR	m_lbGoalPos;
	float		m_lbStartAngle;
	float		m_lbGoalAngle;
	float		m_lbLastParticule;
};


#endif //_MOTIONVEHICLE_H_
