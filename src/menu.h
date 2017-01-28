// menu.h

#ifndef _MENU_H_
#define	_MENU_H_


#include "control.h"


class CD3DEngine;
class CButton;
class CGroup;



class CMenu : public CControl
{
public:
	CMenu(CInstanceManager* iMan);
	~CMenu();

	BOOL		Create(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg);

	BOOL		EventProcess(const Event &event);

	void		Draw();

	void		SetSelect(int select);
	int			RetSelect();

	void		SetSubTotal(int total);
	int			RetSubTotal();

	void		SetSubIcon(int rank, int icon);
	int			RetSubIcon(int rank);

	void		SetSubTexture(int rank, char *filename, FPOINT uv1, FPOINT uv2);

protected:
	void		CreateSubMenu();
	void		DetectSubMenu(FPOINT mouse);
	void		DeleteSubMenu();

protected:
	BOOL		m_bCapture;
	int			m_initSelect;
	int			m_select;
	int			m_total;
	int			m_subIcon[20];
	char		m_texture[20][50];
	FPOINT		m_uv1[20];
	FPOINT		m_uv2[20];
	CButton*	m_buttons[20];
	CGroup*		m_group;
};


#endif //_MENU_H_
