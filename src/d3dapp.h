// D3DApp.h


#ifndef  _D3DAPP_H
#define  _D3DAPP_H

#define  D3D_OVERLOADS

#include <d3d.h>
#include "D3DFrame.h"
#include "D3DEnum.h"
#include "D3DUtil.h"
#include "D3DRes.h"



class CInstanceManager;
class CEvent;
class CD3DEngine;
class CRobotMain;
class CSound;

enum D3DMouse;
enum Error;



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
	BOOL		IsVideo8MB();
	BOOL		IsVideo32MB();
	HRESULT		Create( HINSTANCE, TCHAR* );
	INT			Run();
	LRESULT		MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	VOID		Pause( BOOL bPause );
	FPOINT		ConvPosToInterface(HWND hWnd, LPARAM lParam);
	void		SetMousePos(FPOINT pos);
	void		StepSimul(float rTime);
	char*		RetCDpath();

	void		SetShowStat(BOOL bShow);
	BOOL		RetShowStat();
	void		SetDebugMode(BOOL bMode);
	BOOL		RetDebugMode();

	BOOL		EnumDevices(char *bufDevices, int lenDevices, char *bufModes, int lenModes, int &totalDevices, int &selectDevices, int &totalModes, int &selectModes);
	BOOL		RetFullScreen();
	BOOL		ChangeDevice(char *device, char *mode, BOOL bFull);

	void		FlushPressKey();
	void		ResetKey();
	void		SetKey(int keyRank, int option, int key);
	int			RetKey(int keyRank, int option);
	BOOL		IsKeyMouse(int key);
	BOOL		IsKeyJoystick(int key);

	void		SetForce(float force);
	float		RetForce();
	void		SetFFB(BOOL bMode);
	BOOL		RetFFB();
	void		SetJoystick(int mode);
	int			RetJoystick();
	BOOL		SetJoyForces(float forceX, float forceY);

	void		SetMouseType(D3DMouse type);
	BOOL		RetNiceMouseCap();

	void		SetMouseCapture();
	void		ReleaseMouseCapture();

	BOOL		WriteScreenShot(char *filename, int width, int height);

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
	BOOL			m_bActive;
	BOOL			m_bActivateApp;
	BOOL			m_bReady;
	int				m_joystick;
	float			m_FFBforce;
	BOOL			m_bFFB;

	DWORD			m_vidMemTotal;
	TCHAR*			m_strWindowTitle;
	BOOL			m_bAppUseZBuffer;
	BOOL			m_bAppUseStereo;
	BOOL			m_bShowStats;
	BOOL			m_bDebugMode;
	BOOL			m_bAudioState;
	BOOL			m_bAudioTrack;
	HRESULT			(*m_fnConfirmDevice)(DDCAPS*, D3DDEVICEDESC7*);

public:
	CD3DEngine*		m_pD3DEngine;
	CRobotMain*		m_pRobotMain;
	CSound*			m_pSound;

	int				m_keyState;
	float			m_axeKeyX;		// avancer/reculer
	float			m_axeKeyY;		// tourner
	float			m_axeKeyZ;		// monter/descendre
	float			m_axeKeyW;		// frain à main
	D3DVECTOR		m_axeKey;
	D3DVECTOR		m_axeJoy;
	BOOL			m_bJoyButton[32];
	BOOL			m_bJoyLeft;
	BOOL			m_bJoyRight;
	BOOL			m_bJoyUp;
	BOOL			m_bJoyDown;
	FPOINT			m_mousePos;
	DWORD			m_mshMouseWheel;

	float			m_aTime;
	DWORD			m_key[50][2];
};


#endif // _D3DAPP_H
