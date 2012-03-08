// button.h

#ifndef _BUTTON_H_
#define	_BUTTON_H_


#include "control.h"


class CD3DEngine;



class CButton : public CControl
{
public:
	CButton(CInstanceManager* iMan);
	~CButton();

	BOOL	Create(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg);

	BOOL	EventProcess(const Event &event);

	void	Draw();

	void	SetImmediat(BOOL bRepeat);
	BOOL	RetImmediat();

	void	SetRepeat(BOOL bRepeat);
	BOOL	RetRepeat();

protected:

protected:
	BOOL	m_bCapture;
	BOOL	m_bImmediat;
	BOOL	m_bRepeat;
	float	m_repeat;
};


#endif //_BUTTON_H_
