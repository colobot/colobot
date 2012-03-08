// control.h

#ifndef _CONTROL_H_
#define	_CONTROL_H_


class CInstanceManager;
class CEvent;
class CD3DEngine;
class CRobotMain;
class CParticule;
class CSound;

enum FontType;


#define STATE_ENABLE	(1<<0)	// actif
#define STATE_CHECK		(1<<1)	// enfoncé
#define STATE_HILIGHT	(1<<2)	// survolé par la souris
#define STATE_PRESS		(1<<3)	// pressé par la souris
#define STATE_VISIBLE	(1<<4)	// visible
#define STATE_DEAD		(1<<5)	// inaccessible (x)
#define STATE_DEFAULT	(1<<6)	// actionné par RETURN
#define STATE_OKAY		(1<<7)	// point vert en bas à droite
#define STATE_SHADOW	(1<<8)	// ombre
#define STATE_GLINT		(1<<9)	// reflet dynamique
#define STATE_CARD		(1<<10)	// onglet
#define STATE_EXTEND	(1<<11)	// mode étendu
#define STATE_SIMPLY	(1<<12)	// sans ornements
#define STATE_FRAME		(1<<13)	// cadre de mise en évidence
#define STATE_WARNING	(1<<14)	// cadre hachuré jaune/noir
#define STATE_VALUE		(1<<15)	// affiche la valeur
#define STATE_RUN		(1<<16)	// programme en cours



class CControl
{
public:
	CControl(CInstanceManager* iMan);
	virtual ~CControl();

	virtual BOOL	Create(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg);

	virtual BOOL	EventProcess(const Event &event);

	virtual void	SetPos(FPOINT pos);
	virtual FPOINT	RetPos();
	virtual void	SetDim(FPOINT dim);
	virtual FPOINT	RetDim();
	virtual BOOL	SetState(int state, BOOL bState);
	virtual BOOL	SetState(int state);
	virtual BOOL	ClearState(int state);
	virtual BOOL	TestState(int state);
	virtual	int		RetState();
	virtual void	SetIcon(int icon);
	virtual int		RetIcon();
	virtual void	SetName(char* name, BOOL bTooltip=TRUE);
	virtual char*	RetName();
	virtual void	SetJustif(int mode);
	virtual int		RetJustif();
	virtual	void	SetFontSize(float size);
	virtual	float	RetFontSize();
	virtual	void	SetFontStretch(float stretch);
	virtual	float	RetFontStretch();
	virtual void	SetFontType(FontType font);
	virtual FontType RetFontType();
	virtual BOOL	SetTooltip(char* name);
	virtual BOOL	GetTooltip(FPOINT pos, char* name);
	virtual void	SetFocus(BOOL bFocus);
	virtual BOOL	RetFocus();

	virtual EventMsg RetEventMsg();

	virtual void	Draw();

protected:
			void	GlintDelete();
			void	GlintCreate(FPOINT ref, BOOL bLeft=TRUE, BOOL bUp=TRUE);
			void	GlintFrame(const Event &event);
			void	DrawPart(int icon, float zoom, float ex);
			void	DrawIcon(FPOINT pos, FPOINT dim, FPOINT uv1, FPOINT uv2, float ex=0.0f);
			void	DrawIcon(FPOINT pos, FPOINT dim, FPOINT uv1, FPOINT uv2, FPOINT corner, float ex);
			void	DrawWarning(FPOINT pos, FPOINT dim);
			void	DrawShadow(FPOINT pos, FPOINT dim, float deep=1.0f);
	virtual	BOOL	Detect(FPOINT pos);

protected:
	CInstanceManager* m_iMan;
	CD3DEngine*	m_engine;
	CEvent*		m_event;
	CRobotMain*	m_main;
	CParticule*	m_particule;
	CSound*		m_sound;

	FPOINT		m_pos;			// coin sup/gauche
	FPOINT		m_dim;			// dimensions
	int			m_icon;
	EventMsg	m_eventMsg;		// message à envoyer si clic
	int			m_state;		// états (STATE_*)
	float		m_fontSize;		// taille du nom du bouton
	float		m_fontStretch;	// stretch de la fonte
	FontType	m_fontType;		// type de la fonte
	int			m_justif;		// type de justification (-1,0,1)
	char		m_name[100];	// nom du bouton
	char		m_tooltip[100];	// nom du tooltip
	BOOL		m_bFocus;
	BOOL		m_bCapture;

	BOOL		m_bGlint;
	FPOINT		m_glintCorner1;
	FPOINT		m_glintCorner2;
	float		m_glintProgress;
	FPOINT		m_glintMouse;
};


#endif //_CONTROL_H_
