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

// app.cpp

#include "app/app.h"

#include "app/system.h"
#include "common/logger.h"
#include "common/iman.h"
#include "common/image.h"
#include "graphics/opengl/gldevice.h"


#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

#include <stdio.h>


template<> CApplication* CSingleton<CApplication>::mInstance = NULL;


//! Interval of timer called to update joystick state
const int JOYSTICK_TIMER_INTERVAL = 1000/30;

//! Function called by the timer
Uint32 JoystickTimerCallback(Uint32 interval, void *);


/**
 * \struct ApplicationPrivate
 * \brief Private data of CApplication class
 *
 * Contains SDL-specific variables that should not be visible outside application module.
 */
struct ApplicationPrivate
{
    //! Display surface
    SDL_Surface *surface;
    //! Currently handled event
    SDL_Event currentEvent;
    //! Joystick
    SDL_Joystick *joystick;
    //! Id of joystick timer
    SDL_TimerID joystickTimer;

    ApplicationPrivate()
    {
        memset(&currentEvent, 0, sizeof(SDL_Event));
        surface = NULL;
        joystick = NULL;
        joystickTimer = 0;
    }
};



CApplication::CApplication()
{
    m_private    = new ApplicationPrivate();
    m_iMan       = new CInstanceManager();
    m_eventQueue = new CEventQueue(m_iMan);

    m_engine    = NULL;
    m_device    = NULL;
    m_robotMain = NULL;
    m_sound     = NULL;

    m_keyState = 0;
    m_axeKey   = Math::Vector(0.0f, 0.0f, 0.0f);
    m_axeJoy   = Math::Vector(0.0f, 0.0f, 0.0f);

    m_exitCode  = 0;
    m_active    = false;
    m_debugMode = false;

    m_windowTitle = "COLOBOT";

    m_joystickEnabled = false;

    m_dataPath = "./data";

    ResetKey();
}

CApplication::~CApplication()
{
    delete m_private;
    m_private = NULL;

    delete m_eventQueue;
    m_eventQueue = NULL;

    delete m_iMan;
    m_iMan = NULL;
}

bool CApplication::ParseArguments(int argc, char *argv[])
{
    bool waitDataDir = false;

    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];

        if (waitDataDir)
        {
            waitDataDir = false;
            m_dataPath = arg;
            continue;
        }

        if (arg == "-debug")
        {
            SetDebugMode(true);
        }
        else if (arg == "-datadir")
        {
            waitDataDir = true;
        }
        else
        {
            m_exitCode = 1;
            return false;
        }
    }

    // Data dir not given?
    if (waitDataDir)
        return false;

    return true;
}

bool CApplication::Create()
{
    // TODO: verify that data directory exists

    // Temporarily -- only in windowed mode
    m_deviceConfig.fullScreen = false;

/*    // Create the sound instance.
    m_sound = new CSound(m_iMan);

    // Create the robot application.
    m_robotMain = new CRobotMain(m_iMan); */



    /* SDL initialization sequence */


    Uint32 initFlags = SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_TIMER;

    if (SDL_Init(initFlags) < 0)
    {
        SystemDialog( SDT_ERROR, "COLOBOT - Fatal Error",
                      "SDL initialization error:\n" +
                      std::string(SDL_GetError()) );
        m_exitCode = 2;
        return false;
    }

    if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) == 0)
    {
        SystemDialog( SDT_ERROR, "COLOBOT - Fatal Error",
                      std::string("SDL_Image initialization error:\n") +
                      std::string(IMG_GetError()) );
        m_exitCode = 3;
        return false;
    }

    if (! CreateVideoSurface())
        return false; // dialog is in function

    if (m_private->surface == NULL)
    {
        SystemDialog( SDT_ERROR, "COLOBT - Fatal Error",
                      std::string("SDL error while setting video mode:\n") +
                      std::string(SDL_GetError()) );
        m_exitCode = 2;
        return false;
    }

    SDL_WM_SetCaption(m_windowTitle.c_str(), m_windowTitle.c_str());

    // Enable translating key codes of key press events to unicode chars
    SDL_EnableUNICODE(1);

    // Don't generate joystick events
    SDL_JoystickEventState(SDL_IGNORE);


    // For now, enable joystick for testing
    SetJoystickEnabled(true);


    // The video is ready, we can create and initalize the graphics device
    m_device = new Gfx::CGLDevice(m_deviceConfig);
    if (! m_device->Create() )
    {
        SystemDialog( SDT_ERROR, "COLOBT - Fatal Error",
                      std::string("Error in CDevice::Create()") );
        m_exitCode = 1;
        return false;
    }

    // Create the 3D engine
    m_engine = new Gfx::CEngine(m_iMan, this);

    m_engine->SetDevice(m_device);

    if (! m_engine->Create() )
    {
        SystemDialog( SDT_ERROR, "COLOBT - Fatal Error",
                      std::string("Error in CEngine::Init()") );
        m_exitCode = 1;
        return false;
    }

    return true;
}

bool CApplication::CreateVideoSurface()
{
    const SDL_VideoInfo *videoInfo = SDL_GetVideoInfo();
    if (videoInfo == NULL)
    {
        SystemDialog( SDT_ERROR, "COLOBOT - Fatal Error",
                      std::string("SDL error while getting video info:\n ") +
                      std::string(SDL_GetError()) );
        m_exitCode = 2;
        return false;
    }

    Uint32 videoFlags = SDL_OPENGL | SDL_GL_DOUBLEBUFFER | SDL_HWPALETTE;

    // Use hardware surface if available
    if (videoInfo->hw_available)
        videoFlags |= SDL_HWSURFACE;
    else
        videoFlags |= SDL_SWSURFACE;

    // Enable hardware blit if available
    if (videoInfo->blit_hw)
        videoFlags |= SDL_HWACCEL;

    if (m_deviceConfig.fullScreen)
        videoFlags |= SDL_FULLSCREEN;

    if (m_deviceConfig.resizeable)
        videoFlags |= SDL_RESIZABLE;

    // Set OpenGL attributes

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE,   m_deviceConfig.redSize);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, m_deviceConfig.greenSize);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,  m_deviceConfig.blueSize);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, m_deviceConfig.alphaSize);

    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, m_deviceConfig.depthSize);

    if (m_deviceConfig.doubleBuf)
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    /* If hardware acceleration specifically requested, this will force the hw accel
       and fail with error if not available */
    if (m_deviceConfig.hardwareAccel)
        SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

    m_private->surface = SDL_SetVideoMode(m_deviceConfig.size.x, m_deviceConfig.size.y,
                                          m_deviceConfig.bpp, videoFlags);

    return true;
}

void CApplication::Destroy()
{
    /*if (m_robotMain != NULL)
    {
        delete m_robotMain;
        m_robotMain = NULL;
    }

    if (m_sound != NULL)
    {
        delete m_sound;
        m_sound = NULL;
    }*/

    if (m_engine != NULL)
    {
        m_engine->Destroy();

        delete m_engine;
        m_engine = NULL;
    }

    if (m_device != NULL)
    {
        m_device->Destroy();

        delete m_device;
        m_device = NULL;
    }

    if (m_private->joystick != NULL)
    {
        SDL_JoystickClose(m_private->joystick);
        m_private->joystick = NULL;
    }

    if (m_private->surface != NULL)
    {
        SDL_FreeSurface(m_private->surface);
        m_private->surface = NULL;
    }

    IMG_Quit();

    SDL_Quit();
}

bool CApplication::ChangeVideoConfig(const Gfx::GLDeviceConfig &newConfig)
{
    static bool restore = false;

    m_lastDeviceConfig = m_deviceConfig;
    m_deviceConfig = newConfig;


    SDL_FreeSurface(m_private->surface);

    if (! CreateVideoSurface())
    {
        // Fatal error, so post the quit event
        m_eventQueue->AddEvent(Event(EVENT_QUIT));
        return false;
    }

    if (m_private->surface == NULL)
    {
        if (! restore)
        {
            SystemDialog( SDT_ERROR, "COLOBT - Error",
                          std::string("SDL error while setting video mode:\n") +
                          std::string(SDL_GetError()) + std::string("\n") +
                          std::string("Previous mode will be restored") );

            restore = true;
            ChangeVideoConfig(m_lastDeviceConfig);
            return false;
        }
        else
        {
            restore = false;

            SystemDialog( SDT_ERROR, "COLOBT - Fatal Error",
                          std::string("SDL error while restoring previous video mode:\n") +
                          std::string(SDL_GetError()) );


            // Fatal error, so post the quit event
            m_eventQueue->AddEvent(Event(EVENT_QUIT));
            return false;
        }
    }

    ( static_cast<Gfx::CGLDevice*>(m_device) )->ConfigChanged(m_deviceConfig);

    m_engine->ResetAfterDeviceChanged();

    return true;
}

bool CApplication::OpenJoystick()
{
    if ( (m_joystick.index < 0) || (m_joystick.index >= SDL_NumJoysticks()) )
        return false;

    m_private->joystick = SDL_JoystickOpen(m_joystick.index);
    if (m_private->joystick == NULL)
        return false;

    m_joystick.axisCount   = SDL_JoystickNumAxes(m_private->joystick);
    m_joystick.buttonCount = SDL_JoystickNumButtons(m_private->joystick);

    // Create the vectors with joystick axis & button states to exactly the required size
    m_joyAxeState = std::vector<int>(m_joystick.axisCount, 0);
    m_joyButtonState = std::vector<bool>(m_joystick.buttonCount, false);

    // Create a timer for polling joystick state
    m_private->joystickTimer = SDL_AddTimer(JOYSTICK_TIMER_INTERVAL, JoystickTimerCallback, NULL);

    return true;
}

void CApplication::CloseJoystick()
{
    // Timer will remove itself automatically

    SDL_JoystickClose(m_private->joystick);
    m_private->joystick = NULL;
}

bool CApplication::ChangeJoystick(const JoystickDevice &newJoystick)
{
    if ( (newJoystick.index < 0) || (newJoystick.index >= SDL_NumJoysticks()) )
        return false;

    if (m_private->joystick != NULL)
        CloseJoystick();

    return OpenJoystick();
}

Uint32 JoystickTimerCallback(Uint32 interval, void *)
{
    CApplication *app = CApplication::GetInstancePointer();
    if ((app == NULL) || (! app->GetJoystickEnabled()))
        return 0; // don't run the timer again

    app->UpdateJoystick();

    return interval; // run for the same interval again
}

/** Updates the state info in CApplication and on change, creates SDL events and pushes them to SDL event queue.
    This way, the events get handled properly in the main event loop and besides, SDL_PushEvent() ensures thread-safety. */
void CApplication::UpdateJoystick()
{
    if (! m_joystickEnabled)
        return;

    SDL_JoystickUpdate();

    for (int axis = 0; axis < static_cast<int>( m_joyAxeState.size() ); ++axis)
    {
        int newValue = SDL_JoystickGetAxis(m_private->joystick, axis);

        if (m_joyAxeState[axis] != newValue)
        {
            m_joyAxeState[axis] = newValue;

            SDL_Event joyAxisEvent;

            joyAxisEvent.jaxis.type = SDL_JOYAXISMOTION;
            joyAxisEvent.jaxis.which = 0;
            joyAxisEvent.jaxis.axis = axis;
            joyAxisEvent.jaxis.value = newValue;

            SDL_PushEvent(&joyAxisEvent);
        }
    }

    for (int button = 0; button < static_cast<int>( m_joyButtonState.size() ); ++button)
    {
        bool newValue = SDL_JoystickGetButton(m_private->joystick, button) == 1;

        if (m_joyButtonState[button] != newValue)
        {
            m_joyButtonState[button] = newValue;

            SDL_Event joyButtonEvent;

            if (newValue)
            {
                joyButtonEvent.jbutton.type = SDL_JOYBUTTONDOWN;
                joyButtonEvent.jbutton.state = SDL_PRESSED;
            }
            else
            {
                joyButtonEvent.jbutton.type = SDL_JOYBUTTONUP;
                joyButtonEvent.jbutton.state = SDL_RELEASED;
            }
            joyButtonEvent.jbutton.which = 0;
            joyButtonEvent.jbutton.button = button;

            SDL_PushEvent(&joyButtonEvent);
        }
    }
}

void CApplication::UpdateMouse()
{
    Math::IntPoint pos;
    SDL_GetMouseState(&pos.x, &pos.y);
    m_systemMousePos = m_engine->WindowToInterfaceCoords(pos);
    m_engine->SetMousePos(m_systemMousePos);
}

int CApplication::Run()
{
    m_active = true;

    while (true)
    {
        // To be sure no old event remains
        m_private->currentEvent.type = SDL_NOEVENT;

        // Call SDL_PumpEvents() only once here
        // (SDL_PeepEvents() doesn't call it)
        if (m_active)
            SDL_PumpEvents();

        bool haveEvent = true;
        while (haveEvent)
        {
            haveEvent = false;

            int count = 0;
            // Use SDL_PeepEvents() if the app is active, so we can use idle time to
            // render the scene. Else, use SDL_WaitEvent() to avoid eating CPU time.
            if (m_active)
                count = SDL_PeepEvents(&m_private->currentEvent, 1, SDL_GETEVENT, SDL_ALLEVENTS);
            else
                count = SDL_WaitEvent(&m_private->currentEvent);

            // If received an event
            if (count > 0)
            {
                haveEvent = true;

                Event event = ParseEvent();

                if (event.type == EVENT_QUIT)
                    goto end; // exit the loop

                if (event.type != EVENT_NULL)
                {
                    bool passOn = ProcessEvent(event);

                    if (m_engine != NULL && passOn)
                        passOn = m_engine->ProcessEvent(event);

                    if (passOn)
                        m_eventQueue->AddEvent(event);
                }
            }
        }

        // Enter game update & frame rendering only if active
        if (m_active)
        {
            Event event;
            while (m_eventQueue->GetEvent(event))
            {
                if (event.type == EVENT_QUIT)
                    goto end; // exit both loops

                bool passOn = true;

                // Skip system events (they have been processed earlier)
                if (! event.systemEvent)
                {
                    passOn = ProcessEvent(event);

                    if (passOn && m_engine != NULL)
                        passOn = m_engine->ProcessEvent(event);
                }

                /*if (passOn && m_robotMain != NULL)
                    m_robotMain->ProcessEvent(event); */
            }

            /* Update mouse position explicitly right before rendering
             * because mouse events are usually way behind */
            UpdateMouse();

            // Update game and render a frame during idle time (no messages are waiting)
            Render();
        }
    }

end:
    Destroy();

    return m_exitCode;
}

int CApplication::GetExitCode()
{
    return m_exitCode;
}

//! Translates SDL press state to PressState
PressState TranslatePressState(unsigned char state)
{
    if (state == SDL_PRESSED)
        return STATE_PRESSED;
    else
        return STATE_RELEASED;
}

/** The SDL event parsed is stored internally.
    If event is not available or is not understood, returned event is of type EVENT_NULL. */
Event CApplication::ParseEvent()
{
    Event event;

    event.systemEvent = true;

    if (m_private->currentEvent.type == SDL_QUIT)
    {
        event.type = EVENT_QUIT;
    }
    else if ( (m_private->currentEvent.type == SDL_KEYDOWN) ||
              (m_private->currentEvent.type == SDL_KEYUP) )
    {
        if (m_private->currentEvent.type == SDL_KEYDOWN)
            event.type = EVENT_KEY_DOWN;
        else
            event.type = EVENT_KEY_UP;

        event.key.key = m_private->currentEvent.key.keysym.sym;
        event.key.mod = m_private->currentEvent.key.keysym.mod;
        event.key.state = TranslatePressState(m_private->currentEvent.key.state);
        event.key.unicode = m_private->currentEvent.key.keysym.unicode;
    }
    else if ( (m_private->currentEvent.type == SDL_MOUSEBUTTONDOWN) ||
         (m_private->currentEvent.type == SDL_MOUSEBUTTONUP) )
    {
        if (m_private->currentEvent.type == SDL_MOUSEBUTTONDOWN)
            event.type = EVENT_MOUSE_BUTTON_DOWN;
        else
            event.type = EVENT_MOUSE_BUTTON_UP;

        event.mouseButton.button = m_private->currentEvent.button.button;
        event.mouseButton.state = TranslatePressState(m_private->currentEvent.button.state);
        event.mouseButton.pos = m_engine->WindowToInterfaceCoords(
            Math::IntPoint(m_private->currentEvent.button.x, m_private->currentEvent.button.y));
    }
    else if (m_private->currentEvent.type == SDL_MOUSEMOTION)
    {
        event.type = EVENT_MOUSE_MOVE;

        event.mouseMove.state = TranslatePressState(m_private->currentEvent.button.state);
        event.mouseMove.pos = m_engine->WindowToInterfaceCoords(
            Math::IntPoint(m_private->currentEvent.button.x, m_private->currentEvent.button.y));
    }
    else if (m_private->currentEvent.type == SDL_JOYAXISMOTION)
    {
        event.type = EVENT_JOY_AXIS;

        event.joyAxis.axis = m_private->currentEvent.jaxis.axis;
        event.joyAxis.value = m_private->currentEvent.jaxis.value;
    }
    else if ( (m_private->currentEvent.type == SDL_JOYBUTTONDOWN) ||
              (m_private->currentEvent.type == SDL_JOYBUTTONUP) )
    {
        if (m_private->currentEvent.type == SDL_JOYBUTTONDOWN)
            event.type = EVENT_JOY_BUTTON_DOWN;
        else
            event.type = EVENT_JOY_BUTTON_UP;

        event.joyButton.button = m_private->currentEvent.jbutton.button;
        event.joyButton.state = TranslatePressState(m_private->currentEvent.jbutton.state);
    }
    else if (m_private->currentEvent.type == SDL_ACTIVEEVENT)
    {
        event.type = EVENT_ACTIVE;

        if (m_private->currentEvent.active.type & SDL_APPINPUTFOCUS)
            event.active.flags |= ACTIVE_INPUT;
        if (m_private->currentEvent.active.type & SDL_APPMOUSEFOCUS)
            event.active.flags |= ACTIVE_MOUSE;
        if (m_private->currentEvent.active.type & SDL_APPACTIVE)
            event.active.flags |= ACTIVE_APP;

        event.active.gain = m_private->currentEvent.active.gain == 1;
    }

    return event;
}

/** Processes incoming events. It is the first function called after an event is captures.
    Function returns \c true if the event is to be passed on to other processing functions
    or \c false if not. */
bool CApplication::ProcessEvent(const Event &event)
{
    CLogger *l = GetLogger();

    if (event.type == EVENT_ACTIVE)
    {
        m_active = event.active.gain;
        if (m_debugMode)
            l->Info("Focus change: active = %s\n", m_active ? "true" : "false");
    }

    // Print the events in debug mode to test the code
    if (m_debugMode)
    {
        switch (event.type)
        {
            case EVENT_KEY_DOWN:
            case EVENT_KEY_UP:
                l->Info("EVENT_KEY_%s:\n", (event.type == EVENT_KEY_DOWN) ? "DOWN" : "UP");
                l->Info(" key     = %4x\n", event.key.key);
                l->Info(" state   = %s\n", (event.key.state == STATE_PRESSED) ? "STATE_PRESSED" : "STATE_RELEASED");
                l->Info(" mod     = %4x\n", event.key.mod);
                l->Info(" unicode = %4x\n", event.key.unicode);
                break;
            case EVENT_MOUSE_MOVE:
                l->Info("EVENT_MOUSE_MOVE:\n");
                l->Info(" state  = %s\n", (event.mouseMove.state == STATE_PRESSED) ? "STATE_PRESSED" : "STATE_RELEASED");
                l->Info(" pos    = (%f, %f)\n", event.mouseMove.pos.x, event.mouseMove.pos.y);
                break;
            case EVENT_MOUSE_BUTTON_DOWN:
            case EVENT_MOUSE_BUTTON_UP:
                l->Info("EVENT_MOUSE_BUTTON_%s:\n", (event.type == EVENT_MOUSE_BUTTON_DOWN) ? "DOWN" : "UP");
                l->Info(" button = %d\n", event.mouseButton.button);
                l->Info(" state  = %s\n", (event.mouseButton.state == STATE_PRESSED) ? "STATE_PRESSED" : "STATE_RELEASED");
                l->Info(" pos    = (%f, %f)\n", event.mouseButton.pos.x, event.mouseButton.pos.y);
                break;
            case EVENT_JOY_AXIS:
                l->Info("EVENT_JOY_AXIS:\n");
                l->Info(" axis  = %d\n", event.joyAxis.axis);
                l->Info(" value = %d\n", event.joyAxis.value);
                break;
            case EVENT_JOY_BUTTON_DOWN:
            case EVENT_JOY_BUTTON_UP:
                l->Info("EVENT_JOY_BUTTON_%s:\n", (event.type == EVENT_JOY_BUTTON_DOWN) ? "DOWN" : "UP");
                l->Info(" button = %d\n", event.joyButton.button);
                l->Info(" state  = %s\n", (event.joyButton.state == STATE_PRESSED) ? "STATE_PRESSED" : "STATE_RELEASED");
                break;
            case EVENT_ACTIVE:
                l->Info("EVENT_ACTIVE:\n");
                l->Info(" flags = 0x%x\n", event.active.flags);
                l->Info(" gain  = %s\n", event.active.gain ? "true" : "false");
                break;
            default:
                break;
        }
    }

    // By default, pass on all events
    return true;
}

/** Renders the frame and swaps buffers as necessary */
void CApplication::Render()
{
    m_engine->Render();

    if (m_deviceConfig.doubleBuf)
        SDL_GL_SwapBuffers();
}

void CApplication::StepSimulation(float rTime)
{
    // TODO
}

Gfx::GLDeviceConfig CApplication::GetVideoConfig()
{
    return m_deviceConfig;
}

VideoQueryResult CApplication::GetVideoResolutionList(std::vector<Math::IntPoint> &resolutions,
                                                      bool fullScreen, bool resizeable)
{
    resolutions.clear();

    const SDL_VideoInfo *videoInfo = SDL_GetVideoInfo();
    if (videoInfo == NULL)
        return VIDEO_QUERY_ERROR;

    Uint32 videoFlags = SDL_OPENGL | SDL_GL_DOUBLEBUFFER | SDL_HWPALETTE;

    // Use hardware surface if available
    if (videoInfo->hw_available)
        videoFlags |= SDL_HWSURFACE;
    else
        videoFlags |= SDL_SWSURFACE;

    // Enable hardware blit if available
    if (videoInfo->blit_hw)
        videoFlags |= SDL_HWACCEL;

    if (resizeable)
        videoFlags |= SDL_RESIZABLE;

    if (fullScreen)
        videoFlags |= SDL_FULLSCREEN;


    SDL_Rect **modes = SDL_ListModes(NULL, videoFlags);

    if (modes == reinterpret_cast<SDL_Rect **>(0) )
        return VIDEO_QUERY_NONE; // no modes available

    if (modes == reinterpret_cast<SDL_Rect **>(-1) )
        return VIDEO_QUERY_ALL; // all resolutions are possible


    for (int i = 0; modes[i] != NULL; ++i)
        resolutions.push_back(Math::IntPoint(modes[i]->w, modes[i]->h));

    return VIDEO_QUERY_OK;
}

void CApplication::SetDebugMode(bool mode)
{
    m_debugMode = mode;
}

bool CApplication::GetDebugMode()
{
    return m_debugMode;
}

void CApplication::FlushPressKey()
{
    // TODO
}

void CApplication::ResetKey()
{
    // TODO
}

void CApplication::SetKey(int keyRank, int option, int key)
{
    // TODO
}

int CApplication::GetKey(int keyRank, int option)
{
    // TODO
    return 0;
}

void CApplication::SetGrabInput(bool grab)
{
    SDL_WM_GrabInput(grab ? SDL_GRAB_ON : SDL_GRAB_OFF);
}

bool CApplication::GetGrabInput()
{
    int result = SDL_WM_GrabInput(SDL_GRAB_QUERY);
    return result == SDL_GRAB_ON;
}

void CApplication::SetSystemMouseVisible(bool visible)
{
    SDL_ShowCursor(visible ? SDL_ENABLE : SDL_DISABLE);
}

bool CApplication::GetSystemMouseVisibile()
{
    int result = SDL_ShowCursor(SDL_QUERY);
    return result == SDL_ENABLE;
}

void CApplication::SetSystemMousePos(Math::Point pos)
{
    Math::IntPoint windowPos = m_engine->InterfaceToWindowCoords(pos);
    SDL_WarpMouse(windowPos.x, windowPos.y);
    m_systemMousePos = pos;
}

Math::Point CApplication::GetSystemMousePos()
{
    return m_systemMousePos;
}

std::vector<JoystickDevice> CApplication::GetJoystickList()
{
    std::vector<JoystickDevice> result;

    int count = SDL_NumJoysticks();

    for (int index = 0; index < count; ++index)
    {
        JoystickDevice device;
        device.index = index;
        device.name = SDL_JoystickName(index);
        result.push_back(device);
    }

    return result;
}

JoystickDevice CApplication::GetJoystick()
{
    return m_joystick;
}

void CApplication::SetJoystickEnabled(bool enable)
{
    m_joystickEnabled = enable;

    if (m_joystickEnabled)
    {
        if (! OpenJoystick())
        {
            m_joystickEnabled = false;
        }
    }
    else
    {
        CloseJoystick();
    }
}

bool CApplication::GetJoystickEnabled()
{
    return m_joystickEnabled;
}

std::string CApplication::GetDataFilePath(const std::string& dirName, const std::string& fileName)
{
    return m_dataPath + "/" + dirName + "/" + fileName;
}
