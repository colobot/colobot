// taskroll.h

#ifndef _TASKROLL_H_
#define	_TASKROLL_H_


class CInstanceManager;
class CTerrain;
class CObject;

enum Sound;


enum TaskRoll
{
	TRO_NULL,
	TRO_ROLL,		// roule une sphère
};



class CTaskRoll : public CTask
{
public:
	CTaskRoll(CInstanceManager* iMan, CObject* object);
	~CTaskRoll();

	BOOL		EventProcess(const Event &event);

	Error		Start(D3DVECTOR dir);
	Error		IsEnded();

protected:
	void		ExploProxi();
	void		ExploTremble(float progress);
	void		ExploObject();
	CObject*	SearchBox(D3DVECTOR center, float radius);
	CObject*	SearchObject(D3DVECTOR center, float radius);
	CObject*	SearchBlupi(D3DVECTOR center, float radius);
	BOOL		IsPosFree(D3DVECTOR center);
	BOOL		IsHole(D3DVECTOR center);
	BOOL		IsSpace(D3DVECTOR center);

protected:
	ObjectType	m_type;
	TaskRoll	m_phase;
	int			m_total;
	int			m_totalMash;
	CObject*	m_pMash[51];	// 1+25+25
	ObjectType	m_mashType;
	float		m_time;
	float		m_progress;
	float		m_speed;
	D3DVECTOR	m_dir;
	D3DVECTOR	m_startPos;
	D3DVECTOR	m_goalPos;
	D3DVECTOR	m_boxAngle;
	BOOL		m_bImpossible;
	BOOL		m_bMash;
	BOOL		m_bExplo;
	BOOL		m_bHole;
	BOOL		m_bSpace;
	BOOL		m_bPipe;
	BOOL		m_bPlouf;
	BOOL		m_bError;
	BOOL		m_bFallSound;
	Sound		m_middleSound;
	float		m_lastParticuleSmoke;
};


#endif //_TASKROLL_H_
