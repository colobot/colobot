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
#include "common/iman.h"
#include "graphics/opengl/gldevice.h"


#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

#include <stdio.h>


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
    //! Index of joystick device
    int joystickIndex;
    //! Id of joystick timer
    SDL_TimerID joystickTimer;
    //! Current configuration of OpenGL display device
    Gfx::GLDeviceConfig deviceConfig;

    ApplicationPrivate()
    {
        memset(&currentEvent, 0, sizeof(SDL_Event));
        surface = NULL;
        joystick = NULL;
        joystickIndex = 0;
        joystickTimer = 0;
    }
};


CApplication* CApplication::m_appInstance = NULL;


CApplication::CApplication()
{
    assert(m_appInstance == NULL);
    m_appInstance = this;

    m_private = new ApplicationPrivate();
    m_exitCode = 0;

    m_iMan = new CInstanceManager();

    m_eventQueue = new CEventQueue(m_iMan);

    m_engine    = NULL;
    m_device    = NULL;
    m_robotMain = NULL;
    m_sound     = NULL;

    m_keyState = 0;
    m_axeKey = Math::Vector(0.0f, 0.0f, 0.0f);
    m_axeJoy = Math::Vector(0.0f, 0.0f, 0.0f);

    m_active          = false;
    m_activateApp     = false;
    m_ready           = false;
    m_joystickEnabled = false;

    m_time        = 0.0f;

    m_windowTitle  = "COLOBOT";

    m_showStats      = false;
    m_debugMode      = false;
    m_setupMode      = true;

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

    m_appInstance = NULL;
}

Error CApplication::ParseArguments(int argc, char *argv[])
{
    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];

        if (arg == "-debug")
        {
            m_showStats = true;
            SetDebugMode(true);
        }
        // TODO else {} report invalid argument
    }

    return ERR_OK;
}

bool CApplication::Create()
{
    // Temporarily -- only in windowed mode
    m_private->deviceConfig.fullScreen = false;

    // Create the 3D engine
    m_engine = new Gfx::CEngine(m_iMan, this);

    // Initialize the app's custom scene stuff, but before initializing the graphics device
    if (! m_engine->BeforeCreateInit())
    {
        SystemDialog(SDT_ERROR, "COLOBOT - Error", std::string("Error in CEngine::BeforeCreateInit() :\n") +
                                                   std::string(m_engine->GetError()) );
        return false;
    }

/*    // Create the sound instance.
    m_sound = new CSound(m_iMan);

    // Create the robot application.
    m_robotMain = new CRobotMain(m_iMan); */



    /* SDL initialization sequence */


    Uint32 initFlags = SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_TIMER;

    if (SDL_Init(initFlags) < 0)
    {
        SystemDialog( SDT_ERROR, "COLOBOT - Error", "SDL initialization error:\n" +
                                                    std::string(SDL_GetError()) );
        return false;
    }

    const SDL_VideoInfo *videoInfo = SDL_GetVideoInfo();
    if (videoInfo == NULL)
    {
        SystemDialog( SDT_ERROR, "COLOBOT - Error", "SDL error while getting video info:\n " +
                                                    std::string(SDL_GetError()) );
        return false;
    }

    Uint32 videoFlags = SDL_OPENGL | SDL_GL_DOUBLEBUFFER | SDL_HWPALETTE;

    if (m_private->deviceConfig.resizeable)
        videoFlags |= SDL_RESIZABLE;

    // Use hardware surface if available
    if (videoInfo->hw_available)
        videoFlags |= SDL_HWSURFACE;
    else
        videoFlags |= SDL_SWSURFACE;

    // Enable hardware blit if available
    if (videoInfo->blit_hw)
        videoFlags |= SDL_HWACCEL;

    if (m_private->deviceConfig.fullScreen)
        videoFlags |= SDL_FULLSCREEN;

    // Set OpenGL attributes

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE,   m_private->deviceConfig.redSize);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, m_private->deviceConfig.greenSize);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,  m_private->deviceConfig.blueSize);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, m_private->deviceConfig.alphaSize);

    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, m_private->deviceConfig.depthSize);

    if (m_private->deviceConfig.doubleBuf)
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    /* If hardware acceleration specifically requested, this will force the hw accel
       and fail with error if not available */
    if (m_private->deviceConfig.hardwareAccel)
        SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

    if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) == 0)
    {
        SystemDialog( SDT_ERROR, "COLOBOT - Error", std::string("SDL_Image initialization error:\n") +
                                                    std::string(IMG_GetError()) );
        return false;
    }

    m_private->surface = SDL_SetVideoMode(m_private->deviceConfig.width, m_private->deviceConfig.height,
                                          m_private->deviceConfig.bpp, videoFlags);

    if (m_private->surface == NULL)
    {
        SystemDialog( SDT_ERROR, "COLOBT - Error", std::string("SDL error while setting video mode:\n") +
                                                   std::string(SDL_GetError()) );
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
    m_device = new Gfx::CGLDevice();
    if (! m_device->Create() )
    {
        SystemDialog( SDT_ERROR, "COLOBT - Error", std::string("Error in CDevice::Create() :\n") +
                                                   std::string(m_device->GetError()) );
        return false;
    }

    m_engine->SetDevice(m_device);
    if (! m_engine->Create() )
    {
        SystemDialog( SDT_ERROR, "COLOBT - Error", std::string("Error in CEngine::Create() :\n") +
                                                   std::string(m_engine->GetError()) );
        return false;
    }

    if (! m_engine->AfterDeviceSetInit() )
    {
        SystemDialog( SDT_ERROR, "COLOBT - Error", std::string("Error in CEngine::AfterDeviceSetInit() :\n") +
                                                   std::string(m_engine->GetError()) );
        return false;
    }


    // The app is ready to go
    m_ready = true;

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
        if (m_engine->GetWasInit())
            m_engine->Destroy();

        delete m_engine;
        m_engine = NULL;
    }

    if (m_device != NULL)
    {
        if (m_device->GetWasInit())
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

bool CApplication::OpenJoystick()
{
    m_private->joystick = SDL_JoystickOpen(m_private->joystickIndex);
    if (m_private->joystick == NULL)
        return false;

    // Create the vectors with joystick axis & button states to exactly the required size
    m_joyAxeState = std::vector<int>(SDL_JoystickNumAxes(m_private->joystick), 0);
    m_joyButtonState = std::vector<bool>(SDL_JoystickNumButtons(m_private->joystick), false);

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

Uint32 JoystickTimerCallback(Uint32 interval, void *)
{
    CApplication *app = CApplication::GetInstance();
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

    for (int axis = 0; axis < (int) m_joyAxeState.size(); ++axis)
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

    for (int button = 0; button < (int) m_joyButtonState.size(); ++button)
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

int CApplication::Run()
{
    m_active = true;

    while (m_private->currentEvent.type != SDL_QUIT)
    {
        // Use SDL_PeepEvents() if the app is active, so we can use idle time to
        // render the scene. Else, use SDL_PollEvent() to avoid eating CPU time.
        int count = 0;
        if (m_active)
        {
            SDL_PumpEvents();
            count = SDL_PeepEvents(&m_private->currentEvent, 1, SDL_GETEVENT, SDL_ALLEVENTS);
        }
        else
        {
            SDL_PollEvent(&m_private->currentEvent);
        }

        // If received an event
        if ((m_active && count > 0) || (!m_active))
        {
            ParseEvent();
        }

        // Render a frame during idle time (no messages are waiting)
        if (m_active && m_ready)
        {
            Event event;
            while (m_eventQueue->GetEvent(event))
            {
                if (event.type == EVENT_QUIT)
                {
                    goto end; // exit both loops
                }

                //m_robotMain->EventProcess(event);
            }

            // If an error occurs, push quit event to the queue
            if (! Render())
            {
                SDL_Event quitEvent;
                memset(&quitEvent, 0, sizeof(SDL_Event));
                quitEvent.type = SDL_QUIT;
                SDL_PushEvent(&quitEvent);
            }
        }
    }

end:
    //m_sound->StopMusic();
    Destroy();

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

/** Conversion of the position of the mouse to the following coordinates:

x: 0=left, 1=right

y: 0=down, 1=up
*/
Math::Point CApplication::WindowToInterfaceCoords(int x, int y)
{
    return Math::Point((float)x / (float)m_private->deviceConfig.width,
                       1.0f - (float)y / (float)m_private->deviceConfig.height);
}


void CApplication::ParseEvent()
{
    Event event;

    if ( (m_private->currentEvent.type == SDL_KEYDOWN) ||
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
        event.mouseButton.pos = WindowToInterfaceCoords(m_private->currentEvent.button.x, m_private->currentEvent.button.y);
    }
    else if (m_private->currentEvent.type == SDL_MOUSEMOTION)
    {
        event.type = EVENT_MOUSE_MOVE;

        event.mouseMove.state = TranslatePressState(m_private->currentEvent.button.state);
        event.mouseMove.pos = WindowToInterfaceCoords(m_private->currentEvent.button.x, m_private->currentEvent.button.y);
    }
    // TODO: joystick state polling instead of getting events
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


    if (m_robotMain != NULL && event.type != EVENT_NULL)
    {
        //m_robotMain->EventProcess(event);
    }

    ProcessEvent(event);
}

void CApplication::ProcessEvent(Event event)
{
    // Print the events in debug mode to test the code
    if (m_debugMode)
    {
        switch (event.type)
        {
            case EVENT_KEY_DOWN:
            case EVENT_KEY_UP:
                printf("EVENT_KEY_%s:\n", (event.type == EVENT_KEY_DOWN) ? "DOWN" : "UP");
                printf(" key     = %4x\n", event.key.key);
                printf(" state   = %s\n", (event.key.state == STATE_PRESSED) ? "STATE_PRESSED" : "STATE_RELEASED");
                printf(" mod     = %4x\n", event.key.mod);
                printf(" unicode = %4x\n", event.key.unicode);
                break;
            case EVENT_MOUSE_MOVE:
                printf("EVENT_MOUSE_MOVE:\n");
                printf(" state  = %s\n", (event.mouseMove.state == STATE_PRESSED) ? "STATE_PRESSED" : "STATE_RELEASED");
                printf(" pos    = (%f, %f)\n", event.mouseMove.pos.x, event.mouseMove.pos.y);
                break;
            case EVENT_MOUSE_BUTTON_DOWN:
            case EVENT_MOUSE_BUTTON_UP:
                printf("EVENT_MOUSE_BUTTON_%s:\n", (event.type == EVENT_MOUSE_BUTTON_DOWN) ? "DOWN" : "UP");
                printf(" button = %d\n", event.mouseButton.button);
                printf(" state  = %s\n", (event.mouseButton.state == STATE_PRESSED) ? "STATE_PRESSED" : "STATE_RELEASED");
                printf(" pos    = (%f, %f)\n", event.mouseButton.pos.x, event.mouseButton.pos.y);
                break;
            case EVENT_JOY_AXIS:
                printf("EVENT_JOY_AXIS:\n");
                printf(" axis  = %d\n", event.joyAxis.axis);
                printf(" value = %d\n", event.joyAxis.value);
                break;
            case EVENT_JOY_BUTTON_DOWN:
            case EVENT_JOY_BUTTON_UP:
                printf("EVENT_JOY_BUTTON_%s:\n", (event.type == EVENT_JOY_BUTTON_DOWN) ? "DOWN" : "UP");
                printf(" button = %d\n", event.joyButton.button);
                printf(" state  = %s\n", (event.joyButton.state == STATE_PRESSED) ? "STATE_PRESSED" : "STATE_RELEASED");
                break;
            default:
                break;
        }
    }
}

bool CApplication::Render()
{
    bool result = m_engine->Render();
    if (! result)
        return false;

    if (m_private->deviceConfig.doubleBuf)
        SDL_GL_SwapBuffers();

    return true;
}

/** Called in to toggle the pause state of the app. */
void CApplication::Pause(bool pause)
{
    static long appPausedCount = 0L;

    appPausedCount += ( pause ? +1 : -1 );
    m_ready         = appPausedCount == 0;

    // Handle the first pause request (of many, nestable pause requests)
    if( pause && ( 1 == appPausedCount ) )
    {
        // Stop the scene from animating
        //m_engine->TimeEnterGel();
    }

    // Final pause request done
    if (appPausedCount == 0)
    {
        // Restart the scene
        //m_engine->TimeExitGel();
    }
}

void CApplication::StepSimulation(float rTime)
{
    // TODO
}

void CApplication::SetShowStat(bool show)
{
    m_showStats = show;
}

bool CApplication::GetShowStat()
{
    return m_showStats;
}

void CApplication::SetDebugMode(bool mode)
{
    m_debugMode = mode;
}

bool CApplication::GetDebugMode()
{
    return m_debugMode;
}

bool CApplication::GetSetupMode()
{
    return m_setupMode;
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

void CApplication::SetMousePos(Math::Point pos)
{
    // TODO
}

void CApplication::SetMouseType(Gfx::MouseType type)
{
    // TODO
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

bool CApplication::WriteScreenShot(char *filename, int width, int height)
{
    // TODO
    return false;
}

void CApplication::InitText()
{
    // TODO
}

void CApplication::DrawSuppl()
{
    // TODO
}

void CApplication::ShowStats()
{
    // TODO
}

void CApplication::OutputText(long x, long y, char* str)
{
    // TODO
}
