// gauge.h

#ifndef _GAUGE_H_
#define	_GAUGE_H_


#include "control.h"


class CD3DEngine;


enum GaugeMode
{
	GM_NORMAL	= 0,
	GM_SPEED	= 1,
	GM_RPM		= 2,
	GM_COMPASS	= 3,
	GM_LEVEL1	= 4,
	GM_LEVEL2	= 5,
};



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

	void	SetMode(GaugeMode mode);
	GaugeMode RetMode();

protected:

protected:
	float		m_level;
	GaugeMode	m_mode;
};


#endif //_GAUGE_H_
