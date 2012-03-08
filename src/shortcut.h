// shortcut.h

#ifndef _SHORTCUT_H_
#define	_SHORTCUT_H_


#include "control.h"


class CD3DEngine;



class CShortcut : public CControl
{
public:
	CShortcut(CInstanceManager* iMan);
	~CShortcut();

	BOOL	Create(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg);

	BOOL	EventProcess(const Event &event);

	void	Draw();

protected:
	void	DrawVertex(int icon, float zoom);

protected:
	float	m_time;
};


#endif //_SHORTCUT_H_
