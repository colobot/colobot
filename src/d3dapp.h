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


#ifndef  _D3DAPP_H
#define  _D3DAPP_H

#define  D3D_OVERLOADS

#include <d3d.h>

#include "d3dengine.h"
#include "d3dframe.h"
#include "d3denum.h"
#include "d3dutil.h"
#include "d3dres.h"
#include "misc.h"
#include "struct.h"


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
    LRESULT     OnQuerySuspend( DWORD dwFlags );
    LRESULT     OnResumeSuspend( DWORD dwData );

public:
    Error       RegQuery();
    Error       AudioQuery();
    Error       CheckMistery(char *strCmdLine);
    int         GetVidMemTotal();
    BOOL        IsVideo8MB();
    BOOL        IsVideo32MB();
    HRESULT     Create( HINSTANCE, TCHAR* );
    INT         Run();
    LRESULT     MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
    VOID        Pause( BOOL bPause );
    FPOINT      ConvPosToInterface(HWND hWnd, LPARAM lParam);
    void        SetMousePos(FPOINT pos);
    void        StepSimul(float rTime);
    char*       RetCDpath();

    void        SetShowStat(BOOL bShow);
    BOOL        RetShowStat();
    void        SetDebugMode(BOOL bMode);
    BOOL        RetDebugMode();
    BOOL        RetSetupMode();

    BOOL        EnumDevices(char *bufDevices, int lenDevices, char *bufModes, int lenModes, int &totalDevices, int &selectDevices, int &totalModes, int &selectModes);
    BOOL        RetFullScreen();
    BOOL        ChangeDevice(char *device, char *mode, BOOL bFull);

    void        FlushPressKey();
    void        ResetKey();
    void        SetKey(int keyRank, int option, int key);
    int         RetKey(int keyRank, int option);

    void        SetJoystick(BOOL bEnable);
    BOOL        RetJoystick();

    void        SetMouseType(D3DMouse type);
    void        SetNiceMouse(BOOL bNice);
    BOOL        RetNiceMouse();
    BOOL        RetNiceMouseCap();

    BOOL        WriteScreenShot(char *filename, int width, int height);

    BOOL        GetRenderDC(HDC &hDC);
    BOOL        ReleaseRenderDC(HDC &hDC);
    PBITMAPINFO CreateBitmapInfoStruct(HBITMAP hBmp);
    BOOL        CreateBMPFile(LPTSTR pszFile, PBITMAPINFO pbi, HBITMAP hBMP, HDC hDC);

protected:
    HRESULT     ConfirmDevice( DDCAPS* pddDriverCaps, D3DDEVICEDESC7* pd3dDeviceDesc );
    HRESULT     Initialize3DEnvironment();
    HRESULT     Change3DEnvironment();
    HRESULT     CreateZBuffer(GUID* pDeviceGUID);
    HRESULT     Render3DEnvironment();
    VOID        Cleanup3DEnvironment();
    VOID        DeleteDeviceObjects();
    VOID        DisplayFrameworkError( HRESULT, DWORD );

    void        InitText();
    void        DrawSuppl();
    VOID        ShowStats();
    VOID        OutputText( DWORD x, DWORD y, TCHAR* str );

protected:
    CInstanceManager*       m_iMan;
    CEvent*                 m_event;

    HINSTANCE               m_instance;
    HWND                    m_hWnd;
    D3DEnum_DeviceInfo*     m_pDeviceInfo;
    LPDIRECTDRAW7           m_pDD;
    LPDIRECT3D7             m_pD3D;
    LPDIRECT3DDEVICE7       m_pD3DDevice;
    LPDIRECTDRAWSURFACE7    m_pddsRenderTarget;
    DDSURFACEDESC2          m_ddsdRenderTarget;
    LPDIRECTDRAWSURFACE7    m_pddsDepthBuffer;

    HANDLE          m_thread;
    DWORD           m_threadId;

    char            m_CDpath[100];

    CD3DFramework7* m_pFramework;
    BOOL            m_bActive;
    BOOL            m_bActivateApp;
    BOOL            m_bReady;
    BOOL            m_bJoystick;

    DWORD           m_vidMemTotal;
    TCHAR*          m_strWindowTitle;
    BOOL            m_bAppUseZBuffer;
    BOOL            m_bAppUseStereo;
    BOOL            m_bShowStats;
    BOOL            m_bDebugMode;
    BOOL            m_bAudioState;
    BOOL            m_bAudioTrack;
    BOOL            m_bNiceMouse;
    BOOL            m_bSetupMode;
    HRESULT         (*m_fnConfirmDevice)(DDCAPS*, D3DDEVICEDESC7*);

public:
    CD3DEngine*     m_pD3DEngine;
    CRobotMain*     m_pRobotMain;
    CSound*         m_pSound;

    int             m_keyState;
    D3DVECTOR       m_axeKey;
    D3DVECTOR       m_axeJoy;
    BOOL            m_bJoyButton[32];
    FPOINT          m_mousePos;
    DWORD           m_mshMouseWheel;

    float           m_aTime;
    DWORD           m_key[50][2];
};


#endif // _D3DAPP_H
