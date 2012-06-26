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

// joystick.cpp

#define STRICT
#define DIRECTINPUT_VERSION 0x0700

#include <windows.h>
#include <dinput.h>
#include <stdio.h>

#include "joystick.h"




// Global variables.

LPDIRECTINPUT7          g_pDI       = NULL;
LPDIRECTINPUTDEVICE2    g_pJoystick = NULL;
DIDEVCAPS               g_diDevCaps;





// Called once for each enumerated joystick. If we find one, create a
// device interface on it so we can play with it.

BOOL CALLBACK EnumJoysticksCallback( const DIDEVICEINSTANCE* pdidInstance,
                                     VOID* pContext )
{
    HRESULT hr;

    // Obtain an interface to the enumerated joystick.
    hr = g_pDI->CreateDeviceEx( pdidInstance->guidInstance, IID_IDirectInputDevice2,
                                (VOID**)&g_pJoystick, NULL );

    // If it failed, then we can't use this joystick. (Maybe the user unplugged
    // it while we were in the middle of enumerating it.)
    if( FAILED(hr) )
        return DIENUM_CONTINUE;


    // Stop enumeration. Note: we're just taking the first joystick we get. You
    // could store all the enumerated joysticks and let the user pick.
    return DIENUM_STOP;
}


// Callback function for enumerating the axes on a joystick.

BOOL CALLBACK EnumAxesCallback( const DIDEVICEOBJECTINSTANCE* pdidoi,
                                VOID* pContext )
{
    DIPROPRANGE diprg;
    diprg.diph.dwSize       = sizeof(DIPROPRANGE);
    diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER);
    diprg.diph.dwHow        = DIPH_BYOFFSET;
    diprg.diph.dwObj        = pdidoi->dwOfs; // Specify the enumerated axis
    diprg.lMin              = -1000;
    diprg.lMax              = +1000;

    // Set the range for the axis
    if( FAILED( g_pJoystick->SetProperty( DIPROP_RANGE, &diprg.diph ) ) )
        return DIENUM_STOP;

#ifndef __MINGW32__ // FIXME Doesn't work under MinGW
    // Set the UI to reflect what axes the joystick supports
    switch( pdidoi->dwOfs )
    {
        case DIJOFS_X:
            OutputDebugString("EnumAxesCallback -x\n");
            break;
        case DIJOFS_Y:
            OutputDebugString("EnumAxesCallback -y\n");
            break;
        case DIJOFS_Z:
            OutputDebugString("EnumAxesCallback -z\n");
            break;
        case DIJOFS_RX:
            OutputDebugString("EnumAxesCallback -rx\n");
            break;
        case DIJOFS_RY:
            OutputDebugString("EnumAxesCallback -ry\n");
            break;
        case DIJOFS_RZ:
            OutputDebugString("EnumAxesCallback -rz\n");
            break;
        case DIJOFS_SLIDER(0):
            OutputDebugString("EnumAxesCallback -s0\n");
            break;
        case DIJOFS_SLIDER(1):
            OutputDebugString("EnumAxesCallback -s1\n");
            break;
    }
#endif

    return DIENUM_CONTINUE;
}


// Initialize the DirectInput variables.

BOOL InitDirectInput(HINSTANCE hInst, HWND hWnd)
{
    HRESULT hr;

    // Register with the DirectInput subsystem and get a pointer
    // to a IDirectInput interface we can use.
#ifndef __MINGW32__ // FIXME Doesn't work under MinGW
    hr = DirectInputCreateEx( hInst, DIRECTINPUT_VERSION,IID_IDirectInput7, (LPVOID*)&g_pDI, NULL );
    if( FAILED(hr) )  return FALSE;
#else
    return FALSE;
#endif

    // Look for a simple joystick we can use for this sample program.
    hr = g_pDI->EnumDevices( DIDEVTYPE_JOYSTICK, EnumJoysticksCallback,
                             NULL, DIEDFL_ATTACHEDONLY );
    if( FAILED(hr) )  return FALSE;

    // Make sure we got a joystick
    if( NULL == g_pJoystick )
    {
//?        MessageBox( NULL, "Joystick not found", "DInput Sample",
//?                    MB_ICONERROR | MB_OK );
        return FALSE;
    }

    // Set the data format to "simple joystick" - a predefined data format
    //
    // A data format specifies which controls on a device we are interested in,
    // and how they should be reported. This tells DInput that we will be
    // passing a DIJOYSTATE structure to IDirectInputDevice::GetDeviceState().
    hr = g_pJoystick->SetDataFormat( &c_dfDIJoystick );
    if( FAILED(hr) )  return FALSE;

    // Set the cooperative level to let DInput know how this device should
    // interact with the system and with other DInput applications.
    hr = g_pJoystick->SetCooperativeLevel( hWnd, DISCL_EXCLUSIVE|DISCL_FOREGROUND );
    if( FAILED(hr) )  return FALSE;

    // Determine how many axis the joystick has (so we don't error out setting
    // properties for unavailable axis)
    g_diDevCaps.dwSize = sizeof(DIDEVCAPS);
    hr = g_pJoystick->GetCapabilities(&g_diDevCaps);
    if( FAILED(hr) )  return FALSE;


    // Enumerate the axes of the joyctick and set the range of each axis. Note:
    // we could just use the defaults, but we're just trying to show an example
    // of enumerating device objects (axes, buttons, etc.).
    g_pJoystick->EnumObjects( EnumAxesCallback, (VOID*)g_pJoystick, DIDFT_AXIS );

    return TRUE;
}

// Acquire or unacquire the keyboard, depending on if the app is active
// Input device must be acquired before the GetDeviceState is called.

BOOL SetAcquire(BOOL bActive)
{
    if ( g_pJoystick )
    {
        if( bActive )  g_pJoystick->Acquire();
        else           g_pJoystick->Unacquire();
    }
    return TRUE;
}


// Get the input device's state and display it.

BOOL UpdateInputState( DIJOYSTATE &js )
{
    HRESULT     hr;

    if ( g_pJoystick )
    {
        do
        {
            // Poll the device to read the current state
            hr = g_pJoystick->Poll();
            if ( FAILED(hr) )  return FALSE;

            // Get the input's device state
            hr = g_pJoystick->GetDeviceState( sizeof(DIJOYSTATE), &js );

            if( hr == DIERR_INPUTLOST )
            {
                // DInput is telling us that the input stream has been
                // interrupted. We aren't tracking any state between polls, so
                // we don't have any special reset that needs to be done. We
                // just re-acquire and try again.
                hr = g_pJoystick->Acquire();
                if ( FAILED(hr) )  return FALSE;
            }
        }
        while ( DIERR_INPUTLOST == hr );
        if ( FAILED(hr) )  return FALSE;
    }
    return TRUE;
}


// Initialize the DirectInput variables.

BOOL FreeDirectInput()
{
    // Unacquire and release any DirectInputDevice objects.
    if( NULL != g_pJoystick )
    {
        // Unacquire the device one last time just in case
        // the app tried to exit while the device is still acquired.
        g_pJoystick->Unacquire();
        g_pJoystick->Release();
        g_pJoystick = NULL;
    }


    // Release any DirectInput objects.
    if( g_pDI )
    {
        g_pDI->Release();
        g_pDI = NULL;
    }

    return TRUE;
}

