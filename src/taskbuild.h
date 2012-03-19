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
// * along with this program. If not, see  http://www.gnu.org/licenses/.// taskbuild.h

#ifndef _TASKBUILD_H_
#define	_TASKBUILD_H_


class CInstanceManager;
class CTerrain;
class CBrain;
class CPhysics;
class CObject;



#define BUILDMARGIN		16.0f
#define TBMAXLIGHT		4


enum TaskBuildPhase
{
	TBP_TURN	= 1,	// tourne
	TBP_MOVE	= 2,	// avance/recule
	TBP_TAKE	= 3,	// prend arme
	TBP_PREP	= 4,	// prépare
	TBP_BUILD	= 5,	// construit
	TBP_TERM	= 6,	// termine
	TBP_RECEDE	= 7,	// recule terminal
};



class CTaskBuild : public CTask
{
public:
	CTaskBuild(CInstanceManager* iMan, CObject* object);
	~CTaskBuild();

	BOOL		EventProcess(const Event &event);

	Error		Start(ObjectType type);
	Error		IsEnded();
	BOOL		Abort();

protected:
	Error		FlatFloor();
	BOOL		CreateBuilding(D3DVECTOR pos, float angle);
	void		CreateLight();
	void		BlackLight();
	CObject*	SearchMetalObject(float &angle, float dMin, float dMax, float aLimit, Error &err);
	void		DeleteMark(D3DVECTOR pos, float radius);

protected:
	ObjectType		m_type;			// type de construction
	CObject*		m_metal;		// objet metal transformé
	CObject*		m_power;		// pile du véhicule
	CObject*		m_building;		// batiment construit
	TaskBuildPhase	m_phase;		// phase de l'opération
	BOOL			m_bError;		// TRUE -> opération impossible
	BOOL			m_bBuild;		// TRUE -> batiment construit
	BOOL			m_bBlack;		// TRUE -> lumières noir -> blanc
	float			m_time;			// temps absolu
	float			m_lastParticule;// temps génération dernière particule
	float			m_progress;		// progression (0..1)
	float			m_speed;		// vitesse de la progression
	float			m_angleY;		// angle de rotation du véhicule
	float			m_angleZ;		// angle de rotation du canon
	D3DVECTOR		m_buildingPos;	// position initiale du batiment
	float			m_buildingHeight;// hauteur du building
	int				m_lightRank[TBMAXLIGHT];// lumières pour les effets
	int				m_soundChannel;
};


#endif //_TASKBUILD_H_
