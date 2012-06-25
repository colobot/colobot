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

// lightning.h (aka blitz.h)

#pragma once

#include "common/misc.h"
#include "common/struct.h"
#include "object/object.h"


class CInstanceManager;
class CSound;


namespace Gfx {

class CEngine;
class CTerrain;
class CCamera;


const float BLITZPARA = 200.0f;		// radius of lightning protection
const short BLITZMAX = 50;

enum BlitzPhase
{
	BPH_WAIT,
	BPH_BLITZ,
};


class CLightning
{
public:
	CLightning(CInstanceManager* iMan, CEngine* engine);
	~CLightning();

	void		Flush();
	bool		EventProcess(const Event &event);
	bool		Create(float sleep, float delay, float magnetic);
	bool		GetStatus(float &sleep, float &delay, float &magnetic, float &progress);
	bool		SetStatus(float sleep, float delay, float magnetic, float progress);
	void		Draw();

protected:
	bool		EventFrame(const Event &event);
	CObject*	SearchObject(Math::Vector pos);

protected:
	CInstanceManager*	m_iMan;
	CEngine*			m_engine;
	CTerrain*			m_terrain;
	CCamera*			m_camera;
	CSound*				m_sound;

	bool			m_bBlitzExist;
	float			m_sleep;
	float			m_delay;
	float			m_magnetic;
	BlitzPhase		m_phase;
	float			m_time;
	float			m_speed;
	float			m_progress;
	Math::Vector	m_pos;
	Math::Point		m_shift[BLITZMAX];
	float			m_width[BLITZMAX];
};

}; // namespace Gfx
