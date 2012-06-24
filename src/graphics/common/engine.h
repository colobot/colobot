// * This file is part of the COLOBOT source code
// * Copyright (C) 2001-2008, Daniel ROUX & EPSITEC SA, www.epsitec.ch
// * Copyright (C) 2012, Polish Portal of Colobot (PPC)
// *
// * This program is free software: you can redistribute it and/or modify
// * it under the terms of the GNU General Public License as published by
// * the Free Software Foundation, either version 3 of the License, or
// * (at your option) any later version.
// *
// * This program is distributed in the hope that it will be useful,
// * but WITHOUT ANY WARRANTY; without even the implied warranty of
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// * GNU General Public License for more details.
// *
// * You should have received a copy of the GNU General Public License
// * along with this program. If not, see  http://www.gnu.org/licenses/.

// engine.h

#pragma once


#include "graphics/common/color.h"
#include "graphics/common/material.h"
#include "graphics/common/vertex.h"
#include "math/intpoint.h"
#include "math/matrix.h"
#include "math/point.h"
#include "math/vector.h"


class CApplication;
class CInstanceManager;
class CObject;
class CSound;


namespace Gfx {

class CDevice;
class CLight;
class CText;
class CParticle;
class CWater;
class CCloud;
class CLightning;
class CPlanet;
class CTerrain;


//const int MAXOBJECT		= 1200;
//const int MAXSHADOW		= 500;
//const int MAXGROUNDSPOT	= 100;


enum ObjectType
{
	//! Object doesn't exist
	OBJTYPE_NULL		= 0,
	//! Terrain
	OBJTYPE_TERRAIN		= 1,
	//! Fixed object
	OBJTYPE_FIX			= 2,
	//! Moving object
	OBJTYPE_VEHICULE	= 3,
	//! Part of a moving object
	OBJTYPE_DESCENDANT	= 4,
	//! Fixed object type quartz
	OBJTYPE_QUARTZ		= 5,
	//! Fixed object type metal
	OBJTYPE_METAL		= 6
};

enum TriangleType
{
	//! triangles
	TRIANGLE_TYPE_6T		= 1,
	//! surfaces
	TRIANGLE_TYPE_6S		= 2
};

enum Mapping
{
	MAPPING_X		= 1,
	MAPPING_Y		= 2,
	MAPPING_Z		= 3,
	MAPPING_1X		= 4,
	MAPPING_1Y		= 5,
	MAPPING_1Z		= 6
};

enum MouseType
{
	MOUSE_HIDE		= 0,		// no mouse
	MOUSE_NORM		= 1,
	MOUSE_WAIT		= 2,
	MOUSE_EDIT		= 3,
	MOUSE_HAND		= 4,
	MOUSE_CROSS		= 5,
	MOUSE_SHOW		= 6,
	MOUSE_NO		= 7,
	MOUSE_MOVE		= 8,		// +
	MOUSE_MOVEH		= 9,		// -
	MOUSE_MOVEV		= 10,		// |
	MOUSE_MOVED		= 11,		// /
	MOUSE_MOVEI		= 12,		// \ //
	MOUSE_SCROLLL	= 13,		// <<
	MOUSE_SCROLLR	= 14,		// >>
	MOUSE_SCROLLU	= 15,		// ^
	MOUSE_SCROLLD	= 16,		// v
	MOUSE_TARGET	= 17
};

enum ShadowType
{
	SHADOW_NORM	= 0,
	SHADOW_WORM	= 1
};

enum RenderState
{
	//! Normal opaque materials
	RSTATE_NORMAL			= 0,
	//! The transparent texture (black = no)
	RSTATE_TTEXTURE_BLACK	= (1<<0),
	//! The transparent texture (white = no)
	RSTATE_TTEXTURE_WHITE	= (1<<1),
	//! The transparent diffuse color
	RSTATE_TDIFFUSE			= (1<<2),
	//! Texture wrap
	RSTATE_WRAP				= (1<<3),
	//! Texture borders with solid color
	RSTATE_CLAMP			= (1<<4),
	//! Light texture (ambient max)
	RSTATE_LIGHT			= (1<<5),
	//! Double black texturing
	RSTATE_DUAL_BLACK		= (1<<6),
	//! Double white texturing
	RSTATE_DUAL_WHITE		= (1<<7),
	//! Part 1 (no change in. MOD!)
	RSTATE_PART1			= (1<<8),
	//! Part 2
	RSTATE_PART2			= (1<<9),
	//! Part 3
	RSTATE_PART3			= (1<<10),
	//! Part 4
	RSTATE_PART4			= (1<<11),
	//! Double-sided face
	RSTATE_2FACE			= (1<<12),
	//! Image using alpha channel
	RSTATE_ALPHA			= (1<<13),
	//! Always use 2nd floor texturing
	RSTATE_SECOND			= (1<<14),
	//! Causes the fog
	RSTATE_FOG				= (1<<15),
	//! The transparent color (black = no)
	RSTATE_TCOLOR_BLACK		= (1<<16),
	//! The transparent color (white = no)
	RSTATE_TCOLOR_WHITE		= (1<<17)
};


struct Triangle
{
	Gfx::VertexTex2		triangle[3];
	Gfx::Material		material;
	int					state;
	char				texName1[20];
	char				texName2[20];
};


struct ObjLevel6
{
	int					totalPossible;
	int					totalUsed;
	Gfx::Material		material;
	int					state;
	Gfx::TriangleType	type;
	Gfx::VertexTex2	vertex[1];
};

struct ObjLevel5
{
	int				totalPossible;
	int				totalUsed;
	int				reserve;
	Gfx::ObjLevel6*	table[1];
};

struct ObjLevel4
{
	int				totalPossible;
	int				totalUsed;
	float			min, max;
	Gfx::ObjLevel5*	table[1];
};

struct ObjLevel3
{
	int				totalPossible;
	int				totalUsed;
	int				objRank;
	Gfx::ObjLevel4*	table[1];
};

struct ObjLevel2
{
	int				totalPossible;
	int				totalUsed;
	char			texName1[20];
	char			texName2[20];
	Gfx::ObjLevel3*	table[1];
};

struct ObjLevel1
{
	int				totalPossible;
	int				totalUsed;
	Gfx::ObjLevel2*	table[1];
};


struct Object
{
	bool				used;			// true -> object exists
	bool				visible;		// true -> visible object
	bool				drawWorld;		// true -> shape behind the interface
	bool				drawFront;		// true -> shape before the interface
	int					totalTriangle;		// number of triangles used
	Gfx::ObjectType		type;			// type of the object (TYPE*)
	Math::Matrix		transform;		// transformation matrix
	float				distance;		// distance point of view - original
	Math::Vector		bboxMin;		// bounding box of the object
	Math::Vector		bboxMax;		// (the origin 0, 0, 0 is always included)
	float				radius;			// radius of the sphere at the origin
	int					shadowRank;		// rank of the associated shadow
	float				transparency;		// transparency of the object (0 .. 1)
};

struct Shadow
{
	bool				used;			// true -> object exists
	bool				hide;			// true -> invisible shadow (object carried by ex.)
	int					objRank;		// rank of the object
	Gfx::ShadowType 	type;			// type of shadow
	Math::Vector		pos;			// position for the shadow
	Math::Vector		normal;			// normal terrain
	float				angle;			// angle of the shadow
	float				radius;			// radius of the shadow
	float				intensity;		// intensity of the shadow
	float				height;			// height from the ground
};

struct GroundSpot
{
	bool			used;			// true -> object exists
	Gfx::Color		color;			// color of the shadow
	float			min, max;		// altitudes min / max
	float			smooth;			// transition area
	Math::Vector	pos;			// position for the shadow
	float			radius;			// radius of the shadow
	Math::Vector	drawPos;		// drawn to position the shade
	float			drawRadius;		// radius of the shadow drawn
};

struct GroundMark
{
	bool			used;			// true -> object exists
	bool			draw;			// true -> drawn mark
	int				phase;			// 1 = increase, 2 = fixed, 3 = decrease
	float			delay[3];		// time for 3 phases
	float			fix;			// fixed time
	Math::Vector	pos;			// position for marks
	float			radius;			// radius of marks
	float			intensity;		// color intensity
	Math::Vector	drawPos;		// drawn in position marks
	float			drawRadius;		// radius marks drawn
	float			drawIntensity;		// current drawn
	int				dx, dy;			// dimensions table
	char*			table;			// pointer to the table
};



class CEngine
{
public:
	CEngine(CInstanceManager *iMan, CApplication *app);
	~CEngine();

	void			SetDevice(Gfx::CDevice *device);
	Gfx::CDevice*	RetDevice();

	void			SetTerrain(Gfx::CTerrain* terrain);

	bool			WriteProfile();

	void			SetPause(bool pause);
	bool			RetPause();

	void			SetMovieLock(bool lock);
	bool			RetMovieLock();

	void			SetShowStat(bool show);
	bool			RetShowStat();

	void			SetRenderEnable(bool enable);

	int				OneTimeSceneInit();
	int				InitDeviceObjects();
	int				DeleteDeviceObjects();
	int				RestoreSurfaces();
	int				Render();
	int				FrameMove(float rTime);
	void			StepSimul(float rTime);
	int				FinalCleanup();
	void			AddStatisticTriangle(int nb);
	int				RetStatisticTriangle();
	void			SetHiliteRank(int *rankList);
	bool			GetHilite(Math::Point &p1, Math::Point &p2);
	bool			GetSpriteCoord(int &x, int &y);
	void			SetInfoText(int line, char* text);
	char	*		RetInfoText(int line);
	//LRESULT		MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void			FirstExecuteAdapt(bool first);
	//int				GetVidMemTotal();
	//bool			IsVideo8MB();
	//bool			IsVideo32MB();

	bool			EnumDevices(char *bufDevices, int lenDevices, char *bufModes, int lenModes, int &totalDevices, int &selectDevices, int &totalModes, int &selectModes);
	bool			RetFullScreen();
	bool			ChangeDevice(char *device, char *mode, bool full);

	Math::Matrix*	RetMatView();
	Math::Matrix*	RetMatLeftView();
	Math::Matrix*	RetMatRightView();

	void			TimeInit();
	void			TimeEnterGel();
	void			TimeExitGel();
	float			TimeGet();

	int				RetRestCreate();
	int				CreateObject();
	void			FlushObject();
	bool			DeleteObject(int objRank);
	bool			SetDrawWorld(int objRank, bool draw);
	bool			SetDrawFront(int objRank, bool draw);
	bool			AddTriangle(int objRank, Gfx::VertexTex2* vertex, int nb, const Gfx::Material &mat, int state, char* texName1, char* texName2, float min, float max, bool globalUpdate);
	bool			AddSurface(int objRank, Gfx::VertexTex2* vertex, int nb, const Gfx::Material &mat, int state, char* texName1, char* texName2, float min, float max, bool globalUpdate);
	bool			AddQuick(int objRank, Gfx::ObjLevel6* buffer, char* texName1, char* texName2, float min, float max, bool globalUpdate);
	Gfx::ObjLevel6* SearchTriangle(int objRank, const Gfx::Material &mat, int state, char* texName1, char* texName2, float min, float max);
	void			ChangeLOD();
	bool			ChangeSecondTexture(int objRank, char* texName2);
	int				RetTotalTriangles(int objRank);
	int				GetTriangles(int objRank, float min, float max, Gfx::Triangle* buffer, int size, float percent);
	bool			GetBBox(int objRank, Math::Vector &min, Math::Vector &max);
	bool			ChangeTextureMapping(int objRank, const Gfx::Material &mat, int state, char* texName1, char* texName2, float min, float max, Gfx::Mapping mode, float au, float bu, float av, float bv);
	bool			TrackTextureMapping(int objRank, const Gfx::Material &mat, int state, char* texName1, char* texName2, float min, float max, Gfx::Mapping mode, float pos, float factor, float tl, float ts, float tt);
	bool			SetObjectTransform(int objRank, const Math::Matrix &transform);
	bool			GetObjectTransform(int objRank, Math::Matrix &transform);
	bool			SetObjectType(int objRank, Gfx::ObjectType type);
	Gfx::ObjectType	RetObjectType(int objRank);
	bool			SetObjectTransparency(int objRank, float value);

	bool			ShadowCreate(int objRank);
	void			ShadowDelete(int objRank);
	bool			SetObjectShadowHide(int objRank, bool hide);
	bool			SetObjectShadowType(int objRank, Gfx::ShadowType type);
	bool			SetObjectShadowPos(int objRank, const Math::Vector &pos);
	bool			SetObjectShadowNormal(int objRank, const Math::Vector &n);
	bool			SetObjectShadowAngle(int objRank, float angle);
	bool			SetObjectShadowRadius(int objRank, float radius);
	bool			SetObjectShadowIntensity(int objRank, float intensity);
	bool			SetObjectShadowHeight(int objRank, float h);
	float			RetObjectShadowRadius(int objRank);

	void			GroundSpotFlush();
	int				GroundSpotCreate();
	void			GroundSpotDelete(int rank);
	bool			SetObjectGroundSpotPos(int rank, const Math::Vector &pos);
	bool			SetObjectGroundSpotRadius(int rank, float radius);
	bool			SetObjectGroundSpotColor(int rank, const Gfx::Color &color);
	bool			SetObjectGroundSpotMinMax(int rank, float min, float max);
	bool			SetObjectGroundSpotSmooth(int rank, float smooth);

	int				GroundMarkCreate(Math::Vector pos, float radius, float delay1, float delay2, float delay3, int dx, int dy, char* table);
	bool			GroundMarkDelete(int rank);

	void			Update();

	void			SetViewParams(const Math::Vector &vEyePt, const Math::Vector &vLookatPt, const Math::Vector &vUpVec, float fEyeDistance);

	bool			FreeTexture(char* name);
	bool			LoadTexture(char* name, int stage=0);
	bool			LoadAllTexture();

	void			SetLimitLOD(int rank, float limit);
	float			RetLimitLOD(int rank, bool last=false);

	void			SetTerrainVision(float vision);

	void			SetGroundSpot(bool mode);
	bool			RetGroundSpot();
	void			SetShadow(bool mode);
	bool			RetShadow();
	void			SetDirty(bool mode);
	bool			RetDirty();
	void			SetFog(bool mode);
	bool			RetFog();
	bool			RetStateColor();

	void			SetSecondTexture(int texNum);
	int				RetSecondTexture();

	void			SetRankView(int rank);
	int				RetRankView();

	void			SetDrawWorld(bool draw);
	void			SetDrawFront(bool draw);

	void			SetAmbiantColor(const Gfx::Color &color, int rank=0);
	Gfx::Color		RetAmbiantColor(int rank=0);

	void			SetWaterAddColor(const Gfx::Color &color);
	Gfx::Color 		RetWaterAddColor();

	void			SetFogColor(const Gfx::Color &color, int rank=0);
	Gfx::Color		RetFogColor(int rank=0);

	void			SetDeepView(float length, int rank=0, bool ref=false);
	float			RetDeepView(int rank=0);

	void			SetFogStart(float start, int rank=0);
	float			RetFogStart(int rank=0);

	void			SetBackground(char *name, Gfx::Color up=Gfx::Color(), Gfx::Color down=Gfx::Color(), Gfx::Color cloudUp=Gfx::Color(), Gfx::Color cloudDown=Gfx::Color(), bool full=false, bool quarter=false);
	void			RetBackground(char *name, Gfx::Color &up, Gfx::Color &down, Gfx::Color &cloudUp, Gfx::Color &cloudDown, bool &full, bool &quarter);
	void			SetFrontsizeName(char *name);
	void			SetOverFront(bool front);
	void			SetOverColor(const Gfx::Color &color=Gfx::Color(), int mode=RSTATE_TCOLOR_BLACK);

	void			SetParticuleDensity(float value);
	float			RetParticuleDensity();
	float			ParticuleAdapt(float factor);

	void			SetClippingDistance(float value);
	float			RetClippingDistance();

	void			SetObjectDetail(float value);
	float			RetObjectDetail();

	void			SetGadgetQuantity(float value);
	float			RetGadgetQuantity();

	void			SetTextureQuality(int value);
	int				RetTextureQuality();

	void			SetTotoMode(bool present);
	bool			RetTotoMode();

	void			SetLensMode(bool present);
	bool			RetLensMode();

	void			SetWaterMode(bool present);
	bool			RetWaterMode();

	void			SetBlitzMode(bool present);
	bool			RetBlitzMode();

	void			SetSkyMode(bool present);
	bool			RetSkyMode();

	void			SetBackForce(bool present);
	bool			RetBackForce();

	void			SetPlanetMode(bool present);
	bool			RetPlanetMode();

	void			SetLightMode(bool present);
	bool			RetLightMode();

	void			SetEditIndentMode(bool auto);
	bool			RetEditIndentMode();

	void			SetEditIndentValue(int value);
	int				RetEditIndentValue();

	void			SetSpeed(float speed);
	float			RetSpeed();

	void			SetTracePrecision(float factor);
	float			RetTracePrecision();

	void			SetFocus(float focus);
	float			RetFocus();
	Math::Vector	RetEyePt();
	Math::Vector	RetLookatPt();
	float			RetEyeDirH();
	float			RetEyeDirV();
	Math::Point		RetDim();
	void			UpdateMatProj();

	void			ApplyChange();

	void			FlushPressKey();
	void			ResetKey();
	void			SetKey(int keyRank, int option, int key);
	int				RetKey(int keyRank, int option);

	void			SetJoystick(bool enable);
	bool			RetJoystick();

	void			SetDebugMode(bool mode);
	bool			RetDebugMode();
	bool			RetSetupMode();

	bool			IsVisiblePoint(const Math::Vector &pos);

	int				DetectObject(Math::Point mouse);
	void			SetState(int state, Gfx::Color color=Gfx::Color(1.0f, 1.0f, 1.0f, 1.0f));
	void			SetTexture(char *name, int stage=0);
	void			SetMaterial(const Gfx::Material &mat);

	void			MoveMousePos(Math::Point pos);
	void			SetMousePos(Math::Point pos);
	Math::Point		RetMousePos();
	void			SetMouseType(Gfx::MouseType type);
	Gfx::MouseType	RetMouseType();
	void			SetMouseHide(bool hide);
	bool			RetMouseHide();
	void			SetNiceMouse(bool nice);
	bool			RetNiceMouse();
	bool			RetNiceMouseCap();

	CText*			RetText();

	bool			ChangeColor(char *name, Gfx::Color colorRef1, Gfx::Color colorNew1, Gfx::Color colorRef2, Gfx::Color colorNew2, float tolerance1, float tolerance2, Math::Point ts, Math::Point ti, Math::Point *pExclu=0, float shift=0.0f, bool hSV=false);
	bool			OpenImage(char *name);
	bool			CopyImage();
	bool			LoadImage();
	bool			ScrollImage(int dx, int dy);
	bool			SetDot(int x, int y, Gfx::Color color);
	bool			CloseImage();
	bool			WriteScreenShot(char *filename, int width, int height);
	//bool		GetRenderDC(HDC &hDC);
	//bool		ReleaseRenderDC(HDC &hDC);
	//PBITMAPINFO	CreateBitmapInfoStruct(HBITMAP hBmp);
	//bool		CreateBMPFile(LPTSTR pszFile, PBITMAPINFO pbi, HBITMAP hBMP, HDC hDC);

protected:
	void		MemSpace1(Gfx::ObjLevel1 *&p, int nb);
	void		MemSpace2(Gfx::ObjLevel2 *&p, int nb);
	void		MemSpace3(Gfx::ObjLevel3 *&p, int nb);
	void		MemSpace4(Gfx::ObjLevel4 *&p, int nb);
	void		MemSpace5(Gfx::ObjLevel5 *&p, int nb);
	void		MemSpace6(Gfx::ObjLevel6 *&p, int nb);

	Gfx::ObjLevel2* AddLevel1(Gfx::ObjLevel1 *&p1, char* texName1, char* texName2);
	Gfx::ObjLevel3* AddLevel2(Gfx::ObjLevel2 *&p2, int objRank);
	Gfx::ObjLevel4* AddLevel3(Gfx::ObjLevel3 *&p3, float min, float max);
	Gfx::ObjLevel5* AddLevel4(Gfx::ObjLevel4 *&p4, int reserve);
	Gfx::ObjLevel6* AddLevel5(Gfx::ObjLevel5 *&p5, Gfx::TriangleType type, const Gfx::Material &mat, int state, int nb);

	bool		IsVisible(int objRank);
	bool		DetectBBox(int objRank, Math::Point mouse);
	bool		DetectTriangle(Math::Point mouse, Gfx::VertexTex2 *triangle, int objRank, float &dist);
	bool		TransformPoint(Math::Vector &p2D, int objRank, Math::Vector p3D);
	void		ComputeDistance();
	void		UpdateGeometry();
	void		RenderGroundSpot();
	void		DrawShadow();
	void		DrawBackground();
	void		DrawBackgroundGradient(Gfx::Color up, Gfx::Color down);
	void		DrawBackgroundImageQuarter(Math::Point p1, Math::Point p2, char *name);
	void		DrawBackgroundImage();
	void		DrawPlanet();
	void		DrawFrontsize();
	void		DrawOverColor();
	bool		GetBBox2D(int objRank, Math::Point &min, Math::Point &max);
	void		DrawHilite();
	void		DrawMouse();
	void		DrawSprite(Math::Point pos, Math::Point dim, int icon);

protected:
	CInstanceManager*	m_iMan;
	CApplication*	m_app;
	Gfx::CDevice*	m_device;
	Gfx::CText*			m_text;
	Gfx::CLight*			m_light;
	Gfx::CParticle*		m_particule;
	Gfx::CWater*			m_water;
	Gfx::CCloud*			m_cloud;
	Gfx::CLightning*			m_blitz;
	Gfx::CPlanet*		m_planet;
	Gfx::CTerrain*		m_terrain;
	CSound*			m_sound;

	int				m_blackSrcBlend[2];
	int				m_blackDestBlend[2];
	int				m_whiteSrcBlend[2];
	int				m_whiteDestBlend[2];
	int				m_diffuseSrcBlend[2];
	int				m_diffuseDestBlend[2];
	int				m_alphaSrcBlend[2];
	int				m_alphaDestBlend[2];

	Math::Matrix	m_matProj;
	Math::Matrix	m_matLeftView;
	Math::Matrix	m_matRightView;
	Math::Matrix	m_matView;
	float			m_focus;

	Math::Matrix	m_matWorldInterface;
	Math::Matrix	m_matProjInterface;
	Math::Matrix	m_matViewInterface;

	long			m_baseTime;
	long			m_stopTime;
	float			m_absTime;
	float			m_lastTime;
	float			m_speed;
	bool			m_pause;
	bool			m_render;
	bool			m_movieLock;

	Math::IntPoint		m_dim;
	Math::IntPoint		m_lastDim;
	Gfx::ObjLevel1*		m_objectPointer;
	int					m_objectParamTotal;
	Gfx::Object*		m_objectParam;
	int					m_shadowTotal;
	Gfx::Shadow*		m_shadow;
	Gfx::GroundSpot*	m_groundSpot;
	Gfx::GroundMark		m_groundMark;
	Math::Vector		m_eyePt;
	Math::Vector		m_lookatPt;
	float			m_eyeDirH;
	float			m_eyeDirV;
	int				m_rankView;
	Gfx::Color		m_ambiantColor[2];
	Gfx::Color		m_backColor[2];
	Gfx::Color		m_fogColor[2];
	float			m_deepView[2];
	float			m_fogStart[2];
	Gfx::Color		m_waterAddColor;
	int				m_statisticTriangle;
	bool			m_updateGeometry;
	char			m_infoText[10][200];
	int				m_alphaMode;
	bool			m_stateColor;
	bool			m_forceStateColor;
	bool			m_groundSpotVisible;
	bool			m_shadowVisible;
	bool			m_dirty;
	bool			m_fog;
	bool			m_firstGroundSpot;
	int				m_secondTexNum;
	char			m_backgroundName[50];
	Gfx::Color		m_backgroundColorUp;
	Gfx::Color		m_backgroundColorDown;
	Gfx::Color		m_backgroundCloudUp;
	Gfx::Color		m_backgroundCloudDown;
	bool			m_backgroundFull;
	bool			m_backgroundQuarter;
	bool			m_overFront;
	Gfx::Color		m_overColor;
	int				m_overMode;
	char			m_frontsizeName[50];
	bool			m_drawWorld;
	bool			m_drawFront;
	float			m_limitLOD[2];
	float			m_particuleDensity;
	float			m_clippingDistance;
	float			m_lastClippingDistance;
	float			m_objectDetail;
	float			m_lastObjectDetail;
	float			m_terrainVision;
	float			m_gadgetQuantity;
	int				m_textureQuality;
	bool			m_totoMode;
	bool			m_lensMode;
	bool			m_waterMode;
	bool			m_skyMode;
	bool			m_backForce;
	bool			m_planetMode;
	bool			m_lightMode;
	bool			m_editIndentMode;
	int				m_editIndentValue;
	float			m_tracePrecision;

	int				m_hiliteRank[100];
	bool			m_hilite;
	Math::Point		m_hiliteP1;
	Math::Point		m_hiliteP2;

	int				m_lastState;
	Gfx::Color		m_lastColor;
	char			m_lastTexture[2][50];
	Gfx::Material	m_lastMaterial;

	Math::Point		m_mousePos;
	Gfx::MouseType	m_mouseType;
	bool			m_mouseHide;
	bool			m_niceMouse;

	//LPDIRECTDRAWSURFACE7 m_imageSurface;
	//DDSURFACEDESC2		m_imageDDSD;
	//WORD*				m_imageCopy;
	//int					m_imageDX;
	//int					m_imageDY;
};

}; // namespace Gfx
