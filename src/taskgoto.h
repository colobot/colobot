// taskgoto.h

#ifndef _TASKGOTO_H_
#define	_TASKGOTO_H_


class CInstanceManager;
class CTerrain;
class CObject;



#define MAXPATH		200


enum TaskGotoPhase
{
	TGP_GOTO	= 1,	// goto path list
	TGP_TURN	= 2,	// final turn
	TGP_RUSH	= 3,	// prend de l'élan
};



typedef struct
{
	int			father;
	int			x, y;
}
ListItem;




class CTaskGoto : public CTask
{
public:
	CTaskGoto(CInstanceManager* iMan, CObject* object);
	~CTaskGoto();

	BOOL		EventProcess(const Event &event);

	Error		Start(D3DVECTOR goal, CObject *target, int part);
	Error		IsEnded();
	BOOL		Abort();
	BOOL		IsStopable();
	BOOL		Stop();

protected:
	void		AdjustTarget(CObject* target, int part);
	void		JostleObject(D3DVECTOR center);
	void		StartAction(int action, float speed=0.2f);
	void		ProgressAction(float progress);
	void		ProgressLinSpeed(float speed);
	void		ProgressCirSpeed(float speed);
	BOOL		IsLockZone(const D3DVECTOR &goal);
	void		FreeLockZone();

	BOOL		PathFinder(D3DVECTOR start, D3DVECTOR goal);
	BOOL		PathFinderPass(D3DVECTOR start, D3DVECTOR goal);
	BOOL		IsTerrainFreeR(int ix, int iy, int x, int y);
	BOOL		IsTerrainFree(int ix, int iy, int x, int y);
	BOOL		IsTerrainRound(int ix, int iy, int x, int y);
	void		ListFlush();
	void		ListCreate();
	BOOL		ListAdd(int father, int x, int y);

protected:
	ObjectType		m_type;				// type de l'objet
	ObjectType		m_typeTarget;		// type de l'objet cible
	D3DVECTOR		m_goal;				// position but
	D3DVECTOR		m_goalObject;		// position dans objet cible
	int				m_goalx, m_goaly;	// position but (= m_goal)
	float			m_totalTime;		// temps total mis
	float			m_delay;			// attente initiale
	float			m_angle;			// angle final souhaité
	TaskGotoPhase	m_phase;			// phase en cours
	Error			m_error;			// erreur
	BOOL			m_bFinalTurn;		// rotation finale nécessaire ?
	BOOL			m_bFinalRush;		// élan final nécessaire ?
	BOOL			m_bStopPending;		// arrêt en attente ?
	float			m_finalMargin;		// marge finale
	float			m_finalAngle;		// angle final selon objet
	float			m_linStopLength;	// distance de freinage
	float			m_totalAdvance;		// avance totale effectuée
	D3DVECTOR		m_lastPos;			// dernière position "lockée"

	int				m_nbTiles;			// ne de tuiles du terrain
	int				m_nbTiles2;			// m_nbTiles/2
	float			m_dimTile;			// dimensions d'une tuile

	int				m_pass;				// rang de la passe (0..1)
	int				m_secondNeed;		// nb requis pour 2ème passe év.
	int				m_listGoalx;		// objectif (0..n)
	int				m_listGoaly;		// 
	int				m_listTotal;		// nb item dans m_listTable
	ListItem*		m_listTable;		// cases visitées
	char*			m_listFlags;		// bitmap
	BOOL			m_bListFound;		// but atteint
	int				m_pathIndex;		// nb points dans m_pathPos
	D3DVECTOR		m_pathPos[MAXPATH];	// chemin trouvé
};


#endif //_TASKGOTO_H_
