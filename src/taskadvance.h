// taskadvance.h

#ifndef _TASKADVANCE_H_
#define	_TASKADVANCE_H_


class CInstanceManager;
class CTerrain;
class CBrain;
class CPhysics;
class CObject;


class CTaskAdvance : public CTask
{
public:
	CTaskAdvance(CInstanceManager* iMan, CObject* object);
	~CTaskAdvance();

	BOOL	EventProcess(const Event &event);

	Error	Start(float length);
	Error	IsEnded();

protected:

protected:
	float		m_totalLength;
	float		m_advanceLength;
	float		m_direction;
	float		m_timeLimit;
	D3DVECTOR	m_startPos;
	float		m_lastDist;
	float		m_fixTime;
	BOOL		m_bError;
};


#endif //_TASKADVANCE_H_
