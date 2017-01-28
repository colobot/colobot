// link.h

#ifndef _LINK_H_
#define	_LINK_H_


#include "control.h"


class CD3DEngine;



class CLink : public CControl
{
public:
	CLink(CInstanceManager* iMan);
	~CLink();

	BOOL	Create(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg);

	BOOL	EventProcess(const Event &event);

	void	Draw();

	void	SetPoints(FPOINT src, FPOINT dst, BOOL bGreen);

protected:

protected:
	FPOINT	m_src;
	FPOINT	m_dst;
	BOOL	m_bGreen;
};


#endif //_LINK_H_
