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

#define STRICT
#define D3D_OVERLOADS

#include <windows.h>
#include <winuser.h>
#include <mmsystem.h>
#include <stdio.h>
#include <direct.h>
#include <tchar.h>
#include <zmouse.h>
#include <dinput.h>

#include "struct.h"
#include "d3dtextr.h"
#include "d3dengine.h"
#include "language.h"
#include "event.h"
#include "profile.h"
#include "iman.h"
#include "restext.h"
#include "math3d.h"
#include "joystick.h"
#include "robotmain.h"
#include "sound.h"
#include "d3dapp.h"

// fix for "MSH_MOUSEWHEEL undefined" error
#ifdef UNICODE
#define MSH_MOUSEWHEEL L"MSWHEEL_ROLLMSG"
#else
#define MSH_MOUSEWHEEL "MSWHEEL_ROLLMSG"
#endif


#define AUDIO_TRACK     13      // total number of audio tracks on the CD
#define MAX_STEP        0.2f        // maximum time for a step

#define WINDOW_DX       (640+6)     // dimensions in windowed mode
#define WINDOW_DY       (480+25)

#define USE_THREAD      FALSE       // TRUE does not work!
#define TIME_THREAD     0.02f




// Limit the use of the controls keyboard & joystick.

float AxeLimit(float value)
{
    if ( value < -1.0f )  value = -1.0f;
    if ( value >  1.0f )  value =  1.0f;
    return value;
}


// Entry point to the program. Initializes everything, and goes into a
// message-processing loop. Idle time is used to render the scene.

INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
{
    Error   err;
    char    string[100];

    CD3DApplication d3dApp;  // single instance of the application

    err = d3dApp.CheckMistery(strCmdLine);
    if ( err != ERR_OK )
    {
        GetResource(RES_ERR, err, string);
#if _NEWLOOK
        MessageBox( NULL, string, _T("CeeBot"), MB_ICONERROR|MB_OK );
#else
        MessageBox( NULL, string, _T("COLOBOT"), MB_ICONERROR|MB_OK );
#endif
        return 0;
    }

    if ( FAILED(d3dApp.Create(hInst, strCmdLine)) )
    {
        return 0;
    }

    return d3dApp.Run();  // execution of all
}


// Internal variables and function prototypes.

enum APPMSGTYPE { MSG_NONE, MSGERR_APPMUSTEXIT, MSGWARN_SWITCHEDTOSOFTWARE };

static INT     CALLBACK AboutProc( HWND, UINT, WPARAM, LPARAM );
static LRESULT CALLBACK WndProc( HWND, UINT, WPARAM, LPARAM );

static CD3DApplication* g_pD3DApp;



// Constructor.

CD3DApplication::CD3DApplication()
{
    int     i;

    m_iMan = new(CInstanceManager);
    m_event = new CEvent(m_iMan);

    m_pD3DEngine = 0;
    m_pRobotMain = 0;
    m_pSound     = 0;
    m_pFramework = 0;
    m_instance   = 0;
    m_hWnd       = 0;
    m_pDD        = 0;
    m_pD3D       = 0;
    m_pD3DDevice = 0;

    m_CDpath[0] = 0;

    m_pddsRenderTarget = 0;
    m_pddsDepthBuffer  = 0;

    m_keyState = 0;
    m_axeKey = D3DVECTOR(0.0f, 0.0f, 0.0f);
    m_axeJoy = D3DVECTOR(0.0f, 0.0f, 0.0f);

    m_vidMemTotal  = 0;
    m_bActive      = FALSE;
    m_bActivateApp = FALSE;
    m_bReady       = FALSE;
    m_bJoystick    = FALSE;
    m_aTime        = 0.0f;

    for ( i=0 ; i<32 ; i++ )
    {
        m_bJoyButton[i] = FALSE;
    }

#if _NEWLOOK
    m_strWindowTitle  = _T("CeeBot");
#else
    m_strWindowTitle  = _T("COLOBOT");
#endif
    m_bAppUseZBuffer  = TRUE;
    m_bAppUseStereo   = TRUE;
    m_bShowStats      = FALSE;
    m_bDebugMode      = FALSE;
    m_bAudioState     = TRUE;
    m_bAudioTrack     = TRUE;
    m_bNiceMouse      = FALSE;
    m_bSetupMode      = TRUE;
    m_fnConfirmDevice = 0;

    ResetKey();

    g_pD3DApp = this;

    // Request event sent by Logitech.
    m_mshMouseWheel = RegisterWindowMessage(MSH_MOUSEWHEEL);

    _mkdir("files\\");
}


// Destructor.

CD3DApplication::~CD3DApplication()
{
    delete m_iMan;
}



// Returns the path of the CD.

char* CD3DApplication::RetCDpath()
{
    return m_CDpath;
}

// Reads the information in the registry.

Error CD3DApplication::RegQuery()
{
    FILE*   file = NULL;
    HKEY    key;
    LONG    i;
    DWORD   type, len;
    char    filename[100];

#if _NEWLOOK
 #if _TEEN
    i = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software\\Epsitec\\CeeBot-Teen\\Setup",
 #else
    i = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software\\Epsitec\\CeeBot-A\\Setup",
 #endif
#else
    i = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software\\Epsitec\\Colobot\\Setup",
#endif
                     0, KEY_READ, &key);
    if ( i != ERROR_SUCCESS )  return ERR_INSTALL;

    type = REG_SZ;
    len  = sizeof(m_CDpath);
    i = RegQueryValueEx(key, "CDpath", NULL, &type, (LPBYTE)m_CDpath, &len);
    if ( i != ERROR_SUCCESS || type != REG_SZ )  return ERR_INSTALL;

    filename[0] = m_CDpath[0];
    filename[1] = ':';
    filename[2] = '\\';
    filename[3] = 0;
    i = GetDriveType(filename);
    if ( i != DRIVE_CDROM )  return ERR_NOCD;

    strcat(filename, "install.ini");
    file = fopen(filename, "rb");  // install.ini file exist?
    if ( file == NULL )  return ERR_NOCD;
    fclose(file);

    return ERR_OK;
}

// Checks for audio tracks on the CD.

Error CD3DApplication::AudioQuery()
{
    MCI_OPEN_PARMS      mciOpenParms;
    MCI_STATUS_PARMS    mciStatusParms;
    DWORD               dwReturn;
    UINT                deviceID;
    char                device[10];

    // Open the device by specifying the device and filename.
    // MCI will attempt to choose the MIDI mapper as the output port.
    memset(&mciOpenParms, 0, sizeof(MCI_OPEN_PARMS));
    mciOpenParms.lpstrDeviceType = (LPCTSTR)MCI_DEVTYPE_CD_AUDIO;
    if ( m_CDpath[0] == 0 )
    {
        dwReturn = mciSendCommand(NULL,
                                  MCI_OPEN,
                                  MCI_OPEN_TYPE|MCI_OPEN_TYPE_ID,
                                  (DWORD)(LPVOID)&mciOpenParms);
    }
    else
    {
        device[0] = m_CDpath[0];
        device[1] = ':';
        device[2] = 0;
        mciOpenParms.lpstrElementName = device;
        dwReturn = mciSendCommand(NULL,
                                  MCI_OPEN,
                                  MCI_OPEN_TYPE|MCI_OPEN_TYPE_ID|MCI_OPEN_ELEMENT,
                                  (DWORD)(LPVOID)&mciOpenParms);
    }
    if ( dwReturn != 0 )
    {
        return ERR_NOCD;
    }

    // The device opened successfully; get the device ID.
    deviceID = mciOpenParms.wDeviceID;

    memset(&mciStatusParms, 0, sizeof(MCI_STATUS_PARMS));
    mciStatusParms.dwItem = MCI_STATUS_NUMBER_OF_TRACKS;
    dwReturn = mciSendCommand(deviceID,
                              MCI_STATUS,
                              MCI_WAIT|MCI_STATUS_ITEM,
                              (DWORD)&mciStatusParms);
    if ( dwReturn != 0 )
    {
        mciSendCommand(deviceID, MCI_CLOSE, 0, NULL);
        return ERR_NOCD;
    }

    if ( mciStatusParms.dwReturn != AUDIO_TRACK )
    {
        mciSendCommand(deviceID, MCI_CLOSE, 0, NULL);
        return ERR_NOCD;
    }

    mciSendCommand(deviceID, MCI_CLOSE, 0, NULL);
    return ERR_OK;
}

// Checks for the key.

Error CD3DApplication::CheckMistery(char *strCmdLine)
{
    if ( strstr(strCmdLine, "-debug") != 0 )
    {
        m_bShowStats = TRUE;
        SetDebugMode(TRUE);
    }

    if ( strstr(strCmdLine, "-audiostate") != 0 )
    {
        m_bAudioState = FALSE;
    }

    if ( strstr(strCmdLine, "-audiotrack") != 0 )
    {
        m_bAudioTrack = FALSE;
    }

    m_CDpath[0] = 0;
// disable -nocd check to run without problems
#if 0
//#if _FULL
    if ( strstr(strCmdLine, "-nocd") == 0 && !m_bDebugMode )
    {
        Error   err;

        err = RegQuery();
        if ( err != ERR_OK )  return err;

        //?err = AudioQuery();
        //?if ( err != ERR_OK )  return err;
    }
#endif
#if _SCHOOL & _EDU
    if ( strstr(strCmdLine, "-nosetup") != 0 )
    {
        m_bSetupMode = FALSE;
    }
    m_bAudioTrack = FALSE;
#endif
#if _SCHOOL & _PERSO
    Error err = RegQuery();
    if ( err != ERR_OK )  return err;
    m_bAudioTrack = FALSE;
#endif
#if _SCHOOL & _CEEBOTDEMO
    m_bAudioTrack = FALSE;
#endif
#if _NET
    m_bAudioTrack = FALSE;
#endif
#if _DEMO
    m_bAudioTrack = FALSE;
#endif

    return ERR_OK;
}


// Returns the total amount of video memory for textures.

int CD3DApplication::GetVidMemTotal()
{
    return m_vidMemTotal;
}

BOOL CD3DApplication::IsVideo8MB()
{
    if ( m_vidMemTotal == 0 )  return FALSE;
    return (m_vidMemTotal <= 8388608L);  // 8 Mb or less (2 ^ 23)?
}

BOOL CD3DApplication::IsVideo32MB()
{
    if ( m_vidMemTotal == 0 )  return FALSE;
    return (m_vidMemTotal > 16777216L);  // more than 16 Mb (2 ^ 24)?
}


void CD3DApplication::SetShowStat(BOOL bShow)
{
    m_bShowStats = bShow;
}

BOOL CD3DApplication::RetShowStat()
{
    return m_bShowStats;
}


void CD3DApplication::SetDebugMode(BOOL bMode)
{
    m_bDebugMode = bMode;
    D3DTextr_SetDebugMode(m_bDebugMode);
}

BOOL CD3DApplication::RetDebugMode()
{
    return m_bDebugMode;
}

BOOL CD3DApplication::RetSetupMode()
{
    return m_bSetupMode;
}




// Son process of time management.

DWORD WINAPI ThreadRoutine(LPVOID)
{
    Event   event;
    float   time;
    int     ms, start, end, delay;

    ms = (int)(TIME_THREAD*1000.0f);
    time = 0.0f;
    while ( TRUE )
    {
        start = timeGetTime();

        g_pD3DApp->m_pD3DEngine->FrameMove(TIME_THREAD);

        ZeroMemory(&event, sizeof(Event));
        event.event = EVENT_FRAME;
        event.rTime = TIME_THREAD;
        event.axeX = AxeLimit(g_pD3DApp->m_axeKey.x + g_pD3DApp->m_axeJoy.x);
        event.axeY = AxeLimit(g_pD3DApp->m_axeKey.y + g_pD3DApp->m_axeJoy.y);
        event.axeZ = AxeLimit(g_pD3DApp->m_axeKey.z + g_pD3DApp->m_axeJoy.z);
        event.keyState = g_pD3DApp->m_keyState;

        if ( g_pD3DApp->m_pRobotMain != 0 )
        {
            g_pD3DApp->m_pRobotMain->EventProcess(event);
        }

        end = timeGetTime();

        delay = ms-(end-start);
        if ( delay > 0 )
        {
            Sleep(delay);  // waiting 20ms-used
        }
        time += TIME_THREAD;
    }
    return 0;
}


// Called during device intialization, this code checks the device
// for some minimum set of capabilities.

HRESULT CD3DApplication::ConfirmDevice( DDCAPS* pddDriverCaps,
                                        D3DDEVICEDESC7* pd3dDeviceDesc )
{
//? if( pd3dDeviceDesc->wMaxVertexBlendMatrices < 2 )
//?     return E_FAIL;

    return S_OK;
}

// Create the application.

HRESULT CD3DApplication::Create( HINSTANCE hInst, TCHAR* strCmdLine )
{
    HRESULT hr;
    char    deviceName[100];
    char    modeName[100];
    int     iValue;
    DWORD   style;
    BOOL    bFull, b3D;

    m_instance = hInst;

    InitCurrentDirectory();

    // Enumerate available D3D devices. The callback is used so the app can
    // confirm/reject each enumerated device depending on its capabilities.
    if( FAILED( hr = D3DEnum_EnumerateDevices( m_fnConfirmDevice ) ) )
    {
        DisplayFrameworkError( hr, MSGERR_APPMUSTEXIT );
        return hr;
    }

    if( FAILED( hr = D3DEnum_SelectDefaultDevice( &m_pDeviceInfo ) ) )
    {
        DisplayFrameworkError( hr, MSGERR_APPMUSTEXIT );
        return hr;
    }

    if ( !m_bDebugMode )
    {
        m_pDeviceInfo->bWindowed = FALSE;  // full screen
    }
    if ( GetLocalProfileInt("Device", "FullScreen", bFull) )
    {
        m_pDeviceInfo->bWindowed = !bFull;
    }

    // Create the 3D engine.
    if( (m_pD3DEngine = new CD3DEngine(m_iMan, this)) == NULL )
    {
        DisplayFrameworkError( D3DENUMERR_ENGINE, MSGERR_APPMUSTEXIT );
        return E_OUTOFMEMORY;
    }
    SetEngine(m_pD3DEngine);

    // Initialize the app's custom scene stuff
    if( FAILED( hr = m_pD3DEngine->OneTimeSceneInit() ) )
    {
        DisplayFrameworkError( hr, MSGERR_APPMUSTEXIT );
        return hr;
    }

    // Create a new CD3DFramework class. This class does all of our D3D
    // initialization and manages the common D3D objects.
    if( (m_pFramework = new CD3DFramework7()) == NULL )
    {
        DisplayFrameworkError( E_OUTOFMEMORY, MSGERR_APPMUSTEXIT );
        return E_OUTOFMEMORY;
    }

    // Create the sound instance.
    if( (m_pSound = new CSound(m_iMan)) == NULL )
    {
        DisplayFrameworkError( D3DENUMERR_SOUND, MSGERR_APPMUSTEXIT );
        return E_OUTOFMEMORY;
    }

    // Create the robot application.
    if( (m_pRobotMain = new CRobotMain(m_iMan)) == NULL )
    {
        DisplayFrameworkError( D3DENUMERR_ROBOT, MSGERR_APPMUSTEXIT );
        return E_OUTOFMEMORY;
    }

    // Register the window class
    WNDCLASS wndClass = { 0, WndProc, 0, 0, hInst,
                          LoadIcon( hInst, MAKEINTRESOURCE(IDI_MAIN_ICON) ),
                          LoadCursor( NULL, IDC_ARROW ),
                          (HBRUSH)GetStockObject(WHITE_BRUSH),
                          NULL, _T("D3D Window") };
    RegisterClass( &wndClass );

    // Create the render window
    style = WS_CAPTION|WS_VISIBLE;
    if ( m_bDebugMode )  style |= WS_SYSMENU;  // close box
    m_hWnd = CreateWindow( _T("D3D Window"), m_strWindowTitle,
//?                        WS_OVERLAPPEDWINDOW|WS_VISIBLE,
                           style, CW_USEDEFAULT, CW_USEDEFAULT,
                           WINDOW_DX, WINDOW_DY, 0L,
//?                        LoadMenu( hInst, MAKEINTRESOURCE(IDR_MENU) ),
                           NULL,
                           hInst, 0L );
    UpdateWindow( m_hWnd );

    if ( !GetLocalProfileInt("Setup", "Sound3D", b3D) )
    {
        b3D = TRUE;
    }
    m_pSound->SetDebugMode(m_bDebugMode);
    m_pSound->Create(m_hWnd, b3D);
    m_pSound->CacheAll();
    m_pSound->SetState(m_bAudioState);
    m_pSound->SetAudioTrack(m_bAudioTrack);
    m_pSound->SetCDpath(m_CDpath);

    // Initialize the 3D environment for the app
    if( FAILED( hr = Initialize3DEnvironment() ) )
    {
        DisplayFrameworkError( hr, MSGERR_APPMUSTEXIT );
        Cleanup3DEnvironment();
        return E_FAIL;
    }

    // Change the display device driver.
    GetLocalProfileString("Device", "Name", deviceName, 100);
    GetLocalProfileString("Device", "Mode", modeName, 100);
    GetLocalProfileInt("Device", "FullScreen", bFull);
    if ( deviceName[0] != 0 && modeName[0] != 0 && bFull )
    {
        ChangeDevice(deviceName, modeName, bFull);
    }

    // First execution?
    if ( !GetLocalProfileInt("Setup", "ObjectDirty", iValue) )
    {
        m_pD3DEngine->FirstExecuteAdapt(TRUE);
    }

    // Creates the file colobot.ini at the first execution.
    m_pRobotMain->CreateIni();

#if _DEMO
    m_pRobotMain->ChangePhase(PHASE_NAME);
#else
#if _NET | _SCHOOL
    m_pRobotMain->ChangePhase(PHASE_WELCOME2);
#else
#if _FRENCH
    m_pRobotMain->ChangePhase(PHASE_WELCOME2);
#endif
#if _ENGLISH
    m_pRobotMain->ChangePhase(PHASE_WELCOME2);
#endif
#if _GERMAN
    m_pRobotMain->ChangePhase(PHASE_WELCOME2);
#endif
#if _WG
    m_pRobotMain->ChangePhase(PHASE_WELCOME1);
#endif
#if _POLISH
    m_pRobotMain->ChangePhase(PHASE_WELCOME1);
#endif
#endif
#endif
    m_pD3DEngine->TimeInit();

#if USE_THREAD
    m_thread = CreateThread(NULL, 0, ThreadRoutine, this, 0, &m_threadId);
    SetThreadPriority(m_thread, THREAD_PRIORITY_ABOVE_NORMAL);
#endif

    // The app is ready to go
    m_bReady = TRUE;

    return S_OK;
}


// Message-processing loop. Idle time is used to render the scene.

INT CD3DApplication::Run()
{
    // Load keyboard accelerators
    HACCEL hAccel = LoadAccelerators( NULL, MAKEINTRESOURCE(IDR_MAIN_ACCEL) );

    // Now we're ready to recieve and process Windows messages.
    BOOL bGotMsg;
    MSG  msg;
    PeekMessage( &msg, NULL, 0U, 0U, PM_NOREMOVE );

    while( WM_QUIT != msg.message  )
    {
        // Use PeekMessage() if the app is active, so we can use idle time to
        // render the scene. Else, use GetMessage() to avoid eating CPU time.
        if( m_bActive )
            bGotMsg = PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE );
        else
            bGotMsg = GetMessage( &msg, NULL, 0U, 0U );

        if( bGotMsg )
        {
            // Translate and dispatch the message
            if( TranslateAccelerator( m_hWnd, hAccel, &msg ) == 0 )
            {
                TranslateMessage( &msg );
                DispatchMessage( &msg );
            }
        }
        else
        {
            // Render a frame during idle time (no messages are waiting)
            if( m_bActive && m_bReady )
            {
                Event   event;

                while ( m_event->GetEvent(event) )
                {
                    if ( event.event == EVENT_QUIT )
                    {
//?                     SendMessage( m_hWnd, WM_CLOSE, 0, 0 );
                        m_pSound->StopMusic();
                        Cleanup3DEnvironment();
                        PostQuitMessage(0);
                        return msg.wParam;
                    }
                    m_pRobotMain->EventProcess(event);
                }

                if ( !RetNiceMouse() )
                {
                    SetMouseType(m_pD3DEngine->RetMouseType());
                }

                if( FAILED( Render3DEnvironment() ) )
                    DestroyWindow( m_hWnd );
            }
        }
    }

    return msg.wParam;
}



// Conversion of the position of the mouse.
// x: 0=left, 1=right
// y: 0=down, 1=up

FPOINT CD3DApplication::ConvPosToInterface(HWND hWnd, LPARAM lParam)
{
    POINT   cpos;
    FPOINT  pos;
    float   px, py, w, h;

    cpos.x = (short)LOWORD(lParam);
    cpos.y = (short)HIWORD(lParam);

    if ( !m_pDeviceInfo->bWindowed )
    {
        ClientToScreen(hWnd, &cpos);
    }

    px = (float)cpos.x;
    py = (float)cpos.y;
    w  = (float)m_ddsdRenderTarget.dwWidth;
    h  = (float)m_ddsdRenderTarget.dwHeight;

    pos.x = px/w;
    pos.y = 1.0f-py/h;

    return pos;
}

// Physically moves the mouse.

void CD3DApplication::SetMousePos(FPOINT pos)
{
    POINT   p;

    pos.y = 1.0f-pos.y;

    pos.x *= m_ddsdRenderTarget.dwWidth;
    pos.y *= m_ddsdRenderTarget.dwHeight;

    p.x = (int)pos.x;
    p.y = (int)pos.y;
    ClientToScreen(m_hWnd, &p);

    SetCursorPos(p.x, p.y);
}

// Choosing the type of cursor for the mouse.

void CD3DApplication::SetMouseType(D3DMouse type)
{
    HCURSOR     hc;

    if ( type == D3DMOUSEHAND )
    {
        hc = LoadCursor(m_instance, MAKEINTRESOURCE(IDC_CURSORHAND));
    }
    else if ( type == D3DMOUSECROSS )
    {
        hc = LoadCursor(NULL, IDC_CROSS);
    }
    else if ( type == D3DMOUSEEDIT )
    {
        hc = LoadCursor(NULL, IDC_IBEAM);
    }
    else if ( type == D3DMOUSENO )
    {
        hc = LoadCursor(NULL, IDC_NO);
    }
    else if ( type == D3DMOUSEMOVE )
    {
        hc = LoadCursor(NULL, IDC_SIZEALL);
    }
    else if ( type == D3DMOUSEMOVEH )
    {
        hc = LoadCursor(NULL, IDC_SIZEWE);
    }
    else if ( type == D3DMOUSEMOVEV )
    {
        hc = LoadCursor(NULL, IDC_SIZENS);
    }
    else if ( type == D3DMOUSEMOVED )
    {
        hc = LoadCursor(NULL, IDC_SIZENESW);
    }
    else if ( type == D3DMOUSEMOVEI )
    {
        hc = LoadCursor(NULL, IDC_SIZENWSE);
    }
    else if ( type == D3DMOUSEWAIT )
    {
        hc = LoadCursor(NULL, IDC_WAIT);
    }
    else if ( type == D3DMOUSESCROLLL )
    {
        hc = LoadCursor(m_instance, MAKEINTRESOURCE(IDC_CURSORSCROLLL));
    }
    else if ( type == D3DMOUSESCROLLR )
    {
        hc = LoadCursor(m_instance, MAKEINTRESOURCE(IDC_CURSORSCROLLR));
    }
    else if ( type == D3DMOUSESCROLLU )
    {
        hc = LoadCursor(m_instance, MAKEINTRESOURCE(IDC_CURSORSCROLLU));
    }
    else if ( type == D3DMOUSESCROLLD )
    {
        hc = LoadCursor(m_instance, MAKEINTRESOURCE(IDC_CURSORSCROLLD));
    }
    else if ( type == D3DMOUSETARGET )
    {
        hc = LoadCursor(m_instance, MAKEINTRESOURCE(IDC_CURSORTARGET));
    }
    else
    {
        hc = LoadCursor(NULL, IDC_ARROW);
    }

    if ( hc != NULL )
    {
        SetCursor(hc);
    }
}

// Choice of mode for the mouse.

void CD3DApplication::SetNiceMouse(BOOL bNice)
{
    if ( bNice == m_bNiceMouse )  return;
    m_bNiceMouse = bNice;

    if ( m_bNiceMouse )
    {
        ShowCursor(FALSE);  // hides the ugly windows mouse
        SetCursor(NULL);
    }
    else
    {
        ShowCursor(TRUE);  // shows the ugly windows mouse
        SetCursor(LoadCursor(NULL, IDC_ARROW));
    }
}

// Whether to use the mouse pretty shaded.

BOOL CD3DApplication::RetNiceMouse()
{
    if (  m_pDeviceInfo->bWindowed )  return FALSE;
    if ( !m_pDeviceInfo->bHardware )  return FALSE;

    return m_bNiceMouse;
}

// Indicates whether it is possible to use the mouse pretty shaded.

BOOL CD3DApplication::RetNiceMouseCap()
{
    if (  m_pDeviceInfo->bWindowed )  return FALSE;
    if ( !m_pDeviceInfo->bHardware )  return FALSE;

    return TRUE;
}


// Static msg handler which passes messages to the application class.

LRESULT CALLBACK WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    if ( g_pD3DApp != 0 )
    {
        Event   event;
        short   move;

        ZeroMemory(&event, sizeof(Event));

#if 0
        if ( uMsg == WM_KEYDOWN ||
             uMsg == WM_CHAR ||
             uMsg == WM_XBUTTONDOWN ||
             uMsg == WM_XBUTTONUP )
        {
            char s[100];
            sprintf(s, "event: %d %d %d\n", uMsg, wParam, lParam);
            OutputDebugString(s);
        }
#endif

        if ( uMsg == WM_LBUTTONDOWN )  event.event = EVENT_LBUTTONDOWN;
        if ( uMsg == WM_RBUTTONDOWN )  event.event = EVENT_RBUTTONDOWN;
        if ( uMsg == WM_LBUTTONUP   )  event.event = EVENT_LBUTTONUP;
        if ( uMsg == WM_RBUTTONUP   )  event.event = EVENT_RBUTTONUP;
        if ( uMsg == WM_MOUSEMOVE   )  event.event = EVENT_MOUSEMOVE;
        if ( uMsg == WM_KEYDOWN     )  event.event = EVENT_KEYDOWN;
        if ( uMsg == WM_KEYUP       )  event.event = EVENT_KEYUP;
        if ( uMsg == WM_CHAR        )  event.event = EVENT_CHAR;

        if ( uMsg == WM_XBUTTONUP )
        {
            if ( (wParam>>16) == XBUTTON1 )  event.event = EVENT_HYPER_PREV;
            if ( (wParam>>16) == XBUTTON2 )  event.event = EVENT_HYPER_NEXT;
        }

        event.param = wParam;
        event.axeX = AxeLimit(g_pD3DApp->m_axeKey.x + g_pD3DApp->m_axeJoy.x);
        event.axeY = AxeLimit(g_pD3DApp->m_axeKey.y + g_pD3DApp->m_axeJoy.y);
        event.axeZ = AxeLimit(g_pD3DApp->m_axeKey.z + g_pD3DApp->m_axeJoy.z);
        event.keyState = g_pD3DApp->m_keyState;

        if ( uMsg == WM_LBUTTONDOWN ||
             uMsg == WM_RBUTTONDOWN ||
             uMsg == WM_LBUTTONUP   ||
             uMsg == WM_RBUTTONUP   ||
             uMsg == WM_MOUSEMOVE   )  // mouse event?
        {
            event.pos = g_pD3DApp->ConvPosToInterface(hWnd, lParam);
            g_pD3DApp->m_mousePos = event.pos;
            g_pD3DApp->m_pD3DEngine->SetMousePos(event.pos);
        }

        if ( uMsg == WM_MOUSEWHEEL )  // mouse wheel?
        {
            event.event = EVENT_KEYDOWN;
            event.pos = g_pD3DApp->m_mousePos;
            move = HIWORD(wParam);
            if ( move/WHEEL_DELTA > 0 )  event.param = VK_WHEELUP;
            if ( move/WHEEL_DELTA < 0 )  event.param = VK_WHEELDOWN;
        }
        if ( g_pD3DApp->m_mshMouseWheel != 0 &&
             uMsg == g_pD3DApp->m_mshMouseWheel )  // Logitech mouse wheel?
        {
            event.event = EVENT_KEYDOWN;
            event.pos = g_pD3DApp->m_mousePos;
            move = LOWORD(wParam);
            if ( move/WHEEL_DELTA > 0 )  event.param = VK_WHEELUP;
            if ( move/WHEEL_DELTA < 0 )  event.param = VK_WHEELDOWN;
        }

        if ( event.event == EVENT_KEYDOWN ||
             event.event == EVENT_KEYUP   ||
             event.event == EVENT_CHAR    )
        {
            if ( event.param == 0 )
            {
                event.event = EVENT_NULL;
            }
        }

        if ( g_pD3DApp->m_pRobotMain != 0 && event.event != 0 )
        {
            g_pD3DApp->m_pRobotMain->EventProcess(event);
//?         if ( !g_pD3DApp->RetNiceMouse() )
//?         {
//?             g_pD3DApp->SetMouseType(g_pD3DApp->m_pD3DEngine->RetMouseType());
//?         }
        }
        if ( g_pD3DApp->m_pD3DEngine != 0 )
        {
            g_pD3DApp->m_pD3DEngine->MsgProc( hWnd, uMsg, wParam, lParam );
        }
        return g_pD3DApp->MsgProc( hWnd, uMsg, wParam, lParam );
    }

    return DefWindowProc( hWnd, uMsg, wParam, lParam );
}


// Minimal message proc function for the about box.

BOOL CALLBACK AboutProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM )
{
    if( WM_COMMAND == uMsg )
        if( IDOK == LOWORD(wParam) || IDCANCEL == LOWORD(wParam) )
            EndDialog( hWnd, TRUE );

    return WM_INITDIALOG == uMsg ? TRUE : FALSE;
}



// Ignore keypresses.

void CD3DApplication::FlushPressKey()
{
    m_keyState = 0;
    m_axeKey = D3DVECTOR(0.0f, 0.0f, 0.0f);
    m_axeJoy = D3DVECTOR(0.0f, 0.0f, 0.0f);
}

// Resets the default keys.

void CD3DApplication::ResetKey()
{
    int     i;

    for ( i=0 ; i<50 ; i++ )
    {
        m_key[i][0] = 0;
        m_key[i][1] = 0;
    }
    m_key[KEYRANK_LEFT   ][0] = VK_LEFT;
    m_key[KEYRANK_RIGHT  ][0] = VK_RIGHT;
    m_key[KEYRANK_UP     ][0] = VK_UP;
    m_key[KEYRANK_DOWN   ][0] = VK_DOWN;
    m_key[KEYRANK_GUP    ][0] = VK_SHIFT;
    m_key[KEYRANK_GDOWN  ][0] = VK_CONTROL;
    m_key[KEYRANK_CAMERA ][0] = VK_SPACE;
    m_key[KEYRANK_CAMERA ][1] = VK_BUTTON2;
    m_key[KEYRANK_DESEL  ][0] = VK_NUMPAD0;
    m_key[KEYRANK_DESEL  ][1] = VK_BUTTON6;
    m_key[KEYRANK_ACTION ][0] = VK_RETURN;
    m_key[KEYRANK_ACTION ][1] = VK_BUTTON1;
    m_key[KEYRANK_NEAR   ][0] = VK_ADD;
    m_key[KEYRANK_NEAR   ][1] = VK_BUTTON5;
    m_key[KEYRANK_AWAY   ][0] = VK_SUBTRACT;
    m_key[KEYRANK_AWAY   ][1] = VK_BUTTON4;
    m_key[KEYRANK_NEXT   ][0] = VK_TAB;
    m_key[KEYRANK_NEXT   ][1] = VK_BUTTON3;
    m_key[KEYRANK_HUMAN  ][0] = VK_HOME;
    m_key[KEYRANK_HUMAN  ][1] = VK_BUTTON7;
    m_key[KEYRANK_QUIT   ][0] = VK_ESCAPE;
    m_key[KEYRANK_HELP   ][0] = VK_F1;
    m_key[KEYRANK_PROG   ][0] = VK_F2;
    m_key[KEYRANK_CBOT   ][0] = VK_F3;
    m_key[KEYRANK_VISIT  ][0] = VK_DECIMAL;
    m_key[KEYRANK_SPEED10][0] = VK_F4;
    m_key[KEYRANK_SPEED15][0] = VK_F5;
    m_key[KEYRANK_SPEED20][0] = VK_F6;
//  m_key[KEYRANK_SPEED30][0] = VK_F7;
}

// Modifies a button.

void CD3DApplication::SetKey(int keyRank, int option, int key)
{
    if ( keyRank <  0  ||
         keyRank >= 50 )  return;

    if ( option <  0 ||
         option >= 2 )  return;

    m_key[keyRank][option] = key;
}

// Gives a hint.

int CD3DApplication::RetKey(int keyRank, int option)
{
    if ( keyRank <  0  ||
         keyRank >= 50 )  return 0;

    if ( option <  0 ||
         option >= 2 )  return 0;

    return m_key[keyRank][option];
}



// Use the joystick or keyboard.

void CD3DApplication::SetJoystick(BOOL bEnable)
{
    m_bJoystick = bEnable;

    if ( m_bJoystick )  // joystick ?
    {
        if ( !InitDirectInput(m_instance, m_hWnd) )  // initialise joystick
        {
            m_bJoystick = FALSE;
        }
        else
        {
            SetAcquire(TRUE);
            SetTimer(m_hWnd, 0, 1000/30, NULL);
        }
    }
    else    // keyboard?
    {
        KillTimer(m_hWnd, 0);
        SetAcquire(FALSE);
        FreeDirectInput();
    }
}

BOOL CD3DApplication::RetJoystick()
{
    return m_bJoystick;
}


// Message handling function.

LRESULT CD3DApplication::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam,
                                  LPARAM lParam )
{
    HRESULT     hr;
    DIJOYSTATE  js;
    int         i;

    // The F10 key sends another message to activate
    // menu in standard Windows applications!
    if ( uMsg == WM_SYSKEYDOWN && wParam == VK_F10 )
    {
        uMsg = WM_KEYDOWN;
    }
    if ( uMsg == WM_SYSKEYUP && wParam == VK_F10 )
    {
        uMsg = WM_KEYUP;
    }

    // Mange event "menu" sent by Alt or F10.
    if ( uMsg == WM_SYSCOMMAND && wParam == SC_KEYMENU )
    {
        return 0;
    }

    if ( uMsg == WM_KEYDOWN || uMsg == WM_KEYUP )
    {
        if ( GetKeyState(VK_SHIFT) & 0x8000 )
        {
            m_keyState |= KS_SHIFT;
        }
        else
        {
            m_keyState &= ~KS_SHIFT;
        }

        if ( GetKeyState(VK_CONTROL) & 0x8000 )
        {
            m_keyState |= KS_CONTROL;
        }
        else
        {
            m_keyState &= ~KS_CONTROL;
        }
    }

    switch( uMsg )
    {
        case WM_KEYDOWN:
            if ( wParam == m_key[KEYRANK_UP   ][0] )  m_axeKey.y =  1.0f;
            if ( wParam == m_key[KEYRANK_UP   ][1] )  m_axeKey.y =  1.0f;
            if ( wParam == m_key[KEYRANK_DOWN ][0] )  m_axeKey.y = -1.0f;
            if ( wParam == m_key[KEYRANK_DOWN ][1] )  m_axeKey.y = -1.0f;
            if ( wParam == m_key[KEYRANK_LEFT ][0] )  m_axeKey.x = -1.0f;
            if ( wParam == m_key[KEYRANK_LEFT ][1] )  m_axeKey.x = -1.0f;
            if ( wParam == m_key[KEYRANK_RIGHT][0] )  m_axeKey.x =  1.0f;
            if ( wParam == m_key[KEYRANK_RIGHT][1] )  m_axeKey.x =  1.0f;
            if ( wParam == m_key[KEYRANK_GUP  ][0] )  m_axeKey.z =  1.0f;
            if ( wParam == m_key[KEYRANK_GUP  ][1] )  m_axeKey.z =  1.0f;
            if ( wParam == m_key[KEYRANK_GDOWN][0] )  m_axeKey.z = -1.0f;
            if ( wParam == m_key[KEYRANK_GDOWN][1] )  m_axeKey.z = -1.0f;
            if ( wParam == m_key[KEYRANK_NEAR ][0] )  m_keyState |= KS_NUMPLUS;
            if ( wParam == m_key[KEYRANK_NEAR ][1] )  m_keyState |= KS_NUMPLUS;
            if ( wParam == m_key[KEYRANK_AWAY ][0] )  m_keyState |= KS_NUMMINUS;
            if ( wParam == m_key[KEYRANK_AWAY ][1] )  m_keyState |= KS_NUMMINUS;
            if ( wParam == VK_PRIOR                )  m_keyState |= KS_PAGEUP;
            if ( wParam == VK_NEXT                 )  m_keyState |= KS_PAGEDOWN;
//?         if ( wParam == VK_SHIFT                )  m_keyState |= KS_SHIFT;
//?         if ( wParam == VK_CONTROL              )  m_keyState |= KS_CONTROL;
            if ( wParam == VK_NUMPAD8              )  m_keyState |= KS_NUMUP;
            if ( wParam == VK_NUMPAD2              )  m_keyState |= KS_NUMDOWN;
            if ( wParam == VK_NUMPAD4              )  m_keyState |= KS_NUMLEFT;
            if ( wParam == VK_NUMPAD6              )  m_keyState |= KS_NUMRIGHT;
            break;

        case WM_KEYUP:
            if ( wParam == m_key[KEYRANK_UP   ][0] )  m_axeKey.y = 0.0f;
            if ( wParam == m_key[KEYRANK_UP   ][1] )  m_axeKey.y = 0.0f;
            if ( wParam == m_key[KEYRANK_DOWN ][0] )  m_axeKey.y = 0.0f;
            if ( wParam == m_key[KEYRANK_DOWN ][1] )  m_axeKey.y = 0.0f;
            if ( wParam == m_key[KEYRANK_LEFT ][0] )  m_axeKey.x = 0.0f;
            if ( wParam == m_key[KEYRANK_LEFT ][1] )  m_axeKey.x = 0.0f;
            if ( wParam == m_key[KEYRANK_RIGHT][0] )  m_axeKey.x = 0.0f;
            if ( wParam == m_key[KEYRANK_RIGHT][1] )  m_axeKey.x = 0.0f;
            if ( wParam == m_key[KEYRANK_GUP  ][0] )  m_axeKey.z = 0.0f;
            if ( wParam == m_key[KEYRANK_GUP  ][1] )  m_axeKey.z = 0.0f;
            if ( wParam == m_key[KEYRANK_GDOWN][0] )  m_axeKey.z = 0.0f;
            if ( wParam == m_key[KEYRANK_GDOWN][1] )  m_axeKey.z = 0.0f;
            if ( wParam == m_key[KEYRANK_NEAR ][0] )  m_keyState &= ~KS_NUMPLUS;
            if ( wParam == m_key[KEYRANK_NEAR ][1] )  m_keyState &= ~KS_NUMPLUS;
            if ( wParam == m_key[KEYRANK_AWAY ][0] )  m_keyState &= ~KS_NUMMINUS;
            if ( wParam == m_key[KEYRANK_AWAY ][1] )  m_keyState &= ~KS_NUMMINUS;
            if ( wParam == VK_PRIOR                )  m_keyState &= ~KS_PAGEUP;
            if ( wParam == VK_NEXT                 )  m_keyState &= ~KS_PAGEDOWN;
//?         if ( wParam == VK_SHIFT                )  m_keyState &= ~KS_SHIFT;
//?         if ( wParam == VK_CONTROL              )  m_keyState &= ~KS_CONTROL;
            if ( wParam == VK_NUMPAD8              )  m_keyState &= ~KS_NUMUP;
            if ( wParam == VK_NUMPAD2              )  m_keyState &= ~KS_NUMDOWN;
            if ( wParam == VK_NUMPAD4              )  m_keyState &= ~KS_NUMLEFT;
            if ( wParam == VK_NUMPAD6              )  m_keyState &= ~KS_NUMRIGHT;
            break;

        case WM_LBUTTONDOWN:
            m_keyState |= KS_MLEFT;
            break;

        case WM_RBUTTONDOWN:
            m_keyState |= KS_MRIGHT;
            break;

        case WM_LBUTTONUP:
            m_keyState &= ~KS_MLEFT;
            break;

        case WM_RBUTTONUP:
            m_keyState &= ~KS_MRIGHT;
            break;

        case WM_PAINT:
            // Handle paint messages when the app is not ready
            if( m_pFramework && !m_bReady )
            {
                if( m_pDeviceInfo->bWindowed )
                    m_pFramework->ShowFrame();
                else
                    m_pFramework->FlipToGDISurface( TRUE );
            }
            break;

        case WM_MOVE:
            // If in windowed mode, move the Framework's window
            if( m_pFramework && m_bActive && m_bReady && m_pDeviceInfo->bWindowed )
                m_pFramework->Move( (SHORT)LOWORD(lParam), (SHORT)HIWORD(lParam) );
            break;

        case WM_SIZE:
            // Check to see if we are losing our window...
            if( SIZE_MAXHIDE==wParam || SIZE_MINIMIZED==wParam )
            {
                m_bActive = FALSE;
            }
            else
            {
                m_bActive = TRUE;
            }
//?         char s[100];
//?         sprintf(s, "WM_SIZE %d %d %d\n", m_bActive, m_bReady, m_pDeviceInfo->bWindowed);
//?         OutputDebugString(s);

            // A new window size will require a new backbuffer
            // size, so the 3D structures must be changed accordingly.
            if( m_bActive && m_bReady && m_pDeviceInfo->bWindowed )
            {
                m_bReady = FALSE;

//?             OutputDebugString("WM_SIZE Change3DEnvironment\n");
                if( FAILED( hr = Change3DEnvironment() ) )
                    return 0;

                m_bReady = TRUE;
            }
            break;

        case WM_TIMER:
            if ( m_bActivateApp && m_bJoystick )
            {
                if ( UpdateInputState(js) )
                {
                    m_axeJoy.x =  js.lX/1000.0f+js.lRz/1000.0f;  // tourner
                    m_axeJoy.y = -js.lY/1000.0f;  // avancer
                    m_axeJoy.z = -js.rglSlider[0]/1000.0f;  // monter

                    m_axeJoy.x = Neutral(m_axeJoy.x, 0.2f);
                    m_axeJoy.y = Neutral(m_axeJoy.y, 0.2f);
                    m_axeJoy.z = Neutral(m_axeJoy.z, 0.2f);

//?                 char s[100];
//?                 sprintf(s, "x=%d y=%d z=%  x=%d y=%d z=%d\n", js.lX,js.lY,js.lZ,js.lRx,js.lRy,js.lRz);
//?                 OutputDebugString(s);

                    for ( i=0 ; i<32 ; i++ )
                    {
                        if ( js.rgbButtons[i] != 0 && !m_bJoyButton[i] )
                        {
                            m_bJoyButton[i] = TRUE;
                            PostMessage(m_hWnd, WM_KEYDOWN, VK_BUTTON1+i, 0);
                        }
                        if ( js.rgbButtons[i] == 0 && m_bJoyButton[i] )
                        {
                            m_bJoyButton[i] = FALSE;
                            PostMessage(m_hWnd, WM_KEYUP, VK_BUTTON1+i, 0);
                        }
                    }
                }
                else
                {
                    OutputDebugString("UpdateInputState error\n");
                }
            }
            break;

        case WM_ACTIVATE:
            if( LOWORD(wParam) == WA_INACTIVE )
            {
                m_bActivateApp = FALSE;
            }
            else
            {
                m_bActivateApp = TRUE;
            }

            if ( m_bActivateApp && m_bJoystick )
            {
                SetAcquire(TRUE);  // re-enables the joystick
            }
            break;

        case MM_MCINOTIFY:
            if ( wParam == MCI_NOTIFY_SUCCESSFUL )
            {
                OutputDebugString("Event MM_MCINOTIFY\n");
                m_pSound->SuspendMusic();
                m_pSound->RestartMusic();
            }
            break;

        case WM_SETCURSOR:
            // Prevent a cursor in fullscreen mode
            if( m_bActive && m_bReady && !m_pDeviceInfo->bWindowed )
            {
//?             SetCursor(NULL);
                return 1;
            }
            break;

        case WM_ENTERMENULOOP:
            // Pause the app when menus are displayed
            Pause(TRUE);
            break;
        case WM_EXITMENULOOP:
            Pause(FALSE);
            break;

        case WM_ENTERSIZEMOVE:
            // Halt frame movement while the app is sizing or moving
            m_pD3DEngine->TimeEnterGel();
            break;
        case WM_EXITSIZEMOVE:
            m_pD3DEngine->TimeExitGel();
            break;

        case WM_NCHITTEST:
            // Prevent the user from selecting the menu in fullscreen mode
            if( !m_pDeviceInfo->bWindowed )
                return HTCLIENT;

            break;

        case WM_POWERBROADCAST:
            switch( wParam )
            {
                case PBT_APMQUERYSUSPEND:
                    // At this point, the app should save any data for open
                    // network connections, files, etc.., and prepare to go into
                    // a suspended mode.
                    return OnQuerySuspend( (DWORD)lParam );

                case PBT_APMRESUMESUSPEND:
                    // At this point, the app should recover any data, network
                    // connections, files, etc.., and resume running from when
                    // the app was suspended.
                    return OnResumeSuspend( (DWORD)lParam );
            }
            break;

        case WM_SYSCOMMAND:
            // Prevent moving/sizing and power loss in fullscreen mode
            switch( wParam )
            {
                case SC_MOVE:
                case SC_SIZE:
                case SC_MAXIMIZE:
                case SC_MONITORPOWER:
                    if( FALSE == m_pDeviceInfo->bWindowed )
                        return 1;
                    break;
            }
            break;

        case WM_COMMAND:
            switch( LOWORD(wParam) )
            {
                case IDM_CHANGEDEVICE:
                    // Display the device-selection dialog box.
                    if( m_bActive && m_bReady )
                    {
                        Pause(TRUE);

                        if( SUCCEEDED( D3DEnum_UserChangeDevice( &m_pDeviceInfo ) ) )
                        {
                            if( FAILED( hr = Change3DEnvironment() ) )
                                return 0;
                        }
                        Pause(FALSE);
                    }
                    return 0;

                case IDM_ABOUT:
                    // Display the About box
                    Pause(TRUE);
                    DialogBox( (HINSTANCE)GetWindowLong( hWnd, GWL_HINSTANCE ),
                               MAKEINTRESOURCE(IDD_ABOUT), hWnd, AboutProc );
                    Pause(FALSE);
                    return 0;

                case IDM_EXIT:
                    // Recieved key/menu command to exit app
                    SendMessage( hWnd, WM_CLOSE, 0, 0 );
                    return 0;
            }
            break;

        case WM_GETMINMAXINFO:
            ((MINMAXINFO*)lParam)->ptMinTrackSize.x = 100;
            ((MINMAXINFO*)lParam)->ptMinTrackSize.y = 100;
            break;

        case WM_CLOSE:
            DestroyWindow( hWnd );
            return 0;

        case WM_DESTROY:
            Cleanup3DEnvironment();
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProc( hWnd, uMsg, wParam, lParam );
}


// Enumeration function to report valid pixel formats for z-buffers.

HRESULT WINAPI EnumZBufferFormatsCallback(DDPIXELFORMAT* pddpf,
                                          VOID* pContext)
{
    DDPIXELFORMAT* pddpfOut = (DDPIXELFORMAT*)pContext;

    char s[100];
    sprintf(s, "EnumZBufferFormatsCallback %d\n", pddpf->dwRGBBitCount);
    OutputDebugString(s);

    if( pddpfOut->dwRGBBitCount == pddpf->dwRGBBitCount )
    {
        (*pddpfOut) = (*pddpf);
        return D3DENUMRET_CANCEL;
    }

    return D3DENUMRET_OK;
}

// Internal function called by Create() to make and attach a zbuffer
// to the renderer.

HRESULT CD3DApplication::CreateZBuffer(GUID* pDeviceGUID)
{
    HRESULT hr;

    // Check if the device supports z-bufferless hidden surface removal. If so,
    // we don't really need a z-buffer
    D3DDEVICEDESC7 ddDesc;
    m_pD3DDevice->GetCaps( &ddDesc );
    if( ddDesc.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_ZBUFFERLESSHSR )
        return S_OK;

    // Get z-buffer dimensions from the render target
    DDSURFACEDESC2 ddsd;
    ddsd.dwSize = sizeof(ddsd);
    m_pddsRenderTarget->GetSurfaceDesc( &ddsd );

    // Setup the surface desc for the z-buffer.
    ddsd.dwFlags        = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS | DDSD_PIXELFORMAT;
    ddsd.ddsCaps.dwCaps = DDSCAPS_ZBUFFER | DDSCAPS_VIDEOMEMORY;
    ddsd.ddpfPixelFormat.dwSize = 0;  // Tag the pixel format as unitialized

    // Get an appropiate pixel format from enumeration of the formats. On the
    // first pass, we look for a zbuffer dpeth which is equal to the frame
    // buffer depth (as some cards unfornately require this).
    m_pD3D->EnumZBufferFormats( *pDeviceGUID, EnumZBufferFormatsCallback,
                                (VOID*)&ddsd.ddpfPixelFormat );
    if( 0 == ddsd.ddpfPixelFormat.dwSize )
    {
        // Try again, just accepting any 16-bit zbuffer
        ddsd.ddpfPixelFormat.dwRGBBitCount = 16;
        m_pD3D->EnumZBufferFormats( *pDeviceGUID, EnumZBufferFormatsCallback,
                                    (VOID*)&ddsd.ddpfPixelFormat );

        if( 0 == ddsd.ddpfPixelFormat.dwSize )
        {
            DEBUG_MSG( _T("Device doesn't support requested zbuffer format") );
            return D3DFWERR_NOZBUFFER;
        }
    }

    // Create and attach a z-buffer
    if( FAILED( hr = m_pDD->CreateSurface( &ddsd, &m_pddsDepthBuffer, NULL ) ) )
    {
        DEBUG_MSG( _T("Error: Couldn't create a ZBuffer surface") );
        if( hr != DDERR_OUTOFVIDEOMEMORY )
            return D3DFWERR_NOZBUFFER;
        DEBUG_MSG( _T("Error: Out of video memory") );
        return DDERR_OUTOFVIDEOMEMORY;
    }

    if( FAILED( m_pddsRenderTarget->AddAttachedSurface( m_pddsDepthBuffer ) ) )
    {
        DEBUG_MSG( _T("Error: Couldn't attach zbuffer to render surface") );
        return D3DFWERR_NOZBUFFER;
    }

    // Finally, this call rebuilds internal structures
    if( FAILED( m_pD3DDevice->SetRenderTarget( m_pddsRenderTarget, 0L ) ) )
    {
        DEBUG_MSG( _T("Error: SetRenderTarget() failed after attaching zbuffer!") );
        return D3DFWERR_NOZBUFFER;
    }

    return S_OK;
}

// Initializes the sample framework, then calls the app-specific function
// to initialize device specific objects. This code is structured to
// handled any errors that may occur duing initialization.

HRESULT CD3DApplication::Initialize3DEnvironment()
{
    HRESULT     hr;
    DDSCAPS2    ddsCaps2;
    DWORD       dwFrameworkFlags = 0L;
    DWORD       dwTotal;
    DWORD       dwFree;

    dwFrameworkFlags |= ( !m_pDeviceInfo->bWindowed ? D3DFW_FULLSCREEN : 0L );
    dwFrameworkFlags |= (  m_pDeviceInfo->bStereo   ? D3DFW_STEREO     : 0L );
    dwFrameworkFlags |= (  m_bAppUseZBuffer         ? D3DFW_ZBUFFER    : 0L );

    // Initialize the D3D framework
    if( SUCCEEDED( hr = m_pFramework->Initialize( m_hWnd,
                     m_pDeviceInfo->pDriverGUID, m_pDeviceInfo->pDeviceGUID,
                     &m_pDeviceInfo->ddsdFullscreenMode, dwFrameworkFlags ) ) )
    {
        m_pDD        = m_pFramework->GetDirectDraw();
        m_pD3D       = m_pFramework->GetDirect3D();
        m_pD3DDevice = m_pFramework->GetD3DDevice();

        m_pD3DEngine->SetD3DDevice(m_pD3DDevice);

        m_pddsRenderTarget = m_pFramework->GetRenderSurface();

        m_ddsdRenderTarget.dwSize = sizeof(m_ddsdRenderTarget);
        m_pddsRenderTarget->GetSurfaceDesc( &m_ddsdRenderTarget );

        // Request the amount of video memory.
        ZeroMemory(&ddsCaps2, sizeof(ddsCaps2));
        ddsCaps2.dwCaps = DDSCAPS_TEXTURE;
        dwTotal = 0;
        hr = m_pDD->GetAvailableVidMem(&ddsCaps2, &dwTotal, &dwFree);
        m_vidMemTotal = dwTotal;

        // Let the app run its startup code which creates the 3d scene.
        if( SUCCEEDED( hr = m_pD3DEngine->InitDeviceObjects() ) )
        {
//?         CreateZBuffer(m_pDeviceInfo->pDeviceGUID);
            return S_OK;
        }
        else
        {
            DeleteDeviceObjects();
            m_pFramework->DestroyObjects();
        }
    }

    // If we get here, the first initialization passed failed. If that was with a
    // hardware device, try again using a software rasterizer instead.
    if( m_pDeviceInfo->bHardware )
    {
        // Try again with a software rasterizer
        DisplayFrameworkError( hr, MSGWARN_SWITCHEDTOSOFTWARE );
        D3DEnum_SelectDefaultDevice( &m_pDeviceInfo, D3DENUM_SOFTWAREONLY );
        return Initialize3DEnvironment();
    }

    return hr;
}


// Handles driver, device, and/or mode changes for the app.

HRESULT CD3DApplication::Change3DEnvironment()
{
#if 0
    HRESULT hr;
    static BOOL  bOldWindowedState = TRUE;
    static DWORD dwSavedStyle;
    static RECT  rcSaved;

    // Release all scene objects that will be re-created for the new device
    DeleteDeviceObjects();

    // Release framework objects, so a new device can be created
    if( FAILED( hr = m_pFramework->DestroyObjects() ) )
    {
        DisplayFrameworkError( hr, MSGERR_APPMUSTEXIT );
        SendMessage( m_hWnd, WM_CLOSE, 0, 0 );
        return hr;
    }

    // Check if going from fullscreen to windowed mode, or vice versa.
    if( bOldWindowedState != m_pDeviceInfo->bWindowed )
    {
        if( m_pDeviceInfo->bWindowed )
        {
            // Coming from fullscreen mode, so restore window properties
            SetWindowLong( m_hWnd, GWL_STYLE, dwSavedStyle );
            SetWindowPos( m_hWnd, HWND_NOTOPMOST, rcSaved.left, rcSaved.top,
                          ( rcSaved.right - rcSaved.left ),
                          ( rcSaved.bottom - rcSaved.top ), SWP_SHOWWINDOW );
        }
        else
        {
            // Going to fullscreen mode, save/set window properties as needed
            dwSavedStyle = GetWindowLong( m_hWnd, GWL_STYLE );
            GetWindowRect( m_hWnd, &rcSaved );
            SetWindowLong( m_hWnd, GWL_STYLE, WS_POPUP|WS_SYSMENU|WS_VISIBLE );
        }

        bOldWindowedState = m_pDeviceInfo->bWindowed;
    }

    // Inform the framework class of the driver change. It will internally
    // re-create valid surfaces, a d3ddevice, etc.
    if( FAILED( hr = Initialize3DEnvironment() ) )
    {
        DisplayFrameworkError( hr, MSGERR_APPMUSTEXIT );
        SendMessage( m_hWnd, WM_CLOSE, 0, 0 );
        return hr;
    }

    return S_OK;
#else
    HRESULT hr;

    // Release all scene objects that will be re-created for the new device
    DeleteDeviceObjects();

    // Release framework objects, so a new device can be created
    if( FAILED( hr = m_pFramework->DestroyObjects() ) )
    {
        DisplayFrameworkError( hr, MSGERR_APPMUSTEXIT );
        SendMessage( m_hWnd, WM_CLOSE, 0, 0 );
        return hr;
    }

    if( m_pDeviceInfo->bWindowed )
    {
        SetWindowPos(m_hWnd, HWND_NOTOPMOST, 10, 10, WINDOW_DX, WINDOW_DY, SWP_SHOWWINDOW);
    }

    // Inform the framework class of the driver change. It will internally
    // re-create valid surfaces, a d3ddevice, etc.
    if( FAILED( hr = Initialize3DEnvironment() ) )
    {
        DisplayFrameworkError( hr, MSGERR_APPMUSTEXIT );
        SendMessage( m_hWnd, WM_CLOSE, 0, 0 );
        return hr;
    }

    m_pD3DEngine->ChangeLOD();

    if( m_pDeviceInfo->bWindowed )
    {
        SetNiceMouse(FALSE);  // hides the ugly windows mouse
    }

    return S_OK;
#endif
}



// Evolved throughout the game

void CD3DApplication::StepSimul(float rTime)
{
    Event   event;

    if ( m_pRobotMain == 0 )  return;

    ZeroMemory(&event, sizeof(Event));
    event.event = EVENT_FRAME;  // funny bug release "Maximize speed"!
    event.rTime = rTime;
    event.axeX = AxeLimit(m_axeKey.x + m_axeJoy.x);
    event.axeY = AxeLimit(m_axeKey.y + m_axeJoy.y);
    event.axeZ = AxeLimit(m_axeKey.z + m_axeJoy.z);
    event.keyState = m_keyState;

//?char s[100];
//?sprintf(s, "StepSimul %.3f\n", event.rTime);
//?OutputDebugString(s);
    m_pRobotMain->EventProcess(event);
}


// Draws the scene.

HRESULT CD3DApplication::Render3DEnvironment()
{
    HRESULT hr;
    float   rTime;

    // Check the cooperative level before rendering
    if( FAILED( hr = m_pDD->TestCooperativeLevel() ) )
    {
        switch( hr )
        {
            case DDERR_EXCLUSIVEMODEALREADYSET:
            case DDERR_NOEXCLUSIVEMODE:
                OutputDebugString("DDERR_EXCLUSIVEMODEALREADYSET\n");
                // Do nothing because some other app has exclusive mode
                return S_OK;

            case DDERR_WRONGMODE:
                OutputDebugString("DDERR_WRONGMODE\n");
                // The display mode changed on us. Resize accordingly
                if( m_pDeviceInfo->bWindowed )
                    return Change3DEnvironment();
                break;
        }
        return hr;
    }

    // Get the relative time, in seconds
    rTime = m_pD3DEngine->TimeGet();
    if ( rTime > MAX_STEP )  rTime = MAX_STEP;  // never more than 0.5s!
    m_aTime += rTime;

#if !USE_THREAD
    if( FAILED( hr = m_pD3DEngine->FrameMove(rTime) ) )
        return hr;

    // FrameMove (animate) the scene
    StepSimul(rTime);
#endif

    // Render the scene.
    if( FAILED( hr = m_pD3DEngine->Render() ) )
        return hr;

    DrawSuppl();

    // Show the frame rate, etc.
    if( m_bShowStats )
        ShowStats();

    // Show the frame on the primary surface.
    if( FAILED( hr = m_pFramework->ShowFrame() ) )
    {
        if( DDERR_SURFACELOST != hr )
            return hr;

        m_pFramework->RestoreSurfaces();
        m_pD3DEngine->RestoreSurfaces();
    }

    return S_OK;
}


// Cleanup scene objects

VOID CD3DApplication::Cleanup3DEnvironment()
{
    m_bActive = FALSE;
    m_bReady  = FALSE;

    if( m_pFramework )
    {
        DeleteDeviceObjects();
        SAFE_DELETE( m_pFramework );

        m_pD3DEngine->FinalCleanup();
    }

    D3DEnum_FreeResources();
//? FreeDirectInput();
}

// Called when the app is exitting, or the device is being changed,
// this function deletes any device dependant objects.

VOID CD3DApplication::DeleteDeviceObjects()
{
    if( m_pFramework )
    {
        m_pD3DEngine->DeleteDeviceObjects();
        SAFE_RELEASE( m_pddsDepthBuffer );
    }
}



// Called in to toggle the pause state of the app. This function
// brings the GDI surface to the front of the display, so drawing
// output like message boxes and menus may be displayed.

VOID CD3DApplication::Pause( BOOL bPause )
{
    static DWORD dwAppPausedCount = 0L;

    dwAppPausedCount += ( bPause ? +1 : -1 );
    m_bReady          = ( dwAppPausedCount ? FALSE : TRUE );

    // Handle the first pause request (of many, nestable pause requests)
    if( bPause && ( 1 == dwAppPausedCount ) )
    {
        // Get a surface for the GDI
        if( m_pFramework )
            m_pFramework->FlipToGDISurface( TRUE );

        // Stop the scene from animating
        m_pD3DEngine->TimeEnterGel();
    }

    if( 0 == dwAppPausedCount )
    {
        // Restart the scene
        m_pD3DEngine->TimeExitGel();
    }
}


// Called when the app receives a PBT_APMQUERYSUSPEND message, meaning
// the computer is about to be suspended. At this point, the app should
// save any data for open network connections, files, etc.., and prepare
// to go into a suspended mode.

LRESULT CD3DApplication::OnQuerySuspend( DWORD dwFlags )
{
    OutputDebugString("OnQuerySuspend\n");
    Pause(TRUE);
    return TRUE;
}


// Called when the app receives a PBT_APMRESUMESUSPEND message, meaning
// the computer has just resumed from a suspended state. At this point,
// the app should recover any data, network connections, files, etc..,
// and resume running from when the app was suspended.

LRESULT CD3DApplication::OnResumeSuspend( DWORD dwData )
{
    OutputDebugString("OnResumeSuspend\n");
    Pause(FALSE);
    return TRUE;
}


// Draw all the additional graphic elements.

void CD3DApplication::DrawSuppl()
{
    HDC         hDC;
    FPOINT      p1, p2;
    POINT       list[3];
    RECT        rect;
    HPEN        hPen;
    HGDIOBJ     old;
    FPOINT      pos;
    float       d;
    int         nbOut;

    if ( FAILED(m_pddsRenderTarget->GetDC(&hDC)) )  return;

    // Displays the selection rectangle.
    if ( m_pD3DEngine->GetHilite(p1, p2) )
    {
        nbOut = 0;
        if ( p1.x < 0.0f || p1.x > 1.0f )  nbOut ++;
        if ( p1.y < 0.0f || p1.y > 1.0f )  nbOut ++;
        if ( p2.x < 0.0f || p2.x > 1.0f )  nbOut ++;
        if ( p2.y < 0.0f || p2.y > 1.0f )  nbOut ++;
        if ( nbOut <= 2 )
        {
#if 0
            time = Mod(m_aTime, 0.5f);
            if ( time < 0.25f )  d = time*4.0f;
            else                 d = (2.0f-time*4.0f);
#endif
#if 0
            time = Mod(m_aTime, 0.5f);
            if ( time < 0.4f )  d = time/0.4f;
            else                d = 1.0f-(time-0.4f)/0.1f;
#endif
#if 1
            d = 0.5f+sinf(m_aTime*6.0f)*0.5f;
#endif
            d *= (p2.x-p1.x)*0.1f;
            p1.x += d;
            p1.y += d;
            p2.x -= d;
            p2.y -= d;

            hPen = CreatePen(PS_SOLID, 1, RGB(255,255,0));  // yellow
            old = SelectObject(hDC, hPen);

            rect.left   = (int)(p1.x*m_ddsdRenderTarget.dwWidth);
            rect.right  = (int)(p2.x*m_ddsdRenderTarget.dwWidth);
            rect.top    = (int)((1.0f-p2.y)*m_ddsdRenderTarget.dwHeight);
            rect.bottom = (int)((1.0f-p1.y)*m_ddsdRenderTarget.dwHeight);

            list[0].x = rect.left;
            list[0].y = rect.top+(rect.bottom-rect.top)/5;
            list[1].x = rect.left;
            list[1].y = rect.top;
            list[2].x = rect.left+(rect.right-rect.left)/5;
            list[2].y = rect.top;
            Polyline(hDC, list, 3);

            list[0].x = rect.right;
            list[0].y = rect.top+(rect.bottom-rect.top)/5;
            list[1].x = rect.right;
            list[1].y = rect.top;
            list[2].x = rect.right+(rect.left-rect.right)/5;
            list[2].y = rect.top;
            Polyline(hDC, list, 3);

            list[0].x = rect.left;
            list[0].y = rect.bottom+(rect.top-rect.bottom)/5;
            list[1].x = rect.left;
            list[1].y = rect.bottom;
            list[2].x = rect.left+(rect.right-rect.left)/5;
            list[2].y = rect.bottom;
            Polyline(hDC, list, 3);

            list[0].x = rect.right;
            list[0].y = rect.bottom+(rect.top-rect.bottom)/5;
            list[1].x = rect.right;
            list[1].y = rect.bottom;
            list[2].x = rect.right+(rect.left-rect.right)/5;
            list[2].y = rect.bottom;
            Polyline(hDC, list, 3);

            if ( old != 0 )  SelectObject(hDC, old);
            DeleteObject(hPen);
        }
    }

    m_pddsRenderTarget->ReleaseDC(hDC);
}

// Shows frame rate and dimensions of the rendering device.

VOID CD3DApplication::ShowStats()
{
    static FLOAT fFPS      = 0.0f;
    static FLOAT fLastTime = 0.0f;
    static DWORD dwFrames  = 0L;

    // Keep track of the time lapse and frame count
    FLOAT fTime = timeGetTime() * 0.001f; // Get current time in seconds
    ++dwFrames;

    // Update the frame rate once per second
    if( fTime - fLastTime > 1.0f )
    {
        fFPS      = dwFrames / (fTime - fLastTime);
        fLastTime = fTime;
        dwFrames  = 0L;
    }

    int t = m_pD3DEngine->RetStatisticTriangle();

    // Setup the text buffer to write out dimensions
    TCHAR buffer[100];
    sprintf( buffer, _T("%7.02f fps T=%d (%dx%dx%d)"), fFPS, t,
             m_ddsdRenderTarget.dwWidth, m_ddsdRenderTarget.dwHeight,
             m_ddsdRenderTarget.ddpfPixelFormat.dwRGBBitCount );
    OutputText( 400, 2, buffer );

    int x, y, i;
    if ( m_pD3DEngine->GetSpriteCoord(x, y) )
    {
        OutputText( x, y, "+" );
    }

    for ( i=0 ; i<10 ; i++ )
    {
        char* info = m_pD3DEngine->RetInfoText(i);
        x = 50;
        y = m_ddsdRenderTarget.dwHeight-20-i*20;
        OutputText( x, y, info );
    }
}


// Draws text on the window.

VOID CD3DApplication::OutputText( DWORD x, DWORD y, TCHAR* str )
{
    HDC hDC;

    // Get a DC for the surface. Then, write out the buffer
    if( m_pddsRenderTarget )
    {
        if( SUCCEEDED( m_pddsRenderTarget->GetDC(&hDC) ) )
        {
            SetTextColor( hDC, RGB(255,255,0) );
            SetBkMode( hDC, TRANSPARENT );
            ExtTextOut( hDC, x, y, 0, NULL, str, lstrlen(str), NULL );
            m_pddsRenderTarget->ReleaseDC(hDC);
        }
    }
}




// Defines a function that allocates memory for and initializes
// members within a BITMAPINFOHEADER structure

PBITMAPINFO CD3DApplication::CreateBitmapInfoStruct(HBITMAP hBmp)
{
    BITMAP      bmp;
    PBITMAPINFO pbmi;
    WORD        cClrBits;

    // Retrieve the bitmap's color format, width, and height.
    if ( !GetObject(hBmp, sizeof(BITMAP), (LPSTR)&bmp) )
        return 0;

    // Convert the color format to a count of bits.
    cClrBits = (WORD)(bmp.bmPlanes * bmp.bmBitsPixel);

         if ( cClrBits ==  1 )  cClrBits =  1;
    else if ( cClrBits <=  4 )  cClrBits =  4;
    else if ( cClrBits <=  8 )  cClrBits =  8;
    else if ( cClrBits <= 16 )  cClrBits = 16;
    else if ( cClrBits <= 24 )  cClrBits = 24;
    else                        cClrBits = 32;

    // Allocate memory for the BITMAPINFO structure. (This structure
    // contains a BITMAPINFOHEADER structure and an array of RGBQUAD data
    // structures.)
    if ( cClrBits != 24 )
    {
         pbmi = (PBITMAPINFO)LocalAlloc(LPTR,
                    sizeof(BITMAPINFOHEADER) +
                    sizeof(RGBQUAD) * (2^cClrBits));
    }
    // There is no RGBQUAD array for the 24-bit-per-pixel format.
    else
    {
         pbmi = (PBITMAPINFO)LocalAlloc(LPTR,
                    sizeof(BITMAPINFOHEADER));
    }

    // Initialize the fields in the BITMAPINFO structure.
    pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    pbmi->bmiHeader.biWidth = bmp.bmWidth;
    pbmi->bmiHeader.biHeight = bmp.bmHeight;
    pbmi->bmiHeader.biPlanes = bmp.bmPlanes;
    pbmi->bmiHeader.biBitCount = bmp.bmBitsPixel;
    if ( cClrBits < 24 )
        pbmi->bmiHeader.biClrUsed = 2^cClrBits;

    // If the bitmap is not compressed, set the BI_RGB flag.
    pbmi->bmiHeader.biCompression = BI_RGB;

    // Compute the number of bytes in the array of color
    // indices and store the result in biSizeImage.
    pbmi->bmiHeader.biSizeImage = (pbmi->bmiHeader.biWidth + 7) /8
                                  * pbmi->bmiHeader.biHeight
                                  * cClrBits;

    // Set biClrImportant to 0, indicating that all of the
    // device colors are important.
    pbmi->bmiHeader.biClrImportant = 0;

    return pbmi;
}

// Defines a function that initializes the remaining structures,
// retrieves the array of palette indices, opens the file, copies
// the data, and closes the file.

BOOL CD3DApplication::CreateBMPFile(LPTSTR pszFile, PBITMAPINFO pbi, HBITMAP hBMP, HDC hDC)
{
    FILE*               file;       // file handle
    BITMAPFILEHEADER    hdr;        // bitmap file-header
    PBITMAPINFOHEADER   pbih;       // bitmap info-header
    LPBYTE              lpBits;     // memory pointer
    DWORD               dwTotal;    // total count of bytes

    pbih = (PBITMAPINFOHEADER)pbi;
    lpBits = (LPBYTE)GlobalAlloc(GMEM_FIXED, pbih->biSizeImage);
    if ( !lpBits )  return FALSE;

    // Retrieve the color table (RGBQUAD array) and the bits
    // (array of palette indices) from the DIB.
    if ( !GetDIBits(hDC, hBMP, 0, (WORD)pbih->biHeight,
                    lpBits, pbi, DIB_RGB_COLORS) )
        return FALSE;

    // Create the .BMP file.
    file = fopen(pszFile, "wb");
    if ( file == NULL )  return FALSE;

    hdr.bfType = 0x4d42; // 0x42 = "B" 0x4d = "M"

    // Compute the size of the entire file.
    hdr.bfSize = (DWORD)(sizeof(BITMAPFILEHEADER) +
                         pbih->biSize + pbih->biClrUsed
                         * sizeof(RGBQUAD) + pbih->biSizeImage);

    hdr.bfReserved1 = 0;
    hdr.bfReserved2 = 0;

    // Compute the offset to the array of color indices.
    hdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) +
                    pbih->biSize + pbih->biClrUsed
                    * sizeof (RGBQUAD);

    // Copy the BITMAPFILEHEADER into the .BMP file.
    fwrite(&hdr, sizeof(BITMAPFILEHEADER), 1, file);

    // Copy the BITMAPINFOHEADER and RGBQUAD array into the file.
    fwrite(pbih, sizeof(BITMAPINFOHEADER)+pbih->biClrUsed*sizeof(RGBQUAD), 1, file);

    // Copy the array of color indices into the .BMP file.
    dwTotal = pbih->biSizeImage;
    fwrite(lpBits, dwTotal, 1, file);

    // Close the .BMP file.
    fclose(file);

    // Free memory.
    GlobalFree((HGLOBAL)lpBits);
    return TRUE;
}

// Write a file. BMP screenshot.

BOOL CD3DApplication::WriteScreenShot(char *filename, int width, int height)
{
    D3DVIEWPORT7    vp;
    HDC             hDC;
    HDC             hDCImage;
    HBITMAP         hb;
    PBITMAPINFO     info;
    int             dx, dy;

    m_pD3DDevice->GetViewport(&vp);
    dx = vp.dwWidth;
    dy = vp.dwHeight;

    if ( FAILED(m_pddsRenderTarget->GetDC(&hDC)) )  return FALSE;

    hDCImage = CreateCompatibleDC(hDC);
    if ( hDCImage == 0 )
    {
        m_pddsRenderTarget->ReleaseDC(hDC);
        return FALSE;
    }

    hb = CreateCompatibleBitmap(hDC, width, height);
    if ( hb == 0 )
    {
        DeleteDC(hDCImage);
        m_pddsRenderTarget->ReleaseDC(hDC);
        return FALSE;
    }

    SelectObject(hDCImage, hb);
    StretchBlt(hDCImage, 0, 0, width, height, hDC, 0, 0, dx, dy, SRCCOPY);

    info = CreateBitmapInfoStruct(hb);
    if ( info == 0 )
    {
        DeleteObject(hb);
        DeleteDC(hDCImage);
        m_pddsRenderTarget->ReleaseDC(hDC);
        return FALSE;
    }

    CreateBMPFile(filename, info, hb, hDCImage);

    DeleteObject(hb);
    DeleteDC(hDCImage);
    m_pddsRenderTarget->ReleaseDC(hDC);
    return TRUE;
}


// Initializes an hDC on the rendering surface.

BOOL CD3DApplication::GetRenderDC(HDC &hDC)
{
    if ( FAILED(m_pddsRenderTarget->GetDC(&hDC)) )  return FALSE;
    return TRUE;
}

// Frees the hDC of the rendering surface.

BOOL CD3DApplication::ReleaseRenderDC(HDC &hDC)
{
    m_pddsRenderTarget->ReleaseDC(hDC);
    return TRUE;
}




// Perform the list of all graphics devices available.
// For the device selected, lists the full screen modes
// possible.
// buf* --> nom1<0> nom2<0> <0>

BOOL CD3DApplication::EnumDevices(char *bufDevices,  int lenDevices,
                                  char *bufModes,    int lenModes,
                                  int &totalDevices, int &selectDevices,
                                  int &totalModes,   int &selectModes)
{
    D3DEnum_DeviceInfo* pDeviceList;
    D3DEnum_DeviceInfo* pDevice;
    DDSURFACEDESC2*     pddsdMode;
    DWORD               numDevices, device, mode;
    int                 len;
    char                text[100];

    D3DEnum_GetDevices(&pDeviceList, &numDevices);

    selectDevices = -1;
    selectModes = -1;
    totalModes = 0;
    for( device=0 ; device<numDevices ; device++ )
    {
        pDevice = &pDeviceList[device];

        len = strlen(pDevice->strDesc)+1;
        if ( len >= lenDevices )  break;  // bufDevices full!
        strcpy(bufDevices, pDevice->strDesc);
        bufDevices += len;
        lenDevices -= len;

        if ( pDevice == m_pDeviceInfo )  // select device ?
        {
            selectDevices = device;

            for( mode=0 ; mode<pDevice->dwNumModes ; mode++ )
            {
                pddsdMode = &pDevice->pddsdModes[mode];

                sprintf(text, "%ld x %ld x %ld",
                                pddsdMode->dwWidth,
                                pddsdMode->dwHeight,
                                pddsdMode->ddpfPixelFormat.dwRGBBitCount);

                len = strlen(text)+1;
                if ( len >= lenModes )  break;  // bufModes full !
                strcpy(bufModes, text);
                bufModes += len;
                lenModes -= len;

                if ( mode == m_pDeviceInfo->dwCurrentMode )  // select mode ?
                {
                    selectModes = mode;
                }
            }
            bufModes[0] = 0;
            totalModes = pDevice->dwNumModes;
        }
    }
    bufDevices[0] = 0;
    totalDevices = numDevices;

    return TRUE;
}

// Indicates whether it is in full screen mode.

BOOL CD3DApplication::RetFullScreen()
{
    return !m_pDeviceInfo->bWindowed;
}

// Change the graphics mode.

BOOL CD3DApplication::ChangeDevice(char *deviceName, char *modeName,
                                   BOOL bFull)
{
    D3DEnum_DeviceInfo* pDeviceList;
    D3DEnum_DeviceInfo* pDevice;
    DDSURFACEDESC2*     pddsdMode;
    DWORD               numDevices, device, mode;
    HRESULT             hr;
    char                text[100];

    D3DEnum_GetDevices(&pDeviceList, &numDevices);

    for( device=0 ; device<numDevices ; device++ )
    {
        pDevice = &pDeviceList[device];

        if ( strcmp(pDevice->strDesc, deviceName) == 0 )  // device found ?
        {
            for( mode=0 ; mode<pDevice->dwNumModes ; mode++ )
            {
                pddsdMode = &pDevice->pddsdModes[mode];

                sprintf(text, "%ld x %ld x %ld",
                                pddsdMode->dwWidth,
                                pddsdMode->dwHeight,
                                pddsdMode->ddpfPixelFormat.dwRGBBitCount);

                if ( strcmp(text, modeName) == 0 )  // mode found ?
                {
                    m_pDeviceInfo               = pDevice;
                    pDevice->bWindowed          = !bFull;
                    pDevice->dwCurrentMode      = mode;
                    pDevice->ddsdFullscreenMode = pDevice->pddsdModes[mode];

                    m_bReady = FALSE;

                    if ( FAILED( hr = Change3DEnvironment() ) )
                    {
                        return FALSE;
                    }

                    SetLocalProfileString("Device", "Name", deviceName);
                    SetLocalProfileString("Device", "Mode", modeName);
                    SetLocalProfileInt("Device", "FullScreen", bFull);
                    m_bReady = TRUE;
                    return TRUE;
                }
            }
        }
    }

    return FALSE;
}



// Displays error messages in a message box.

VOID CD3DApplication::DisplayFrameworkError( HRESULT hr, DWORD dwType )
{
    TCHAR strMsg[512];

    switch( hr )
    {
        case D3DENUMERR_ENGINE:
            lstrcpy( strMsg, _T("Could not create 3D Engine application!") );
            break;
        case D3DENUMERR_ROBOT:
            lstrcpy( strMsg, _T("Could not create Robot application!") );
            break;
        case D3DENUMERR_NODIRECTDRAW:
            lstrcpy( strMsg, _T("Could not create DirectDraw!") );
            break;
        case D3DENUMERR_NOCOMPATIBLEDEVICES:
            lstrcpy( strMsg, _T("Could not find any compatible Direct3D\n"
                     "devices.") );
            break;
        case D3DENUMERR_SUGGESTREFRAST:
            lstrcpy( strMsg, _T("Could not find any compatible devices.\n\n"
                     "Try enabling the reference rasterizer using\n"
                     "EnableRefRast.reg.") );
            break;
        case D3DENUMERR_ENUMERATIONFAILED:
            lstrcpy( strMsg, _T("Enumeration failed. Your system may be in an\n"
                     "unstable state and need to be rebooted") );
            break;
        case D3DFWERR_INITIALIZATIONFAILED:
            lstrcpy( strMsg, _T("Generic initialization error.\n\nEnable "
                     "debug output for detailed information.") );
            break;
        case D3DFWERR_NODIRECTDRAW:
            lstrcpy( strMsg, _T("No DirectDraw") );
            break;
        case D3DFWERR_NODIRECT3D:
            lstrcpy( strMsg, _T("No Direct3D") );
            break;
        case D3DFWERR_INVALIDMODE:
            lstrcpy( strMsg, _T("COLOBOT requires a 16-bit (or higher) "
                                "display mode\nto run in a window.\n\nPlease "
                                "switch your desktop settings accordingly.") );
            break;
        case D3DFWERR_COULDNTSETCOOPLEVEL:
            lstrcpy( strMsg, _T("Could not set Cooperative Level") );
            break;
        case D3DFWERR_NO3DDEVICE:
            lstrcpy( strMsg, _T("Could not create the Direct3DDevice object.") );

            if( MSGWARN_SWITCHEDTOSOFTWARE == dwType )
                lstrcat( strMsg, _T("\nThe 3D hardware chipset may not support"
                                    "\nrendering in the current display mode.") );
            break;
        case D3DFWERR_NOZBUFFER:
            lstrcpy( strMsg, _T("No ZBuffer") );
            break;
        case D3DFWERR_INVALIDZBUFFERDEPTH:
            lstrcpy( strMsg, _T("Invalid Z-buffer depth. Try switching modes\n"
                     "from 16- to 32-bit (or vice versa)") );
            break;
        case D3DFWERR_NOVIEWPORT:
            lstrcpy( strMsg, _T("No Viewport") );
            break;
        case D3DFWERR_NOPRIMARY:
            lstrcpy( strMsg, _T("No primary") );
            break;
        case D3DFWERR_NOCLIPPER:
            lstrcpy( strMsg, _T("No Clipper") );
            break;
        case D3DFWERR_BADDISPLAYMODE:
            lstrcpy( strMsg, _T("Bad display mode") );
            break;
        case D3DFWERR_NOBACKBUFFER:
            lstrcpy( strMsg, _T("No backbuffer") );
            break;
        case D3DFWERR_NONZEROREFCOUNT:
            lstrcpy( strMsg, _T("A DDraw object has a non-zero reference\n"
                     "count (meaning it was not properly cleaned up)." ) );
            break;
        case D3DFWERR_NORENDERTARGET:
            lstrcpy( strMsg, _T("No render target") );
            break;
        case E_OUTOFMEMORY:
            lstrcpy( strMsg, _T("Not enough memory!") );
            break;
        case DDERR_OUTOFVIDEOMEMORY:
            lstrcpy( strMsg, _T("There was insufficient video memory "
                     "to use the\nhardware device.") );
            break;
        default:
            lstrcpy( strMsg, _T("Generic application error.\n\nEnable "
                     "debug output for detailed information.") );
    }

    if( MSGERR_APPMUSTEXIT == dwType )
    {
        lstrcat( strMsg, _T("\n\nCOLOBOT will now exit.") );
        MessageBox( NULL, strMsg, m_strWindowTitle, MB_ICONERROR|MB_OK );
    }
    else
    {
        if( MSGWARN_SWITCHEDTOSOFTWARE == dwType )
            lstrcat( strMsg, _T("\n\nSwitching to software rasterizer.") );
        MessageBox( NULL, strMsg, m_strWindowTitle, MB_ICONWARNING|MB_OK );
    }
}


