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


#include "common/misc.h"
#include "common/singleton.h"
#include "graphics/core/device.h"
#include "graphics/engine/engine.h"
#include "graphics/opengl/gldevice.h"

#include <string>
#include <vector>


class CInstanceManager;
class CEvent;
class CRobotMain;
class CSound;

/**
  \struct JoystickDevice
  \brief Information about a joystick device */
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
  \enum VideoQueryResult
  \brief Result of querying for available video resolutions */
enum VideoQueryResult
{
    VIDEO_QUERY_ERROR,
    VIDEO_QUERY_NONE,
    VIDEO_QUERY_ALL,
    VIDEO_QUERY_OK
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
 * CRobotMain and CSound classes.
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
    bool        ParseArguments(int argc, char *argv[]);
    //! Initializes the application
    bool        Create();
    //! Main event loop
    int         Run();
    //! Returns the code to be returned at main() exit
    int         GetExitCode();

    //! Cleans up before exit
    void        Destroy();

    //! Returns a list of possible video modes
    VideoQueryResult GetVideoResolutionList(std::vector<Math::IntPoint> &resolutions,
                                            bool fullScreen, bool resizeable);

    //! Returns the current video mode
    Gfx::GLDeviceConfig GetVideoConfig();

    //! Change the video mode to given mode
    bool        ChangeVideoConfig(const Gfx::GLDeviceConfig &newConfig);

    //! Updates the simulation state
    void        StepSimulation(float rTime);

    //! Returns a list of available joystick devices
    std::vector<JoystickDevice> GetJoystickList();

    //! Returns info about the current joystick
    JoystickDevice GetJoystick();

    //! Change the current joystick device
    bool        ChangeJoystick(const JoystickDevice &newJoystick);

    //! Enables/disables joystick
    void        SetJoystickEnabled(bool enable);
    //! Returns whether joystick is enabled
    bool        GetJoystickEnabled();

    //! Polls the state of joystick axes and buttons
    void        UpdateJoystick();

    void        FlushPressKey();
    void        ResetKey();
    void        SetKey(int keyRank, int option, int key);
    int         GetKey(int keyRank, int option);

    //! Sets the grab mode for input (keyboard & mouse)
    void        SetGrabInput(bool grab);
    //! Returns the grab mode
    bool        GetGrabInput();

    //! Sets the visiblity of system mouse cursor
    void        SetSystemMouseVisible(bool visible);
    //! Returns the visiblity of system mouse cursor
    bool        GetSystemMouseVisibile();

    //! Sets the position of system mouse cursor (in interface coords)
    void        SetSystemMousePos(Math::Point pos);
    //! Returns the position of system mouse cursor (in interface coords)
    Math::Point GetSystemMousePos();

    //! Enables/disables debug mode (prints more info in logger)
    void        SetDebugMode(bool mode);
    //! Returns whether debug mode is enabled
    bool        GetDebugMode();

    //! Returns the full path to a file in data directory
    std::string GetDataFilePath(const std::string &dirName, const std::string &fileName);

protected:
    //! Creates the window's SDL_Surface
    bool CreateVideoSurface();

    //! Processes the captured SDL event to Event struct
    Event       ParseEvent();
    //! Handles some incoming events
    bool        ProcessEvent(const Event &event);
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
    CSound*                 m_sound;
    //! Main class of the proper game engine
    CRobotMain*             m_robotMain;

    //! Code to return at exit
    int             m_exitCode;
    //! Whether application window is active
    bool            m_active;
    //! Whether debug mode is enabled
    bool            m_debugMode;

    //! Current configuration of OpenGL display device
    Gfx::GLDeviceConfig m_deviceConfig;
    //! Previous configuration of OpenGL display device
    Gfx::GLDeviceConfig m_lastDeviceConfig;

    //! Text set as window title
    std::string     m_windowTitle;

    int             m_keyState;
    Math::Vector    m_axeKey;
    Math::Vector    m_axeJoy;
    Math::Point     m_systemMousePos;
    long            m_mouseWheel;

    long            m_key[50][2];

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
};

