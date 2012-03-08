// tasksearch.h

#ifndef _TASKSEARCH_H_
#define	_TASKSEARCH_H_


class CInstanceManager;
class CTerrain;
class CBrain;
class CPhysics;
class CObject;



enum TaskSearchHand
{
	TSH_UP		= 1,	// capteur en haut
	TSH_DOWN	= 2,	// capteur en bas
};

enum TaskSearchPhase
{
	TSP_DOWN	= 1,	// descend
	TSP_SEARCH	= 2,	// cherche
	TSP_UP		= 3,	// remonte
};



class CTaskSearch : public CTask
{
public:
	CTaskSearch(CInstanceManager* iMan, CObject* object);
	~CTaskSearch();

	BOOL		EventProcess(const Event &event);

	Error		Start();
	Error		IsEnded();
	BOOL		Abort();

protected:
	void	InitAngle();
	BOOL	CreateMark();
	void	DeleteMark(ObjectType type);

protected:
	TaskSearchHand	m_hand;
	TaskSearchPhase	m_phase;
	float			m_progress;
	float			m_speed;
	float			m_time;
	float			m_lastParticule;
	float			m_initialAngle[3];
	float			m_finalAngle[3];
	BOOL			m_bError;
};


#endif //_TASKSEARCH_H_
