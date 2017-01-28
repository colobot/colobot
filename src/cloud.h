// cloud.h

#ifndef _CLOUD_H_
#define	_CLOUD_H_


class CInstanceManager;
class CD3DEngine;
class CTerrain;



#define MAXCLOUDLINE	100

typedef struct
{
	short		x, y;		// début
	short		len;		// longueur en x
	float		px1, px2, pz;
}
CloudLine;


class CCloud
{
public:
	CCloud(CInstanceManager* iMan, CD3DEngine* engine);
	~CCloud();

	BOOL		EventProcess(const Event &event);
	void		Flush();
	BOOL		Create(const char *filename, D3DCOLORVALUE diffuse, D3DCOLORVALUE ambient, float level);
	void		Draw();

	BOOL		SetLevel(float level);
	float		RetLevel();

	void		SetEnable(BOOL bEnable);
	BOOL		RetEnable();

protected:
	BOOL		EventFrame(const Event &event);
	void		AdjustLevel(D3DVECTOR &pos, D3DVECTOR &eye, float deep, FPOINT &uv1, FPOINT &uv2);
	BOOL		CreateLine(int x, int y, int len);

protected:
	CInstanceManager*	m_iMan;
	CD3DEngine*			m_engine;
	CTerrain*			m_terrain;

	char			m_filename[100];
	float			m_level;		// niveau global
	FPOINT			m_speed;		// vitesse d'avance (vent)
	D3DCOLORVALUE	m_diffuse;		// couleur diffuse
	D3DCOLORVALUE	m_ambient;		// couleur ambiante
	float			m_time;
	float			m_lastTest;

	D3DVECTOR		m_wind;			// vitesse du vent
	int				m_nbTiles;		// nb de tuiles
	float			m_dimTile;		// taille d'une tuile

	int				m_lineUsed;
	CloudLine		m_line[MAXCLOUDLINE];

	BOOL			m_bEnable;
};


#endif //_CLOUD_H_
