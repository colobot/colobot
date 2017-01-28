// autocompass.h

#ifndef _AUTOCOMPASS_H_
#define	_AUTOCOMPASS_H_


class CInstanceManager;
class CD3DEngine;
class CParticule;
class CTerrain;
class CCamera;
class CObject;



class CAutoCompass : public CAuto
{
public:
	CAutoCompass(CInstanceManager* iMan, CObject* object);
	~CAutoCompass();

	void		DeleteObject(BOOL bAll=FALSE);

	void		Init();
	void		Start(int param);
	BOOL		EventProcess(const Event &event);
	BOOL		Abort();
	Error		RetError();

protected:
	CObject*	SearchObject(ObjectType type, D3DVECTOR center, float radius);

protected:
	float		m_progress;
	float		m_speed;
	float		m_angle;
	float		m_carSpeed;
	float		m_lastDetect;
	D3DVECTOR	m_targetPos;
};


#endif //_AUTOCOMPASS_H_
