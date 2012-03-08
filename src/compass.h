// compass.h

#ifndef _COMPASS_H_
#define	_COMPASS_H_


#include "control.h"


class CD3DEngine;



class CCompass : public CControl
{
public:
	CCompass(CInstanceManager* iMan);
	~CCompass();

	BOOL	Create(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg);

	BOOL	EventProcess(const Event &event);

	void	Draw();

	void	SetDirection(float dir);
	float	RetDirection();

protected:

protected:
	float	m_dir;
};


#endif //_COMPASS_H_
