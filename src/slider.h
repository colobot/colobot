// slider.h

#ifndef _SLIDER_H_
#define	_SLIDER_H_


#include "control.h"


class CD3DEngine;
class CButton;



class CSlider : public CControl
{
public:
	CSlider(CInstanceManager* iMan);
	~CSlider();

	BOOL		Create(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg);

	void		SetPos(FPOINT pos);
	void		SetDim(FPOINT dim);

	BOOL		SetState(int state, BOOL bState);
	BOOL		SetState(int state);
	BOOL		ClearState(int state);

	BOOL		EventProcess(const Event &event);
	void		Draw();

	void		SetLimit(float min, float max);

	void		SetVisibleValue(float value);
	float		RetVisibleValue();

	void		SetArrowStep(float step);
	float		RetArrowStep();

protected:
	void		MoveAdjust();
	void		AdjustGlint();
	void		DrawVertex(FPOINT pos, FPOINT dim, int icon);

protected:
	CButton*	m_buttonLeft;
	CButton*	m_buttonRight;

	float		m_min;
	float		m_max;
	float		m_visibleValue;
	float		m_step;

	BOOL		m_bHoriz;
	float		m_marginButton;

	BOOL		m_bCapture;
	FPOINT		m_pressPos;
	float		m_pressValue;

	EventMsg	m_eventUp;
	EventMsg	m_eventDown;
};


#endif //_SLIDER_H_
