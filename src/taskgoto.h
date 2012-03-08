// taskgoto.h

#ifndef _TASKGOTO_H_
#define	_TASKGOTO_H_


class CInstanceManager;
class CTerrain;
class CBrain;
class CPhysics;
class CObject;



#define MAXPOINTS	500



enum TaskGotoGoal
{
	TGG_DEFAULT		= -1,	// mode par défaut
	TGG_STOP		= 0,	// va à destination en s'arrêtant avec précision
	TGG_EXPRESS		= 1,	// va à destination sans s'arrêter
};

enum TaskGotoCrash
{
	TGC_DEFAULT		= -1,	// mode par défaut
	TGC_HALT		= 0,	// stoppe si collision
	TGC_RIGHTLEFT	= 1,	// droite-gauche
	TGC_LEFTRIGHT	= 2,	// gauche-droite
	TGC_LEFT		= 3,	// gauche
	TGC_RIGHT		= 4,	// droite
	TGC_BEAM		= 5,	// algorithme "rayons de soleil"
};


enum TaskGotoPhase
{
	TGP_ADVANCE		= 1,	// avance
	TGP_LAND		= 2,	// atterri
	TGP_TURN		= 3,	// tourne pour finir
	TGP_MOVE		= 4,	// avance pour finir
	TGP_CRWAIT		= 5,	// attend après collision
	TGP_CRTURN		= 6,	// tourne à droite après collision
	TGP_CRADVANCE	= 7,	// avance à droite après collision
	TGP_CLWAIT		= 8,	// attend après collision
	TGP_CLTURN		= 9,	// tourne à gauche après collision
	TGP_CLADVANCE	= 10,	// avance à gauche après collision
	TGP_BEAMLEAK	= 11,	// beam: leak (fuite)
	TGP_BEAMSEARCH	= 12,	// beam: search
	TGP_BEAMWCOLD	= 13,	// beam: attend refroidissement réacteur
	TGP_BEAMUP		= 14,	// beam: décolle
	TGP_BEAMGOTO	= 15,	// beam: goto dot list
	TGP_BEAMDOWN	= 16,	// beam: atterri
};



class CTaskGoto : public CTask
{
public:
	CTaskGoto(CInstanceManager* iMan, CObject* object);
	~CTaskGoto();

	BOOL		EventProcess(const Event &event);

	Error		Start(D3DVECTOR goal, float altitude, TaskGotoGoal goalMode, TaskGotoCrash crashMode);
	Error		IsEnded();

protected:
	CObject*	WormSearch(D3DVECTOR &impact);
	void		WormFrame(float rTime);
	CObject*	SearchTarget(D3DVECTOR pos, float margin);
	BOOL		AdjustTarget(CObject* pObj, D3DVECTOR &pos, float &distance);
	BOOL		AdjustBuilding(D3DVECTOR &pos, float margin, float &distance);
	BOOL		GetHotPoint(CObject *pObj, D3DVECTOR &pos, BOOL bTake, float distance, float &suppl);
	BOOL		LeakSearch(D3DVECTOR &pos, float &delay);
	void		ComputeRepulse(FPOINT &dir);
	void		ComputeFlyingRepulse(float &dir);

	int			BeamShortcut();
	void		BeamStart();
	void		BeamInit();
	Error		BeamSearch(const D3DVECTOR &start, const D3DVECTOR &goal, float goalRadius);
	Error		BeamExplore(const D3DVECTOR &prevPos, const D3DVECTOR &curPos, const D3DVECTOR &goalPos, float goalRadius, float angle, int nbDiv, float step, int i, int nbIter);
	D3DVECTOR	BeamPoint(const D3DVECTOR &startPoint, const D3DVECTOR &goalPoint, float angle, float step);

	void		BitmapDebug(const D3DVECTOR &min, const D3DVECTOR &max, const D3DVECTOR &start, const D3DVECTOR &goal);
	BOOL		BitmapTestLine(const D3DVECTOR &start, const D3DVECTOR &goal, float stepAngle, BOOL bSecond);
	void		BitmapObject();
	void		BitmapTerrain(const D3DVECTOR &min, const D3DVECTOR &max);
	void		BitmapTerrain(int minx, int miny, int maxx, int maxy);
	BOOL		BitmapOpen();
	BOOL		BitmapClose();
	void		BitmapSetCircle(const D3DVECTOR &pos, float radius);
	void		BitmapClearCircle(const D3DVECTOR &pos, float radius);
	void		BitmapSetDot(int rank, int x, int y);
	void		BitmapClearDot(int rank, int x, int y);
	BOOL		BitmapTestDot(int rank, int x, int y);

protected:
	D3DVECTOR		m_goal;
	D3DVECTOR		m_goalObject;
	float			m_angle;
	float			m_altitude;
	TaskGotoCrash	m_crashMode;
	TaskGotoGoal	m_goalMode;
	TaskGotoPhase	m_phase;
	int				m_try;
	Error			m_error;
	BOOL			m_bTake;
	float			m_stopLength;	// distance de freinage
	float			m_time;
	D3DVECTOR		m_pos;
	BOOL			m_bWorm;
	BOOL			m_bApprox;
	float			m_wormLastTime;
	float			m_lastDistance;

	int				m_bmSize;		// largeur ou hauteur du tableau
	int				m_bmOffset;		// m_bmSize/2
	int				m_bmLine;		// incrément ligne m_bmSize/8
	unsigned char*	m_bmArray;		// tableau de bits
	int				m_bmMinX, m_bmMinY;
	int				m_bmMaxX, m_bmMaxY;
	int				m_bmTotal;		// nb de points dans m_bmPoints
	int				m_bmIndex;		// index dans m_bmPoints
	D3DVECTOR		m_bmPoints[MAXPOINTS+2];
	char			m_bmIter[MAXPOINTS+2];
	int				m_bmIterCounter;
	CObject*		m_bmFretObject;
	float			m_bmFinalMove;	// distance finale à avancer
	float			m_bmFinalDist;	// distance effective à avancer
	D3DVECTOR		m_bmFinalPos;	// position initiale avant avance
	float			m_bmTimeLimit;
	int				m_bmStep;
	D3DVECTOR		m_bmWatchDogPos;
	float			m_bmWatchDogTime;
	D3DVECTOR		m_leakPos;		// position initiale à fuire
	float			m_leakDelay;
	float			m_leakTime;
	BOOL			m_bLeakRecede;
};


#endif //_TASKGOTO_H_
