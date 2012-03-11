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

#ifndef _AUTOPORTICO_H_
#define	_AUTOPORTICO_H_


class CInstanceManager;
class CD3DEngine;
class CParticule;
class CTerrain;
class CCamera;
class CObject;



enum AutoPorticoPhase
{
	APOP_WAIT		= 1,	// attend
	APOP_START		= 2,	// d�part de l'action
	APOP_MOVE		= 3,	// avance
	APOP_WAIT1		= 4,	// attend
	APOP_DOWN		= 5,	// descend
	APOP_WAIT2		= 6,	// attend
	APOP_OPEN		= 7,	// ouvre
};



class CAutoPortico : public CAuto
{
public:
	CAutoPortico(CInstanceManager* iMan, CObject* object);
	~CAutoPortico();

	void		DeleteObject(BOOL bAll=FALSE);

	void		Init();
	void		Start(int param);
	BOOL		EventProcess(const Event &event);
	BOOL		Abort();
	Error		RetError();

protected:
	void		UpdateTrackMapping(float left, float right);

protected:
	AutoPorticoPhase m_phase;
	float			m_progress;
	float			m_speed;
	float			m_cameraProgress;
	float			m_cameraSpeed;
	float			m_lastParticule;
	D3DVECTOR		m_finalPos;
	D3DVECTOR		m_startPos;
	float			m_posTrack;
	int				m_param;
	int				m_soundChannel;
};


#endif //_AUTOPORTICO_H_
