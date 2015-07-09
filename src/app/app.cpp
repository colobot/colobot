/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2014, Daniel Roux, EPSITEC SA & TerranovaTeam
 * http://epsiteс.ch; http://colobot.info; http://github.com/colobot
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://gnu.org/licenses
 */

#include "common/config.h"

#include "app/app.h"

#include "app/controller.h"
#include "app/input.h"
#include "app/system.h"

#include "common/logger.h"
#include "common/image.h"
#include "common/key.h"
#include "common/pathman.h"
#include "common/stringutils.h"
#include "common/resources/resourcemanager.h"

#include "graphics/core/nulldevice.h"
#include "graphics/opengl/glutil.h"

#include "object/robotmain.h"
#include "object/object_manager.h"

#ifdef OPENAL_SOUND
    #include "sound/oalsound/alsound.h"
#endif

#include <boost/filesystem.hpp>
#include <boost/tokenizer.hpp>

#include <SDL.h>
#include <SDL_image.h>

#include <stdlib.h>
#include <libintl.h>
#include <getopt.h>
#include <localename.h>


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
 : m_private(new ApplicationPrivate())
 , m_eventQueue(new CEventQueue())
 , m_profile(new CProfile())
 , m_input(new CInput())
 , m_pathManager(new CPathManager())
{
    m_exitCode      = 0;
    m_active        = false;
    m_debugModes    = 0;
    m_restart       = false;

    m_windowTitle = "Colobot: Gold Edition";

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

    m_runSceneName = "";
    m_runSceneRank = 0;

    m_sceneTest = false;
    m_headless = false;
    m_resolutionOverride = false;

    m_language = LANGUAGE_ENV;

    m_lowCPU = true;

    m_graphics = "opengl";
}

CApplication::~CApplication()
{
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
    return m_eventQueue.get();
}

CSoundInterface* CApplication::GetSound()
{
    return m_sound.get();
}

ParseArgsStatus CApplication::ParseArguments(int argc, char *argv[])
{
    enum OptionType
    {
        OPT_HELP = 1,
        OPT_DEBUG,
        OPT_RUNSCENE,
        OPT_SCENETEST,
        OPT_LOGLEVEL,
        OPT_LANGUAGE,
        OPT_LANGDIR,
        OPT_DATADIR,
        OPT_SAVEDIR,
        OPT_MOD,
        OPT_RESOLUTION,
        OPT_HEADLESS,
        OPT_DEVICE
    };

    option options[] =
    {
        { "help", no_argument, nullptr, OPT_HELP },
        { "debug", required_argument, nullptr, OPT_DEBUG },
        { "runscene", required_argument, nullptr, OPT_RUNSCENE },
        { "scenetest", no_argument, nullptr, OPT_SCENETEST },
        { "loglevel", required_argument, nullptr, OPT_LOGLEVEL },
        { "language", required_argument, nullptr, OPT_LANGUAGE },
        { "langdir", required_argument, nullptr, OPT_LANGDIR },
        { "datadir", required_argument, nullptr, OPT_DATADIR },
        { "savedir", required_argument, nullptr, OPT_SAVEDIR },
        { "mod", required_argument, nullptr, OPT_MOD },
        { "resolution", required_argument, nullptr, OPT_RESOLUTION },
        { "headless", no_argument, nullptr, OPT_HEADLESS },
        { "graphics", required_argument, nullptr, OPT_DEVICE },
        { nullptr, 0, nullptr, 0}
    };

    opterr = 0;
    optind = 1;

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
                GetLogger()->Message("%s\n", COLOBOT_FULLNAME);
                GetLogger()->Message("\n");
                GetLogger()->Message("List of available options:\n");
                GetLogger()->Message("  -help               this help\n");
                GetLogger()->Message("  -debug modes        enable debug modes (more info printed in logs; see code for reference of modes)\n");
                GetLogger()->Message("  -runscene sceneNNN  run given scene on start\n");
                GetLogger()->Message("  -scenetest          win every mission right after it's loaded\n");
                GetLogger()->Message("  -loglevel level     set log level to level (one of: trace, debug, info, warn, error, none)\n");
                GetLogger()->Message("  -language lang      set language (one of: en, de, fr, pl, ru)\n");
                GetLogger()->Message("  -langdir path       set custom language directory path\n");
                GetLogger()->Message("  -datadir path       set custom data directory path\n");
                GetLogger()->Message("  -savedir path       set custom save directory path (must be writable)\n");
                GetLogger()->Message("  -mod path           load datadir mod from given path\n");
                GetLogger()->Message("  -resolution WxH     set resolution\n");
                GetLogger()->Message("  -headless           headless mode - disables graphics, sound and user interaction\n");
                GetLogger()->Message("  -graphics           changes graphics device (defaults to opengl)\n");
                return PARSE_ARGS_HELP;
            }
            case OPT_DEBUG:
            {
                if (optarg == nullptr)
                {
                    m_debugModes = DEBUG_ALL;
                    GetLogger()->Info("All debug modes active\n");
                }
                else
                {
                    int debugModes;
                    if (! ParseDebugModes(optarg, debugModes))
                    {
                        return PARSE_ARGS_FAIL;
                    }

                    m_debugModes = debugModes;
                    GetLogger()->Info("Active debug modes: %s\n", optarg);
                }
                break;
            }
            case OPT_RUNSCENE:
            {
                std::string file = optarg;
                m_runSceneName = file.substr(0, file.size()-3);
                m_runSceneRank = StrUtils::FromString<int>(file.substr(file.size()-3, 3));
                GetLogger()->Info("Running scene '%s%d' on start\n", m_runSceneName.c_str(), m_runSceneRank);
                break;
            }
            case OPT_SCENETEST:
            {
                m_sceneTest = true;
                break;
            }
            case OPT_LOGLEVEL:
            {
                LogLevel logLevel;
                if (! CLogger::ParseLogLevel(optarg, logLevel))
                {
                    GetLogger()->Error("Invalid log level: '%s'\n", optarg);
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
                    GetLogger()->Error("Invalid language: '%s'\n", optarg);
                    return PARSE_ARGS_FAIL;
                }

                GetLogger()->Info("Using language %s\n", optarg);
                m_language = language;
                break;
            }
            case OPT_DATADIR:
            {
                m_pathManager->SetDataPath(optarg);
                GetLogger()->Info("Using data dir: '%s'\n", optarg);
                break;
            }
            case OPT_LANGDIR:
            {
                m_pathManager->SetLangPath(optarg);
                GetLogger()->Info("Using language dir: '%s'\n", optarg);
                break;
            }
            case OPT_SAVEDIR:
            {
                m_pathManager->SetSavePath(optarg);
                GetLogger()->Info("Using save dir: '%s'\n", optarg);
                break;
            }
            case OPT_MOD:
            {
                m_pathManager->AddMod(optarg);
                break;
            }
            case OPT_RESOLUTION:
            {
                std::istringstream resolution(optarg);
                std::string w, h;
                std::getline(resolution, w, 'x');
                std::getline(resolution, h, 'x');

                m_deviceConfig.size.x = atoi(w.c_str());
                m_deviceConfig.size.y = atoi(h.c_str());
                m_resolutionOverride = true;
                break;
            }
            case OPT_HEADLESS:
            {
                m_headless = true;
                break;
            }
            case OPT_DEVICE:
            {
                m_graphics = optarg;
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
    std::string path;
    bool defaultValues = false;

    GetLogger()->Info("Creating CApplication\n");

    m_errorMessage = m_pathManager->VerifyPaths();
    if (!m_errorMessage.empty())
    {
        m_exitCode = 1;
        return false;
    }
    m_pathManager->InitPaths();

    if (!GetProfile().Init())
    {
        GetLogger()->Warn("Config not found. Default values will be used!\n");
        defaultValues = true;
    }

    if (GetProfile().GetStringProperty("Language", "Lang", path)) {
        Language language;
        if (ParseLanguage(path, language)) {
            m_language = language;
            GetLogger()->Info("Setting language '%s' from ini file\n", path.c_str());
        } else {
            GetLogger()->Error("Invalid language '%s' in ini file\n", path.c_str());
        }
    }

    SetLanguage(m_language);

    //Create the sound instance.
    #ifdef OPENAL_SOUND
    if (!m_headless)
    {
        m_sound.reset(new ALSound());
    }
    else
    {
        m_sound.reset(new CSoundInterface());
    }
    #else
    GetLogger()->Info("No sound support.\n");
    m_sound = new CSoundInterface();
    #endif

    m_sound->Create();
    m_sound->CacheAll();
    m_sound->AddMusicFiles();

    GetLogger()->Info("CApplication created successfully\n");

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

    if (!m_headless)
    {
        // load settings from profile
        int iValue;
        std::string sValue;

        // GetVideoResolutionList() has to be called here because it is responsible
        // for list of resolutions in options menu, not calling it results in empty list
        std::vector<Math::IntPoint> modes;
        GetVideoResolutionList(modes, true, true);

        if ( GetProfile().GetStringProperty("Setup", "Resolution", sValue) && !m_resolutionOverride )
        {
            std::istringstream resolution(sValue);
            std::string ws, hs;
            std::getline(resolution, ws, 'x');
            std::getline(resolution, hs, 'x');
            int w = 800, h = 600;
            if (!ws.empty() && !hs.empty()) {
                w = atoi(ws.c_str());
                h = atoi(hs.c_str());
            }

            // Why not just set m_deviceConfig.size to w,h? Because this way if the resolution is no longer supported (e.g. changimg monitor) defaults will be used instead
            for(auto it = modes.begin(); it != modes.end(); ++it) {
                if (it->x == w && it->y == h) {
                    m_deviceConfig.size = *it;
                    break;
                }
            }
        }

        if ( GetProfile().GetIntProperty("Setup", "Fullscreen", iValue) && !m_resolutionOverride )
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
    }

    // Enable translating key codes of key press events to unicode chars
    SDL_EnableUNICODE(1);
    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

    // Don't generate joystick events
    SDL_JoystickEventState(SDL_IGNORE);

    // Report joystick list to log, since we still don't have a GUI for them so you have to set the ID manually in the config
    auto joysticks = GetJoystickList();
    bool first = true;
    for (const auto& joystick : joysticks)
    {
        if (first)
        {
            ChangeJoystick(joystick);
            first = false;
        }
        GetLogger()->Info("Detected joystick: %s [ID %d]\n", joystick.name.c_str(), joystick.index);
    }
    if (first)
    {
        GetLogger()->Info("No joysticks detected\n");
    }

    if (!m_headless)
    {
        m_device = Gfx::CreateDevice(m_deviceConfig, m_graphics.c_str());

        if (m_device == nullptr)
        {
            m_device.reset(new Gfx::CNullDevice());
            GetLogger()->Error("Unknown graphics device: %s\n", m_graphics.c_str());
        }
    }
    else
    {
        m_device.reset(new Gfx::CNullDevice());
    }

    if (! m_device->Create() )
    {
        m_errorMessage = std::string("Error in CDevice::Create()\n") + standardInfoMessage;
        m_exitCode = 5;
        return false;
    }

    // Create the 3D engine
    m_engine.reset(new Gfx::CEngine(this));

    m_engine->SetDevice(m_device.get());

    if (! m_engine->Create() )
    {
        m_errorMessage = std::string("Error in CEngine::Init()\n") + standardInfoMessage;
        m_exitCode = 6;
        return false;
    }

    // Create the robot application.
    m_controller.reset(new CController(this, !defaultValues));

    if (m_runSceneName.empty())
        m_controller->StartApp();
    else
    {
        m_controller->GetRobotMain()->ChangePhase(PHASE_USER); // To load userlevel list - TODO: this is ugly
        m_controller->GetRobotMain()->SetExitAfterMission(true);
        m_controller->StartGame(m_runSceneName, m_runSceneRank/100, m_runSceneRank%100);
    }

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

    //if (m_deviceConfig.resizeable)
    //    videoFlags |= SDL_RESIZABLE;

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

    m_controller.reset();
    m_sound.reset();

    if (m_engine != nullptr)
    {
        m_engine->Destroy();

        m_engine.reset();
    }

    if (m_device != nullptr)
    {
        m_device->Destroy();

        m_device.reset();
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

void CApplication::Restart()
{
    m_restart = true;
    m_eventQueue->AddEvent(Event(EVENT_SYS_QUIT));
}

bool CApplication::IsRestarting()
{
    return m_restart;
}

bool CApplication::ChangeVideoConfig(const Gfx::DeviceConfig &newConfig)
{
    static bool restore = false;

    m_lastDeviceConfig = m_deviceConfig;
    m_deviceConfig = newConfig;


    SDL_FreeSurface(m_private->surface);

    if (! CreateVideoSurface())
    {
        // Fatal error, so post the quit event
        m_eventQueue->AddEvent(Event(EVENT_SYS_QUIT));
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
            GetSystemUtils()->SystemDialog( SDT_ERROR, "COLOBOT - Error", error);

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
            GetSystemUtils()->SystemDialog( SDT_ERROR, "COLOBOT - Fatal Error", error);


            // Fatal error, so post the quit event
            m_eventQueue->AddEvent(Event(EVENT_SYS_QUIT));
            return false;
        }
    }

    m_device->ConfigChanged(m_deviceConfig);

    m_engine->ResetAfterDeviceChanged();
    m_controller->GetRobotMain()->ResetAfterDeviceChanged();

    return true;
}

bool CApplication::OpenJoystick()
{
    if ( (m_joystick.index < 0) || (m_joystick.index >= SDL_NumJoysticks()) )
        return false;

    GetLogger()->Info("Opening joystick %d\n", m_joystick.index);

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

    GetLogger()->Info("Closing joystick\n");

    SDL_JoystickClose(m_private->joystick);
    m_private->joystick = nullptr;
}

bool CApplication::ChangeJoystick(const JoystickDevice &newJoystick)
{
    if ( (newJoystick.index < 0) || (newJoystick.index >= SDL_NumJoysticks()) )
        return false;

    m_joystick = newJoystick;

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
    m_input->MouseMove(pos);
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

                if (event.type == EVENT_SYS_QUIT)
                    goto end; // exit the loop

                if (event.type != EVENT_NULL)
                    m_eventQueue->AddEvent(event);

                Event virtualEvent = CreateVirtualEvent(event);
                if (virtualEvent.type != EVENT_NULL)
                    m_eventQueue->AddEvent(virtualEvent);
            }
        }

        // Now, process the last received mouse motion
        if (m_private->lastMouseMotionEvent.type != SDL_NOEVENT)
        {
            m_private->currentEvent = m_private->lastMouseMotionEvent;

            Event event = ProcessSystemEvent();

            if (event.type == EVENT_SYS_QUIT)
                goto end; // exit the loop

            if (event.type != EVENT_NULL)
                m_eventQueue->AddEvent(event);
        }

        // Enter game update & frame rendering only if active
        if (m_active)
        {
            Event event;
            while (m_eventQueue->GetEvent(event))
            {
                if (event.type == EVENT_SYS_QUIT || event.type == EVENT_QUIT)
                    goto end; // exit both loops

                LogEvent(event);

                bool passOn = true;
                if (m_engine != nullptr)
                    passOn = m_engine->ProcessEvent(event);

                if (passOn && m_controller != nullptr)
                    m_controller->ProcessEvent(event);
            }

            StopPerformanceCounter(PCNT_EVENT_PROCESSING);

            StartPerformanceCounter(PCNT_UPDATE_ALL);

            // Prepare and process step simulation event
            event = CreateUpdateEvent();
            if (event.type != EVENT_NULL && m_controller != nullptr)
            {
                LogEvent(event);

                m_sound->FrameMove(m_relTime);

                StartPerformanceCounter(PCNT_UPDATE_GAME);
                m_controller->ProcessEvent(event);
                StopPerformanceCounter(PCNT_UPDATE_GAME);

                StartPerformanceCounter(PCNT_UPDATE_ENGINE);
                m_engine->FrameUpdate();
                StopPerformanceCounter(PCNT_UPDATE_ENGINE);
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
                GetSystemUtils()->Usleep(20000); // should still give plenty of fps
            }
        }
    }

end:
    Destroy();

    return m_exitCode;
}

int CApplication::GetExitCode() const
{
    return m_exitCode;
}

const std::string& CApplication::GetErrorMessage() const
{
    return m_errorMessage;
}

/** The SDL event parsed is stored internally.
    If event is not available or is not understood, returned event is of type EVENT_NULL. */
Event CApplication::ProcessSystemEvent()
{
    Event event;

    if (m_private->currentEvent.type == SDL_QUIT)
    {
        event.type = EVENT_SYS_QUIT;
    }
    else if (m_private->currentEvent.type == SDL_VIDEORESIZE)
    {
        Gfx::DeviceConfig newConfig = m_deviceConfig;
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
        event.kmodState = m_private->currentEvent.key.keysym.mod;

        // Some keyboards return numerical enter keycode instead of normal enter
        // See issue #427 for details
        if (event.key.key == KEY(KP_ENTER))
            event.key.key = KEY(RETURN);

        if (event.key.key == KEY(TAB) && ((event.kmodState & KEY_MOD(ALT)) != 0))
        {
            GetLogger()->Debug("Minimize to taskbar\n");
            SDL_WM_IconifyWindow();
            event.type = EVENT_NULL;
        }
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
        }
    }
    else if (m_private->currentEvent.type == SDL_MOUSEMOTION)
    {
        event.type = EVENT_MOUSE_MOVE;

        m_input->MouseMove(Math::IntPoint(m_private->currentEvent.button.x, m_private->currentEvent.button.y));
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

    m_input->EventProcess(event);

    return event;
}

void CApplication::LogEvent(const Event &event)
{
    CLogger *l = GetLogger();

    auto PrintEventDetails = [&]()
    {
        l->Trace(" rTime = %f\n", event.rTime);
        l->Trace(" kmodState = %04x\n", event.kmodState);
        l->Trace(" mousePos = %f, %f\n", event.mousePos.x, event.mousePos.y);
        l->Trace(" mouseButtonsState = %02x\n", event.mouseButtonsState);
        l->Trace(" customParam = %d\n", event.customParam);
    };

    // Print the events in debug mode to test the code
    if (IsDebugModeActive(DEBUG_SYS_EVENTS) || IsDebugModeActive(DEBUG_APP_EVENTS))
    {
        std::string eventType = ParseEventType(event.type);

        if (IsDebugModeActive(DEBUG_SYS_EVENTS) && event.type <= EVENT_SYS_MAX)
        {
            l->Trace("System event %s:\n", eventType.c_str());
            switch (event.type)
            {
                case EVENT_KEY_DOWN:
                case EVENT_KEY_UP:
                    l->Trace(" virt    = %s\n", (event.key.virt) ? "true" : "false");
                    l->Trace(" key     = %d\n", event.key.key);
                    l->Trace(" unicode = 0x%04x\n", event.key.unicode);
                    break;
                case EVENT_MOUSE_BUTTON_DOWN:
                case EVENT_MOUSE_BUTTON_UP:
                    l->Trace(" button = %d\n", event.mouseButton.button);
                    break;
                case EVENT_MOUSE_WHEEL:
                    l->Trace(" dir = %s\n", (event.mouseWheel.dir == WHEEL_DOWN) ? "WHEEL_DOWN" : "WHEEL_UP");
                break;
                case EVENT_JOY_AXIS:
                    l->Trace(" axis  = %d\n", event.joyAxis.axis);
                    l->Trace(" value = %d\n", event.joyAxis.value);
                    break;
                case EVENT_JOY_BUTTON_DOWN:
                case EVENT_JOY_BUTTON_UP:
                    l->Trace(" button = %d\n", event.joyButton.button);
                    break;
                case EVENT_ACTIVE:
                    l->Trace(" flags = 0x%x\n", event.active.flags);
                    l->Trace(" gain  = %s\n", event.active.gain ? "true" : "false");
                    break;
                default:
                    break;
            }

            PrintEventDetails();
        }

        if (IsDebugModeActive(DEBUG_APP_EVENTS) && event.type > EVENT_SYS_MAX)
        {
            l->Trace("App event %s:\n", eventType.c_str());
            PrintEventDetails();
        }
    }
}


Event CApplication::CreateVirtualEvent(const Event& sourceEvent)
{
    Event virtualEvent;

    if ((sourceEvent.type == EVENT_KEY_DOWN) || (sourceEvent.type == EVENT_KEY_UP))
    {
        virtualEvent.type = sourceEvent.type;
        virtualEvent.key = sourceEvent.key;
        virtualEvent.key.key = GetVirtualKey(sourceEvent.key.key);
        virtualEvent.key.virt = true;

        if (virtualEvent.key.key == sourceEvent.key.key)
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

    m_input->EventProcess(virtualEvent);

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
    InternalResumeSimulation();

    GetLogger()->Info("Resume simulation\n");
}

void CApplication::ResetTimeAfterLoading()
{
    InternalResumeSimulation();

    GetLogger()->Trace("Resume simulation on loading\n");
}

void CApplication::InternalResumeSimulation()
{
    GetSystemUtils()->GetCurrentTimeStamp(m_baseTimeStamp);
    GetSystemUtils()->CopyTimeStamp(m_curTimeStamp, m_baseTimeStamp);
    m_realAbsTimeBase = m_realAbsTime;
    m_absTimeBase = m_exactAbsTime;
}

bool CApplication::GetSimulationSuspended() const
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
        m_eventQueue->AddEvent(Event(EVENT_SYS_QUIT));
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
    m_input->EventProcess(frameEvent);
    frameEvent.rTime = m_relTime;

    return frameEvent;
}

float CApplication::GetSimulationSpeed() const
{
    return m_simulationSpeed;
}

float CApplication::GetAbsTime() const
{
    return m_absTime;
}

long long CApplication::GetExactAbsTime() const
{
    return m_exactAbsTime;
}

long long CApplication::GetRealAbsTime() const
{
    return m_realAbsTime;
}

float CApplication::GetRelTime() const
{
    return m_relTime;
}

long long CApplication::GetExactRelTime() const
{
    return m_exactRelTime;
}

long long CApplication::GetRealRelTime() const
{
    return m_realRelTime;
}

Gfx::DeviceConfig CApplication::GetVideoConfig() const
{
    return m_deviceConfig;
}

VideoQueryResult CApplication::GetVideoResolutionList(std::vector<Math::IntPoint> &resolutions,
                                                      bool fullScreen, bool resizeable) const
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

void CApplication::SetDebugModeActive(DebugMode mode, bool active)
{
    if (active)
        m_debugModes |= mode;
    else
        m_debugModes &= (~mode);
}

bool CApplication::IsDebugModeActive(DebugMode mode) const
{
    return (m_debugModes & mode) != 0;
}

bool CApplication::ParseDebugModes(const std::string& str, int& debugModes)
{
    debugModes = 0;

    boost::char_separator<char> sep(",");
    boost::tokenizer<boost::char_separator<char>> tokens(str, sep);
    for (const auto& modeToken : tokens)
    {
        if (modeToken == "sys_events")
        {
            debugModes |= DEBUG_SYS_EVENTS;
        }
        else if (modeToken == "app_events")
        {
            debugModes |= DEBUG_APP_EVENTS;
        }
        else if (modeToken == "events")
        {
            debugModes |= DEBUG_EVENTS;
        }
        else if (modeToken == "models")
        {
            debugModes |= DEBUG_MODELS;
        }
        else if (modeToken == "all")
        {
            debugModes = DEBUG_ALL;
        }
        else
        {
            GetLogger()->Error("Invalid debug mode: '%s'\n", modeToken.c_str());
            return false;
        }
    }

    return true;
}

void CApplication::SetGrabInput(bool grab)
{
    SDL_WM_GrabInput(grab ? SDL_GRAB_ON : SDL_GRAB_OFF);
}

bool CApplication::GetGrabInput() const
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

MouseMode CApplication::GetMouseMode() const
{
    return m_mouseMode;
}

void CApplication::MoveMouse(Math::Point pos)
{
    Math::IntPoint windowPos = m_engine->InterfaceToWindowCoords(pos);
    m_input->MouseMove(windowPos);
    SDL_WarpMouse(windowPos.x, windowPos.y);
}

std::vector<JoystickDevice> CApplication::GetJoystickList() const
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

JoystickDevice CApplication::GetJoystick() const
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

bool CApplication::GetJoystickEnabled() const
{
    return m_joystickEnabled;
}

Language CApplication::GetLanguage() const
{
    return m_language;
}

char CApplication::GetLanguageChar() const
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

        case LANGUAGE_RUSSIAN:
            langChar = 'R';
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
    else if (str == "ru")
    {
        language = LANGUAGE_RUSSIAN;
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

        case LANGUAGE_RUSSIAN:
            locale = "ru_RU.utf8";
            break;
    }

    if (locale.empty())
    {
        const char* envLang = gl_locale_name(LC_MESSAGES, "LC_MESSAGES");
        if (envLang == NULL)
        {
            GetLogger()->Error("Failed to get language from environment, setting default language\n");
            m_language = LANGUAGE_ENGLISH;
        }
        else
        {
            GetLogger()->Trace("gl_locale_name: '%s'\n", envLang);

            if (strncmp(envLang,"en",2) == 0)
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
            else if (strncmp(envLang,"ru",2) == 0)
            {
                m_language = LANGUAGE_RUSSIAN;
            }
            else
            {
                GetLogger()->Warn("Enviromnent locale ('%s') is not supported, setting default language\n", envLang);
                m_language = LANGUAGE_ENGLISH;
            }
        }
    }
    else
    {
        std::string langStr = "LANGUAGE=";
        langStr += locale;
        strcpy(S_LANGUAGE, langStr.c_str());
        putenv(S_LANGUAGE);
        GetLogger()->Trace("SetLanguage: Set LANGUAGE=%s in environment\n", locale.c_str());
    }

    char* defaultLocale = setlocale(LC_ALL, ""); // Load system locale
    setlocale(LC_NUMERIC, "C"); // Force numeric locale to "C" (fixes decimal point problems)
    char* systemLocale = setlocale(LC_ALL, nullptr); // Get current locale configuration
    GetLogger()->Debug("Default system locale: %s\n", defaultLocale);
    GetLogger()->Debug("Setting locale: %s\n", systemLocale);
    // Update C++ locale
    try
    {
        std::locale::global(std::locale(systemLocale));
    }
    catch (...)
    {
        GetLogger()->Warn("Failed to update locale, possibly incorect system configuration. Will fallback to classic locale.\n");
        try
        {
            std::locale::global(std::locale::classic());
        }
        catch (...)
        {
            GetLogger()->Warn("Failed to set classic locale. Something is really messed up in your system configuration. Translations might not work.\n");
        }

        // C locale might still work correctly
        setlocale(LC_ALL, "");
        setlocale(LC_NUMERIC, "C");
    }

    bindtextdomain("colobot", m_pathManager->GetLangPath().c_str());
    bind_textdomain_codeset("colobot", "UTF-8");
    textdomain("colobot");

    GetLogger()->Debug("SetLanguage: Test gettext translation: '%s'\n", gettext("Colobot rules!"));
}

void CApplication::SetLowCPU(bool low)
{
    m_lowCPU = low;
}

bool CApplication::GetLowCPU() const
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

float CApplication::GetPerformanceCounterData(PerformanceCounter counter) const
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

bool CApplication::GetSceneTestMode()
{
    return m_sceneTest;
}
