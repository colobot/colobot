// group.h

#ifndef _GROUP_H_
#define	_GROUP_H_


#include "control.h"


class CD3DEngine;



class CGroup : public CControl
{
public:
	CGroup(CInstanceManager* iMan);
	~CGroup();

	BOOL	Create(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg);

	BOOL	EventProcess(const Event &event);

	void	Draw();

protected:

protected:
};


#endif //_GROUP_H_
