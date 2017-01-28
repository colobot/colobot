// displaytext.h

#ifndef _DISPLAYTEXT_H_
#define	_DISPLAYTEXT_H_


class CInstanceManager;
class CD3DEngine;
class CInterface;
class CObject;
class CSound;



#define FONTSIZE	12.0f



enum TextType
{
	TT_ERROR	= 1,
	TT_WARNING	= 2,
	TT_INFO		= 3,
	TT_MESSAGE	= 4,
	TT_START	= 5,
};

enum Sound;

#define MAXDTLINE	1


class CDisplayText
{
public:
	CDisplayText(CInstanceManager* iMan);
	~CDisplayText();

	void		DeleteObject();

	BOOL		EventProcess(const Event &event);

	void		DisplayError(Error err, float time=10.0f, float size=FONTSIZE);
	void		DisplayText(char *text, float time=10.0f, float size=FONTSIZE, TextType type=TT_INFO, Sound sound=SOUND_CLICK);
	void		HideText(BOOL bHide);
	void		ClearText();
	BOOL		ClearLastText();
	void		SetDelay(float factor);
	void		SetEnable(BOOL bEnable);

protected:
	CInstanceManager* m_iMan;
	CD3DEngine*		m_engine;
	CInterface*		m_interface;
	CSound*			m_sound;

	BOOL			m_bExist[MAXDTLINE];
	float			m_time[MAXDTLINE];

	BOOL			m_bHide;
	BOOL			m_bEnable;
	float			m_delayFactor;
	int				m_channelSound;
};


#endif //_DISPLAYTEXT_H_
