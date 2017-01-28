// water.h

#ifndef _WATER_H_
#define	_WATER_H_


class CInstanceManager;
class CD3DEngine;
class CTerrain;
class CParticule;
class CSound;

enum ParticuleType;



#define MAXWATERLINE	500

typedef struct
{
	short		x, y;		// début
	short		len;		// longueur en x
	float		px1, px2, pz;
}
WaterLine;


#define MAXWATVAPOR		10

typedef struct
{
	BOOL			bUsed;
	ParticuleType	type;
	D3DVECTOR		pos;
	float			delay;
	float			time;
	float			last;
}
WaterVapor;


#define MAXWATERPICK	5

typedef struct
{
	float		progress;
	float		speed;
	float		height;
	float		radius;
	D3DVECTOR	pos;
}
WaterPick;


enum WaterType
{
	WATER_NULL		= 0,	// pas d'eau
	WATER_TT		= 1,	// texture transparente
	WATER_TO		= 2,	// texture opaque
	WATER_CT		= 3,	// couleur transparente
	WATER_CO		= 4,	// couleur opaque
};

enum Meteo
{
	METEO_NORM		= 0,	// beau fixe
	METEO_LAVA		= 1,	// lave rouge
	METEO_ORGA		= 2,	// lave organique
	METEO_SNOW		= 3,	// pluie
	METEO_RAIN		= 4,	// neige
};



class CWater
{
public:
	CWater(CInstanceManager* iMan, CD3DEngine* engine);
	~CWater();

	void		SetD3DDevice(LPDIRECT3DDEVICE7 device);
	BOOL		EventProcess(const Event &event);
	void		Flush();
	BOOL		Init(WaterType type1, WaterType type2, const char *filename, D3DCOLORVALUE diffuse, D3DCOLORVALUE ambient, float level, float glint, D3DVECTOR eddy, D3DVECTOR vortex, float pick, float tension, float shadowForce, D3DCOLOR farColorLight, D3DCOLOR farColorDark, float farStart, float farEnd, Meteo meteo, BOOL bBold);
	BOOL		Create();
	void		DrawBack();
	void		DrawSurf();

	BOOL		SetLevel(float level);
	float		RetLevel();
	float		RetLevel(CObject* object);

	void		AdjustEye(D3DVECTOR &eye);

	BOOL		SearchArea(D3DVECTOR &pos, float length);
	BOOL		IsWaterRect(const D3DVECTOR &p1, const D3DVECTOR &p2);

protected:
	BOOL		EventFrame(const Event &event);
	void		LavaFrame(float rTime);
	void		WaterPickFlush();
	BOOL		WaterPickCheck(int rank);
	void		WaterPickFrame(float rTime);
	void		AdjustLevel(D3DVECTOR &pos, D3DVECTOR &norm, FPOINT &uv1, FPOINT &uv2);
	BOOL		RetWater(int x, int y);
	BOOL		CreateLine(int x, int y, int len);

	void		VaporFlush();
	BOOL		VaporCreate(ParticuleType type, D3DVECTOR pos, float delay);
	void		VaporFrame(int i, float rTime);

protected:
	CInstanceManager*	m_iMan;
	CD3DEngine*			m_engine;
	LPDIRECT3DDEVICE7	m_pD3DDevice;
	CTerrain*			m_terrain;
	CParticule*			m_particule;
	CSound*				m_sound;

	WaterType		m_type[2];
	char			m_filename[100];
	float			m_level;		// niveau global
	float			m_glint;		// amplitude des reflets
	D3DVECTOR		m_eddy;			// amplitude des remous
	D3DVECTOR		m_vortex;		// amplitude des tourbillons
	D3DCOLORVALUE	m_diffuse;		// couleur diffuse
	D3DCOLORVALUE	m_ambient;		// couleur ambiante
	D3DCOLOR		m_farColorLight;// couleur brouillard
	D3DCOLOR		m_farColorDark;	// couleur brouillard
	float			m_farStart;
	float			m_farEnd;
	float			m_pick;
	float			m_tension;
	float			m_shadowForce;
	float			m_time;
	float			m_lastLava;
	BOOL			m_bBold;

	int				m_nbTiles;		// nb de tuiles
	float			m_dimTile;		// taille d'un élément dans une brique

	int				m_lineUsed;
	WaterLine		m_line[MAXWATERLINE];

	WaterVapor		m_vapor[MAXWATVAPOR];
	WaterPick		m_waterPick[MAXWATERPICK];

	BOOL			m_bDraw;
	Meteo			m_meteo;
	D3DCOLOR		m_color;
};


#endif //_WATER_H_
