// scroll.h

#ifndef _SCROLL_H_
#define	_SCROLL_H_


#include "control.h"


class CD3DEngine;
class CButton;


#define SCROLL_WIDTH	(15.0f/640.0f)



class CScroll : public CControl
{
public:
	CScroll(CInstanceManager* iMan);
	~CScroll();

	BOOL		Create(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg);

	void		SetPos(FPOINT pos);
	void		SetDim(FPOINT dim);

	BOOL		SetState(int state, BOOL bState);
	BOOL		SetState(int state);
	BOOL		ClearState(int state);

	BOOL		EventProcess(const Event &event);
	void		Draw();

	void		SetVisibleValue(float value);
	float		RetVisibleValue();

	void		SetVisibleRatio(float value);
	float		RetVisibleRatio();

	void		SetArrowStep(float step);
	float		RetArrowStep();

protected:
	void		MoveAdjust();
	void		AdjustGlint();
	void		DrawVertex(FPOINT pos, FPOINT dim, int icon);

protected:
	CButton*	m_buttonUp;
	CButton*	m_buttonDown;

	float		m_visibleValue;
	float		m_visibleRatio;
	float		m_step;

	BOOL		m_bCapture;
	FPOINT		m_pressPos;
	float		m_pressValue;

	EventMsg	m_eventUp;
	EventMsg	m_eventDown;
};


#endif //_SCROLL_H_
