// autohuston.h

#ifndef _AUTOHUSTON_H_
#define	_AUTOHUSTON_H_


class CInstanceManager;
class CD3DEngine;
class CParticule;
class CTerrain;
class CCamera;
class CObject;

enum ParticuleType;



typedef struct
{
	int				parti;
	ParticuleType	type;
	D3DVECTOR		pos;
	float			dim;
	float			total;
	float			off;
}
HustonLens;


#define HUSTONMAXLENS	20


class CAutoHuston : public CAuto
{
public:
	CAutoHuston(CInstanceManager* iMan, CObject* object);
	~CAutoHuston();

	void		DeleteObject(BOOL bAll=FALSE);

	void		Init();
	void		Start(int param);
	BOOL		EventProcess(const Event &event);
	BOOL		Abort();
	Error		RetError();

	BOOL		CreateInterface(BOOL bSelect);

protected:

protected:
	float		m_progress;
	float		m_speed;
	HustonLens	m_lens[HUSTONMAXLENS];
	int			m_lensTotal;
};


#endif //_AUTOHUSTON_H_
