// modfile.h

#ifndef _MODFILE_H_
#define	_MODFILE_H_


class CInstanceManager;
class CD3DEngine;



typedef struct
{
	char			bUsed;		// TRUE -> utilisé
	char			bSelect;	// TRUE -> sélectionné
	D3DVERTEX2		p1;
	D3DVERTEX2		p2;
	D3DVERTEX2		p3;
	D3DMATERIAL7	material;
	char			texName[20];
	float			min;
	float			max;
	long			state;
	short			texNum2;
	short			reserve2;
	short			reserve3;
	short			reserve4;
}
ModelTriangle;		// longueur = 208 bytes



class CModFile
{
public:
	CModFile(CInstanceManager* iMan);
	~CModFile();

	BOOL			ReadDXF(char *filename, float min, float max);
	BOOL			AddModel(char *filename, int first, BOOL bEdit=FALSE, BOOL bMeta=TRUE);
	BOOL			ReadModel(char *filename, BOOL bEdit=FALSE, BOOL bMeta=TRUE);
	BOOL			WriteModel(char *filename);

	BOOL			CreateEngineObject(int objRank, int addState=0);
	void			Mirror();
	void			Rotate(float angle);
	void			Translate(const D3DVECTOR &dist);
	void			TerrainNormalAdjust();
	void			TerrainNormalShadow(const D3DVECTOR &pos, float factor);
	void			TerrainRandomize(float rv, float rh);
	void			TerrainTexture(char *name, int rank);

	void			SetTriangleUsed(int total);
	int				RetTriangleUsed();
	int				RetTriangleMax();
	ModelTriangle*	RetTriangleList();

	float			RetHeight(D3DVECTOR pos);

protected:
	BOOL			CreateTriangle(D3DVECTOR p1, D3DVECTOR p2, D3DVECTOR p3, float min, float max);

protected:
	CInstanceManager* m_iMan;
	CD3DEngine*		m_engine;

	ModelTriangle*	m_triangleTable;
	int				m_triangleUsed;
};


#endif //_MODFILE_H_
