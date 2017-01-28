// D3DEngine.h

#ifndef _D3DENGINE_H_
#define	_D3DENGINE_H_

#include "D3DApp.h"



class CInstanceManager;
class CObject;
class CLight;
class CText;
class CParticule;
class CWater;
class CCloud;
class CBlitz;
class CPlanet;
class CSound;
class CTerrain;


#define D3DMAXOBJECT		1200
#define D3DMAXSHADOW		500
#define D3DMAXGROUNDSPOT	100


enum SetupType
{
	ST_SHADOW,		// ombres
	ST_DIRTY,		// salissures
	ST_SUNBEAM,		// rayons du soleil
	ST_LENSFLARE,	// reflets dans l'objectif
	ST_DECOR,		// objets décoratifs
	ST_DETAIL,		// objets détaillés
	ST_METEO,		// particules météo
	ST_AMBIANCE,	// particules d'ambiance
	ST_EXPLOVIB,	// secousses lors d'explosions
	ST_SPEEDSCH,	// vitesse scroll horizontal
	ST_SPEEDSCV,	// vitesse scroll vertical
	ST_MOUSESCROLL,	// scroll avec la souris
	ST_INVSCH,		// inversion scroll horitzontal
	ST_INVSCV,		// inversion scroll vertical
	ST_MOVIE,		// séquences cinématiques
	ST_HELP,		// instructions et conseils
	ST_TOOLTIPS,	// bulles d'aide
	ST_NICEMOUSE,	// souris ombrée
	ST_ACCEL	,	// accélère si long à résoudre
	ST_VOLBLUPI,	// volume voix blupi
	ST_VOLSOUND,	// volume bruitages
	ST_VOLAMBIANCE,	// volume ambiance
	ST_SOUND3D,		// son 3d
};

enum D3DTypeObj
{
	TYPENULL		= 0,		// object inexistant
	TYPETERRAIN		= 1,		// terrain
	TYPEOBJECT		= 2,		// objet quelconque
	TYPEDESCENDANT	= 3,		// partie mobile d'un objet
	TYPEQUARTZ		= 4,		// objet de type quartz
	TYPEMETAL		= 5,		// objet de type métalique
	TYPEBLUPI		= 6,		// objet de type blupi
};

enum D3DTypeTri
{
	D3DTYPE6T		= 1,		// triangles
	D3DTYPE6S		= 2,		// surfaces
};

enum D3DMaping
{
	D3DMAPPINGX		= 1,
	D3DMAPPINGY		= 2,
	D3DMAPPINGZ		= 3,
	D3DMAPPING1X	= 4,
	D3DMAPPING1Y	= 5,
	D3DMAPPING1Z	= 6,
	D3DMAPPINGMX	= 7,
	D3DMAPPINGMY	= 8,
	D3DMAPPINGMZ	= 9,
};

enum D3DMouse
{
	D3DMOUSEHIDE	= 0,		// pas de souris
	D3DMOUSENORM	= 1,		// flèche standard
	D3DMOUSEWAIT	= 2,		// sablier
	D3DMOUSEEDIT	= 3,		// i-beam
	D3DMOUSEHAND	= 4,		// main pour actionner bouton
	D3DMOUSESELECT	= 5,		// sélectionne un blupi
	D3DMOUSEGOTO	= 6,		// déplace blupi
	D3DMOUSESCROLLL	= 7,		// <
	D3DMOUSESCROLLR	= 8,		// >
	D3DMOUSESCROLLU	= 9,		// ^
	D3DMOUSESCROLLD	= 10,		// v
};

enum D3DShadowType
{
	D3DSHADOWNORM		= 0,
	D3DSHADOWGLASS		= 1,
	D3DSHADOWPALISSADE	= 2,
	D3DSHADOWDOCK		= 3,
	D3DSHADOWBIRD		= 4,
	D3DSHADOWSQUARE		= 5,
};


#define D3DSTATENORMAL	0		// matériaux opaque normal
#define D3DSTATETTb		(1<<0)	// transparent selon texture (noir=rien)
#define D3DSTATETTw		(1<<1)	// transparent selon texture (blanc=rien)
#define D3DSTATETD		(1<<2)	// transparent selon couleur diffuse
#define D3DSTATEWRAP	(1<<3)	// texture wrappée
#define D3DSTATECLAMP	(1<<4)	// texture bordée d'une couleur unie
#define D3DSTATELIGHT	(1<<5)	// texture lumineuse (ambiance max)
#define D3DSTATEDUALb	(1<<6)	// double texturage noir
#define D3DSTATEDUALw	(1<<7)	// double texturage blanc
#define D3DSTATEPART1	(1<<8)	// partie 1 (ne pas changer, dans .MOD !)
#define D3DSTATEPART2	(1<<9)	// partie 2
#define D3DSTATEPART3	(1<<10)	// partie 3
#define D3DSTATEPART4	(1<<11)	// partie 4
#define D3DSTATE2FACE	(1<<12)	// mode double-face
#define D3DSTATEALPHA	(1<<13)	// image avec canal alpha
#define D3DSTATESECOND	(1<<14)	// utilise tjrs 2ème étage de texturage
#define D3DSTATEFOG		(1<<15)	// force le brouillard
#define D3DSTATETCb		(1<<16)	// transparent selon couleur (noir=rien)
#define D3DSTATETCw		(1<<17)	// transparent selon couleur (blanc=rien)
#define D3DSTATEACTION	(1<<18)	// bouton, flèche, etc.


typedef struct
{
	D3DVERTEX2		triangle[3];
	D3DMATERIAL7	material;
	int				state;
	char			texName1[20];
	char			texName2[20];
}
D3DTriangle;


typedef struct
{
	int				totalPossible;
	int				totalUsed;
	D3DMATERIAL7	material;
	int				state;
	D3DTypeTri		type;		// D3DTYPE6x
	D3DVERTEX2		vertex[1];
}
D3DObjLevel6;

typedef struct
{
	int				totalPossible;
	int				totalUsed;
	int				reserve;
	D3DObjLevel6*	table[1];
}
D3DObjLevel5;

typedef struct
{
	int				totalPossible;
	int				totalUsed;
	float			min, max;
	D3DObjLevel5*	table[1];
}
D3DObjLevel4;

typedef struct
{
	int				totalPossible;
	int				totalUsed;
	int				objRank;
	D3DObjLevel4*	table[1];
}
D3DObjLevel3;

typedef struct
{
	int				totalPossible;
	int				totalUsed;
	char			texName1[20];
	char			texName2[20];
	D3DObjLevel3*	table[1];
}
D3DObjLevel2b;

typedef struct
{
	int				totalPossible;
	int				totalUsed;
	short			type;
	D3DObjLevel2b*	table[1];
}
D3DObjLevel2a;

typedef struct
{
	int				totalPossible;
	int				totalUsed;
	D3DObjLevel2a*	table[1];
}
D3DObjLevel1;


typedef struct
{
	char		bUsed;			// TRUE -> objet existe
	char		bVisible;		// TRUE -> objet visible
	char		bHide;			// TRUE -> objet totalement caché
	char		bDetect;		// TRUE -> objet détectable
	char		bDrawWorld;		// TRUE -> dessine derrière l'interface
	char		bDrawFront;		// TRUE -> dessine devant l'interface
	int			totalTriangle;	// nb de triangles utilisés
	D3DTypeObj	type;			// type de l'objet (TYPE*)
	D3DMATRIX	transform;		// matrice de transformation
	float		distance;		// distance point de vue - origine
	D3DVECTOR	bboxMin;		// bounding box de l'objet
	D3DVECTOR	bboxMax;		// (l'origine 0;0;0 est tjrs incluse)
	float		radius;			// rayon de la sphère à l'origine
	int			shadowRank;		// rang de l'ombre associée
	float		transparency;	// transparence de l'objet (0..1)
}
D3DObject;

typedef struct
{
	char		bUsed;			// TRUE -> objet existe
	char		bHide;			// TRUE -> ombre invisible (objet porté par ex.)
	char		bSelect;		// TRUE -> objet sélectionné
	int			objRank;		// rang de l'objet
	D3DShadowType type;			// type de l'ombre
	D3DVECTOR	pos;			// position pour l'ombre
	float		angle;			// angle de l'ombre
	float		radius;			// rayon de l'ombre
	float		intensity;		// intensité de l'ombre
	float		height;			// hauteur depuis le sol
	float		sunFactor;		// hauteur pour le soleil
	float		time;			// temps depuis la création
	D3DVECTOR	posTerrain;		// position centre du terrain
	char		bHole[9];		// trous (ul,u,ur,l,c,r,dl,d,dr)
}
D3DShadow;

typedef struct
{
	char		bUsed;			// TRUE -> objet existe
	D3DCOLORVALUE color;		// couleur de l'ombre
	float		min, max;		// altitudes min/max
	float		smooth;			// zone de transition
	D3DVECTOR	pos;			// position pour l'ombre
	float		radius;			// rayon de l'ombre
	D3DVECTOR	drawPos;		// position pour l'ombre dessinée
	float		drawRadius;		// rayon de l'ombre dessinée
}
D3DGroundSpot;

typedef struct
{
	char		bUsed;			// TRUE -> objet existe
	char		bDraw;			// TRUE -> marque dessinée
	int			phase;			// 1=croissance, 2=fixe, 3=décroissance
	float		delay[3];		// délais pour les 3 phases
	float		fix;			// temps fixe
	D3DVECTOR	pos;			// position pour marques
	float		radius;			// rayon des marques
	float		intensity;		// intensité couleur
	D3DVECTOR	drawPos;		// position pour marques dessinées
	float		drawRadius;		// rayon des marques dessinées
	float		drawIntensity;	// intensité dessinée
	int			dx, dy;			// dimensions table
	char*		table;			// pointeur à la table
}
D3DGroundMark;


#define REPLACETEXMAX	10

typedef struct
{
	char		actual[20];		// nom actuel de la texture
	char		future[20];		// nouveau nom souhaité
}
ReplaceTex;


#define MAXLENSFLARE	10

typedef struct
{
	char		texName[20];	// nom de la texture
	float		dist;			// distance dans l'objectif (-1..1)
	float		dim;			// dimension (côté du carré)
	float		intensity;		// intensité de base
	FPOINT		offset;			// offset par-rapport à l'objectif
}
LensFlare;




class CD3DEngine
{
public:
	CD3DEngine(CInstanceManager *iMan, CD3DApplication *app);
	~CD3DEngine();

	void		SetD3DDevice(LPDIRECT3DDEVICE7 device);
	LPDIRECT3DDEVICE7 RetD3DDevice();

	void		SetTerrain(CTerrain* terrain);

	BOOL		WriteProfile();

	void		SetPause(BOOL bPause);
	BOOL		RetPause();

	void		SetMovieLock(BOOL bLock);
	BOOL		RetMovieLock();

	void		SetShowStat(BOOL bShow);
	BOOL		RetShowStat();

	void		SetRenderEnable(BOOL bEnable);

	HRESULT		OneTimeSceneInit();
	HRESULT		InitDeviceObjects();
	HRESULT		DeleteDeviceObjects();
	void		SetInterfaceMat(FPOINT center, float zoom, float angle);
	HRESULT		RestoreSurfaces();
	HRESULT		Render();
	HRESULT		FrameMove(float rTime);
	void		StepSimul(float rTime);
	HRESULT		FinalCleanup();
	void		AddStatisticTriangle(int nb);
	int			RetStatisticTriangle();
	void		SetHiliteRank(int *rankList);
	BOOL		GetHilite(FPOINT &p1, FPOINT &p2);
	void		SetSelect(FPOINT p1, FPOINT p2, BOOL bSelect);
	BOOL		GetSelect(FPOINT &p1, FPOINT &p2);
	BOOL		GetSpriteCoord(int &x, int &y);
	void		SetInfoText(int line, char* text);
	char*		RetInfoText(int line);
	LRESULT		MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void		FirstExecuteAdapt(BOOL bFirst);
	int			GetVidMemTotal();
	BOOL		IsVideo8MB();
	BOOL		IsVideo32MB();

	BOOL		EnumDevices(char *bufDevices, int lenDevices, char *bufModes, int lenModes, int &totalDevices, int &selectDevices, int &totalModes, int &selectModes);
	BOOL		RetFullScreen();
	BOOL		ChangeDevice(char *device, char *mode, BOOL bFull);

	D3DMATRIX*	RetMatView();
	D3DMATRIX*	RetMatLeftView();
	D3DMATRIX*	RetMatRightView();

	void		TimeInit();
	void		TimeEnterGel();
	void		TimeExitGel();
	float		TimeGet();

	int			RetRestCreate();
	int			CreateObject();
	void		FlushObject();
	BOOL		DeleteObject(int objRank);
	BOOL		SetDrawWorld(int objRank, BOOL bDraw);
	BOOL		SetDrawFront(int objRank, BOOL bDraw);
	BOOL		AddTriangle(int objRank, D3DVERTEX2* vertex, int nb, const D3DMATERIAL7 &mat, int state, char* texName1, char* texName2, float min, float max, BOOL bGlobalUpdate);
	BOOL		AddSurface(int objRank, D3DVERTEX2* vertex, int nb, const D3DMATERIAL7 &mat, int state, char* texName1, char* texName2, float min, float max, BOOL bGlobalUpdate);
	BOOL		AddQuick(int objRank, D3DObjLevel6* buffer, char* texName1, char* texName2, float min, float max, BOOL bGlobalUpdate);
	D3DObjLevel6* SearchTriangle(int objRank, const D3DMATERIAL7 &mat, int state, char* texName1, char* texName2, float min, float max);
	D3DObjLevel6* SearchTriangle(int objRank, char* texName1, char* texName2, float min, float max);
	void		ChangeLOD();
	BOOL		ChangeSecondTexture(int objRank, char* texName2);
	int			RetTotalTriangles(int objRank);
	int			GetTriangles(int objRank, float min, float max, D3DTriangle* buffer, int size, float percent);
	BOOL		GetBBox(int objRank, D3DVECTOR &min, D3DVECTOR &max);
	BOOL		ChangeTextureMapping(int objRank, const D3DMATERIAL7 &mat, int state, char* texName1, char* texName2, float min, float max, D3DMaping mode, float au, float bu, float av, float bv);
	BOOL		AddTextureMapping(int objRank, char* texName1, char* texName2, float min, float max, D3DMaping mode, float offset);
	BOOL		TrackTextureMapping(int objRank, const D3DMATERIAL7 &mat, int state, char* texName1, char* texName2, float min, float max, D3DMaping mode, float pos, float factor, float tl, float ts, float tt);
	void		TransformObject(int objRank, D3DVECTOR move1, D3DVECTOR angle, D3DVECTOR move2);
	BOOL		SetObjectTransform(int objRank, const D3DMATRIX &transform);
	BOOL		GetObjectTransform(int objRank, D3DMATRIX &transform);
	BOOL		SetObjectType(int objRank, D3DTypeObj type);
	D3DTypeObj	RetObjectType(int objRank);
	BOOL		SetObjectHide(int objRank, BOOL bHide);
	BOOL		SetObjectDetect(int objRank, BOOL bDetect);
	BOOL		SetObjectTransparency(int objRank, float value);

	void		FlushReplaceTex();
	BOOL		SetReplaceTex(char *actual, char *future);
	void		ReplaceTexDo(char *texName);

	BOOL		ShadowCreate(int objRank);
	void		ShadowDelete(int objRank);
	BOOL		SetObjectShadowHide(int objRank, BOOL bHide);
	BOOL		SetObjectShadowType(int objRank, D3DShadowType type);
	BOOL		SetObjectShadowSelect(int objRank, BOOL bSelect);
	BOOL		SetObjectShadowPos(int objRank, const D3DVECTOR &pos);
	BOOL		SetObjectShadowAngle(int objRank, float angle);
	BOOL		SetObjectShadowRadius(int objRank, float radius);
	BOOL		SetObjectShadowIntensity(int objRank, float intensity);
	BOOL		SetObjectShadowHeight(int objRank, float h);
	BOOL		SetObjectShadowSunFactor(int objRank, float factor);
	BOOL		SetObjectShadowHole(int objRank, const D3DVECTOR &pos, char bHole[9]);
	float		RetObjectShadowRadius(int objRank);

	void		GroundSpotFlush();
	int			GroundSpotCreate();
	void		GroundSpotDelete(int rank);
	BOOL		SetObjectGroundSpotPos(int rank, const D3DVECTOR &pos);
	BOOL		SetObjectGroundSpotRadius(int rank, float radius);
	BOOL		SetObjectGroundSpotColor(int rank, D3DCOLORVALUE color);
	BOOL		SetObjectGroundSpotMinMax(int rank, float min, float max);
	BOOL		SetObjectGroundSpotSmooth(int rank, float smooth);

	int			GroundMarkCreate(D3DVECTOR pos, float radius, float delay1, float delay2, float delay3, int dx, int dy, char* table);
	BOOL		GroundMarkDelete(int rank);

	void		Update();
	
	void		SetViewParams(const D3DVECTOR &vEyePt, const D3DVECTOR &vLookatPt, const D3DVECTOR &vUpVec, FLOAT fEyeDistance);

	BOOL		FreeTexture(char* name);
	BOOL		LoadTexture(char* name, int stage=0);
	BOOL		LoadAllTexture();

	void		SetLimitLOD(int rank, float limit);
	float		RetLimitLOD(int rank, BOOL bLast=FALSE);

	void		SetGroundSpot(BOOL bMode);
	BOOL		RetGroundSpot();
	BOOL		RetStateColor();

	void		SetSecondTexture(int texNum);
	int			RetSecondTexture();

	void		SetRankView(int rank);
	int			RetRankView();

	void		SetDrawWorld(BOOL bDraw);
	void		SetDrawFront(BOOL bDraw);

	void		SetAmbiantColor(D3DCOLOR color, int rank=0);
	D3DCOLOR	RetAmbiantColor(int rank=0);

	void		SetWaterAddColor(D3DCOLORVALUE color);
	D3DCOLORVALUE RetWaterAddColor();

	void		SetFogColor(D3DCOLOR color, int rank=0);
	D3DCOLOR	RetFogColor(int rank=0);

	void		SetDeepView(float length, int rank=0, BOOL bRef=FALSE);
	float		RetDeepView(int rank=0);

	void		SetFogStart(float start, int rank=0);
	float		RetFogStart(int rank=0);

	void		SetBackground(char *name, D3DCOLOR up=0, D3DCOLOR down=0, float sunFlash=0.5f, BOOL bFull=FALSE, BOOL bQuarter=FALSE, BOOL bPanel=FALSE);
	void		RetBackground(char *name, D3DCOLOR &up, D3DCOLOR &down, float &sunFlash, BOOL &bFull, BOOL &bQuarter);
	void		LensFlareFlush();
	void		LensFlareAdd(char *texName, float dist, float dim, float intensity, float offsetH, float offsetV);
	void		SetOverFront(BOOL bFront);
	void		SetOverColor(D3DCOLOR color=0, int mode=D3DSTATETCb);

	void		DefaultSetup();
	BOOL		RetSetupName(SetupType type, char *buffer);
	void		SetSetup(SetupType type, float value);
	float		RetSetup(SetupType type);

	void		SetBlitzMode(BOOL bPresent);
	BOOL		RetBlitzMode();

	void		SetBackForce(BOOL bPresent);
	BOOL		RetBackForce();

	void		SetEditIndentMode(BOOL bAuto);
	BOOL		RetEditIndentMode();

	void		SetEditIndentValue(int value);
	int			RetEditIndentValue();

	void		SetSpeed(float speed);
	float		RetSpeed();

	void		SetFocus(float focus);
	float		RetFocus();
	D3DVECTOR	RetEyePt();
	D3DVECTOR	RetLookatPt();
	float		RetEyeDirH();
	float		RetEyeDirV();
	POINT		RetDim();
	void		UpdateMatProj();

	void		ApplyChange();

	void		FlushPressKey();
	void		ResetKey();
	void		SetKey(int keyRank, int option, int key);
	int			RetKey(int keyRank, int option);
	BOOL		IsKeyMouse(int key);
	BOOL		IsKeyJoystick(int key);

	void		SetForce(float force);
	float		RetForce();
	void		SetFFB(BOOL bMode);
	BOOL		RetFFB();
	void		SetJoystick(int mode);
	int			RetJoystick();
	BOOL		SetJoyForces(float forceX, float forceY);

	void		SetDebugMode(BOOL bMode);
	BOOL		RetDebugMode();

	BOOL		IsVisiblePoint(const D3DVECTOR &pos);

	BOOL		DetectTerrain(FPOINT mouse, D3DVECTOR &pos, int maxLevel);
	int			DetectObject(FPOINT mouse, BOOL bAction, BOOL bAllDetect=FALSE);
	BOOL		DetectTriangle(FPOINT mouse, D3DVERTEX2 *vertex, int objRank, float &dist, D3DVECTOR &pos, int level, int maxLevel);
	BOOL		DetectTriangle(FPOINT mouse, D3DVERTEX2 *vertex, int objRank, float &dist);

	void		SetState(int state, D3DCOLOR color=0xffffffff);
	void		SetTexture(char *name, int stage=0);
	void		SetMaterial(const D3DMATERIAL7 &mat);

	void		MoveMousePos(FPOINT pos);
	void		SetMousePos(FPOINT pos);
	FPOINT		RetMousePos();
	void		SetMouseType(D3DMouse type);
	D3DMouse	RetMouseType();
	void		SetMouseHide(BOOL bHide);
	BOOL		RetMouseHide();
	BOOL		RetNiceMouseCap();
	void		SetMouseCapture();
	void		ReleaseMouseCapture();

	void		InitLockZone();
	void		ShowLockZone(D3DVECTOR pos, BOOL bLock);
	void		CreateLockZone();

	CText*		RetText();

	BOOL		ChangeColor(char *name, D3DCOLORVALUE colorRef1, D3DCOLORVALUE colorNew1, D3DCOLORVALUE colorRef2, D3DCOLORVALUE colorNew2, float tolerance1, float tolerance2, FPOINT ts, FPOINT ti, FPOINT *pExclu=0, float shift=0.0f, BOOL bHSV=FALSE);
	BOOL		OpenImage(char *name);
	BOOL		CopyImage();
	BOOL		LoadImage();
	BOOL		ScrollImage(int dx, int dy);
	BOOL		SetDot(int x, int y, D3DCOLORVALUE color);
	BOOL		CloseImage();
	BOOL		WriteScreenShot(char *filename, int width, int height);

protected:
	void		MemSpace1 (D3DObjLevel1  *&p, int nb);
	void		MemSpace2a(D3DObjLevel2a *&p, int nb);
	void		MemSpace2b(D3DObjLevel2b *&p, int nb);
	void		MemSpace3 (D3DObjLevel3  *&p, int nb);
	void		MemSpace4 (D3DObjLevel4  *&p, int nb);
	void		MemSpace5 (D3DObjLevel5  *&p, int nb);
	void		MemSpace6 (D3DObjLevel6  *&p, int nb);

	D3DObjLevel2a* AddLevel1 (D3DObjLevel1  *&p1, short type);
	D3DObjLevel2b* AddLevel2a(D3DObjLevel2a *&p2, char* texName1, char* texName2);
	D3DObjLevel3*  AddLevel2b(D3DObjLevel2b *&p2, int objRank);
	D3DObjLevel4*  AddLevel3 (D3DObjLevel3  *&p3, float min, float max);
	D3DObjLevel5*  AddLevel4 (D3DObjLevel4  *&p4, int reserve);
	D3DObjLevel6*  AddLevel5 (D3DObjLevel5  *&p5, D3DTypeTri type, const D3DMATERIAL7 &mat, int state, int nb);

	void		TransformVertex(D3DVERTEX2* pv, int nb, const D3DVECTOR &move1, const D3DVECTOR &angle, const D3DVECTOR &move2);
	BOOL		IsVisible(int objRank);
	BOOL		DetectBBox(int objRank, FPOINT mouse);
	BOOL		TransformPoint(D3DVECTOR &p2D, int objRank, D3DVECTOR p3D);
	void		ComputeDistance();
	void		UpdateGeometry();
	void		RenderGroundSpot();
	void		FrameShadow(float rTime);
	void		DrawShadowPiece(int i, const D3DVECTOR &pos, const D3DVECTOR &shift, float adjust, float minx, float maxx, float minz, float maxz, FPOINT ti, FPOINT ts);
	void		DrawShadowRot(const D3DVECTOR &pos, float radius, float angle, FPOINT ti, FPOINT ts);
	void		DrawShadow();
	void		DrawBackground();
	void		DrawBackgroundGradient(FPOINT p1, FPOINT p2, D3DCOLOR up, D3DCOLOR down);
	void		DrawBackgroundImageQuarter(FPOINT p1, FPOINT p2, char *name);
	void		DrawBackgroundImage();
	void		DrawPlanet();
	void		DrawLensFlare();
	void		DrawLensFlare(int i);
	void		DrawOverColor();
	BOOL		GetBBox2D(int objRank, FPOINT &min, FPOINT &max);
	void		DrawHilite();
	void		DrawMouse();
	void		DrawSprite(FPOINT pos, FPOINT dim, int icon);

protected:
	CInstanceManager* m_iMan;
	CD3DApplication* m_app;
	LPDIRECT3DDEVICE7 m_pD3DDevice;
	CText*			m_text;
	CLight*			m_light;
	CParticule*		m_particule;
	CWater*			m_water;
	CCloud*			m_cloud;
	CBlitz*			m_blitz;
	CPlanet*		m_planet;
	CSound*			m_sound;
	CTerrain*		m_terrain;

	int				m_blackSrcBlend[2];
	int				m_blackDestBlend[2];
	int				m_whiteSrcBlend[2];
	int				m_whiteDestBlend[2];
	int				m_diffuseSrcBlend[2];
	int				m_diffuseDestBlend[2];
	int				m_alphaSrcBlend[2];
	int				m_alphaDestBlend[2];

	D3DMATRIX		m_matProj;
	D3DMATRIX		m_matLeftView;
	D3DMATRIX		m_matRightView;
	D3DMATRIX		m_matView;
	float			m_focus;

	D3DMATRIX		m_matWorldBack;
	D3DMATRIX		m_matProjBack;
	D3DMATRIX		m_matViewBack;

	D3DMATRIX		m_matWorldInterface;
	D3DMATRIX		m_matProjInterface;
	D3DMATRIX		m_matViewInterface;

	DWORD			m_baseTime;
	DWORD			m_stopTime;
	float			m_absTime;
	float			m_lastTime;
	float			m_speed;
	BOOL			m_bPause;
	BOOL			m_bRender;
	BOOL			m_bMovieLock;

	POINT			m_dim;
	POINT			m_lastDim;
	D3DObjLevel1*	m_objectPointer;
	int				m_objectParamTotal;
	D3DObject*		m_objectParam;
	int				m_shadowTotal;
	D3DShadow*		m_shadow;
	D3DGroundSpot*	m_groundSpot;
	D3DGroundMark	m_groundMark;
	D3DVECTOR		m_eyePt;
	D3DVECTOR		m_lookatPt;
	float			m_eyeDirH;
	float			m_eyeDirV;
	int				m_rankView;
	D3DCOLOR		m_ambiantColor[2];
	D3DCOLOR		m_backColor[2];
	D3DCOLOR		m_fogColor[2];
	float			m_deepView[2];
	float			m_fogStart[2];
	D3DCOLORVALUE	m_waterAddColor;
	int				m_statisticTriangle;
	BOOL			m_bUpdateGeometry;
	char			m_infoText[10][200];
	int				m_alphaMode;
	BOOL			m_bStateColor;
	BOOL			m_bForceStateColor;
	BOOL			m_bGroundSpot;
	BOOL			m_bFirstGroundSpot;
	int				m_secondTexNum;
	char			m_backgroundName[50];
	D3DCOLOR		m_backgroundColorUp;
	D3DCOLOR		m_backgroundColorDown;
	float			m_backgroundSunFlash;
	BOOL			m_bBackgroundFull;
	BOOL			m_bBackgroundQuarter;
	BOOL			m_bBackgroundPanel;
	BOOL			m_bOverFront;
	D3DCOLOR		m_overColor;
	int				m_overMode;
	int				m_lensFlareTotal;
	LensFlare		m_lensFlareTable[MAXLENSFLARE];
	BOOL			m_bDrawWorld;
	BOOL			m_bDrawFront;
	float			m_limitLOD[2];
	float			m_clippingDistance;
	float			m_lastClippingDistance;
	float			m_objectDetail;
	float			m_lastObjectDetail;
	float			m_terrainVision;
	BOOL			m_bBackForce;
	BOOL			m_bEditIndentMode;
	int				m_editIndentValue;
	float			m_setup[100];

	int				m_hiliteRank[100];
	BOOL			m_bHilite;
	FPOINT			m_hiliteP1;
	FPOINT			m_hiliteP2;
	BOOL			m_bSelect;
	FPOINT			m_selectP1;
	FPOINT			m_selectP2;

	int				m_lastState;
	D3DCOLOR		m_lastColor;
	char			m_lastTexture[2][50];
	D3DMATERIAL7	m_lastMaterial;

	FPOINT			m_mousePos;
	D3DMouse		m_mouseType;
	BOOL			m_bMouseHide;

	LPDIRECTDRAWSURFACE7 m_imageSurface;
	DDSURFACEDESC2	m_imageDDSD;
	WORD*			m_imageCopy;
	int				m_imageDX;
	int				m_imageDY;

	int*			m_lockZoneBuffer;

	ReplaceTex		m_replaceTex[REPLACETEXMAX];
};


#endif //_D3DENGINE_H_
