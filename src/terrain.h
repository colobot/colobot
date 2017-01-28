// terrain.h

#ifndef _TERRAIN_H_
#define	_TERRAIN_H_


class CInstanceManager;
class CD3DEngine;
class CModFile;
class CParticule;

enum ParticuleType;



enum TerrainRes
{
	TR_SOLID0	= 0,		// sol de niveau 0
	TR_SOLID1	= 1,
	TR_SOLID2	= 2,
	TR_SOLID3	= 3,
	TR_SOLID4	= 4,
	TR_SOLID5	= 5,
	TR_SOLID6	= 6,
	TR_SOLID7	= 7,
	TR_BOX		= 20,		// sol bouché par une caisse
	TR_LIFT		= 21,		// arrivée de blupi
	TR_HOLE		= -1,		// trou de niveau -8
	TR_SPACE	= -2,		// trou infini
};


enum LockZone
{
	LZ_FREE		= 0,		// zone libre
	LZ_MAX1X	= 1,		// zone libre (un passage fermé)
	LZ_TUNNELh	= 2,		// zone tunnel -
	LZ_TUNNELv	= 3,		// zone tunnel |
	LZ_FIX		= 10,		// zone fixe
	LZ_FIXo		= 11,		// zone fixe ronde
	LZ_MINE		= 12,		// zone avec mine
	LZ_FIOLE	= 13,		// zone avec fiole
	LZ_GLASS	= 14,		// zone avec vitre
	LZ_PIPE		= 15,		// zone avec tuyau
	LZ_BLUPI	= 20,		// zone occupée par blupi
	LZ_BOT		= 21,		// zone occupée par un robot
	LZ_BOX		= 22,		// zone occupée par une caisse
	LZ_BOXo		= 23,		// zone occupée par une caisse ronde
};


#define MAXMATTERRAIN		100

typedef struct
{
	short		id;
	char		texName[20];
	float		u,v;
	float		hardness;
	char		mat[4];		// up, right, down, left
}
TerrainMaterial;

typedef struct
{
	short		id;
	char		mat[4];		// up, right, down, left
}
DotLevel;


#define MAXSIGNMARK		50

typedef struct
{
	BOOL			bUsed;
	ParticuleType	type;
	D3DVECTOR		pos;
	float			angle;
	int				channel;
}
SignMark;




class CTerrain
{
public:
	CTerrain(CInstanceManager* iMan);
	~CTerrain();

	BOOL		Generate(int nbTiles, float dimTile);
	BOOL		ResInit(BOOL bEmpty);
	BOOL		InitTextures(char *texName1, char *texName2, int *idWall1, int *idWall2, int *idWall3, int *idFlat, int *idHole, int *idGround, int depth, float slope, int model, float rv, float rh, float bv, float bh, BOOL bSmooth);
	void		SetHardness(float *hardness);
	BOOL		CreateObjects();
	BOOL		DeleteObjects();

	int			RetObjRank();
	int			RetModel();

	int			LenResource();
	void		GetResource(char *buffer);
	void		PutResource(char *buffer);
	void		RestoreResourceCopy();
	TerrainRes	RetResource(int x, int y, BOOL bCopy=FALSE);
	TerrainRes	RetResource(const D3DVECTOR &p, BOOL bCopy=FALSE);
	BOOL		SetResource(int x, int y, TerrainRes res, BOOL bCopy=FALSE);
	BOOL		SetResource(const D3DVECTOR &p, TerrainRes res, BOOL bCopy=FALSE);
	BOOL		IsSolid(TerrainRes res);
	BOOL		IsSolid(int x, int y);
	BOOL		IsSolid(const D3DVECTOR &p);

	float		RetHardness(int x, int y);
	float		RetHardness(const D3DVECTOR &p);

	float		RetShadows(int x, int y);
	float		RetShadows(const D3DVECTOR &p);

	int			LenLockZone();
	void		GetLockZone(char *buffer);
	void		PutLockZone(char *buffer);
	void		SetLockZone(int x, int y, LockZone type, BOOL bInit=FALSE);
	void		SetLockZone(const D3DVECTOR &pos, LockZone type, BOOL bInit=FALSE);
	LockZone	RetLockZone(int x, int y, BOOL bInit=FALSE);
	LockZone	RetLockZone(const D3DVECTOR &pos, BOOL bInit=FALSE);
	BOOL		IsLockZone(int x, int y);
	BOOL		IsLockZone(const D3DVECTOR &pos);
	BOOL		IsLockZoneSquare(int x, int y);
	BOOL		IsLockZoneSquare(const D3DVECTOR &pos);

	void		SignMarkFlush();
	BOOL		SignMarkCreate(D3DVECTOR pos, float angle, ParticuleType type);
	BOOL		SignMarkDelete(D3DVECTOR pos);
	void		SignMarkShow(BOOL bShow);
	BOOL		SignMarkGet(D3DVECTOR pos, float &angle, ParticuleType &type);
	BOOL		SignMarkGet(int i, D3DVECTOR &pos, float &angle, ParticuleType &type);

	void		SetDebugLockZone(BOOL bShow);
	BOOL		RetDebugLockZone();

	void		SetWind(D3DVECTOR speed);
	D3DVECTOR	RetWind();

	float		RetFineSlope(const D3DVECTOR &pos);
	float		RetCoarseSlope(const D3DVECTOR &pos);
	BOOL		GetNormal(D3DVECTOR &n, const D3DVECTOR &p);
	float		RetFloorLevel(const D3DVECTOR &p);
	float		RetFloorHeight(const D3DVECTOR &p);
	BOOL		MoveOnFloor(D3DVECTOR &p);
	BOOL		ValidPosition(D3DVECTOR &p, float marging);
	void		LimitPos(D3DVECTOR &pos);
	BOOL		GroundDetect(FPOINT mouse, D3DVECTOR &pos);

	int			RetNbTiles();
	float		RetDimTile();
	float		RetDim();
	int			RetGeneration();

protected:
	BOOL		InitShadows();
	void		AdapteTileVertex(D3DVERTEX2 &v);
	void		CreateTile(int objRank, int x, int y, float level, float tu, float tv);
	void		CreateWall(int objRank, int x, int y, float level, float tu, float tv, const D3DVECTOR &n, float dx0, float dz0, float dx2, float dz2);
	void		CreateBorder(CModFile *pModFile, int objRank, float angle, int x, int y, char *type);
	void		ComputeUV(int *id, float &tu, float &tv);
	float		RetShadowPound(float x, float y);

protected:
	CInstanceManager* m_iMan;
	CD3DEngine*		m_engine;
	CParticule*		m_particule;

	int				m_nbTiles;		// nb de tuiles
	int				m_nbTiles2;		// nb de tuiles /2
	float			m_dimTile;		// dimensions d'une tuile
	int				m_depth;
	float			m_slope;
	int				m_model;
	float			m_modelRv;
	float			m_modelRh;
	float			m_baseRv;
	float			m_baseRh;
	BOOL			m_bSmooth;

	char			m_texName1[50];	// texture principale
	char			m_texName2[50];	// texture secondaire
	int				m_idWall1[50];	// id parois verticales
	int				m_idWall2[50];	// id parois verticales
	int				m_idWall3[50];	// id parois verticales
	int				m_idFlat[50];	// id sol au niveau 0
	int				m_idHole[50];	// id sol au niveau -8
	int				m_idGround[50];	// id sol terrain 3D
	float			m_hardness[8];	// duretés
	char*			m_resources;	// nature du terrain
	char*			m_shadows;		// ombres pour l'eau
	char*			m_lockZone;		// zones bloquées
	BOOL			m_lockZoneDebug;
	int				m_objRank;		// rang de l'objet terrain
	int				m_generation;
	SignMark		m_signMark[MAXSIGNMARK];

	D3DVECTOR		m_wind;			// vitesse du vent
};


#endif //_TERRAIN_H_
