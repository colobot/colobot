/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2023, Daniel Roux, EPSITEC SA & TerranovaTeam
 * http://epsitec.ch; http://colobot.info; http://github.com/colobot
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

#include "app/app.h"

#include "app/controller.h"
#include "app/input.h"
#include "app/modman.h"
#include "app/pathman.h"

#include "common/config_file.h"
#include "common/image.h"
#include "common/key.h"
#include "common/logger.h"
#include "common/profiler.h"
#include "common/stringutils.h"
#include "common/version.h"

#include "common/resources/resourcemanager.h"

#include "common/system/system.h"

#include "graphics/core/device.h"
#include "graphics/engine/engine.h"
#include "graphics/opengl33/glutil.h"

#include "level/robotmain.h"

#include "object/object_manager.h"

#include "sound/sound.h"
#ifdef OPENAL_SOUND
    #include "sound/oalsound/alsound.h"
#endif

#include <SDL.h>
#include <SDL_image.h>

#include <stdlib.h>
#include <getopt.h>
#include <localename.h>
#include <thread>

#include <libintl.h>

using TimeUtils::TimeStamp;
using TimeUtils::TimeUnit;


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
    //! Main game window
    SDL_Window *window;
    //! Main game OpenGL context
    SDL_GLContext glcontext;
    //! Currently handled event
    SDL_Event currentEvent;
    //! Mouse motion event to be handled
    SDL_Event lastMouseMotionEvent;
    //! Joystick
    SDL_Joystick *joystick;
    //! Id of joystick timer
    SDL_TimerID joystickTimer;
    //! Haptic subsystem for the joystick
    SDL_Haptic *haptic;

    ApplicationPrivate()
    {
        SDL_memset(&currentEvent, 0, sizeof(SDL_Event));
        SDL_memset(&lastMouseMotionEvent, 0, sizeof(SDL_Event));
        window = nullptr;
        glcontext = nullptr;
        joystick = nullptr;
        joystickTimer = 0;
        haptic = nullptr;
    }
};



CApplication::CApplication(CSystemUtils* systemUtils)
    : m_systemUtils(systemUtils),
      m_private(std::make_unique<ApplicationPrivate>()),
      m_configFile(std::make_unique<CConfigFile>()),
      m_input(std::make_unique<CInput>()),
      m_pathManager(std::make_unique<CPathManager>(systemUtils)),
      m_modManager(std::make_unique<CModManager>(this, m_pathManager.get())),
      m_deviceConfig(std::make_unique<Gfx::DeviceConfig>())
{
}

CApplication::~CApplication()
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

    if (m_private->glcontext != nullptr)
    {
        SDL_GL_DeleteContext(m_private->glcontext);
        m_private->glcontext = nullptr;
    }

    if (m_private->window != nullptr)
    {
        SDL_DestroyWindow(m_private->window);
        m_private->window = nullptr;
    }

    IMG_Quit();

    if (SDL_WasInit(0))
        SDL_Quit();
}

CEventQueue* CApplication::GetEventQueue()
{
    return m_eventQueue.get();
}

CSoundInterface* CApplication::GetSound()
{
    return m_sound.get();
}

CModManager* CApplication::GetModManager()
{
    return m_modManager.get();
}

void CApplication::LoadEnvironmentVariables()
{
    auto dataDir = m_systemUtils->GetEnvVar("COLOBOT_DATA_DIR");
    if (!dataDir.empty())
    {
        m_pathManager->SetDataPath(StrUtils::ToPath(dataDir));
        GetLogger()->Info("Using data dir (based on environment variable): '%%'", dataDir);
    }

    auto langDir = m_systemUtils->GetEnvVar("COLOBOT_LANG_DIR");
    if (!langDir.empty())
    {
        m_pathManager->SetLangPath(StrUtils::ToPath(langDir));
        GetLogger()->Info("Using lang dir (based on environment variable): '%%'", langDir);
    }

    auto saveDir = m_systemUtils->GetEnvVar("COLOBOT_SAVE_DIR");
    if (!saveDir.empty())
    {
        m_pathManager->SetSavePath(StrUtils::ToPath(saveDir));
        GetLogger()->Info("Using save dir (based on environment variable): '%%'", saveDir);
    }
}

ParseArgsStatus CApplication::ParseArguments(const std::vector<std::string>& args)
{
    enum OptionType
    {
        OPT_HELP = 1,
        OPT_DEBUG,
        OPT_RUNSCENE,
        OPT_SCENETEST,
        OPT_LOADSAVE,
        OPT_LOGLEVEL,
        OPT_LANGDIR,
        OPT_DATADIR,
        OPT_SAVEDIR,
        OPT_MOD,
        OPT_RESOLUTION,
        OPT_HEADLESS,
        OPT_DEVICE,
        OPT_OPENGL_VERSION,
        OPT_OPENGL_PROFILE
    };

    option options[] =
    {
        { "help", no_argument, nullptr, OPT_HELP },
        { "debug", required_argument, nullptr, OPT_DEBUG },
        { "runscene", required_argument, nullptr, OPT_RUNSCENE },
        { "scenetest", no_argument, nullptr, OPT_SCENETEST },
        { "loadsave", required_argument, nullptr, OPT_LOADSAVE },
        { "loglevel", required_argument, nullptr, OPT_LOGLEVEL },
        { "langdir", required_argument, nullptr, OPT_LANGDIR },
        { "datadir", required_argument, nullptr, OPT_DATADIR },
        { "savedir", required_argument, nullptr, OPT_SAVEDIR },
        { "mod", required_argument, nullptr, OPT_MOD },
        { "resolution", required_argument, nullptr, OPT_RESOLUTION },
        { "headless", no_argument, nullptr, OPT_HEADLESS },
        { "graphics", required_argument, nullptr, OPT_DEVICE },
        { "glversion", required_argument, nullptr, OPT_OPENGL_VERSION },
        { "glprofile", required_argument, nullptr, OPT_OPENGL_PROFILE },
        { nullptr, 0, nullptr, 0}
    };

    opterr = 0;
    optind = 1;

    int c = 0;
    int index = -1;

    std::vector<std::string> copy = args;
    std::vector<char*> arguments;

    for (auto& arg : copy)
        arguments.push_back(arg.data());

    while ((c = getopt_long_only(arguments.size(), arguments.data(), "", options, &index)) != -1)
    {
        if (c == '?')
        {
            if (optopt == 0)
                GetLogger()->Error("Invalid argument: %%", args[optind-1]);
            else
                GetLogger()->Error("Expected argument for option: %%", args[optind-1]);

            m_exitCode = 1;
            return PARSE_ARGS_FAIL;
        }

        index = -1;

        switch (c)
        {
            case OPT_HELP:
            {
                GetLogger()->Message("");
                GetLogger()->Message("%%", Version::FULL_NAME);
                GetLogger()->Message("");
                GetLogger()->Message("List of available options and environment variables:");
                GetLogger()->Message("  -help               this help");
                GetLogger()->Message("  -debug modes        enable debug modes (more info printed in logs; see code for reference of modes)");
                GetLogger()->Message("  -runscene sceneNNN  run given scene on start");
                GetLogger()->Message("  -scenetest          win every mission right after it's loaded");
                GetLogger()->Message("  -loadsave path      load given saved game on start (path is <player>/<save>");
                GetLogger()->Message("  -loglevel level     set log level to level (one of: trace, debug, info, warn, error, none)");
                GetLogger()->Message("  -langdir path       set custom language directory path");
                GetLogger()->Message("                      environment variable: COLOBOT_LANG_DIR");
                GetLogger()->Message("  -datadir path       set custom data directory path");
                GetLogger()->Message("                      environment variable: COLOBOT_DATA_DIR");
                GetLogger()->Message("  -savedir path       set custom save directory path (must be writable)");
                GetLogger()->Message("                      environment variable: COLOBOT_SAVE_DIR");
                GetLogger()->Message("  -mod path           load datadir mod from given path");
                GetLogger()->Message("  -resolution WxH     set resolution");
                GetLogger()->Message("  -headless           headless mode - disables graphics, sound and user interaction");
                GetLogger()->Message("  -graphics           changes graphics device (one of: default, auto, opengl, gl14, gl21, gl33");
                GetLogger()->Message("  -glversion          sets OpenGL context version to use (either default or version in format #.#)");
                GetLogger()->Message("  -glprofile          sets OpenGL context profile to use (one of: default, core, compatibility, opengles)");
                return PARSE_ARGS_HELP;
            }
            case OPT_DEBUG:
            {
                if (optarg == nullptr)
                {
                    m_debugModes = DEBUG_ALL;
                    GetLogger()->Info("All debug modes active");
                }
                else
                {
                    int debugModes;
                    if (! ParseDebugModes(optarg, debugModes))
                    {
                        return PARSE_ARGS_FAIL;
                    }

                    m_debugModes = debugModes;
                    GetLogger()->Info("Active debug modes: %%", optarg);
                }
                break;
            }
            case OPT_RUNSCENE:
            {
                std::string file = optarg;
                std::string cat = file.substr(0, file.size()-3);
                m_runSceneCategory = GetLevelCategoryFromDir(cat);
                m_runSceneRank = StrUtils::FromString<int>(file.substr(file.size()-3, 3));
                if(m_runSceneCategory != LevelCategory::Max)
                {
                    GetLogger()->Info("Running scene '%%%%' on start", cat, m_runSceneRank);
                }
                else
                {
                    GetLogger()->Error("Requested to run scene from unknown category '%%'", cat);
                    return PARSE_ARGS_FAIL;
                }
                break;
            }
            case OPT_SCENETEST:
            {
                m_sceneTest = true;
                break;
            }
            case OPT_LOADSAVE:
            {
                std::string playerAndPath = optarg;
                size_t pos = playerAndPath.find_first_of("/");
                if (pos != std::string::npos && pos > 0 && pos+1 < playerAndPath.length()) // Split player name from path
                {
                    m_loadSavePlayerName = playerAndPath.substr(0, pos);
                    m_loadSaveDirName = playerAndPath.substr(pos+1, playerAndPath.length()-pos-1);
                }
                else
                {
                    GetLogger()->Error("Invalid path: '%%'", optarg);
                    return PARSE_ARGS_FAIL;
                }
                break;
            }
            case OPT_LOGLEVEL:
            {
                LogLevel logLevel;
                if (! CLogger::ParseLogLevel(optarg, logLevel))
                {
                    GetLogger()->Error("Invalid log level: '%%'", optarg);
                    return PARSE_ARGS_FAIL;
                }

                GetLogger()->Message("[*****] Log level changed to %%", optarg);
                GetLogger()->SetLogLevel(logLevel);
                break;
            }
            case OPT_DATADIR:
            {
                m_pathManager->SetDataPath(StrUtils::ToPath(optarg));
                GetLogger()->Info("Using data dir: '%%'", optarg);
                break;
            }
            case OPT_LANGDIR:
            {
                m_pathManager->SetLangPath(StrUtils::ToPath(optarg));
                GetLogger()->Info("Using language dir: '%%'", optarg);
                break;
            }
            case OPT_SAVEDIR:
            {
                m_pathManager->SetSavePath(StrUtils::ToPath(optarg));
                GetLogger()->Info("Using save dir: '%%'", optarg);
                break;
            }
            case OPT_MOD:
            {
                m_pathManager->AddMod(StrUtils::ToPath(optarg));
                break;
            }
            case OPT_RESOLUTION:
            {
                std::istringstream resolution(optarg);
                std::string w, h;
                std::getline(resolution, w, 'x');
                std::getline(resolution, h, 'x');

                m_deviceConfig->size.x = atoi(w.c_str());
                m_deviceConfig->size.y = atoi(h.c_str());
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
                m_graphicsOverride = true;
                break;
            }
            case OPT_OPENGL_VERSION:
            {
                if (strcmp(optarg, "default") == 0)
                {
                    m_glMajor = -1;
                    m_glMinor = -1;
                    m_glVersionOverride = true;
                }
                else
                {
                    int major = 1, minor = 1;

                    int parsed = sscanf(optarg, "%d.%d", &major, &minor);

                    if (parsed < 2)
                    {
                        GetLogger()->Error("Invalid OpenGL version: %%", optarg);
                        return PARSE_ARGS_FAIL;
                    }

                    m_glMajor = major;
                    m_glMinor = minor;
                    m_glVersionOverride = true;
                }
                break;
            }
            case OPT_OPENGL_PROFILE:
            {
                if (strcmp(optarg, "default") == 0)
                {
                    m_glProfile = 0;
                    m_glProfileOverride = true;
                }
                else if (strcmp(optarg, "core") == 0)
                {
                    m_glProfile = SDL_GL_CONTEXT_PROFILE_CORE;
                    m_glProfileOverride = true;
                }
                else if (strcmp(optarg, "compatibility") == 0)
                {
                    m_glProfile = SDL_GL_CONTEXT_PROFILE_COMPATIBILITY;
                    m_glProfileOverride = true;
                }
                else if (strcmp(optarg, "opengles") == 0)
                {
                    m_glProfile = SDL_GL_CONTEXT_PROFILE_ES;
                    m_glProfileOverride = true;
                }
                else
                {
                    GetLogger()->Error("Invalid OpenGL profile: %%", optarg);
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
    GetLogger()->Info("Creating CApplication");

    m_baseTimeStamp = TimeUtils::GetCurrentTimeStamp();
    m_lastTimeStamp = m_baseTimeStamp;
    m_curTimeStamp = m_baseTimeStamp;

    m_errorMessage = m_pathManager->VerifyPaths();
    if (!m_errorMessage.empty())
    {
        m_exitCode = 1;
        return false;
    }
    m_pathManager->InitPaths();

    if (!GetConfigFile().Init())
    {
        GetLogger()->Warn("Config could not be loaded. Default values will be used!");
    }

    m_modManager->FindMods();
    m_modManager->SaveMods();
    m_modManager->MountAllMods();

    // Create the sound instance.
    #ifdef OPENAL_SOUND
    if (!m_headless)
    {
        m_sound = std::make_unique<CALSound>();
    }
    else
    {
        m_sound = std::make_unique<CSoundInterface>();
    }
    #else
    GetLogger()->Info("No sound support.");
    m_sound = std::make_unique<CSoundInterface>();
    #endif

    m_sound->Create();

    GetLogger()->Info("CApplication created successfully");

    std::string standardInfoMessage =
      "\nPlease see the console output or log file\n"
      "to get more information on the source of error";

    /* SDL initialization sequence */

    // Creating the m_engine now because it holds the vsync flag
    m_engine = std::make_unique<Gfx::CEngine>(this, m_systemUtils);

    Uint32 initFlags = SDL_INIT_VIDEO | SDL_INIT_TIMER;

    if (SDL_Init(initFlags) < 0)
    {
        m_errorMessage = std::string("SDL initialization error:") +
                         std::string(SDL_GetError());
        GetLogger()->Error(m_errorMessage);
        m_exitCode = 2;
        return false;
    }

    // This is non-fatal and besides seems to fix some memory leaks
    if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) < 0)
    {
        GetLogger()->Warn("Joystick subsystem init failed\nJoystick(s) will not be available");
    }
    if (SDL_InitSubSystem(SDL_INIT_HAPTIC) < 0)
    {
        GetLogger()->Warn("Joystick haptic subsystem init failed\nForce feedback will not be available");
    }

    if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) == 0)
    {
        m_errorMessage = std::string("SDL_Image initialization error:") +
                         std::string(IMG_GetError());
        GetLogger()->Error(m_errorMessage);
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
        auto modes = GetVideoResolutionList();

        std::vector<int> values;

        if ( GetConfigFile().GetArrayProperty("Setup", "Resolution", values) && !m_resolutionOverride )
        {
            int w = 800, h = 600;

            if (values.size() >= 2)
            {
                w = values[0];
                h = values[1];
            }

            // Why not just set m_deviceConfig.size to w,h? Because this way if the resolution is no longer supported (e.g. changing monitor) defaults will be used instead
            for (auto it = modes.begin(); it != modes.end(); ++it)
            {
                if (it->x == w && it->y == h)
                {
                    m_deviceConfig->size = *it;
                    break;
                }
            }
        }

        if ( GetConfigFile().GetIntProperty("Setup", "Fullscreen", iValue) && !m_resolutionOverride )
        {
            m_deviceConfig->fullScreen = (iValue == 1);
        }

        if (! CreateVideoSurface())
            return false; // dialog is in function

        if (m_private->window == nullptr)
        {
            m_errorMessage = std::string("SDL error while setting video mode:\n") +
                            std::string(SDL_GetError());
            GetLogger()->Error(m_errorMessage);
            m_exitCode = 4;
            return false;
        }
    }

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
        GetLogger()->Info("Detected joystick: %% [ID %%]", joystick.name, joystick.index);
    }
    if (first)
    {
        GetLogger()->Info("No joysticks detected");
    }

    //if (!m_headless)
    {
        std::string graphics = "default";
        std::string value;

        if (m_graphicsOverride)
        {
            graphics = m_graphics;
        }
        else if (GetConfigFile().GetStringProperty("Experimental", "GraphicsDevice", value))
        {
            graphics = value;
        }

        m_device = Gfx::CreateDevice(*m_deviceConfig, graphics);

        if (m_device == nullptr)
        {
            GetLogger()->Error("Unknown graphics device: %%", graphics);
            GetLogger()->Info("Changing to default device");
            m_systemUtils->SystemDialog(SystemDialogType::ERROR_MSG, "Graphics initialization error", "You have selected invalid graphics device with -graphics switch. Game will use default OpenGL device instead.");
            m_device = Gfx::CreateDevice(*m_deviceConfig, "opengl");
        }
    }
    //else
    //{
    //    m_device = std::make_unique<Gfx::CNullDevice>();
    //}

    if (! m_device->Create() )
    {
        m_errorMessage = std::string("Error in CDevice::Create()")
            + "\n\n"
            + m_device->GetError()
            + standardInfoMessage;
        m_exitCode = 5;
        return false;
    }

    // Create the 3D engine
    m_engine->SetDevice(m_device.get());

    if (! m_engine->Create() )
    {
        m_errorMessage = std::string("Error in CEngine::Init()") + standardInfoMessage;
        m_exitCode = 6;
        return false;
    }

    m_eventQueue = std::make_unique<CEventQueue>();

    // Create the robot application.
    m_controller = std::make_unique<CController>();

    StartLoadingMusic();

    if (!m_loadSaveDirName.empty())
    {
        m_controller->GetRobotMain()->SelectPlayer(m_loadSavePlayerName);
        m_controller->GetRobotMain()->LoadSaveFromDirName(TempToPath(m_loadSaveDirName));
    }
    else if (m_runSceneCategory == LevelCategory::Max)
    {
        m_controller->StartApp();
    }
    else
    {
        m_controller->GetRobotMain()->UpdateCustomLevelList(); // To load the userlevels
        m_controller->GetRobotMain()->SetExitAfterMission(true);
        m_controller->StartGame(m_runSceneCategory, m_runSceneRank/100, m_runSceneRank%100);
    }

    return true;
}

void CApplication::ReloadResources()
{
    GetLogger()->Info("Reloading resources");
    m_engine->ReloadAllTextures();
    StartLoadingMusic();
    m_controller->GetRobotMain()->UpdateCustomLevelList();
}


bool CApplication::CreateVideoSurface()
{
    Uint32 videoFlags = SDL_WINDOW_OPENGL;

    if (m_deviceConfig->fullScreen)
        videoFlags |= SDL_WINDOW_FULLSCREEN;

    if (m_deviceConfig->resizeable)
        videoFlags |= SDL_WINDOW_RESIZABLE;

    // Set OpenGL attributes

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE,   m_deviceConfig->redSize);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, m_deviceConfig->greenSize);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,  m_deviceConfig->blueSize);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, m_deviceConfig->alphaSize);

    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, m_deviceConfig->depthSize);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, m_deviceConfig->stencilSize);

    if (m_deviceConfig->doubleBuf)
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    std::string value;

    // set OpenGL context version
    // -glversion switch overrides config settings
    if (m_glVersionOverride)
    {
        if ((m_glMajor >= 0) && (m_glMinor >= 0))
        {
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, m_glMajor);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, m_glMinor);

            GetLogger()->Info("Requesting OpenGL context version %%.%%", m_glMajor, m_glMinor);
        }
    }
    else if (GetConfigFile().GetStringProperty("Experimental", "OpenGLVersion", value))
    {
        int major = 1, minor = 1;

        sscanf(value.c_str(), "%d.%d", &major, &minor);

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, major);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, minor);

        GetLogger()->Info("Requesting OpenGL context version %%.%%", major, minor);
    }

    // set OpenGL context profile
    // -glprofile switch overrides config settings
    int profile = 0;

    if (m_glProfileOverride)
    {
        profile = m_glProfile;
    }
    else if (GetConfigFile().GetStringProperty("Experimental", "OpenGLProfile", value))
    {
        if (value == "core")
        {
            profile = SDL_GL_CONTEXT_PROFILE_CORE;
        }
        else if (value == "compatibility")
        {
            profile = SDL_GL_CONTEXT_PROFILE_COMPATIBILITY;
        }
        else if (value == "opengles")
        {
            profile = SDL_GL_CONTEXT_PROFILE_ES;
        }
    }

    if (profile != 0)
    {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, profile);

        switch (profile)
        {
        case SDL_GL_CONTEXT_PROFILE_CORE:
            GetLogger()->Info("Requesting OpenGL core profile");
            break;
        case SDL_GL_CONTEXT_PROFILE_COMPATIBILITY:
            GetLogger()->Info("Requesting OpenGL compatibility profile");
            break;
        case SDL_GL_CONTEXT_PROFILE_ES:
            GetLogger()->Info("Requesting OpenGL ES profile");
            break;
        }
    }

    int msaa = 0;
    if (GetConfigFile().GetIntProperty("Experimental", "MSAA", msaa))
    {
        if (msaa > 1)
        {
            SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
            SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, msaa);

            GetLogger()->Info("Using MSAA on default framebuffer (%% samples)", msaa);
        }
    }

    /* If hardware acceleration specifically requested, this will force the hw accel
       and fail with error if not available */
    if (m_deviceConfig->hardwareAccel)
        SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

    m_private->window = SDL_CreateWindow(m_windowTitle.c_str(),
                                         SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                         m_deviceConfig->size.x, m_deviceConfig->size.y,
                                         videoFlags);

    m_private->glcontext = SDL_GL_CreateContext(m_private->window);

    int vsync = 0;
    if (GetConfigFile().GetIntProperty("Setup", "VSync", vsync))
    {
        m_engine->SetVSync(vsync);
        TryToSetVSync();
        vsync = m_engine->GetVSync();
        GetConfigFile().SetIntProperty("Setup", "VSync", vsync);

        GetLogger()->Info("Using Vsync: %%", (vsync == -1 ? "adaptive" : (vsync ? "true" : "false")));
    }

    return true;
}

void CApplication::TryToSetVSync()
{
    int vsync = m_engine->GetVSync();
    int result = SDL_GL_SetSwapInterval(vsync);
    if (result == -1)
    {
        switch (vsync)
        {
        case -1:
            GetLogger()->Warn("Adaptive sync not supported: %%", SDL_GetError());
            m_engine->SetVSync(1);
            TryToSetVSync();
            break;
        case 1:
            GetLogger()->Warn("Couldn't enable VSync: %%", SDL_GetError());
            m_engine->SetVSync(0);
            TryToSetVSync();
            break;
        case 0:
            GetLogger()->Warn("Couldn't disable VSync: %%", SDL_GetError());
            m_engine->SetVSync(SDL_GL_GetSwapInterval());
            break;
        }
    }
}

bool CApplication::ChangeVideoConfig(const Gfx::DeviceConfig &newConfig)
{
    *m_deviceConfig = newConfig;

    // TODO: Somehow this doesn't work for maximized windows (at least on Ubuntu)
    SDL_SetWindowSize(m_private->window, m_deviceConfig->size.x, m_deviceConfig->size.y);
    SDL_SetWindowFullscreen(m_private->window, m_deviceConfig->fullScreen ? SDL_WINDOW_FULLSCREEN : 0);

    TryToSetVSync();

    m_device->ConfigChanged(*m_deviceConfig);

    m_eventQueue->AddEvent(Event(EVENT_RESOLUTION_CHANGED));

    return true;
}

bool CApplication::OpenJoystick()
{
    if ( (m_joystick.index < 0) || (m_joystick.index >= SDL_NumJoysticks()) )
        return false;

    assert(m_private->joystick == nullptr);
    GetLogger()->Info("Opening joystick %%", m_joystick.index);

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

    // Initialize haptic subsystem
    m_private->haptic = SDL_HapticOpenFromJoystick(m_private->joystick);
    if (m_private->haptic == nullptr)
    {
        GetLogger()->Warn("Haptic subsystem open failed: %%", SDL_GetError());
        return true;
    }

    if (SDL_HapticRumbleInit(m_private->haptic) != 0)
    {
        GetLogger()->Warn("Haptic rumble effect init failed: %%", SDL_GetError());
        return true;
    }

    return true;
}

void CApplication::CloseJoystick()
{
    // Timer will remove itself automatically

    GetLogger()->Info("Closing joystick");

    StopForceFeedbackEffect();

    SDL_HapticClose(m_private->haptic);
    m_private->haptic = nullptr;

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

    if (m_joystickEnabled)
        return OpenJoystick();
    else
        return true;
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
    glm::ivec2 pos{};
    SDL_GetMouseState(&pos.x, &pos.y);
    m_input->MouseMove(pos);
}

int CApplication::Run()
{
    m_active = true;

    m_baseTimeStamp = TimeUtils::GetCurrentTimeStamp();
    m_lastTimeStamp = m_baseTimeStamp;
    m_curTimeStamp = m_baseTimeStamp;

    MoveMouse({ 0.5f, 0.5f }); // center mouse on start

    TimeStamp previousTimeStamp{};
    TimeStamp currentTimeStamp{};
    TimeStamp interpolatedTimeStamp{};

    while (true)
    {
        if (m_active)
        {
            CProfiler::StartPerformanceCounter(PCNT_ALL);
            CProfiler::StartPerformanceCounter(PCNT_EVENT_PROCESSING);
        }

        // To be sure no old event remains
        m_private->currentEvent.type = SDL_LASTEVENT;

        // Call SDL_PumpEvents() only once here
        // (SDL_PeepEvents() doesn't call it)
        if (m_active)
            SDL_PumpEvents();

        m_private->lastMouseMotionEvent.type = SDL_LASTEVENT;

        bool haveEvent = true;
        while (haveEvent)
        {
            haveEvent = false;

            int count = 0;
            // Use SDL_PeepEvents() if the app is active, so we can use idle time to
            // render the scene. Else, use SDL_WaitEvent() to avoid eating CPU time.
            if (m_active)
                count = SDL_PeepEvents(&m_private->currentEvent, 1, SDL_GETEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT);
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

                Event virtualEvent = CreateVirtualEvent(event);

                if (event.type != EVENT_NULL)
                    m_eventQueue->AddEvent(std::move(event));

                if (virtualEvent.type != EVENT_NULL)
                    m_eventQueue->AddEvent(std::move(virtualEvent));
            }
        }

        // Now, process the last received mouse motion
        if (m_private->lastMouseMotionEvent.type != SDL_LASTEVENT)
        {
            m_private->currentEvent = m_private->lastMouseMotionEvent;

            Event event = ProcessSystemEvent();

            if (event.type == EVENT_SYS_QUIT)
                goto end; // exit the loop

            if (event.type != EVENT_NULL)
                m_eventQueue->AddEvent(std::move(event));
        }

        // Enter game update & frame rendering only if active
        if (m_active)
        {
            while (! m_eventQueue->IsEmpty())
            {
                Event event = m_eventQueue->GetEvent();

                if (event.type == EVENT_SYS_QUIT || event.type == EVENT_QUIT)
                    goto end; // exit both loops

                LogEvent(event);

                m_input->EventProcess(event);

                bool passOn = true;
                if (m_engine != nullptr)
                    passOn = m_engine->ProcessEvent(event);

                if (passOn && m_controller != nullptr)
                    m_controller->ProcessEvent(event);
            }

            CProfiler::StopPerformanceCounter(PCNT_EVENT_PROCESSING);

            CProfiler::StartPerformanceCounter(PCNT_UPDATE_ALL);

            // Prepare and process step simulation event(s)
            // If game speed is increased then we do extra ticks per loop iteration to improve physics accuracy.
            int numTickSlices = static_cast<int>(GetSimulationSpeed());
            if(numTickSlices < 1) numTickSlices = 1;
            previousTimeStamp = m_curTimeStamp;
            currentTimeStamp = TimeUtils::GetCurrentTimeStamp();
            for(int tickSlice = 0; tickSlice < numTickSlices; tickSlice++)
            {
                interpolatedTimeStamp = TimeUtils::Lerp(previousTimeStamp, currentTimeStamp, (tickSlice+1)/static_cast<float>(numTickSlices));
                Event event = CreateUpdateEvent(interpolatedTimeStamp);
                if (event.type != EVENT_NULL && m_controller != nullptr)
                {
                    LogEvent(event);

                    m_sound->FrameMove(m_relTime);

                    CProfiler::StartPerformanceCounter(PCNT_UPDATE_GAME);
                    m_controller->ProcessEvent(event);
                    CProfiler::StopPerformanceCounter(PCNT_UPDATE_GAME);

                    CProfiler::StartPerformanceCounter(PCNT_UPDATE_ENGINE);
                    m_engine->FrameUpdate();
                    CProfiler::StopPerformanceCounter(PCNT_UPDATE_ENGINE);
                }
            }

            CProfiler::StopPerformanceCounter(PCNT_UPDATE_ALL);

            /* Update mouse position explicitly right before rendering
             * because mouse events are usually way behind */
            UpdateMouse();

            Render();

            CProfiler::StopPerformanceCounter(PCNT_ALL);
        }
    }

end:

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
    else if (m_private->currentEvent.type == SDL_WINDOWEVENT)
    {
        if (m_private->currentEvent.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
        {
            Gfx::DeviceConfig newConfig = *m_deviceConfig;
            newConfig.size.x = m_private->currentEvent.window.data1;
            newConfig.size.y = m_private->currentEvent.window.data2;
            if (newConfig.size != m_deviceConfig->size)
                ChangeVideoConfig(newConfig);
        }

        if (m_private->currentEvent.window.event == SDL_WINDOWEVENT_ENTER)
        {
            event.type = EVENT_MOUSE_ENTER;
        }

        if (m_private->currentEvent.window.event == SDL_WINDOWEVENT_LEAVE)
        {
            event.type = EVENT_MOUSE_LEAVE;
        }

        if (m_private->currentEvent.window.event == SDL_WINDOWEVENT_FOCUS_GAINED)
        {
            event.type = EVENT_FOCUS_GAINED;
        }

        if (m_private->currentEvent.window.event == SDL_WINDOWEVENT_FOCUS_LOST)
        {
            event.type = EVENT_FOCUS_LOST;
        }
    }
    else if ( (m_private->currentEvent.type == SDL_KEYDOWN) ||
              (m_private->currentEvent.type == SDL_KEYUP) )
    {
        if (m_private->currentEvent.type == SDL_KEYDOWN)
            event.type = EVENT_KEY_DOWN;
        else
            event.type = EVENT_KEY_UP;

        auto data = std::make_unique<KeyEventData>();

        data->virt = false;
        data->key = m_private->currentEvent.key.keysym.sym;
        event.kmodState = m_private->currentEvent.key.keysym.mod;

        // Some keyboards return numerical enter keycode instead of normal enter
        // See issue #427 for details
        if (data->key == KEY(KP_ENTER))
            data->key = KEY(RETURN);

        if (data->key == KEY(TAB) && ((event.kmodState & KEY_MOD(ALT)) != 0))
        {
            GetLogger()->Debug("Minimize to taskbar");
            SDL_MinimizeWindow(m_private->window);
            event.type = EVENT_NULL;
        }

        event.data = std::move(data);
    }
    else if (m_private->currentEvent.type == SDL_TEXTINPUT)
    {
        event.type = EVENT_TEXT_INPUT;
        auto data = std::make_unique<TextInputData>();
        data->text = m_private->currentEvent.text.text;
        event.data = std::move(data);
    }
    else if (m_private->currentEvent.type == SDL_MOUSEWHEEL)
    {
        event.type = EVENT_MOUSE_WHEEL;

        auto data = std::make_unique<MouseWheelEventData>();
        data->y = m_private->currentEvent.wheel.y;
        data->x = m_private->currentEvent.wheel.x;

        event.data = std::move(data);
    }
    else if ( (m_private->currentEvent.type == SDL_MOUSEBUTTONDOWN) ||
         (m_private->currentEvent.type == SDL_MOUSEBUTTONUP) )
    {
        auto data = std::make_unique<MouseButtonEventData>();

        if (m_private->currentEvent.type == SDL_MOUSEBUTTONDOWN)
            event.type = EVENT_MOUSE_BUTTON_DOWN;
        else
            event.type = EVENT_MOUSE_BUTTON_UP;

        data->button = static_cast<MouseButton>(1 << m_private->currentEvent.button.button);

        event.data = std::move(data);
    }
    else if (m_private->currentEvent.type == SDL_MOUSEMOTION)
    {
        event.type = EVENT_MOUSE_MOVE;

        m_input->MouseMove({ m_private->currentEvent.button.x, m_private->currentEvent.button.y });
    }
    else if (m_private->currentEvent.type == SDL_JOYAXISMOTION)
    {
        event.type = EVENT_JOY_AXIS;

        auto data = std::make_unique<JoyAxisEventData>();
        data->axis = m_private->currentEvent.jaxis.axis;
        data->value = m_private->currentEvent.jaxis.value;
        event.data = std::move(data);
    }
    else if ( (m_private->currentEvent.type == SDL_JOYBUTTONDOWN) ||
              (m_private->currentEvent.type == SDL_JOYBUTTONUP) )
    {
        if (m_private->currentEvent.type == SDL_JOYBUTTONDOWN)
            event.type = EVENT_JOY_BUTTON_DOWN;
        else
            event.type = EVENT_JOY_BUTTON_UP;

        auto data = std::make_unique<JoyButtonEventData>();
        data->button = m_private->currentEvent.jbutton.button;
        event.data = std::move(data);
    }

    return event;
}

void CApplication::LogEvent(const Event &event)
{
    CLogger *l = GetLogger();

    auto PrintEventDetails = [&]()
    {
        l->Trace(" rTime = %%", event.rTime);
        l->Trace(" kmodState = %%", event.kmodState);
        l->Trace(" mousePos = %%, %%", event.mousePos.x, event.mousePos.y);
        l->Trace(" mouseButtonsState = %%", event.mouseButtonsState);
        l->Trace(" customParam = %%", event.customParam);
    };

    // Print the events in debug mode to test the code
    if (IsDebugModeActive(DEBUG_SYS_EVENTS) || IsDebugModeActive(DEBUG_UPDATE_EVENTS) || IsDebugModeActive(DEBUG_APP_EVENTS))
    {
        std::string eventType = ParseEventType(event.type);

        if (IsDebugModeActive(DEBUG_UPDATE_EVENTS) && event.type == EVENT_FRAME)
        {
            l->Trace("Update event: %%", eventType);
            PrintEventDetails();
        }

        if (IsDebugModeActive(DEBUG_SYS_EVENTS) && (event.type <= EVENT_SYS_MAX && event.type != EVENT_FRAME))
        {
            l->Trace("System event %%:", eventType);
            switch (event.type)
            {
                case EVENT_KEY_DOWN:
                case EVENT_KEY_UP:
                {
                    auto data = event.GetData<KeyEventData>();
                    l->Trace(" virt    = %%", data->virt);
                    l->Trace(" key     = %%", data->key);
                    break;
                }
                case EVENT_TEXT_INPUT:
                {
                    auto data = event.GetData<TextInputData>();
                    l->Trace(" text = %%", data->text);
                    break;
                }
                case EVENT_MOUSE_BUTTON_DOWN:
                case EVENT_MOUSE_BUTTON_UP:
                {
                    auto data = event.GetData<MouseButtonEventData>();
                    l->Trace(" button = %%", data->button);
                    break;
                }
                case EVENT_MOUSE_WHEEL:
                {
                    auto data = event.GetData<MouseWheelEventData>();
                    l->Trace(" y = %%", data->y);
                    l->Trace(" x = %%", data->x);
                    break;
                }
                case EVENT_JOY_AXIS:
                {
                    auto data = event.GetData<JoyAxisEventData>();
                    l->Trace(" axis  = %%", data->axis);
                    l->Trace(" value = %%", data->value);
                    break;
                }
                case EVENT_JOY_BUTTON_DOWN:
                case EVENT_JOY_BUTTON_UP:
                {
                    auto data = event.GetData<JoyButtonEventData>();
                    l->Trace(" button = %%", data->button);
                    break;
                }
                default:
                    break;
            }

            PrintEventDetails();
        }

        if (IsDebugModeActive(DEBUG_APP_EVENTS) && event.type > EVENT_SYS_MAX)
        {
            l->Trace("App event %%:", eventType);
            PrintEventDetails();
        }
    }
}


Event CApplication::CreateVirtualEvent(const Event& sourceEvent)
{
    Event virtualEvent;

    if ((sourceEvent.type == EVENT_KEY_DOWN) || (sourceEvent.type == EVENT_KEY_UP))
    {
        auto sourceData = sourceEvent.GetData<KeyEventData>();
        auto virtualKey = GetVirtualKey(sourceData->key, sourceEvent.kmodState);

        if (virtualKey == sourceData->key)
        {
            virtualEvent.type = EVENT_NULL;
        }
        else
        {
            virtualEvent.type = sourceEvent.type;

            auto data = sourceData->Clone();
            auto keyData = static_cast<KeyEventData*>(data.get());
            keyData->key = virtualKey;
            keyData->virt = true;
            virtualEvent.data = std::move(data);
        }
    }
    else if ((sourceEvent.type == EVENT_JOY_BUTTON_DOWN) || (sourceEvent.type == EVENT_JOY_BUTTON_UP))
    {
        if (sourceEvent.type == EVENT_JOY_BUTTON_DOWN)
            virtualEvent.type = EVENT_KEY_DOWN;
        else
            virtualEvent.type = EVENT_KEY_UP;

        auto sourceData = sourceEvent.GetData<JoyButtonEventData>();

        auto data = std::make_unique<KeyEventData>();
        data->virt = true;
        data->key = VIRTUAL_JOY(sourceData->button);
        virtualEvent.data = std::move(data);
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
    CProfiler::StartPerformanceCounter(PCNT_RENDER_ALL);
    m_engine->Render();
    CProfiler::StopPerformanceCounter(PCNT_RENDER_ALL);

    CProfiler::StartPerformanceCounter(PCNT_SWAP_BUFFERS);
    if (m_deviceConfig->doubleBuf)
        SDL_GL_SwapWindow(m_private->window);
    CProfiler::StopPerformanceCounter(PCNT_SWAP_BUFFERS);
}

void CApplication::RenderIfNeeded(int updateRate)
{
    m_manualFrameTime = TimeUtils::GetCurrentTimeStamp();
    long long diff = TimeUtils::ExactDiff(m_manualFrameLast, m_manualFrameTime);
    if (diff < 1e9f / updateRate)
    {
        return;
    }
    m_manualFrameLast = m_manualFrameTime;

    Render();
}

void CApplication::SuspendSimulation()
{
    m_simulationSuspended = true;
    GetLogger()->Info("Suspend simulation");
}

void CApplication::ResumeSimulation()
{
    m_simulationSuspended = false;
    InternalResumeSimulation();

    GetLogger()->Info("Resume simulation");
}

void CApplication::ResetTimeAfterLoading()
{
    InternalResumeSimulation();

    GetLogger()->Trace("Resume simulation on loading");
}

void CApplication::InternalResumeSimulation()
{
    m_baseTimeStamp = TimeUtils::GetCurrentTimeStamp();
    m_curTimeStamp = m_baseTimeStamp;
    m_realAbsTimeBase = m_realAbsTime;
    m_absTimeBase = m_exactAbsTime;
}

void CApplication::StartLoadingMusic()
{
    std::thread{[this]()
    {
        GetLogger()->Debug("Cache sounds...");
        TimeStamp musicLoadStart{TimeUtils::GetCurrentTimeStamp()};

        m_sound->Reset();
        m_sound->CacheAll();

        TimeStamp musicLoadEnd{TimeUtils::GetCurrentTimeStamp()};
        float musicLoadTime = TimeUtils::Diff<TimeUnit::MILLISECONDS>(musicLoadStart, musicLoadEnd);
        GetLogger()->Debug("Sound loading took %% ms", static_cast<int>(musicLoadTime));
    }}.detach();
}

bool CApplication::GetSimulationSuspended() const
{
    return m_simulationSuspended;
}

void CApplication::SetSimulationSpeed(float speed)
{
    m_simulationSpeed = speed;

    m_baseTimeStamp = m_curTimeStamp;
    m_realAbsTimeBase = m_realAbsTime;
    m_absTimeBase = m_exactAbsTime;

    GetLogger()->Info("Simulation speed = %%", speed);
}

Event CApplication::CreateUpdateEvent(TimeStamp newTimeStamp)
{
    if (m_simulationSuspended)
        return Event(EVENT_NULL);

    m_lastTimeStamp = m_curTimeStamp;
    m_curTimeStamp = newTimeStamp;

    long long absDiff = TimeUtils::ExactDiff(m_baseTimeStamp, m_curTimeStamp);
    long long newRealAbsTime = m_realAbsTimeBase + absDiff;
    long long newRealRelTime = TimeUtils::ExactDiff(m_lastTimeStamp, m_curTimeStamp);

    if (newRealAbsTime < m_realAbsTime || newRealRelTime < 0)
    {
        GetLogger()->Error("Fatal error: got negative system counter difference!");
        GetLogger()->Error("This should never happen. Please report this error.");
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
    frameEvent.rTime = m_relTime;
    m_input->EventProcess(frameEvent);

    return frameEvent;
}

float CApplication::GetSimulationSpeed() const
{
    return m_simulationSpeed;
}

TimeUtils::TimeStamp CApplication::GetBaseTimeStamp() const
{
    return m_baseTimeStamp;
}

TimeUtils::TimeStamp CApplication::GetLastTimeStamp() const
{
    return m_lastTimeStamp;
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

const Gfx::DeviceConfig& CApplication::GetVideoConfig() const
{
    return *m_deviceConfig;
}

std::vector<glm::ivec2> CApplication::GetVideoResolutionList(int display) const
{
    std::vector<glm::ivec2> resolutions;

    for(int i = 0; i < SDL_GetNumDisplayModes(display); i++)
    {
        SDL_DisplayMode mode;
        SDL_GetDisplayMode(display, i, &mode);
        glm::ivec2 resolution = { mode.w, mode.h };

        if (std::find(resolutions.begin(), resolutions.end(), resolution) == resolutions.end())
            resolutions.push_back(resolution);
    }

    return resolutions;
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

    std::istringstream stream(str);
    std::string modeToken;

    while (std::getline(stream, modeToken, ','))
    {
        if (modeToken == "sys_events")
        {
            debugModes |= DEBUG_SYS_EVENTS;
        }
        else if (modeToken == "update_events")
        {
            debugModes |= DEBUG_UPDATE_EVENTS;
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
            GetLogger()->Error("Invalid debug mode: '%%'", modeToken);
            return false;
        }
    }

    return true;
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

void CApplication::MoveMouse(const glm::vec2& pos)
{
    glm::ivec2 windowPos = m_engine->InterfaceToWindowCoords(pos);
    m_input->MouseMove(windowPos);
    SDL_WarpMouseInWindow(m_private->window, windowPos.x, windowPos.y);
}

std::vector<JoystickDevice> CApplication::GetJoystickList() const
{
    std::vector<JoystickDevice> result;

    int count = SDL_NumJoysticks();

    for (int index = 0; index < count; ++index)
    {
        JoystickDevice device;
        device.index = index;
        device.name = SDL_JoystickNameForIndex(index);
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

        case LANGUAGE_CZECH:
            langChar = 'C';
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

        case LANGUAGE_PORTUGUESE_BRAZILIAN:
            langChar = 'B';
            break;

    }
    return langChar;
}

std::filesystem::path CApplication::GetLanguageDir() const
{
    return StrUtils::ToPath(std::string(1, GetLanguageChar()));
}

void CApplication::SetLanguage(Language language)
{
    m_language = language;

    /* Gettext initialization */

    static std::string envLang = "";

    if (envLang.empty())
    {
        // Get this only at the first call, since this code modifies it
        const char* currentEnvLang = gl_locale_name(LC_MESSAGES, "LC_MESSAGES");
        if (currentEnvLang != nullptr)
        {
            // Take only the two first chars; to support "fr_CH" or "en_US;en"
            envLang = std::string(currentEnvLang).substr(0,2);
        }
    }

    if (language == LANGUAGE_ENV)
    {
        if (envLang.empty())
        {
            GetLogger()->Error("Failed to get language from environment, setting default language");
            m_language = LANGUAGE_ENGLISH;
        }
        else
        {
            GetLogger()->Trace("gl_locale_name: '%%'", envLang);

            if (envLang == "en")
            {
                m_language = LANGUAGE_ENGLISH;
            }
            else if (envLang == "cs")
            {
                m_language = LANGUAGE_CZECH;
            }
            else if (envLang == "de")
            {
                m_language = LANGUAGE_GERMAN;
            }
            else if (envLang == "fr")
            {
                m_language = LANGUAGE_FRENCH;
            }
            else if (envLang == "pl")
            {
                m_language = LANGUAGE_POLISH;
            }
            else if (envLang == "ru")
            {
                m_language = LANGUAGE_RUSSIAN;
            }
            else if (envLang == "pt")
            {
                m_language = LANGUAGE_PORTUGUESE_BRAZILIAN;
            }
            else
            {
                GetLogger()->Warn("Environment locale ('%%') is not supported, setting default language", envLang);
                m_language = LANGUAGE_ENGLISH;
            }
        }
    }

    std::string locale = "";
    switch (m_language)
    {
        default:
            locale = "";
            break;

        case LANGUAGE_CZECH:
            locale = "cs_CZ.utf8";
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

        case LANGUAGE_PORTUGUESE_BRAZILIAN:
            locale = "pt_BR.utf8";
            break;
    }

    std::string langStr = "LANGUAGE=";
    langStr += locale;

    std::copy_n(langStr.data(), langStr.size() + 1, m_languageLocale.data());

    putenv(m_languageLocale.data());

    GetLogger()->Trace("SetLanguage: Set LANGUAGE=%% in environment", locale);

    char* defaultLocale = setlocale(LC_ALL, ""); // Load system locale
    GetLogger()->Debug("Default system locale: %%", defaultLocale);
    if (!locale.empty()) // Override system locale?
    {
        setlocale(LC_ALL, locale.c_str());
    }
    setlocale(LC_NUMERIC, "C"); // Force numeric locale to "C" (fixes decimal point problems)
    std::string systemLocale = setlocale(LC_ALL, nullptr); // Get current locale configuration
    GetLogger()->Debug("Setting locale: %%", systemLocale);
    // Update C++ locale
    try
    {
#if defined(_MSC_VER) && defined(_DEBUG)
        // Avoids failed assertion in VS debugger
        throw -1;
#else
        std::locale::global(std::locale(systemLocale.c_str()));
#endif
    }
    catch (...)
    {
        GetLogger()->Warn("Failed to update locale, possibly incorrect system configuration. Will fallback to classic locale.");
        try
        {
            std::locale::global(std::locale::classic());
        }
        catch (...)
        {
            GetLogger()->Warn("Failed to set classic locale. Something is really messed up in your system configuration. Translations might not work.");
        }

        // C locale might still work correctly
        setlocale(LC_ALL, "");
        setlocale(LC_NUMERIC, "C");
    }

    bindtextdomain("colobot", StrUtils::ToString(m_pathManager->GetLangPath()).c_str());
    bind_textdomain_codeset("colobot", "UTF-8");
    textdomain("colobot");

    GetLogger()->Debug("SetLanguage: Test gettext translation: '%%'", gettext("Colobot rules!"));
}

bool CApplication::GetSceneTestMode()
{
    return m_sceneTest;
}

void CApplication::SetTextInput(bool textInputEnabled, int id)
{
    m_textInputEnabled[id] = textInputEnabled;
    if (std::any_of(m_textInputEnabled.begin(), m_textInputEnabled.end(), [](std::pair<int, bool> v) { return v.second; }))
    {
        SDL_StartTextInput();
    }
    else
    {
        SDL_StopTextInput();
    }
}

void CApplication::PlayForceFeedbackEffect(float strength, int length)
{
    if (m_private->haptic == nullptr) return;

    GetLogger()->Trace("Force feedback! length = %% ms, strength = %%", length, strength);
    if (SDL_HapticRumblePlay(m_private->haptic, strength, length) != 0)
    {
        GetLogger()->Debug("Failed to play haptic effect: %%", SDL_GetError());
    }
}

void CApplication::StopForceFeedbackEffect()
{
    if (m_private->haptic == nullptr) return;
    SDL_HapticRumbleStop(m_private->haptic);
}
