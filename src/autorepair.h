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
// * along with this program. If not, see .

#ifndef _AUTOREPAIR_H_
#define	_AUTOREPAIR_H_


class CInstanceManager;
class CD3DEngine;
class CParticule;
class CTerrain;
class CCamera;
class CObject;



enum AutoRepairPhase
{
	ARP_WAIT		= 1,	// attend m�tal
	ARP_DOWN		= 2,	// descend le couvercle
	ARP_REPAIR		= 3,	// construit le v�hicule
	ARP_UP			= 4,	// remonte le couvercle
};



class CAutoRepair : public CAuto
{
public:
	CAutoRepair(CInstanceManager* iMan, CObject* object);
	~CAutoRepair();

	void		DeleteObject(BOOL bAll=FALSE);

	void		Init();
	BOOL		EventProcess(const Event &event);
	Error		RetError();

	BOOL		CreateInterface(BOOL bSelect);

	BOOL		Write(char *line);
	BOOL		Read(char *line);

protected:
	CObject*	SearchVehicle();

protected:
	AutoRepairPhase	m_phase;
	float			m_progress;
	float			m_speed;
	float			m_timeVirus;
	float			m_lastParticule;
};


#endif //_AUTOREPAIR_H_
