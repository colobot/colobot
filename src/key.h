// key.h

#ifndef _KEY_H_
#define	_KEY_H_


#include "control.h"


class CD3DEngine;



class CKey : public CControl
{
public:
	CKey(CInstanceManager* iMan);
	~CKey();

	BOOL	Create(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg);

	BOOL	EventProcess(const Event &event);

	void	Draw();

	void	SetKey(int option, int key);
	int		RetKey(int option);

protected:
	BOOL	TestKey(int key);

protected:
	int		m_key[2];
	BOOL	m_bCatch;
};


#endif //_KEY_H_
