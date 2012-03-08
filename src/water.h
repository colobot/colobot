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


enum WaterType
{
	WATER_NULL		= 0,	// pas d'eau
	WATER_TT		= 1,	// texture transparente
	WATER_TO		= 2,	// texture opaque
	WATER_CT		= 3,	// couleur transparente
	WATER_CO		= 4,	// couleur opaque
};


class CWater
{
public:
	CWater(CInstanceManager* iMan, CD3DEngine* engine);
	~CWater();

	void		SetD3DDevice(LPDIRECT3DDEVICE7 device);
	BOOL		EventProcess(const Event &event);
	void		Flush();
	BOOL		Create(WaterType type1, WaterType type2, const char *filename, D3DCOLORVALUE diffuse, D3DCOLORVALUE ambient, float level, float glint, D3DVECTOR eddy);
	void		DrawBack();
	void		DrawSurf();

	BOOL		SetLevel(float level);
	float		RetLevel();
	float		RetLevel(CObject* object);

	void		SetLava(BOOL bLava);
	BOOL		RetLava();

	void		AdjustEye(D3DVECTOR &eye);

protected:
	BOOL		EventFrame(const Event &event);
	void		LavaFrame(float rTime);
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
	D3DCOLORVALUE	m_diffuse;		// couleur diffuse
	D3DCOLORVALUE	m_ambient;		// couleur ambiante
	float			m_time;
	float			m_lastLava;
	int				m_subdiv;

	int				m_brick;		// nb de briques*mosaïque
	float			m_size;			// taille d'un élément dans une brique

	int				m_lineUsed;
	WaterLine		m_line[MAXWATERLINE];

	WaterVapor		m_vapor[MAXWATVAPOR];

	BOOL			m_bDraw;
	BOOL			m_bLava;
	D3DCOLOR		m_color;
};


#endif //_WATER_H_
