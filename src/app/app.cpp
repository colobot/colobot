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

#include "common/config.h"

#include "app/app.h"

#include "app/system.h"

#include "common/logger.h"
#include "common/iman.h"
#include "common/image.h"
#include "common/key.h"

#include "graphics/engine/modelmanager.h"
#include "graphics/opengl/gldevice.h"

#include "object/robotmain.h"

#include <boost/filesystem.hpp>

#include <SDL.h>
#include <SDL_image.h>

#include <stdlib.h>
#include <libintl.h>
#include <unistd.h>
#include <getopt.h>


#ifdef OPENAL_SOUND
    #include "sound/oalsound/alsound.h"
#endif


template<> CApplication* CSingleton<CApplication>::m_instance = nullptr;

//! Static buffer for putenv locale
static char S_LANGUAGE[50] = { 0 };


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
    //! Mouse motion event to be handled
    SDL_Event lastMouseMotionEvent;
    //! Joystick
    SDL_Joystick *joystick;
    //! Id of joystick timer
    SDL_TimerID joystickTimer;

    ApplicationPrivate()
    {
        memset(&currentEvent, 0, sizeof(SDL_Event));
        memset(&lastMouseMotionEvent, 0, sizeof(SDL_Event));
        surface = nullptr;
        joystick = nullptr;
        joystickTimer = 0;
    }
};



CApplication::CApplication()
{
    m_private       = new ApplicationPrivate();
    m_iMan          = new CInstanceManager();
    m_eventQueue    = new CEventQueue();
    m_profile       = new CProfile();

    m_engine    = nullptr;
    m_device    = nullptr;
    m_modelManager = nullptr;
    m_robotMain = nullptr;
    m_sound     = nullptr;

    m_exitCode  = 0;
    m_active    = false;
    m_debugMode = false;

    m_windowTitle = "COLOBOT";

    m_simulationSuspended = false;

    m_simulationSpeed = 1.0f;

    m_realAbsTimeBase = 0LL;
    m_realAbsTime = 0LL;
    m_realRelTime = 0LL;

    m_absTimeBase = 0LL;
    m_exactAbsTime = 0LL;
    m_exactRelTime = 0LL;

    m_absTime = 0.0f;
    m_relTime = 0.0f;

    m_baseTimeStamp = GetSystemUtils()->CreateTimeStamp();
    m_curTimeStamp = GetSystemUtils()->CreateTimeStamp();
    m_lastTimeStamp = GetSystemUtils()->CreateTimeStamp();

    for (int i = 0; i < PCNT_MAX; ++i)
    {
        m_performanceCounters[i][0] = GetSystemUtils()->CreateTimeStamp();
        m_performanceCounters[i][1] = GetSystemUtils()->CreateTimeStamp();
    }

    m_joystickEnabled = false;

    m_mouseMode = MOUSE_SYSTEM;

    m_kmodState = 0;
    m_mouseButtonsState = 0;
    m_trackedKeys = 0;

    m_dataPath = COLOBOT_DEFAULT_DATADIR;

    m_language = LANGUAGE_ENV;

    m_lowCPU = true;

    for (int i = 0; i < DIR_MAX; ++i)
        m_dataDirs[i] = nullptr;

    m_dataDirs[DIR_AI]       = "ai";
    m_dataDirs[DIR_FONT]     = "fonts";
    m_dataDirs[DIR_HELP]     = "help";
    m_dataDirs[DIR_ICON]     = "icons";
    m_dataDirs[DIR_LEVEL]    = "levels";
    m_dataDirs[DIR_MODEL]    = "models";
    m_dataDirs[DIR_MUSIC]    = "music";
    m_dataDirs[DIR_SOUND]    = "sounds";
    m_dataDirs[DIR_TEXTURE]  = "textures";
}

CApplication::~CApplication()
{
    delete m_private;
    m_private = nullptr;

    delete m_eventQueue;
    m_eventQueue = nullptr;

    delete m_profile;
    m_profile = nullptr;

    delete m_iMan;
    m_iMan = nullptr;

    GetSystemUtils()->DestroyTimeStamp(m_baseTimeStamp);
    GetSystemUtils()->DestroyTimeStamp(m_curTimeStamp);
    GetSystemUtils()->DestroyTimeStamp(m_lastTimeStamp);

    for (int i = 0; i < PCNT_MAX; ++i)
    {
        GetSystemUtils()->DestroyTimeStamp(m_performanceCounters[i][0]);
        GetSystemUtils()->DestroyTimeStamp(m_performanceCounters[i][1]);
    }
}

CEventQueue* CApplication::GetEventQueue()
{
    return m_eventQueue;
}

CSoundInterface* CApplication::GetSound()
{
    return m_sound;

    for (int i = 0; i < PCNT_MAX; ++i)
    {
        GetSystemUtils()->DestroyTimeStamp(m_performanceCounters[i][0]);
        GetSystemUtils()->DestroyTimeStamp(m_performanceCounters[i][1]);
    }
}

ParseArgsStatus CApplication::ParseArguments(int argc, char *argv[])
{
    enum OptionType
    {
        OPT_HELP = 1,
        OPT_DEBUG,
        OPT_DATADIR,
        OPT_LOGLEVEL,
        OPT_LANGUAGE,
        OPT_VBO
    };

    option options[] =
    {
        { "help", no_argument, nullptr, OPT_HELP },
        { "debug", no_argument, nullptr, OPT_DEBUG },
        { "datadir", required_argument, nullptr, OPT_DATADIR },
        { "loglevel", required_argument, nullptr, OPT_LOGLEVEL },
        { "language", required_argument, nullptr, OPT_LANGUAGE },
        { "vbo", required_argument, nullptr, OPT_VBO }
    };

    opterr = 0;

    int c = 0;
    int index = -1;
    while ((c = getopt_long_only(argc, argv, "", options, &index)) != -1)
    {
        if (c == '?')
        {
            if (optopt == 0)
                GetLogger()->Error("Invalid argument: %s\n", argv[optind-1]);
            else
                GetLogger()->Error("Expected argument for option: %s\n", argv[optind-1]);

            m_exitCode = 1;
            return PARSE_ARGS_FAIL;
        }

        index = -1;

        switch (c)
        {
            case OPT_HELP:
            {
                GetLogger()->Message("\n");
                GetLogger()->Message("Colobot %s (%s)\n", COLOBOT_CODENAME, COLOBOT_VERSION);
                GetLogger()->Message("\n");
                GetLogger()->Message("List of available options:\n");
                GetLogger()->Message("  -help            this help\n");
                GetLogger()->Message("  -debug           enable debug mode (more info printed in logs)\n");
                GetLogger()->Message("  -datadir path    set custom data directory path\n");
                GetLogger()->Message("  -loglevel level  set log level to level (one of: trace, debug, info, warn, error, none)\n");
                GetLogger()->Message("  -language lang   set language (one of: en, de, fr, pl)\n");
                GetLogger()->Message("  -vbo mode        set OpenGL VBO mode (one of: auto, enable, disable)\n");
                return PARSE_ARGS_HELP;
            }
            case OPT_DEBUG:
            {
                SetDebugMode(true);
                break;
            }
            case OPT_DATADIR:
            {
                m_dataPath = optarg;
                GetLogger()->Info("Using custom data dir: '%s'\n", m_dataPath.c_str());
                break;
            }
            case OPT_LOGLEVEL:
            {
                LogLevel logLevel;
                if (! CLogger::ParseLogLevel(optarg, logLevel))
                {
                    GetLogger()->Error("Invalid log level: \"%s\"\n", optarg);
                    return PARSE_ARGS_FAIL;
                }

                GetLogger()->Message("[*****] Log level changed to %s\n", optarg);
                GetLogger()->SetLogLevel(logLevel);
                break;
            }
            case OPT_LANGUAGE:
            {
                Language language;
                if (! ParseLanguage(optarg, language))
                {
                    GetLogger()->Error("Invalid language: \"%s\"\n", optarg);
                    return PARSE_ARGS_FAIL;
                }

                GetLogger()->Info("Using language %s\n", optarg);
                m_language = language;
                break;
            }
            case OPT_VBO:
            {
                std::string vbo;
                vbo = optarg;
                if (vbo == "auto")
                    m_deviceConfig.vboMode = Gfx::VBO_MODE_AUTO;
                else if (vbo == "enable")
                    m_deviceConfig.vboMode = Gfx::VBO_MODE_ENABLE;
                else if (vbo == "disable")
                    m_deviceConfig.vboMode = Gfx::VBO_MODE_DISABLE;
                else
                {
                    GetLogger()->Error("Invalid vbo mode: \"%s\"\n", optarg);
                    return PARSE_ARGS_FAIL;
                }

                break;
            }
            default:
                assert(false); // should never get here
        }
    }

    return PARSE_ARGS_OK;
}

bool CApplication::Create()
{
    GetLogger()->Info("Creating CApplication\n");

    boost::filesystem::path dataPath(m_dataPath);
    if (! (boost::filesystem::exists(dataPath) && boost::filesystem::is_directory(dataPath)) )
    {
        GetLogger()->Error("Data directory '%s' doesn't exist or is not a directory\n", m_dataPath.c_str());
        m_errorMessage = std::string("Could not read from data directory:\n") +
                         std::string("'") + m_dataPath + std::string("'\n") +
                         std::string("Please check your installation, or supply a valid data directory by -datadir option.");
        m_exitCode = 1;
        return false;
    }

    SetLanguage(m_language);

    //Create the sound instance.
    if (!GetProfile().InitCurrentDirectory())
    {
        GetLogger()->Warn("Config not found. Default values will be used!\n");
        m_sound = new CSoundInterface();
    }
    else
    {
        std::string path;
        if (GetProfile().GetLocalProfileString("Resources", "Data", path))
            m_dataPath = path;

        #ifdef OPENAL_SOUND
        m_sound = static_cast<CSoundInterface *>(new ALSound());
        #else
        GetLogger()->Info("No sound support.\n");
        m_sound = new CSoundInterface();
        #endif

        m_sound->Create(true);
        if (GetProfile().GetLocalProfileString("Resources", "Sound", path)) {
            m_sound->CacheAll(path);
        } else {
            m_sound->CacheAll(GetDataSubdirPath(DIR_SOUND));
        }

        if (GetProfile().GetLocalProfileString("Resources", "Music", path)) {
            m_sound->AddMusicFiles(path);
        } else {
            m_sound->AddMusicFiles(GetDataSubdirPath(DIR_MUSIC));
        }
    }

    std::string standardInfoMessage =
      "\nPlease see the console output or log file\n"
      "to get more information on the source of error";

    /* SDL initialization sequence */


    Uint32 initFlags = SDL_INIT_VIDEO | SDL_INIT_TIMER;

    if (SDL_Init(initFlags) < 0)
    {
        m_errorMessage = std::string("SDL initialization error:\n") +
                         std::string(SDL_GetError());
        GetLogger()->Error(m_errorMessage.c_str());
        m_exitCode = 2;
        return false;
    }

    // This is non-fatal and besides seems to fix some memory leaks
    if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) < 0)
    {
        GetLogger()->Warn("Joystick subsystem init failed\nJoystick(s) will not be available\n");
    }

    if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) == 0)
    {
        m_errorMessage = std::string("SDL_Image initialization error:\n") +
                         std::string(IMG_GetError());
        GetLogger()->Error(m_errorMessage.c_str());
        m_exitCode = 3;
        return false;
    }

    // load settings from profile
    int iValue;
    if ( GetProfile().GetLocalProfileInt("Setup", "Resolution", iValue) )
    {
        std::vector<Math::IntPoint> modes;
        GetVideoResolutionList(modes, true, true);
        if (static_cast<unsigned int>(iValue) < modes.size())
            m_deviceConfig.size = modes.at(iValue);
    }

    if ( GetProfile().GetLocalProfileInt("Setup", "Fullscreen", iValue) )
    {
        m_deviceConfig.fullScreen = (iValue == 1);
    }

    if (! CreateVideoSurface())
        return false; // dialog is in function

    if (m_private->surface == nullptr)
    {
        m_errorMessage = std::string("SDL error while setting video mode:\n") +
                         std::string(SDL_GetError());
        GetLogger()->Error(m_errorMessage.c_str());
        m_exitCode = 4;
        return false;
    }

    SDL_WM_SetCaption(m_windowTitle.c_str(), m_windowTitle.c_str());

    // Enable translating key codes of key press events to unicode chars
    SDL_EnableUNICODE(1);

    // Don't generate joystick events
    SDL_JoystickEventState(SDL_IGNORE);

    // The video is ready, we can create and initalize the graphics device
    m_device = new Gfx::CGLDevice(m_deviceConfig);
    if (! m_device->Create() )
    {
        m_errorMessage = std::string("Error in CDevice::Create()\n") + standardInfoMessage;
        m_exitCode = 5;
        return false;
    }

    // Create the 3D engine
    m_engine = new Gfx::CEngine(this);

    m_engine->SetDevice(m_device);

    if (! m_engine->Create() )
    {
        m_errorMessage = std::string("Error in CEngine::Init()\n") + standardInfoMessage;
        m_exitCode = 6;
        return false;
    }

    // Create model manager
    m_modelManager = new Gfx::CModelManager(m_engine);

    // Create the robot application.
    m_robotMain = new CRobotMain(this);

    m_robotMain->ChangePhase(PHASE_WELCOME1);

    GetLogger()->Info("CApplication created successfully\n");

    return true;
}

bool CApplication::CreateVideoSurface()
{
    const SDL_VideoInfo *videoInfo = SDL_GetVideoInfo();
    if (videoInfo == nullptr)
    {
        m_errorMessage = std::string("SDL error while getting video info:\n ") +
                         std::string(SDL_GetError());
        GetLogger()->Error(m_errorMessage.c_str());
        m_exitCode = 7;
        return false;
    }

    Uint32 videoFlags = SDL_OPENGL | SDL_GL_DOUBLEBUFFER | SDL_HWPALETTE;

    // Use hardware surface if available
    if (videoInfo->hw_available)
        videoFlags |= SDL_HWSURFACE;

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
    m_joystickEnabled = false;

    if (m_robotMain != nullptr)
    {
        delete m_robotMain;
        m_robotMain = nullptr;
    }

    if (m_sound != nullptr)
    {
        delete m_sound;
        m_sound = nullptr;
    }

    if (m_modelManager != nullptr)
    {
        delete m_modelManager;
        m_modelManager = nullptr;
    }

    if (m_engine != nullptr)
    {
        m_engine->Destroy();

        delete m_engine;
        m_engine = nullptr;
    }

    if (m_device != nullptr)
    {
        m_device->Destroy();

        delete m_device;
        m_device = nullptr;
    }

    if (m_private->joystick != nullptr)
    {
        SDL_JoystickClose(m_private->joystick);
        m_private->joystick = nullptr;
    }

    if (m_private->surface != nullptr)
    {
        SDL_FreeSurface(m_private->surface);
        m_private->surface = nullptr;
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

    if (m_private->surface == nullptr)
    {
        if (! restore)
        {
            std::string error = std::string("SDL error while setting video mode:\n") +
                          std::string(SDL_GetError()) + std::string("\n") +
                          std::string("Previous mode will be restored");
            GetLogger()->Error(error.c_str());
            GetSystemUtils()->SystemDialog( SDT_ERROR, "COLOBT - Error", error);

            restore = true;
            ChangeVideoConfig(m_lastDeviceConfig);
            return false;
        }
        else
        {
            restore = false;

            std::string error = std::string("SDL error while restoring previous video mode:\n") +
                          std::string(SDL_GetError());
            GetLogger()->Error(error.c_str());
            GetSystemUtils()->SystemDialog( SDT_ERROR, "COLOBT - Fatal Error", error);


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
    if (m_private->joystick == nullptr)
        return false;

    m_joystick.axisCount   = SDL_JoystickNumAxes(m_private->joystick);
    m_joystick.buttonCount = SDL_JoystickNumButtons(m_private->joystick);

    // Create the vectors with joystick axis & button states to exactly the required size
    m_joyAxeState = std::vector<int>(m_joystick.axisCount, 0);
    m_joyButtonState = std::vector<bool>(m_joystick.buttonCount, false);

    // Create a timer for polling joystick state
    m_private->joystickTimer = SDL_AddTimer(JOYSTICK_TIMER_INTERVAL, JoystickTimerCallback, nullptr);

    return true;
}

void CApplication::CloseJoystick()
{
    // Timer will remove itself automatically

    SDL_JoystickClose(m_private->joystick);
    m_private->joystick = nullptr;
}

bool CApplication::ChangeJoystick(const JoystickDevice &newJoystick)
{
    if ( (newJoystick.index < 0) || (newJoystick.index >= SDL_NumJoysticks()) )
        return false;

    if (m_private->joystick != nullptr)
        CloseJoystick();

    return OpenJoystick();
}

Uint32 JoystickTimerCallback(Uint32 interval, void *)
{
    CApplication *app = CApplication::GetInstancePointer();
    if ((app == nullptr) || (! app->GetJoystickEnabled()))
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
    m_mousePos = m_engine->WindowToInterfaceCoords(pos);
}

int CApplication::Run()
{
    m_active = true;

    GetSystemUtils()->GetCurrentTimeStamp(m_baseTimeStamp);
    GetSystemUtils()->GetCurrentTimeStamp(m_lastTimeStamp);
    GetSystemUtils()->GetCurrentTimeStamp(m_curTimeStamp);

    MoveMouse(Math::Point(0.5f, 0.5f)); // center mouse on start

    while (true)
    {
        ResetPerformanceCounters();

        if (m_active)
        {
            StartPerformanceCounter(PCNT_ALL);
            StartPerformanceCounter(PCNT_EVENT_PROCESSING);
        }

        // To be sure no old event remains
        m_private->currentEvent.type = SDL_NOEVENT;

        // Call SDL_PumpEvents() only once here
        // (SDL_PeepEvents() doesn't call it)
        if (m_active)
            SDL_PumpEvents();

        m_private->lastMouseMotionEvent.type = SDL_NOEVENT;

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

                // Skip mouse motion events, for now
                if (m_private->currentEvent.type == SDL_MOUSEMOTION)
                {
                    m_private->lastMouseMotionEvent = m_private->currentEvent;
                    continue;
                }

                Event event = ProcessSystemEvent();

                if (event.type == EVENT_QUIT)
                    goto end; // exit the loop

                if (event.type != EVENT_NULL)
                {
                    bool passOn = ProcessEvent(event);

                    if (m_engine != nullptr && passOn)
                        passOn = m_engine->ProcessEvent(event);

                    if (passOn)
                        m_eventQueue->AddEvent(event);
                }

                Event virtualEvent = CreateVirtualEvent(event);
                if (virtualEvent.type != EVENT_NULL)
                {
                    bool passOn = ProcessEvent(virtualEvent);

                    if (m_engine != nullptr && passOn)
                        passOn = m_engine->ProcessEvent(virtualEvent);

                    if (passOn)
                        m_eventQueue->AddEvent(virtualEvent);
                }
            }
        }

        // Now, process the last received mouse motion
        if (m_private->lastMouseMotionEvent.type != SDL_NOEVENT)
        {
            m_private->currentEvent = m_private->lastMouseMotionEvent;

            Event event = ProcessSystemEvent();

            if (event.type == EVENT_QUIT)
                goto end; // exit the loop

            if (event.type != EVENT_NULL)
            {
                bool passOn = ProcessEvent(event);

                if (m_engine != nullptr && passOn)
                    passOn = m_engine->ProcessEvent(event);

                if (passOn)
                    m_eventQueue->AddEvent(event);
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

                    if (passOn && m_engine != nullptr)
                        passOn = m_engine->ProcessEvent(event);
                }

                if (passOn && m_robotMain != nullptr)
                    m_robotMain->EventProcess(event);
            }

            StopPerformanceCounter(PCNT_EVENT_PROCESSING);

            StartPerformanceCounter(PCNT_UPDATE_ALL);

            // Prepare and process step simulation event
            event = CreateUpdateEvent();
            if (event.type != EVENT_NULL && m_robotMain != nullptr)
            {
                StartPerformanceCounter(PCNT_UPDATE_ENGINE);
                m_engine->FrameUpdate();
                StopPerformanceCounter(PCNT_UPDATE_ENGINE);

                m_sound->FrameMove(m_relTime);

                StartPerformanceCounter(PCNT_UPDATE_GAME);
                m_robotMain->EventProcess(event);
                StopPerformanceCounter(PCNT_UPDATE_GAME);
            }

            StopPerformanceCounter(PCNT_UPDATE_ALL);

            /* Update mouse position explicitly right before rendering
             * because mouse events are usually way behind */
            UpdateMouse();

            StartPerformanceCounter(PCNT_RENDER_ALL);
            Render();
            StopPerformanceCounter(PCNT_RENDER_ALL);

            StopPerformanceCounter(PCNT_ALL);

            UpdatePerformanceCountersData();

            if (m_lowCPU)
            {
                usleep(20000); // should still give plenty of fps
            }
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

const std::string& CApplication::GetErrorMessage()
{
    return m_errorMessage;
}

/** The SDL event parsed is stored internally.
    If event is not available or is not understood, returned event is of type EVENT_NULL. */
Event CApplication::ProcessSystemEvent()
{
    Event event;
    event.systemEvent = true;

    if (m_private->currentEvent.type == SDL_QUIT)
    {
        event.type = EVENT_QUIT;
    }
    else if (m_private->currentEvent.type == SDL_VIDEORESIZE)
    {
        Gfx::GLDeviceConfig newConfig = m_deviceConfig;
        newConfig.size.x = m_private->currentEvent.resize.w;
        newConfig.size.y = m_private->currentEvent.resize.h;
        if (newConfig.size != m_deviceConfig.size)
            ChangeVideoConfig(newConfig);
    }
    else if ( (m_private->currentEvent.type == SDL_KEYDOWN) ||
              (m_private->currentEvent.type == SDL_KEYUP) )
    {
        if (m_private->currentEvent.type == SDL_KEYDOWN)
            event.type = EVENT_KEY_DOWN;
        else
            event.type = EVENT_KEY_UP;

        event.key.virt = false;
        event.key.key = m_private->currentEvent.key.keysym.sym;
        event.key.unicode = m_private->currentEvent.key.keysym.unicode;

        // Use the occasion to update kmods
        m_kmodState = m_private->currentEvent.key.keysym.mod;
    }
    else if ( (m_private->currentEvent.type == SDL_MOUSEBUTTONDOWN) ||
         (m_private->currentEvent.type == SDL_MOUSEBUTTONUP) )
    {
        if ((m_private->currentEvent.button.button == SDL_BUTTON_WHEELUP) ||
            (m_private->currentEvent.button.button == SDL_BUTTON_WHEELDOWN))
        {
            if (m_private->currentEvent.type == SDL_MOUSEBUTTONDOWN) // ignore the following up event
            {
                event.type = EVENT_MOUSE_WHEEL;
                if (m_private->currentEvent.button.button == SDL_BUTTON_WHEELDOWN)
                    event.mouseWheel.dir = WHEEL_DOWN;
                else
                    event.mouseWheel.dir = WHEEL_UP;
            }
        }
        else
        {
            if (m_private->currentEvent.type == SDL_MOUSEBUTTONDOWN)
                event.type = EVENT_MOUSE_BUTTON_DOWN;
            else
                event.type = EVENT_MOUSE_BUTTON_UP;

            event.mouseButton.button = static_cast<MouseButton>(1 << m_private->currentEvent.button.button);

            // Use the occasion to update mouse button state
            if (m_private->currentEvent.type == SDL_MOUSEBUTTONDOWN)
                m_mouseButtonsState |= event.mouseButton.button;
            else
                m_mouseButtonsState &= ~event.mouseButton.button;
        }

        // Use the occasion to update mouse pos
        m_mousePos = m_engine->WindowToInterfaceCoords(
            Math::IntPoint(m_private->currentEvent.button.x, m_private->currentEvent.button.y));
    }
    else if (m_private->currentEvent.type == SDL_MOUSEMOTION)
    {
        event.type = EVENT_MOUSE_MOVE;

        m_mousePos = m_engine->WindowToInterfaceCoords(
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


    if (event.type == EVENT_KEY_DOWN)
    {
        if      (event.key.key == KEY(KP8))
            m_trackedKeys |= TRKEY_NUM_UP;
        else if (event.key.key == KEY(KP2))
            m_trackedKeys |= TRKEY_NUM_DOWN;
        else if (event.key.key == KEY(KP4))
            m_trackedKeys |= TRKEY_NUM_LEFT;
        else if (event.key.key == KEY(KP6))
            m_trackedKeys |= TRKEY_NUM_RIGHT;
        else if (event.key.key == KEY(KP_PLUS))
            m_trackedKeys |= TRKEY_NUM_PLUS;
        else if (event.key.key == KEY(KP_MINUS))
            m_trackedKeys |= TRKEY_NUM_MINUS;
        else if (event.key.key == KEY(PAGEUP))
            m_trackedKeys |= TRKEY_PAGE_UP;
        else if (event.key.key == KEY(PAGEDOWN))
            m_trackedKeys |= TRKEY_PAGE_DOWN;
    }
    else if (event.type == EVENT_KEY_UP)
    {
        if      (event.key.key == KEY(KP8))
            m_trackedKeys &= ~TRKEY_NUM_UP;
        else if (event.key.key == KEY(KP2))
            m_trackedKeys &= ~TRKEY_NUM_DOWN;
        else if (event.key.key == KEY(KP4))
            m_trackedKeys &= ~TRKEY_NUM_LEFT;
        else if (event.key.key == KEY(KP6))
            m_trackedKeys &= ~TRKEY_NUM_RIGHT;
        else if (event.key.key == KEY(KP_PLUS))
            m_trackedKeys &= ~TRKEY_NUM_PLUS;
        else if (event.key.key == KEY(KP_MINUS))
            m_trackedKeys &= ~TRKEY_NUM_MINUS;
        else if (event.key.key == KEY(PAGEUP))
            m_trackedKeys &= ~TRKEY_PAGE_UP;
        else if (event.key.key == KEY(PAGEDOWN))
            m_trackedKeys &= ~TRKEY_PAGE_DOWN;
    }

    event.trackedKeysState = m_trackedKeys;
    event.kmodState = m_kmodState;
    event.mousePos = m_mousePos;
    event.mouseButtonsState = m_mouseButtonsState;

    return event;
}

/**
 * Processes incoming events. It is the first function called after an event is captured.
 * Event is modified, updating its tracked keys state and mouse position to current values.
 * Function returns \c true if the event is to be passed on to other processing functions
 * or \c false if not. */
bool CApplication::ProcessEvent(const Event &event)
{
    CLogger *l = GetLogger();

    // Print the events in debug mode to test the code
    if (m_debugMode)
    {
        switch (event.type)
        {
            case EVENT_KEY_DOWN:
            case EVENT_KEY_UP:
                l->Trace("EVENT_KEY_%s:\n", (event.type == EVENT_KEY_DOWN) ? "DOWN" : "UP");
                l->Trace(" virt    = %s\n", (event.key.virt) ? "true" : "false");
                l->Trace(" key     = %d\n", event.key.key);
                l->Trace(" unicode = 0x%04x\n", event.key.unicode);
                break;
            case EVENT_MOUSE_MOVE:
                l->Trace("EVENT_MOUSE_MOVE:\n");
                break;
            case EVENT_MOUSE_BUTTON_DOWN:
            case EVENT_MOUSE_BUTTON_UP:
                l->Trace("EVENT_MOUSE_BUTTON_%s:\n", (event.type == EVENT_MOUSE_BUTTON_DOWN) ? "DOWN" : "UP");
                l->Trace(" button = %d\n", event.mouseButton.button);
                break;
            case EVENT_MOUSE_WHEEL:
                l->Trace("EVENT_MOUSE_WHEEL:\n");
                l->Trace(" dir = %s\n", (event.mouseWheel.dir == WHEEL_DOWN) ? "WHEEL_DOWN" : "WHEEL_UP");
               break;
            case EVENT_JOY_AXIS:
                l->Trace("EVENT_JOY_AXIS:\n");
                l->Trace(" axis  = %d\n", event.joyAxis.axis);
                l->Trace(" value = %d\n", event.joyAxis.value);
                break;
            case EVENT_JOY_BUTTON_DOWN:
            case EVENT_JOY_BUTTON_UP:
                l->Trace("EVENT_JOY_BUTTON_%s:\n", (event.type == EVENT_JOY_BUTTON_DOWN) ? "DOWN" : "UP");
                l->Trace(" button = %d\n", event.joyButton.button);
                break;
            case EVENT_ACTIVE:
                l->Trace("EVENT_ACTIVE:\n");
                l->Trace(" flags = 0x%x\n", event.active.flags);
                l->Trace(" gain  = %s\n", event.active.gain ? "true" : "false");
                break;
            default:
                l->Trace("Event type = %d:\n", static_cast<int>(event.type));
                break;
        }

        l->Trace(" systemEvent = %s\n", event.systemEvent ? "true" : "false");
        l->Trace(" rTime = %f\n", event.rTime);
        l->Trace(" kmodState = %04x\n", event.kmodState);
        l->Trace(" trackedKeysState = %04x\n", event.trackedKeysState);
        l->Trace(" mousePos = %f, %f\n", event.mousePos.x, event.mousePos.y);
        l->Trace(" mouseButtonsState = %02x\n", event.mouseButtonsState);
    }

    // By default, pass on all events
    return true;
}


Event CApplication::CreateVirtualEvent(const Event& sourceEvent)
{
    Event virtualEvent;
    virtualEvent.systemEvent = true;

    if ((sourceEvent.type == EVENT_KEY_DOWN) || (sourceEvent.type == EVENT_KEY_UP))
    {
        virtualEvent.type = sourceEvent.type;
        virtualEvent.key = sourceEvent.key;
        virtualEvent.key.virt = true;

        if (sourceEvent.key.key == KEY(LCTRL) || sourceEvent.key.key == KEY(RCTRL))
            virtualEvent.key.key = VIRTUAL_KMOD(CTRL);
        else if (sourceEvent.key.key == KEY(LSHIFT) || sourceEvent.key.key == KEY(RSHIFT))
            virtualEvent.key.key = VIRTUAL_KMOD(SHIFT);
        else if (sourceEvent.key.key == KEY(LALT) || sourceEvent.key.key == KEY(RALT))
            virtualEvent.key.key = VIRTUAL_KMOD(ALT);
        else if (sourceEvent.key.key == KEY(LMETA) || sourceEvent.key.key == KEY(RMETA))
            virtualEvent.key.key = VIRTUAL_KMOD(META);
        else
            virtualEvent.type = EVENT_NULL;
    }
    else if ((sourceEvent.type == EVENT_JOY_BUTTON_DOWN) || (sourceEvent.type == EVENT_JOY_BUTTON_UP))
    {
        if (sourceEvent.type == EVENT_JOY_BUTTON_DOWN)
            virtualEvent.type = EVENT_KEY_DOWN;
        else
            virtualEvent.type = EVENT_KEY_UP;

        virtualEvent.key.virt = true;
        virtualEvent.key.key = VIRTUAL_JOY(sourceEvent.joyButton.button);
        virtualEvent.key.unicode = 0;
    }
    else
    {
        virtualEvent.type = EVENT_NULL;
    }

    return virtualEvent;
}

/** Renders the frame and swaps buffers as necessary */
void CApplication::Render()
{
    m_engine->Render();

    if (m_deviceConfig.doubleBuf)
        SDL_GL_SwapBuffers();
}

void CApplication::SuspendSimulation()
{
    m_simulationSuspended = true;
    GetLogger()->Info("Suspend simulation\n");
}

void CApplication::ResumeSimulation()
{
    m_simulationSuspended = false;

    GetSystemUtils()->GetCurrentTimeStamp(m_baseTimeStamp);
    GetSystemUtils()->CopyTimeStamp(m_curTimeStamp, m_baseTimeStamp);
    m_realAbsTimeBase = m_realAbsTime;
    m_absTimeBase = m_exactAbsTime;

    GetLogger()->Info("Resume simulation\n");
}

bool CApplication::GetSimulationSuspended()
{
    return m_simulationSuspended;
}

void CApplication::SetSimulationSpeed(float speed)
{
    m_simulationSpeed = speed;

    GetSystemUtils()->GetCurrentTimeStamp(m_baseTimeStamp);
    m_realAbsTimeBase = m_realAbsTime;
    m_absTimeBase = m_exactAbsTime;

    GetLogger()->Info("Simulation speed = %.2f\n", speed);
}

Event CApplication::CreateUpdateEvent()
{
    if (m_simulationSuspended)
        return Event(EVENT_NULL);

    GetSystemUtils()->CopyTimeStamp(m_lastTimeStamp, m_curTimeStamp);
    GetSystemUtils()->GetCurrentTimeStamp(m_curTimeStamp);

    long long absDiff = GetSystemUtils()->TimeStampExactDiff(m_baseTimeStamp, m_curTimeStamp);
    long long newRealAbsTime = m_realAbsTimeBase + absDiff;
    long long newRealRelTime = GetSystemUtils()->TimeStampExactDiff(m_lastTimeStamp, m_curTimeStamp);

    if (newRealAbsTime < m_realAbsTime || newRealRelTime < 0)
    {
        GetLogger()->Error("Fatal error: got negative system counter difference!\n");
        GetLogger()->Error("This should never happen. Please report this error.\n");
        m_eventQueue->AddEvent(Event(EVENT_QUIT));
        return Event(EVENT_NULL);
    }
    else
    {
        m_realAbsTime = newRealAbsTime;
        // m_baseTimeStamp is updated on simulation speed change, so this is OK
        m_exactAbsTime = m_absTimeBase + m_simulationSpeed * absDiff;
        m_absTime = (m_absTimeBase + m_simulationSpeed * absDiff) / 1e9f;

        m_realRelTime = newRealRelTime;
        m_exactRelTime = m_simulationSpeed * m_realRelTime;
        m_relTime = (m_simulationSpeed * m_realRelTime) / 1e9f;
    }

    Event frameEvent(EVENT_FRAME);
    frameEvent.systemEvent = true;
    frameEvent.trackedKeysState = m_trackedKeys;
    frameEvent.kmodState = m_kmodState;
    frameEvent.mousePos = m_mousePos;
    frameEvent.mouseButtonsState = m_mouseButtonsState;
    frameEvent.rTime = m_relTime;

    return frameEvent;
}

float CApplication::GetSimulationSpeed()
{
    return m_simulationSpeed;
}

float CApplication::GetAbsTime()
{
    return m_absTime;
}

long long CApplication::GetExactAbsTime()
{
    return m_exactAbsTime;
}

long long CApplication::GetRealAbsTime()
{
    return m_realAbsTime;
}

float CApplication::GetRelTime()
{
    return m_relTime;
}

long long CApplication::GetExactRelTime()
{
    return m_exactRelTime;
}

long long CApplication::GetRealRelTime()
{
    return m_realRelTime;
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
    if (videoInfo == nullptr)
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

int CApplication::GetKmods()
{
    return m_kmodState;
}

bool CApplication::GetKmodState(int kmod)
{
    return (m_kmodState & kmod) != 0;
}

bool CApplication::GetTrackedKeyState(TrackedKey key)
{
    return (m_trackedKeys & key) != 0;
}

bool CApplication::GetMouseButtonState(int index)
{
    return (m_mouseButtonsState & (1<<index)) != 0;
}

void CApplication::ResetKeyStates()
{
    GetLogger()->Info("Reset key states\n");
    m_trackedKeys = 0;
    m_kmodState = 0;
    m_robotMain->ResetKeyStates();
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

void CApplication::SetMouseMode(MouseMode mode)
{
    m_mouseMode = mode;
    if ((m_mouseMode == MOUSE_SYSTEM) || (m_mouseMode == MOUSE_BOTH))
        SDL_ShowCursor(SDL_ENABLE);
    else
        SDL_ShowCursor(SDL_DISABLE);
}

MouseMode CApplication::GetMouseMode()
{
    return m_mouseMode;
}

Math::Point CApplication::GetMousePos()
{
    return m_mousePos;
}

void CApplication::MoveMouse(Math::Point pos)
{
    m_mousePos = pos;

    Math::IntPoint windowPos = m_engine->InterfaceToWindowCoords(pos);
    SDL_WarpMouse(windowPos.x, windowPos.y);
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

std::string CApplication::GetDataDirPath()
{
    return m_dataPath;
}

std::string CApplication::GetDataSubdirPath(DataDir stdDir)
{
    int index = static_cast<int>(stdDir);
    assert(index >= 0 && index < DIR_MAX);
    std::stringstream str;
    str << m_dataPath;
    str << "/";
    str << m_dataDirs[index];
    return str.str();
}

std::string CApplication::GetDataFilePath(DataDir stdDir, const std::string& subpath)
{
    int index = static_cast<int>(stdDir);
    assert(index >= 0 && index < DIR_MAX);
    std::stringstream str;
    str << m_dataPath;
    str << "/";
    str << m_dataDirs[index];
    str << "/";
    str << subpath;
    return str.str();
}

Language CApplication::GetLanguage()
{
    return m_language;
}

char CApplication::GetLanguageChar()
{
    char langChar = 'E';
    switch (m_language)
    {
        default:
        case LANGUAGE_ENV:
        case LANGUAGE_ENGLISH:
            langChar = 'E';
            break;

        case LANGUAGE_GERMAN:
            langChar = 'D';
            break;

        case LANGUAGE_FRENCH:
            langChar = 'F';
            break;

        case LANGUAGE_POLISH:
            langChar = 'P';
            break;
    }
    return langChar;
}

bool CApplication::ParseLanguage(const std::string& str, Language& language)
{
    if (str == "en")
    {
        language = LANGUAGE_ENGLISH;
        return true;
    }
    else if (str == "de")
    {
        language = LANGUAGE_GERMAN;
        return true;
    }
    else if (str == "fr")
    {
        language = LANGUAGE_FRENCH;
        return true;
    }
    else if (str == "pl")
    {
        language = LANGUAGE_POLISH;
        return true;
    }

    return false;
}

void CApplication::SetLanguage(Language language)
{
    m_language = language;

    /* Gettext initialization */

    std::string locale = "";
    switch (m_language)
    {
        default:
        case LANGUAGE_ENV:
            locale = "";
            break;

        case LANGUAGE_ENGLISH:
            locale = "en_US.utf8";
            break;

        case LANGUAGE_GERMAN:
            locale = "de_DE.utf8";
            break;

        case LANGUAGE_FRENCH:
            locale = "fr_FR.utf8";
            break;

        case LANGUAGE_POLISH:
            locale = "pl_PL.utf8";
            break;
    }

    if (locale.empty())
    {
        char *envLang = getenv("LANGUAGE");
        if (envLang == NULL)
        {
            envLang = getenv("LANG");
        }
        if (envLang == NULL)
        {
            GetLogger()->Error("Failed to get language from environment, setting default language");
            m_language = LANGUAGE_ENGLISH;
        }
        else if (strncmp(envLang,"en",2) == 0)
        {
           m_language = LANGUAGE_ENGLISH;
        }
        else if (strncmp(envLang,"de",2) == 0)
        {
           m_language = LANGUAGE_GERMAN;
        }
        else if (strncmp(envLang,"fr",2) == 0)
        {
           m_language = LANGUAGE_FRENCH;
        }
        else if (strncmp(envLang,"pl",2) == 0)
        {
           m_language = LANGUAGE_POLISH;
        }
        GetLogger()->Trace("SetLanguage: Inherit LANGUAGE=%s from environment\n", envLang);
    }
    else
    {
        std::string langStr = "LANGUAGE=";
        langStr += locale;
        strcpy(S_LANGUAGE, langStr.c_str());
        putenv(S_LANGUAGE);
        GetLogger()->Trace("SetLanguage: Set LANGUAGE=%s in environment\n", locale.c_str());
    }
    setlocale(LC_ALL, "");

    bindtextdomain("colobot", COLOBOT_I18N_DIR);
    bind_textdomain_codeset("colobot", "UTF-8");
    textdomain("colobot");

    GetLogger()->Debug("SetLanguage: Test gettext translation: '%s'\n", gettext("Colobot rules!"));
}

void CApplication::SetLowCPU(bool low)
{
    m_lowCPU = low;
}

bool CApplication::GetLowCPU()
{
    return m_lowCPU;
}

void CApplication::StartPerformanceCounter(PerformanceCounter counter)
{
    GetSystemUtils()->GetCurrentTimeStamp(m_performanceCounters[counter][0]);
}

void CApplication::StopPerformanceCounter(PerformanceCounter counter)
{
    GetSystemUtils()->GetCurrentTimeStamp(m_performanceCounters[counter][1]);
}

float CApplication::GetPerformanceCounterData(PerformanceCounter counter)
{
    return m_performanceCountersData[counter];
}

void CApplication::ResetPerformanceCounters()
{
    for (int i = 0; i < PCNT_MAX; ++i)
    {
        StartPerformanceCounter(static_cast<PerformanceCounter>(i));
        StopPerformanceCounter(static_cast<PerformanceCounter>(i));
    }
}

void CApplication::UpdatePerformanceCountersData()
{
    long long sum = GetSystemUtils()->TimeStampExactDiff(m_performanceCounters[PCNT_ALL][0],
                                                         m_performanceCounters[PCNT_ALL][1]);

    for (int i = 0; i < PCNT_MAX; ++i)
    {
        long long diff = GetSystemUtils()->TimeStampExactDiff(m_performanceCounters[i][0],
                                                              m_performanceCounters[i][1]);

        m_performanceCountersData[static_cast<PerformanceCounter>(i)] =
            static_cast<float>(diff) / static_cast<float>(sum);
    }
}

