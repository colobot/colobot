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

// d3dapp.h

#pragma once


#define  D3D_OVERLOADS
#include <d3d.h>

#include "math/vector.h"
#include "graphics/d3d/d3dengine.h"
#include "graphics/d3d/d3dframe.h"
#include "graphics/d3d/d3denum.h"
#include "graphics/d3d/d3dutil.h"
#include "old/d3dres.h"
#include "common/misc.h"
#include "common/struct.h"


class CInstanceManager;
class CEvent;
class CRobotMain;
class CSound;



class CD3DApplication
{
public:
	CD3DApplication();
	~CD3DApplication();

protected:
	LRESULT		OnQuerySuspend( DWORD dwFlags );
	LRESULT		OnResumeSuspend( DWORD dwData );

public:
	Error		RegQuery();
	Error		AudioQuery();
	Error		CheckMistery(char *strCmdLine);
	int			GetVidMemTotal();
	bool		IsVideo8MB();
	bool		IsVideo32MB();
	HRESULT		Create( HINSTANCE, TCHAR* );
	INT			Run();
	LRESULT		MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	VOID		Pause( bool bPause );
	Math::Point		ConvPosToInterface(HWND hWnd, LPARAM lParam);
	void		SetMousePos(Math::Point pos);
	void		StepSimul(float rTime);
	char*		RetCDpath();

	void		SetShowStat(bool bShow);
	bool		RetShowStat();
	void		SetDebugMode(bool bMode);
	bool		RetDebugMode();
	bool		RetSetupMode();

	bool		EnumDevices(char *bufDevices, int lenDevices, char *bufModes, int lenModes, int &totalDevices, int &selectDevices, int &totalModes, int &selectModes);
	bool		RetFullScreen();
	bool		ChangeDevice(char *device, char *mode, bool bFull);

	void		FlushPressKey();
	void		ResetKey();
	void		SetKey(int keyRank, int option, int key);
	int			RetKey(int keyRank, int option);

	void		SetJoystick(bool bEnable);
	bool		RetJoystick();

	void		SetMouseType(D3DMouse type);
	void		SetNiceMouse(bool bNice);
	bool		RetNiceMouse();
	bool		RetNiceMouseCap();

	bool		WriteScreenShot(char *filename, int width, int height);

	bool		GetRenderDC(HDC &hDC);
	bool		ReleaseRenderDC(HDC &hDC);
	PBITMAPINFO	CreateBitmapInfoStruct(HBITMAP hBmp);
	bool		CreateBMPFile(LPTSTR pszFile, PBITMAPINFO pbi, HBITMAP hBMP, HDC hDC);

protected:
	HRESULT		ConfirmDevice( DDCAPS* pddDriverCaps, D3DDEVICEDESC7* pd3dDeviceDesc );
	HRESULT		Initialize3DEnvironment();
	HRESULT		Change3DEnvironment();
	HRESULT		CreateZBuffer(GUID* pDeviceGUID);
	HRESULT		Render3DEnvironment();
	VOID		Cleanup3DEnvironment();
	VOID		DeleteDeviceObjects();
	VOID		DisplayFrameworkError( HRESULT, DWORD );

	void		InitText();
	void		DrawSuppl();
	VOID		ShowStats();
	VOID		OutputText( DWORD x, DWORD y, TCHAR* str );

protected:
	CInstanceManager*		m_iMan;
	CEvent*					m_event;

	HINSTANCE				m_instance;
	HWND					m_hWnd;
	D3DEnum_DeviceInfo*		m_pDeviceInfo;
	LPDIRECTDRAW7			m_pDD;
	LPDIRECT3D7				m_pD3D;
	LPDIRECT3DDEVICE7		m_pD3DDevice;
	LPDIRECTDRAWSURFACE7	m_pddsRenderTarget;
	DDSURFACEDESC2			m_ddsdRenderTarget;
	LPDIRECTDRAWSURFACE7	m_pddsDepthBuffer;

	HANDLE			m_thread;
	DWORD			m_threadId;

	char			m_CDpath[100];

	CD3DFramework7*	m_pFramework;
	bool			m_bActive;
	bool			m_bActivateApp;
	bool			m_bReady;
	bool			m_bJoystick;

	DWORD			m_vidMemTotal;
	TCHAR*			m_strWindowTitle;
	bool			m_bAppUseZBuffer;
	bool			m_bAppUseStereo;
	bool			m_bShowStats;
	bool			m_bDebugMode;
	bool			m_bAudioState;
	bool			m_bAudioTrack;
	bool			m_bNiceMouse;
	bool			m_bSetupMode;
	HRESULT			(*m_fnConfirmDevice)(DDCAPS*, D3DDEVICEDESC7*);

public:
	CD3DEngine*		m_pD3DEngine;
	CRobotMain*		m_pRobotMain;
	CSound*			m_pSound;

	int				m_keyState;
	Math::Vector		m_axeKey;
	Math::Vector		m_axeJoy;
	bool			m_bJoyButton[32];
	Math::Point			m_mousePos;
	DWORD			m_mshMouseWheel;

	float			m_aTime;
	DWORD			m_key[50][2];
};


