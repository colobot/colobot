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

/**
 * \file app/app.h
 * \brief CApplication class
 */

#pragma once


#include "common/global.h"
#include "common/singleton.h"
#include "common/profile.h"

#include "graphics/core/device.h"
#include "graphics/engine/engine.h"
#include "graphics/opengl/gldevice.h"


#include <string>
#include <vector>


class CInstanceManager;
class CEventQueue;
class CRobotMain;
class CSoundInterface;

/**
 * \struct JoystickDevice
 * \brief Information about a joystick device
 */
struct JoystickDevice
{
    //! Device index (-1 = invalid device)
    int index;
    //! Device name
    std::string name;
    //! Number of axes (only available after joystick opened)
    int axisCount;
    //! Number of buttons (only available after joystick opened)
    int buttonCount;

    JoystickDevice()
        : index(-1), axisCount(0), buttonCount(0) {}
};

/**
 * \enum VideoQueryResult
 * \brief Result of querying for available video resolutions
 */
enum VideoQueryResult
{
    VIDEO_QUERY_ERROR,
    VIDEO_QUERY_NONE,
    VIDEO_QUERY_ALL,
    VIDEO_QUERY_OK
};


/**
 * \enum TrackedKey
 * \brief Additional keys whose state (pressed/released) is tracked by CApplication
 */
enum TrackedKey
{
    TRKEY_NUM_UP    = (1<<0),
    TRKEY_NUM_DOWN  = (1<<1),
    TRKEY_NUM_LEFT  = (1<<2),
    TRKEY_NUM_RIGHT = (1<<3),
    TRKEY_NUM_PLUS  = (1<<4),
    TRKEY_NUM_MINUS = (1<<5),
    TRKEY_PAGE_UP   = (1<<6),
    TRKEY_PAGE_DOWN = (1<<7)
};

/**
 * \enum ParseArgsStatus
 * \brief State of parsing commandline arguments
 */
enum ParseArgsStatus
{
    PARSE_ARGS_OK   = 1, //! < all ok
    PARSE_ARGS_FAIL = 2, //! < invalid syntax
    PARSE_ARGS_HELP = 3  //! < -help requested
};

/**
 * \enum MouseMode
 * \brief Mode of mouse cursor
 */
enum MouseMode
{
    MOUSE_SYSTEM, //! < system cursor visible; in-game cursor hidden
    MOUSE_ENGINE, //! < in-game cursor visible; system cursor hidden
    MOUSE_BOTH,   //! < both cursors visible (only for debug)
    MOUSE_NONE,   //! < no cursor visible
};

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
 * \section Creation Creation of other main objects
 *
 * The class creates the only instance of CInstanceManager, CEventQueue, CEngine,
 * CRobotMain and CSoundInterface classes.
 *
 * \section Window Window management
 *
 * The class is responsible for creating app window, setting and changing the video mode,
 * joystick management, grabbing input and changing the system mouse cursor
 * position and visibility.
 * ("System mouse cursor" means the cursor displayed by the OS in constrast to the cursor
 * displayed by CEngine).
 *
 * \section Events Events
 *
 * Events are taken from SDL event queue, translated to common events from src/common.h
 * and pushed to global event queue CEventQueue.
 *
 * Joystick events are generated somewhat differently, by running a separate timer,
 * polling the device for changes and synthesising events on change. It avoids flooding
 * the event queue with too many joystick events and the granularity of the timer can be
 * adjusted.
 *
 * The events are passed to ProcessEvent() of classes in this order: CApplication, CEngine
 * and CRobotMain. CApplication and CEngine's ProcessEvent() functions return bool, which
 * means whether to pass the event on, or stop the chain. This is to enable handling some
 * events which are internal to CApplication or CEngine.
 *
 * \section Portability Portability
 *
 * Currently, the class only handles OpenGL devices. SDL can be used with DirectX, but
 * for that to work, video initialization and video setting must be done differently.
 *
 */
class CApplication : public CSingleton<CApplication>
{
public:
    //! Constructor (can only be called once!)
    CApplication();
    //! Destructor
    ~CApplication();

public:
    //! Parses commandline arguments
    ParseArgsStatus ParseArguments(int argc, char *argv[]);
    //! Initializes the application
    bool        Create();
    //! Main event loop
    int         Run();
    //! Returns the code to be returned at main() exit
    int         GetExitCode();

    //! Returns the message of error (set to something if exit code is not 0)
    const std::string& GetErrorMessage();

    //! Cleans up before exit
    void        Destroy();

    //! Returns a list of possible video modes
    VideoQueryResult GetVideoResolutionList(std::vector<Math::IntPoint> &resolutions,
                                            bool fullScreen, bool resizeable);

    //! Returns the current video mode
    Gfx::GLDeviceConfig GetVideoConfig();

    //! Change the video mode to given mode
    bool        ChangeVideoConfig(const Gfx::GLDeviceConfig &newConfig);

    //! Suspends animation (time will not be updated)
    void        SuspendSimulation();
    //! Resumes animation
    void        ResumeSimulation();
    //! Returns whether simulation is suspended
    bool        GetSimulationSuspended();

    //! Updates the simulation state
    void        StepSimulation();

    //@{
    //! Management of simulation speed
    void            SetSimulationSpeed(float speed);
    float           GetSimulationSpeed();
    //@}

    //! Returns the absolute time counter [seconds]
    float       GetAbsTime();
    //! Returns the exact absolute time counter [nanoseconds]
    long long   GetExactAbsTime();

    //! Returns the exact absolute time counter disregarding speed setting [nanoseconds]
    long long   GetRealAbsTime();

    //! Returns the relative time since last update [seconds]
    float       GetRelTime();
    //! Returns the exact realative time since last update [nanoseconds]
    long long   GetExactRelTime();

    //! Returns the exact relative time since last update disregarding speed setting [nanoseconds]
    long long   GetRealRelTime();

    //! Returns a list of available joystick devices
    std::vector<JoystickDevice> GetJoystickList();

    //! Returns info about the current joystick
    JoystickDevice GetJoystick();

    //! Change the current joystick device
    bool        ChangeJoystick(const JoystickDevice &newJoystick);

    //! Management of joystick enable state
    //@{
    void        SetJoystickEnabled(bool enable);
    bool        GetJoystickEnabled();
    //@}

    //! Polls the state of joystick axes and buttons
    void        UpdateJoystick();

    //! Updates the mouse position explicitly
    void        UpdateMouse();

    //! Returns the current key modifiers
    int         GetKmods();
    //! Returns whether the given kmod is active
    bool        GetKmodState(int kmod);

    //! Returns whether the tracked key is pressed
    bool        GetTrackedKeyState(TrackedKey key);

    //! Returns whether the mouse button is pressed
    bool        GetMouseButtonState(int index);

    //! Resets tracked key states and modifiers
    void        ResetKeyStates();

    //! Management of the grab mode for input (keyboard & mouse)
    //@{
    void        SetGrabInput(bool grab);
    bool        GetGrabInput();
    //@}

    //! Management of mouse mode
    //@{
    void        SetMouseMode(MouseMode mode);
    MouseMode   GetMouseMode();
    //@}

    //! Returns the position of mouse cursor (in interface coords)
    Math::Point GetMousePos();

    //! Moves (warps) the mouse cursor to the specified position (in interface coords)
    void        MoveMouse(Math::Point pos);

    //! Management of debug mode (prints more info in logger)
    //@{
    void        SetDebugMode(bool mode);
    bool        GetDebugMode();
    //@}

    //! Returns the full path to data directory
    std::string GetDataDirPath();

    //! Returns the full path to a standard dir in data directory
    std::string GetDataSubdirPath(DataDir stdDir);

    //! Returns the full path to a file in data directory given standard dir and subpath
    std::string GetDataFilePath(DataDir stdDir, const std::string &subpath);

    //! Management of language
    //@{
    Language    GetLanguage();
    char        GetLanguageChar();
    void        SetLanguage(Language language);
    //@}

    //! Management of sleep in main loop (lowers CPU usage)
    //@{
    void        SetLowCPU(bool low);
    bool        GetLowCPU();
    //@}

protected:
    //! Creates the window's SDL_Surface
    bool CreateVideoSurface();

    //! Processes the captured SDL event to Event struct
    Event       ProcessSystemEvent();
    //! If applicable, creates a virtual event to match the changed state as of new event
    Event       CreateVirtualEvent(const Event& sourceEvent);
    //! Handles some incoming events
    bool        ProcessEvent(const Event& event);
    //! Renders the image in window
    void        Render();

    //! Opens the joystick device
    bool OpenJoystick();
    //! Closes the joystick device
    void CloseJoystick();

protected:
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
    CSoundInterface*        m_sound;
    //! Main class of the proper game engine
    CRobotMain*             m_robotMain;
    CProfile*               m_profile;

    //! Code to return at exit
    int             m_exitCode;
    //! Whether application window is active
    bool            m_active;
    //! Whether debug mode is enabled
    bool            m_debugMode;

    //! Message to be displayed as error to the user
    std::string     m_errorMessage;

    //! Current configuration of OpenGL display device
    Gfx::GLDeviceConfig m_deviceConfig;
    //! Previous configuration of OpenGL display device
    Gfx::GLDeviceConfig m_lastDeviceConfig;

    //! Text set as window title
    std::string     m_windowTitle;

    //! Animation time stamps, etc.
    //@{
    SystemTimeStamp* m_baseTimeStamp;
    SystemTimeStamp* m_lastTimeStamp;
    SystemTimeStamp* m_curTimeStamp;

    long long       m_realAbsTimeBase;
    long long       m_realAbsTime;
    long long       m_realRelTime;

    long long       m_absTimeBase;
    long long       m_exactAbsTime;
    long long       m_exactRelTime;

    float           m_absTime;
    float           m_relTime;

    float           m_simulationSpeed;
    bool            m_simulationSuspended;
    //@}

    //! Current state of key modifiers (bitmask of SDLMod)
    unsigned int    m_kmodState;
    //! Current state of some tracked keys (bitmask of TrackedKey enum values)
    unsigned int    m_trackedKeys;

    //! Current mode of mouse
    MouseMode       m_mouseMode;
    //! Current position of mouse cursor
    Math::Point     m_mousePos;
    //! Current state of mouse buttons (bitmask of MouseButton enum values)
    unsigned int    m_mouseButtonsState;

    //! Info about current joystick device
    JoystickDevice  m_joystick;
    //! Whether joystick is enabled
    bool            m_joystickEnabled;
    //! Current state of joystick axes; may be updated from another thread
    std::vector<int> m_joyAxeState;
    //! Current state of joystick buttons; may be updated from another thread
    std::vector<bool> m_joyButtonState;

    //! Path to directory with data files
    std::string     m_dataPath;

    const char*     m_dataDirs[DIR_MAX];

    //! Application language
    Language        m_language;

    //! Low cpu mode
    bool            m_lowCPU;
};

