// D3DEngine.cpp

#define STRICT
#define D3D_OVERLOADS

#include <stdio.h>
#include <math.h>

#include "struct.h"
#include "D3DApp.h"
#include "D3DTextr.h"
#include "D3DUtil.h"
#include "D3DMath.h"
#include "D3DEngine.h"
#include "language.h"
#include "iman.h"
#include "event.h"
#include "profile.h"
#include "math3d.h"
#include "object.h"
#include "interface.h"
#include "light.h"
#include "text.h"
#include "particule.h"
#include "terrain.h"
#include "water.h"
#include "cloud.h"
#include "blitz.h"
#include "planet.h"
#include "sound.h"



#define SIZEBLOC_TEXTURE	50
#define SIZEBLOC_TRANSFORM	100
#define SIZEBLOC_MINMAX		5
#define SIZEBLOC_LIGHT		10
#define SIZEBLOC_MATERIAL	100
#define SIZEBLOC_TRIANGLE	200



#if 0
static int debug_blend1 = 1;
static int debug_blend2 = 3;
static int debug_blend3 = 8;
static int debug_blend4 = 0;

static int table_blend[13] =
{
	D3DBLEND_ZERO,              // 0
	D3DBLEND_ONE,               // 1
	D3DBLEND_SRCCOLOR,          // 2
	D3DBLEND_INVSRCCOLOR,       // 3
	D3DBLEND_SRCALPHA,          // 4
	D3DBLEND_INVSRCALPHA,       // 5
	D3DBLEND_DESTALPHA,         // 6
	D3DBLEND_INVDESTALPHA,      // 7
	D3DBLEND_DESTCOLOR,         // 8
	D3DBLEND_INVDESTCOLOR,      // 9
	D3DBLEND_SRCALPHASAT,       // 10
	D3DBLEND_BOTHSRCALPHA,      // 11
	D3DBLEND_BOTHINVSRCALPHA,   // 12
};
#endif

static int s_resol = 0;



// Converts a FLOAT to a DWORD for use in SetRenderState() calls.

inline DWORD F2DW( FLOAT f )
{
	return *((DWORD*)&f);
}




// Application constructor. Sets attributes for the app.

CD3DEngine::CD3DEngine(CInstanceManager *iMan, CD3DApplication *app)
{
	int		i;

	m_iMan = iMan;
	m_iMan->AddInstance(CLASS_ENGINE, this);
	m_app = app;

	m_light      = new CLight(m_iMan, this);
	m_text       = new CText(m_iMan, this);
	m_particule  = new CParticule(m_iMan, this);
	m_water      = new CWater(m_iMan, this);
	m_cloud      = new CCloud(m_iMan, this);
	m_blitz      = new CBlitz(m_iMan, this);
	m_planet     = new CPlanet(m_iMan, this);
	m_pD3DDevice = 0;
	m_sound      = 0;
	m_terrain    = 0;

	m_dim.x = 640;
	m_dim.y = 480;
	m_lastDim = m_dim;
	m_focus = 0.75f;
	m_baseTime = 0;
	m_lastTime = 0;
	m_absTime = 0.0f;
	m_rankView        = 0;
	m_ambiantColor[0] = 0x80808080;
	m_ambiantColor[1] = 0x80808080;
	m_fogColor[0]     = 0xffffffff;  // white
	m_fogColor[1]     = 0xffffffff;  // white
	m_deepView[0]     = 1000.0f;
	m_deepView[1]     = 1000.0f;
	m_fogStart[0]     = 0.75f;
	m_fogStart[1]     = 0.75f;
	m_waterAddColor.r = 0.0f;
	m_waterAddColor.g = 0.0f;
	m_waterAddColor.b = 0.0f;
	m_waterAddColor.a = 0.0f;
	m_bPause          = FALSE;
	m_bRender         = TRUE;
	m_bMovieLock      = FALSE;
	m_bGroundSpot     = TRUE;
	m_speed           = 1.0f;
	m_secondTexNum    = 0;
	m_eyeDirH         = 0.0f;
	m_eyeDirV         = 0.0f;
	m_backgroundName[0] = 0;  // pas d'image de fond
	m_backgroundColorUp   = 0;
	m_backgroundColorDown = 0;
	m_backgroundSunFlash = 0.5f;
	m_bBackgroundFull = FALSE;
	m_bBackgroundQuarter = FALSE;
	m_bBackgroundPanel = FALSE;
	m_bOverFront = TRUE;
	m_overColor = 0;
	m_overMode  = D3DSTATETCb;
	m_lensFlareTotal = 0;
	m_hiliteRank[0] = -1;  // liste vide
	m_bHilite = FALSE;
	m_bSelect = FALSE;
	m_mousePos = FPOINT(0.5f, 0.5f);
	m_mouseType = D3DMOUSENORM;
	m_bMouseHide = FALSE;
	m_imageSurface = 0;
	m_imageCopy = 0;
	m_eyePt    = D3DVECTOR(0.0f, 0.0f, 0.0f);
	m_lookatPt = D3DVECTOR(0.0f, 0.0f, 1.0f);
	m_bDrawWorld = TRUE;
	m_bDrawFront = FALSE;
	m_limitLOD[0] = 100.0f;
	m_limitLOD[1] = 200.0f;
	m_clippingDistance = 1.0f;
	m_lastClippingDistance = m_clippingDistance;
	m_objectDetail = 1.0f;
	m_lastObjectDetail = m_objectDetail;
	m_terrainVision = 1000.0f;
	m_bBackForce = TRUE;
	m_bEditIndentMode = TRUE;
	m_editIndentValue = 4;
	m_lockZoneBuffer = 0;

	DefaultSetup();

	m_alphaMode = 1;
	if ( GetProfileInt("Engine", "AlphaMode", i) )
	{
		m_alphaMode = i;
	}

	if ( GetProfileInt("Engine", "StateColor", i) && i != -1 )
	{
		m_bForceStateColor = TRUE;
		m_bStateColor = i;
	}
	else
	{
		m_bForceStateColor = FALSE;
		m_bStateColor = FALSE;
	}
	
	m_blackSrcBlend[0]    = 0;
	m_blackDestBlend[0]   = 0;
	m_whiteSrcBlend[0]    = 0;
	m_whiteDestBlend[0]   = 0;
	m_diffuseSrcBlend[0]  = 0;
	m_diffuseDestBlend[0] = 0;
	m_alphaSrcBlend[0]    = 0;
	m_alphaDestBlend[0]   = 0;

	if ( GetProfileInt("Engine", "BlackSrcBlend",    i) )  m_blackSrcBlend[0]    = i;
	if ( GetProfileInt("Engine", "BlackDestBlend",   i) )  m_blackDestBlend[0]   = i;
	if ( GetProfileInt("Engine", "WhiteSrcBlend",    i) )  m_whiteSrcBlend[0]    = i;
	if ( GetProfileInt("Engine", "WhiteDestBlend",   i) )  m_whiteDestBlend[0]   = i;
	if ( GetProfileInt("Engine", "DiffuseSrcBlend",  i) )  m_diffuseSrcBlend[0]  = i;
	if ( GetProfileInt("Engine", "DiffuseDestBlend", i) )  m_diffuseDestBlend[0] = i;
	if ( GetProfileInt("Engine", "AlphaSrcBlend",    i) )  m_alphaSrcBlend[0]    = i;
	if ( GetProfileInt("Engine", "AlphaDestBlend",   i) )  m_alphaDestBlend[0]   = i;

	m_bUpdateGeometry = FALSE;

	for ( i=0 ; i<10 ; i++ )
	{
		m_infoText[i][0] = 0;
	}

	m_objectPointer = 0;
	MemSpace1(m_objectPointer, 0);

	m_objectParam = (D3DObject*)malloc(sizeof(D3DObject)*D3DMAXOBJECT);
	ZeroMemory(m_objectParam, sizeof(D3DObject)*D3DMAXOBJECT);
	m_objectParamTotal = 0;

	m_shadow = (D3DShadow*)malloc(sizeof(D3DShadow)*D3DMAXSHADOW);
	ZeroMemory(m_shadow, sizeof(D3DShadow)*D3DMAXSHADOW);
	m_shadowTotal = 0;

	m_groundSpot = (D3DGroundSpot*)malloc(sizeof(D3DGroundSpot)*D3DMAXGROUNDSPOT);
	ZeroMemory(m_groundSpot, sizeof(D3DGroundSpot)*D3DMAXGROUNDSPOT);

	ZeroMemory(&m_groundMark, sizeof(D3DGroundMark));

	D3DTextr_SetTexturePath("textures\\");
	FlushReplaceTex();

	timeBeginPeriod(1);  // résolution de 1ms pour timeGetTime()
}

// Application destructor. Free memory.

CD3DEngine::~CD3DEngine()
{
	D3DObjLevel1*	p1;
	D3DObjLevel2a*	p2a;
	D3DObjLevel2b*	p2b;
	D3DObjLevel3*	p3;
	D3DObjLevel4*	p4;
	D3DObjLevel5*	p5;
	D3DObjLevel6*	p6;
	int				l1, l2a, l2b, l3, l4, l5;

	p1 = m_objectPointer;
	for ( l1=0 ; l1<p1->totalUsed ; l1++ )
	{
		p2a = p1->table[l1];
		if ( p2a == 0 )  continue;
		for ( l2a=0 ; l2a<p2a->totalUsed ; l2a++ )
		{
			p2b = p2a->table[l2a];
			if ( p2b == 0 )  continue;
			for ( l2b=0 ; l2b<p2b->totalUsed ; l2b++ )
			{
				p3 = p2b->table[l2b];
				if ( p3 == 0 )  continue;
				for ( l3=0 ; l3<p3->totalUsed ; l3++ )
				{
					p4 = p3->table[l3];
					if ( p4 == 0 )  continue;
					for ( l4=0 ; l4<p4->totalUsed ; l4++ )
					{
						p5 = p4->table[l4];
						if ( p5 == 0 )  continue;
						for ( l5=0 ; l5<p5->totalUsed ; l5++ )
						{
							p6 = p5->table[l5];
							if ( p6 == 0 )  continue;
							free(p6);
						}
						free(p5);
					}
					free(p4);
				}
				free(p3);
			}
			free(p2b);
		}
		free(p2a);
	}
	free(p1);

	delete m_light;
	delete m_particule;
	delete m_water;
	delete m_cloud;
	delete m_blitz;
	delete m_planet;
}



void CD3DEngine::SetD3DDevice(LPDIRECT3DDEVICE7 device)
{
	D3DDEVICEDESC7	ddDesc;

	m_pD3DDevice = device;
	m_light->SetD3DDevice(device);
	m_text->SetD3DDevice(device);
	m_particule->SetD3DDevice(device);

	if ( !m_bForceStateColor )
	{
		m_pD3DDevice->GetCaps(&ddDesc);
		if( ddDesc.dpcTriCaps.dwTextureBlendCaps & D3DPTBLENDCAPS_ADD )
		{
			m_bStateColor = TRUE;
		}
		else
		{
			m_bStateColor = FALSE;
		}
	}

	m_blackSrcBlend[1]    = D3DBLEND_ONE;          // = 2
	m_blackDestBlend[1]   = D3DBLEND_INVSRCCOLOR;  // = 4
	m_whiteSrcBlend[1]    = D3DBLEND_DESTCOLOR;    // = 9
	m_whiteDestBlend[1]   = D3DBLEND_ZERO;         // = 1
	m_diffuseSrcBlend[1]  = D3DBLEND_SRCALPHA;     // = 5
	m_diffuseDestBlend[1] = D3DBLEND_DESTALPHA;    // = 7
	m_alphaSrcBlend[1]    = D3DBLEND_ONE;          // = 2
	m_alphaDestBlend[1]   = D3DBLEND_INVSRCCOLOR;  // = 4

//?	if ( !m_bStateColor )  m_whiteDestBlend[1] = D3DBLEND_INVSRCALPHA;  // = 6

	if ( m_blackSrcBlend[0]    )  m_blackSrcBlend[1]    = m_blackSrcBlend[0];
	if ( m_blackDestBlend[0]   )  m_blackDestBlend[1]   = m_blackDestBlend[0];
	if ( m_whiteSrcBlend[0]    )  m_whiteSrcBlend[1]    = m_whiteSrcBlend[0];
	if ( m_whiteDestBlend[0]   )  m_whiteDestBlend[1]   = m_whiteDestBlend[0];
	if ( m_diffuseSrcBlend[0]  )  m_diffuseSrcBlend[1]  = m_diffuseSrcBlend[0];
	if ( m_diffuseDestBlend[0] )  m_diffuseDestBlend[1] = m_diffuseDestBlend[0];
	if ( m_alphaSrcBlend[0]    )  m_alphaSrcBlend[1]    = m_alphaSrcBlend[0];
	if ( m_alphaDestBlend[0]   )  m_alphaDestBlend[1]   = m_alphaDestBlend[0];

#if 0
	DWORD	pass;
	m_pD3DDevice->ValidateDevice(&pass);
	char s[100];
	sprintf(s, "NbPass=%d", pass);
	SetInfoText(3, s);
#endif
}

LPDIRECT3DDEVICE7 CD3DEngine::RetD3DDevice()
{
	return m_pD3DDevice;
}


// Donne le pointeur au terrain existant.

void CD3DEngine::SetTerrain(CTerrain* terrain)
{
	m_terrain = terrain;
}


// Sauvegarde l'état du moteur graphique dans blupimania.ini.

BOOL CD3DEngine::WriteProfile()
{
	SetProfileInt("Engine", "AlphaMode", m_alphaMode);

	if ( m_bForceStateColor )
	{
		SetProfileInt("Engine", "StateColor", m_bStateColor);
	}
	else
	{
		SetProfileInt("Engine", "StateColor", -1);
	}

	SetProfileInt("Engine", "BlackSrcBlend",    m_blackSrcBlend[0]);
	SetProfileInt("Engine", "BlackDestBlend",   m_blackDestBlend[0]);
	SetProfileInt("Engine", "WhiteSrcBlend",    m_whiteSrcBlend[0]);
	SetProfileInt("Engine", "WhiteDestBlend",   m_whiteDestBlend[0]);
	SetProfileInt("Engine", "DiffuseSrcBlend",  m_diffuseSrcBlend[0]);
	SetProfileInt("Engine", "DiffuseDestBlend", m_diffuseDestBlend[0]);
	SetProfileInt("Engine", "AlphaSrcBlend",    m_alphaSrcBlend[0]);
	SetProfileInt("Engine", "AlphaDestBlend",   m_alphaDestBlend[0]);

	return TRUE;
}


// Setup the app so it can support single-stepping.

void CD3DEngine::TimeInit()
{
    m_baseTime = timeGetTime();
	m_lastTime = 0;
	m_absTime = 0.0f;
}

void CD3DEngine::TimeEnterGel()
{
	m_stopTime = timeGetTime();
}

void CD3DEngine::TimeExitGel()
{
	m_baseTime += timeGetTime() - m_stopTime;
}

float CD3DEngine::TimeGet()
{
	float	aTime, rTime;

	aTime = (timeGetTime()-m_baseTime)*0.001f;  // en ms
	rTime = (aTime - m_lastTime)*m_speed;
	m_absTime += rTime;
	m_lastTime = aTime;

	return rTime;
}


void CD3DEngine::SetPause(BOOL bPause)
{
	m_bPause = bPause;
}

BOOL CD3DEngine::RetPause()
{
	return m_bPause;
}


void CD3DEngine::SetMovieLock(BOOL bLock)
{
	m_bMovieLock = bLock;
}

BOOL CD3DEngine::RetMovieLock()
{
	return m_bMovieLock;
}


void CD3DEngine::SetShowStat(BOOL bShow)
{
	m_app->SetShowStat(bShow);
}

BOOL CD3DEngine::RetShowStat()
{
	return m_app->RetShowStat();
}


void CD3DEngine::SetRenderEnable(BOOL bEnable)
{
	m_bRender = bEnable;
}


// Prépare une structure D3DObjLevel6 pour pouvoir ajouter
// qq éléments D3DVERTEX2.

void CD3DEngine::MemSpace6(D3DObjLevel6 *&p, int nb)
{
	D3DObjLevel6*	pp;
	int				total, size;

	if ( p == 0 )
	{
		total = SIZEBLOC_TRIANGLE+nb;
		size = sizeof(D3DObjLevel6)+sizeof(D3DVERTEX2)*(total-1);
		p = (D3DObjLevel6*)malloc(size);
		ZeroMemory(p, size);
		p->totalPossible = total;
		return;
	}

	if ( p->totalUsed+nb > p->totalPossible )
	{
		total = p->totalPossible+SIZEBLOC_TRIANGLE+nb;
		size = sizeof(D3DObjLevel6)+sizeof(D3DVERTEX2)*(total-1);
		pp = (D3DObjLevel6*)malloc(size);
		ZeroMemory(pp, size);
		CopyMemory(pp, p, sizeof(D3DObjLevel6)+sizeof(D3DVERTEX2)*(p->totalPossible-1));
		pp->totalPossible = total;
		free(p);
		p = pp;
	}
}

// Prépare une structure D3DObjLevel5 pour pouvoir ajouter
// qq éléments D3DObjLevel6.

void CD3DEngine::MemSpace5(D3DObjLevel5 *&p, int nb)
{
	D3DObjLevel5*	pp;
	int				total, size;

	if ( p == 0 )
	{
		total = SIZEBLOC_MATERIAL+nb;
		size = sizeof(D3DObjLevel5)+sizeof(D3DObjLevel6*)*(total-1);
		p = (D3DObjLevel5*)malloc(size);
		ZeroMemory(p, size);
		p->totalPossible = total;
		return;
	}

	if ( p->totalUsed+nb > p->totalPossible )
	{
		total = p->totalPossible+SIZEBLOC_MATERIAL+nb;
		size = sizeof(D3DObjLevel5)+sizeof(D3DObjLevel6*)*(total-1);
		pp = (D3DObjLevel5*)malloc(size);
		ZeroMemory(pp, size);
		CopyMemory(pp, p, sizeof(D3DObjLevel5)+sizeof(D3DObjLevel6*)*(p->totalPossible-1));
		pp->totalPossible = total;
		free(p);
		p = pp;
	}
}

// Prépare une structure D3DObjLevel4 pour pouvoir ajouter
// qq éléments D3DObjLevel5.

void CD3DEngine::MemSpace4(D3DObjLevel4 *&p, int nb)
{
	D3DObjLevel4*	pp;
	int				total, size;

	if ( p == 0 )
	{
		total = SIZEBLOC_LIGHT+nb;
		size = sizeof(D3DObjLevel4)+sizeof(D3DObjLevel5*)*(total-1);
		p = (D3DObjLevel4*)malloc(size);
		ZeroMemory(p, size);
		p->totalPossible = total;
		return;
	}

	if ( p->totalUsed+nb > p->totalPossible )
	{
		total = p->totalPossible+SIZEBLOC_LIGHT+nb;
		size = sizeof(D3DObjLevel4)+sizeof(D3DObjLevel5*)*(total-1);
		pp = (D3DObjLevel4*)malloc(size);
		ZeroMemory(pp, size);
		CopyMemory(pp, p, sizeof(D3DObjLevel4)+sizeof(D3DObjLevel5*)*(p->totalPossible-1));
		pp->totalPossible = total;
		free(p);
		p = pp;
	}
}

// Prépare une structure D3DObjLevel3 pour pouvoir ajouter
// qq éléments D3DObjLevel4.

void CD3DEngine::MemSpace3(D3DObjLevel3 *&p, int nb)
{
	D3DObjLevel3*	pp;
	int				total, size;

	if ( p == 0 )
	{
		total = SIZEBLOC_MINMAX+nb;
		size = sizeof(D3DObjLevel3)+sizeof(D3DObjLevel4*)*(total-1);
		p = (D3DObjLevel3*)malloc(size);
		ZeroMemory(p, size);
		p->totalPossible = total;
		return;
	}

	if ( p->totalUsed+nb > p->totalPossible )
	{
		total = p->totalPossible+SIZEBLOC_MINMAX+nb;
		size = sizeof(D3DObjLevel3)+sizeof(D3DObjLevel4*)*(total-1);
		pp = (D3DObjLevel3*)malloc(size);
		ZeroMemory(pp, size);
		CopyMemory(pp, p, sizeof(D3DObjLevel3)+sizeof(D3DObjLevel4*)*(p->totalPossible-1));
		pp->totalPossible = total;
		free(p);
		p = pp;
	}
}

// Prépare une structure D3DObjLevel2b pour pouvoir ajouter
// qq éléments D3DObjLevel3.

void CD3DEngine::MemSpace2b(D3DObjLevel2b *&p, int nb)
{
	D3DObjLevel2b*	pp;
	int				total, size;

	if ( p == 0 )
	{
		total = SIZEBLOC_TRANSFORM+nb;
		size = sizeof(D3DObjLevel2b)+sizeof(D3DObjLevel3*)*(total-1);
		p = (D3DObjLevel2b*)malloc(size);
		ZeroMemory(p, size);
		p->totalPossible = total;
		return;
	}

	if ( p->totalUsed+nb > p->totalPossible )
	{
		total = p->totalPossible+SIZEBLOC_TRANSFORM+nb;
		size = sizeof(D3DObjLevel2b)+sizeof(D3DObjLevel3*)*(total-1);
		pp = (D3DObjLevel2b*)malloc(size);
		ZeroMemory(pp, size);
		CopyMemory(pp, p, sizeof(D3DObjLevel2b)+sizeof(D3DObjLevel3*)*(p->totalPossible-1));
		pp->totalPossible = total;
		free(p);
		p = pp;
	}
}

// Prépare une structure D3DObjLevel2a pour pouvoir ajouter
// qq éléments D3DObjLevel2b.

void CD3DEngine::MemSpace2a(D3DObjLevel2a *&p, int nb)
{
	D3DObjLevel2a*	pp;
	int				total, size;

	if ( p == 0 )
	{
		total = SIZEBLOC_TRANSFORM+nb;
		size = sizeof(D3DObjLevel2a)+sizeof(D3DObjLevel2b*)*(total-1);
		p = (D3DObjLevel2a*)malloc(size);
		ZeroMemory(p, size);
		p->totalPossible = total;
		return;
	}

	if ( p->totalUsed+nb > p->totalPossible )
	{
		total = p->totalPossible+SIZEBLOC_TRANSFORM+nb;
		size = sizeof(D3DObjLevel2a)+sizeof(D3DObjLevel2b*)*(total-1);
		pp = (D3DObjLevel2a*)malloc(size);
		ZeroMemory(pp, size);
		CopyMemory(pp, p, sizeof(D3DObjLevel2a)+sizeof(D3DObjLevel2b*)*(p->totalPossible-1));
		pp->totalPossible = total;
		free(p);
		p = pp;
	}
}

// Prépare une structure D3DObjLevel1 pour pouvoir ajouter
// qq éléments D3DObjLevel2a.

void CD3DEngine::MemSpace1(D3DObjLevel1 *&p, int nb)
{
	D3DObjLevel1*	pp;
	int				total, size;

	if ( p == 0 )
	{
		total = SIZEBLOC_TEXTURE+nb;
		size = sizeof(D3DObjLevel1)+sizeof(D3DObjLevel2a*)*(total-1);
		p = (D3DObjLevel1*)malloc(size);
		ZeroMemory(p, size);
		p->totalPossible = total;
		return;
	}

	if ( p->totalUsed+nb > p->totalPossible )
	{
		total = p->totalPossible+SIZEBLOC_TEXTURE+nb;
		size = sizeof(D3DObjLevel1)+sizeof(D3DObjLevel2a*)*(total-1);
		pp = (D3DObjLevel1*)malloc(size);
		ZeroMemory(pp, size);
		CopyMemory(pp, p, sizeof(D3DObjLevel1)+sizeof(D3DObjLevel2a*)*(p->totalPossible-1));
		pp->totalPossible = total;
		free(p);
		p = pp;
	}
}


// Retourne le nombre d'objets qu'il est encore possible de créer.

int CD3DEngine::RetRestCreate()
{
	return D3DMAXOBJECT-m_objectParamTotal-2;
}

// Crée un nouvel objet. Retourne son rang ou -1 en cas d'erreur.

int CD3DEngine::CreateObject()
{
	D3DMATRIX	mat;
	int			i;

	for ( i=0 ; i<D3DMAXOBJECT ; i++ )
	{
		if ( m_objectParam[i].bUsed == FALSE )
		{
			ZeroMemory(&m_objectParam[i], sizeof(D3DObject));
			m_objectParam[i].bUsed = TRUE;
			m_objectParam[i].bHide = FALSE;
			m_objectParam[i].bDetect = TRUE;

			D3DUtil_SetIdentityMatrix(mat);
			SetObjectTransform(i, mat);

			m_objectParam[i].bDrawWorld = TRUE;
			m_objectParam[i].distance = 0.0f;
			m_objectParam[i].bboxMin = D3DVECTOR(0.0f, 0.0f, 0.0f);
			m_objectParam[i].bboxMax = D3DVECTOR(0.0f, 0.0f, 0.0f);
			m_objectParam[i].shadowRank = -1;

			if ( i >= m_objectParamTotal )
			{
				m_objectParamTotal = i+1;
			}
			return i;
		}
	}
	OutputDebugString("CD3DEngine::CreateObject() -> Too many object\n");
	return -1;
}


// Supprime tous les objets.

void CD3DEngine::FlushObject()
{
	D3DObjLevel1*	p1;
	D3DObjLevel2a*	p2a;
	D3DObjLevel2b*	p2b;
	D3DObjLevel3*	p3;
	D3DObjLevel4*	p4;
	D3DObjLevel5*	p5;
	D3DObjLevel6*	p6;
	int				l1, l2a, l2b, l3, l4, l5, i;

	p1 = m_objectPointer;
	for ( l1=0 ; l1<p1->totalUsed ; l1++ )
	{
		p2a = p1->table[l1];
		if ( p2a == 0 )  continue;
		for ( l2a=0 ; l2a<p2a->totalUsed ; l2a++ )
		{
			p2b = p2a->table[l2a];
			if ( p2b == 0 )  continue;
			for ( l2b=0 ; l2b<p2b->totalUsed ; l2b++ )
			{
				p3 = p2b->table[l2b];
				if ( p3 == 0 )  continue;
				for ( l3=0 ; l3<p3->totalUsed ; l3++ )
				{
					p4 = p3->table[l3];
					if ( p4 == 0 )  continue;
					for ( l4=0 ; l4<p4->totalUsed ; l4++ )
					{
						p5 = p4->table[l4];
						if ( p5 == 0 )  continue;
						for ( l5=0 ; l5<p5->totalUsed ; l5++ )
						{
							p6 = p5->table[l5];
							if ( p6 == 0 )  continue;
							free(p6);
						}
						free(p5);
					}
					free(p4);
				}
				free(p3);
			}
			free(p2b);
		}
		free(p2a);
		p1->table[l1] = 0;
	}
	p1->totalUsed = 0;

	for ( i=0 ; i<D3DMAXOBJECT ; i++ )
	{
		m_objectParam[i].bUsed = FALSE;
	}
	m_objectParamTotal = 0;

	ZeroMemory(m_shadow, sizeof(D3DShadow)*D3DMAXSHADOW);
	m_shadowTotal = 0;

	GroundSpotFlush();
}

// Détruit un objet existant.

BOOL CD3DEngine::DeleteObject(int objRank)
{
	D3DObjLevel1*	p1;
	D3DObjLevel2a*	p2a;
	D3DObjLevel2b*	p2b;
	D3DObjLevel3*	p3;
	D3DObjLevel4*	p4;
	D3DObjLevel5*	p5;
	D3DObjLevel6*	p6;
	int				l1, l2a, l2b, l3, l4, l5, i;

	p1 = m_objectPointer;
	for ( l1=0 ; l1<p1->totalUsed ; l1++ )
	{
		p2a = p1->table[l1];
		if ( p2a == 0 )  continue;
		for ( l2a=0 ; l2a<p2a->totalUsed ; l2a++ )
		{
			p2b = p2a->table[l2a];
			if ( p2b == 0 )  continue;
			for ( l2b=0 ; l2b<p2b->totalUsed ; l2b++ )
			{
				p3 = p2b->table[l2b];
				if ( p3 == 0 )  continue;
				if ( p3->objRank != objRank )  continue;
				for ( l3=0 ; l3<p3->totalUsed ; l3++ )
				{
					p4 = p3->table[l3];
					if ( p4 == 0 )  continue;
					for ( l4=0 ; l4<p4->totalUsed ; l4++ )
					{
						p5 = p4->table[l4];
						if ( p5 == 0 )  continue;
						for ( l5=0 ; l5<p5->totalUsed ; l5++ )
						{
							p6 = p5->table[l5];
							if ( p6 == 0 )  continue;
							free(p6);
						}
						free(p5);
					}
					free(p4);
				}
				free(p3);
				p2b->table[l2b] = 0;
			}
		}
	}

	ShadowDelete(objRank);  // supprime l'ombre

	m_objectParam[objRank].bUsed = FALSE;

	m_objectParamTotal = 0;
	for ( i=0 ; i<D3DMAXOBJECT ; i++ )
	{
		if ( m_objectParam[i].bUsed )
		{
			m_objectParamTotal = i+1;
		}
	}

	return TRUE;
}


// Indique si un objet doit être dessiné par dessous l'interface.

BOOL CD3DEngine::SetDrawWorld(int objRank, BOOL bDraw)
{
	if ( objRank < 0 || objRank >= D3DMAXOBJECT )  return FALSE;

	m_objectParam[objRank].bDrawWorld = bDraw;
	return TRUE;
}

// Indique si un objet doit être dessiné par dessus l'interface.

BOOL CD3DEngine::SetDrawFront(int objRank, BOOL bDraw)
{
	if ( objRank < 0 || objRank >= D3DMAXOBJECT )  return FALSE;

	m_objectParam[objRank].bDrawFront = bDraw;
	return TRUE;
}


// Prépare le niveau 1 pour ajouter un triangle.

D3DObjLevel2a* CD3DEngine::AddLevel1(D3DObjLevel1 *&p1, short type)
{
	D3DObjLevel2a*	p2a;
	int				l1;

	for ( l1=0 ; l1<p1->totalUsed ; l1++ )
	{
		p2a = p1->table[l1];
		if ( p2a == 0 )  continue;
		if ( p2a->type == type )
		{
			MemSpace2a(p1->table[l1], 1);
			return p1->table[l1];
		}
	}

	MemSpace1(p1, 1);
	l1 = p1->totalUsed++;
	p1->table[l1] = 0;

	MemSpace2a(p1->table[l1], 1);
	p1->table[l1]->type = type;
	return p1->table[l1];
}

// Prépare le niveau 2a pour ajouter un triangle.

D3DObjLevel2b* CD3DEngine::AddLevel2a(D3DObjLevel2a *&p2a, char* texName1, char* texName2)
{
	D3DObjLevel2b*	p2b;
	int				l2a;

	for ( l2a=0 ; l2a<p2a->totalUsed ; l2a++ )
	{
		p2b = p2a->table[l2a];
		if ( p2b == 0 )  continue;
		if ( strcmp(p2b->texName1, texName1) == 0 &&
			 strcmp(p2b->texName2, texName2) == 0 )
		{
			MemSpace2b(p2a->table[l2a], 1);
			return p2a->table[l2a];
		}
	}

	MemSpace2a(p2a, 1);
	l2a = p2a->totalUsed++;
	p2a->table[l2a] = 0;

	MemSpace2b(p2a->table[l2a], 1);
	strcpy(p2a->table[l2a]->texName1, texName1);
	strcpy(p2a->table[l2a]->texName2, texName2);
	return p2a->table[l2a];
}

// Prépare le niveau 2b pour ajouter un triangle.

D3DObjLevel3* CD3DEngine::AddLevel2b(D3DObjLevel2b *&p2b, int objRank)
{
	D3DObjLevel3*	p3;
	int				l2b;

	for ( l2b=0 ; l2b<p2b->totalUsed ; l2b++ )
	{
		p3 = p2b->table[l2b];
		if ( p3 == 0 )  continue;
		if ( p3->objRank == objRank )
		{
			MemSpace3(p2b->table[l2b], 1);
			return p2b->table[l2b];
		}
	}

	MemSpace2b(p2b, 1);
	l2b = p2b->totalUsed++;
	p2b->table[l2b] = 0;

	MemSpace3(p2b->table[l2b], 1);
	p2b->table[l2b]->objRank = objRank;
	return p2b->table[l2b];
}

// Prépare le niveau 3 pour ajouter un triangle.

D3DObjLevel4* CD3DEngine::AddLevel3(D3DObjLevel3 *&p3, float min, float max)
{
	D3DObjLevel4*	p4;
	int				l3;

	for ( l3=0 ; l3<p3->totalUsed ; l3++ )
	{
		p4 = p3->table[l3];
		if ( p4 == 0 )  continue;
		if ( p4->min == min && p4->max == max )
		{
			MemSpace4(p3->table[l3], 1);
			return p3->table[l3];
		}
	}

	MemSpace3(p3, 1);
	l3 = p3->totalUsed++;
	p3->table[l3] = 0;

	MemSpace4(p3->table[l3], 1);
	p3->table[l3]->min = min;
	p3->table[l3]->max = max;
	return p3->table[l3];
}

// Prépare le niveau 4 pour ajouter un triangle.

D3DObjLevel5* CD3DEngine::AddLevel4(D3DObjLevel4 *&p4, int reserve)
{
	D3DObjLevel5*	p5;
	int				l4;

	for ( l4=0 ; l4<p4->totalUsed ; l4++ )
	{
		p5 = p4->table[l4];
		if ( p5 == 0 )  continue;
		if ( p5->reserve == reserve )
		{
			MemSpace5(p4->table[l4], 1);
			return p4->table[l4];
		}
	}

	MemSpace4(p4, 1);
	l4 = p4->totalUsed++;
	p4->table[l4] = 0;

	MemSpace5(p4->table[l4], 1);
	p4->table[l4]->reserve = reserve;
	return p4->table[l4];
}

// Prépare le niveau 5 pour ajouter des vertex.

D3DObjLevel6* CD3DEngine::AddLevel5(D3DObjLevel5 *&p5, D3DTypeTri type,
									const D3DMATERIAL7 &mat, int state,
									int nb)
{
	D3DObjLevel6*	p6;
	int				l5;

	if ( type == D3DTYPE6T )
	{
		for ( l5=0 ; l5<p5->totalUsed ; l5++ )
		{
			p6 = p5->table[l5];
			if ( p6 == 0 )  continue;
			if ( p6->type == type &&
				 memcmp(&p6->material, &mat, sizeof(D3DMATERIAL7)) == 0 &&
				 p6->state == state )
			{
				MemSpace6(p5->table[l5], nb);
				return p5->table[l5];
			}
		}
	}

	MemSpace5(p5, 1);
	l5 = p5->totalUsed++;
	p5->table[l5] = 0;

	MemSpace6(p5->table[l5], nb);
	p5->table[l5]->type     = type;
	p5->table[l5]->material = mat;
	p5->table[l5]->state    = state;
	return p5->table[l5];
}

// Ajoute un ou plusieurs triangles à un objet existant.
// Le nombre doit être multiple de 3.

BOOL CD3DEngine::AddTriangle(int objRank, D3DVERTEX2* vertex, int nb,
							 const D3DMATERIAL7 &mat, int state,
							 char* texName1, char* texName2,
							 float min, float max, BOOL bGlobalUpdate)
{
	D3DObjLevel2a*	p2a;
	D3DObjLevel2b*	p2b;
	D3DObjLevel3*	p3;
	D3DObjLevel4*	p4;
	D3DObjLevel5*	p5;
	D3DObjLevel6*	p6;
	int				i;
	short			type;

	m_lastDim = m_dim;
	m_lastObjectDetail = m_objectDetail;
	m_lastClippingDistance = m_clippingDistance;

	type = 0;
	if ( state & (D3DSTATETTb|D3DSTATETTw) )  type = 1;

	p2a = AddLevel1 (m_objectPointer, type);
	p2b = AddLevel2a(p2a, texName1, texName2);
	p3  = AddLevel2b(p2b, objRank);
	p4  = AddLevel3 (p3,  min, max);
	p5  = AddLevel4 (p4,  0);
	p6  = AddLevel5 (p5,  D3DTYPE6T, mat, state, nb);  // place pour nb vertex

	CopyMemory(&p6->vertex[p6->totalUsed], vertex, sizeof(D3DVERTEX2)*nb);
	p6->totalUsed += nb;

	if ( bGlobalUpdate )
	{
		m_bUpdateGeometry = TRUE;
	}
	else
	{
		for ( i=0 ; i<nb ; i++ )
		{
			m_objectParam[objRank].bboxMin.x = Min(vertex[i].x, m_objectParam[objRank].bboxMin.x);
			m_objectParam[objRank].bboxMin.y = Min(vertex[i].y, m_objectParam[objRank].bboxMin.y);
			m_objectParam[objRank].bboxMin.z = Min(vertex[i].z, m_objectParam[objRank].bboxMin.z);
			m_objectParam[objRank].bboxMax.x = Max(vertex[i].x, m_objectParam[objRank].bboxMax.x);
			m_objectParam[objRank].bboxMax.y = Max(vertex[i].y, m_objectParam[objRank].bboxMax.y);
			m_objectParam[objRank].bboxMax.z = Max(vertex[i].z, m_objectParam[objRank].bboxMax.z);
		}

		m_objectParam[objRank].radius = Max(Length(m_objectParam[objRank].bboxMin),
											Length(m_objectParam[objRank].bboxMax));
	}
	m_objectParam[objRank].totalTriangle += nb/3;

	return TRUE;
}

// Ajoute une surface constituée de triangles jointifs.

BOOL CD3DEngine::AddSurface(int objRank, D3DVERTEX2* vertex, int nb,
							const D3DMATERIAL7 &mat, int state,
							char* texName1, char* texName2,
							float min, float max, BOOL bGlobalUpdate)
{
	D3DObjLevel2a*	p2a;
	D3DObjLevel2b*	p2b;
	D3DObjLevel3*	p3;
	D3DObjLevel4*	p4;
	D3DObjLevel5*	p5;
	D3DObjLevel6*	p6;
	int				i;
	short			type;

	type = 0;
	if ( state & (D3DSTATETTb|D3DSTATETTw) )  type = 1;

	p2a = AddLevel1 (m_objectPointer, type);
	p2b = AddLevel2a(p2a, texName1, texName2);
	p3  = AddLevel2b(p2b, objRank);
	p4  = AddLevel3 (p3,  min, max);
	p5  = AddLevel4 (p4,  0);
	p6  = AddLevel5 (p5,  D3DTYPE6S, mat, state, nb);  // place pour nb vertex

	CopyMemory(&p6->vertex[p6->totalUsed], vertex, sizeof(D3DVERTEX2)*nb);
	p6->totalUsed += nb;

	if ( bGlobalUpdate )
	{
		m_bUpdateGeometry = TRUE;
	}
	else
	{
		for ( i=0 ; i<nb ; i++ )
		{
			m_objectParam[objRank].bboxMin.x = Min(vertex[i].x, m_objectParam[objRank].bboxMin.x);
			m_objectParam[objRank].bboxMin.y = Min(vertex[i].y, m_objectParam[objRank].bboxMin.y);
			m_objectParam[objRank].bboxMin.z = Min(vertex[i].z, m_objectParam[objRank].bboxMin.z);
			m_objectParam[objRank].bboxMax.x = Max(vertex[i].x, m_objectParam[objRank].bboxMax.x);
			m_objectParam[objRank].bboxMax.y = Max(vertex[i].y, m_objectParam[objRank].bboxMax.y);
			m_objectParam[objRank].bboxMax.z = Max(vertex[i].z, m_objectParam[objRank].bboxMax.z);
		}

		m_objectParam[objRank].radius = Max(Length(m_objectParam[objRank].bboxMin),
											Length(m_objectParam[objRank].bboxMax));
	}
	m_objectParam[objRank].totalTriangle += nb-2;

	return TRUE;
}

// Ajoute une surface constituée de triangles jointifs.
// Le buffer n'est pas copié.

BOOL CD3DEngine::AddQuick(int objRank, D3DObjLevel6* buffer,
						  char* texName1, char* texName2,
						  float min, float max, BOOL bGlobalUpdate)
{
	D3DObjLevel2a*	p2a;
	D3DObjLevel2b*	p2b;
	D3DObjLevel3*	p3;
	D3DObjLevel4*	p4;
	D3DObjLevel5*	p5;
	int				l5, i;

	p2a = AddLevel1 (m_objectPointer, 0);
	p2b = AddLevel2a(p2a, texName1, texName2);
	p3  = AddLevel2b(p2b, objRank);
	p4  = AddLevel3 (p3,  min, max);
	p5  = AddLevel4 (p4,  0);

	MemSpace5(p5, 1);
	l5 = p5->totalUsed++;
	p5->table[l5] = buffer;

	if ( bGlobalUpdate )
	{
		m_bUpdateGeometry = TRUE;
	}
	else
	{
		for ( i=0 ; i<buffer->totalUsed ; i++ )
		{
			m_objectParam[objRank].bboxMin.x = Min(buffer->vertex[i].x, m_objectParam[objRank].bboxMin.x);
			m_objectParam[objRank].bboxMin.y = Min(buffer->vertex[i].y, m_objectParam[objRank].bboxMin.y);
			m_objectParam[objRank].bboxMin.z = Min(buffer->vertex[i].z, m_objectParam[objRank].bboxMin.z);
			m_objectParam[objRank].bboxMax.x = Max(buffer->vertex[i].x, m_objectParam[objRank].bboxMax.x);
			m_objectParam[objRank].bboxMax.y = Max(buffer->vertex[i].y, m_objectParam[objRank].bboxMax.y);
			m_objectParam[objRank].bboxMax.z = Max(buffer->vertex[i].z, m_objectParam[objRank].bboxMax.z);
		}

		m_objectParam[objRank].radius = Max(Length(m_objectParam[objRank].bboxMin),
											Length(m_objectParam[objRank].bboxMax));
	}
	m_objectParam[objRank].totalTriangle += buffer->totalUsed-2;

	return TRUE;
}


// Cherche une liste de triangles.

void CD3DEngine::ChangeLOD()
{
	D3DObjLevel1*	p1;
	D3DObjLevel2a*	p2a;
	D3DObjLevel2b*	p2b;
	D3DObjLevel3*	p3;
	D3DObjLevel4*	p4;
	int				l1, l2a, l2b, l3;
	float			oldLimit[2], newLimit[2];
	float			oldTerrain, newTerrain;

	oldLimit[0] = RetLimitLOD(0, TRUE);
	oldLimit[1] = RetLimitLOD(1, TRUE);

	newLimit[0] = RetLimitLOD(0, FALSE);
	newLimit[1] = RetLimitLOD(1, FALSE);

	oldTerrain = m_terrainVision*m_lastClippingDistance;
	newTerrain = m_terrainVision*m_clippingDistance;

	p1 = m_objectPointer;
	for ( l1=0 ; l1<p1->totalUsed ; l1++ )
	{
		p2a = p1->table[l1];
		if ( p2a == 0 )  continue;
		for ( l2a=0 ; l2a<p2a->totalUsed ; l2a++ )
		{
			p2b = p2a->table[l2a];
			if ( p2b == 0 )  continue;
			for ( l2b=0 ; l2b<p2b->totalUsed ; l2b++ )
			{
				p3 = p2b->table[l2b];
				if ( p3 == 0 )  continue;
				for ( l3=0 ; l3<p3->totalUsed ; l3++ )
				{
					p4 = p3->table[l3];
					if ( p4 == 0 )  continue;

					if ( IsEqual(p4->min, 0.0f       ) &&
						 IsEqual(p4->max, oldLimit[0]) )
					{
						p4->max = newLimit[0];
					}
					else if ( IsEqual(p4->min, oldLimit[0]) &&
							  IsEqual(p4->max, oldLimit[1]) )
					{
						p4->min = newLimit[0];
						p4->max = newLimit[1];
					}
					else if ( IsEqual(p4->min, oldLimit[1]) &&
							  IsEqual(p4->max, 1000000.0f ) )
					{
						p4->min = newLimit[1];
					}
					else if ( IsEqual(p4->min, 0.0f      ) &&
							  IsEqual(p4->max, oldTerrain) )
					{
						p4->max = newTerrain;
					}
				}
			}
		}
	}

	m_lastDim = m_dim;
	m_lastObjectDetail = m_objectDetail;
	m_lastClippingDistance = m_clippingDistance;
}

// Cherche une liste de triangles.

D3DObjLevel6* CD3DEngine::SearchTriangle(int objRank,
										 const D3DMATERIAL7 &mat, int state,
										 char* texName1, char* texName2,
										 float min, float max)
{
	D3DObjLevel1*	p1;
	D3DObjLevel2a*	p2a;
	D3DObjLevel2b*	p2b;
	D3DObjLevel3*	p3;
	D3DObjLevel4*	p4;
	D3DObjLevel5*	p5;
	D3DObjLevel6*	p6;
	int				l1, l2a, l2b, l3, l4, l5;
	short			type;

	type = 0;
	if ( state & (D3DSTATETTb|D3DSTATETTw) )  type = 1;

	p1 = m_objectPointer;
	for ( l1=0 ; l1<p1->totalUsed ; l1++ )
	{
		p2a = p1->table[l1];
		if ( p2a->type != type )  continue;
		for ( l2a=0 ; l2a<p2a->totalUsed ; l2a++ )
		{
			p2b = p2a->table[l2a];
			if ( p2b == 0 )  continue;
//?			if ( strcmp(p2b->texName1, texName1) != 0 ||
//?				 strcmp(p2b->texName2, texName2) != 0 )  continue;
			if ( strcmp(p2b->texName1, texName1) != 0 )  continue;
			for ( l2b=0 ; l2b<p2b->totalUsed ; l2b++ )
			{
				p3 = p2b->table[l2b];
				if ( p3 == 0 )  continue;
				if ( p3->objRank != objRank )  continue;
				for ( l3=0 ; l3<p3->totalUsed ; l3++ )
				{
					p4 = p3->table[l3];
					if ( p4 == 0 )  continue;
					if ( p4->min != min ||
						 p4->max != max )  continue;
					for ( l4=0 ; l4<p4->totalUsed ; l4++ )
					{
						p5 = p4->table[l4];
						if ( p5 == 0 )  continue;
						for ( l5=0 ; l5<p5->totalUsed ; l5++ )
						{
							p6 = p5->table[l5];
							if ( p6 == 0 )  continue;
//?							if ( p6->state != state ||
							if ( (p6->state&(~(D3DSTATEDUALb|D3DSTATEDUALw))) != state ||
								 memcmp(&p6->material, &mat, sizeof(D3DMATERIAL7)) != 0 )  continue;
							return p6;
						}
					}
				}
			}
		}
	}
	return 0;
}

// Cherche une liste de triangles.

D3DObjLevel6* CD3DEngine::SearchTriangle(int objRank,
										 char* texName1, char* texName2,
										 float min, float max)
{
	D3DObjLevel1*	p1;
	D3DObjLevel2a*	p2a;
	D3DObjLevel2b*	p2b;
	D3DObjLevel3*	p3;
	D3DObjLevel4*	p4;
	D3DObjLevel5*	p5;
	D3DObjLevel6*	p6;
	int				l1, l2a, l2b, l3, l4, l5;
	short			type;

	type = 0;

	p1 = m_objectPointer;
	for ( l1=0 ; l1<p1->totalUsed ; l1++ )
	{
		p2a = p1->table[l1];
		if ( p2a->type != type )  continue;
		for ( l2a=0 ; l2a<p2a->totalUsed ; l2a++ )
		{
			p2b = p2a->table[l2a];
			if ( p2b == 0 )  continue;
//?			if ( strcmp(p2b->texName1, texName1) != 0 ||
//?				 strcmp(p2b->texName2, texName2) != 0 )  continue;
			if ( strcmp(p2b->texName1, texName1) != 0 )  continue;
			for ( l2b=0 ; l2b<p2b->totalUsed ; l2b++ )
			{
				p3 = p2b->table[l2b];
				if ( p3 == 0 )  continue;
				if ( p3->objRank != objRank )  continue;
				for ( l3=0 ; l3<p3->totalUsed ; l3++ )
				{
					p4 = p3->table[l3];
					if ( p4 == 0 )  continue;
					if ( p4->min != min ||
						 p4->max != max )  continue;
					for ( l4=0 ; l4<p4->totalUsed ; l4++ )
					{
						p5 = p4->table[l4];
						if ( p5 == 0 )  continue;
						for ( l5=0 ; l5<p5->totalUsed ; l5++ )
						{
							p6 = p5->table[l5];
							if ( p6 == 0 )  continue;
							return p6;
						}
					}
				}
			}
		}
	}
	return 0;
}

// Change la texture secondaire d'un objet.

BOOL CD3DEngine::ChangeSecondTexture(int objRank, char* texName2)
{
	D3DObjLevel2b*	newp2;
	D3DObjLevel1*	p1;
	D3DObjLevel2a*	p2a;
	D3DObjLevel2b*	p2b;
	D3DObjLevel3*	p3;
	int				l1, l2a, l2b;

	p1 = m_objectPointer;
	for ( l1=0 ; l1<p1->totalUsed ; l1++ )
	{
		p2a = p1->table[l1];
		for ( l2a=0 ; l2a<p2a->totalUsed ; l2a++ )
		{
			p2b = p2a->table[l2a];
			if ( p2b == 0 )  continue;
			if ( strcmp(p2b->texName2, texName2) == 0 )  continue;  // déjà nouvelle
			for ( l2b=0 ; l2b<p2b->totalUsed ; l2b++ )
			{
				p3 = p2b->table[l2b];
				if ( p3 == 0 )  continue;
				if ( p3->objRank != objRank )  continue;

				newp2 = AddLevel2a(p2a, p2b->texName1, texName2);

				if ( newp2->totalUsed >= newp2->totalPossible )  continue;  // faire mieux !!!
				newp2->table[newp2->totalUsed++] = p3;

				p2b->table[l2b] = 0;
			}
		}
	}
	return TRUE;
}

// Transforme un objet au complet.
// Effectue dans l'ordre :
//	1)	Une translation "move1"
//	2)	Une rotation "angle"
//	3)	Une translation "move2"

void CD3DEngine::TransformObject(int objRank, D3DVECTOR move1, D3DVECTOR angle, D3DVECTOR move2)
{
	D3DObjLevel1*	p1;
	D3DObjLevel2a*	p2a;
	D3DObjLevel2b*	p2b;
	D3DObjLevel3*	p3;
	D3DObjLevel4*	p4;
	D3DObjLevel5*	p5;
	D3DObjLevel6*	p6;
	D3DVERTEX2*		pv;
	int				l1, l2a, l2b, l3, l4, l5;

	p1 = m_objectPointer;
	for ( l1=0 ; l1<p1->totalUsed ; l1++ )
	{
		p2a = p1->table[l1];
		if ( p2a == 0 )  continue;
		for ( l2a=0 ; l2a<p2a->totalUsed ; l2a++ )
		{
			p2b = p2a->table[l2a];
			if ( p2b == 0 )  continue;
			for ( l2b=0 ; l2b<p2b->totalUsed ; l2b++ )
			{
				p3 = p2b->table[l2b];
				if ( p3 == 0 )  continue;
				if ( p3->objRank != objRank )  continue;
				for ( l3=0 ; l3<p3->totalUsed ; l3++ )
				{
					p4 = p3->table[l3];
					if ( p4 == 0 )  continue;
					for ( l4=0 ; l4<p4->totalUsed ; l4++ )
					{
						p5 = p4->table[l4];
						if ( p5 == 0 )  continue;
						for ( l5=0 ; l5<p5->totalUsed ; l5++ )
						{
							p6 = p5->table[l5];
							if ( p6 == 0 )  continue;
							pv = &p6->vertex[0];
							TransformVertex(pv, p6->totalUsed, move1, angle, move2);
						}
					}
				}
			}
		}
	}
}

// Transforme une liste de vertex.

void CD3DEngine::TransformVertex(D3DVERTEX2* pv, int nb,
								 const D3DVECTOR &move1,
								 const D3DVECTOR &angle,
								 const D3DVECTOR &move2)
{
	FPOINT	rot;
	int		i;

	for ( i=0 ; i<nb ; i++ )
	{
		pv->x += move1.x;
		pv->y += move1.y;
		pv->z += move1.z;

		if ( angle.x != 0.0f )
		{
			rot.x = pv->z;
			rot.y = pv->y;
			rot = RotatePoint(angle.x, rot);
			pv->z = rot.x;
			pv->y = rot.y;

			rot.x = pv->nz;
			rot.y = pv->ny;
			rot = RotatePoint(angle.x, rot);
			pv->nz = rot.x;
			pv->ny = rot.y;
		}
		if ( angle.y != 0.0f )
		{
			rot.x = pv->x;
			rot.y = pv->z;
			rot = RotatePoint(angle.y, rot);
			pv->x = rot.x;
			pv->z = rot.y;

			rot.x = pv->nx;
			rot.y = pv->nz;
			rot = RotatePoint(angle.y, rot);
			pv->nx = rot.x;
			pv->nz = rot.y;
		}
		if ( angle.z != 0.0f )
		{
			rot.x = pv->x;
			rot.y = pv->y;
			rot = RotatePoint(angle.z, rot);
			pv->x = rot.x;
			pv->y = rot.y;

			rot.x = pv->nx;
			rot.y = pv->ny;
			rot = RotatePoint(angle.z, rot);
			pv->nx = rot.x;
			pv->ny = rot.y;
		}

		pv->x += move2.x;
		pv->y += move2.y;
		pv->z += move2.z;

		pv ++;
	}
}


// Retourne le nombre de triangles de l'objet.

int CD3DEngine::RetTotalTriangles(int objRank)
{
	return m_objectParam[objRank].totalTriangle;
}

// Retourne qq triangles d'un objet. Utilisé pour extraire qq
// triangles d'un objet qui explose.
// "percent" est compris entre 0 et 1.

int CD3DEngine::GetTriangles(int objRank, float min, float max,
							  D3DTriangle* buffer, int size, float percent)
{
	D3DObjLevel1*	p1;
	D3DObjLevel2a*	p2a;
	D3DObjLevel2b*	p2b;
	D3DObjLevel3*	p3;
	D3DObjLevel4*	p4;
	D3DObjLevel5*	p5;
	D3DObjLevel6*	p6;
	D3DVERTEX2*		pv;
	int				l1, l2a, l2b, l3, l4, l5, l6, i, rank;

	rank = 0;
	i = 0;
	p1 = m_objectPointer;
	for ( l1=0 ; l1<p1->totalUsed ; l1++ )
	{
		p2a = p1->table[l1];
		if ( p2a == 0 )  continue;
		for ( l2a=0 ; l2a<p2a->totalUsed ; l2a++ )
		{
			p2b = p2a->table[l2a];
			if ( p2b == 0 )  continue;
//?			if ( p2b->texName[0] == 0 )  continue;
			for ( l2b=0 ; l2b<p2b->totalUsed ; l2b++ )
			{
				p3 = p2b->table[l2b];
				if ( p3 == 0 )  continue;
				if ( p3->objRank != objRank )  continue;
				for ( l3=0 ; l3<p3->totalUsed ; l3++ )
				{
					p4 = p3->table[l3];
					if ( p4 == 0 )  continue;
					if ( p4->min != min ||
						 p4->max != max )  continue;
					for ( l4=0 ; l4<p4->totalUsed ; l4++ )
					{
						p5 = p4->table[l4];
						if ( p5 == 0 )  continue;
						for ( l5=0 ; l5<p5->totalUsed ; l5++ )
						{
							p6 = p5->table[l5];
							if ( p6 == 0 )  continue;
							if ( p6->type == D3DTYPE6T )
							{
								pv = &p6->vertex[0];
								for ( l6=0 ; l6<p6->totalUsed/3 ; l6++ )
								{
									if ( (float)i/rank <= percent )
									{
										if ( i >= size )  break;
										buffer[i].triangle[0] = pv[0];
										buffer[i].triangle[1] = pv[1];
										buffer[i].triangle[2] = pv[2];
										buffer[i].material = p6->material;
										buffer[i].state = p6->state;
										strcpy(buffer[i].texName1, p2b->texName1);
										strcpy(buffer[i].texName2, p2b->texName2);
										i ++;
									}
									rank ++;
									pv += 3;
								}
							}
							if ( p6->type == D3DTYPE6S )
							{
								pv = &p6->vertex[0];
								for ( l6=0 ; l6<p6->totalUsed-2 ; l6++ )
								{
									if ( (float)i/rank <= percent )
									{
										if ( i >= size )  break;
										buffer[i].triangle[0] = pv[0];
										buffer[i].triangle[1] = pv[1];
										buffer[i].triangle[2] = pv[2];
										buffer[i].material = p6->material;
										buffer[i].state = p6->state;
										strcpy(buffer[i].texName1, p2b->texName1);
										strcpy(buffer[i].texName2, p2b->texName2);
										i ++;
									}
									rank ++;
									pv += 1;
								}
							}
						}
					}
				}
			}
		}
	}
	return i;
}

// Donne la bbox d'un objet.

BOOL CD3DEngine::GetBBox(int objRank, D3DVECTOR &min, D3DVECTOR &max)
{
	min = m_objectParam[objRank].bboxMin;
	max = m_objectParam[objRank].bboxMax;
	return TRUE;
}


// Change le mapping de texture pour toute une liste de triangles.

BOOL CD3DEngine::ChangeTextureMapping(int objRank,
									  const D3DMATERIAL7 &mat, int state,
									  char* texName1, char* texName2,
									  float min, float max,
									  D3DMaping mode,
									  float au, float bu,
									  float av, float bv)
{
	D3DObjLevel6*	p6;
	D3DVERTEX2*		pv;
	float			tmin, tmax, auu, buu;
	int				l6, nb;

	p6 = SearchTriangle(objRank, mat, state, texName1, texName2, min, max);
	if ( p6 == 0 )  return FALSE;

	pv = &p6->vertex[0];
	nb = p6->totalUsed;

	if ( mode == D3DMAPPINGX )
	{
		for ( l6=0 ; l6<nb ; l6++ )
		{
			pv->tu = pv->z*au+bu;
			pv->tv = pv->y*av+bv;
			pv ++;
		}
	}

	if ( mode == D3DMAPPINGY )
	{
		for ( l6=0 ; l6<nb ; l6++ )
		{
			pv->tu = pv->x*au+bu;
			pv->tv = pv->z*av+bv;
			pv ++;
		}
	}

	if ( mode == D3DMAPPINGZ )
	{
		for ( l6=0 ; l6<nb ; l6++ )
		{
			pv->tu = pv->x*au+bu;
			pv->tv = pv->y*av+bv;
			pv ++;
		}
	}

	if ( mode == D3DMAPPING1X )
	{
		for ( l6=0 ; l6<nb ; l6++ )
		{
			pv->tu = pv->x*au+bu;
			pv ++;
		}
	}

	if ( mode == D3DMAPPING1Y )
	{
		for ( l6=0 ; l6<nb ; l6++ )
		{
			pv->tv = pv->y*au+bu;
			pv ++;
		}
	}

	if ( mode == D3DMAPPING1Z )
	{
		for ( l6=0 ; l6<nb ; l6++ )
		{
			pv->tu = pv->z*au+bu;
			pv ++;
		}
	}

	if ( mode == D3DMAPPINGMX )
	{
		for ( l6=0 ; l6<nb ; l6+=3 )
		{
			tmin = Min(pv[0].z, pv[1].z, pv[2].z);
			tmax = Max(pv[0].z, pv[1].z, pv[2].z);

			auu = (bu-au)/(tmax-tmin);
			buu = bu-tmax*(bu-au)/(tmax-tmin);

			pv[0].tu = pv[0].z*auu+buu;
			pv[1].tu = pv[1].z*auu+buu;
			pv[2].tu = pv[2].z*auu+buu;
			pv += 3;
		}
	}

	if ( mode == D3DMAPPINGMY )
	{
	}

	if ( mode == D3DMAPPINGMZ )
	{
	}

	return TRUE;
}

// Change le mapping de texture pour toute une liste de triangles.

BOOL CD3DEngine::AddTextureMapping(int objRank,
								  char* texName1, char* texName2,
								  float min, float max,
								  D3DMaping mode,
								  float offset)
{
	D3DObjLevel6*	p6;
	D3DVERTEX2*		pv;
	int				l6, nb;

	p6 = SearchTriangle(objRank, texName1, texName2, min, max);
	if ( p6 == 0 )  return FALSE;

	pv = &p6->vertex[0];
	nb = p6->totalUsed;

	if ( mode == D3DMAPPINGX )
	{
		for ( l6=0 ; l6<nb ; l6++ )
		{
			pv->tu += offset;
			pv ++;
		}
	}

	if ( mode == D3DMAPPINGY )
	{
		for ( l6=0 ; l6<nb ; l6++ )
		{
			pv->tv += offset;
			pv ++;
		}
	}

	return TRUE;
}

// Change le mapping de texture pour toute une liste de triangles,
// afin de simuler une chenille qui tourne.
// Seul le mapping selon "u" est changé.
//
//	pos: position sur le pourtour [p]
//	tl:  longeur élément répétitif de la texture [t]
//	ts:  début de la texture [t]
//	tt:  largeur totale de la texture [t]
//
//	[p] = distance dans l'univers 3D
//	[t] = position dans la texture (pixels)

//  ^ y         5 
//  |   6  o---------o  4
//  |    /             \
//  |   o               o
//  | 7 |               | 3
//  |   o  current      o
//  |    \ |           /
//  |   0  o---------o  2
//  |           1
// -o-----------------------> x
//  |
//
// Quand l6=1 :
//     0      1     2  3  4  ...  7
//    o--o---------o--o--o--o-//-o--o développement chenille
//    |ps|         |
//    <-->  pe     |
//    <------------>
//
// Texture :
//   o---------------o
//   |               |
//   |     o-o-o-o-o |
//   |     | | | | |<--- texture de la chenille
//   |     o-o-o-o-o |
//   |     | | tl    |
//   |   ->|-|<---   |
//   |     |         |
//   o-----|---------o--> u
//   | ts  |         |
//   <-----> tt      |
//   <--------------->

BOOL CD3DEngine::TrackTextureMapping(int objRank,
									 const D3DMATERIAL7 &mat, int state,
									 char* texName1, char* texName2,
									 float min, float max,
									 D3DMaping mode, float pos, float factor,
									 float tl, float ts, float tt)
{
	D3DObjLevel6*	p6;
	D3DVERTEX2*		pv;
	D3DVECTOR		current;
	float			ps, pe, pps, ppe, offset;
	int				l6, nb, i, j, s, e;
	int				is[6], ie[6];

	p6 = SearchTriangle(objRank, mat, state, texName1, texName2, min, max);
	if ( p6 == 0 )  return FALSE;

	pv = &p6->vertex[0];
	nb = p6->totalUsed;

	if ( nb < 12 || nb%6 != 0 )  return FALSE;

	while ( pos < 0.0f )
	{
		pos += 1000000.0f;  // jamais négatif !
	}

	for ( i=0 ; i<6 ; i++ )
	{
		for ( j=0 ; j<6 ; j++ )
		{
			if ( pv[i].x == pv[j+6].x &&
				 pv[i].y == pv[j+6].y )
			{
				current.x = pv[i].x;  // position fin maillon
				current.y = pv[i].y;
				break;
			}
		}
	}

	ps = 0.0f;  // position de début sur le pourtour
	for ( l6=0 ; l6<nb/6 ; l6++ )
	{
		s = e = 0;
		for ( i=0 ; i<6 ; i++ )
		{
			if ( Abs(pv[i].x-current.x) < 0.0001f &&
				 Abs(pv[i].y-current.y) < 0.0001f )
			{
				ie[e++] = i;
			}
			else
			{
				is[s++] = i;
			}
		}
		if ( s == 3 && e == 3 )
		{
			pe = ps+Length(pv[is[0]].x-pv[ie[0]].x,
						   pv[is[0]].y-pv[ie[0]].y)/factor;  // position de fin sur le pourtour

			pps = ps+pos;
			ppe = pe+pos;
			offset = (float)((int)pps);
			pps -= offset;
			ppe -= offset;

			for ( i=0 ; i<3 ; i++ )
			{
				pv[is[i]].tu = ((pps*tl)+ts)/tt;
				pv[ie[i]].tu = ((ppe*tl)+ts)/tt;
			}
		}

		if ( l6 >= (nb/6)-1 )  break;
		for ( i=0 ; i<6 ; i++ )
		{
			if ( Abs(pv[i+6].x-current.x) > 0.0001f ||
				 Abs(pv[i+6].y-current.y) > 0.0001f )
			{
				current.x = pv[i+6].x;  // fin maillon suivant
				current.y = pv[i+6].y;
				break;
			}
		}
		ps = pe;  // position de début suivante sur le pourtour
		pv += 6;
	}

	return TRUE;
}


// Met à jour tous les paramètres géométriques des objets.

void CD3DEngine::UpdateGeometry()
{
	D3DObjLevel1*	p1;
	D3DObjLevel2a*	p2a;
	D3DObjLevel2b*	p2b;
	D3DObjLevel3*	p3;
	D3DObjLevel4*	p4;
	D3DObjLevel5*	p5;
	D3DObjLevel6*	p6;
	int				l1, l2a, l2b, l3, l4, l5, objRank, i;

	if ( !m_bUpdateGeometry )  return;

	for ( i=0 ; i<m_objectParamTotal ; i++ )
	{
		m_objectParam[i].bboxMin.x = 0;
		m_objectParam[i].bboxMin.y = 0;
		m_objectParam[i].bboxMin.z = 0;
		m_objectParam[i].bboxMax.x = 0;
		m_objectParam[i].bboxMax.y = 0;
		m_objectParam[i].bboxMax.z = 0;
		m_objectParam[i].radius = 0;
	}

	p1 = m_objectPointer;
	for ( l1=0 ; l1<p1->totalUsed ; l1++ )
	{
		p2a = p1->table[l1];
		if ( p2a == 0 )  continue;
		for ( l2a=0 ; l2a<p2a->totalUsed ; l2a++ )
		{
			p2b = p2a->table[l2a];
			if ( p2b == 0 )  continue;
			for ( l2b=0 ; l2b<p2b->totalUsed ; l2b++ )
			{
				p3 = p2b->table[l2b];
				if ( p3 == 0 )  continue;
				objRank = p3->objRank;
				for ( l3=0 ; l3<p3->totalUsed ; l3++ )
				{
					p4 = p3->table[l3];
					if ( p4 == 0 )  continue;
					for ( l4=0 ; l4<p4->totalUsed ; l4++ )
					{
						p5 = p4->table[l4];
						if ( p5 == 0 )  continue;
						for ( l5=0 ; l5<p5->totalUsed ; l5++ )
						{
							p6 = p5->table[l5];
							if ( p6 == 0 )  continue;

							for ( i=0 ; i<p6->totalUsed ; i++ )
							{
								m_objectParam[objRank].bboxMin.x = Min(p6->vertex[i].x, m_objectParam[objRank].bboxMin.x);
								m_objectParam[objRank].bboxMin.y = Min(p6->vertex[i].y, m_objectParam[objRank].bboxMin.y);
								m_objectParam[objRank].bboxMin.z = Min(p6->vertex[i].z, m_objectParam[objRank].bboxMin.z);
								m_objectParam[objRank].bboxMax.x = Max(p6->vertex[i].x, m_objectParam[objRank].bboxMax.x);
								m_objectParam[objRank].bboxMax.y = Max(p6->vertex[i].y, m_objectParam[objRank].bboxMax.y);
								m_objectParam[objRank].bboxMax.z = Max(p6->vertex[i].z, m_objectParam[objRank].bboxMax.z);
							}

							m_objectParam[objRank].radius = Max(Length(m_objectParam[objRank].bboxMin),
																Length(m_objectParam[objRank].bboxMax));
						}
					}
				}
			}
		}
	}

	m_bUpdateGeometry = FALSE;
}


// Détermine si un objet est visible, même partiellement.
// La transformation "world" doit être faite !

BOOL CD3DEngine::IsVisible(int objRank)
{
	D3DVECTOR	center;
	DWORD		flags;
	float		radius;

	radius = m_objectParam[objRank].radius;
	center = D3DVECTOR(0.0f, 0.0f, 0.0f);
	m_pD3DDevice->ComputeSphereVisibility(&center, &radius, 1, 0, &flags);

	if ( flags & D3DSTATUS_CLIPINTERSECTIONALL )
	{
		m_objectParam[objRank].bVisible = FALSE;
		return FALSE;
	}
	m_objectParam[objRank].bVisible = TRUE;
	return TRUE;
}


// Détecte la position au sol visée par la souris.

BOOL CD3DEngine::DetectTerrain(FPOINT mouse, D3DVECTOR &pos, int maxLevel)
{
	D3DObjLevel1*	p1;
	D3DObjLevel2a*	p2a;
	D3DObjLevel2b*	p2b;
	D3DObjLevel3*	p3;
	D3DObjLevel4*	p4;
	D3DObjLevel5*	p5;
	D3DObjLevel6*	p6;
	D3DVERTEX2*		pv;
	int				l1, l2a, l2b, l3, l4, l5, i, objRank, nearest;
	float			dist, min;
	D3DVECTOR		p;

	min = 1000000.0f;
	nearest = -1;

	p1 = m_objectPointer;
	for ( l1=0 ; l1<p1->totalUsed ; l1++ )
	{
		p2a = p1->table[l1];
		if ( p2a == 0 )  continue;
		for ( l2a=0 ; l2a<p2a->totalUsed ; l2a++ )
		{
			p2b = p2a->table[l2a];
			if ( p2b == 0 )  continue;
			for ( l2b=0 ; l2b<p2b->totalUsed ; l2b++ )
			{
				p3 = p2b->table[l2b];
				if ( p3 == 0 )  continue;
				objRank = p3->objRank;
				if ( m_objectParam[objRank].bHide )  continue;
				if ( !m_objectParam[objRank].bDetect )  continue;
				if ( m_objectParam[objRank].type != TYPETERRAIN )  continue;
//?				if ( !DetectBBox(objRank, mouse) )  continue;
				for ( l3=0 ; l3<p3->totalUsed ; l3++ )
				{
					p4 = p3->table[l3];
					if ( p4 == 0 )  continue;
					for ( l4=0 ; l4<p4->totalUsed ; l4++ )
					{
						p5 = p4->table[l4];
						if ( p5 == 0 )  continue;
						for ( l5=0 ; l5<p5->totalUsed ; l5++ )
						{
							p6 = p5->table[l5];
							if ( p6 == 0 )  continue;

							if ( p6->type == D3DTYPE6T )
							{
								pv = &p6->vertex[0];
								for ( i=0 ; i<p6->totalUsed/3 ; i++ )
								{
									if ( DetectTriangle(mouse, pv, objRank, dist, p, 0, maxLevel) &&
										 dist < min )
									{
										min = dist;
										nearest = objRank;
										pos = p;
									}
									pv += 3;
								}
							}
							if ( p6->type == D3DTYPE6S )
							{
								pv = &p6->vertex[0];
								for ( i=0 ; i<p6->totalUsed-2 ; i++ )
								{
									if ( DetectTriangle(mouse, pv, objRank, dist, p, 0, maxLevel) &&
										 dist < min )
									{
										min = dist;
										nearest = objRank;
										pos = p;
									}
									pv += 1;
								}
							}
						}
					}
				}
			}
		}
	}
	return ( nearest != -1 );
}

// Détecte l'objet visé par la souris.
// Retourne le rang de l'objet ou -1.
//	bAction=FALSE -> détecte tout
//	bAction=TRUE  -> détecte seulement les boutons-actions
//  bAllDetect=TRUE -> détecte même les objets non détectables !

int CD3DEngine::DetectObject(FPOINT mouse, BOOL bAction, BOOL bAllDetect)
{
	D3DObjLevel1*	p1;
	D3DObjLevel2a*	p2a;
	D3DObjLevel2b*	p2b;
	D3DObjLevel3*	p3;
	D3DObjLevel4*	p4;
	D3DObjLevel5*	p5;
	D3DObjLevel6*	p6;
	D3DVERTEX2*		pv;
	int				l1, l2a, l2b, l3, l4, l5, i, objRank, nearest;
	float			dist, min;

	min = 1000000.0f;
	nearest = -1;

	p1 = m_objectPointer;
	for ( l1=0 ; l1<p1->totalUsed ; l1++ )
	{
		p2a = p1->table[l1];
		if ( p2a == 0 )  continue;
		for ( l2a=0 ; l2a<p2a->totalUsed ; l2a++ )
		{
			p2b = p2a->table[l2a];
			if ( p2b == 0 )  continue;
			for ( l2b=0 ; l2b<p2b->totalUsed ; l2b++ )
			{
				p3 = p2b->table[l2b];
				if ( p3 == 0 )  continue;
				objRank = p3->objRank;
				if ( m_objectParam[objRank].bHide )  continue;
				if ( !bAllDetect && !m_objectParam[objRank].bDetect )  continue;
				if ( m_objectParam[objRank].type == TYPETERRAIN )  continue;
				if ( !DetectBBox(objRank, mouse) )  continue;
				for ( l3=0 ; l3<p3->totalUsed ; l3++ )
				{
					p4 = p3->table[l3];
					if ( p4 == 0 )  continue;
					if ( p4->min != 0.0f )  continue;  // LOD B ou C ?
					for ( l4=0 ; l4<p4->totalUsed ; l4++ )
					{
						p5 = p4->table[l4];
						if ( p5 == 0 )  continue;
						for ( l5=0 ; l5<p5->totalUsed ; l5++ )
						{
							p6 = p5->table[l5];
							if ( p6 == 0 )  continue;
							if ( bAction )
							{
								if ( (p6->state&D3DSTATEACTION) == 0 )  continue;
							}

							if ( p6->type == D3DTYPE6T )
							{
								pv = &p6->vertex[0];
								for ( i=0 ; i<p6->totalUsed/3 ; i++ )
								{
									if ( DetectTriangle(mouse, pv, objRank, dist) &&
										 dist < min )
									{
										min = dist;
										nearest = objRank;
									}
									pv += 3;
								}
							}
							if ( p6->type == D3DTYPE6S )
							{
								pv = &p6->vertex[0];
								for ( i=0 ; i<p6->totalUsed-2 ; i++ )
								{
									if ( DetectTriangle(mouse, pv, objRank, dist) &&
										 dist < min )
									{
										min = dist;
										nearest = objRank;
									}
									pv += 1;
								}
							}
						}
					}
				}
			}
		}
	}
	return nearest;
}

// Détecte si un objet est visé par la souris.

BOOL CD3DEngine::DetectBBox(int objRank, FPOINT mouse)
{
	D3DVECTOR	p, pp;
	FPOINT		min, max;
	int			i;

	min.x =  1000000.0f;
	min.y =  1000000.0f;
	max.x = -1000000.0f;
	max.y = -1000000.0f;

	for ( i=0 ; i<8 ; i++ )
	{
		if ( i & (1<<0) )  p.x = m_objectParam[objRank].bboxMin.x;
		else               p.x = m_objectParam[objRank].bboxMax.x;
		if ( i & (1<<1) )  p.y = m_objectParam[objRank].bboxMin.y;
		else               p.y = m_objectParam[objRank].bboxMax.y;
		if ( i & (1<<2) )  p.z = m_objectParam[objRank].bboxMin.z;
		else               p.z = m_objectParam[objRank].bboxMax.z;
		if ( TransformPoint(pp, objRank, p) )
		{
			if ( pp.x < min.x )  min.x = pp.x;
			if ( pp.x > max.x )  max.x = pp.x;
			if ( pp.y < min.y )  min.y = pp.y;
			if ( pp.y > max.y )  max.y = pp.y;
		}
	}

	return ( mouse.x >= min.x &&
			 mouse.x <= max.x &&
			 mouse.y >= min.y &&
			 mouse.y <= max.y );
}

// Détecte si la souris est dans un triangle du terrain.

BOOL CD3DEngine::DetectTriangle(FPOINT mouse, D3DVERTEX2 *vertex,
								int objRank, float &dist, D3DVECTOR &pos,
								int level, int maxLevel)
{
	D3DVECTOR	subTriangle[6];
	D3DVERTEX2	subVertex[3];
	D3DVECTOR	p2D[3], p3D;
	FPOINT		a, b, c, t1[3], t2[3];
	int			i, nb;

	nb = 0;
	for ( i=0 ; i<3 ; i++ )
	{
		p3D.x = vertex[i].x;
		p3D.y = vertex[i].y;
		p3D.z = vertex[i].z;
		if ( !TransformPoint(p2D[i], objRank, p3D) )  nb ++;
	}
	if ( nb == 3 )  return FALSE;  // tous les points derrière la caméra ?

	if ( mouse.x < p2D[0].x &&
		 mouse.x < p2D[1].x &&
		 mouse.x < p2D[2].x )  return FALSE;
	if ( mouse.x > p2D[0].x &&
		 mouse.x > p2D[1].x &&
		 mouse.x > p2D[2].x )  return FALSE;
	if ( mouse.y < p2D[0].y &&
		 mouse.y < p2D[1].y &&
		 mouse.y < p2D[2].y )  return FALSE;
	if ( mouse.y > p2D[0].y &&
		 mouse.y > p2D[1].y &&
		 mouse.y > p2D[2].y )  return FALSE;

	a.x = p2D[0].x;
	a.y = p2D[0].y;
	b.x = p2D[1].x;
	b.y = p2D[1].y;
	c.x = p2D[2].x;
	c.y = p2D[2].y;
	if ( !IsInsideTriangle(a, b, c, mouse) )  return FALSE;

	if ( level < maxLevel )
	{
		subTriangle[0].x = vertex[0].x;
		subTriangle[0].y = vertex[0].y;
		subTriangle[0].z = vertex[0].z;
		subTriangle[1].x = vertex[1].x;
		subTriangle[1].y = vertex[1].y;
		subTriangle[1].z = vertex[1].z;
		subTriangle[2].x = vertex[2].x;
		subTriangle[2].y = vertex[2].y;
		subTriangle[2].z = vertex[2].z;
		subTriangle[3].x = (vertex[0].x+vertex[1].x)/2.0f;
		subTriangle[3].y = (vertex[0].y+vertex[1].y)/2.0f;
		subTriangle[3].z = (vertex[0].z+vertex[1].z)/2.0f;
		subTriangle[4].x = (vertex[1].x+vertex[2].x)/2.0f;
		subTriangle[4].y = (vertex[1].y+vertex[2].y)/2.0f;
		subTriangle[4].z = (vertex[1].z+vertex[2].z)/2.0f;
		subTriangle[5].x = (vertex[2].x+vertex[0].x)/2.0f;
		subTriangle[5].y = (vertex[2].y+vertex[0].y)/2.0f;
		subTriangle[5].z = (vertex[2].z+vertex[0].z)/2.0f;

		subVertex[0].x = subTriangle[0].x;
		subVertex[0].y = subTriangle[0].y;
		subVertex[0].z = subTriangle[0].z;
		subVertex[1].x = subTriangle[3].x;
		subVertex[1].y = subTriangle[3].y;
		subVertex[1].z = subTriangle[3].z;
		subVertex[2].x = subTriangle[5].x;
		subVertex[2].y = subTriangle[5].y;
		subVertex[2].z = subTriangle[5].z;
		if ( DetectTriangle(mouse, subVertex, objRank, dist, pos, level+1, maxLevel) )  return TRUE;

		subVertex[0].x = subTriangle[3].x;
		subVertex[0].y = subTriangle[3].y;
		subVertex[0].z = subTriangle[3].z;
		subVertex[1].x = subTriangle[4].x;
		subVertex[1].y = subTriangle[4].y;
		subVertex[1].z = subTriangle[4].z;
		subVertex[2].x = subTriangle[5].x;
		subVertex[2].y = subTriangle[5].y;
		subVertex[2].z = subTriangle[5].z;
		if ( DetectTriangle(mouse, subVertex, objRank, dist, pos, level+1, maxLevel) )  return TRUE;

		subVertex[0].x = subTriangle[2].x;
		subVertex[0].y = subTriangle[2].y;
		subVertex[0].z = subTriangle[2].z;
		subVertex[1].x = subTriangle[5].x;
		subVertex[1].y = subTriangle[5].y;
		subVertex[1].z = subTriangle[5].z;
		subVertex[2].x = subTriangle[4].x;
		subVertex[2].y = subTriangle[4].y;
		subVertex[2].z = subTriangle[4].z;
		if ( DetectTriangle(mouse, subVertex, objRank, dist, pos, level+1, maxLevel) )  return TRUE;

		subVertex[0].x = subTriangle[1].x;
		subVertex[0].y = subTriangle[1].y;
		subVertex[0].z = subTriangle[1].z;
		subVertex[1].x = subTriangle[4].x;
		subVertex[1].y = subTriangle[4].y;
		subVertex[1].z = subTriangle[4].z;
		subVertex[2].x = subTriangle[3].x;
		subVertex[2].y = subTriangle[3].y;
		subVertex[2].z = subTriangle[3].z;
		if ( DetectTriangle(mouse, subVertex, objRank, dist, pos, level+1, maxLevel) )  return TRUE;

		return FALSE;
	}

//?	if ( nb != 0 )  return FALSE;

	dist = (p2D[0].z+p2D[1].z+p2D[2].z)/3.0f;

	for ( i=0 ; i<3 ; i++ )
	{
		t1[i].x = p2D[i].x;
		t1[i].y = p2D[i].y;

		p3D.x = vertex[i].x;
		p3D.y = vertex[i].y;
		p3D.z = vertex[i].z;
		pos = Transform(m_objectParam[objRank].transform, p3D);
		t2[i].x = pos.x;
		t2[i].y = pos.z;
	}

	a = mouse;
	if ( !Deforme(t1, t2, a) )  return FALSE;
	pos.x = a.x;
	pos.z = a.y;
	m_terrain->MoveOnFloor(pos);

	return TRUE;
}

// Détecte si la souris est dans un triangle d'un objet.

BOOL CD3DEngine::DetectTriangle(FPOINT mouse, D3DVERTEX2 *vertex,
								int objRank, float &dist)
{
	D3DVECTOR	p2D[3], p3D;
	FPOINT		a, b, c;
	int			i;

	for ( i=0 ; i<3 ; i++ )
	{
		p3D.x = vertex[i].x;
		p3D.y = vertex[i].y;
		p3D.z = vertex[i].z;
		if ( !TransformPoint(p2D[i], objRank, p3D) )  return FALSE;
	}

	if ( mouse.x < p2D[0].x &&
		 mouse.x < p2D[1].x &&
		 mouse.x < p2D[2].x )  return FALSE;
	if ( mouse.x > p2D[0].x &&
		 mouse.x > p2D[1].x &&
		 mouse.x > p2D[2].x )  return FALSE;
	if ( mouse.y < p2D[0].y &&
		 mouse.y < p2D[1].y &&
		 mouse.y < p2D[2].y )  return FALSE;
	if ( mouse.y > p2D[0].y &&
		 mouse.y > p2D[1].y &&
		 mouse.y > p2D[2].y )  return FALSE;

	a.x = p2D[0].x;
	a.y = p2D[0].y;
	b.x = p2D[1].x;
	b.y = p2D[1].y;
	c.x = p2D[2].x;
	c.y = p2D[2].y;
	if ( !IsInsideTriangle(a, b, c, mouse) )  return FALSE;

	dist = (p2D[0].z+p2D[1].z+p2D[2].z)/3.0f;
	return TRUE;
}

// Transforme un point 3D (x,y,z) dans l'espace 2D (x,y,-) de la fenêtre.
// La coordonnée p2D.z donne l'éloignement. La valeur est négative
// lorsque le point est derrière la caméra.

BOOL CD3DEngine::TransformPoint(D3DVECTOR &p2D, int objRank, D3DVECTOR p3D)
{
#if 0
	p3D = Transform(m_objectParam[objRank].transform, p3D);
	p3D = Transform(m_matView, p3D);

	if ( p3D.z < 2.0f )  return FALSE;  // derrière ?

	p2D.x = (p3D.x/p3D.z)*m_matProj._11;
	p2D.y = (p3D.y/p3D.z)*m_matProj._22;
	p2D.z = p3D.z;

	p2D.x = (p2D.x+1.0f)/2.0f;  // [-1..1] -> [0..1]
	p2D.y = (p2D.y+1.0f)/2.0f;

	return TRUE;
#else
	BOOL	bOK = TRUE;

	p3D = Transform(m_objectParam[objRank].transform, p3D);
	p3D = Transform(m_matView, p3D);

//?	if ( p3D.z < 2.0f )  bOK = FALSE;  // derrière ?
	if ( p3D.z < 0.1f )  bOK = FALSE;  // derrière ?

	p2D.x = (p3D.x/p3D.z)*m_matProj._11;
	p2D.y = (p3D.y/p3D.z)*m_matProj._22;
	p2D.z = p3D.z;

	p2D.x = (p2D.x+1.0f)/2.0f;  // [-1..1] -> [0..1]
	p2D.y = (p2D.y+1.0f)/2.0f;

	if ( p2D.z < 0.0f )  // derrière la caméra ?
	{
		p2D.x = -p2D.x;  // eh oui !
		p2D.y = -p2D.y;
	}

	return bOK;
#endif
}

// Calcul les distances entre le point de vue et l'origine
// des différents objets.

void CD3DEngine::ComputeDistance()
{
	int			i;
	float		distance;

	if ( s_resol == 0 )
	{
		if ( m_setup[ST_DETAIL] == 0.0f )
		{
			distance = (RetLimitLOD(0)+RetLimitLOD(1))/2.0f;
		}
		else
		{
			distance = 0.0f;
		}

		for ( i=0 ; i<m_objectParamTotal ; i++ )
		{
			if ( m_objectParam[i].bUsed == FALSE )  continue;

			m_objectParam[i].distance = distance;
		}
	}
	else
	{
		if ( s_resol == 1 )
		{
			distance = 100000.0f;
		}
		if ( s_resol == 2 )
		{
			distance = (RetLimitLOD(0)+RetLimitLOD(1))/2.0f;
		}
		if ( s_resol == 3 )
		{
			distance = 0.0f;
		}

		for ( i=0 ; i<m_objectParamTotal ; i++ )
		{
			if ( m_objectParam[i].bUsed == FALSE )  continue;

			m_objectParam[i].distance = distance;
		}
	}
}


// Adapte les réglages lors de la première exécution.

void CD3DEngine::FirstExecuteAdapt(BOOL bFirst)
{
	if ( m_app->IsVideo8MB() )
	{
		SetGroundSpot(FALSE);
	}
}

// Retourne la quantité totale de mémoire vidéo pour les textures.

int CD3DEngine::GetVidMemTotal()
{
	return m_app->GetVidMemTotal();
}

BOOL CD3DEngine::IsVideo8MB()
{
	return m_app->IsVideo8MB();
}

BOOL CD3DEngine::IsVideo32MB()
{
	return m_app->IsVideo32MB();
}


// Effectue la liste de tous les devices graphiques disponibles.

BOOL CD3DEngine::EnumDevices(char *bufDevices, int lenDevices,
							 char *bufModes, int lenModes,
							 int &totalDevices, int &selectDevices,
							 int &totalModes, int &selectModes)
{
	return m_app->EnumDevices(bufDevices, lenDevices,
							  bufModes, lenModes,
							  totalDevices, selectDevices,
							  totalModes, selectModes);
}

BOOL CD3DEngine::RetFullScreen()
{
	return m_app->RetFullScreen();
}

BOOL CD3DEngine::ChangeDevice(char *device, char *mode, BOOL bFull)
{
	return m_app->ChangeDevice(device, mode, bFull);
}



D3DMATRIX* CD3DEngine::RetMatView()
{
	return &m_matView;
}

D3DMATRIX* CD3DEngine::RetMatLeftView()
{
	return &m_matLeftView;
}

D3DMATRIX* CD3DEngine::RetMatRightView()
{
	return &m_matRightView;
}


// Spécifie l'emplacement et la direction du point de vue.

void CD3DEngine::SetViewParams(const D3DVECTOR &vEyePt,
							   const D3DVECTOR &vLookatPt,
							   const D3DVECTOR &vUpVec,
							   FLOAT fEyeDistance)
{
#if 0
	m_eyePt = vEyePt;

	// Adjust camera position for left or right eye along the axis
	// perpendicular to the view direction vector and the up vector.
	D3DVECTOR vView = (vLookatPt) - (vEyePt);
	vView = CrossProduct( vView, (vUpVec) );
	vView = Normalize( vView ) * fEyeDistance;

	D3DVECTOR vLeftEyePt  = (vEyePt) + vView;
	D3DVECTOR vRightEyePt = (vEyePt) - vView;

	// Set the view matrices
	D3DUtil_SetViewMatrix( m_matLeftView,  (D3DVECTOR)vLeftEyePt,  (D3DVECTOR)vLookatPt, (D3DVECTOR)vUpVec );
	D3DUtil_SetViewMatrix( m_matRightView, (D3DVECTOR)vRightEyePt, (D3DVECTOR)vLookatPt, (D3DVECTOR)vUpVec );
	D3DUtil_SetViewMatrix( m_matView,      (D3DVECTOR)vEyePt,      (D3DVECTOR)vLookatPt, (D3DVECTOR)vUpVec );
#else
	m_eyePt = vEyePt;
	m_lookatPt = vLookatPt;
	m_eyeDirH = RotateAngle(vEyePt.x-vLookatPt.x, vEyePt.z-vLookatPt.z);
	m_eyeDirV = RotateAngle(Length2d(vEyePt, vLookatPt), vEyePt.y-vLookatPt.y);

	D3DUtil_SetViewMatrix(m_matView, (D3DVECTOR)vEyePt, (D3DVECTOR)vLookatPt, (D3DVECTOR)vUpVec);

	if ( m_sound == 0 )
	{
		m_sound = (CSound*)m_iMan->SearchInstance(CLASS_SOUND);
	}
	m_sound->SetListener(vEyePt, vLookatPt);
#endif
}


// Spécifie la matrice de transformation d'un objet.

BOOL CD3DEngine::SetObjectTransform(int objRank, const D3DMATRIX &transform)
{
	if ( objRank < 0 || objRank >= D3DMAXOBJECT )  return FALSE;

	m_objectParam[objRank].transform = transform;
	return TRUE;
}

// Donne la matrice de transformation d'un objet.

BOOL CD3DEngine::GetObjectTransform(int objRank, D3DMATRIX &transform)
{
	if ( objRank < 0 || objRank >= D3DMAXOBJECT )  return FALSE;

	transform = m_objectParam[objRank].transform;
	return TRUE;
}

// Spécifie le type d'un objet.

BOOL CD3DEngine::SetObjectType(int objRank, D3DTypeObj type)
{
	if ( objRank < 0 || objRank >= D3DMAXOBJECT )  return FALSE;

	m_objectParam[objRank].type = type;
	return TRUE;
}

// Retourne le type d'un objet.

D3DTypeObj CD3DEngine::RetObjectType(int objRank)
{
	return m_objectParam[objRank].type;
}

// Spécifie si un objet est caché.

BOOL CD3DEngine::SetObjectHide(int objRank, BOOL bHide)
{
	if ( objRank < 0 || objRank >= D3DMAXOBJECT )  return FALSE;

	m_objectParam[objRank].bHide = bHide;
	return TRUE;
}

// Spécifie si un objet est détectable.

BOOL CD3DEngine::SetObjectDetect(int objRank, BOOL bDetect)
{
	if ( objRank < 0 || objRank >= D3DMAXOBJECT )  return FALSE;

	m_objectParam[objRank].bDetect = bDetect;
	return TRUE;
}

// Spécifie la transparence d'un objet.

BOOL CD3DEngine::SetObjectTransparency(int objRank, float value)
{
	if ( objRank < 0 || objRank >= D3DMAXOBJECT )  return FALSE;

	m_objectParam[objRank].transparency = value;
	return TRUE;
}


// Efface tous les remplacements de textures.

void CD3DEngine::FlushReplaceTex()
{
	ZeroMemory(m_replaceTex, sizeof(ReplaceTex)*REPLACETEXMAX);
}

// Ajoute un remplacement de texture.

BOOL CD3DEngine::SetReplaceTex(char *actual, char *future)
{
	int		i;

	for ( i=0 ; i<REPLACETEXMAX ; i++ )
	{
		if ( m_replaceTex[i].actual[0] == 0 )
		{
			strcpy(m_replaceTex[i].actual, actual);
			strcpy(m_replaceTex[i].future, future);
			return TRUE;
		}
	}
	return FALSE;
}

// Remplace un nom de texture si nécessaire.

void CD3DEngine::ReplaceTexDo(char *texName)
{
	int		i;

	for ( i=0 ; i<REPLACETEXMAX ; i++ )
	{
		if ( m_replaceTex[i].actual[0] == 0 )  break;

		if ( strcmp(texName, m_replaceTex[i].actual) == 0 )
		{
			strcpy(texName, m_replaceTex[i].future);
			return;
		}
	}
}


// Alloue une table pour l'ombre, si nécessaire.

BOOL CD3DEngine::ShadowCreate(int objRank)
{
	int		i;

	// Déjà alloué ?
	if ( m_objectParam[objRank].shadowRank != -1 )  return TRUE;

	for ( i=0 ; i<D3DMAXSHADOW ; i++ )
	{
		if ( m_shadow[i].bUsed == FALSE )  // libre ?
		{
			ZeroMemory(&m_shadow[i], sizeof(D3DShadow));

			m_shadow[i].bUsed = TRUE;
			m_shadow[i].objRank = objRank;
			m_shadow[i].height = 0.0f;

			m_objectParam[objRank].shadowRank = i;

			if ( m_shadowTotal < i+1 )
			{
				m_shadowTotal = i+1;
			}
			return TRUE;
		}
	}
	return FALSE;  // pas trouvé
}

// Supprime l'ombre associée à un objet.

void CD3DEngine::ShadowDelete(int objRank)
{
	int		i;

	if ( objRank == -1 )  return;

	i = m_objectParam[objRank].shadowRank;
	if ( i == -1 )  return;

	m_shadow[i].bUsed = FALSE;
	m_shadow[i].objRank = -1;
	m_shadow[i].pos = D3DVECTOR(0.0f, 0.0f, 0.0f);
	m_shadow[i].type = D3DSHADOWNORM;

	m_objectParam[objRank].shadowRank = -1;

	m_shadowTotal = 0;
	for ( i=0 ; i<D3DMAXSHADOW ; i++ )
	{
		if ( m_shadow[i].bUsed )  m_shadowTotal = i+1;
	}
}

// Spécifie si l'ombre est visible. Par exemple, lorsqu'un objet est
// porté, il n'a plus d'ombre.

BOOL CD3DEngine::SetObjectShadowHide(int objRank, BOOL bHide)
{
	if ( objRank < 0 || objRank >= D3DMAXOBJECT )  return FALSE;

	int i = m_objectParam[objRank].shadowRank;
	if ( i == -1 )  return FALSE;

	m_shadow[i].bHide = bHide;
	return TRUE;
}

// Spécifie le type de l'ombre de l'objet.

BOOL CD3DEngine::SetObjectShadowType(int objRank, D3DShadowType type)
{
	if ( objRank < 0 || objRank >= D3DMAXOBJECT )  return FALSE;

	int i = m_objectParam[objRank].shadowRank;
	if ( i == -1 )  return FALSE;

	m_shadow[i].type = type;
	return TRUE;
}

// Spécifie si l'objet est sélectionné.

BOOL CD3DEngine::SetObjectShadowSelect(int objRank, BOOL bSelect)
{
	if ( objRank < 0 || objRank >= D3DMAXOBJECT )  return FALSE;

	int i = m_objectParam[objRank].shadowRank;
	if ( i == -1 )  return FALSE;

	m_shadow[i].bSelect = bSelect;
	m_shadow[i].time = 0.0f;
	return TRUE;
}

// Spécifie la position de l'ombre de l'objet.

BOOL CD3DEngine::SetObjectShadowPos(int objRank, const D3DVECTOR &pos)
{
	if ( objRank < 0 || objRank >= D3DMAXOBJECT )  return FALSE;

	int i = m_objectParam[objRank].shadowRank;
	if ( i == -1 )  return FALSE;

	m_shadow[i].pos = pos;
	return TRUE;
}

// Spécifie l'angle de l'ombre de l'objet.

BOOL CD3DEngine::SetObjectShadowAngle(int objRank, float angle)
{
	if ( objRank < 0 || objRank >= D3DMAXOBJECT )  return FALSE;

	int i = m_objectParam[objRank].shadowRank;
	if ( i == -1 )  return FALSE;

	m_shadow[i].angle = angle;
	return TRUE;
}

// Spécifie le rayon de l'ombre de l'objet.

BOOL CD3DEngine::SetObjectShadowRadius(int objRank, float radius)
{
	if ( objRank < 0 || objRank >= D3DMAXOBJECT )  return FALSE;

	int i = m_objectParam[objRank].shadowRank;
	if ( i == -1 )  return FALSE;

	m_shadow[i].radius = radius;
	return TRUE;
}

// Retourne le rayon de l'ombre de l'objet.

float CD3DEngine::RetObjectShadowRadius(int objRank)
{
	if ( objRank < 0 || objRank >= D3DMAXOBJECT )  return 0.0f;

	int i = m_objectParam[objRank].shadowRank;
	if ( i == -1 )  return FALSE;

	return m_shadow[i].radius;
}

// Spécifie l'intensité de l'ombre de l'objet.

BOOL CD3DEngine::SetObjectShadowIntensity(int objRank, float intensity)
{
	if ( objRank < 0 || objRank >= D3DMAXOBJECT )  return FALSE;

	int i = m_objectParam[objRank].shadowRank;
	if ( i == -1 )  return FALSE;

	m_shadow[i].intensity = intensity;
	return TRUE;
}

// Spécifie la hauteur de l'ombre de l'objet.

BOOL CD3DEngine::SetObjectShadowHeight(int objRank, float h)
{
	if ( objRank < 0 || objRank >= D3DMAXOBJECT )  return FALSE;

	int i = m_objectParam[objRank].shadowRank;
	if ( i == -1 )  return FALSE;

	m_shadow[i].height = h;
	return TRUE;
}

// Spécifie la hauteur du soleil pour l'objet.
// Détermine l'amplitude du décalage selon le soleil au sud-est.
// Pour un objet normal, une valeur de 1 est l'idéal.
// Plus l'objet est haut et plus cette valeur peut grandir.
// A l'inverse, un objet tout plat (comme la cible du dock) doit utiliser 0.

BOOL CD3DEngine::SetObjectShadowSunFactor(int objRank, float factor)
{
	if ( objRank < 0 || objRank >= D3DMAXOBJECT )  return FALSE;

	int i = m_objectParam[objRank].shadowRank;
	if ( i == -1 )  return FALSE;

	m_shadow[i].sunFactor = factor;
	return TRUE;
}

// Spécifie où sont les trous (ul,u,ur,l,c,r,dl,d,dr).
// La position est au centre d'un tile 8x8 du terrain.

BOOL CD3DEngine::SetObjectShadowHole(int objRank, const D3DVECTOR &pos,
									 char bHole[9])
{
	int		j;

	if ( objRank < 0 || objRank >= D3DMAXOBJECT )  return FALSE;

	int i = m_objectParam[objRank].shadowRank;
	if ( i == -1 )  return FALSE;

	m_shadow[i].posTerrain = pos;

	for ( j=0 ; j<9 ; j++ )
	{
		m_shadow[i].bHole[j]  = bHole[j];
	}
	return TRUE;
}

// Efface toutes les marques au sol.

void CD3DEngine::GroundSpotFlush()
{
	LPDIRECTDRAWSURFACE7	surface;
	DDSURFACEDESC2			ddsd;
	WORD*					pbSurf;
	char					texName[20];
	int						s, y;

	ZeroMemory(m_groundSpot, sizeof(D3DGroundSpot)*D3DMAXGROUNDSPOT);
	m_bFirstGroundSpot = TRUE;  // force le dessin la première fois

	for ( s=0 ; s<16 ; s++ )
	{
		sprintf(texName, "shadow%.2d.tga", s);
		surface = D3DTextr_GetSurface(texName);
		if ( surface == 0 )  continue;

		ZeroMemory(&ddsd, sizeof(DDSURFACEDESC2));
		ddsd.dwSize = sizeof(DDSURFACEDESC2);
		if ( surface->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL) != DD_OK )  continue;

		if ( ddsd.ddpfPixelFormat.dwRGBBitCount != 16 )  continue;

		for ( y=0 ; y<(int)ddsd.dwHeight ; y++ )
		{
			pbSurf = (WORD*)ddsd.lpSurface;
			pbSurf += ddsd.lPitch*y/2;
			memset(pbSurf, -1, ddsd.lPitch);  // tout blanc
		}

		surface->Unlock(NULL);
	}
}

// Alloue une table pour une marque au sol, si nécessaire.

int CD3DEngine::GroundSpotCreate()
{
	int		i;

	for ( i=0 ; i<D3DMAXGROUNDSPOT ; i++ )
	{
		if ( m_groundSpot[i].bUsed == FALSE )  // libre ?
		{
			ZeroMemory(&m_groundSpot[i], sizeof(D3DGroundSpot));
			m_groundSpot[i].bUsed = TRUE;
			m_groundSpot[i].smooth = 1.0f;
			return i;
		}
	}
	return -1;  // pas trouvé
}

// Supprime une marque au sol.

void CD3DEngine::GroundSpotDelete(int rank)
{
	m_groundSpot[rank].bUsed = FALSE;
	m_groundSpot[rank].pos = D3DVECTOR(0.0f, 0.0f, 0.0f);
}

// Spécifie la position d'une marque au sol de l'objet.

BOOL CD3DEngine::SetObjectGroundSpotPos(int rank, const D3DVECTOR &pos)
{
	m_groundSpot[rank].pos = pos;
	return TRUE;
}

// Spécifie le rayon d'une marque au sol de l'objet.

BOOL CD3DEngine::SetObjectGroundSpotRadius(int rank, float radius)
{
	m_groundSpot[rank].radius = radius;
	return TRUE;
}

// Spécifie la couleur d'une marque au sol.

BOOL CD3DEngine::SetObjectGroundSpotColor(int rank, D3DCOLORVALUE color)
{
	m_groundSpot[rank].color = color;
	return TRUE;
}

// Spécifie les hauteurs min/max.

BOOL CD3DEngine::SetObjectGroundSpotMinMax(int rank, float min, float max)
{
	m_groundSpot[rank].min = min;
	m_groundSpot[rank].max = max;
	return TRUE;
}

// Spécifie le facteur de transition.

BOOL CD3DEngine::SetObjectGroundSpotSmooth(int rank, float smooth)
{
	m_groundSpot[rank].smooth = smooth;
	return TRUE;
}


// Crée les marques au sol.

int CD3DEngine::GroundMarkCreate(D3DVECTOR pos, float radius,
								 float delay1, float delay2, float delay3,
								 int dx, int dy, char* table)
{
	ZeroMemory(&m_groundMark, sizeof(D3DGroundMark));
	m_groundMark.bUsed     = TRUE;
	m_groundMark.phase     = 1;
	m_groundMark.delay[0]  = delay1;
	m_groundMark.delay[1]  = delay2;
	m_groundMark.delay[2]  = delay3;
	m_groundMark.pos       = pos;
	m_groundMark.radius    = radius;
	m_groundMark.intensity = 0.0f;
	m_groundMark.dx        = dx;
	m_groundMark.dy        = dy;
	m_groundMark.table     = table;
	return 0;
}

// Efface les marques au sol.

BOOL CD3DEngine::GroundMarkDelete(int rank)
{
	ZeroMemory(&m_groundMark, sizeof(D3DGroundMark));
	return TRUE;
}


// Gestion des frontières (distances limites) pour changer de résolution.
// LOD = level-of-detail.

void CD3DEngine::SetLimitLOD(int rank, float limit)
{
	m_limitLOD[rank] = limit;
}

float CD3DEngine::RetLimitLOD(int rank, BOOL bLast)
{
	float	limit;

	if ( bLast )
	{
		limit = m_limitLOD[rank];
		limit *= m_lastDim.x/640.0f;  // limite plus loin si fenêtre grande !
//?		limit += m_limitLOD[0]*(m_lastObjectDetail*2.0f-1.0f);
		limit += m_limitLOD[0]*(m_lastObjectDetail*2.0f);
	}
	else
	{
		limit = m_limitLOD[rank];
		limit *= m_dim.x/640.0f;  // limite plus loin si fenêtre grande !
//?		limit += m_limitLOD[0]*(m_objectDetail*2.0f-1.0f);
		limit += m_limitLOD[0]*(m_objectDetail*2.0f);
	}
	if ( limit < 0.0f )  limit = 0.0f;

	return limit;
}


// Gestion du mode global de marquage au sol.

void CD3DEngine::SetGroundSpot(BOOL bMode)
{
	m_bGroundSpot = bMode;
}

BOOL CD3DEngine::RetGroundSpot()
{
	return m_bGroundSpot;
}


// Indique s'il est possible de donner une couleur dans SetState.

BOOL CD3DEngine::RetStateColor()
{
	return m_bStateColor;
}


// Gestion du mode global de texturage secondaire.

void CD3DEngine::SetSecondTexture(int texNum)
{
	m_secondTexNum = texNum;
}

int CD3DEngine::RetSecondTexture()
{
	return m_secondTexNum;
}


// Choix du rang de la vue active.

void CD3DEngine::SetRankView(int rank)
{
	if ( rank < 0 )  rank = 0;
	if ( rank > 1 )  rank = 1;

	if ( m_rankView == 0 && rank == 1 )  // entre dans l'eau ?
	{
		m_light->AdaptLightColor(m_waterAddColor, +1.0f);
	}

	if ( m_rankView == 1 && rank == 0 )  // sort de l'eau ?
	{
		m_light->AdaptLightColor(m_waterAddColor, -1.0f);
	}

	m_rankView = rank;
}

int CD3DEngine::RetRankView()
{
	return m_rankView;
}

// Indique s'il faut dessiner le monde sous l'interface.

void CD3DEngine::SetDrawWorld(BOOL bDraw)
{
	m_bDrawWorld = bDraw;
}

// Indique s'il faut dessiner le monde sur l'interface.

void CD3DEngine::SetDrawFront(BOOL bDraw)
{
	m_bDrawFront = bDraw;
}

// Gestion de la couleur ambiante.
// color = 0x00rrggbb
//	rr: rouge
//	gg: vert
//	bb: bleu

void CD3DEngine::SetAmbiantColor(D3DCOLOR color, int rank)
{
	m_ambiantColor[rank] = color;
}

D3DCOLOR CD3DEngine::RetAmbiantColor(int rank)
{
	return m_ambiantColor[rank];
}


// Gestion de la couleur sous l'eau.

void CD3DEngine::SetWaterAddColor(D3DCOLORVALUE color)
{
	m_waterAddColor = color;
}

D3DCOLORVALUE CD3DEngine::RetWaterAddColor()
{
	return m_waterAddColor;
}


// Gestion de la couleur du brouillard.

void CD3DEngine::SetFogColor(D3DCOLOR color, int rank)
{
	m_fogColor[rank] = color;
}

D3DCOLOR CD3DEngine::RetFogColor(int rank)
{
	return m_fogColor[rank];
}


// Gestion de la profondeur de champ.
// Au-delà de cette distance, plus rien n'est visible.
// Un peu avant (selon SetFogStart), on entre dans le brouillard.

void CD3DEngine::SetDeepView(float length, int rank, BOOL bRef)
{
	if ( bRef )
	{
		length *= m_clippingDistance;
	}

	m_deepView[rank] = length;
}

float CD3DEngine::RetDeepView(int rank)
{
	return m_deepView[rank];
}


// Gestion du départ de brouillard.
// Avec 0.0, le brouillard part du point de vue (brouillard max).
// Avec 1.0, le brouillard part de la profondeur de champ (pas de brouillard).

void CD3DEngine::SetFogStart(float start, int rank)
{
	m_fogStart[rank] = start;
}

float CD3DEngine::RetFogStart(int rank)
{
	return m_fogStart[rank];
}


// Donne l'image d'arrière-plan à utiliser.

void CD3DEngine::SetBackground(char *name, D3DCOLOR up, D3DCOLOR down,
							   float sunFlash,
							   BOOL bFull, BOOL bQuarter, BOOL bPanel)
{
	strcpy(m_backgroundName, name);
	m_backgroundColorUp   = up;
	m_backgroundColorDown = down;
	m_backgroundSunFlash  = sunFlash;
	m_bBackgroundFull     = bFull;
	m_bBackgroundQuarter  = bQuarter;
	m_bBackgroundPanel    = bPanel;
}

// Donne l'image d'arrière-plan utilisée.

void CD3DEngine::RetBackground(char *name, D3DCOLOR &up, D3DCOLOR &down,
							   float &sunFlash,
							   BOOL &bFull, BOOL &bQuarter)
{
	strcpy(name, m_backgroundName);
	up        = m_backgroundColorUp;
	down      = m_backgroundColorDown;
	sunFlash  = m_backgroundSunFlash;
	bFull     = m_bBackgroundFull;
	bQuarter  = m_bBackgroundQuarter;
}

// Supprime tous les effets de lentille.

void CD3DEngine::LensFlareFlush()
{
	int		i;

	for ( i=0 ; i<m_lensFlareTotal ; i++ )
	{
		FreeTexture(m_lensFlareTable[i].texName);
	}
	m_lensFlareTotal = 0;
}

// Ajoute un effet de lentille.

void CD3DEngine::LensFlareAdd(char *texName, float dist, float dim,
							  float intensity, float offsetH, float offsetV)
{
	if ( m_lensFlareTotal >= MAXLENSFLARE )  return;

	strcpy(m_lensFlareTable[m_lensFlareTotal].texName, texName);
	m_lensFlareTable[m_lensFlareTotal].dist = dist;
	m_lensFlareTable[m_lensFlareTotal].dim = dim;
	m_lensFlareTable[m_lensFlareTotal].intensity = intensity;
	m_lensFlareTable[m_lensFlareTotal].offset.x = offsetH;
	m_lensFlareTable[m_lensFlareTotal].offset.y = offsetV;
	m_lensFlareTotal ++;
}

// Indique s'il faut dessiner d'avant-plan.

void CD3DEngine::SetOverFront(BOOL bFront)
{
	m_bOverFront = bFront;
}

// Donne la couleur d'avant-plan.

void CD3DEngine::SetOverColor(D3DCOLOR color, int mode)
{
	m_overColor = color;
	m_overMode  = mode;
}



// Initialise les réglages par défaut.

void CD3DEngine::DefaultSetup()
{
	int		i;

	for ( i=0 ; i<100 ; i++ )
	{
		m_setup[i] = 0.0f;
	}

	m_setup[ST_SHADOW]      = 1.0f;
	m_setup[ST_DIRTY]       = 1.0f;
	m_setup[ST_SUNBEAM]     = 1.0f;
	m_setup[ST_LENSFLARE]   = 1.0f;
	m_setup[ST_DECOR]       = 1.0f;
	m_setup[ST_DETAIL]      = 1.0f;
	m_setup[ST_METEO]       = 1.0f;
	m_setup[ST_AMBIANCE]    = 1.0f;
	m_setup[ST_EXPLOVIB]    = 1.0f;
	m_setup[ST_SPEEDSCH]    = 0.5f;
	m_setup[ST_SPEEDSCV]    = 0.5f;
	m_setup[ST_MOUSESCROLL] = 1.0f;
	m_setup[ST_MOVIE]       = 1.0f;
	m_setup[ST_HELP]        = 1.0f;
	m_setup[ST_TOOLTIPS]    = 1.0f;
	m_setup[ST_NICEMOUSE]   = 1.0f;
	m_setup[ST_ACCEL]       = 1.0f;
	m_setup[ST_VOLBLUPI]    = 0.8f;
	m_setup[ST_VOLSOUND]    = 0.8f;
	m_setup[ST_VOLAMBIANCE] = 0.8f;
	m_setup[ST_SOUND3D]     = 1.0f;
}

// Donne le nom d'un réglage.

BOOL CD3DEngine::RetSetupName(SetupType type, char *buffer)
{
	switch ( type )
	{
		case ST_SHADOW:       strcpy(buffer, "Shadow");        return TRUE;
		case ST_DIRTY:        strcpy(buffer, "Dirty");         return TRUE;
		case ST_SUNBEAM:      strcpy(buffer, "Sunbeam");       return TRUE;
		case ST_LENSFLARE:    strcpy(buffer, "LensFlare");     return TRUE;
		case ST_DECOR:        strcpy(buffer, "ObjectDecor");   return TRUE;
		case ST_DETAIL:       strcpy(buffer, "ObjectDetail");  return TRUE;
		case ST_METEO:        strcpy(buffer, "Meteo");         return TRUE;
		case ST_AMBIANCE:     strcpy(buffer, "Ambiance");      return TRUE;
		case ST_EXPLOVIB:     strcpy(buffer, "ExploVib");      return TRUE;
		case ST_SPEEDSCH:     strcpy(buffer, "SpeedScrollH");  return TRUE;
		case ST_SPEEDSCV:     strcpy(buffer, "SpeedScrollV");  return TRUE;
		case ST_MOUSESCROLL:  strcpy(buffer, "MouseScroll");   return TRUE;
		case ST_INVSCH:       strcpy(buffer, "InvScrollH");    return TRUE;
		case ST_INVSCV:       strcpy(buffer, "InvSCrollV");    return TRUE;
		case ST_MOVIE:        strcpy(buffer, "Movie");         return TRUE;
		case ST_HELP:         strcpy(buffer, "Help");          return TRUE;
		case ST_TOOLTIPS:     strcpy(buffer, "Tooltips");      return TRUE;
		case ST_NICEMOUSE:    strcpy(buffer, "NiceMouse");     return TRUE;
		case ST_ACCEL:        strcpy(buffer, "Accel");         return TRUE;
		case ST_VOLBLUPI:     strcpy(buffer, "VolBlupi");      return TRUE;
		case ST_VOLSOUND:     strcpy(buffer, "VolSound");      return TRUE;
		case ST_VOLAMBIANCE:  strcpy(buffer, "VolAmbiance");   return TRUE;
		case ST_SOUND3D:      strcpy(buffer, "Sound3D");       return TRUE;
	}
	return FALSE;
}

// Modifie un réglage.

void CD3DEngine::SetSetup(SetupType type, float value)
{
	if ( type == ST_NICEMOUSE )
	{
		if ( !m_app->RetNiceMouseCap() )  value = 0.0f;
//-		if ( value == m_setup[type] )  return;

		if ( value == 0.0f )
		{
			ShowCursor(TRUE);  // montre la vilaine souris windows
			SetCursor(LoadCursor(NULL, IDC_ARROW));
		}
		else
		{
			ShowCursor(FALSE);  // cache la vilaine souris windows
			SetCursor(NULL);
		}
	}

	m_setup[type] = value;

	if ( type == ST_SOUND3D )
	{
		if ( m_sound == 0 )
		{
			m_sound = (CSound*)m_iMan->SearchInstance(CLASS_SOUND);
		}
		if ( m_sound != 0 )
		{
			m_sound->SetSound3D(value!=0.0f);
		}
	}
}

// Donne un réglage.

float CD3DEngine::RetSetup(SetupType type)
{
	if ( type == ST_NICEMOUSE )
	{
		if ( !m_app->RetNiceMouseCap() )  return 0.0f;
	}

	return m_setup[type];
}


// Gestion du mode d'arrière-plan.

void CD3DEngine::SetBackForce(BOOL bPresent)
{
	m_bBackForce = bPresent;
}

BOOL CD3DEngine::RetBackForce()
{
	return m_bBackForce;
}


// Gestion du mode d'indentation pendant l'édition (CEdit).

void CD3DEngine::SetEditIndentMode(BOOL bAuto)
{
	m_bEditIndentMode = bAuto;
}

BOOL CD3DEngine::RetEditIndentMode()
{
	return m_bEditIndentMode;
}


// Gestion de l'avance d'un tabulateur pendant l'édition (CEdit).

void CD3DEngine::SetEditIndentValue(int value)
{
	m_editIndentValue = value;
}

int CD3DEngine::RetEditIndentValue()
{
	return m_editIndentValue;
}


// Gestion de la vitesse du jeu.

void CD3DEngine::SetSpeed(float speed)
{
	m_speed = speed;
}

float CD3DEngine::RetSpeed()
{
	return m_speed;
}


// Met à jour la scène après un changement de paramètres.

void CD3DEngine::ApplyChange()
{
	m_deepView[0] /= m_lastClippingDistance;
	m_deepView[1] /= m_lastClippingDistance;

	SetFocus(m_focus);
	ChangeLOD();

	m_deepView[0] *= m_clippingDistance;
	m_deepView[1] *= m_clippingDistance;
}



// Retourne le point de vue de l'utilisateur.

D3DVECTOR CD3DEngine::RetEyePt()
{
	return m_eyePt;
}

D3DVECTOR CD3DEngine::RetLookatPt()
{
	return m_lookatPt;
}

float CD3DEngine::RetEyeDirH()
{
	return m_eyeDirH;
}

float CD3DEngine::RetEyeDirV()
{
	return m_eyeDirV;
}

POINT CD3DEngine::RetDim()
{
	return m_dim;
}


// Génère un nom de quart d'image.

void QuarterName(char *buffer, char *name, int quarter)
{
	while ( *name != 0 )
	{
		if ( *name == '.' )
		{
			*buffer++ = 'a'+quarter;
		}
		*buffer++ = *name++;
	}
	*buffer++ = 0;
}

// Libère une texture.

BOOL CD3DEngine::FreeTexture(char* name)
{
	if ( name[0] == 0 )  return TRUE;

	if ( D3DTextr_DestroyTexture(name) != S_OK )
	{
		return FALSE;
	}
	return TRUE;
}

// Charge une texture.

BOOL CD3DEngine::LoadTexture(char* name, int stage)
{
	DWORD	mode;

	if ( name[0] == 0 )  return TRUE;

	if ( D3DTextr_GetSurface(name) == NULL )
	{
		if ( strstr(name, ".tga") == 0 )
		{
			mode = 0;
		}
		else
		{
			mode = D3DTEXTR_CREATEWITHALPHA;
		}

		if ( D3DTextr_CreateTextureFromFile(name, stage, mode) != S_OK )
		{
			return FALSE;
		}

		if ( D3DTextr_Restore(name, m_pD3DDevice) != S_OK )
		{
			return FALSE;
		}
	}
	return TRUE;
}

// Charge toutes les textures de la scène.

BOOL CD3DEngine::LoadAllTexture()
{
	D3DObjLevel1*	p1;
	D3DObjLevel2a*	p2a;
	D3DObjLevel2b*	p2b;
	int				l1, l2a, i;
	char			name[50];
	BOOL			bOK = TRUE;

	LoadTexture("text.tga");
	LoadTexture("button1.tga");
	LoadTexture("button2.tga");
	LoadTexture("button3.tga");
	LoadTexture("effect00.tga");
	LoadTexture("effect01.tga");
	LoadTexture("effect02.tga");
	LoadTexture("effect03.tga");
	LoadTexture("map.tga");

	if ( m_backgroundName[0] != 0 )
	{
		if ( m_bBackgroundQuarter )  // image en 4 morceaux ?
		{
			for ( i=0 ; i<4 ; i++ )
			{
				QuarterName(name, m_backgroundName, i);
				LoadTexture(name);
			}
		}
		else
		{
			LoadTexture(m_backgroundName);
		}
	}

	if ( m_setup[ST_LENSFLARE] != 0 )
	{
		for ( i=0 ; i<m_lensFlareTotal ; i++ )
		{
			LoadTexture(m_lensFlareTable[i].texName);
		}
	}

	m_planet->LoadTexture();

	p1 = m_objectPointer;
	for ( l1=0 ; l1<p1->totalUsed ; l1++ )
	{
		p2a = p1->table[l1];
		for ( l2a=0 ; l2a<p2a->totalUsed ; l2a++ )
		{
			p2b = p2a->table[l2a];

			if ( p2b == 0 || p2b->texName1[0] != 0 )
			{
				if ( !LoadTexture(p2b->texName1, 0) )  bOK = FALSE;
			}

			if ( p2b == 0 || p2b->texName2[0] != 0 )
			{
				if ( !LoadTexture(p2b->texName2, 1) )  bOK = FALSE;
			}
		}
	}
	return bOK;
}


// Called during initial app startup, this function performs all the
// permanent initialization.

HRESULT CD3DEngine::OneTimeSceneInit()
{
    return S_OK;
}


// Mise à jour après avoir créé des objets.

void CD3DEngine::Update()
{
	ComputeDistance();
	UpdateGeometry();
}

// Called once per frame, the call is the entry point for animating
// the scene.

HRESULT CD3DEngine::FrameMove(float rTime)
{
	m_light->FrameLight(rTime);
	m_particule->SetFrameUpdate(SH_WORLD, !m_bPause);
	m_particule->FrameParticule(rTime);
	FrameShadow(rTime);
	ComputeDistance();
	UpdateGeometry();

	if ( m_groundMark.bUsed )
	{
		if ( m_groundMark.phase == 1 )  // croissance ?
		{
			m_groundMark.intensity += rTime*(1.0f/m_groundMark.delay[0]);
			if ( m_groundMark.intensity >= 1.0f )
			{
				m_groundMark.intensity = 1.0f;
				m_groundMark.fix = 0.0f;
				m_groundMark.phase = 2;
			}
		}
		else if ( m_groundMark.phase == 2 )  // fixe ?
		{
			m_groundMark.fix += rTime*(1.0f/m_groundMark.delay[1]);
			if ( m_groundMark.fix >= 1.0f )
			{
				m_groundMark.phase = 3;
			}
		}
		else if ( m_groundMark.phase == 3 )  // décroissance ?
		{
			m_groundMark.intensity -= rTime*(1.0f/m_groundMark.delay[2]);
			if ( m_groundMark.intensity < 0.0f )
			{
				m_groundMark.intensity = 0.0f;
				m_groundMark.phase     = 0;
				m_groundMark.bUsed     = FALSE;
			}
		}
	}

	if ( m_sound == 0 )
	{
		m_sound = (CSound*)m_iMan->SearchInstance(CLASS_SOUND);
	}
	m_sound->FrameMove(rTime);

	return S_OK;
}

// Fait évoluer tout le jeu.

void CD3DEngine::StepSimul(float rTime)
{
	m_app->StepSimul(rTime);
}



// Modifie l'état associé à un matériaux.
// (*) Ne fonctionne pas sans cette instruction, mystère !

void CD3DEngine::SetState(int state, D3DCOLOR color)
{
	BOOL	bSecond;

	if ( state == m_lastState &&
		 color == m_lastColor )  return;
	m_lastState = state;
	m_lastColor = color;

	if ( m_alphaMode != 1 && (state & D3DSTATEALPHA) )
	{
		state &= ~D3DSTATEALPHA;

		if ( m_alphaMode == 2 )
		{
			state |= D3DSTATETTb;
		}
	}

	if ( state & D3DSTATETTb )  // transparent selon noir de la texture ?
	{
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, FALSE);
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, FALSE);
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,  m_blackSrcBlend[1]);
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, m_blackDestBlend[1]);
//?		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,  table_blend[debug_blend1]);
//?		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, table_blend[debug_blend2]);

		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_TEXTUREFACTOR, color);
		m_pD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_MODULATE);
		m_pD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		m_pD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TEXTURE);  // (*)
		m_pD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TFACTOR);
		m_pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_DISABLE);
	}
	else if ( state & D3DSTATETTw )  // transparent selon blanc de la texture ?
	{
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, FALSE);
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, FALSE);
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,  m_whiteSrcBlend[1]);
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, m_whiteDestBlend[1]);
//?		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,  table_blend[debug_blend3]);
//?		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, table_blend[debug_blend4]);

		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_TEXTUREFACTOR, ~color);
		m_pD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_ADD);
		m_pD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		m_pD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TEXTURE);  // (*)
		m_pD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TFACTOR);
		m_pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_DISABLE);
	}
	else if ( state & D3DSTATETCb )  // transparent selon noir de la couleur ?
	{
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, FALSE);
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, FALSE);
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,  m_blackSrcBlend[1]);
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, m_blackDestBlend[1]);
//?		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,  table_blend[debug_blend1]);
//?		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, table_blend[debug_blend2]);

		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_TEXTUREFACTOR, color);
		m_pD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_DISABLE);
		m_pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_DISABLE);
	}
	else if ( state & D3DSTATETCw )  // transparent selon blanc de la couleur ?
	{
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, FALSE);
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, FALSE);
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,  m_whiteSrcBlend[1]);
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, m_whiteDestBlend[1]);
//?		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,  table_blend[debug_blend3]);
//?		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, table_blend[debug_blend4]);

		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_TEXTUREFACTOR, ~color);
		m_pD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_DISABLE);
		m_pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_DISABLE);
	}
	else if ( state & D3DSTATETD )  // transparent selon couleur diffuse ?
	{
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, FALSE);
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, FALSE);
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,  m_diffuseSrcBlend[1]);
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, m_diffuseDestBlend[1]);

		m_pD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1);
		m_pD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		m_pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_DISABLE);
	}
	else if ( state & D3DSTATEALPHA )  // image avec canal alpha ?
	{
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, TRUE);
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE);
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, TRUE);
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_ALPHAFUNC, D3DCMP_GREATER);
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_ALPHAREF,  (DWORD)(128));
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,  m_alphaSrcBlend[1]);
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, m_alphaSrcBlend[1]);

		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_TEXTUREFACTOR, color);
		m_pD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_MODULATE);
		m_pD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		m_pD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
		m_pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1);
		m_pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	}
	else	// normal ?
	{
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, TRUE);
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE);
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, FALSE);

		m_pD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_MODULATE);
		m_pD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		m_pD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
		m_pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_DISABLE);
	}

	if ( state & D3DSTATEFOG )
	{
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, TRUE);
	}

	bSecond = m_bGroundSpot | (m_setup[ST_DIRTY]!=0);
	if ( !m_bGroundSpot && (state & D3DSTATESECOND) != 0 )  bSecond = FALSE;
	if ( (m_setup[ST_DIRTY]==0) && (state & D3DSTATESECOND) == 0 )  bSecond = FALSE;

	if ( (state & D3DSTATEDUALb) && bSecond )
	{
		m_pD3DDevice->SetTextureStageState(1, D3DTSS_COLOROP,   D3DTOP_MODULATE);
		m_pD3DDevice->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		m_pD3DDevice->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
		m_pD3DDevice->SetTextureStageState(1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE);
		m_pD3DDevice->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1);
	}
	else if ( (state & D3DSTATEDUALw) && bSecond )
	{
		m_pD3DDevice->SetTextureStageState(1, D3DTSS_COLOROP,   D3DTOP_ADD);
		m_pD3DDevice->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		m_pD3DDevice->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
		m_pD3DDevice->SetTextureStageState(1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE);
		m_pD3DDevice->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1);
	}
	else
	{
		m_pD3DDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
		m_pD3DDevice->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	}

	if ( state & D3DSTATEWRAP )
	{
//?		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_WRAP0, D3DWRAP_U|D3DWRAP_V);
		m_pD3DDevice->SetTextureStageState(0, D3DTSS_ADDRESS, D3DTADDRESS_WRAP);
		m_pD3DDevice->SetTextureStageState(1, D3DTSS_ADDRESS, D3DTADDRESS_WRAP);
	}
	else if ( state & D3DSTATECLAMP )
	{
//?		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_WRAP0, 0);
		m_pD3DDevice->SetTextureStageState(0, D3DTSS_ADDRESS, D3DTADDRESS_CLAMP);
		m_pD3DDevice->SetTextureStageState(1, D3DTSS_ADDRESS, D3DTADDRESS_CLAMP);
	}
	else
	{
//?		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_WRAP0, 0);
		m_pD3DDevice->SetTextureStageState(0, D3DTSS_ADDRESS, D3DTADDRESS_CLAMP);
		m_pD3DDevice->SetTextureStageState(1, D3DTSS_ADDRESS, D3DTADDRESS_CLAMP);
	}

	if ( state & D3DSTATE2FACE )
	{
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE);
	}
	else
	{
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_CCW);
	}

	if ( state & D3DSTATELIGHT )
	{
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_AMBIENT, 0xffffffff);
	}
	else
	{
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_AMBIENT, m_ambiantColor[m_rankView]);
	}
}

// Spécifie une texture à utiliser.

void CD3DEngine::SetTexture(char *name, int stage)
{
	if ( strcmp(name, m_lastTexture[stage]) == 0 )  return;
	strcpy(m_lastTexture[stage], name);

	m_pD3DDevice->SetTexture(stage, D3DTextr_GetSurface(name));
}

// Spécifie le matérial à utiliser.

void CD3DEngine::SetMaterial(const D3DMATERIAL7 &mat)
{
	if ( memcmp(&mat, &m_lastMaterial, sizeof(D3DMATERIAL7)) == 0 )  return;
	m_lastMaterial = mat;

	m_pD3DDevice->SetMaterial(&m_lastMaterial);
}


// Efface un point dans une surface (dessine en blanc).

inline void ClearDot(DDSURFACEDESC2* ddsd, int x, int y)
{
	WORD*		pbSurf;

	if ( ddsd->ddpfPixelFormat.dwRGBBitCount != 16 )  return;

	pbSurf = (WORD*)ddsd->lpSurface;
	pbSurf += ddsd->lPitch*y/2;
	pbSurf += x;

	*pbSurf = 0xffff;  // blanc
}

// Affiche un point dans une surface.

void AddDot(DDSURFACEDESC2* ddsd, int x, int y, D3DCOLORVALUE color)
{
	WORD*		pbSurf;
	WORD		r,g,b, w;

	if ( ddsd->ddpfPixelFormat.dwRGBBitCount != 16 )  return;

	if ( color.r < 0.0f )  color.r = 0.0f;
	if ( color.r > 1.0f )  color.r = 1.0f;
	r = (int)(color.r*32.0f);
	if ( r >= 32 )  r = 31;  // 5 bits

	if ( color.g < 0.0f )  color.g = 0.0f;
	if ( color.g > 1.0f )  color.g = 1.0f;
	g = (int)(color.g*32.0f);
	if ( g >= 32 )  g = 31;  // 5 bits

	if ( color.b < 0.0f )  color.b = 0.0f;
	if ( color.b > 1.0f )  color.b = 1.0f;
	b = (int)(color.b*32.0f);
	if ( b >= 32 )  b = 31;  // 5 bits

	if ( ddsd->ddpfPixelFormat.dwRBitMask == 0xf800 )  // 565 ?
	{
		w = (r<<11)|(g<<6)|b;
	}
	else if ( ddsd->ddpfPixelFormat.dwRBitMask == 0x7c00 )  // 555 ?
	{
		w = (r<<10)|(g<<5)|b;
	}
	else
	{
		w = -1;  // blanc
	}

	pbSurf = (WORD*)ddsd->lpSurface;
	pbSurf += ddsd->lPitch*y/2;
	pbSurf += x;

	*pbSurf &= w;
}

// Affiche un point dans une surface.

void SetDot(DDSURFACEDESC2* ddsd, int x, int y, D3DCOLORVALUE color)
{
	if ( ddsd->ddpfPixelFormat.dwRGBBitCount == 16 )
	{
		WORD*		pbSurf;
		WORD		r,g,b, w;

		if ( color.r < 0.0f )  color.r = 0.0f;
		if ( color.r > 1.0f )  color.r = 1.0f;
		if ( color.g < 0.0f )  color.g = 0.0f;
		if ( color.g > 1.0f )  color.g = 1.0f;
		if ( color.b < 0.0f )  color.b = 0.0f;
		if ( color.b > 1.0f )  color.b = 1.0f;

		r = (int)(color.r*32.0f);
		g = (int)(color.g*32.0f);
		b = (int)(color.b*32.0f);

		if ( r >= 32 )  r = 31;  // 5 bits
		if ( g >= 32 )  g = 31;  // 5 bits
		if ( b >= 32 )  b = 31;  // 5 bits

		if ( ddsd->ddpfPixelFormat.dwRBitMask == 0xf800 )  // 565 ?
		{
			w = (r<<11)|(g<<6)|b;
		}
		else if ( ddsd->ddpfPixelFormat.dwRBitMask == 0x7c00 )  // 555 ?
		{
			w = (r<<10)|(g<<5)|b;
		}
		else
		{
			w = -1;  // blanc
		}

		pbSurf = (WORD*)ddsd->lpSurface;
		pbSurf += ddsd->lPitch*y/2;
		pbSurf += x;

		*pbSurf = w;
	}

	if ( ddsd->ddpfPixelFormat.dwRGBBitCount == 32 )  // image .tga ?
	{
		LONG*		pbSurf;
		LONG		r,g,b, w;

		if ( color.r < 0.0f )  color.r = 0.0f;
		if ( color.r > 1.0f )  color.r = 1.0f;
		if ( color.g < 0.0f )  color.g = 0.0f;
		if ( color.g > 1.0f )  color.g = 1.0f;
		if ( color.b < 0.0f )  color.b = 0.0f;
		if ( color.b > 1.0f )  color.b = 1.0f;

		r = (int)(color.r*256.0f);
		g = (int)(color.g*256.0f);
		b = (int)(color.b*256.0f);

		if ( r >= 256 )  r = 255;  // 8 bits
		if ( g >= 256 )  g = 255;  // 8 bits
		if ( b >= 256 )  b = 255;  // 8 bits

		if ( ddsd->ddpfPixelFormat.dwRBitMask == 0xff0000 )
		{
			w = (r<<16)|(g<<8)|b;

			pbSurf = (LONG*)ddsd->lpSurface;
			pbSurf += ddsd->lPitch*y/4;
			pbSurf += x;

			*pbSurf &= 0xff000000;  // garde canal alpha
			*pbSurf |= w;
		}
	}
}

// Donne un point dans une surface.

D3DCOLORVALUE GetDot(DDSURFACEDESC2* ddsd, int x, int y)
{
	D3DCOLORVALUE	color;

	if ( ddsd->ddpfPixelFormat.dwRGBBitCount == 16 )
	{
		WORD*		pbSurf;
		WORD		r,g,b, w;

		pbSurf = (WORD*)ddsd->lpSurface;
		pbSurf += ddsd->lPitch*y/2;
		pbSurf += x;

		w = *pbSurf;

		if ( ddsd->ddpfPixelFormat.dwRBitMask == 0xf800 )  // 565 ?
		{
			r = (w>>10)&0x003e;
			g = (w>> 5)&0x003f;
			b = (w<< 1)&0x003e;
		}
		else if ( ddsd->ddpfPixelFormat.dwRBitMask == 0x7c00 )  // 555 ?
		{
			r = (w>> 9)&0x003e;
			g = (w>> 4)&0x003e;
			b = (w<< 1)&0x003e;
		}
		else
		{
			r = 0;
			g = 0;
			b = 0;  // noir
		}

		color.r = (float)r/63.0f;
		color.g = (float)g/63.0f;
		color.b = (float)b/63.0f;
		color.a = 0.0f;
		return color;
	}

	if ( ddsd->ddpfPixelFormat.dwRGBBitCount == 32 )  // image .tga ?
	{
		LONG*		pbSurf;
		LONG		r,g,b, w;

		pbSurf = (LONG*)ddsd->lpSurface;
		pbSurf += ddsd->lPitch*y/4;
		pbSurf += x;

		w = *pbSurf;

		if ( ddsd->ddpfPixelFormat.dwRBitMask == 0xff0000 )
		{
			r = (w>>16)&0x00ff;
			g = (w>> 8)&0x00ff;
			b = (w<< 0)&0x00ff;
		}
		else
		{
			r = 0;
			g = 0;
			b = 0;  // noir
		}

		color.r = (float)r/255.0f;
		color.g = (float)g/255.0f;
		color.b = (float)b/255.0f;
		color.a = 0.0f;
		return color;
	}

	color.r = 0.0f;
	color.g = 0.0f;
	color.b = 0.0f;
	color.a = 0.0f;  // noir
	return color;
}

// Dessine toutes les ombres.

// Il y a 1 pixel de recouvrement autour de chacune des 16 surfaces :
//
//	    |<----------------------->|<----------------------->|<---- ...
//	  0 | 1   2            253 254|255                      |
//	|---|---|---|-- ... --|---|---|---|                     |
//	                            0 | 1   2            253 254|255
//	                          |---|---|---|-- ... --|---|---|---|
//
// On dessine donc dans des surfaces de 254x254 pixels. Le pixel de
// marge tout autour est dessiné à double (dans 2 surfaces adjacentes),
// pour que le filtrage produise des résultats identiques !

void CD3DEngine::RenderGroundSpot()
{
	LPDIRECTDRAWSURFACE7	surface;
	DDSURFACEDESC2			ddsd;
	WORD*					pbSurf;
	D3DCOLORVALUE			color;
	D3DVECTOR				pos;
	FPOINT					min, max;
	int						s, i, j, dot, ix, iy, y;
	float					tu, tv, cx, cy, px, py, ppx, ppy;
	float					intensity, level;
	char					texName[20];
	BOOL					bClear, bSet;

	if ( !m_bFirstGroundSpot                                  &&
		 m_groundMark.drawPos.x     == m_groundMark.pos.x     &&
		 m_groundMark.drawPos.z     == m_groundMark.pos.z     &&
		 m_groundMark.drawRadius    == m_groundMark.radius    &&
		 m_groundMark.drawIntensity == m_groundMark.intensity )  return;
	
	for ( s=0 ; s<16 ; s++ )
	{
		min.x = (s%4)*254.0f-1.0f;  // 1 pixel de recouvrement
		min.y = (s/4)*254.0f-1.0f;
		max.x = min.x+254.0f+2.0f;
		max.y = min.y+254.0f+2.0f;

		bClear = FALSE;
		bSet   = FALSE;

		// Calcule la zone à effacer.
		dot = (int)(m_groundMark.drawRadius/2.0f);

		tu = (m_groundMark.drawPos.x+1600.0f)/3200.0f;
		tv = (m_groundMark.drawPos.z+1600.0f)/3200.0f;  // 0..1

		cx = (tu*254.0f*4.0f)-0.5f;
		cy = (tv*254.0f*4.0f)-0.5f;

		if ( dot == 0 )
		{
			cx += 0.5f;
			cy += 0.5f;
		}

		px = cx-Mod(cx, 1.0f);
		py = cy-Mod(cy, 1.0f);  // multiple de 1

		if ( m_bFirstGroundSpot ||
			 ( m_groundMark.drawRadius != 0.0f    &&
			   px+dot >= min.x && py+dot >= min.y &&
			   px-dot <= max.x && py-dot <= max.y ) )
		{
			bClear = TRUE;
		}

		// Calcule la zone à dessiner.
		dot = (int)(m_groundMark.radius/2.0f);

		tu = (m_groundMark.pos.x+1600.0f)/3200.0f;
		tv = (m_groundMark.pos.z+1600.0f)/3200.0f;  // 0..1

		cx = (tu*254.0f*4.0f)-0.5f;
		cy = (tv*254.0f*4.0f)-0.5f;

		if ( dot == 0 )
		{
			cx += 0.5f;
			cy += 0.5f;
		}

		px = cx-Mod(cx, 1.0f);
		py = cy-Mod(cy, 1.0f);  // multiple de 1

		if ( m_groundMark.bUsed                 &&
			 px+dot >= min.x && py+dot >= min.y &&
			 px-dot <= max.x && py-dot <= max.y )
		{
			bSet = TRUE;
		}
		
		if ( bClear || bSet )
		{
			// Charge le morceau.
			sprintf(texName, "shadow%.2d.tga", s);
			surface = D3DTextr_GetSurface(texName);
			if ( surface == 0 )  continue;

			ZeroMemory(&ddsd, sizeof(DDSURFACEDESC2));
			ddsd.dwSize = sizeof(DDSURFACEDESC2);
			if ( surface->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL) != DD_OK )  continue;

			// Efface en blanc tout le morceau.
			if ( ddsd.ddpfPixelFormat.dwRGBBitCount == 16 )
			{
				for ( y=0 ; y<(int)ddsd.dwHeight ; y++ )
				{
					pbSurf = (WORD*)ddsd.lpSurface;
					pbSurf += ddsd.lPitch*y/2;
					memset(pbSurf, -1, ddsd.lPitch);  // tout blanc
				}
			}

			// Dessine les nouvelles ombres.
			for ( i=0 ; i<D3DMAXGROUNDSPOT ; i++ )
			{
				if ( m_groundSpot[i].bUsed == FALSE ||
					 m_groundSpot[i].radius == 0.0f )  continue;

				if ( m_groundSpot[i].min == 0.0f &&
					 m_groundSpot[i].max == 0.0f )
				{
					dot = (int)(m_groundSpot[i].radius/2.0f);

					tu = (m_groundSpot[i].pos.x+1600.0f)/3200.0f;
					tv = (m_groundSpot[i].pos.z+1600.0f)/3200.0f;  // 0..1

					cx = (tu*254.0f*4.0f)-0.5f;
					cy = (tv*254.0f*4.0f)-0.5f;

					if ( dot == 0 )
					{
						cx += 0.5f;
						cy += 0.5f;
					}

					px = cx-Mod(cx, 1.0f);
					py = cy-Mod(cy, 1.0f);  // multiple de 1

					if ( px+dot < min.x || py+dot < min.y ||
						 px-dot > max.x || py-dot > max.y )  continue;

					for ( iy=-dot ; iy<=dot ; iy++ )
					{
						for ( ix=-dot ; ix<=dot ; ix++ )
						{
							ppx = px+ix;
							ppy = py+iy;

							if ( ppx <  min.x || ppy <  min.y ||
								 ppx >= max.x || ppy >= max.y )  continue;

							if ( dot == 0 )
							{
								intensity = 0.0f;
							}
							else
							{
								intensity = Length(ppx-cx, ppy-cy)/dot;
	//?							intensity = powf(intensity, m_groundSpot[i].smooth);
							}

							color.r = m_groundSpot[i].color.r+intensity;
							color.g = m_groundSpot[i].color.g+intensity;
							color.b = m_groundSpot[i].color.b+intensity;

							ppx -= min.x;  // relatif à la texture
							ppy -= min.y;
							AddDot(&ddsd, (int)ppx, (int)ppy, color);
						}
					}
				}
				else
				{
					for ( iy=0 ; iy<256 ; iy++ )
					{
						for ( ix=0 ; ix<256 ; ix++ )
						{
							pos.x = (256.0f*(s%4)+ix)*3200.0f/1024.0f - 1600.0f;
							pos.z = (256.0f*(s/4)+iy)*3200.0f/1024.0f - 1600.0f;
							pos.y = 0.0f;
							level = m_terrain->RetFloorLevel(pos);
							if ( level < m_groundSpot[i].min ||
								 level > m_groundSpot[i].max )  continue;

							if ( level > (m_groundSpot[i].max+m_groundSpot[i].min)/2.0f )
							{
								intensity = 1.0f-(m_groundSpot[i].max-level)/m_groundSpot[i].smooth;
							}
							else
							{
								intensity = 1.0f-(level-m_groundSpot[i].min)/m_groundSpot[i].smooth;
							}
							if ( intensity < 0.0f )  intensity = 0.0f;

							color.r = m_groundSpot[i].color.r+intensity;
							color.g = m_groundSpot[i].color.g+intensity;
							color.b = m_groundSpot[i].color.b+intensity;

							AddDot(&ddsd, ix, iy, color);
						}
					}
				}
			}

			if ( bSet )
			{
				dot = (int)(m_groundMark.radius/2.0f);

				tu = (m_groundMark.pos.x+1600.0f)/3200.0f;
				tv = (m_groundMark.pos.z+1600.0f)/3200.0f;  // 0..1

				cx = (tu*254.0f*4.0f)-0.5f;
				cy = (tv*254.0f*4.0f)-0.5f;

				if ( dot == 0 )
				{
					cx += 0.5f;
					cy += 0.5f;
				}

				px = cx-Mod(cx, 1.0f);
				py = cy-Mod(cy, 1.0f);  // multiple de 1

				for ( iy=-dot ; iy<=dot ; iy++ )
				{
					for ( ix=-dot ; ix<=dot ; ix++ )
					{
						ppx = px+ix;
						ppy = py+iy;

						if ( ppx <  min.x || ppy <  min.y ||
							 ppx >= max.x || ppy >= max.y )  continue;

						ppx -= min.x;  // relatif à la texture
						ppy -= min.y;

						intensity = 1.0f-Length((float)ix, (float)iy)/dot;
						if ( intensity <= 0.0f )  continue;
						intensity *= m_groundMark.intensity;

						j = (ix+dot) + (iy+dot)*m_groundMark.dx;
						if ( m_groundMark.table[j] == 1 )  // vert ?
						{
							color.r = 1.0f-intensity;
							color.g = 1.0f;
							color.b = 1.0f-intensity;
							AddDot(&ddsd, (int)ppx, (int)ppy, color);
						}
						if ( m_groundMark.table[j] == 2 )  // rouge ?
						{
							color.r = 1.0f;
							color.g = 1.0f-intensity;
							color.b = 1.0f-intensity;
							AddDot(&ddsd, (int)ppx, (int)ppy, color);
						}
					}
				}
			}

			surface->Unlock(NULL);
		}
	}

	for ( i=0 ; i<D3DMAXGROUNDSPOT ; i++ )
	{
		if ( m_groundSpot[i].bUsed == FALSE ||
			 m_groundSpot[i].radius == 0.0f )
		{
			m_groundSpot[i].drawRadius = 0.0f;
		}
		else
		{
			m_groundSpot[i].drawPos    = m_groundSpot[i].pos;
			m_groundSpot[i].drawRadius = m_groundSpot[i].radius;
		}
	}

	m_groundMark.drawPos       = m_groundMark.pos;
	m_groundMark.drawRadius    = m_groundMark.radius;
	m_groundMark.drawIntensity = m_groundMark.intensity;

	m_bFirstGroundSpot = FALSE;
}

// Fait évoluer les ombres.

void CD3DEngine::FrameShadow(float rTime)
{
	int		i;

	if ( m_bPause )  return;

	for ( i=0 ; i<m_shadowTotal ; i++ )
	{
		if ( !m_shadow[i].bUsed )  continue;
		if ( m_shadow[i].bHide )  continue;

		m_shadow[i].time += rTime;
	}
}

// Dessine un morceau rectangulaire d'ombre.

void CD3DEngine::DrawShadowPiece(int i,
								 const D3DVECTOR &pos,
								 const D3DVECTOR &shift,
								 float adjust,
								 float minx, float maxx,
								 float minz, float maxz,
								 FPOINT ti, FPOINT ts)
{
	D3DVERTEX2	vertex[4];	// 2 triangles
	D3DVECTOR	corner[4];
	FPOINT		rot, dim;
	float		radius, rmx, rpx, rmz, rpz, limit, dp;

	radius = rmx = rpx = rmz = rpz = m_shadow[i].radius*1.5f;

	limit = maxx-(m_shadow[i].pos.x+shift.x);
	if ( rpx > limit )  rpx = limit;

	limit = (m_shadow[i].pos.x+shift.x)-minx;
	if ( rmx > limit )  rmx = limit;

	limit = maxz-(m_shadow[i].pos.z+shift.z);
	if ( rpz > limit )  rpz = limit;

	limit = (m_shadow[i].pos.z+shift.z)-minz;
	if ( rmz > limit )  rmz = limit;

	rmx    *= adjust;
	rpx    *= adjust;
	rmz    *= adjust;
	rpz    *= adjust;
	radius *= adjust;

	corner[0].x = +rpx;
	corner[0].z = +rpz;
	corner[0].y = 0.0f;

	corner[1].x = -rmx;
	corner[1].z = +rpz;
	corner[1].y = 0.0f;

	corner[2].x = +rpx;
	corner[2].z = -rmz;
	corner[2].y = 0.0f;

	corner[3].x = -rmx;
	corner[3].z = -rmz;
	corner[3].y = 0.0f;

	if ( corner[0].x <= corner[1].x ||
		 corner[0].z <= corner[2].z )  return;

	dim.x = (ts.x-ti.x)/2.0f;
	dim.y = (ts.y-ti.y)/2.0f;
	ti.x += ((radius-rmx)/radius) * dim.x;
	ts.x -= ((radius-rpx)/radius) * dim.x;
	ti.y += ((radius-rmz)/radius) * dim.y;
	ts.y -= ((radius-rpz)/radius) * dim.y;

	corner[0] += pos;
	corner[1] += pos;
	corner[2] += pos;
	corner[3] += pos;

	dp = 0.5f/256.0f;
	ti.x += dp;
	ti.y += dp;
	ts.x -= dp;
	ts.y -= dp;

	vertex[0] = D3DVERTEX2(corner[1], D3DVECTOR(0.0f, 1.0f, 0.0f), ti.x, ts.y);
	vertex[1] = D3DVERTEX2(corner[0], D3DVECTOR(0.0f, 1.0f, 0.0f), ts.x, ts.y);
	vertex[2] = D3DVERTEX2(corner[3], D3DVECTOR(0.0f, 1.0f, 0.0f), ti.x, ti.y);
	vertex[3] = D3DVERTEX2(corner[2], D3DVECTOR(0.0f, 1.0f, 0.0f), ts.x, ti.y);

	m_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_VERTEX2, vertex, 4, NULL);
	AddStatisticTriangle(2);
}

// Dessine une ombre entière avec une rotation quelconque.

void CD3DEngine::DrawShadowRot(const D3DVECTOR &pos,
							   float radius,
							   float angle,
							   FPOINT ti, FPOINT ts)
{
	D3DVERTEX2	vertex[4];	// 2 triangles
	D3DVECTOR	corner[4];
	FPOINT		rot;
	float		dp;

	rot = RotatePoint(angle, FPOINT(radius, radius));
	corner[0].x = rot.x;
	corner[0].z = rot.y;
	corner[0].y = 0.0f;

	rot = RotatePoint(angle, FPOINT(-radius, radius));
	corner[1].x = rot.x;
	corner[1].z = rot.y;
	corner[1].y = 0.0f;

	rot = RotatePoint(angle, FPOINT(radius, -radius));
	corner[2].x = rot.x;
	corner[2].z = rot.y;
	corner[2].y = 0.0f;

	rot = RotatePoint(angle, FPOINT(-radius, -radius));
	corner[3].x = rot.x;
	corner[3].z = rot.y;
	corner[3].y = 0.0f;

	corner[0] += pos;
	corner[1] += pos;
	corner[2] += pos;
	corner[3] += pos;

	dp = 0.5f/256.0f;
	ti.x += dp;
	ti.y += dp;
	ts.x -= dp;
	ts.y -= dp;

	vertex[0] = D3DVERTEX2(corner[1], D3DVECTOR(0.0f, 1.0f, 0.0f), ti.x, ts.y);
	vertex[1] = D3DVERTEX2(corner[0], D3DVECTOR(0.0f, 1.0f, 0.0f), ts.x, ts.y);
	vertex[2] = D3DVERTEX2(corner[3], D3DVECTOR(0.0f, 1.0f, 0.0f), ti.x, ti.y);
	vertex[3] = D3DVERTEX2(corner[2], D3DVECTOR(0.0f, 1.0f, 0.0f), ts.x, ti.y);

	m_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_VERTEX2, vertex, 4, NULL);
	AddStatisticTriangle(2);
}

// Dessine toutes les ombres.

void CD3DEngine::DrawShadow()
{
	D3DVECTOR		pos, shift;
	D3DMATERIAL7	material;
	D3DMATRIX		matrix;
	FPOINT			ti, ts;
	float			startDeepView, endDeepView;
	float			intensity, lastIntensity, hFactor, radius, max, height, angle;
	float			h, d, D, r;
	float			adjust, minx, maxx, minz, maxz;
	int				i;
	BOOL			bRot;

	m_pD3DDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);
	m_pD3DDevice->SetRenderState(D3DRENDERSTATE_LIGHTING, FALSE);

	D3DUtil_SetIdentityMatrix(matrix);
	m_pD3DDevice->SetTransform(D3DTRANSFORMSTATE_WORLD, &matrix);

	ZeroMemory( &material, sizeof(D3DMATERIAL7) );
	material.diffuse.r = 1.0f;
	material.diffuse.g = 1.0f;
	material.diffuse.b = 1.0f;  // blanc
	material.ambient.r = 0.5f;
	material.ambient.g = 0.5f;
	material.ambient.b = 0.5f;
	SetMaterial(material);

	SetTexture("effect03.tga");

	startDeepView = m_deepView[m_rankView]*m_fogStart[m_rankView];
	endDeepView = m_deepView[m_rankView];

	lastIntensity = -1.0f;
	for ( i=0 ; i<m_shadowTotal ; i++ )
	{
		if ( !m_shadow[i].bUsed )  continue;
		if ( m_shadow[i].bHide )  continue;

		// Décalage selon l'éclairage placé au sud-est.
		shift = D3DVECTOR(-1.0f, 0.0f, 1.0f)*m_shadow[i].sunFactor;

		pos = m_shadow[i].pos;
		pos.x += shift.x;
		pos.z += shift.z;  // pos = centre de l'ombre au sol

		if ( m_eyePt.y == pos.y )  continue;  // caméra au même niveau ?

		// h est la hauteur au dessus du sol à laquelle l'ombre
		// sera dessinée.
		if ( m_eyePt.y > pos.y )  // caméra en dessus ?
		{
			height = m_eyePt.y-pos.y;
			h = m_shadow[i].radius;
			max = height*0.5f;
			if ( h > max  )  h = max;
			if ( h > 4.0f )  h = 4.0f;

			D = Length(m_eyePt, pos);
			if ( D >= endDeepView )  continue;
			d = D*h/height;

			pos.x += (m_eyePt.x-pos.x)*d/D;
			pos.z += (m_eyePt.z-pos.z)*d/D;
			pos.y += h;
		}
		else	// caméra en dessous ?
		{
			height = pos.y-m_eyePt.y;
			h = m_shadow[i].radius;
			max = height*0.1f;
			if ( h > max  )  h = max;
			if ( h > 4.0f )  h = 4.0f;

			D = Length(m_eyePt, pos);
			if ( D >= endDeepView )  continue;
			d = D*h/height;

			pos.x += (m_eyePt.x-pos.x)*d/D;
			pos.z += (m_eyePt.z-pos.z)*d/D;
			pos.y -= h;
		}

		if ( m_setup[ST_SHADOW] == 0.0f )  goto next;

		// Le hFactor diminue l'intensité et augmente la taille plus
		// l'objet est haut par rapport au sol.
		hFactor = Norm(m_shadow[i].height/40.0f);
		hFactor = powf(1.0f-hFactor, 2.0f);
		if ( hFactor < 0.0f )  hFactor = 0.0f;
		if ( hFactor == 0.0f )  continue;

		intensity = (0.5f+m_shadow[i].intensity*0.5f)*hFactor;

		// Diminue l'intensité de l'ombre si on est dans la zone
		// comprise entre le début et la fin du brouillard.
		if ( D > startDeepView )
		{
			intensity *= 1.0f-(D-startDeepView)/(endDeepView-startDeepView);
		}

		if ( intensity == 0.0f )  continue;

		if ( lastIntensity != intensity )  // intensité changée ?
		{
			lastIntensity = intensity;
			SetState(D3DSTATETTw, RetColor(intensity));
		}

		adjust = (2.0f-hFactor)*  // plus grand si haut
				 (1.0f-d/D);      // plus petit si proche

		bRot = FALSE;
		if ( m_shadow[i].type == D3DSHADOWNORM )
		{
			ti.x =  0.0f/256.0f;
			ti.y =  0.0f/256.0f;
			ts.x = 32.0f/256.0f;
			ts.y = 32.0f/256.0f;
		}
		else if ( m_shadow[i].type == D3DSHADOWSQUARE )
		{
			ti.x = 32.0f/256.0f;
			ti.y =  0.0f/256.0f;
			ts.x = 64.0f/256.0f;
			ts.y = 32.0f/256.0f;
		}
		else if ( m_shadow[i].type == D3DSHADOWDOCK )
		{
			ti.x =  64.0f/256.0f;
			ti.y =   0.0f/256.0f;
			ts.x = 128.0f/256.0f;
			ts.y =  64.0f/256.0f;
			bRot = TRUE;
		}
		else if ( m_shadow[i].type == D3DSHADOWBIRD )
		{
			ti.x = 192.0f/256.0f;
			ti.y = 192.0f/256.0f;
			ts.x = 224.0f/256.0f;
			ts.y = 224.0f/256.0f;
			bRot = TRUE;
		}
		else if ( m_shadow[i].type == D3DSHADOWGLASS )
		{
			ti.x =   0.0f/256.0f;
			ti.y =  32.0f/256.0f;
			ts.x =  32.0f/256.0f;
			ts.y =  64.0f/256.0f;
			bRot = TRUE;
		}
		else if ( m_shadow[i].type == D3DSHADOWPALISSADE )
		{
			ti.x =  32.0f/256.0f;
			ti.y =  32.0f/256.0f;
			ts.x =  64.0f/256.0f;
			ts.y =  64.0f/256.0f;
			bRot = TRUE;
		}
		else
		{
			ti.x =   0.0f/256.0f;
			ti.y =   0.0f/256.0f;
			ts.x =  32.0f/256.0f;
			ts.y =  32.0f/256.0f;
		}

#if 0
		ti.x = 224.0f/256.0f;  // motif de debug
		ti.y = 224.0f/256.0f;
		ts.x = 256.0f/256.0f;
		ts.y = 256.0f/256.0f;
#endif

		if ( bRot )
		{
			radius = m_shadow[i].radius*1.5f;
			radius *= adjust;
			DrawShadowRot(pos, radius, -m_shadow[i].angle, ti, ts);
		}
		else
		{
			if ( !m_shadow[i].bHole[0] &&
				 !m_shadow[i].bHole[1] &&
				 !m_shadow[i].bHole[2] &&
				 !m_shadow[i].bHole[3] &&
				 !m_shadow[i].bHole[4] &&
				 !m_shadow[i].bHole[5] &&
				 !m_shadow[i].bHole[6] &&
				 !m_shadow[i].bHole[7] &&
				 !m_shadow[i].bHole[8] )  // ombre en un seul morceau ?
			{
				minx = m_shadow[i].posTerrain.x-12.0f;
				maxx = m_shadow[i].posTerrain.x+12.0f;
				minz = m_shadow[i].posTerrain.z-12.0f;
				maxz = m_shadow[i].posTerrain.z+12.0f;
				DrawShadowPiece(i, pos, shift, adjust, minx, maxx, minz, maxz, ti, ts);
				goto next;
			}

			if ( (!m_shadow[i].bHole[0] &&
				  !m_shadow[i].bHole[1] &&
				  !m_shadow[i].bHole[2] ) ||
				 (!m_shadow[i].bHole[6] &&
				  !m_shadow[i].bHole[7] &&
				  !m_shadow[i].bHole[8] ) )  // ombre en bandes horizontales ?
			{
				minz = m_shadow[i].posTerrain.z+ 4.0f;
				maxz = m_shadow[i].posTerrain.z+12.0f;

				if ( !m_shadow[i].bHole[0] &&
					 !m_shadow[i].bHole[1] &&
					 !m_shadow[i].bHole[2] )
				{
					minx = m_shadow[i].posTerrain.x-12.0f;
					maxx = m_shadow[i].posTerrain.x+12.0f;
					DrawShadowPiece(i, pos, shift, adjust, minx, maxx, minz, maxz, ti, ts);
				}
				else
				{
					if ( !m_shadow[i].bHole[0] )
					{
						minx = m_shadow[i].posTerrain.x-12.0f;
						maxx = m_shadow[i].posTerrain.x- 4.0f;
						DrawShadowPiece(i, pos, shift, adjust, minx, maxx, minz, maxz, ti, ts);
					}
					if ( !m_shadow[i].bHole[1] )
					{
						minx = m_shadow[i].posTerrain.x- 4.0f;
						maxx = m_shadow[i].posTerrain.x+ 4.0f;
						DrawShadowPiece(i, pos, shift, adjust, minx, maxx, minz, maxz, ti, ts);
					}
					if ( !m_shadow[i].bHole[2] )
					{
						minx = m_shadow[i].posTerrain.x+ 4.0f;
						maxx = m_shadow[i].posTerrain.x+12.0f;
						DrawShadowPiece(i, pos, shift, adjust, minx, maxx, minz, maxz, ti, ts);
					}
				}

				minz = m_shadow[i].posTerrain.z- 4.0f;
				maxz = m_shadow[i].posTerrain.z+ 4.0f;

				if ( !m_shadow[i].bHole[3] &&
					 !m_shadow[i].bHole[4] &&
					 !m_shadow[i].bHole[5] )
				{
					minx = m_shadow[i].posTerrain.x-12.0f;
					maxx = m_shadow[i].posTerrain.x+12.0f;
					DrawShadowPiece(i, pos, shift, adjust, minx, maxx, minz, maxz, ti, ts);
				}
				else
				{
					if ( !m_shadow[i].bHole[3] )
					{
						minx = m_shadow[i].posTerrain.x-12.0f;
						maxx = m_shadow[i].posTerrain.x- 4.0f;
						DrawShadowPiece(i, pos, shift, adjust, minx, maxx, minz, maxz, ti, ts);
					}
					if ( !m_shadow[i].bHole[4] )
					{
						minx = m_shadow[i].posTerrain.x- 4.0f;
						maxx = m_shadow[i].posTerrain.x+ 4.0f;
						DrawShadowPiece(i, pos, shift, adjust, minx, maxx, minz, maxz, ti, ts);
					}
					if ( !m_shadow[i].bHole[5] )
					{
						minx = m_shadow[i].posTerrain.x+ 4.0f;
						maxx = m_shadow[i].posTerrain.x+12.0f;
						DrawShadowPiece(i, pos, shift, adjust, minx, maxx, minz, maxz, ti, ts);
					}
				}

				minz = m_shadow[i].posTerrain.z-12.0f;
				maxz = m_shadow[i].posTerrain.z -4.0f;

				if ( !m_shadow[i].bHole[6] &&
					 !m_shadow[i].bHole[7] &&
					 !m_shadow[i].bHole[8] )
				{
					minx = m_shadow[i].posTerrain.x-12.0f;
					maxx = m_shadow[i].posTerrain.x+12.0f;
					DrawShadowPiece(i, pos, shift, adjust, minx, maxx, minz, maxz, ti, ts);
				}
				else
				{
					if ( !m_shadow[i].bHole[6] )
					{
						minx = m_shadow[i].posTerrain.x-12.0f;
						maxx = m_shadow[i].posTerrain.x- 4.0f;
						DrawShadowPiece(i, pos, shift, adjust, minx, maxx, minz, maxz, ti, ts);
					}
					if ( !m_shadow[i].bHole[7] )
					{
						minx = m_shadow[i].posTerrain.x- 4.0f;
						maxx = m_shadow[i].posTerrain.x+ 4.0f;
						DrawShadowPiece(i, pos, shift, adjust, minx, maxx, minz, maxz, ti, ts);
					}
					if ( !m_shadow[i].bHole[8] )
					{
						minx = m_shadow[i].posTerrain.x+ 4.0f;
						maxx = m_shadow[i].posTerrain.x+12.0f;
						DrawShadowPiece(i, pos, shift, adjust, minx, maxx, minz, maxz, ti, ts);
					}
				}
				goto next;
			}

			if ( (!m_shadow[i].bHole[0] &&
				  !m_shadow[i].bHole[3] &&
				  !m_shadow[i].bHole[6] ) ||
				 (!m_shadow[i].bHole[2] &&
				  !m_shadow[i].bHole[5] &&
				  !m_shadow[i].bHole[8] ) )  // ombre en bandes verticales ?
			{
				minx = m_shadow[i].posTerrain.x-12.0f;
				maxx = m_shadow[i].posTerrain.x- 4.0f;

				if ( !m_shadow[i].bHole[0] &&
					 !m_shadow[i].bHole[3] &&
					 !m_shadow[i].bHole[6] )
				{
					minz = m_shadow[i].posTerrain.z-12.0f;
					maxz = m_shadow[i].posTerrain.z+12.0f;
					DrawShadowPiece(i, pos, shift, adjust, minx, maxx, minz, maxz, ti, ts);
				}
				else
				{
					if ( !m_shadow[i].bHole[6] )
					{
						minz = m_shadow[i].posTerrain.z-12.0f;
						maxz = m_shadow[i].posTerrain.z- 4.0f;
						DrawShadowPiece(i, pos, shift, adjust, minx, maxx, minz, maxz, ti, ts);
					}
					if ( !m_shadow[i].bHole[3] )
					{
						minz = m_shadow[i].posTerrain.z- 4.0f;
						maxz = m_shadow[i].posTerrain.z+ 4.0f;
						DrawShadowPiece(i, pos, shift, adjust, minx, maxx, minz, maxz, ti, ts);
					}
					if ( !m_shadow[i].bHole[0] )
					{
						minz = m_shadow[i].posTerrain.z+ 4.0f;
						maxz = m_shadow[i].posTerrain.z+12.0f;
						DrawShadowPiece(i, pos, shift, adjust, minx, maxx, minz, maxz, ti, ts);
					}
				}

				minx = m_shadow[i].posTerrain.x- 4.0f;
				maxx = m_shadow[i].posTerrain.x+ 4.0f;

				if ( !m_shadow[i].bHole[1] &&
					 !m_shadow[i].bHole[4] &&
					 !m_shadow[i].bHole[7] )
				{
					minz = m_shadow[i].posTerrain.z-12.0f;
					maxz = m_shadow[i].posTerrain.z+12.0f;
					DrawShadowPiece(i, pos, shift, adjust, minx, maxx, minz, maxz, ti, ts);
				}
				else
				{
					if ( !m_shadow[i].bHole[7] )
					{
						minz = m_shadow[i].posTerrain.z-12.0f;
						maxz = m_shadow[i].posTerrain.z- 4.0f;
						DrawShadowPiece(i, pos, shift, adjust, minx, maxx, minz, maxz, ti, ts);
					}
					if ( !m_shadow[i].bHole[4] )
					{
						minz = m_shadow[i].posTerrain.z- 4.0f;
						maxz = m_shadow[i].posTerrain.z+ 4.0f;
						DrawShadowPiece(i, pos, shift, adjust, minx, maxx, minz, maxz, ti, ts);
					}
					if ( !m_shadow[i].bHole[1] )
					{
						minz = m_shadow[i].posTerrain.z+ 4.0f;
						maxz = m_shadow[i].posTerrain.z+12.0f;
						DrawShadowPiece(i, pos, shift, adjust, minx, maxx, minz, maxz, ti, ts);
					}
				}

				minx = m_shadow[i].posTerrain.x+ 4.0f;
				maxx = m_shadow[i].posTerrain.x+12.0f;

				if ( !m_shadow[i].bHole[2] &&
					 !m_shadow[i].bHole[5] &&
					 !m_shadow[i].bHole[8] )
				{
					minz = m_shadow[i].posTerrain.z-12.0f;
					maxz = m_shadow[i].posTerrain.z+12.0f;
					DrawShadowPiece(i, pos, shift, adjust, minx, maxx, minz, maxz, ti, ts);
				}
				else
				{
					if ( !m_shadow[i].bHole[8] )
					{
						minz = m_shadow[i].posTerrain.z-12.0f;
						maxz = m_shadow[i].posTerrain.z- 4.0f;
						DrawShadowPiece(i, pos, shift, adjust, minx, maxx, minz, maxz, ti, ts);
					}
					if ( !m_shadow[i].bHole[5] )
					{
						minz = m_shadow[i].posTerrain.z- 4.0f;
						maxz = m_shadow[i].posTerrain.z+ 4.0f;
						DrawShadowPiece(i, pos, shift, adjust, minx, maxx, minz, maxz, ti, ts);
					}
					if ( !m_shadow[i].bHole[2] )
					{
						minz = m_shadow[i].posTerrain.z+ 4.0f;
						maxz = m_shadow[i].posTerrain.z+12.0f;
						DrawShadowPiece(i, pos, shift, adjust, minx, maxx, minz, maxz, ti, ts);
					}
				}
				goto next;
			}

			// Ombre en 9 morceaux individuels :
			if ( !m_shadow[i].bHole[0] )
			{
				minx = m_shadow[i].posTerrain.x-12.0f;
				maxx = m_shadow[i].posTerrain.x- 4.0f;
				minz = m_shadow[i].posTerrain.z+ 4.0f;
				maxz = m_shadow[i].posTerrain.z+12.0f;
				DrawShadowPiece(i, pos, shift, adjust, minx, maxx, minz, maxz, ti, ts);
			}
			if ( !m_shadow[i].bHole[1] )
			{
				minx = m_shadow[i].posTerrain.x- 4.0f;
				maxx = m_shadow[i].posTerrain.x+ 4.0f;
				minz = m_shadow[i].posTerrain.z+ 4.0f;
				maxz = m_shadow[i].posTerrain.z+12.0f;
				DrawShadowPiece(i, pos, shift, adjust, minx, maxx, minz, maxz, ti, ts);
			}
			if ( !m_shadow[i].bHole[2] )
			{
				minx = m_shadow[i].posTerrain.x+ 4.0f;
				maxx = m_shadow[i].posTerrain.x+12.0f;
				minz = m_shadow[i].posTerrain.z+ 4.0f;
				maxz = m_shadow[i].posTerrain.z+12.0f;
				DrawShadowPiece(i, pos, shift, adjust, minx, maxx, minz, maxz, ti, ts);
			}
			if ( !m_shadow[i].bHole[3] )
			{
				minx = m_shadow[i].posTerrain.x-12.0f;
				maxx = m_shadow[i].posTerrain.x- 4.0f;
				minz = m_shadow[i].posTerrain.z- 4.0f;
				maxz = m_shadow[i].posTerrain.z+ 4.0f;
				DrawShadowPiece(i, pos, shift, adjust, minx, maxx, minz, maxz, ti, ts);
			}
			if ( !m_shadow[i].bHole[4] )
			{
				minx = m_shadow[i].posTerrain.x- 4.0f;
				maxx = m_shadow[i].posTerrain.x+ 4.0f;
				minz = m_shadow[i].posTerrain.z- 4.0f;
				maxz = m_shadow[i].posTerrain.z+ 4.0f;
				DrawShadowPiece(i, pos, shift, adjust, minx, maxx, minz, maxz, ti, ts);
			}
			if ( !m_shadow[i].bHole[5] )
			{
				minx = m_shadow[i].posTerrain.x+ 4.0f;
				maxx = m_shadow[i].posTerrain.x+12.0f;
				minz = m_shadow[i].posTerrain.z- 4.0f;
				maxz = m_shadow[i].posTerrain.z+ 4.0f;
				DrawShadowPiece(i, pos, shift, adjust, minx, maxx, minz, maxz, ti, ts);
			}
			if ( !m_shadow[i].bHole[6] )
			{
				minx = m_shadow[i].posTerrain.x-12.0f;
				maxx = m_shadow[i].posTerrain.x- 4.0f;
				minz = m_shadow[i].posTerrain.z-12.0f;
				maxz = m_shadow[i].posTerrain.z- 4.0f;
				DrawShadowPiece(i, pos, shift, adjust, minx, maxx, minz, maxz, ti, ts);
			}
			if ( !m_shadow[i].bHole[7] )
			{
				minx = m_shadow[i].posTerrain.x- 4.0f;
				maxx = m_shadow[i].posTerrain.x+ 4.0f;
				minz = m_shadow[i].posTerrain.z-12.0f;
				maxz = m_shadow[i].posTerrain.z- 4.0f;
				DrawShadowPiece(i, pos, shift, adjust, minx, maxx, minz, maxz, ti, ts);
			}
			if ( !m_shadow[i].bHole[8] )
			{
				minx = m_shadow[i].posTerrain.x+ 4.0f;
				maxx = m_shadow[i].posTerrain.x+12.0f;
				minz = m_shadow[i].posTerrain.z-12.0f;
				maxz = m_shadow[i].posTerrain.z- 4.0f;
				DrawShadowPiece(i, pos, shift, adjust, minx, maxx, minz, maxz, ti, ts);
			}
		}

next:
//?		if ( m_shadow[i].bSelect && !m_bPause )  // object sélectionné ?
		if ( m_shadow[i].bSelect )  // object sélectionné ?
		{
			pos.x -= shift.x;
			pos.z -= shift.z;

			if ( m_shadow[i].time < 1.0f )
			{
				SetState(D3DSTATETTb);
				radius = m_shadow[i].radius*1.5f;
				angle = -m_shadow[i].time*3.0f;
				r = radius*Norm(1.0f-m_shadow[i].time)*3.0f;
				ti.x =   0.0f/256.0f;
				ti.y = 128.0f/256.0f;
				ts.x =  64.0f/256.0f;
				ts.y = 192.0f/256.0f;
				DrawShadowRot(pos, r, angle, ti, ts);

				SetState(D3DSTATETTw);
				ti.x =  64.0f/256.0f;
				ti.y = 128.0f/256.0f;
				ts.x = 128.0f/256.0f;
				ts.y = 192.0f/256.0f;
				DrawShadowRot(pos, r, angle, ti, ts);
			}

			SetState(D3DSTATETTb);
			angle = m_shadow[i].time*1.0f;
			d = (1.0f+sinf(m_shadow[i].time*10.0f))/2.0f;
			radius = m_shadow[i].radius*1.5f;
			radius *= 1.0f+d*0.5f;
			ti.x = 128.0f/256.0f;
			ti.y = 128.0f/256.0f;
			ts.x = 192.0f/256.0f;
			ts.y = 192.0f/256.0f;
			DrawShadowRot(pos, radius, angle, ti, ts);

			SetState(D3DSTATETTw);
			ti.x = 192.0f/256.0f;
			ti.y = 128.0f/256.0f;
			ts.x = 256.0f/256.0f;
			ts.y = 192.0f/256.0f;
			DrawShadowRot(pos, radius, angle, ti, ts);

			lastIntensity = -1.0f;
		}
	}

	m_pD3DDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE);
	m_pD3DDevice->SetRenderState(D3DRENDERSTATE_LIGHTING, TRUE);
}


// Called once per frame, the call is the entry point for 3d
// rendering. This function sets up render states, clears the
// viewport, and renders the scene.

HRESULT CD3DEngine::Render()
{
	D3DObjLevel1*	p1;
	D3DObjLevel2a*	p2a;
	D3DObjLevel2b*	p2b;
	D3DObjLevel3*	p3;
	D3DObjLevel4*	p4;
	D3DObjLevel5*	p5;
	D3DObjLevel6*	p6;
	D3DVERTEX2*		pv;
	int				l1, l2a, l2b, l3, l4, l5, objRank, tState;
	CInterface*		pInterface;
	BOOL			bTransparent;
	D3DCOLOR		color, tColor;
	D3DCOLOR		fogColor;
	D3DCOLORVALUE	cvLight, cvDark, cv;
	float			angle, intensity;

	if ( !m_bRender )  return S_OK;

	m_statisticTriangle = 0;
	m_lastState = -1;
	m_lastColor = 999;
	m_lastTexture[0][0] = 0;
	m_lastTexture[1][0] = 0;
	ZeroMemory(&m_lastMaterial, sizeof(D3DMATERIAL7));

	if ( m_bGroundSpot )
	{
		RenderGroundSpot();
	}

	// Clear the viewport
	color = m_backgroundColorDown;
	m_pD3DDevice->Clear( 0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,
						 color, 1.0f, 0L );

	m_light->LightUpdate();

	// Begin the scene
	if( FAILED( m_pD3DDevice->BeginScene() ) )  return S_OK;

	// L'intensité est maximale (1.0) si on regarde contre le soleil,
	// c'est-à-dire au sud-est. On est face au soleil lorsque
	// m_eyeDirH = PI*0.75 (135 degrés).
	angle = NormAngle(m_eyeDirH-PI*0.75f);
	intensity = 0.5f+sinf(PI*0.5f+angle)*0.5f;

	cvLight = RetColor(m_fogColor[0]);
	cvDark  = RetColor(m_fogColor[1]);
	cv.r = cvDark.r+(cvLight.r-cvDark.r)*intensity;
	cv.g = cvDark.g+(cvLight.g-cvDark.g)*intensity;
	cv.b = cvDark.b+(cvLight.b-cvDark.b)*intensity;
	fogColor = RetColor(cv);
	
	if ( m_bDrawWorld )
	{
		DrawBackground();  // dessine l'arrière-plan

		// Display the objects
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, TRUE);
//?		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_ZBIAS, F2DW(16));
//?		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_ZFUNC, D3DCMP_LESSEQUAL);
		m_pD3DDevice->SetTransform(D3DTRANSFORMSTATE_PROJECTION, &m_matProj);
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_LIGHTING, TRUE);

		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, TRUE);
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_FOGCOLOR, fogColor);
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_FOGVERTEXMODE, D3DFOG_LINEAR);
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_FOGSTART, F2DW(m_deepView[m_rankView]*m_fogStart[m_rankView]));
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_FOGEND,   F2DW(m_deepView[m_rankView]));

		m_pD3DDevice->SetTransform(D3DTRANSFORMSTATE_VIEW, &m_matView);

		// Dessine le terrain.
		p1 = m_objectPointer;
		for ( l1=0 ; l1<p1->totalUsed ; l1++ )
		{
			p2a = p1->table[l1];
			if ( p2a == 0 )  continue;
			for ( l2a=0 ; l2a<p2a->totalUsed ; l2a++ )
			{
				p2b = p2a->table[l2a];
				if ( p2b == 0 )  continue;
				SetTexture(p2b->texName1, 0);
				SetTexture(p2b->texName2, 1);
				for ( l2b=0 ; l2b<p2b->totalUsed ; l2b++ )
				{
					p3 = p2b->table[l2b];
					if ( p3 == 0 )  continue;
					objRank = p3->objRank;
					if ( m_objectParam[objRank].bHide )  continue;
					if ( m_objectParam[objRank].type != TYPETERRAIN )  continue;
					if ( !m_objectParam[objRank].bDrawWorld )  continue;
					m_pD3DDevice->SetTransform(D3DTRANSFORMSTATE_WORLD,
											   &m_objectParam[objRank].transform);
					if ( !IsVisible(objRank) )  continue;
					m_light->LightUpdate(m_objectParam[objRank].type);
					for ( l3=0 ; l3<p3->totalUsed ; l3++ )
					{
						p4 = p3->table[l3];
						if ( p4 == 0 )  continue;
						if ( m_objectParam[objRank].distance <  p4->min ||
							 m_objectParam[objRank].distance >= p4->max )  continue;
						for ( l4=0 ; l4<p4->totalUsed ; l4++ )
						{
							p5 = p4->table[l4];
							if ( p5 == 0 )  continue;
							for ( l5=0 ; l5<p5->totalUsed ; l5++ )
							{
								p6 = p5->table[l5];
								if ( p6 == 0 )  continue;
								SetMaterial(p6->material);
								SetState(p6->state);
								if ( p6->type == D3DTYPE6T )
								{
									pv = &p6->vertex[0];
									m_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLELIST,
																D3DFVF_VERTEX2,
																pv, p6->totalUsed,
																NULL);
									m_statisticTriangle += p6->totalUsed/3;
								}
								if ( p6->type == D3DTYPE6S )
								{
									pv = &p6->vertex[0];
									m_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP,
																D3DFVF_VERTEX2,
																pv, p6->totalUsed,
																NULL);
									m_statisticTriangle += p6->totalUsed-2;
								}
							}
						}
					}
				}
			}
		}

		m_light->LightUpdate(TYPETERRAIN);

		m_water->DrawSurf();  // dessine l'eau

		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, TRUE);
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_FOGCOLOR, fogColor);
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_FOGVERTEXMODE, D3DFOG_LINEAR);
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_FOGSTART, F2DW(m_deepView[m_rankView]*m_fogStart[m_rankView]));
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_FOGEND,   F2DW(m_deepView[m_rankView]));

		DrawShadow();  // dessine les ombres

		// Dessine les objets.
		bTransparent = FALSE;
		p1 = m_objectPointer;
		for ( l1=0 ; l1<p1->totalUsed ; l1++ )
		{
			p2a = p1->table[l1];
			if ( p2a == 0 )  continue;
			for ( l2a=0 ; l2a<p2a->totalUsed ; l2a++ )
			{
				p2b = p2a->table[l2a];
				if ( p2b == 0 )  continue;
				SetTexture(p2b->texName1, 0);
				SetTexture(p2b->texName2, 1);
				for ( l2b=0 ; l2b<p2b->totalUsed ; l2b++ )
				{
					p3 = p2b->table[l2b];
					if ( p3 == 0 )  continue;
					objRank = p3->objRank;
					if ( m_objectParam[objRank].bHide )  continue;
					if ( m_objectParam[objRank].type == TYPETERRAIN )  continue;
					if ( !m_objectParam[objRank].bDrawWorld )  continue;
					m_pD3DDevice->SetTransform(D3DTRANSFORMSTATE_WORLD,
											   &m_objectParam[objRank].transform);
					if ( !IsVisible(objRank) )  continue;
					m_light->LightUpdate(m_objectParam[objRank].type);
					for ( l3=0 ; l3<p3->totalUsed ; l3++ )
					{
						p4 = p3->table[l3];
						if ( p4 == 0 )  continue;
						if ( m_objectParam[objRank].distance <  p4->min ||
							 m_objectParam[objRank].distance >= p4->max )  continue;
						for ( l4=0 ; l4<p4->totalUsed ; l4++ )
						{
							p5 = p4->table[l4];
							if ( p5 == 0 )  continue;
							for ( l5=0 ; l5<p5->totalUsed ; l5++ )
							{
								p6 = p5->table[l5];
								if ( p6 == 0 )  continue;
								SetMaterial(p6->material);
								if ( m_objectParam[objRank].transparency != 0.0f )  // transparent ?
								{
									bTransparent = TRUE;
									continue;
								}
								SetState(p6->state);
								if ( p6->type == D3DTYPE6T )
								{
									pv = &p6->vertex[0];
									m_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLELIST,
																D3DFVF_VERTEX2,
																pv, p6->totalUsed,
																NULL);
									m_statisticTriangle += p6->totalUsed/3;
								}
								if ( p6->type == D3DTYPE6S )
								{
									pv = &p6->vertex[0];
									m_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP,
																D3DFVF_VERTEX2,
																pv, p6->totalUsed,
																NULL);
									m_statisticTriangle += p6->totalUsed-2;
								}
							}
						}
					}
				}
			}
		}

		if ( bTransparent )
		{
			if ( m_bStateColor )
			{
				tState = D3DSTATETTb|D3DSTATE2FACE;
				tColor = 0x44444444;
			}
			else
			{
				tState = D3DSTATETTb;
				tColor = 0x88888888;
			}

			// Dessine les objets transparents.
			p1 = m_objectPointer;
			for ( l1=0 ; l1<p1->totalUsed ; l1++ )
			{
				p2a = p1->table[l1];
				if ( p2a == 0 )  continue;
				for ( l2a=0 ; l2a<p2a->totalUsed ; l2a++ )
				{
					p2b = p2a->table[l2a];
					if ( p2b == 0 )  continue;
					SetTexture(p2b->texName1, 0);
					SetTexture(p2b->texName2, 1);
					for ( l2b=0 ; l2b<p2b->totalUsed ; l2b++ )
					{
						p3 = p2b->table[l2b];
						if ( p3 == 0 )  continue;
						objRank = p3->objRank;
						if ( m_objectParam[objRank].bHide )  continue;
						if ( m_objectParam[objRank].type == TYPETERRAIN )  continue;
						if ( !m_objectParam[objRank].bDrawWorld )  continue;
						m_pD3DDevice->SetTransform(D3DTRANSFORMSTATE_WORLD,
												   &m_objectParam[objRank].transform);
						if ( !IsVisible(objRank) )  continue;
						m_light->LightUpdate(m_objectParam[objRank].type);
						for ( l3=0 ; l3<p3->totalUsed ; l3++ )
						{
							p4 = p3->table[l3];
							if ( p4 == 0 )  continue;
							if ( m_objectParam[objRank].distance <  p4->min ||
								 m_objectParam[objRank].distance >= p4->max )  continue;
							for ( l4=0 ; l4<p4->totalUsed ; l4++ )
							{
								p5 = p4->table[l4];
								if ( p5 == 0 )  continue;
								for ( l5=0 ; l5<p5->totalUsed ; l5++ )
								{
									p6 = p5->table[l5];
									if ( p6 == 0 )  continue;
									SetMaterial(p6->material);
									if ( m_objectParam[objRank].transparency == 0.0f )  continue;
									SetState(tState, tColor);
									if ( p6->type == D3DTYPE6T )
									{
										pv = &p6->vertex[0];
										m_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLELIST,
																	D3DFVF_VERTEX2,
																	pv, p6->totalUsed,
																	NULL);
										m_statisticTriangle += p6->totalUsed/3;
									}
									if ( p6->type == D3DTYPE6S )
									{
										pv = &p6->vertex[0];
										m_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP,
																	D3DFVF_VERTEX2,
																	pv, p6->totalUsed,
																	NULL);
										m_statisticTriangle += p6->totalUsed-2;
									}
								}
							}
						}
					}
				}
			}
		}

//-		m_light->LightUpdate(TYPETERRAIN);

		m_particule->DrawParticule(SH_WORLD);  // dessine les particules du monde 3D
		m_blitz->Draw();  // dessine les éclairs
		if ( m_setup[ST_LENSFLARE] != 0.0f )  DrawLensFlare();  // dessine l'avant-plan
		if ( !m_bOverFront )  DrawOverColor();  // dessine la couleur d'avant-plan
	}

	// Dessine l'interface utilisateur par-dessus la scène.
	m_pD3DDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, FALSE);
	m_pD3DDevice->SetRenderState(D3DRENDERSTATE_AMBIENT, 0xffffffff);
	m_pD3DDevice->SetRenderState(D3DRENDERSTATE_LIGHTING, FALSE);
	m_pD3DDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, FALSE);

	m_pD3DDevice->SetTransform(D3DTRANSFORMSTATE_VIEW,       &m_matViewInterface);
	m_pD3DDevice->SetTransform(D3DTRANSFORMSTATE_PROJECTION, &m_matProjInterface);
	m_pD3DDevice->SetTransform(D3DTRANSFORMSTATE_WORLD,      &m_matWorldInterface);

	pInterface = (CInterface*)m_iMan->SearchInstance(CLASS_INTERFACE);
	if ( pInterface != 0 )
	{
		pInterface->Draw();  // dessine toute l'interface
	}
	m_particule->DrawParticule(SH_INTERFACE);  // dessine les particules de l'interface

	if ( m_bDrawFront )
	{
		// Display the objects
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, TRUE);
//?		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_ZBIAS, F2DW(16));
//?		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_ZFUNC, D3DCMP_LESSEQUAL);
		m_pD3DDevice->SetTransform(D3DTRANSFORMSTATE_PROJECTION, &m_matProj);
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_AMBIENT, m_ambiantColor[m_rankView]);
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_LIGHTING, TRUE);

		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, TRUE);
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_FOGCOLOR, fogColor);
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_FOGVERTEXMODE, D3DFOG_LINEAR);
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_FOGSTART, F2DW(m_deepView[m_rankView]*m_fogStart[m_rankView]));
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_FOGEND,   F2DW(m_deepView[m_rankView]));

		m_pD3DDevice->SetTransform(D3DTRANSFORMSTATE_VIEW, &m_matView);

		p1 = m_objectPointer;
		for ( l1=0 ; l1<p1->totalUsed ; l1++ )
		{
			p2a = p1->table[l1];
			if ( p2a == 0 )  continue;
			for ( l2a=0 ; l2a<p2a->totalUsed ; l2a++ )
			{
				p2b = p2a->table[l2a];
				if ( p2b == 0 )  continue;
				SetTexture(p2b->texName1, 0);
				SetTexture(p2b->texName2, 1);
				for ( l2b=0 ; l2b<p2b->totalUsed ; l2b++ )
				{
					p3 = p2b->table[l2b];
					if ( p3 == 0 )  continue;
					objRank = p3->objRank;
					if ( m_objectParam[objRank].bHide )  continue;
					if ( !m_objectParam[objRank].bDrawFront )  continue;
					m_pD3DDevice->SetTransform(D3DTRANSFORMSTATE_WORLD,
											   &m_objectParam[objRank].transform);
					if ( !IsVisible(objRank) )  continue;
					m_light->LightUpdate(m_objectParam[objRank].type);
					for ( l3=0 ; l3<p3->totalUsed ; l3++ )
					{
						p4 = p3->table[l3];
						if ( p4 == 0 )  continue;
						if ( m_objectParam[objRank].distance <  p4->min ||
							 m_objectParam[objRank].distance >= p4->max )  continue;
						for ( l4=0 ; l4<p4->totalUsed ; l4++ )
						{
							p5 = p4->table[l4];
							if ( p5 == 0 )  continue;
							for ( l5=0 ; l5<p5->totalUsed ; l5++ )
							{
								p6 = p5->table[l5];
								if ( p6 == 0 )  continue;
								SetMaterial(p6->material);
								SetState(p6->state);
								if ( p6->type == D3DTYPE6T )
								{
									pv = &p6->vertex[0];
									m_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLELIST,
																D3DFVF_VERTEX2,
																pv, p6->totalUsed,
																NULL);
									m_statisticTriangle += p6->totalUsed/3;
								}
								if ( p6->type == D3DTYPE6S )
								{
									pv = &p6->vertex[0];
									m_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP,
																D3DFVF_VERTEX2,
																pv, p6->totalUsed,
																NULL);
									m_statisticTriangle += p6->totalUsed-2;
								}
							}
						}
					}
				}
			}
		}

//?		m_particule->DrawParticule(SH_FRONT);  // dessine les particules du monde 3D
		m_particule->DrawParticule(SH_WORLD);  // dessine les particules du monde 3D

		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, FALSE);
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_AMBIENT, 0xffffffff);
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_LIGHTING, FALSE);
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, FALSE);

		m_pD3DDevice->SetTransform(D3DTRANSFORMSTATE_VIEW,       &m_matViewInterface);
		m_pD3DDevice->SetTransform(D3DTRANSFORMSTATE_PROJECTION, &m_matProjInterface);
		m_pD3DDevice->SetTransform(D3DTRANSFORMSTATE_WORLD,      &m_matWorldInterface);
	}

	if ( m_bOverFront )  DrawOverColor();  // dessine la couleur d'avant-plan

	if ( m_mouseType != D3DMOUSEHIDE )
	{
		DrawMouse();
	}

	// End the scene.
	m_pD3DDevice->EndScene();

	DrawHilite();
	return S_OK;
}


// Dessine le dégradé d'arrière-plan.

void CD3DEngine::DrawBackground()
{
	float	horizon, sup;

	horizon = Norm((m_eyeDirV-30.0f*PI/180.0f)/(30.0f*PI/180.0f));
	sup = horizon+1.0f;

	if ( m_backgroundColorUp != m_backgroundColorDown )  // dégradé ?
	{
		DrawBackgroundGradient(FPOINT(0.0f, horizon), FPOINT(1.0f, sup),
							   m_backgroundColorUp, m_backgroundColorDown);
	}

	if ( m_bBackForce || m_backgroundName[0] != 0 )
	{
		DrawBackgroundImage();  // image
	}

	if ( m_bBackgroundPanel )
	{
		DrawBackgroundGradient(FPOINT( 94.0f/640.0f, 112.0f/480.0f),
							   FPOINT(546.0f/640.0f, 342.0f/480.0f),
							   m_backgroundColorUp, m_backgroundColorDown);
	}
}

// Dessine le dégradé d'arrière-plan.

void CD3DEngine::DrawBackgroundGradient(FPOINT p1, FPOINT p2, D3DCOLOR up, D3DCOLOR down)
{
	D3DLVERTEX		vertex[4];	// 2 triangles
	D3DCOLOR		color[3], black;
	D3DCOLORVALUE	cvLight, cvDark, cv;
	float			angle, intensity;

	black = 0x00000000;

	// Le fond devient plus clair si on regarde contre le soleil,
	// c'est-à-dire au sud-est. On est face au soleil lorsque
	// m_eyeDirH = PI*0.75 (135 degrés).
	if ( m_backgroundSunFlash == -1.0f )
	{
		angle = NormAngle(m_eyeDirH-PI*0.75f);
		intensity = 0.5f+sinf(PI*0.5f+angle)*0.5f;

		cvLight = RetColor(up);
		cvDark  = RetColor(down);
		cv.r = cvDark.r+(cvLight.r-cvDark.r)*intensity;
		cv.g = cvDark.g+(cvLight.g-cvDark.g)*intensity;
		cv.b = cvDark.b+(cvLight.b-cvDark.b)*intensity;
		up = down = RetColor(cv);

		p1 = FPOINT(0.0f, 0.0f);
		p2 = FPOINT(1.0f, 1.0f);
	}
	else
	{
		angle = m_eyeDirH;
		if ( angle <= PI*0.25f || angle >= PI*1.25f )
		{
			intensity = 0.0f;
		}
		else
		{
			angle -= PI*0.25f;
			intensity = sinf(angle);
		}

		if ( intensity > 0.0f )
		{
			intensity = 1.0f-intensity*m_backgroundSunFlash;

			cv = RetColor(up);
			cv.r = 1.0f-(1.0f-cv.r)*intensity;
			cv.g = 1.0f-(1.0f-cv.g)*intensity;
			cv.b = 1.0f-(1.0f-cv.b)*intensity;
			up = RetColor(cv);
		}
	}

	color[0] = up;
	color[1] = down;
	color[2] = black;

//?	m_pD3DDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, FALSE );
	m_pD3DDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);
	m_pD3DDevice->SetRenderState(D3DRENDERSTATE_AMBIENT, 0xffffffff);
	m_pD3DDevice->SetRenderState(D3DRENDERSTATE_LIGHTING, FALSE );
	m_pD3DDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, FALSE);

	SetTexture("xxx.tga");  // pas de texture
	SetState(D3DSTATENORMAL);

	m_pD3DDevice->SetTransform(D3DTRANSFORMSTATE_VIEW,       &m_matViewBack);
	m_pD3DDevice->SetTransform(D3DTRANSFORMSTATE_PROJECTION, &m_matProjBack);
	m_pD3DDevice->SetTransform(D3DTRANSFORMSTATE_WORLD,      &m_matWorldBack);

	vertex[0] = D3DLVERTEX(D3DVECTOR(p1.x, p1.y, 0.0f), color[1],color[2], 0.0f,0.0f);
	vertex[1] = D3DLVERTEX(D3DVECTOR(p1.x, p2.y, 0.0f), color[0],color[2], 0.0f,0.0f);
	vertex[2] = D3DLVERTEX(D3DVECTOR(p2.x, p1.y, 0.0f), color[1],color[2], 0.0f,0.0f);
	vertex[3] = D3DLVERTEX(D3DVECTOR(p2.x, p2.y, 0.0f), color[0],color[2], 0.0f,0.0f);

	m_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_LVERTEX, vertex, 4, NULL);
	AddStatisticTriangle(2);
}
	
// Dessine une partie de l'image d'arrière-plan.

void CD3DEngine::DrawBackgroundImageQuarter(FPOINT p1, FPOINT p2, char *name)
{
	D3DVERTEX2	vertex[4];	// 2 triangles
	D3DVECTOR	n;
	float		u1, u2, v1, v2, h, a;

	n = D3DVECTOR(0.0f, 0.0f, -1.0f);  // normale

	if ( m_bBackgroundFull )
	{
		u1 = 0.0f;
		v1 = 0.0f;
		u2 = 1.0f;
		v2 = 1.0f;

		if ( m_bBackgroundQuarter )
		{
			u1 += 0.5f/512.0f;
			v1 += 0.5f/384.0f;
			u2 -= 0.5f/512.0f;
			v2 -= 0.5f/384.0f;
		}
	}
	else
	{
		h = 0.5f;  // partie visible verticalement (1=tout)
		a = m_eyeDirV-PI*0.15f;
		if ( a >  PI      )  a -= PI*2.0f;  // a = -PI..PI
		if ( a >  PI/4.0f )  a =  PI/4.0f;
		if ( a < -PI/4.0f )  a = -PI/4.0f;

		u1 = -m_eyeDirH/PI;
		u2 = u1+1.0f/PI;
//?		u1 = -m_eyeDirH/(PI*2.0f);
//?		u2 = u1+1.0f/(PI*2.0f);

		v1 = (1.0f-h)*(0.5f+a/(2.0f*PI/4.0f))+0.1f;
		v2 = v1+h;
	}

//?	m_pD3DDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, FALSE );
	m_pD3DDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);
	m_pD3DDevice->SetRenderState(D3DRENDERSTATE_AMBIENT, 0xffffffff);
	m_pD3DDevice->SetRenderState(D3DRENDERSTATE_LIGHTING, FALSE );
	m_pD3DDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, FALSE);

	SetTexture(name);
	SetState(D3DSTATEWRAP);

	m_pD3DDevice->SetTransform(D3DTRANSFORMSTATE_VIEW,       &m_matViewBack);
	m_pD3DDevice->SetTransform(D3DTRANSFORMSTATE_PROJECTION, &m_matProjBack);
	m_pD3DDevice->SetTransform(D3DTRANSFORMSTATE_WORLD,      &m_matWorldBack);

	vertex[0] = D3DVERTEX2(D3DVECTOR(p1.x, p1.y, 0.0f), n, u1,v2);
	vertex[1] = D3DVERTEX2(D3DVECTOR(p1.x, p2.y, 0.0f), n, u1,v1);
	vertex[2] = D3DVERTEX2(D3DVECTOR(p2.x, p1.y, 0.0f), n, u2,v2);
	vertex[3] = D3DVERTEX2(D3DVECTOR(p2.x, p2.y, 0.0f), n, u2,v1);

	m_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_VERTEX2, vertex, 4, NULL);
	AddStatisticTriangle(2);
}

// Dessine l'image d'arrière-plan.

void CD3DEngine::DrawBackgroundImage()
{
	FPOINT	p1, p2;
	char	name[50];

	if ( m_bBackgroundQuarter )
	{
		p1.x = 0.0f;
		p1.y = 0.5f;
		p2.x = 0.5f;
		p2.y = 1.0f;
		QuarterName(name, m_backgroundName, 0);
		DrawBackgroundImageQuarter(p1, p2, name);

		p1.x = 0.5f;
		p1.y = 0.5f;
		p2.x = 1.0f;
		p2.y = 1.0f;
		QuarterName(name, m_backgroundName, 1);
		DrawBackgroundImageQuarter(p1, p2, name);

		p1.x = 0.0f;
		p1.y = 0.0f;
		p2.x = 0.5f;
		p2.y = 0.5f;
		QuarterName(name, m_backgroundName, 2);
		DrawBackgroundImageQuarter(p1, p2, name);

		p1.x = 0.5f;
		p1.y = 0.0f;
		p2.x = 1.0f;
		p2.y = 0.5f;
		QuarterName(name, m_backgroundName, 3);
		DrawBackgroundImageQuarter(p1, p2, name);
	}
	else
	{
		p1.x = 0.0f;
		p1.y = 0.0f;
		p2.x = 1.0f;
		p2.y = 1.0f;
		DrawBackgroundImageQuarter(p1, p2, m_backgroundName);
	}
}

// Dessine toutes les planètes.

void CD3DEngine::DrawPlanet()
{
	if ( !m_planet->PlanetExist() )  return;

//?	m_pD3DDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, FALSE );
	m_pD3DDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);
	m_pD3DDevice->SetRenderState(D3DRENDERSTATE_AMBIENT, 0xffffffff);
	m_pD3DDevice->SetRenderState(D3DRENDERSTATE_LIGHTING, FALSE );
	m_pD3DDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, FALSE);

	m_pD3DDevice->SetTransform(D3DTRANSFORMSTATE_VIEW,       &m_matViewBack);
	m_pD3DDevice->SetTransform(D3DTRANSFORMSTATE_PROJECTION, &m_matProjBack);
	m_pD3DDevice->SetTransform(D3DTRANSFORMSTATE_WORLD,      &m_matWorldBack);

	m_planet->Draw();  // dessine les planètes
}

// Dessine l'image d'avant-plan.

void CD3DEngine::DrawLensFlare()
{
	int		i;

	for ( i=0 ; i<m_lensFlareTotal ; i++ )
	{
		DrawLensFlare(i);
	}
}

// Dessine un effet de lentille.

void CD3DEngine::DrawLensFlare(int i)
{
	D3DVERTEX2	vertex[4];	// 2 triangles
	D3DVECTOR	n;
	FPOINT		p1, p2, pos, dim;
	float		u1, u2, v1, v2, angle, intensity;

	n = D3DVECTOR(0.0f, 0.0f, -1.0f);  // normale

	dim.x = m_lensFlareTable[i].dim;
	dim.y = dim.x*(640.0f/480.0f);  // carré sur l'écran

	angle = m_eyeDirH;
	if ( angle <= PI*0.25f || angle >= PI*1.25f )  return;
	angle -= PI*0.25f;
	pos.x = angle/PI;  // 0..1
	intensity = sinf(angle);

	pos.y = Norm((m_eyeDirV-30.0f*PI/180.0f)/(30.0f*PI/180.0f));
	intensity *= 1.0f-pos.y;
	pos.y += 0.25f;

	pos.x -= 0.5f;
	pos.x *= m_lensFlareTable[i].dist;
	pos.x += 0.5f;
	pos.x += m_lensFlareTable[i].offset.x;

	pos.y -= 0.5f;
	pos.y *= m_lensFlareTable[i].dist;
	pos.y += 0.5f;
	pos.y += m_lensFlareTable[i].offset.y;

	p1.x = pos.x-dim.x/2.0f;
	p2.x = pos.x+dim.x/2.0f;
	p1.y = pos.y-dim.y/2.0f;
	p2.y = pos.y+dim.y/2.0f;

	intensity *= m_lensFlareTable[i].intensity;

	u1 = 0.0f;
	v1 = 0.0f;
	u2 = 1.0f;
	v2 = 1.0f;

	vertex[0] = D3DVERTEX2(D3DVECTOR(p1.x, p1.y, 0.0f), n, u1,v2);
	vertex[1] = D3DVERTEX2(D3DVECTOR(p1.x, p2.y, 0.0f), n, u1,v1);
	vertex[2] = D3DVERTEX2(D3DVECTOR(p2.x, p1.y, 0.0f), n, u2,v2);
	vertex[3] = D3DVERTEX2(D3DVECTOR(p2.x, p2.y, 0.0f), n, u2,v1);

	m_pD3DDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);
	m_pD3DDevice->SetRenderState(D3DRENDERSTATE_AMBIENT, 0xffffffff);
	m_pD3DDevice->SetRenderState(D3DRENDERSTATE_LIGHTING, FALSE );
	m_pD3DDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, FALSE);

	SetTexture(m_lensFlareTable[i].texName);
	SetState(D3DSTATETTb, RetColor(intensity));

	m_pD3DDevice->SetTransform(D3DTRANSFORMSTATE_VIEW,       &m_matViewBack);
	m_pD3DDevice->SetTransform(D3DTRANSFORMSTATE_PROJECTION, &m_matProjBack);
	m_pD3DDevice->SetTransform(D3DTRANSFORMSTATE_WORLD,      &m_matWorldBack);

	m_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_VERTEX2, vertex, 4, NULL);
	AddStatisticTriangle(2);
}

// Dessine la couleur d'avant-plan.

void CD3DEngine::DrawOverColor()
{
	D3DLVERTEX	vertex[4];	// 2 triangles
	D3DCOLOR	color[3];
	FPOINT		p1, p2;

	if ( !m_bStateColor )  return;
	if ( (m_overColor == 0x00000000 && m_overMode == D3DSTATETCb) ||
		 (m_overColor == 0xffffffff && m_overMode == D3DSTATETCw) )  return;

	p1.x = 0.0f;
	p1.y = 0.0f;
	p2.x = 1.0f;
	p2.y = 1.0f;

	color[0] = m_overColor;
	color[1] = m_overColor;
	color[2] = 0x00000000;

//?	m_pD3DDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, FALSE );
	m_pD3DDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);
	m_pD3DDevice->SetRenderState(D3DRENDERSTATE_AMBIENT, 0xffffffff);
	m_pD3DDevice->SetRenderState(D3DRENDERSTATE_LIGHTING, FALSE );
	m_pD3DDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, FALSE);

	SetTexture("xxx.tga");  // pas de texture
	SetState(m_overMode);

	m_pD3DDevice->SetTransform(D3DTRANSFORMSTATE_VIEW,       &m_matViewBack);
	m_pD3DDevice->SetTransform(D3DTRANSFORMSTATE_PROJECTION, &m_matProjBack);
	m_pD3DDevice->SetTransform(D3DTRANSFORMSTATE_WORLD,      &m_matWorldBack);

	vertex[0] = D3DLVERTEX(D3DVECTOR(p1.x, p1.y, 0.0f), color[1],color[2], 0.0f,0.0f);
	vertex[1] = D3DLVERTEX(D3DVECTOR(p1.x, p2.y, 0.0f), color[0],color[2], 0.0f,0.0f);
	vertex[2] = D3DLVERTEX(D3DVECTOR(p2.x, p1.y, 0.0f), color[1],color[2], 0.0f,0.0f);
	vertex[3] = D3DLVERTEX(D3DVECTOR(p2.x, p2.y, 0.0f), color[0],color[2], 0.0f,0.0f);

	m_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_LVERTEX, vertex, 4, NULL);
	AddStatisticTriangle(2);
}


// Donne la liste des rangs des objets et sous-objets sélectionnés.

void CD3DEngine::SetHiliteRank(int *rankList)
{
	int		i;

	i = 0;
	while ( *rankList != -1 )
	{
		m_hiliteRank[i++] = *rankList++;
	}
	m_hiliteRank[i] = -1;  // terminateur
}

// Donne la bbox dans l'écran 2D d'un objet quelconque.

BOOL CD3DEngine::GetBBox2D(int objRank, FPOINT &min, FPOINT &max)
{
	D3DVECTOR	p, pp;
	int			i;

	min.x =  1000000.0f;
	min.y =  1000000.0f;
	max.x = -1000000.0f;
	max.y = -1000000.0f;

	for ( i=0 ; i<8 ; i++ )
	{
		if ( i & (1<<0) )  p.x = m_objectParam[objRank].bboxMin.x;
		else               p.x = m_objectParam[objRank].bboxMax.x;
		if ( i & (1<<1) )  p.y = m_objectParam[objRank].bboxMin.y;
		else               p.y = m_objectParam[objRank].bboxMax.y;
		if ( i & (1<<2) )  p.z = m_objectParam[objRank].bboxMin.z;
		else               p.z = m_objectParam[objRank].bboxMax.z;
		if ( TransformPoint(pp, objRank, p) )
		{
			if ( pp.x < min.x )  min.x = pp.x;
			if ( pp.x > max.x )  max.x = pp.x;
			if ( pp.y < min.y )  min.y = pp.y;
			if ( pp.y > max.y )  max.y = pp.y;
		}
	}
	
	if ( min.x ==  1000000.0f ||
		 min.y ==  1000000.0f ||
		 max.x == -1000000.0f ||
		 max.y == -1000000.0f )  return FALSE;

	return TRUE;
}

// Détermine le rectangle de l'objet mis en évidence, qui sera
// dessiné par CD3DApplication.

void CD3DEngine::DrawHilite()
{
	FPOINT	min, max, omin, omax;
	int		i;

	min.x =  1000000.0f;
	min.y =  1000000.0f;
	max.x = -1000000.0f;
	max.y = -1000000.0f;

	i = 0;
	while ( m_hiliteRank[i] != -1 )
	{
		if ( GetBBox2D(m_hiliteRank[i++], omin, omax) )
		{
			min.x = Min(min.x, omin.x);
			min.y = Min(min.y, omin.y);
			max.x = Max(max.x, omax.x);
			max.y = Max(max.y, omax.y);
		}
	}

	if ( min.x ==  1000000.0f ||
		 min.y ==  1000000.0f ||
		 max.x == -1000000.0f ||
		 max.y == -1000000.0f )
	{
		m_bHilite = FALSE;  // pas de mise en évidence
	}
	else
	{
		m_hiliteP1 = min;
		m_hiliteP2 = max;
		m_bHilite = TRUE;
	}
}

// Donne le rectangle de mise en évidence à dessiner
// par CD3DApplication.

BOOL CD3DEngine::GetHilite(FPOINT &p1, FPOINT &p2)
{
	p1 = m_hiliteP1;
	p2 = m_hiliteP2;
	return m_bHilite;
}

// Détermine le rectangle de sélection.

void CD3DEngine::SetSelect(FPOINT p1, FPOINT p2, BOOL bSelect)
{
	m_selectP1 = p1;
	m_selectP2 = p2;
	m_bSelect = bSelect;
}

// Donne le rectangle de sélection.

BOOL CD3DEngine::GetSelect(FPOINT &p1, FPOINT &p2)
{
	p1 = m_selectP1;
	p2 = m_selectP2;
	return m_bSelect;
}


// Ajoute qq triangles rendus pour les statistiques.

void CD3DEngine::AddStatisticTriangle(int nb)
{
	m_statisticTriangle += nb;
}

// Retourne le nombre de triangles rendus.

int CD3DEngine::RetStatisticTriangle()
{
	return m_statisticTriangle;
}

BOOL CD3DEngine::GetSpriteCoord(int &x, int &y)
{
	D3DVIEWPORT7	vp;
	D3DVECTOR		v, vv;

	return FALSE;
	//?
	vv = D3DVECTOR(0.0f, 0.0f, 0.0f);
	if ( !TransformPoint(v, 20*20+1, vv) )  return FALSE;

	m_pD3DDevice->GetViewport(&vp);
	v.x *= vp.dwWidth/2;
	v.y *= vp.dwHeight/2;
	v.x = v.x+vp.dwWidth/2;
	v.y = vp.dwHeight-(v.y+vp.dwHeight/2);
	
	x = (int)v.x;
	y = (int)v.y;
	return TRUE;
}


// Teste s'il faut exclure un point.

BOOL IsExcludeColor(FPOINT *pExclu, int x, int y)
{
	int		i;

	i = 0;
	while ( pExclu[i+0].x != 0.0f || pExclu[i+0].y != 0.0f ||
			pExclu[i+1].y != 0.0f || pExclu[i+1].y != 0.0f )
	{
		if ( x >= (int)(pExclu[i+0].x*256.0f) &&
			 x <  (int)(pExclu[i+1].x*256.0f) &&
			 y >= (int)(pExclu[i+0].y*256.0f) &&
			 y <  (int)(pExclu[i+1].y*256.0f) )  return TRUE;  // exclure

		i += 2;
	}

	return FALSE;  // point à inclure
}

// Change la couleur d'une texture.

BOOL CD3DEngine::ChangeColor(char *name,
							 D3DCOLORVALUE colorRef1, D3DCOLORVALUE colorNew1,
							 D3DCOLORVALUE colorRef2, D3DCOLORVALUE colorNew2,
							 float tolerance1, float tolerance2,
							 FPOINT ts, FPOINT ti,
							 FPOINT *pExclu, float shift, BOOL bHSV)
{
	LPDIRECTDRAWSURFACE7	surface;
	DDSURFACEDESC2			ddsd;
	D3DCOLORVALUE			color;
	ColorHSV				cr1, cn1, cr2, cn2, c;
	int						dx, dy, x, y, sx, sy, ex, ey;

	D3DTextr_Invalidate(name);
	LoadTexture(name);  // recharge la texture initiale

	if ( colorRef1.r == colorNew1.r &&
		 colorRef1.g == colorNew1.g &&
		 colorRef1.b == colorNew1.b &&
		 colorRef2.r == colorNew2.r &&
		 colorRef2.g == colorNew2.g &&
		 colorRef2.b == colorNew2.b )  return TRUE;

	surface = D3DTextr_GetSurface(name);
	if ( surface == 0 )  return FALSE;

	ZeroMemory(&ddsd, sizeof(DDSURFACEDESC2));
	ddsd.dwSize = sizeof(DDSURFACEDESC2);
	if ( surface->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL) != DD_OK )  return FALSE;

	dx = ddsd.dwWidth;
	dy = ddsd.dwHeight;

	sx = (int)(ts.x*dx);
	sy = (int)(ts.y*dy);
	ex = (int)(ti.x*dx);
	ey = (int)(ti.y*dy);

	RGB2HSV(colorRef1, cr1);
	RGB2HSV(colorNew1, cn1);
	RGB2HSV(colorRef2, cr2);
	RGB2HSV(colorNew2, cn2);

	for ( y=sy ; y<ey ; y++ )
	{
		for ( x=sx ; x<ex ; x++ )
		{
			if ( pExclu != 0 && IsExcludeColor(pExclu, x,y) )  continue;

			color = GetDot(&ddsd, x, y);

			if ( bHSV )
			{
				RGB2HSV(color, c);
				if ( c.s > 0.01f && Abs(c.h-cr1.h) < tolerance1 )
				{
					c.h += cn1.h-cr1.h;
					c.s += cn1.s-cr1.s;
					c.v += cn1.v-cr1.v;
					if ( c.h < 0.0f )  c.h -= 1.0f;
					if ( c.h > 1.0f )  c.h += 1.0f;
					HSV2RGB(c, color);
					color.r += shift;
					color.g += shift;
					color.b += shift;
					::SetDot(&ddsd, x, y, color);
				}
				else
				if ( tolerance2 != -1.0f &&
					 c.s > 0.01f && Abs(c.h-cr2.h) < tolerance2 )
				{
					c.h += cn2.h-cr2.h;
					c.s += cn2.s-cr2.s;
					c.v += cn2.v-cr2.v;
					if ( c.h < 0.0f )  c.h -= 1.0f;
					if ( c.h > 1.0f )  c.h += 1.0f;
					HSV2RGB(c, color);
					color.r += shift;
					color.g += shift;
					color.b += shift;
					::SetDot(&ddsd, x, y, color);
				}
			}
			else
			{
				if ( Abs(color.r-colorRef1.r)+
					 Abs(color.g-colorRef1.g)+
					 Abs(color.b-colorRef1.b) < tolerance1*3.0f )
				{
					color.r = colorNew1.r+color.r-colorRef1.r+shift;
					color.g = colorNew1.g+color.g-colorRef1.g+shift;
					color.b = colorNew1.b+color.b-colorRef1.b+shift;
					::SetDot(&ddsd, x, y, color);
				}
				else
				if ( tolerance2 != -1 &&
					 Abs(color.r-colorRef2.r)+
					 Abs(color.g-colorRef2.g)+
					 Abs(color.b-colorRef2.b) < tolerance2*3.0f )
				{
					color.r = colorNew2.r+color.r-colorRef2.r+shift;
					color.g = colorNew2.g+color.g-colorRef2.g+shift;
					color.b = colorNew2.b+color.b-colorRef2.b+shift;
					::SetDot(&ddsd, x, y, color);
				}
			}
		}
	}

	surface->Unlock(NULL);
	return TRUE;
}


// Ouvre une image pour travailler directement dedans.

BOOL CD3DEngine::OpenImage(char *name)
{
//?	D3DTextr_Invalidate(name);
//?	LoadTexture(name);

	m_imageSurface = D3DTextr_GetSurface(name);
	if ( m_imageSurface == 0 )  return FALSE;

	ZeroMemory(&m_imageDDSD, sizeof(DDSURFACEDESC2));
	m_imageDDSD.dwSize = sizeof(DDSURFACEDESC2);
	if ( m_imageSurface->Lock(NULL, &m_imageDDSD, DDLOCK_WAIT, NULL) != DD_OK )
	{
		return FALSE;
	}

	if ( m_imageDDSD.ddpfPixelFormat.dwRGBBitCount != 16 )
	{
		m_imageSurface->Unlock(NULL);
		return FALSE;
	}

	m_imageDX = m_imageDDSD.dwWidth;
	m_imageDY = m_imageDDSD.dwHeight;

	return TRUE;
}

// Copie l'image de travail.

BOOL CD3DEngine::CopyImage()
{
	WORD*		pbSurf;
	int			y;

	if ( m_imageCopy == 0 )
	{
		m_imageCopy = (WORD*)malloc(m_imageDX*m_imageDY*sizeof(WORD));
	}

	for ( y=0 ; y<m_imageDY ; y++ )
	{
		pbSurf = (WORD*)m_imageDDSD.lpSurface;
		pbSurf += m_imageDDSD.lPitch*y/2;
		memcpy(m_imageCopy+y*m_imageDX, pbSurf, m_imageDX*sizeof(WORD));
	}
	return TRUE;
}

// Restitue l'image de travail.

BOOL CD3DEngine::LoadImage()
{
	WORD*		pbSurf;
	int			y;

	if ( m_imageCopy == 0 )  return FALSE;

	for ( y=0 ; y<m_imageDY ; y++ )
	{
		pbSurf = (WORD*)m_imageDDSD.lpSurface;
		pbSurf += m_imageDDSD.lPitch*y/2;
		memcpy(pbSurf, m_imageCopy+y*m_imageDX, m_imageDX*sizeof(WORD));
	}
	return TRUE;
}

// Scroll la copie de l'image de travial.

BOOL CD3DEngine::ScrollImage(int dx, int dy)
{
	int		x, y;

	if ( dx > 0 )
	{
		for ( y=0 ; y<m_imageDY ; y++ )
		{
			for ( x=0 ; x<m_imageDX-dx ; x++ )
			{
				m_imageCopy[x+y*m_imageDX] = m_imageCopy[x+dx+y*m_imageDX];
			}
		}
	}

	if ( dx < 0 )
	{
		for ( y=0 ; y<m_imageDY ; y++ )
		{
			for ( x=m_imageDX-1 ; x>=-dx ; x-- )
			{
				m_imageCopy[x+y*m_imageDX] = m_imageCopy[x+dx+y*m_imageDX];
			}
		}
	}

	if ( dy > 0 )
	{
		for ( y=0 ; y<m_imageDY-dy ; y++ )
		{
			memcpy(m_imageCopy+y*m_imageDX, m_imageCopy+(y+dy)*m_imageDX, m_imageDX*sizeof(WORD));
		}
	}

	if ( dy < 0 )
	{
		for ( y=m_imageDY-1 ; y>=-dy ; y-- )
		{
			memcpy(m_imageCopy+y*m_imageDX, m_imageCopy+(y+dy)*m_imageDX, m_imageDX*sizeof(WORD));
		}
	}

	return TRUE;
}

// Dessine un point dans l'image de travail.

BOOL CD3DEngine::SetDot(int x, int y, D3DCOLORVALUE color)
{
	WORD*		pbSurf;
	WORD		r,g,b, w;

	if ( x < 0 || x >= m_imageDX ||
		 y < 0 || y >= m_imageDY )  return FALSE;

#if 1
	if ( color.r < 0.0f )  color.r = 0.0f;
	if ( color.r > 1.0f )  color.r = 1.0f;
	if ( color.g < 0.0f )  color.g = 0.0f;
	if ( color.g > 1.0f )  color.g = 1.0f;
	if ( color.b < 0.0f )  color.b = 0.0f;
	if ( color.b > 1.0f )  color.b = 1.0f;

	r = (int)(color.r*32.0f);
	g = (int)(color.g*32.0f);
	b = (int)(color.b*32.0f);

	if ( r >= 32 )  r = 31;  // 5 bits
	if ( g >= 32 )  g = 31;  // 5 bits
	if ( b >= 32 )  b = 31;  // 5 bits
#else
	r = (int)(color.r*31.0f);
	g = (int)(color.g*31.0f);
	b = (int)(color.b*31.0f);
#endif

	if ( m_imageDDSD.ddpfPixelFormat.dwRBitMask == 0xf800 )  // 565 ?
	{
		w = (r<<11)|(g<<6)|b;
	}
	else if ( m_imageDDSD.ddpfPixelFormat.dwRBitMask == 0x7c00 )  // 555 ?
	{
		w = (r<<10)|(g<<5)|b;
	}
	else
	{
		w = -1;  // blanc
	}

	pbSurf = (WORD*)m_imageDDSD.lpSurface;
	pbSurf += m_imageDDSD.lPitch*y/2;
	pbSurf += x;

	*pbSurf = w;
	return TRUE;
}

// Ferme l'image de travail.

BOOL CD3DEngine::CloseImage()
{
	m_imageSurface->Unlock(NULL);
	return TRUE;
}


// Ecrit un fichier .BMP copie d'écran.

BOOL CD3DEngine::WriteScreenShot(char *filename, int width, int height)
{
	return m_app->WriteScreenShot(filename, width, height);
}


// Retourne le pointeur à la classe CText.

CText* CD3DEngine::RetText()
{
	return m_text;
}


// Gestion du texte d'informations affiché dans la fenêtre.

void CD3DEngine::SetInfoText(int line, char* text)
{
	strcpy(m_infoText[line], text);
}

char* CD3DEngine::RetInfoText(int line)
{
	return m_infoText[line];
}



// Spécifie la focale de la caméra.
//	0.75 = normal
//	1.50 = grand-angle

void CD3DEngine::SetFocus(float focus)
{
	D3DVIEWPORT7	vp;
	float			fAspect;

	m_focus = focus;

	if ( m_pD3DDevice != 0 )
	{
		m_pD3DDevice->GetViewport(&vp);
		m_dim.x = vp.dwWidth;
		m_dim.y = vp.dwHeight;
	}

	fAspect = ((float)m_dim.y) / m_dim.x;
//?	D3DUtil_SetProjectionMatrix(m_matProj, m_focus, fAspect, 0.5f, m_deepView[m_rankView]);
	D3DUtil_SetProjectionMatrix(m_matProj, m_focus, fAspect, 0.5f, m_deepView[0]);
}

float CD3DEngine::RetFocus()
{
	return m_focus;
}

// 

void CD3DEngine::UpdateMatProj()
{
	m_pD3DDevice->SetTransform(D3DTRANSFORMSTATE_PROJECTION, &m_matProj);
}



// Ignore les touches pressées.

void CD3DEngine::FlushPressKey()
{
	m_app->FlushPressKey();
}

// Remet les touches par défaut.

void CD3DEngine::ResetKey()
{
	m_app->ResetKey();
}

// Modifie une touche.

void CD3DEngine::SetKey(int keyRank, int option, int key)
{
	m_app->SetKey(keyRank, option, key);
}

// Donne une touche.

int CD3DEngine::RetKey(int keyRank, int option)
{
	return m_app->RetKey(keyRank, option);
}

// Indique si une touche est produite par la souris.

BOOL CD3DEngine::IsKeyMouse(int key)
{
	return m_app->IsKeyMouse(key);
}

// Indique si une touche est produite par le joystick.

BOOL CD3DEngine::IsKeyJoystick(int key)
{
	return m_app->IsKeyJoystick(key);
}


// Gestion de la force de l'effet FFB.

void CD3DEngine::SetForce(float force)
{
	m_app->SetForce(force);
}

float CD3DEngine::RetForce()
{
	return m_app->RetForce();
}

// Gestion du force feedback.

void CD3DEngine::SetFFB(BOOL bMode)
{
	m_app->SetFFB(bMode);
}

BOOL CD3DEngine::RetFFB()
{
	return m_app->RetFFB();
}

// Utilise le joystick ou le clavier.

void CD3DEngine::SetJoystick(int mode)
{
	m_app->SetJoystick(mode);
}

int CD3DEngine::RetJoystick()
{
	return m_app->RetJoystick();
}

BOOL CD3DEngine::SetJoyForces(float forceX, float forceY)
{
	return m_app->SetJoyForces(forceX, forceY);
}


void CD3DEngine::SetDebugMode(BOOL bMode)
{
	m_app->SetDebugMode(bMode);
}

BOOL CD3DEngine::RetDebugMode()
{
	return m_app->RetDebugMode();
}


// Indique si un point est visible.

BOOL CD3DEngine::IsVisiblePoint(const D3DVECTOR &pos)
{
	return ( Length(m_eyePt, pos) <= m_deepView[0] );
}


// Initialize scene objects.

HRESULT CD3DEngine::InitDeviceObjects()
{
	// Set miscellaneous render states.
	m_pD3DDevice->SetRenderState(D3DRENDERSTATE_DITHERENABLE,   TRUE);
	m_pD3DDevice->SetRenderState(D3DRENDERSTATE_SPECULARENABLE, TRUE);
	m_pD3DDevice->SetRenderState(D3DRENDERSTATE_SHADEMODE, D3DSHADE_GOURAUD);
	m_pD3DDevice->SetRenderState(D3DRENDERSTATE_FILLMODE, D3DFILL_SOLID);

	// Set up the textures.
	D3DTextr_RestoreAllTextures(m_pD3DDevice);
	m_pD3DDevice->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTFN_LINEAR);
	m_pD3DDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTFG_LINEAR);
	m_pD3DDevice->SetTextureStageState(1, D3DTSS_MINFILTER, D3DTFN_LINEAR);
	m_pD3DDevice->SetTextureStageState(1, D3DTSS_MAGFILTER, D3DTFG_LINEAR);

	SetFocus(m_focus);

	// Définitions des matrices pour le fond.
	D3DUtil_SetIdentityMatrix(m_matWorldBack);

	D3DUtil_SetIdentityMatrix(m_matViewBack);
	m_matViewBack._41 = -0.5f;
	m_matViewBack._42 = -0.5f;
	m_matViewBack._43 =  1.0f;

	D3DUtil_SetIdentityMatrix(m_matProjBack);
	m_matProjBack._11 =  2.0f;
	m_matProjBack._22 =  2.0f;
	m_matProjBack._34 =  1.0f;
	m_matProjBack._43 = -1.0f;
	m_matProjBack._44 =  0.0f;

	// Définitions des matrices pour l'interface.
	D3DUtil_SetIdentityMatrix(m_matWorldInterface);

	D3DUtil_SetIdentityMatrix(m_matViewInterface);
	m_matViewInterface._41 = -0.5f;
	m_matViewInterface._42 = -0.5f;
	m_matViewInterface._43 =  1.0f;

	D3DUtil_SetIdentityMatrix(m_matProjInterface);
	m_matProjInterface._11 =  2.0f;
	m_matProjInterface._22 =  2.0f;
	m_matProjInterface._34 =  1.0f;
	m_matProjInterface._43 = -1.0f;
	m_matProjInterface._44 =  0.0f;
	
	return S_OK;
}

// Spécifie le centre, le zoom et la rotation pour toute l'interface.
// Le mode normal est center=0.5;0.5, zoom=1 et angle=0.

void CD3DEngine::SetInterfaceMat(FPOINT center, float zoom, float angle)
{
	m_matViewInterface._22 =  cosf(angle);
	m_matViewInterface._23 = -sinf(angle);
	m_matViewInterface._32 =  sinf(angle);
	m_matViewInterface._33 =  cosf(angle);  // rotation autour de X

	m_matViewInterface._41 = -center.x;
	m_matViewInterface._42 = -center.y;  // translation

	m_matProjInterface._11 = zoom*2.0f;
	m_matProjInterface._22 = zoom*2.0f;  // zoom
}


// Restore all surfaces.

HRESULT CD3DEngine::RestoreSurfaces()
{
	return S_OK;
}


// Called when the app is exitting, or the device is being changed,
// this function deletes any device dependant objects.

HRESULT CD3DEngine::DeleteDeviceObjects()
{
    D3DTextr_InvalidateAllTextures();
    return S_OK;
}


// Called before the app exits, this function gives the app the chance
// to cleanup after itself.

HRESULT CD3DEngine::FinalCleanup()
{
    return S_OK;
}


// Overrrides the main WndProc, so the sample can do custom message 
// handling (e.g. processing mouse, keyboard, or menu commands).

LRESULT CD3DEngine::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
#if 0
	if ( uMsg == WM_KEYDOWN )  // Alt+key ?
	{
		if ( wParam == 'Q' )
		{
			debug_blend1 ++;
			if ( debug_blend1 > 13 )  debug_blend1 = 0;
		}
		if ( wParam == 'W' )
		{
			debug_blend2 ++;
			if ( debug_blend2 > 13 )  debug_blend2 = 0;
		}
		if ( wParam == 'E' )
		{
			debug_blend3 ++;
			if ( debug_blend3 > 13 )  debug_blend3 = 0;
		}
		if ( wParam == 'R' )
		{
			debug_blend4 ++;
			if ( debug_blend4 > 13 )  debug_blend4 = 0;
		}
		char s[100];
		sprintf(s, "src=%d, dest=%d, src=%d, dest=%d", debug_blend1, debug_blend2, debug_blend3, debug_blend4);
		SetInfoText(4, s);
	}
#endif

#if 1
	if ( uMsg == WM_SYSKEYDOWN )  // Alt+key ?
	{
		if ( wParam == VK_F7 )  // Alt+F7 ?
		{
			s_resol = 0;
		}
		if ( wParam == VK_F8 )  // Alt+F8 ?
		{
			s_resol = 1;
		}
		if ( wParam == VK_F9 )  // Alt+F9 ?
		{
			s_resol = 2;
		}
		if ( wParam == VK_F10 )  // Alt+F10 ?
		{
			s_resol = 3;
		}
	}
#endif

	return 0;
}


// Gestion de la souris.

void CD3DEngine::MoveMousePos(FPOINT pos)
{
	m_mousePos = pos;
	m_app->SetMousePos(pos);
}

void CD3DEngine::SetMousePos(FPOINT pos)
{
	m_mousePos = pos;
}

FPOINT CD3DEngine::RetMousePos()
{
	return m_mousePos;
}

void CD3DEngine::SetMouseType(D3DMouse type)
{
	m_mouseType = type;
}

D3DMouse CD3DEngine::RetMouseType()
{
	return m_mouseType;
}

void CD3DEngine::SetMouseHide(BOOL bHide)
{
	if ( m_bMouseHide == bHide )  return;

	if ( bHide )  // cache la souris ?
	{
		if ( m_setup[ST_NICEMOUSE] == 0.0f )
		{
			SetSetup(ST_NICEMOUSE, 1.0f);
		}
		m_bMouseHide = TRUE;
	}
	else	// montre la souris ?
	{
		if ( m_setup[ST_NICEMOUSE] == 0.0f )
		{
			SetSetup(ST_NICEMOUSE, 0.0f);
		}
		m_bMouseHide = FALSE;
	}
}

BOOL CD3DEngine::RetMouseHide()
{
	return m_bMouseHide;
}

BOOL CD3DEngine::RetNiceMouseCap()
{
	return m_app->RetNiceMouseCap();
}

void CD3DEngine::SetMouseCapture()
{
	m_app->SetMouseCapture();
}

void CD3DEngine::ReleaseMouseCapture()
{
	m_app->ReleaseMouseCapture();
}


// Initialise le buffer pour les zones bloquées.

void CD3DEngine::InitLockZone()
{
	int		x, y, channel;

	if ( m_lockZoneBuffer != 0 )
	{
		for ( y=0 ; y<100 ; y++ )
		{
			for ( x=0 ; x<100 ; x++ )
			{
				channel = m_lockZoneBuffer[x+y*100];
				if ( channel != 0 )
				{
					m_particule->DeleteParticule(channel);
					m_lockZoneBuffer[x+y*100] = 0;
				}
			}
		}

		free(m_lockZoneBuffer);
		m_lockZoneBuffer = 0;
	}
}

// Montre une zone bloquée.

void CD3DEngine::ShowLockZone(D3DVECTOR pos, BOOL bLock)
{
	D3DVECTOR	speed;
	FPOINT		dim;
	int			x, y, channel;

	pos = Grid(pos, 8.0f);
	x = (int)(pos.x/8.0f)+50;
	y = (int)(pos.z/8.0f)+50;
	if ( x < 0 || x >= 100 || y < 0 || y >= 100 )  return;

	CreateLockZone();

	if ( bLock )
	{
		if ( m_lockZoneBuffer[x+y*100] == 0 )
		{
			speed = D3DVECTOR(0.0f, 0.0f, 0.0f);
			dim.x = 4.2f;
			dim.y = dim.x;
			channel = m_particule->CreateParticule(pos, speed, dim, PARTILOCKZONE);
			m_lockZoneBuffer[x+y*100] = channel;
		}
	}
	else
	{
		channel = m_lockZoneBuffer[x+y*100];
		if ( channel != 0 )
		{
			m_particule->DeleteParticule(channel);
			m_lockZoneBuffer[x+y*100] = 0;
		}
	}
}

// Crée le buffer pour pouvoir montrer les zones.

void CD3DEngine::CreateLockZone()
{
	if ( m_lockZoneBuffer == 0 )
	{
		m_lockZoneBuffer = (int*)malloc(sizeof(int)*100*100);
		ZeroMemory(m_lockZoneBuffer, sizeof(int)*100*100);
	}
}


// Dessine le sprite de la souris.

void CD3DEngine::DrawMouse()
{
	D3DMATERIAL7	material;
	FPOINT			pos, ppos, dim;
	int				i;

	typedef struct
	{
		D3DMouse	type;
		int			icon1, icon2, iconShadow;
		int			mode1, mode2;
		float		hotx, hoty;
	}
	Mouse;

	static Mouse table[] =
	{
		{ D3DMOUSENORM,    44,45,53, D3DSTATETTw, D3DSTATETTb,  1.0f,  1.0f},
		{ D3DMOUSEWAIT,    38,39,-1, D3DSTATETTw, D3DSTATETTb,  8.0f, 12.0f},
		{ D3DMOUSEHAND,    46,47,54, D3DSTATETTw, D3DSTATETTb,  7.0f,  2.0f},
		{ D3DMOUSEEDIT,    62,63,-1, D3DSTATETTb, D3DSTATETTw,  6.0f, 10.0f},
		{ D3DMOUSESELECT,  60,61,-1, D3DSTATETTw, D3DSTATETTb, 16.0f, 16.0f},
		{ D3DMOUSEGOTO,    58,59,-1, D3DSTATETTw, D3DSTATETTb, 16.0f, 16.0f},
		{ D3DMOUSESCROLLL, 56,57,-1, D3DSTATETTb, D3DSTATETTw, 16.0f, 16.0f},
		{ D3DMOUSESCROLLR, 56,57,-1, D3DSTATETTb, D3DSTATETTw, 16.0f, 16.0f},
		{ D3DMOUSESCROLLU, 56,57,-1, D3DSTATETTb, D3DSTATETTw, 16.0f, 16.0f},
		{ D3DMOUSESCROLLD, 56,57,-1, D3DSTATETTb, D3DSTATETTw, 16.0f, 16.0f},
		{ D3DMOUSEHIDE },
	};

	if ( m_bMouseHide )  return;
	if ( m_setup[ST_NICEMOUSE] == 0.0f )  return;  // souris windows ?

	ZeroMemory( &material, sizeof(D3DMATERIAL7) );
	material.diffuse.r = 1.0f;
	material.diffuse.g = 1.0f;
	material.diffuse.b = 1.0f;
	material.ambient.r = 0.5f;
	material.ambient.g = 0.5f;
	material.ambient.b = 0.5f;
	SetMaterial(material);

	SetTexture("button1.tga");

	i = 0;
	while ( table[i].type != D3DMOUSEHIDE )
	{
		if ( m_mouseType == table[i].type )
		{
			dim.x = 0.05f*0.75f;
			dim.y = 0.05f;

			pos.x = m_mousePos.x - (table[i].hotx*dim.x)/32.0f;
			pos.y = m_mousePos.y - ((32.0f-table[i].hoty)*dim.y)/32.0f;

			ppos.x = pos.x+(4.0f/640.0f);
			ppos.y = pos.y-(3.0f/480.0f);
			SetState(D3DSTATETTw);
			DrawSprite(ppos, dim, table[i].iconShadow);

			SetState(table[i].mode1);
			DrawSprite(pos, dim, table[i].icon1);

			SetState(table[i].mode2);
			DrawSprite(pos, dim, table[i].icon2);
			break;
		}
		i ++;
	}
}

// Dessine le sprite de la souris.

void CD3DEngine::DrawSprite(FPOINT pos, FPOINT dim, int icon)
{
	D3DVERTEX2	vertex[4];	// 2 triangles
	FPOINT		p1, p2;
	D3DVECTOR	n;
	float		u1, u2, v1, v2, dp;

	if ( icon == -1 )  return;

	p1.x = pos.x;
	p1.y = pos.y;
	p2.x = pos.x + dim.x;
	p2.y = pos.y + dim.y;

	u1 = (32.0f/256.0f)*(icon%8);
	v1 = (32.0f/256.0f)*(icon/8);  // u-v texture
	u2 = (32.0f/256.0f)+u1;
	v2 = (32.0f/256.0f)+v1;

	dp = 0.5f/256.0f;
	u1 += dp;
	v1 += dp;
	u2 -= dp;
	v2 -= dp;

	n = D3DVECTOR(0.0f, 0.0f, -1.0f);  // normale

	vertex[0] = D3DVERTEX2(D3DVECTOR(p1.x, p1.y, 0.0f), n, u1,v2);
	vertex[1] = D3DVERTEX2(D3DVECTOR(p1.x, p2.y, 0.0f), n, u1,v1);
	vertex[2] = D3DVERTEX2(D3DVECTOR(p2.x, p1.y, 0.0f), n, u2,v2);
	vertex[3] = D3DVERTEX2(D3DVECTOR(p2.x, p2.y, 0.0f), n, u2,v1);

	m_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_VERTEX2, vertex, 4, NULL);
	AddStatisticTriangle(2);
}

