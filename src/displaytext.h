// displaytext.h

#ifndef _DISPLAYTEXT_H_
#define	_DISPLAYTEXT_H_


class CInstanceManager;
class CD3DEngine;
class CInterface;
class CObject;
class CSound;


enum TextType
{
	TT_ERROR	= 1,
	TT_WARNING	= 2,
	TT_INFO		= 3,
	TT_MESSAGE	= 4,
};

#define MAXDTLINE	4


class CDisplayText
{
public:
	CDisplayText(CInstanceManager* iMan);
	~CDisplayText();

	void		DeleteObject();

	BOOL		EventProcess(const Event &event);

	void		DisplayError(Error err, CObject* pObj, float time=10.0f);
	void		DisplayError(Error err, D3DVECTOR goal, float height=15.0f, float dist=60.0f, float time=10.0f);
	void		DisplayText(char *text, CObject* pObj, float time=10.0f, TextType type=TT_INFO);
	void		DisplayText(char *text, D3DVECTOR goal, float height=15.0f, float dist=60.0f, float time=10.0f, TextType type=TT_INFO);
	void		HideText(BOOL bHide);
	void		ClearText();
	BOOL		ClearLastText();
	void		SetDelay(float factor);
	void		SetEnable(BOOL bEnable);

	D3DVECTOR	RetVisitGoal(EventMsg event);
	float		RetVisitDist(EventMsg event);
	float		RetVisitHeight(EventMsg event);

	float		RetIdealDist(CObject* pObj);
	float		RetIdealHeight(CObject* pObj);

	void		ClearVisit();
	void		SetVisit(EventMsg event);
	BOOL		IsVisit(EventMsg event);

protected:
	CObject*	SearchToto();

protected:
	CInstanceManager* m_iMan;
	CD3DEngine*		m_engine;
	CInterface*		m_interface;
	CSound*			m_sound;

	BOOL			m_bExist[MAXDTLINE];
	float			m_time[MAXDTLINE];
	D3DVECTOR		m_visitGoal[MAXDTLINE];
	float			m_visitDist[MAXDTLINE];
	float			m_visitHeight[MAXDTLINE];

	BOOL			m_bHide;
	BOOL			m_bEnable;
	float			m_delayFactor;
};


#endif //_DISPLAYTEXT_H_
