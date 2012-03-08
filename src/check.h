// check.h

#ifndef _CHECK_H_
#define	_CHECK_H_


#include "control.h"


class CD3DEngine;



class CCheck : public CControl
{
public:
	CCheck(CInstanceManager* iMan);
	~CCheck();

	BOOL	Create(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg);

	BOOL	EventProcess(const Event &event);

	void	Draw();

protected:

protected:
};


#endif //_CHECK_H_
