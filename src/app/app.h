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

// app.h

#pragma once


#include "common/misc.h"
#include "graphics/common/device.h"
#include "graphics/common/engine.h"

#include <string>
#include <vector>


class CInstanceManager;
class CEvent;
class CRobotMain;
class CSound;

struct ApplicationPrivate;


/**
 * \class CApplication
 * \brief Main application
 *
 * This class is responsible for main application execution, including creating
 * and handling main application window, receiving events, etc.
 *
 * It is a singleton class with only one instance that can be created.
 *
 * Creation of other main objects
 *
 * The class creates the only instance of CInstanceManager, CEventQueue, CEngine,
 * CRobotMain and CSound classes.
 *
 * Window management
 *
 * The class is responsible for creating app window, setting and changing the video mode,
 * setting the position of mouse and changing the cursor, grabbing and writing screenshots.
 *
 * Events
 *
 * Events are taken from SDL event queue and either handled by CApplication or translated
 * to common events from src/common.h and pushed to global event queue CEventQueue.
 * Joystick events are generated somewhat differently, by running a separate timer,
 * polling the device for changes and synthesising events on change. It avoids flooding
 * the event queue with too many joystick events and the granularity of the timer can be
 * adjusted.
 *
 * The events are further handled in CRobotMain class.
 *
 */
class CApplication
{
public:
    //! Constructor (can only be called once!)
    CApplication();
    //! Destructor
    ~CApplication();

    //! Returns the only CApplication instance
    static CApplication* GetInstance()
      { return m_appInstance; }

public:
    //! Parses commandline arguments
    Error       ParseArguments(int argc, char *argv[]);
    //! Initializes the application
    bool        Create();
    //! Main event loop
    int         Run();
    //! Returns the code to be returned at main() exit
    int         GetExitCode();

    //! Cleans up before exit
    void        Destroy();

    //! Enters the pause mode
    void        Pause(bool pause);

    //! Updates the simulation state
    void        StepSimulation(float rTime);

    //! Polls the state of joystick axes and buttons
    void        UpdateJoystick();

    void        SetShowStat(bool show);
    bool        GetShowStat();

    void        SetDebugMode(bool mode);
    bool        GetDebugMode();

    bool        GetSetupMode();

    void        SetJoystickEnabled(bool enable);
    bool        GetJoystickEnabled();

    void        FlushPressKey();
    void        ResetKey();
    void        SetKey(int keyRank, int option, int key);
    int         GetKey(int keyRank, int option);

    void        SetMouseType(Gfx::MouseType type);
    void        SetMousePos(Math::Point pos);

    //? void        SetNiceMouse(bool nice);
    //? bool        GetNiceMouse();
    //? bool        GetNiceMouseCap();

    bool        WriteScreenShot(char *filename, int width, int height);

protected:
    //! Processes an SDL event to Event struct
    void        ParseEvent();
    //! Handles some incoming events
    void        ProcessEvent(Event event);
    //! Renders the image in window
    bool        Render();

    //! Opens the joystick device
    bool OpenJoystick();
    //! Closes the joystick device
    void CloseJoystick();

    //! Converts window coords to interface coords
    Math::Point WindowToInterfaceCoords(int x, int y);

    //HRESULT       ConfirmDevice( DDCAPS* pddDriverCaps, D3DDEVICEDESC7* pd3dDeviceDesc );
    //HRESULT       Initialize3DEnvironment();
    //HRESULT       Change3DEnvironment();
    //HRESULT       CreateZBuffer(GUID* pDeviceGUID);
    //HRESULT       Render3DEnvironment();
    //VOID      Cleanup3DEnvironment();
    //VOID      DeleteDeviceObjects();
    //VOID      DisplayFrameworkError( HRESULT, DWORD );

    void        InitText();
    void        DrawSuppl();
    void        ShowStats();
    void        OutputText(long x, long y, char* str);

protected:
    //! The only instance of CApplication
    static CApplication*    m_appInstance;
    //! Instance manager
    CInstanceManager*       m_iMan;
    //! Private (SDL-dependent data)
    ApplicationPrivate*     m_private;
    //! Global event queue
    CEventQueue*            m_eventQueue;
    //! Graphics engine
    Gfx::CEngine*           m_engine;
    //! Graphics device
    Gfx::CDevice*           m_device;
    //! Sound subsystem
    CSound*                 m_sound;
    //! Main class of the proper game engine
    CRobotMain*             m_robotMain;

    //! Code to return at exit
    int             m_exitCode;

    bool            m_active;
    bool            m_activateApp;
    bool            m_ready;

    bool            m_showStats;
    bool            m_debugMode;
    bool            m_setupMode;

    bool            m_joystickEnabled;

    std::string     m_windowTitle;

    //? long            m_vidMemTotal;
    //? bool            m_appUseZBuffer;
    //? bool            m_appUseStereo;
    //? bool            m_audioState;
    //? bool            m_audioTrack;
    //? bool            m_niceMouse;

    int             m_keyState;
    Math::Vector    m_axeKey;
    Math::Vector    m_axeJoy;
    Math::Point     m_mousePos;
    long            m_mouseWheel;

    //! Current state of joystick axes; may be updated from another thread
    std::vector<int> m_joyAxeState;
    //! Current state of joystick buttons; may be updated from another thread
    std::vector<bool> m_joyButtonState;

    float           m_time;
    long            m_key[50][2];
};

