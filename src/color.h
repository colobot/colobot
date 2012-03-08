// color.h

#ifndef _COLOR_H_
#define	_COLOR_H_


#include "control.h"


class CD3DEngine;



class CColor : public CControl
{
public:
	CColor(CInstanceManager* iMan);
	~CColor();

	BOOL	Create(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg);

	BOOL	EventProcess(const Event &event);

	void	Draw();

	void	SetRepeat(BOOL bRepeat);
	BOOL	RetRepeat();

	void	SetColor(D3DCOLORVALUE color);
	D3DCOLORVALUE RetColor();

protected:

protected:
	BOOL			m_bRepeat;
	float			m_repeat;
	D3DCOLORVALUE	m_color;
};


#endif //_COLOR_H_
