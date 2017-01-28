// interface.h

#ifndef _INTERFACE_H_
#define	_INTERFACE_H_


class CInstanceManager;
class CD3DEngine;
class CControl;
class CWindow;
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
class CArray;
class CShortcut;
class CMap;
class CGauge;
class CCompass;
class CProgress;
class CMenu;
class CCamera;

enum D3DMouse;


#define MAXCONTROL	100


class CInterface
{
public:
	CInterface(CInstanceManager* iMan);
	~CInterface();

	BOOL		EventProcess(const Event &event);
	BOOL		GetTooltip(FPOINT pos, char* name);

	void		SetDefMouse(D3DMouse mouse);

	void		Flush();
	CWindow*	CreateWindows(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg);
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
	CArray*		CreateArray(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg, float expand=1.2f);
	CShortcut*	CreateShortcut(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg);
	CCompass*	CreateCompass(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg);
	CGauge*		CreateGauge(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg);
	CProgress*	CreateProgress(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg);
	CMenu*		CreateMenu(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg);
	CMap*		CreateMap(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg);
	BOOL		DeleteControl(EventMsg eventMsg);
	CControl*	SearchControl(EventMsg eventMsg);
	CControl*	SearchControl(int tabOrder);
	CControl*	SearchControl();

	void		Draw();

protected:

protected:
	CInstanceManager* m_iMan;
	CD3DEngine*		m_engine;
	CCamera*		m_camera;

	CControl*		m_table[MAXCONTROL];
	D3DMouse		m_defMouse;
};


#endif //_INTERFACE_H_
