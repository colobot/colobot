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
#include "common/stringutils.h"

#include "graphics/engine/modelmanager.h"
#include "graphics/opengl/gldevice.h"

#include "object/robotmain.h"

#ifdef OPENAL_SOUND
    #include "sound/oalsound/alsound.h"
#endif

#include <boost/filesystem.hpp>
#include <boost/tokenizer.hpp>

#include <SDL.h>
#include <SDL_image.h>

#include <stdlib.h>
#include <libintl.h>
#include <unistd.h>
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
{
    m_private       = new ApplicationPrivate();
    m_iMan          = new CInstanceManager();
    m_objMan        = new CObjectManager();
    m_eventQueue    = new CEventQueue();
    m_profile       = new CProfile();

    m_engine    = nullptr;
    m_device    = nullptr;
    m_modelManager = nullptr;
    m_robotMain = nullptr;
    m_sound     = nullptr;

    m_exitCode  = 0;
    m_active    = false;
    m_debugModes = 0;

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
    m_langPath = COLOBOT_I18N_DIR;
    m_texPackPath = "";

    m_runSceneName = "";
    m_runSceneRank = 0;

    m_language = LANGUAGE_ENV;

    m_lowCPU = true;

    m_protoMode = false;

    for (int i = 0; i < DIR_MAX; ++i)
        m_standardDataDirs[i] = nullptr;

    m_standardDataDirs[DIR_AI]       = "ai";
    m_standardDataDirs[DIR_FONT]     = "fonts";
    m_standardDataDirs[DIR_HELP]     = "help";
    m_standardDataDirs[DIR_ICON]     = "icons";
    m_standardDataDirs[DIR_LEVEL]    = "levels";
    m_standardDataDirs[DIR_MODEL]    = "models";
    m_standardDataDirs[DIR_MUSIC]    = "music";
    m_standardDataDirs[DIR_SOUND]    = "sounds";
    m_standardDataDirs[DIR_TEXTURE]  = "textures";
}

CApplication::~CApplication()
{
    delete m_private;
    m_private = nullptr;

    delete m_objMan;
    m_objMan = nullptr;

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
        OPT_RUNSCENE,
        OPT_LOGLEVEL,
        OPT_LANGUAGE,
        OPT_DATADIR,
        OPT_LANGDIR,
        OPT_TEXPACK,
        OPT_VBO,
        OPT_PROTO
    };

    option options[] =
    {
        { "help", no_argument, nullptr, OPT_HELP },
        { "debug", required_argument, nullptr, OPT_DEBUG },
        { "runscene", required_argument, nullptr, OPT_RUNSCENE },
        { "loglevel", required_argument, nullptr, OPT_LOGLEVEL },
        { "language", required_argument, nullptr, OPT_LANGUAGE },
        { "datadir", required_argument, nullptr, OPT_DATADIR },
        { "langdir", required_argument, nullptr, OPT_LANGDIR },
        { "texpack", required_argument, nullptr, OPT_TEXPACK },
        { "vbo", required_argument, nullptr, OPT_VBO },
        { "proto", no_argument, nullptr, OPT_PROTO },
        { nullptr, 0, nullptr, 0}
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
                GetLogger()->Message("  -help               this help\n");
                GetLogger()->Message("  -debug modes        enable debug modes (more info printed in logs; see code for reference of modes)\n");
                GetLogger()->Message("  -runscene sceneNNN  run given scene on start\n");
                GetLogger()->Message("  -loglevel level     set log level to level (one of: trace, debug, info, warn, error, none)\n");
                GetLogger()->Message("  -language lang      set language (one of: en, de, fr, pl)\n");
                GetLogger()->Message("  -datadir path       set custom data directory path\n");
                GetLogger()->Message("  -langdir path       set custom language directory path\n");
                GetLogger()->Message("  -texpack path       set path to custom texture pack\n");
                GetLogger()->Message("  -vbo mode           set OpenGL VBO mode (one of: auto, enable, disable)\n");
                GetLogger()->Message("  -proto              show prototype levels\n");
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
                m_dataPath = optarg;
                GetLogger()->Info("Using custom data dir: '%s'\n", m_dataPath.c_str());
                break;
            }
            case OPT_LANGDIR:
            {
                m_langPath = optarg;
                GetLogger()->Info("Using custom language dir: '%s'\n", m_langPath.c_str());
                break;
            }
            case OPT_TEXPACK:
            {
                m_texPackPath = optarg;
                GetLogger()->Info("Using texturepack: '%s'\n", m_texPackPath.c_str());
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
                    GetLogger()->Error("Invalid vbo mode: '%s'\n", optarg);
                    return PARSE_ARGS_FAIL;
                }

                break;
            }
            case OPT_PROTO:
            {
                m_protoMode = true;
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

    if (!GetProfile().InitCurrentDirectory())
    {
        GetLogger()->Warn("Config not found. Default values will be used!\n");
        defaultValues = true;
    }
    else
    {
        if (GetProfile().GetLocalProfileString("Resources", "Data", path))
            m_dataPath = path;
    }

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

    GetProfile().SetLocalProfileString("Resources", "Data", m_dataPath);

    SetLanguage(m_language);

    //Create the sound instance.
    #ifdef OPENAL_SOUND
    m_sound = static_cast<CSoundInterface *>(new ALSound());
    #else
    GetLogger()->Info("No sound support.\n");
    m_sound = new CSoundInterface();
    #endif

    m_sound->Create(true);

    // Cache sound files
    if (defaultValues)
    {
        GetProfile().SetLocalProfileString("Resources", "Sound", GetDataSubdirPath(DIR_SOUND));
        GetProfile().SetLocalProfileString("Resources", "Music", GetDataSubdirPath(DIR_MUSIC));
    }

    if (GetProfile().GetLocalProfileString("Resources", "Sound", path))
    {
        m_sound->CacheAll(path);
    }
    else
    {
        m_sound->CacheAll(GetDataSubdirPath(DIR_SOUND));
    }

    if (GetProfile().GetLocalProfileString("Resources", "Music", path))
    {
        m_sound->AddMusicFiles(path);
    }
    else
    {
        m_sound->AddMusicFiles(GetDataSubdirPath(DIR_MUSIC));
    }

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
    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

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
    m_robotMain = new CRobotMain(this, !defaultValues);

    if (defaultValues) m_robotMain->CreateIni();

    if (! m_runSceneName.empty())
        m_robotMain->LoadSceneOnStart(m_runSceneName, m_runSceneRank);
    else
        m_robotMain->ChangePhase(PHASE_WELCOME1);

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
            m_eventQueue->AddEvent(Event(EVENT_SYS_QUIT));
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

                if (passOn && m_robotMain != nullptr)
                    m_robotMain->ProcessEvent(event);
            }

            StopPerformanceCounter(PCNT_EVENT_PROCESSING);

            StartPerformanceCounter(PCNT_UPDATE_ALL);

            // Prepare and process step simulation event
            event = CreateUpdateEvent();
            if (event.type != EVENT_NULL && m_robotMain != nullptr)
            {
                LogEvent(event);

                StartPerformanceCounter(PCNT_UPDATE_ENGINE);
                m_engine->FrameUpdate();
                StopPerformanceCounter(PCNT_UPDATE_ENGINE);

                m_sound->FrameMove(m_relTime);

                StartPerformanceCounter(PCNT_UPDATE_GAME);
                m_robotMain->ProcessEvent(event);
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

void CApplication::LogEvent(const Event &event)
{
    CLogger *l = GetLogger();

    auto PrintEventDetails = [&]()
    {
        l->Trace(" rTime = %f\n", event.rTime);
        l->Trace(" kmodState = %04x\n", event.kmodState);
        l->Trace(" trackedKeysState = %04x\n", event.trackedKeysState);
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
    frameEvent.trackedKeysState = m_trackedKeys;
    frameEvent.kmodState = m_kmodState;
    frameEvent.mousePos = m_mousePos;
    frameEvent.mouseButtonsState = m_mouseButtonsState;
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

Gfx::GLDeviceConfig CApplication::GetVideoConfig() const
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

int CApplication::GetKmods() const
{
    return m_kmodState;
}

bool CApplication::GetKmodState(int kmod) const
{
    return (m_kmodState & kmod) != 0;
}

bool CApplication::GetTrackedKeyState(TrackedKey key) const
{
    return (m_trackedKeys & key) != 0;
}

bool CApplication::GetMouseButtonState(int index) const
{
    return (m_mouseButtonsState & (1<<index)) != 0;
}

void CApplication::ResetKeyStates()
{
    GetLogger()->Trace("Reset key states\n");
    m_trackedKeys = 0;
    m_kmodState = 0;
    m_robotMain->ResetKeyStates();
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

Math::Point CApplication::GetMousePos() const
{
    return m_mousePos;
}

void CApplication::MoveMouse(Math::Point pos)
{
    m_mousePos = pos;

    Math::IntPoint windowPos = m_engine->InterfaceToWindowCoords(pos);
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

std::string CApplication::GetDataDirPath() const
{
    return m_dataPath;
}

std::string CApplication::GetDataSubdirPath(DataDir stdDir) const
{
    int index = static_cast<int>(stdDir);
    assert(index >= 0 && index < DIR_MAX);
    std::stringstream str;
    str << m_dataPath;
    str << "/";
    str << m_standardDataDirs[index];
    return str.str();
}

std::string CApplication::GetDataFilePath(DataDir stdDir, const std::string& subpath) const
{
    int index = static_cast<int>(stdDir);
    assert(index >= 0 && index < DIR_MAX);
    std::stringstream str;
    str << m_dataPath;
    str << "/";
    str << m_standardDataDirs[index];
    if (stdDir == DIR_HELP)
    {
        str << "/";
        str << GetLanguageChar();
    }
    str << "/";
    str << subpath;
    return str.str();
}

std::string CApplication::GetTexPackFilePath(const std::string& textureName) const
{
    std::stringstream str;

    if (! m_texPackPath.empty())
    {
        str << m_texPackPath;
        str << "/";
        str << textureName;
        if (! boost::filesystem::exists(str.str()))
        {
            GetLogger()->Trace("Texture '%s' not in texpack\n", textureName.c_str());
            str.str("");
        }
    }

    return str.str();
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

    setlocale(LC_ALL, "");

    bindtextdomain("colobot", m_langPath.c_str());
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

bool CApplication::GetProtoMode() const
{
    return m_protoMode;
}

