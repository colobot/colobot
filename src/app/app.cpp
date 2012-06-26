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


#include <SDL/SDL.h>
#include <SDL/SDL_image.h>


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
    int joystickDevice;

    ApplicationPrivate()
    {
        memset(&currentEvent, 0, sizeof(SDL_Event));
        surface = NULL;
        joystick = NULL;
        joystickDevice = 0;
    }
};


CApplication::CApplication()
{
    m_private = new ApplicationPrivate();
    m_exitCode = 0;

    m_iMan = new CInstanceManager();
    m_event = new CEvent(m_iMan);

    m_engine = 0;
    m_robotMain = 0;
    m_sound     = 0;

    m_keyState = 0;
    m_axeKey = Math::Vector(0.0f, 0.0f, 0.0f);
    m_axeJoy = Math::Vector(0.0f, 0.0f, 0.0f);

    m_vidMemTotal  = 0;
    m_active      = false;
    m_activateApp = false;
    m_ready       = false;
    m_joystick    = false;
    m_time        = 0.0f;

    for (int i = 0; i < 32; i++)
    {
        m_joyButton[i] = false;
    }

    m_windowTitle  = "COLOBOT";

    m_appUseZBuffer  = true;
    m_appUseStereo   = true;
    m_showStats      = false;
    m_debugMode      = false;
    m_audioState     = true;
    m_audioTrack     = true;
    m_niceMouse      = false;
    m_setupMode      = true;

    ResetKey();
}

CApplication::~CApplication()
{
    delete m_private;
    m_private = NULL;

    delete m_iMan;
    m_iMan = NULL;
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
        else if (arg == "-audiostate")
        {
            m_audioState = false;
        }
        else if (arg == "-audiotrack")
        {
            m_audioTrack = false;
        }
        // TODO else {} report invalid argument
    }

    return ERR_OK;
}

bool CApplication::Create()
{
/*
TODO
    Full screen by default unless in debug mode
    if (! m_debugMode)
        m_deviceConfig.fullScreen = true;

    int full = 0;
    if (GetProfileInt("Device", "FullScreen", full))
        m_deviceConfig.fullScreen = full == 1;
*/

    // Temporarily -- only in windowed mode
    m_deviceConfig.fullScreen = false;

/*
TODO
    // Create the 3D engine.
    m_engine = new CEngine(m_iMan, this);

    // Initialize the app's custom scene stuff
    if (! m_engine->OneTimeSceneInit())
    {
        SystemDialog(SDT_ERROR, "COLOBOT - Error", m_engine->RetError());
        return false;
    }

    // Create the sound instance.
    m_sound = new CSound(m_iMan);

    // Create the robot application.
    m_robotMain = new CRobotMain(m_iMan);
*/


    Uint32 initFlags = SDL_INIT_VIDEO;
    if (m_joystick)
        initFlags |= SDL_INIT_JOYSTICK;

    if (SDL_Init(initFlags) < 0)
    {
        SystemDialog( SDT_ERROR, "COLOBOT - Error", "SDL initialization error:\n" +  std::string(SDL_GetError()) );
        return false;
    }

    const SDL_VideoInfo *videoInfo = SDL_GetVideoInfo();
    if (! videoInfo)
    {
        SystemDialog( SDT_ERROR, "COLOBOT - Error", "SDL error while getting video info:\n " + std::string(SDL_GetError()) );
        return false;
    }

    Uint32 videoFlags = SDL_OPENGL | SDL_GL_DOUBLEBUFFER | SDL_HWPALETTE;

    if (m_deviceConfig.resizeable)
        videoFlags |= SDL_RESIZABLE;

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

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);

    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) == 0)
    {
        SystemDialog( SDT_ERROR, "COLOBOT - Error", std::string("SDL_Image initialization error:\n") +
                                                    std::string(IMG_GetError()) );
        return false;
    }

    m_private->surface = SDL_SetVideoMode(m_deviceConfig.width, m_deviceConfig.height,
                                          m_deviceConfig.bpp, videoFlags);

    if (! m_private->surface)
    {
        SystemDialog( SDT_ERROR, "COLOBT - Error", std::string("SDL error while setting video mode:\n") +
                                                   std::string(SDL_GetError()) );
        return false;
    }

    SDL_WM_SetCaption(m_windowTitle.c_str(), m_windowTitle.c_str());

    SDL_EnableUNICODE(1);


/*
TODO

    InitJoystick();

    if ( !GetProfileInt("Setup", "Sound3D", b3D) )
    {
        b3D = true;
    }
    m_pSound->SetDebugMode(m_bDebugMode);
    m_pSound->Create(m_hWnd, b3D);
    m_pSound->CacheAll();
    m_pSound->SetState(m_bAudioState);
    m_pSound->SetAudioTrack(m_bAudioTrack);
    m_pSound->SetCDpath(m_CDpath);

    // First execution?
    if ( !GetProfileInt("Setup", "ObjectDirty", iValue) )
    {
        m_pD3DEngine->FirstExecuteAdapt(true);
    }

    // Creates the file colobot.ini at the first execution.
    m_pRobotMain->CreateIni();

    m_pRobotMain->ChangePhase(PHASE_WELCOME2);

    m_engine->TimeInit();
*/

    // The app is ready to go
    m_ready = true;

    return true;
}

void CApplication::Destroy()
{
    if (m_private->joystick != NULL)
    {
        SDL_JoystickClose(m_private->joystick);
        m_private->joystick = NULL;
    }

    SDL_FreeSurface(m_private->surface);
    m_private->surface = NULL;

    IMG_Quit();

    SDL_Quit();
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
            while (m_event->GetEvent(event))
            {
                if (event.event == EVENT_QUIT)
                {
                    goto end; // exit both loops
                }

                //m_robotMain->EventProcess(event);
            }

            //if ( !RetNiceMouse())
            //{
            //  SetMouseType(m_engine->RetMouseType());
            //}

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

void CApplication::ParseEvent()
{
/*  Event event;

    if (m_private->currentEvent.type == SDL_MOUSEBUTTONDOWN)
    {
        if (m_private->currentEvent.button.button == SDL_BUTTON_LEFT)
            event.event = EVENT_LBUTTONDOWN;
        else if (m_private->currentEvent.button.button == SDL_BUTTON_RIGHT)
            event.event = EVENT_RBUTTONDOWN;
    }
    else if (m_private->currentEvent.type == SDL_MOUSEBUTTONUP)
    {
        if (m_private->currentEvent.button.button == SDL_BUTTON_LEFT)
            event.event = EVENT_LBUTTONUP;
        else if (m_private->currentEvent.button.button == SDL_BUTTON_RIGHT)
            event.event = EVENT_RBUTTONUP;
    }
    else if (m_private->currentEvent.type == SDL_MOUSEMOTION)
    {
        event.event = EVENT_MOUSEMOVE;
    }
    else if (m_private->currentEvent.type == SDL_KEYDOWN)
    {
        event.event = EVENT_KEYDOWN;
    }
    else if (m_private->currentEvent.type == SDL_KEYUP)
    {
        event.event = EVENT_KEYUP;
    }

    if (m_robotMain != NULL && event.event != EVENT_NULL)
    {
        m_robotMain->EventProcess(event);
    }
    if (m_engine != NULL)
    {
        m_engine->MsgProc( hWnd, uMsg, wParam, lParam );
    }

    ProcessEvent(event);*/
}

void CApplication::ProcessEvent(Event event)
{
    
}

bool CApplication::Render()
{
    bool result = m_engine->Render();
    if (! result)
        return false;

    if (m_deviceConfig.doubleBuf)
        SDL_GL_SwapBuffers();

    return true;
}

void CApplication::Pause(bool pause)
{
    // TODO
}

void CApplication::SetMousePos(Math::Point pos)
{
    // TODO
}

void CApplication::StepSimulation(float rTime)
{
    // TODO
}

void SetShowStat(bool show)
{
    // TODO
}

bool CApplication::RetShowStat()
{
    // TODO
    return false;
}

void CApplication::SetDebugMode(bool mode)
{
    // TODO
}

bool CApplication::RetDebugMode()
{
    // TODO
    return false;
}

bool CApplication::RetSetupMode()
{
    // TODO
    return false;
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

int CApplication::RetKey(int keyRank, int option)
{
    // TODO
    return 0;
}

void CApplication::SetJoystick(bool enable)
{
    // TODO
}

bool CApplication::RetJoystick()
{
    // TODO
    return false;
}

void SetMouseType(Gfx::MouseType type)
{
    // TODO
}

void SetNiceMouse(bool nice)
{
    // TODO
}

bool CApplication::RetNiceMouse()
{
    return false;
}

bool CApplication::RetNiceMouseCap()
{
    return false;
}

bool CApplication::WriteScreenShot(char *filename, int width, int height)
{
    // TODO
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
