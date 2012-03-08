// mainshort.h

#ifndef _MAINSHORT_H_
#define	_MAINSHORT_H_


class CInstanceManager;
class CEvent;
class CD3DEngine;
class CInterface;
class CRobotMain;
class CObject;



class CMainShort
{
public:
	CMainShort(CInstanceManager* iMan);
	~CMainShort();

	void		SetMode(BOOL bBuilding);
	void		FlushShortcuts();
	BOOL		CreateShortcuts();
	BOOL		UpdateShortcuts();
	void		SelectShortcut(EventMsg event);
	void		SelectNext();
	CObject*	DetectShort(FPOINT pos);
	void		SetHilite(CObject* pObj);

protected:

protected:
	CInstanceManager* m_iMan;
	CEvent*			m_event;
	CD3DEngine*		m_engine;
	CInterface*		m_interface;
	CRobotMain*		m_main;

	CObject*		m_shortcuts[20];
	BOOL			m_bBuilding;
};


#endif //_MAINSHORT_H_
