// joystick.cpp

#define STRICT

#include <windows.h>
#include <dinput.h>
#include <stdio.h>
#include <math.h>

#include "joystick.h"




// Global variables.

LPDIRECTINPUT7			g_pDI		= NULL; 		
LPDIRECTINPUTDEVICE7	g_pJoystick = NULL; 	
DIDEVCAPS				g_diDevCaps;
LPDIRECTINPUTEFFECT		g_pEffect   = NULL;





// Called once for each enumerated joystick. If we find one, create a
// device interface on it so we can play with it.

BOOL CALLBACK EnumJoysticksCallback( const DIDEVICEINSTANCE* pdidInstance,
									 VOID* pContext )
{
	HRESULT hr;

	// Obtain an interface to the enumerated joystick.
	hr = g_pDI->CreateDeviceEx( pdidInstance->guidInstance, IID_IDirectInputDevice7,
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
	diprg.diph.dwSize		= sizeof(DIPROPRANGE); 
	diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER); 
	diprg.diph.dwHow		= DIPH_BYOFFSET; 
	diprg.diph.dwObj		= pdidoi->dwOfs; // Specify the enumerated axis
	diprg.lMin				= -1000; 
	diprg.lMax				= +1000; 
	
	// Set the range for the axis
	if( FAILED( g_pJoystick->SetProperty( DIPROP_RANGE, &diprg.diph ) ) )
		return DIENUM_STOP;

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

	return DIENUM_CONTINUE;
}


// Initialize the DirectInput variables.

BOOL InitDirectInput(HINSTANCE hInst, HWND hWnd, BOOL &bFFB)
{
	HRESULT		hr;
	DWORD		flags;

	// Register with the DirectInput subsystem and get a pointer
	// to a IDirectInput interface we can use.
	hr = DirectInputCreateEx( hInst, DIRECTINPUT_VERSION,IID_IDirectInput7, (LPVOID*)&g_pDI, NULL );
	if( FAILED(hr) )  return FALSE;;

	// Look for a simple joystick we can use for this sample program.
	g_pJoystick = NULL;
	flags = DIEDFL_ATTACHEDONLY;
	if ( bFFB )  flags |= DIEDFL_FORCEFEEDBACK;
	hr = g_pDI->EnumDevices( DIDEVTYPE_JOYSTICK, EnumJoysticksCallback,
							 NULL, flags );
	if( FAILED(hr) )  return FALSE;

	// Make sure we got a joystick
	if( g_pJoystick == NULL )
	{
		if ( bFFB )
		{
			flags = DIEDFL_ATTACHEDONLY;  // try without ffb
			hr = g_pDI->EnumDevices( DIDEVTYPE_JOYSTICK, EnumJoysticksCallback,
									 NULL, flags );
			if( FAILED(hr) )  return FALSE;
			if( g_pJoystick == NULL )  return FALSE;
			bFFB = FALSE;  // ok without ffb
		}
		else
		{
			return FALSE;
		}
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

	if ( bFFB )  // force feedback ?
	{
		// Since we will be playing force feedback effects, we should disable the
		// auto-centering spring.
		DIPROPDWORD dipdw;
		dipdw.diph.dwSize       = sizeof(DIPROPDWORD);
		dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
		dipdw.diph.dwObj        = 0;
		dipdw.diph.dwHow        = DIPH_DEVICE;
		dipdw.dwData            = FALSE;

		hr = g_pJoystick->SetProperty( DIPROP_AUTOCENTER, &dipdw.diph );
		if( FAILED(hr) )  return FALSE;

		// This application needs only one effect: Applying raw forces.
		DWORD    rgdwAxes[2] = { DIJOFS_X, DIJOFS_Y };
		LONG     rglDirection[2] = { 0, 0 };
		DICONSTANTFORCE cf = { 0 };

		DIEFFECT eff;
		ZeroMemory( &eff, sizeof(eff) );
		eff.dwSize                  = sizeof(DIEFFECT);
		eff.dwFlags                 = DIEFF_CARTESIAN | DIEFF_OBJECTOFFSETS;
		eff.dwDuration              = INFINITE;
		eff.dwSamplePeriod          = 0;
		eff.dwGain                  = DI_FFNOMINALMAX;
		eff.dwTriggerButton         = DIEB_NOTRIGGER;
		eff.dwTriggerRepeatInterval = 0;
		eff.cAxes                   = 2;
		eff.rgdwAxes                = rgdwAxes;
		eff.rglDirection            = rglDirection;
		eff.lpEnvelope              = 0;
		eff.cbTypeSpecificParams    = sizeof(DICONSTANTFORCE);
		eff.lpvTypeSpecificParams   = &cf;
		eff.dwStartDelay            = 0;

		// Create the prepared effect
		hr = g_pJoystick->CreateEffect( GUID_ConstantForce, &eff, &g_pEffect, NULL );
		if( FAILED(hr) )  return FALSE;
	}

	return TRUE;
}

// Acquire or unacquire the keyboard, depending on if the app is active
// Input device must be acquired before the GetDeviceState is called.

BOOL SetAcquire(BOOL bActive)
{
	if ( g_pJoystick )
	{
		if( bActive )
		{
			g_pJoystick->Acquire();

			if( g_pEffect != NULL ) 
			{
				g_pEffect->Start( 1, 0 );  // start the effect
			}
		}
		else
		{
			g_pJoystick->Unacquire();
		}
	}
	return TRUE;
}


// Get the input device's state and display it.

BOOL UpdateInputState( DIJOYSTATE &js )
{
	HRESULT 	hr;

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
	// Release any DirectInputEffect objects.
	if( g_pEffect != NULL )
	{
		g_pEffect->Release();
		g_pEffect = NULL;
	}

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



// 

BOOL  EffectFound = FALSE;  // global flag
 
BOOL CALLBACK DIEnumEffectsProc(LPCDIEFFECTINFO pei, LPVOID pv)
{
	*((GUID *)pv) = pei->guid;
	EffectFound = TRUE;
	return DIENUM_STOP;  // one is enough
}

// Cherche les effets disponibles.

BOOL EnumEffects()
{
	HRESULT  hr;
	GUID	g_guidEffect;
 
	hr = g_pJoystick->EnumEffects( 
							(LPDIENUMEFFECTSCALLBACK) DIEnumEffectsProc,
							&g_guidEffect, 
							DIEFT_PERIODIC);
	if (FAILED(hr))
	{
		OutputDebugString("Effect enumeration failed\n");
		return FALSE;
	}

	// Since we will be playing force feedback effects, we should disable the
	// auto-centering spring.
	DIPROPDWORD dipdw;
	dipdw.diph.dwSize       = sizeof(DIPROPDWORD);
	dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	dipdw.diph.dwObj        = 0;
	dipdw.diph.dwHow        = DIPH_DEVICE;
	dipdw.dwData            = FALSE;

	hr = g_pJoystick->SetProperty( DIPROP_AUTOCENTER, &dipdw.diph );
	if( FAILED(hr) ) 
	{
		OutputDebugString("SetProperty failed\n");
		return FALSE;
	}

	return TRUE;
}

// Spécifie les forces à appliquer sur le joystick.
// Pour un volant, seule le force X est utilisée.
// forces: -1.0 .. +1.0

BOOL SetJoyForces(float forceX, float forceY)
{
	HRESULT  hr;

	if ( g_pEffect == NULL )  return FALSE;

	LONG rglDirection[2] = { (int)(10000.0f*forceX), (int)(10000.0f*forceY) };

	DICONSTANTFORCE cf;
	cf.lMagnitude = (DWORD)(10000.0f*sqrtf(forceX*forceX+forceY*forceY));

	DIEFFECT eff;
	ZeroMemory( &eff, sizeof(eff) );
	eff.dwSize				  = sizeof(DIEFFECT);
	eff.dwFlags 			  = DIEFF_CARTESIAN | DIEFF_OBJECTOFFSETS;
	eff.cAxes				  = 2;
	eff.rglDirection		  = rglDirection;
	eff.lpEnvelope			  = 0;
	eff.cbTypeSpecificParams  = sizeof(DICONSTANTFORCE);
	eff.lpvTypeSpecificParams = &cf;
	eff.dwStartDelay		  = 0;

	// Now set the new parameters and start the effect immediately.
	hr = g_pEffect->SetParameters( &eff, DIEP_DIRECTION|
								   DIEP_TYPESPECIFICPARAMS|DIEP_START );
	if( FAILED(hr) )  return FALSE;
	return TRUE;
}

