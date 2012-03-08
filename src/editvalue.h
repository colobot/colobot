// editvalue.h

#ifndef _EDITVALUE_H_
#define	_EDITVALUE_H_


#include "control.h"


enum EditValueType
{
	EVT_INT		= 1,	// valeur entière
	EVT_FLOAT	= 2,	// valeur réelle
	EVT_100		= 3,	// pour-cent (0..1)
};


class CD3DEngine;
class CEdit;
class CButton;



class CEditValue : public CControl
{
public:
	CEditValue(CInstanceManager* iMan);
	~CEditValue();

	BOOL		Create(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg);

	void		SetPos(FPOINT pos);
	void		SetDim(FPOINT dim);

	BOOL		EventProcess(const Event &event);
	void		Draw();

	void		SetType(EditValueType type);
	EditValueType RetType();

	void		SetValue(float value, BOOL bSendMessage=FALSE);
	float		RetValue();

	void		SetStepValue(float value);
	float		RetStepValue();

	void		SetMinValue(float value);
	float		RetMinValue();

	void		SetMaxValue(float value);
	float		RetMaxValue();

protected:
	void		MoveAdjust();
	void		HiliteValue(const Event &event);

protected:
	CEdit*		m_edit;
	CButton*	m_buttonUp;
	CButton*	m_buttonDown;

	EditValueType m_type;
	float		m_stepValue;
	float		m_minValue;
	float		m_maxValue;
};


#endif //_EDITVALUE_H_
