// label.h

#ifndef _LABEL_H_
#define	_LABEL_H_


#include "control.h"


class CD3DEngine;



class CLabel : public CControl
{
public:
	CLabel(CInstanceManager* iMan);
	~CLabel();

	BOOL	Create(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg);

	BOOL	EventProcess(const Event &event);

	void	Draw();

protected:

protected:
};


#endif //_LABEL_H_
