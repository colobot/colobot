// window.h

#ifndef _WINDOW_H_
#define	_WINDOW_H_


#include "control.h"


class CD3DEngine;
class CButton;
class CColor;
class CCheck;
class CKey;
class CGroup;
class CImage;
class CLabel;
class CEdit;
class CEditValue;
class CScroll;
class CSlider;
class CList;
class CShortcut;
class CMap;
class CGauge;
class CCompass;
class CTarget;


#define MAXWINDOW	100


class CWindow : public CControl
{
public:
	CWindow(CInstanceManager* iMan);
	~CWindow();

	void		Flush();
	BOOL		Create(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg);
	CButton*	CreateButton(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg);
	CColor*		CreateColor(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg);
	CCheck*		CreateCheck(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg);
	CKey*		CreateKey(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg);
	CGroup*		CreateGroup(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg);
	CImage*		CreateImage(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg);
	CLabel*		CreateLabel(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg, char *name);
	CEdit*		CreateEdit(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg);
	CEditValue*	CreateEditValue(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg);
	CScroll*	CreateScroll(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg);
	CSlider*	CreateSlider(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg);
	CList*		CreateList(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg, float expand=1.2f);
	CShortcut*	CreateShortcut(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg);
	CMap*		CreateMap(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg);
	CGauge*		CreateGauge(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg);
	CCompass*	CreateCompass(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg);
	CTarget*	CreateTarget(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg);
	BOOL		DeleteControl(EventMsg eventMsg);
	CControl*	SearchControl(EventMsg eventMsg);

	EventMsg	RetEventMsgReduce();
	EventMsg	RetEventMsgFull();
	EventMsg	RetEventMsgClose();

	void		SetName(char* name);

	void		SetTrashEvent(BOOL bTrash);
	BOOL		RetTrashEvent();

	void		SetPos(FPOINT pos);
	void		SetDim(FPOINT dim);

	void		SetMinDim(FPOINT dim);
	void		SetMaxDim(FPOINT dim);
	FPOINT		RetMinDim();
	FPOINT		RetMaxDim();

	void		SetMovable(BOOL bMode);
	BOOL		RetMovable();

	void		SetRedim(BOOL bMode);
	BOOL		RetRedim();

	void		SetClosable(BOOL bMode);
	BOOL		RetClosable();

	void		SetMaximized(BOOL bMaxi);
	BOOL		RetMaximized();
	void		SetMinimized(BOOL bMini);
	BOOL		RetMinimized();
	void		SetFixed(BOOL bFix);
	BOOL		RetFixed();

	BOOL		GetTooltip(FPOINT pos, char* name);

	BOOL		EventProcess(const Event &event);

	void		Draw();

protected:
	int			BorderDetect(FPOINT pos);
	void		AdjustButtons();
	void		MoveAdjust();
	void		DrawVertex(FPOINT pos, FPOINT dim, int icon);
	void		DrawHach(FPOINT pos, FPOINT dim);

protected:
	CControl*	m_table[MAXWINDOW];

	BOOL		m_bTrashEvent;
	BOOL		m_bMaximized;
	BOOL		m_bMinimized;
	BOOL		m_bFixed;

	FPOINT		m_minDim;
	FPOINT		m_maxDim;

	CButton*	m_buttonReduce;
	CButton*	m_buttonFull;
	CButton*	m_buttonClose;

	BOOL		m_bMovable;
	BOOL		m_bRedim;
	BOOL		m_bClosable;
	BOOL		m_bCapture;
	FPOINT		m_pressPos;
	int			m_pressFlags;
	D3DMouse	m_pressMouse;
};


#endif //_WINDOW_H_
