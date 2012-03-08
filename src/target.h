// target.h

#ifndef _TARGET_H_
#define	_TARGET_H_


#include "control.h"


class CD3DEngine;
class CObject;



class CTarget : public CControl
{
public:
	CTarget(CInstanceManager* iMan);
	~CTarget();

	BOOL		Create(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg);

	BOOL		EventProcess(const Event &event);
	void		Draw();
	BOOL		GetTooltip(FPOINT pos, char* name);

protected:
	CObject*	DetectFriendObject(FPOINT pos);

protected:
};


#endif //_TARGET_H_
