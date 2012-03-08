// gauge.h

#ifndef _GAUGE_H_
#define	_GAUGE_H_


#include "control.h"


class CD3DEngine;



class CGauge : public CControl
{
public:
	CGauge(CInstanceManager* iMan);
	~CGauge();

	BOOL	Create(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg);

	BOOL	EventProcess(const Event &event);

	void	Draw();

	void	SetLevel(float level);
	float	RetLevel();

protected:

protected:
	float	m_level;
};


#endif //_GAUGE_H_
