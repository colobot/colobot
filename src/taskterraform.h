// taskterraform.h

#ifndef _TASKSTERRAFORM_H_
#define	_TASKSTERRAFORM_H_


class CInstanceManager;
class CTerrain;
class CBrain;
class CPhysics;
class CObject;



enum TaskTerraPhase
{
	TTP_CHARGE	= 1,	// charge d'énergie
	TTP_DOWN	= 2,	// descend
	TTP_TERRA	= 3,	// frappe
	TTP_UP		= 4,	// remonte
};



class CTaskTerraform : public CTask
{
public:
	CTaskTerraform(CInstanceManager* iMan, CObject* object);
	~CTaskTerraform();

	BOOL		EventProcess(const Event &event);

	Error		Start();
	Error		IsEnded();
	BOOL		Abort();

protected:
	BOOL		Terraform();

protected:
	TaskTerraPhase	m_phase;
	float			m_progress;
	float			m_speed;
	float			m_time;
	float			m_lastParticule;
	int				m_soundChannel;
	BOOL			m_bError;
	D3DVECTOR		m_terraPos;
};


#endif //_TASKSTERRAFORM_H_
