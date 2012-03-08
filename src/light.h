// light.h

#ifndef _LIGHT_H_
#define	_LIGHT_H_



class CInstanceManager;
class CD3DEngine;


#define D3DMAXLIGHT		100


typedef struct
{
	float	starting;
	float	ending;
	float	current;
	float	progress;
	float	speed;
}
LightProg;


typedef struct
{
	char			bUsed;			// TRUE -> lumière existe
	char			bEnable;		// TRUE -> lumière allumée

	D3DTypeObj		incluType;		// type de tous les objets inclus
	D3DTypeObj		excluType;		// type de tous les objets exclus

	D3DLIGHT7		light;			// configuration de la lumière

	LightProg		intensity;		// intensité (0..1)
	LightProg		colorRed;
	LightProg		colorGreen;
	LightProg		colorBlue;
}
Light;



class CLight
{
public:
	CLight(CInstanceManager *iMan, CD3DEngine* engine);
	~CLight();

	void			SetD3DDevice(LPDIRECT3DDEVICE7 device);

	void			FlushLight();
	int				CreateLight();
	BOOL			DeleteLight(int lightRank);
	BOOL			SetLight(int lightRank, const D3DLIGHT7 &light);
	BOOL			GetLight(int lightRank, D3DLIGHT7 &light);
	BOOL			LightEnable(int lightRank, BOOL bEnable);

	BOOL			SetLightIncluType(int lightRank, D3DTypeObj type);
	BOOL			SetLightExcluType(int lightRank, D3DTypeObj type);

	BOOL			SetLightPos(int lightRank, D3DVECTOR pos);
	D3DVECTOR		RetLightPos(int lightRank);

	BOOL			SetLightDir(int lightRank, D3DVECTOR dir);
	D3DVECTOR		RetLightDir(int lightRank);

	BOOL			SetLightIntensitySpeed(int lightRank, float speed);
	BOOL			SetLightIntensity(int lightRank, float value);
	float			RetLightIntensity(int lightRank);
	void			AdaptLightColor(D3DCOLORVALUE color, float factor);

	BOOL			SetLightColorSpeed(int lightRank, float speed);
	BOOL			SetLightColor(int lightRank, D3DCOLORVALUE color);
	D3DCOLORVALUE	RetLightColor(int lightRank);

	void			FrameLight(float rTime);
	void			LightUpdate();
	void			LightUpdate(D3DTypeObj type);

protected:

protected:
	CInstanceManager* m_iMan;
	CD3DEngine*		  m_engine;
	LPDIRECT3DDEVICE7 m_pD3DDevice;

	float			m_time;
	int				m_lightUsed;
	Light*			m_lightTable;
};


#endif //_LIGHT_H_
