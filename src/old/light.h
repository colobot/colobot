// * This file is part of the COLOBOT source code
// * Copyright (C) 2001-2008, Daniel ROUX & EPSITEC SA, www.epsitec.ch
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

// light.h

#pragma once


#include "graphics/d3d/d3dengine.h"
#include "graphics/common/color.h"


class CInstanceManager;
class CD3DEngine;


const int D3DMAXLIGHT = 100;


struct LightProg
{
	float	starting;
	float	ending;
	float	current;
	float	progress;
	float	speed;
};


struct Light
{
	char			bUsed;			// true -> light exists
	char			bEnable;		// true -> light turned on

	D3DTypeObj		incluType;		// type of all objects included
	D3DTypeObj		excluType;		// type of all objects excluded

	D3DLIGHT7		light;			// configuration of the light

	LightProg		intensity;		// intensity (0 .. 1)
	LightProg		colorRed;
	LightProg		colorGreen;
	LightProg		colorBlue;
};


// temporary!
namespace Gfx {

/** \enum LightType Type of light */
enum LightType
{
	LT_Point,
	LT_Spot,
	LT_Directional
};

/**
 * \struct Light Light
 *
 * This structure was created as analog to DirectX's D3DLIGHT.
 *
 * It contains analogous fields as the D3DLIGHT struct.
 */
struct Light
{
	//! Type of light source
	Gfx::LightType		type;
	//! Color of light
	Gfx::Color			color;
	//! Position in world space
	Math::Vector	position;
	//! Direction in world space
	Math::Vector	direction;
	//! Cutoff range
	float			range;
	//! Falloff
	float			falloff;
	//! Constant attenuation
	float			attenuation0;
	//! Linear attenuation
	float			attenuation1;
	//! Quadratic attenuation
	float			attenuation2;
	//! Inner angle of spotlight cone
	float			theta;
	//! Outer angle of spotlight cone
	float			phi;

	Light() : type(LT_Point), range(0.0f), falloff(0.0f),
			  attenuation0(0.0f), attenuation1(0.0f), attenuation2(0.0f),
			  theta(0.0f), phi(0.0f) {}
};

struct LightProg
{
	float	starting;
	float	ending;
	float	current;
	float	progress;
	float	speed;
};

/**
 * \struct SceneLight Dynamic light in 3D scene
 *
 * TODO documentation
 */
struct SceneLight
{
	//! true -> light exists
	bool			used;
	//! true -> light turned on
	bool			enable;

	//! Type of all objects included
	D3DTypeObj		incluType;
	//! Type of all objects excluded
	D3DTypeObj		excluType;

	//! Configuration of the light
	Gfx::Light		light;

	//! intensity (0 .. 1)
	Gfx::LightProg		intensity;
	Gfx::LightProg		colorRed;
	Gfx::LightProg		colorGreen;
	Gfx::LightProg		colorBlue;
};

}; // namespace Gfx



class CLight
{
public:
	CLight(CInstanceManager *iMan, CD3DEngine* engine);
	virtual ~CLight();

	void			SetD3DDevice(LPDIRECT3DDEVICE7 device);

	void			FlushLight();
	int				CreateLight();
	bool			DeleteLight(int lightRank);
	bool			SetLight(int lightRank, const D3DLIGHT7 &light);
	bool			GetLight(int lightRank, D3DLIGHT7 &light);
	bool			LightEnable(int lightRank, bool bEnable);

	bool			SetLightIncluType(int lightRank, D3DTypeObj type);
	bool			SetLightExcluType(int lightRank, D3DTypeObj type);

	bool			SetLightPos(int lightRank, Math::Vector pos);
	Math::Vector		RetLightPos(int lightRank);

	bool			SetLightDir(int lightRank, Math::Vector dir);
	Math::Vector		RetLightDir(int lightRank);

	bool			SetLightIntensitySpeed(int lightRank, float speed);
	bool			SetLightIntensity(int lightRank, float value);
	float			RetLightIntensity(int lightRank);
	void			AdaptLightColor(D3DCOLORVALUE color, float factor);

	bool			SetLightColorSpeed(int lightRank, float speed);
	bool			SetLightColor(int lightRank, D3DCOLORVALUE color);
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

