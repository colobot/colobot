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

#include "level/robotmain.h"

#include "CBot/CBot.h"

#include "app/app.h"
#include "app/input.h"
#include "app/pausemanager.h"

#include "common/config_file.h"
#include "common/event.h"
#include "common/logger.h"
#include "common/make_unique.h"
#include "common/restext.h"
#include "common/settings.h"
#include "common/stringutils.h"

#include "common/resources/inputstream.h"
#include "common/resources/outputstream.h"
#include "common/resources/resourcemanager.h"

#include "graphics/engine/camera.h"
#include "graphics/engine/cloud.h"
#include "graphics/engine/engine.h"
#include "graphics/engine/lightman.h"
#include "graphics/engine/lightning.h"
#include "graphics/engine/oldmodelmanager.h"
#include "graphics/engine/particle.h"
#include "graphics/engine/planet.h"
#include "graphics/engine/pyro_manager.h"
#include "graphics/engine/terrain.h"
#include "graphics/engine/text.h"
#include "graphics/engine/water.h"

#include "graphics/model/model_manager.h"

#include "level/mainmovie.h"
#include "level/player_profile.h"
#include "level/scene_conditions.h"
#include "level/scoreboard.h"

#include "level/parser/parser.h"

#include "math/const.h"
#include "math/func.h"
#include "math/geometry.h"

#include "object/object.h"
#include "object/object_create_exception.h"
#include "object/object_manager.h"

#include "object/auto/auto.h"

#include "object/interface/slotted_object.h"

#include "object/motion/motion.h"
#include "object/motion/motionhuman.h"
#include "object/motion/motiontoto.h"

#include "object/subclass/exchange_post.h"

#include "object/task/task.h"
#include "object/task/taskbuild.h"
#include "object/task/taskmanip.h"

#include "physics/physics.h"

#include "script/cbottoken.h"
#include "script/script.h"
#include "script/scriptfunc.h"

#include "sound/sound.h"

#include "ui/debug_menu.h"
#include "ui/displayinfo.h"
#include "ui/displaytext.h"
#include "ui/maindialog.h"
#include "ui/mainmap.h"
#include "ui/mainshort.h"
#include "ui/mainui.h"

#include "ui/controls/button.h"
#include "ui/controls/edit.h"
#include "ui/controls/group.h"
#include "ui/controls/interface.h"
#include "ui/controls/label.h"
#include "ui/controls/map.h"
#include "ui/controls/shortcut.h"
#include "ui/controls/slider.h"
#include "ui/controls/window.h"

#include "ui/screen/screen_loading.h"

#include <algorithm>
#include <iomanip>
#include <stdexcept>
#include <cmath>
#include <ctime>

#include <boost/lexical_cast.hpp>


// Global variables.

const float UNIT = 4.0f;    // default for g_unit
float   g_unit;             // conversion factor

// Min/max values for the game speed.
const float MIN_SPEED = 1/8.0f;
const float MAX_SPEED = 256.0f;

// Reference colors used when recoloring textures, see ChangeColor()
const Gfx::Color COLOR_REF_BOT   = Gfx::Color( 10.0f/256.0f, 166.0f/256.0f, 254.0f/256.0f);  // blue
const Gfx::Color COLOR_REF_ALIEN = Gfx::Color(135.0f/256.0f, 170.0f/256.0f,  13.0f/256.0f);  // green
const Gfx::Color COLOR_REF_GREEN = Gfx::Color(135.0f/256.0f, 170.0f/256.0f,  13.0f/256.0f);  // green
const Gfx::Color COLOR_REF_WATER = Gfx::Color( 25.0f/256.0f, 255.0f/256.0f, 240.0f/256.0f);  // cyan

//! Constructor of robot application
CRobotMain::CRobotMain()
{
    m_app        = CApplication::GetInstancePointer();

    m_eventQueue = m_app->GetEventQueue();
    m_sound      = m_app->GetSound();

    m_engine     = Gfx::CEngine::GetInstancePointer();
    m_oldModelManager = m_engine->GetModelManager();
    m_lightMan   = m_engine->GetLightManager();
    m_particle   = m_engine->GetParticle();
    m_water      = m_engine->GetWater();
    m_cloud      = m_engine->GetCloud();
    m_lightning  = m_engine->GetLightning();
    m_planet     = m_engine->GetPlanet();
    m_input      = CInput::GetInstancePointer();

    m_modelManager = MakeUnique<Gfx::CModelManager>();
    m_settings    = MakeUnique<CSettings>();
    m_pause       = MakeUnique<CPauseManager>();
    m_interface   = MakeUnique<Ui::CInterface>();
    m_terrain     = MakeUnique<Gfx::CTerrain>();
    m_camera      = MakeUnique<Gfx::CCamera>();
    m_displayText = MakeUnique<Ui::CDisplayText>();
    m_movie       = MakeUnique<CMainMovie>();
    m_ui          = MakeUnique<Ui::CMainUserInterface>();
    m_short       = MakeUnique<Ui::CMainShort>();
    m_map         = MakeUnique<Ui::CMainMap>();

    m_objMan = MakeUnique<CObjectManager>(
        m_engine,
        m_terrain.get(),
        m_oldModelManager,
        m_modelManager.get(),
        m_particle);

    m_debugMenu   = MakeUnique<Ui::CDebugMenu>(this, m_engine, m_objMan.get(), m_sound);

    m_time = 0.0f;
    m_gameTime = 0.0f;
    m_gameTimeAbsolute = 0.0f;

    m_levelCategory = LevelCategory::Exercises;
    m_levelChap = 0;
    m_levelRank = 0;
    m_sceneReadPath = "";

    m_missionTimerEnabled = false;
    m_missionTimerStarted = false;
    m_missionTimer = 0.0f;

    m_phase       = PHASE_PLAYER_SELECT;
    m_visitLast   = EVENT_NULL;
    m_visitObject = nullptr;
    m_visitArrow  = nullptr;
    m_audioTrack  = "";
    m_audioRepeat = true;
    m_satcomTrack  = "";
    m_satcomRepeat = true;
    m_editorTrack  = "";
    m_editorRepeat = true;
    m_selectObject = nullptr;
    m_infoUsed     = 0;

    m_controller   = nullptr;
    m_missionType  = MISSION_NORMAL;
    m_immediatSatCom = false;
    m_beginSatCom  = false;
    m_lockedSatCom = false;
    m_movieLock    = false;
    m_satComLock   = false;
    m_editLock     = false;
    m_editFull     = false;
    m_hilite       = false;
    m_cheatSelectInsect = false;
    m_cheatShowSoluce   = false;

    m_codeBattleInit = false;
    m_codeBattleStarted = false;

    m_teamNames.clear();

    #if DEV_BUILD
    m_cheatAllMission      = true; // for development
    #else
    m_cheatAllMission      = false;
    #endif

    m_cheatRadar   = false;
    m_fixScene     = false;
    m_cheatTrainerPilot = false;
    m_friendAim    = false;
    m_resetCreate  = false;
    m_shortCut     = true;

    m_commandHistoryIndex = -1;

    m_movieInfoIndex = -1;

    m_tooltipPos = Math::Point(0.0f, 0.0f);
    m_tooltipName.clear();
    m_tooltipTime = 0.0f;

    m_winTerminate   = false;

    m_globalMagnifyDamage = 1.0f;

    m_autosave = true;
    m_autosaveInterval = 5;
    m_autosaveSlots = 3;
    m_autosaveLast = 0.0f;

    m_shotSaving = 0;

    m_build = 0;
    m_researchDone.clear();  // no research done
    m_researchDone[0] = 0;
    m_researchEnable = 0;
    g_unit = UNIT;

    for (int i = 0; i < MAXSHOWLIMIT; i++)
    {
        m_showLimit[i].used = false;
        m_showLimit[i].total = 0;
        m_showLimit[i].link = nullptr;
    }

    m_debugCrashSpheres = false;

    m_engine->SetTerrain(m_terrain.get());

    m_app->SetMouseMode(MOUSE_ENGINE);

    m_movie->Flush();

    FlushDisplayInfo();

    InitEye();

    m_engine->SetTracePrecision(1.0f);

    m_settings->LoadSettings();
    m_settings->SaveSettings();
    m_settings->SaveResolutionSettings(m_app->GetVideoConfig());

    SelectPlayer(CPlayerProfile::GetLastName());

    CScriptFunctions::Init();
}

//! Destructor of robot application
CRobotMain::~CRobotMain()
{
}

Gfx::CCamera* CRobotMain::GetCamera()
{
    return m_camera.get();
}

Gfx::CTerrain* CRobotMain::GetTerrain()
{
    return m_terrain.get();
}

Ui::CInterface* CRobotMain::GetInterface()
{
    return m_interface.get();
}

Ui::CDisplayText* CRobotMain::GetDisplayText()
{
    return m_displayText.get();
}

CPauseManager* CRobotMain::GetPauseManager()
{
    return m_pause.get();
}

std::string PhaseToString(Phase phase)
{
    if (phase == PHASE_WELCOME1) return "PHASE_WELCOME1";
    if (phase == PHASE_WELCOME2) return "PHASE_WELCOME2";
    if (phase == PHASE_WELCOME3) return "PHASE_WELCOME3";
    if (phase == PHASE_PLAYER_SELECT) return "PHASE_PLAYER_SELECT";
    if (phase == PHASE_APPEARANCE) return "PHASE_APPEARANCE";
    if (phase == PHASE_MAIN_MENU) return "PHASE_MAIN_MENU";
    if (phase == PHASE_LEVEL_LIST) return "PHASE_LEVEL_LIST";
    if (phase == PHASE_MOD_LIST) return "PHASE_MOD_LIST";
    if (phase == PHASE_SIMUL) return "PHASE_SIMUL";
    if (phase == PHASE_SETUPd) return "PHASE_SETUPd";
    if (phase == PHASE_SETUPg) return "PHASE_SETUPg";
    if (phase == PHASE_SETUPp) return "PHASE_SETUPp";
    if (phase == PHASE_SETUPc) return "PHASE_SETUPc";
    if (phase == PHASE_SETUPs) return "PHASE_SETUPs";
    if (phase == PHASE_SETUPds) return "PHASE_SETUPds";
    if (phase == PHASE_SETUPgs) return "PHASE_SETUPgs";
    if (phase == PHASE_SETUPps) return "PHASE_SETUPps";
    if (phase == PHASE_SETUPcs) return "PHASE_SETUPcs";
    if (phase == PHASE_SETUPss) return "PHASE_SETUPss";
    if (phase == PHASE_WRITEs) return "PHASE_WRITEs";
    if (phase == PHASE_READ) return "PHASE_READ";
    if (phase == PHASE_READs) return "PHASE_READs";
    if (phase == PHASE_WIN) return "PHASE_WIN";
    if (phase == PHASE_LOST) return "PHASE_LOST";
    if (phase == PHASE_QUIT_SCREEN) return "PHASE_QUIT_SCREEN";
    if (phase == PHASE_SATCOM) return "PHASE_SATCOM";
    return "(unknown)";
}

bool IsInSimulationConfigPhase(Phase phase)
{
    return (phase >= PHASE_SETUPds && phase <= PHASE_SETUPss) || phase == PHASE_READs || phase == PHASE_WRITEs;
}

bool IsPhaseWithWorld(Phase phase)
{
    if (phase == PHASE_SIMUL    ) return true;
    if (phase == PHASE_WIN      ) return true;
    if (phase == PHASE_LOST     ) return true;
    if (phase == PHASE_APPEARANCE) return true;
    if (IsInSimulationConfigPhase(phase)) return true;
    return false;
}

bool IsMainMenuPhase(Phase phase)
{
    if (phase == PHASE_APPEARANCE) return true;
    return !IsPhaseWithWorld(phase);
}

//! Changes phase
void CRobotMain::ChangePhase(Phase phase)
{
    bool resetWorld = false;
    if ((IsPhaseWithWorld(m_phase) || IsPhaseWithWorld(phase)) && !IsInSimulationConfigPhase(m_phase) && !IsInSimulationConfigPhase(phase))
    {
        if (IsPhaseWithWorld(m_phase) && !IsPhaseWithWorld(phase) && m_exitAfterMission)
        {
            GetLogger()->Info("Mission finished in single mission mode, exiting\n");
            m_eventQueue->AddEvent(Event(EVENT_QUIT));
            return;
        }

        GetLogger()->Info("Reseting world on phase change...\n");
        resetWorld = true;
    }

    if (resetWorld)
    {
        m_missionTimerEnabled = m_missionTimerStarted = false;
        m_missionTimer = 0.0f;

        if (m_phase == PHASE_SIMUL)  // ends a simulation?
        {
            SaveAllScript();
            m_sound->StopMusic(0.0f);
            m_camera->SetControllingObject(nullptr);

            if (m_gameTime > 10.0f)  // did you play at least 10 seconds?
            {
                m_playerProfile->IncrementLevelTryCount(m_levelCategory, m_levelChap, m_levelRank);
            }

            if (m_userPause != nullptr)
            {
                m_pause->DeactivatePause(m_userPause);
                m_userPause = nullptr;
            }
        }

        if (phase == PHASE_WIN)  // wins a simulation?
        {
            m_playerProfile->SetLevelPassed(m_levelCategory, m_levelChap, m_levelRank, true);
            m_ui->NextMission();  // passes to the next mission
        }

        DeleteAllObjects();  // removes all the current 3D Scene
    }

    m_phase = phase;

    if (m_phase != PHASE_SIMUL)
    {
        Ui::CWindow* pw = static_cast<Ui::CWindow*>(m_interface->SearchControl(EVENT_WINDOW6));
        if ( pw != nullptr )  pw->ClearState(Ui::STATE_VISIBLE | Ui::STATE_ENABLE);
    }

    if (resetWorld)
    {
        m_winDelay     = 0.0f;
        m_lostDelay    = 0.0f;
        m_beginSatCom = false;
        m_movieLock   = false;
        m_satComLock  = false;
        m_editLock    = false;
        m_resetCreate = false;
        m_infoObject  = nullptr;

        m_pause->FlushPause();
        m_freePhotoPause = nullptr;
        m_userPause = nullptr;
        m_focusPause = nullptr;
        FlushDisplayInfo();
        m_engine->SetRankView(0);
        m_terrain->FlushRelief();
        m_engine->DeleteAllObjects();
        m_oldModelManager->DeleteAllModelCopies();
        m_engine->SetWaterAddColor(Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f));
        m_engine->SetBackground("");
        m_engine->SetBackForce(false);
        m_engine->SetForegroundName("");
        m_engine->SetOverColor();
        m_engine->DeleteGroundMark(0);
        SetSpeed(1.0f);
        m_terrain->SetWind(Math::Vector(0.0f, 0.0f, 0.0f));
        m_terrain->FlushBuildingLevel();
        m_terrain->FlushFlyingLimit();
        m_lightMan->FlushLights();
        m_particle->FlushParticle();
        m_water->Flush();
        m_cloud->Flush();
        m_lightning->Flush();
        m_planet->Flush();
        m_interface->Flush();
        m_newScriptName.clear();
        m_sound->SetListener(Math::Vector(0.0f, 0.0f, 0.0f), Math::Vector(0.0f, 0.0f, 1.0f));
        m_sound->StopAll();
        m_camera->SetType(Gfx::CAM_TYPE_NULL);
        m_movie->Flush();
        m_movieInfoIndex = -1;
        m_shortCut = true;

        m_viewpoints.clear();
    }
    ClearInterface();

    Math::Point dim, pos;

    // Creates and hide the command console.
    dim.x = 200.0f/640.0f;
    dim.y =  18.0f/480.0f;
    pos.x =  20.0f/640.0f;
    pos.y = 100.0f/480.0f;
    Ui::CEdit* pe = m_interface->CreateEdit(pos, dim, 0, EVENT_CMD);
    pe->ClearState(Ui::STATE_VISIBLE);
    pe->SetMaxChar(100);
    m_cmdEdit = false;  // hidden for now

    // Creates the speedometer.
    dim.x =  30.0f/640.0f;
    dim.y =  20.0f/480.0f;
    pos.x =   4.0f/640.0f;
    pos.y = 426.0f/480.0f;

    // Creates the save indicator
    Ui::CButton* pb = m_interface->CreateButton(pos, dim, 0, EVENT_SPEED);
    pb->SetState(Ui::STATE_SIMPLY);
    pb->ClearState(Ui::STATE_VISIBLE);

    if (m_phase == PHASE_PLAYER_SELECT)
    {
        if (CResourceManager::DirectoryExists("crashsave"))
        {
            GetLogger()->Info("Pre-crash save found!\n");
            m_ui->GetDialog()->StartQuestion(
                "Your game seems to have crashed. Do you want to restore pre-crash state?", false, false, false,
                [&]()
                {
                    GetLogger()->Info("Trying to restore pre-crash state...\n");
                    assert(m_playerProfile != nullptr);
                    m_playerProfile->LoadScene("../../crashsave");
                    CResourceManager::RemoveExistingDirectory("crashsave");
                },
                [&]()
                {
                    GetLogger()->Info("Not restoring pre-crash state\n");
                    CResourceManager::RemoveExistingDirectory("crashsave");
                }
            );
        }
    }

    m_ui->ChangePhase(m_phase);
    if (m_phase == PHASE_SATCOM)
    {
        m_interface->DeleteControl(EVENT_WINDOW5);
        StartDisplayInfo(InjectLevelPathsForCurrentLevel("cbot.txt", "help/%lng%"), 0);
    }

    if (!resetWorld) return;

    dim.x = 32.0f/640.0f;
    dim.y = 32.0f/480.0f;
    float ox = 3.0f/640.0f;
    float oy = 3.0f/480.0f;
    float sx = (32.0f+2.0f)/640.0f;
    float sy = (32.0f+2.0f)/480.0f;

    if (m_phase != PHASE_APPEARANCE)
    {
        m_engine->SetDrawWorld(true);
        m_engine->SetDrawFront(false);
        m_fixScene = false;
    }

    if (m_phase == PHASE_SIMUL)
    {
        bool loading = !m_sceneReadPath.empty();

        m_ui->ShowLoadingScreen(true);
        m_ui->GetLoadingScreen()->SetProgress(0.0f, RT_LOADING_INIT);

        m_map->CreateMap();
        m_map->ShowMap(false);

        try
        {
            CreateScene(m_ui->GetSceneSoluce(), false, false);  // interactive scene
            if (m_mapImage)
                m_map->SetFixImage(m_mapFilename);

            m_app->ResetTimeAfterLoading();

            m_sound->StopMusic(0.0f);
            if (m_base == nullptr || loading) StartMusic();

            if (m_immediatSatCom && !loading  &&
                m_infoFilename[SATCOM_HUSTON][0] != 0)
                StartDisplayInfo(SATCOM_HUSTON, false);  // shows the instructions
        }
        catch (const std::runtime_error& e)
        {
            LevelLoadingError("An error occurred while trying to load a level", e);
        }
    }

    if (m_phase == PHASE_WIN)
    {
        m_sound->StopAll();
        if (m_endingWin.empty())
        {
            ChangePhase(PHASE_LEVEL_LIST);
        }
        else
        {
            m_winTerminate = (m_endingWin.substr(m_endingWin.find_last_of("/")+1) == "win904.txt");
            m_levelFile = m_endingWin;
            try
            {
                CreateScene(false, true, false);  // sets scene

                pos.x = ox+sx*1;  pos.y = oy+sy*1;
                Math::Point ddim;
                ddim.x = dim.x*2;  ddim.y = dim.y*2;
                m_interface->CreateButton(pos, ddim, 16, EVENT_BUTTON_OK);

                if (m_winTerminate)
                {
                    pos.x = ox+sx*3;  pos.y = oy+sy*0.2f;
                    ddim.x = dim.x*15;  ddim.y = dim.y*3.0f;
                    pe = m_interface->CreateEdit(pos, ddim, 0, EVENT_EDIT0);
                    pe->SetGenericMode(true);
                    pe->SetFontType(Gfx::FONT_COMMON);
                    pe->SetEditCap(false);
                    pe->SetHighlightCap(false);
                    pe->ReadText(std::string("help/") + m_app->GetLanguageChar() + std::string("/win.txt"));
                }
                else
                {
                    m_displayText->DisplayError(INFO_WIN, Math::Vector(0.0f,0.0f,0.0f), 15.0f, 60.0f, 1000.0f);
                }
                StartMusic();
            }
            catch (const std::runtime_error& e)
            {
                LevelLoadingError("An error occurred while trying to load win scene", e);
            }
        }
    }

    if (m_phase == PHASE_LOST)
    {
        m_sound->StopAll();
        if (m_endingLost.empty())
        {
            ChangePhase(PHASE_LEVEL_LIST);
        }
        else
        {
            m_winTerminate = false;
            m_levelFile = m_endingLost;
            try
            {
                CreateScene(false, true, false);  // sets scene

                pos.x = ox+sx*1;  pos.y = oy+sy*1;
                Math::Point ddim;
                ddim.x = dim.x*2;  ddim.y = dim.y*2;
                m_interface->CreateButton(pos, ddim, 16, EVENT_BUTTON_OK);
                m_displayText->DisplayError(INFO_LOST, Math::Vector(0.0f,0.0f,0.0f), 15.0f, 60.0f, 1000.0f);

                StartMusic();
            }
            catch (const std::runtime_error& e)
            {
                LevelLoadingError("An error occurred while trying to load lost scene", e);
            }
        }
    }

    m_engine->LoadAllTextures();
}

Phase CRobotMain::GetPhase()
{
    return m_phase;
}

//! Processes an event
bool CRobotMain::ProcessEvent(Event &event)
{
    if (!m_ui->EventProcess(event)) return false;
    if (m_phase == PHASE_SIMUL)
    {
        if (!m_editFull)
            m_camera->EventProcess(event);
    }
    if (!m_debugMenu->EventProcess(event)) return false;

    if (event.type == EVENT_FRAME)
    {
        if (!m_movie->EventProcess(event))  // end of the movie?
        {
            MainMovieType type = m_movie->GetStopType();
            if (type == MM_SATCOMopen)
            {
                m_pause->DeactivatePause(m_satcomMoviePause);
                m_satcomMoviePause = nullptr;
                SelectObject(m_infoObject, false);  // hands over the command buttons
                m_map->ShowMap(m_mapShow);
                m_displayText->HideText(false);
                int i = m_movieInfoIndex;
                StartDisplayInfo(m_movieInfoIndex, false);
                m_movieInfoIndex = i;
            }
        }

        m_displayText->EventProcess(event);

        if (m_displayInfo != nullptr)  // current edition?
            m_displayInfo->EventProcess(event);

        UpdateInfoText();

        return EventFrame(event);
    }

    if (event.type == EVENT_RELOAD_TEXTURES)
    {
        if (IsPhaseWithWorld(m_phase))
        {
            ChangeColor();
            UpdateMap();
        }
        m_engine->LoadAllTextures();
    }

    if (event.type == EVENT_RESOLUTION_CHANGED)
    {
        // Recreate the interface (needed if the aspect ratio changes)
        // TODO: This can sometimes cause unwanted side effects, like hidden windows reappearing. To be fixed during CEGUI refactoring.
        m_eventQueue->AddEvent(Event(EVENT_UPDINTERFACE));
        CreateShortcuts();
    }

    if (event.type == EVENT_FOCUS_LOST)
    {
        GetLogger()->Trace("Window unfocused\n");
        if (m_settings->GetFocusLostPause())
        {
            m_focusPause = m_pause->ActivatePause(PAUSE_ENGINE);
        }

        if (m_settings->GetFocusLostMute())
        {
            m_sound->SetAudioVolume(0);
            m_sound->SetMusicVolume(0);
        }
        return false;
    }

    if (event.type == EVENT_FOCUS_GAINED)
    {
        GetLogger()->Trace("Window focused\n");
        if (m_focusPause != nullptr)
        {
            m_pause->DeactivatePause(m_focusPause);
            m_focusPause = nullptr;
        }

        if (m_settings->GetFocusLostMute())
        {
            int volume;
            // Set music volume
            if (GetConfigFile().GetIntProperty("Setup", "MusicVolume", volume))
            {
                m_sound->SetMusicVolume(volume);
            }
            else
            {
                m_sound->SetMusicVolume(MAXVOLUME*3/4);
            }
            // Set audio volume
            if (GetConfigFile().GetIntProperty("Setup", "AudioVolume", volume))
            {
                m_sound->SetAudioVolume(volume);
            }
            else
            {
                m_sound->SetAudioVolume(MAXVOLUME);
            }
        }

        return false;
    }

    if (event.type == EVENT_WRITE_SCENE_FINISHED)
    {
        IOWriteSceneFinished();
        return false;
    }

    if (event.type == EVENT_UPDINTERFACE)
    {
        if (m_missionType == MISSION_CODE_BATTLE && !m_codeBattleStarted)
        {
            CreateCodeBattleInterface();
        }
    }

    if (event.type == EVENT_CODE_BATTLE_START)
    {
        m_pause->DeactivatePause(m_userPause);
        m_userPause = nullptr;
    }

    if (event.type == EVENT_CODE_BATTLE_SPECTATOR)
    {
        SetCodeBattleSpectatorMode(!m_codeBattleSpectator);
    }

    if (event.type >= EVENT_VIEWPOINT0 && event.type <= EVENT_VIEWPOINT9)
    {
        m_camera->SetType(Gfx::CAM_TYPE_SCRIPT);
        m_camera->SetSmooth(Gfx::CAM_SMOOTH_HARD);
        m_camera->SetScriptCameraAnimate(m_viewpoints[event.type - EVENT_VIEWPOINT0].eye, m_viewpoints[event.type - EVENT_VIEWPOINT0].look);
    }

    // Management of the console.
    if (event.type == EVENT_KEY_DOWN)
    {
        auto data = event.GetData<KeyEventData>();

        if (data->slot == INPUT_SLOT_CMDLINE)
        {
            if (m_phase != PHASE_PLAYER_SELECT &&
                !m_movie->IsExist()   &&
                !m_movieLock && !m_editLock && !m_cmdEdit)
            {
                Ui::CEdit* pe = static_cast<Ui::CEdit*>(m_interface->SearchControl(EVENT_CMD));
                if (pe == nullptr) return false;
                pe->SetState(Ui::STATE_VISIBLE);
                m_interface->SetFocus(pe);
                if (m_phase == PHASE_SIMUL) m_cmdEditPause = m_pause->ActivatePause(PAUSE_ENGINE);
                m_cmdEdit = true;
                m_commandHistoryIndex = -1; // no element selected in command history
            }
            return false;
        }

        if (IsPhaseWithWorld(m_phase))
        {
            if (data->key == KEY(F10))
            {
                m_debugMenu->ToggleInterface();
                return false;
            }
        }
    }

    // Browse forward command history with UP key
    if (event.type == EVENT_KEY_DOWN &&
        event.GetData<KeyEventData>()->key == KEY(UP) && m_cmdEdit)
    {
        Ui::CEdit* pe = static_cast<Ui::CEdit*>(m_interface->SearchControl(EVENT_CMD));
        if (pe == nullptr) return false;
        std::string cmd = GetNextFromCommandHistory();
        if (!cmd.empty()) pe->SetText(cmd);
        return false;
    }

    // Browse backward command history with DOWN key
    if (event.type == EVENT_KEY_DOWN &&
        event.GetData<KeyEventData>()->key == KEY(DOWN) && m_cmdEdit)
    {
        Ui::CEdit* pe = static_cast<Ui::CEdit*>(m_interface->SearchControl(EVENT_CMD));
        if (pe == nullptr) return false;
        std::string cmd = GetPreviousFromCommandHistory();
        if (!cmd.empty()) pe->SetText(cmd);
        return false;
    }

    if (event.type == EVENT_KEY_DOWN &&
        event.GetData<KeyEventData>()->key == KEY(RETURN) && m_cmdEdit)
    {
        std::string cmd;
        Ui::CEdit* pe = static_cast<Ui::CEdit*>(m_interface->SearchControl(EVENT_CMD));
        if (pe == nullptr) return false;
        cmd = pe->GetText(50);
        pe->SetText("");
        pe->ClearState(Ui::STATE_VISIBLE);
        m_interface->SetFocus(nullptr);
        if (m_phase == PHASE_SIMUL)
        {
            m_pause->DeactivatePause(m_cmdEditPause);
            m_cmdEditPause = nullptr;
        }
        ExecuteCmd(cmd);
        PushToCommandHistory(cmd);
        m_cmdEdit = false;
        return false;
    }

    if (event.type == EVENT_KEY_DOWN && m_cmdEdit)
        return false; // cheat console active, so ignore keys

    // Management of the speed change.
    if (event.type == EVENT_SPEED)
        SetSpeed(1.0f);

    if (!m_displayText->EventProcess(event))
        return false;

    if (event.type == EVENT_MOUSE_MOVE)
    {
        HiliteObject(event.mousePos);
    }

    if (m_displayInfo != nullptr)  // current info?
    {
        m_displayInfo->EventProcess(event);

        if (event.type == EVENT_KEY_DOWN)
        {
            auto data = event.GetData<KeyEventData>();

            if (data->slot == INPUT_SLOT_HELP ||
                data->slot == INPUT_SLOT_PROG ||
                data->key == KEY(ESCAPE))
            {
                StopDisplayInfo();
            }
        }

        if (event.type == EVENT_OBJECT_INFOOK)
            StopDisplayInfo();

        if (m_displayInfo == nullptr && m_phase == PHASE_SATCOM)
            ChangePhase(PHASE_MAIN_MENU);

        return false;
    }

    CObject* obj;

    // Simulation phase of the game
    if (m_phase == PHASE_SIMUL)
    {
        switch (event.type)
        {
            case EVENT_KEY_DOWN:
            {
                auto data = event.GetData<KeyEventData>();

                HiliteClear();
                if (m_editLock)  // current edition?
                {
                    if (data->slot == INPUT_SLOT_HELP)
                    {
                        StartDisplayInfo(SATCOM_HUSTON, false);
                        return false;
                    }
                    if (data->slot == INPUT_SLOT_PROG)
                    {
                        StartDisplayInfo(SATCOM_PROG, false);
                        return false;
                    }
                    break;
                }
                if (m_movieLock)  // current movie?
                {
                    if (data->slot == INPUT_SLOT_QUIT ||
                        data->key == KEY(ESCAPE))
                    {
                        AbortMovie();
                    }
                    return false;
                }
                if (m_camera->GetType() == Gfx::CAM_TYPE_VISIT)
                {
                    if (data->slot == INPUT_SLOT_VISIT)
                    {
                        StartDisplayVisit(EVENT_NULL);
                    }
                    if (data->slot == INPUT_SLOT_QUIT ||
                        data->key == KEY(ESCAPE))
                    {
                        StopDisplayVisit();
                    }
                    return false;
                }
                if (data->slot == INPUT_SLOT_QUIT)
                {
                    if (m_movie->IsExist())
                        StartDisplayInfo(SATCOM_HUSTON, false);
                    else if (m_winDelay > 0.0f)
                        ChangePhase(PHASE_WIN);
                    else if (m_lostDelay > 0.0f)
                        ChangePhase(PHASE_LOST);
                    else if (!m_cmdEdit)
                        m_ui->GetDialog()->StartPauseMenu();  // do you want to leave?
                }
                if (data->slot == INPUT_SLOT_PAUSE)
                {
                    if (m_userPause == nullptr)
                    {
                        if (!m_pause->IsPauseType(PAUSE_ENGINE))
                        {
                            m_userPause = m_pause->ActivatePause(PAUSE_ENGINE);
                        }
                    }
                    else
                    {
                        m_pause->DeactivatePause(m_userPause);
                        m_userPause = nullptr;
                    }
                }
                if (data->slot == INPUT_SLOT_CAMERA)
                {
                    ChangeCamera();
                }
                if (data->slot == INPUT_SLOT_DESEL)
                {
                    if (m_shortCut)
                        DeselectObject();
                }
                if (data->slot == INPUT_SLOT_HUMAN)
                {
                    SelectObject(SearchHuman());
                }
                if (data->slot == INPUT_SLOT_NEXT && ((event.kmodState & KEY_MOD(CTRL)) != 0))
                {
                    m_short->SelectShortcut(EVENT_OBJECT_SHORTCUT_MODE); // switch bots <-> buildings
                    return false;
                }
                if (data->slot == INPUT_SLOT_NEXT)
                {
                    if (m_shortCut)
                        m_short->SelectNext();
                }
                if (data->slot == INPUT_SLOT_HELP)
                {
                    StartDisplayInfo(SATCOM_HUSTON, true);
                }
                if (data->slot == INPUT_SLOT_PROG)
                {
                    StartDisplayInfo(SATCOM_PROG, true);
                }
                if (data->slot == INPUT_SLOT_VISIT)
                {
                    StartDisplayVisit(EVENT_NULL);
                }
                if (data->slot == INPUT_SLOT_SPEED_DEC)
                {
                    SetSpeed(GetSpeed()*0.5f);
                }
                if (data->slot == INPUT_SLOT_SPEED_RESET)
                {
                    SetSpeed(1.0f);
                }
                if (data->slot == INPUT_SLOT_SPEED_INC)
                {
                    SetSpeed(GetSpeed()*2.0f);
                }
                if (data->slot == INPUT_SLOT_QUICKSAVE)
                {
                    QuickSave();
                }
                if (data->slot == INPUT_SLOT_QUICKLOAD)
                {
                    QuickLoad();
                }
                if (data->key == KEY(c) && ((event.kmodState & KEY_MOD(CTRL)) != 0) && m_engine->GetShowStats())
                {
                    CObject* obj = GetSelect();
                    if (obj != nullptr)
                    {
                        CLevelParserLine line("CreateObject");
                        line.AddParam("type", MakeUnique<CLevelParserParam>(obj->GetType()));

                        Math::Vector pos = obj->GetPosition()/g_unit;
                        pos.y = 0.0f;
                        line.AddParam("pos", MakeUnique<CLevelParserParam>(pos));

                        float dir = Math::NormAngle(obj->GetRotationY()) / Math::PI;
                        line.AddParam("dir", MakeUnique<CLevelParserParam>(dir));

                        std::stringstream ss;
                        ss << line;
                        SDL_SetClipboardText(ss.str().c_str());
                    }
                }
                break;
            }

            case EVENT_MOUSE_BUTTON_DOWN:
            {
                if (event.GetData<MouseButtonEventData>()->button != MOUSE_BUTTON_LEFT) // only left mouse button
                    break;

                obj = DetectObject(event.mousePos);
                if (!m_shortCut) obj = nullptr;
                if (obj != nullptr && obj->GetType() == OBJECT_TOTO)
                {
                    if (m_displayInfo != nullptr)  // current info?
                    {
                        StopDisplayInfo();
                    }
                    else
                    {
                        if (!m_editLock)
                            StartDisplayInfo(SATCOM_HUSTON, true);
                    }
                }
                else
                {
                    SelectObject(obj);
                }
                break;
            }

            case EVENT_OBJECT_LIMIT:
                StartShowLimit();
                break;

            case EVENT_OBJECT_DESELECT:
                if (m_shortCut)
                    DeselectObject();
                break;

            case EVENT_OBJECT_HELP:
                HelpObject();
                break;

            case EVENT_OBJECT_CAMERA:
                ChangeCamera();
                break;

            case EVENT_OBJECT_DELETE:
                m_ui->GetDialog()->StartQuestion(
                    RT_DIALOG_DELOBJ, true, false, false,
                    [&]()
                    {
                        DestroySelectedObject();
                    }
                );
                break;

            case EVENT_OBJECT_BHELP:
                StartDisplayInfo(SATCOM_HUSTON, true);
                break;

            case EVENT_OBJECT_SOLUCE:
                StartDisplayInfo(SATCOM_SOLUCE, true);
                break;

            case EVENT_OBJECT_MAPZOOM:
                m_map->ZoomMap();
                break;

            case EVENT_DT_VISIT0:
            case EVENT_DT_VISIT1:
            case EVENT_DT_VISIT2:
            case EVENT_DT_VISIT3:
            case EVENT_DT_VISIT4:
                StartDisplayVisit(event.type);
                break;

            case EVENT_DT_END:
                StopDisplayVisit();
                break;

            case EVENT_OBJECT_MOVIELOCK:
                AbortMovie();
                break;

            case EVENT_WIN:
                m_missionTimerEnabled = m_missionTimerStarted = false;
                ChangePhase(PHASE_WIN);
                break;

            case EVENT_LOST:
                m_missionTimerEnabled = m_missionTimerStarted = false;
                ChangePhase(PHASE_LOST);
                break;

            default:
                break;
        }

        if (event.type >= EVENT_OBJECT_SHORTCUT_MODE && event.type <= EVENT_OBJECT_SHORTCUT_MAX)
        {
            m_short->SelectShortcut(event.type);
        }

        EventObject(event);
        return false;
    }

    if (m_phase == PHASE_APPEARANCE)
        EventObject(event);

    if (m_phase == PHASE_WIN  ||
        m_phase == PHASE_LOST)
    {
        EventObject(event);

        switch (event.type)
        {
            case EVENT_KEY_DOWN:
            {
                auto data = event.GetData<KeyEventData>();

                if (data->key == KEY(ESCAPE) ||
                    data->key == KEY(RETURN))
                {
                    if (m_winTerminate)
                        ChangePhase(PHASE_MAIN_MENU);
                    else
                        ChangePhase(PHASE_LEVEL_LIST);
                }
                break;
            }

            case EVENT_BUTTON_OK:
                if (m_winTerminate)
                    ChangePhase(PHASE_MAIN_MENU);
                else
                    ChangePhase(PHASE_LEVEL_LIST);

                break;

            default:
                break;
        }
    }

    return true;
}



//! Executes a command
void CRobotMain::ExecuteCmd(const std::string& cmd)
{
    if (cmd.empty()) return;

    if (m_phase == PHASE_SIMUL)
    {
        if (cmd == "winmission")
            m_eventQueue->AddEvent(Event(EVENT_WIN));

        if (cmd == "lostmission")
            m_eventQueue->AddEvent(Event(EVENT_LOST));

        if (cmd == "trainerpilot")
        {
            m_cheatTrainerPilot = !m_cheatTrainerPilot;
            return;
        }

        if (cmd == "fly")
        {
            m_researchDone[0] |= RESEARCH_FLY;

            m_eventQueue->AddEvent(Event(EVENT_UPDINTERFACE));
            return;
        }

        if (cmd == "allresearch")
        {
            m_researchDone[0] = -1;  // all research are done

            m_eventQueue->AddEvent(Event(EVENT_UPDINTERFACE));
            return;
        }

        if (cmd == "allbuildings")
        {
            m_build = -1;  // all buildings are available

            m_eventQueue->AddEvent(Event(EVENT_UPDINTERFACE));
            return;
        }

        if (cmd == "all")
        {
            m_researchDone[0] = -1;  // all research are done
            m_build = -1;  // all buildings are available

            m_eventQueue->AddEvent(Event(EVENT_UPDINTERFACE));
            return;
        }

        if (cmd == "nolimit")
        {
            m_terrain->SetFlyingMaxHeight(280.0f);
            return;
        }

        if (cmd == "controller")
        {
            if (m_controller == nullptr)
            {
                GetLogger()->Error("No LevelController on the map to select\n");
                return;
            }

            // Don't use SelectObject because it checks if the object is selectable
            if (m_camera->GetType() == Gfx::CAM_TYPE_VISIT)
                StopDisplayVisit();

            CObject* prev = DeselectAll();
            if (prev != nullptr && prev != m_controller)
                PushToSelectionHistory(prev);

            SelectOneObject(m_controller, true);
            m_short->UpdateShortcuts();
            return;
        }

        if (cmd == "photo1")
        {
            if (m_freePhotoPause == nullptr)
            {
                m_camera->SetType(Gfx::CAM_TYPE_FREE);
                m_freePhotoPause = m_pause->ActivatePause(PAUSE_ENGINE|PAUSE_PHOTO|PAUSE_OBJECT_UPDATES);
            }
            else
            {
                m_camera->SetType(Gfx::CAM_TYPE_BACK);
                m_pause->DeactivatePause(m_freePhotoPause);
                m_freePhotoPause = nullptr;
            }
            return;
        }

        if (cmd == "photo2")
        {
            if (m_freePhotoPause == nullptr)
            {
                m_camera->SetType(Gfx::CAM_TYPE_FREE);
                DeselectAll();  // removes the control buttons
                m_freePhotoPause = m_pause->ActivatePause(PAUSE_ENGINE|PAUSE_PHOTO|PAUSE_OBJECT_UPDATES);
                m_map->ShowMap(false);
                m_displayText->HideText(true);
            }
            else
            {
                m_camera->SetType(Gfx::CAM_TYPE_BACK);
                m_pause->DeactivatePause(m_freePhotoPause);
                m_freePhotoPause = nullptr;
                m_map->ShowMap(m_mapShow);
                m_displayText->HideText(false);
            }
            return;
        }

        int camtype;
        if (sscanf(cmd.c_str(), "camtype %d", &camtype) > 0)
        {
            m_camera->SetType(static_cast<Gfx::CameraType>(camtype));
            return;
        }

        float camspeed;
        if (sscanf(cmd.c_str(), "camspeed %f", &camspeed) > 0)
        {
            m_camera->SetCameraSpeed(camspeed);
            return;
        }

        if (cmd == "freecam")
        {
            m_camera->SetType(Gfx::CAM_TYPE_FREE);
            return;
        }

        if (cmd == "noclip")
        {
            CObject* object = GetSelect();
            if (object != nullptr)
                object->SetCollisions(false);
            return;
        }

        if (cmd == "clip")
        {
            CObject* object = GetSelect();
            if (object != nullptr)
                object->SetCollisions(true);
            return;
        }

        if (cmd == "addhusky")
        {
            CObject* object = GetSelect();
            if (object != nullptr && object->Implements(ObjectInterfaceType::Shielded))
                dynamic_cast<CShieldedObject&>(*object).SetMagnifyDamage(dynamic_cast<CShieldedObject&>(*object).GetMagnifyDamage()*0.1f);
            return;
        }

        if (cmd == "addfreezer")
        {
            CObject* object = GetSelect();
            if (object != nullptr && object->Implements(ObjectInterfaceType::JetFlying))
                dynamic_cast<CJetFlyingObject&>(*object).SetRange(dynamic_cast<CJetFlyingObject&>(*object).GetRange()*10.0f);
            return;
        }

        if (cmd == "\155\157\157")
        {
            // VGhpcyBpcyBlYXN0ZXItZWdnIGFuZCBzbyBpdCBzaG91bGQgYmUgb2JmdXNjYXRlZCEgRG8gbm90
            // IGNsZWFuLXVwIHRoaXMgY29kZSEK
            GetLogger()->Info(" _________________________\n");
            GetLogger()->Info("< \x50\x6F\x6C\x73\x6B\x69 \x50\x6F\x72\x74\x61\x6C C\x6F\x6C\x6F\x62\x6F\x74\x61! \x3E\n");
            GetLogger()->Info(" -------------------------\n");
            GetLogger()->Info("        \x5C\x20\x20\x20\x5E\x5F\x5F\x5E\n");
            GetLogger()->Info("        \x20\x5C\x20\x20\x28\x6F\x6F\x29\x5C\x5F\x5F\x5F\x5F\x5F\x5F\x5F\n");
            GetLogger()->Info("            \x28\x5F\x5F\x29\x5C   \x20\x20\x20\x20\x29\x5C\x2F\x5C\n");
            GetLogger()->Info("            \x20\x20\x20\x20\x7C|\x2D\x2D\x2D\x2D\x77\x20\x7C\n");
            GetLogger()->Info("          \x20\x20    \x7C\x7C\x20\x20\x20\x20 ||\n");
        }

        if (cmd == "fullpower")
        {
            CObject* object = GetSelect();
            if (object != nullptr)
            {
                if (CPowerContainerObject *power = GetObjectPowerCell(object))
                    power->SetEnergyLevel(1.0f);

                if (object->Implements(ObjectInterfaceType::Shielded))
                    dynamic_cast<CShieldedObject&>(*object).SetShield(1.0f);

                if (object->Implements(ObjectInterfaceType::JetFlying))
                    dynamic_cast<CJetFlyingObject&>(*object).SetReactorRange(1.0f);
            }
            return;
        }

        if (cmd == "fullenergy")
        {
            CObject* object = GetSelect();

            if (object != nullptr)
            {
                if (CPowerContainerObject *power = GetObjectPowerCell(object))
                    power->SetEnergyLevel(1.0f);
            }
            return;
        }

        if (cmd == "fullshield")
        {
            CObject* object = GetSelect();
            if (object != nullptr && object->Implements(ObjectInterfaceType::Shielded))
                dynamic_cast<CShieldedObject&>(*object).SetShield(1.0f);
            return;
        }

        if (cmd == "fullrange")
        {
            CObject* object = GetSelect();
            if (object != nullptr)
            {
                if (object->Implements(ObjectInterfaceType::JetFlying))
                    dynamic_cast<CJetFlyingObject&>(*object).SetReactorRange(1.0f);
            }
            return;
        }
    }

    if (cmd == "debugmode")
    {
        if (m_app->IsDebugModeActive(DEBUG_ALL))
        {
            m_app->SetDebugModeActive(DEBUG_ALL, false);
        }
        else
        {
            m_app->SetDebugModeActive(DEBUG_ALL, true);
        }
        return;
    }

    if (cmd == "showstat")
    {
        m_engine->SetShowStats(!m_engine->GetShowStats());
        return;
    }

    if (cmd == "invui")
    {
        m_engine->SetRenderInterface(!m_engine->GetRenderInterface());
        return;
    }

    if (cmd == "selectinsect")
    {
        m_cheatSelectInsect = !m_cheatSelectInsect;
        return;
    }

    if (cmd == "showsoluce")
    {
        m_cheatShowSoluce = !m_cheatShowSoluce;
        m_ui->ShowSoluceUpdate();
        return;
    }

    if (cmd == "allmission")
    {
        m_cheatAllMission = !m_cheatAllMission;
        m_ui->AllMissionUpdate();
        return;
    }

    if (cmd == "invradar")
    {
        m_cheatRadar = !m_cheatRadar;
        return;
    }

    float speed;
    if (sscanf(cmd.c_str(), "speed %f", &speed) > 0)
    {
        SetSpeed(speed);
        UpdateSpeedLabel();
        return;
    }

    if (m_phase == PHASE_SIMUL)
        m_displayText->DisplayError(ERR_CMD, Math::Vector(0.0f,0.0f,0.0f));
}



//! Returns the type of current movie
MainMovieType CRobotMain::GetMainMovie()
{
    return m_movie->GetType();
}


//! Clears the display of instructions
void CRobotMain::FlushDisplayInfo()
{
    for (int i = 0; i < SATCOM_MAX; i++)
    {
        m_infoFilename[i][0] = 0;
    }
    strcpy(m_infoFilename[SATCOM_OBJECT], "objects.txt");
}

//! Beginning of the displaying of instructions.
//! index: SATCOM_*
void CRobotMain::StartDisplayInfo(int index, bool movie)
{
    if (m_cmdEdit || m_satComLock || m_lockedSatCom) return;

    CObject* obj = GetSelect();
    bool human = obj != nullptr && obj->GetType() == OBJECT_HUMAN;

    if (!m_editLock && movie && !m_movie->IsExist() && human)
    {
        assert(obj->Implements(ObjectInterfaceType::Movable));
        if (dynamic_cast<CMovableObject&>(*obj).GetMotion()->GetAction() == -1)
        {
            m_movieInfoIndex = index;
            m_movie->Start(MM_SATCOMopen, 2.5f);
            m_satcomMoviePause = m_pause->ActivatePause(PAUSE_ENGINE|PAUSE_HIDE_SHORTCUTS);
            m_infoObject = DeselectAll();  // removes the control buttons
            m_displayText->HideText(true);
            return;
        }
    }

    if (m_movie->IsExist())
    {
        m_movie->Stop();
        m_pause->DeactivatePause(m_satcomMoviePause);
        m_satcomMoviePause = nullptr;
        SelectObject(m_infoObject, false);  // hands over the command buttons
        m_displayText->HideText(false);
    }

    StartDisplayInfo(m_infoFilename[index], index);
}

//! Beginning of the displaying of instructions
void CRobotMain::StartDisplayInfo(const std::string& filename, int index)
{
    if (m_cmdEdit) return;

    m_movieInfoIndex = -1;
    ClearInterface();  // removes setting evidence and tooltip

    if (!m_editLock)
    {
        m_infoObject = DeselectAll();  // removes the control buttons
        m_displayText->HideText(true);
        m_sound->MuteAll(true);
    }

    bool soluce = m_ui->GetSceneSoluce();

    m_displayInfo = MakeUnique<Ui::CDisplayInfo>();
    m_displayInfo->StartDisplayInfo(filename, index, soluce);
    m_displayInfo->SetPosition(0);
}

//! End of displaying of instructions
void CRobotMain::StopDisplayInfo()
{
    if (m_cmdEdit) return;

    if (m_movieInfoIndex != -1)  // film to read the SatCom?
        m_movie->Start(MM_SATCOMclose, 2.0f);

    m_displayInfo->StopDisplayInfo();

    m_displayInfo.reset();

    if (!m_editLock)
    {
        SelectObject(m_infoObject, false);  // gives the command buttons
        m_displayText->HideText(false);

        m_sound->MuteAll(false);
    }

    if (m_infoUsed == 0)
        m_displayText->ClearText();  // removes message "see SatCom ..."
    m_infoUsed ++;
}

//! Returns the name of the text display
char* CRobotMain::GetDisplayInfoName(int index)
{
    return m_infoFilename[index];
}


//! Beginning of a dialogue during the game
void CRobotMain::StartSuspend()
{
    if (m_suspend != nullptr) return; // already suspended
    if (!IsPhaseWithWorld(m_phase)) return;
    GetLogger()->Info("Start suspend\n");

    m_sound->MuteAll(true);
    ClearInterface();
    m_suspend = m_pause->ActivatePause(PAUSE_ENGINE | PAUSE_HIDE_SHORTCUTS | PAUSE_MUTE_SOUND | PAUSE_CAMERA);
    m_engine->SetOverFront(false);  // over flat behind
    CreateShortcuts();

    m_map->ShowMap(false);
    m_infoObject = DeselectAll();  // removes the control buttons
    m_displayText->HideText(true);

    m_engine->EnablePauseBlur();
}

//! End of dialogue during the game
void CRobotMain::StopSuspend()
{
    if (m_suspend == nullptr) return; // not suspended
    GetLogger()->Info("Stop suspend\n");

    m_sound->MuteAll(false);
    ClearInterface();
    m_pause->DeactivatePause(m_suspend);
    m_suspend = nullptr;
    m_engine->SetOverFront(true);  // over flat front
    CreateShortcuts();

    if (m_infoObject != nullptr)
        SelectObject(m_infoObject, false);  // gives the command buttons
    m_map->ShowMap(m_mapShow);
    m_displayText->HideText(false);

    m_engine->DisablePauseBlur();
}


//! Returns the absolute time of the game
float CRobotMain::GetGameTime()
{
    return m_gameTime;
}


//! Start of the visit instead of an error
void CRobotMain::StartDisplayVisit(EventType event)
{
    if (m_editLock) return;

    if (m_visitPause)
    {
        m_pause->DeactivatePause(m_visitPause);
        m_visitPause = nullptr;
    }

    Ui::CWindow* pw = static_cast<Ui::CWindow*>(m_interface->SearchControl(EVENT_WINDOW2));
    if (pw == nullptr) return;

    if (event == EVENT_NULL)  // visit by keyboard shortcut?
    {
        int i;
        if (m_visitLast != EVENT_NULL)  // already a current visit?
            i = m_visitLast-EVENT_DT_VISIT0;
        else
            i = Ui::MAXDTLINE;

        // Seeks the last.
        for (int j = 0; j < Ui::MAXDTLINE; j++)
        {
            i --;
            if (i < 0) i = Ui::MAXDTLINE-1;

            Ui::CButton* button = static_cast<Ui::CButton*>(pw->SearchControl(static_cast<EventType>(EVENT_DT_VISIT0+i)));
            if (button == nullptr || !button->TestState(Ui::STATE_ENABLE)) continue;

            Ui::CGroup* group = static_cast<Ui::CGroup*>(pw->SearchControl(static_cast<EventType>(EVENT_DT_GROUP0+i)));
            if (group != nullptr)
            {
                event = static_cast<EventType>(EVENT_DT_VISIT0+i);
                break;
            }
        }
    }
    if (event == EVENT_NULL)
    {
        m_sound->Play(SOUND_TZOING);  // nothing to do!
        return;
    }

    m_visitLast = event;

    ClearInterface();  // removes setting evidence and tooltip

    if (m_camera->GetType() == Gfx::CAM_TYPE_VISIT)  // already a current visit?
    {
        m_camera->StopVisit();
        m_displayText->ClearVisit();
    }
    else
    {
        m_visitObject = DeselectAll();  // removes the control buttons
    }

    // Creates the "continue" button.
    if (m_interface->SearchControl(EVENT_DT_END) == nullptr)
    {
        Math::Point pos, dim;
        pos.x = 10.0f/640.0f;
        pos.y = 10.0f/480.0f;
        dim.x = 50.0f/640.0f;
        dim.y = 50.0f/480.0f;
        m_interface->CreateButton(pos, dim, 16, EVENT_DT_END);
    }

    // Creates the arrow to show the place.
    if (m_visitArrow != nullptr)
    {
        CObjectManager::GetInstancePointer()->DeleteObject(m_visitArrow);
        m_visitArrow = nullptr;
    }

    ObjectCreateParams params;
    params.pos = m_displayText->GetVisitGoal(event);
    params.type = OBJECT_SHOW;
    params.height = 10.0f;
    m_visitArrow = m_objMan->CreateObject(params);

    m_visitPos = m_visitArrow->GetPosition();
    m_visitPosArrow = m_visitPos;
    m_visitPosArrow.y += m_displayText->GetVisitHeight(event);
    m_visitArrow->SetPosition(m_visitPosArrow);

    m_visitTime = 0.0;
    m_visitParticle = 0.0f;

    m_particle->DeleteParticle(Gfx::PARTISHOW);

    m_camera->StartVisit(m_displayText->GetVisitGoal(event),
                         m_displayText->GetVisitDist(event));
    m_displayText->SetVisit(event);
    m_visitPause = m_pause->ActivatePause(PAUSE_ENGINE);
}

//! Move the arrow to visit
void CRobotMain::FrameVisit(float rTime)
{
    if (m_visitArrow == nullptr) return;

    // Moves the arrow.
    m_visitTime += rTime;

    Math::Vector pos = m_visitPosArrow;
    pos.y += 1.5f+sinf(m_visitTime*4.0f)*4.0f;
    m_visitArrow->SetPosition(pos);
    m_visitArrow->SetRotationY(m_visitTime*2.0f);

    // Manages the particles "arrows".
    m_visitParticle -= rTime;
    if (m_visitParticle <= 0.0f)
    {
        m_visitParticle = 1.5f;

        pos = m_visitPos;
        float level = m_terrain->GetFloorLevel(pos)+2.0f;
        if (pos.y < level) pos.y = level;  // not below the ground
        Math::Vector speed(0.0f, 0.0f, 0.0f);
        Math::Point dim;
        dim.x = 30.0f;
        dim.y = dim.x;
        m_particle->CreateParticle(pos, speed, dim, Gfx::PARTISHOW, 2.0f);
    }
}

//! End of the visit instead of an error
void CRobotMain::StopDisplayVisit()
{
    m_visitLast = EVENT_NULL;

    // Removes the button.
    m_interface->DeleteControl(EVENT_DT_END);

    // Removes the arrow.
    if (m_visitArrow != nullptr)
    {
        CObjectManager::GetInstancePointer()->DeleteObject(m_visitArrow);
        m_visitArrow = nullptr;
    }

    // Removes particles "arrows".
    m_particle->DeleteParticle(Gfx::PARTISHOW);

    m_camera->StopVisit();
    m_displayText->ClearVisit();
    m_pause->DeactivatePause(m_visitPause);
    m_visitPause = nullptr;
    if (m_visitObject != nullptr)
    {
        SelectObject(m_visitObject, false);  // gives the command buttons
        m_visitObject = nullptr;
    }
}



void CRobotMain::UpdateShortcuts()
{
    m_short->UpdateShortcuts();
}

CObject* CRobotMain::GetSelectObject()
{
    if (m_selectObject != nullptr) return m_selectObject;
    return SearchHuman();
}

CObject* CRobotMain::DeselectAll()
{
    CObject* prev = nullptr;
    for (CObject* obj : m_objMan->GetAllObjects())
    {
        if (!obj->Implements(ObjectInterfaceType::Controllable)) continue;
        auto controllableObj = dynamic_cast<CControllableObject*>(obj);
        if (controllableObj->GetSelect()) prev = obj;
        controllableObj->SetSelect(false);
    }
    return prev;
}

//! Selects an object, without attending to deselect the rest
void CRobotMain::SelectOneObject(CObject* obj, bool displayError)
{
    assert(obj->Implements(ObjectInterfaceType::Controllable));
    dynamic_cast<CControllableObject&>(*obj).SetSelect(true, displayError);
    m_camera->SetControllingObject(obj);

    ObjectType type = obj->GetType();
    if ( type == OBJECT_HUMAN    ||
         type == OBJECT_MOBILEfa ||
         type == OBJECT_MOBILEta ||
         type == OBJECT_MOBILEwa ||
         type == OBJECT_MOBILEia ||
         type == OBJECT_MOBILEfb ||
         type == OBJECT_MOBILEtb ||
         type == OBJECT_MOBILEwb ||
         type == OBJECT_MOBILEib ||
         type == OBJECT_MOBILEfc ||
         type == OBJECT_MOBILEtc ||
         type == OBJECT_MOBILEwc ||
         type == OBJECT_MOBILEic ||
         type == OBJECT_MOBILEfi ||
         type == OBJECT_MOBILEti ||
         type == OBJECT_MOBILEwi ||
         type == OBJECT_MOBILEii ||
         type == OBJECT_MOBILEfs ||
         type == OBJECT_MOBILEts ||
         type == OBJECT_MOBILEws ||
         type == OBJECT_MOBILEis ||
         type == OBJECT_MOBILErt ||
         type == OBJECT_MOBILErc ||
         type == OBJECT_MOBILErr ||
         type == OBJECT_MOBILErs ||
         type == OBJECT_MOBILEsa ||
         type == OBJECT_MOBILEft ||
         type == OBJECT_MOBILEtt ||
         type == OBJECT_MOBILEwt ||
         type == OBJECT_MOBILEit ||
         type == OBJECT_MOBILErp ||
         type == OBJECT_MOBILEst ||
         type == OBJECT_MOBILEdr ||
         type == OBJECT_APOLLO2  )
    {
        m_camera->SetType(dynamic_cast<CControllableObject&>(*obj).GetCameraType());
    }
    else
    {
        m_camera->SetType(Gfx::CAM_TYPE_BACK);
    }
}

bool CRobotMain::SelectObject(CObject* obj, bool displayError)
{
    if (m_camera->GetType() == Gfx::CAM_TYPE_VISIT)
        StopDisplayVisit();

    if (m_movieLock || m_editLock) return false;
    if (m_movie->IsExist()) return false;
    if (obj != nullptr &&
        (!obj->Implements(ObjectInterfaceType::Controllable) || !(dynamic_cast<CControllableObject&>(*obj).GetSelectable() || m_cheatSelectInsect))) return false;

    if (m_missionType == MISSION_CODE_BATTLE && m_codeBattleStarted && m_codeBattleSpectator)
    {
        DeselectAll();

        // During code battles, only change camera
        m_camera->SetControllingObject(obj);
        if (obj != nullptr)
        {
            m_camera->SetType(Gfx::CAM_TYPE_PLANE);
        }
        else
        {
            m_camera->SetType(Gfx::CAM_TYPE_FREE);
        }
    }
    else
    {
        if (obj == nullptr) return false;
        CObject* prev = DeselectAll();

        if (prev != nullptr && prev != obj)
           PushToSelectionHistory(prev);

        SelectOneObject(obj, displayError);
    }
    m_short->UpdateShortcuts();
    return true;
}

bool CRobotMain::DeselectObject()
{
    DeselectAll();

    CObject* obj = PopFromSelectionHistory();
    if (obj == nullptr)
        obj = SearchHuman();

    if (obj != nullptr)
        SelectOneObject(obj);
    else
        m_camera->SetType(Gfx::CAM_TYPE_FREE);

    m_short->UpdateShortcuts();
    return true;
}

//! Quickly removes all objects
void CRobotMain::DeleteAllObjects()
{
    m_engine->GetPyroManager()->DeleteAll();

    // Removes the arrow.
    if (m_visitArrow != nullptr)
    {
        CObjectManager::GetInstancePointer()->DeleteObject(m_visitArrow);
        m_visitArrow = nullptr;
    }

    for (int i = 0; i < MAXSHOWLIMIT; i++)
        FlushShowLimit(i);

    m_objMan->DeleteAllObjects();
}

CObject* CRobotMain::SearchHuman()
{
    return m_objMan->FindNearest(nullptr, OBJECT_HUMAN);
}

CObject* CRobotMain::GetSelect()
{
    for (CObject* obj : m_objMan->GetAllObjects())
    {
        if (!obj->Implements(ObjectInterfaceType::Controllable)) continue;
        if (dynamic_cast<CControllableObject&>(*obj).GetSelect())
            return obj;
    }
    return nullptr;
}

//! Detects the object aimed by the mouse
CObject* CRobotMain::DetectObject(Math::Point pos)
{
    Math::Vector p;
    int objRank = m_engine->DetectObject(pos, p);

    for (CObject* obj : m_objMan->GetAllObjects())
    {
        if (!obj->GetDetectable()) continue;

        CObject* transporter = nullptr;
        if (obj->Implements(ObjectInterfaceType::Transportable))
            transporter = dynamic_cast<CTransportableObject&>(*obj).GetTransporter();

        if (transporter != nullptr && !transporter->GetDetectable()) continue;
        if (obj->GetProxyActivate()) continue;

        CObject* target = obj;
        // TODO: should this also apply to slots other than power cell slots?
        if (obj->Implements(ObjectInterfaceType::PowerContainer) && obj->Implements(ObjectInterfaceType::Transportable))
        {
            CObject *transporter = dynamic_cast<CTransportableObject&>(*obj).GetTransporter();  // battery connected
            if (transporter != nullptr && obj == GetObjectInPowerCellSlot(transporter))
                target = transporter;
        }

        if (!obj->Implements(ObjectInterfaceType::Old)) continue;
        for (int j = 0; j < OBJECTMAXPART; j++)
        {
            int rank = obj->GetObjectRank(j);
            if (rank == -1) continue;
            if (rank != objRank) continue;
            return target;
        }
    }
    return nullptr;
}


//! Deletes the selected object
bool CRobotMain::DestroySelectedObject()
{
    CObject* obj = GetSelect();
    if (obj == nullptr) return false;
    assert(obj->Implements(ObjectInterfaceType::Controllable));

    m_engine->GetPyroManager()->Create(Gfx::PT_FRAGT, obj);

    dynamic_cast<CControllableObject&>(*obj).SetSelect(false);  // deselects the object
    m_camera->SetType(Gfx::CAM_TYPE_EXPLO);
    DeselectAll();
    RemoveFromSelectionHistory(obj);

    return true;
}


//! Removes setting evidence of the object with the mouse hovers over
void CRobotMain::HiliteClear()
{
    ClearTooltip();
    m_tooltipName.clear();  // really removes the tooltip

    if (!m_hilite) return;

    int rank = -1;
    m_engine->SetHighlightRank(&rank);  // nothing more selected

    for (CObject* obj : m_objMan->GetAllObjects())
    {
        if (!obj->Implements(ObjectInterfaceType::Controllable)) continue;
        dynamic_cast<CControllableObject&>(*obj).SetHighlight(false);
    }
    m_map->SetHighlight(nullptr);
    m_short->SetHighlight(nullptr);

    m_hilite = false;
}

//! Highlights the object with the mouse hovers over
void CRobotMain::HiliteObject(Math::Point pos)
{
    if (m_fixScene && m_phase != PHASE_APPEARANCE) return;
    if (m_movieLock) return;
    if (m_movie->IsExist()) return;
    if (m_app->GetMouseMode() == MOUSE_NONE) return;

    ClearInterface();  // removes setting evidence and tooltip

    CObject* obj = m_short->DetectShort(pos);

    std::string interfaceTooltipName;
    if (m_settings->GetTooltips() && m_interface->GetTooltip(pos, interfaceTooltipName))
    {
        m_tooltipPos = pos;
        m_tooltipName = interfaceTooltipName;
        m_tooltipTime = 0.0f;
        if (obj == nullptr) return;
    }

    if (m_suspend != nullptr) return;

    if (obj == nullptr)
    {
        bool inMap = false;
        obj = m_map->DetectMap(pos, inMap);
        if (obj == nullptr)
        {
            if (inMap)  return;

            obj = DetectObject(pos);

            if ((m_camera->GetType() == Gfx::CAM_TYPE_ONBOARD) &&
                (m_camera->GetControllingObject() == obj))
                return;
        }
    }

    if (obj != nullptr)
    {
        if (m_settings->GetTooltips())
        {
            std::string objectTooltipName = obj->GetTooltipText();
            if (!objectTooltipName.empty())
            {
                m_tooltipPos = pos;
                m_tooltipName = objectTooltipName;
                m_tooltipTime = 0.0f;
            }
        }

        if (obj->Implements(ObjectInterfaceType::Controllable) && (dynamic_cast<CControllableObject&>(*obj).GetSelectable() || m_cheatSelectInsect))
        {
            if (dynamic_cast<CControllableObject&>(*obj).GetSelectable())
            {
                // Don't highlight objects that would not be selectable without selectinsect
                dynamic_cast<CControllableObject&>(*obj).SetHighlight(true);
            }
            m_map->SetHighlight(obj);
            m_short->SetHighlight(obj);
            m_hilite = true;
        }
    }
}

//! Highlights the object with the mouse hovers over
void CRobotMain::HiliteFrame(float rTime)
{
    if (m_fixScene && m_phase != PHASE_APPEARANCE) return;
    if (m_movieLock) return;
    if (m_movie->IsExist()) return;

    m_tooltipTime += rTime;

    ClearTooltip();

    if (m_tooltipTime >= 0.2f && !m_tooltipName.empty())
    {
        CreateTooltip(m_tooltipPos, m_tooltipName);
    }
}

//! Creates a tooltip
void CRobotMain::CreateTooltip(Math::Point pos, const std::string& text)
{
    Math::Point corner;
    corner.x = pos.x+0.022f;
    corner.y = pos.y-0.052f;

    Math::Point start, end;

    m_engine->GetText()->SizeText(text, Gfx::FONT_COMMON, Gfx::FONT_SIZE_SMALL,
                                  corner, Gfx::TEXT_ALIGN_LEFT,
                                  start, end);

    start.x -= 0.010f;
    start.y -= 0.006f;
    end.x   += 0.010f;
    end.y   += 0.008f;  // small'ish margin

    pos.x = start.x;
    pos.y = start.y;

    Math::Point dim;
    dim.x = end.x-start.x;
    dim.y = end.y-start.y;

    Math::Point offset;
    offset.x = 0.0f;
    offset.y = 0.0f;
    if (pos.x+dim.x > 1.0f) offset.x = 1.0f-(pos.x+dim.x);
    if (pos.y       < 0.0f) offset.y = -pos.y;

    corner.x += offset.x;
    corner.y += offset.y;
    pos.x += offset.x;
    pos.y += offset.y;

    m_interface->CreateWindows(pos, dim, 1, EVENT_TOOLTIP);

    Ui::CWindow* pw = static_cast<Ui::CWindow*>(m_interface->SearchControl(EVENT_TOOLTIP));
    if (pw != nullptr)
    {
        pw->SetState(Ui::STATE_SHADOW);
        pw->SetTrashEvent(false);

        pos.y -= m_engine->GetText()->GetHeight(Gfx::FONT_COMMON, Gfx::FONT_SIZE_SMALL) / 2.0f;
        pw->CreateLabel(pos, dim, -1, EVENT_LABEL2, text);
    }
}

//! Clears the previous tooltip
void CRobotMain::ClearTooltip()
{
    m_interface->DeleteControl(EVENT_TOOLTIP);
}


//! Displays help for an object
void CRobotMain::HelpObject()
{
    CObject* obj = GetSelect();
    if (obj == nullptr) return;

    std::string filename = GetHelpFilename(obj->GetType());
    if (filename.empty()) return;

    StartDisplayInfo(filename, -1);
}


//! Change the mode of the camera
void CRobotMain::ChangeCamera()
{
    CObject* obj = GetSelect();
    if (obj == nullptr) return;
    assert(obj->Implements(ObjectInterfaceType::Controllable));
    auto controllableObj = dynamic_cast<CControllableObject*>(obj);

    if (controllableObj->GetCameraLock()) return;

    ObjectType oType = obj->GetType();
    Gfx::CameraType type = controllableObj->GetCameraType();

    if ( oType != OBJECT_HUMAN &&
         oType != OBJECT_TECH &&
         oType != OBJECT_MOBILEfa &&
         oType != OBJECT_MOBILEta &&
         oType != OBJECT_MOBILEwa &&
         oType != OBJECT_MOBILEia &&
         oType != OBJECT_MOBILEfb &&
         oType != OBJECT_MOBILEtb &&
         oType != OBJECT_MOBILEwb &&
         oType != OBJECT_MOBILEib &&
         oType != OBJECT_MOBILEfc &&
         oType != OBJECT_MOBILEtc &&
         oType != OBJECT_MOBILEwc &&
         oType != OBJECT_MOBILEic &&
         oType != OBJECT_MOBILEfi &&
         oType != OBJECT_MOBILEti &&
         oType != OBJECT_MOBILEwi &&
         oType != OBJECT_MOBILEii &&
         oType != OBJECT_MOBILEfs &&
         oType != OBJECT_MOBILEts &&
         oType != OBJECT_MOBILEws &&
         oType != OBJECT_MOBILEis &&
         oType != OBJECT_MOBILErt &&
         oType != OBJECT_MOBILErc &&
         oType != OBJECT_MOBILErr &&
         oType != OBJECT_MOBILErs &&
         oType != OBJECT_MOBILEsa &&
         oType != OBJECT_MOBILEtg &&
         oType != OBJECT_MOBILEft &&
         oType != OBJECT_MOBILEtt &&
         oType != OBJECT_MOBILEwt &&
         oType != OBJECT_MOBILEit &&
         oType != OBJECT_MOBILErp &&
         oType != OBJECT_MOBILEst &&
         oType != OBJECT_MOBILEdr &&
         oType != OBJECT_APOLLO2  )  return;

    if (oType == OBJECT_MOBILEdr)  // designer?
    {
             if (type == Gfx::CAM_TYPE_PLANE  )  type = Gfx::CAM_TYPE_BACK;
        else if (type == Gfx::CAM_TYPE_BACK   )  type = Gfx::CAM_TYPE_PLANE;
    }
    else if (controllableObj->GetTrainer())  // trainer?
    {
             if (type == Gfx::CAM_TYPE_ONBOARD)  type = Gfx::CAM_TYPE_FIX;
        else if (type == Gfx::CAM_TYPE_FIX    )  type = Gfx::CAM_TYPE_PLANE;
        else if (type == Gfx::CAM_TYPE_PLANE  )  type = Gfx::CAM_TYPE_BACK;
        else if (type == Gfx::CAM_TYPE_BACK   )  type = Gfx::CAM_TYPE_ONBOARD;
    }
    else
    {
             if (type == Gfx::CAM_TYPE_ONBOARD)  type = Gfx::CAM_TYPE_BACK;
        else if (type == Gfx::CAM_TYPE_BACK   )  type = Gfx::CAM_TYPE_ONBOARD;
    }

    controllableObj->SetCameraType(type);
    m_camera->SetType(type);
}


//! Cancels the current movie
void CRobotMain::AbortMovie()
{
    for (CObject* obj : m_objMan->GetAllObjects())
    {
        if (obj->Implements(ObjectInterfaceType::Old))
        {
            CAuto* automat = obj->GetAuto();
            if (automat != nullptr)
                automat->Abort();
        }
    }
}


static std::string TimeFormat(float time)
{
    int minutes = static_cast<int>(floor(time/60));
    double time2 = fmod(time, 60);
    double seconds;
    double fraction = modf(time2, &seconds)*100;
    std::ostringstream sstream;
    sstream << std::setfill('0') << std::setw(2) << minutes << ":" << std::setfill('0') << std::setw(2) << floor(seconds) << "." << std::setfill('0') << std::setw(2) << floor(fraction);
    return sstream.str();
}

//! Updates the text information
void CRobotMain::UpdateInfoText()
{
    if (m_phase == PHASE_SIMUL)
    {
        CObject* obj = GetSelect();
        if (obj != nullptr)
        {
            Math::Vector pos = obj->GetPosition();
            m_engine->SetStatisticPos(pos / g_unit);
        }
    }
    m_engine->SetTimerDisplay(m_missionTimerEnabled && m_missionTimerStarted ? TimeFormat(m_missionTimer) : "");
}


//! Initializes the view
void CRobotMain::InitEye()
{
    if (m_phase == PHASE_SIMUL)
        m_camera->Init(Math::Vector( 0.0f, 10.0f, 0.0f),
                       Math::Vector(10.0f,  5.0f, 0.0f), 0.0f);
}

//! Advances the entire scene
bool CRobotMain::EventFrame(const Event &event)
{
    m_time += event.rTime;

    m_water->EventProcess(event);
    m_cloud->EventProcess(event);
    m_lightning->EventProcess(event);
    m_planet->EventProcess(event);

    UpdateDebugCrashSpheres();

    Ui::CMap* pm = nullptr;
    Ui::CWindow* pw = static_cast<Ui::CWindow*>(m_interface->SearchControl(EVENT_WINDOW1));
    if (pw == nullptr)
    {
        pm = nullptr;
    }
    else
    {
        pm = static_cast<Ui::CMap*>(pw->SearchControl(EVENT_OBJECT_MAP));
        if (pm != nullptr) pm->FlushObject();
    }

    CObject* toto = nullptr;
    if (!m_pause->IsPauseType(PAUSE_OBJECT_UPDATES))
    {
        // Advances all the robots, but not toto.
        for (CObject* obj : m_objMan->GetAllObjects())
        {
            if (pm != nullptr)
                pm->UpdateObject(obj);

            if (IsObjectBeingTransported(obj))
                continue;

            if (obj->GetType() == OBJECT_TOTO)
                toto = obj;
            else if (obj->Implements(ObjectInterfaceType::Interactive))
                dynamic_cast<CInteractiveObject&>(*obj).EventProcess(event);

            if ( obj->GetProxyActivate() )  // active if it is near?
            {
                Math::Vector eye = m_engine->GetLookatPt();
                float dist = Math::Distance(eye, obj->GetPosition());
                if ( dist < obj->GetProxyDistance() )
                {
                    obj->SetProxyActivate(false);
                    CreateShortcuts();
                    m_sound->Play(SOUND_FINDING);
                    m_engine->GetPyroManager()->Create(Gfx::PT_FINDING, obj, 0.0f);
                    DisplayError(INFO_FINDING, obj);
                }
            }
        }
        // Advances all objects transported by robots.
        for (CObject* obj : m_objMan->GetAllObjects())
        {
            if (! IsObjectBeingTransported(obj))
                continue;

            if (obj->Implements(ObjectInterfaceType::Interactive))
                dynamic_cast<CInteractiveObject&>(*obj).EventProcess(event);
        }

        m_engine->GetPyroManager()->EventProcess(event);
    }

    // The camera follows the object, because its position
    // may depend on the selected object (Gfx::CAM_TYPE_ONBOARD or Gfx::CAM_TYPE_BACK).
    if (m_phase == PHASE_SIMUL && !m_editFull)
    {
        m_camera->EventProcess(event);

        if (m_engine->GetFog())
            m_camera->SetOverBaseColor(m_particle->GetFogColor(m_engine->GetEyePt()));
    }
    if (m_phase == PHASE_APPEARANCE ||
        m_phase == PHASE_WIN   ||
        m_phase == PHASE_LOST)
    {
        m_camera->EventProcess(event);
    }

    // Advances toto following the camera, because its position depends on the camera.
    if (toto != nullptr)
        dynamic_cast<CInteractiveObject&>(*toto).EventProcess(event);

    // NOTE: m_movieLock is set only after the first update of CAutoBase finishes

    if (m_phase == PHASE_SIMUL)
    {
        if (!m_immediatSatCom && !m_beginSatCom && !m_movieLock)
        {
            m_displayText->DisplayError(INFO_BEGINSATCOM, Math::Vector(0.0f, 0.0f, 0.0f));
            m_beginSatCom = true;  // message appears
        }

        if (!m_pause->IsPauseType(PAUSE_ENGINE) && !m_movieLock)
        {
            m_gameTime += event.rTime;
            m_gameTimeAbsolute += m_app->GetRealRelTime() / 1e9f;

            if (m_missionTimerStarted)
                m_missionTimer += event.rTime;

            if (m_autosave && m_gameTimeAbsolute >= m_autosaveLast + (m_autosaveInterval * 60))
            {
                if (m_levelCategory == LevelCategory::Missions ||
                    m_levelCategory == LevelCategory::FreeGame ||
                    m_levelCategory == LevelCategory::GamePlus ||
                    m_levelCategory == LevelCategory::CustomLevels)
                {
                    if (!IOIsBusy() && m_missionType != MISSION_CODE_BATTLE)
                    {
                        m_autosaveLast = m_gameTimeAbsolute;
                        Autosave();
                    }
                }
            }
        }
    }

    HiliteFrame(event.rTime);

    // Moves the film indicator.
    if (m_movieLock && !m_editLock)  // movie in progress?
    {
        Ui::CControl* pc = m_interface->SearchControl(EVENT_OBJECT_MOVIELOCK);
        if (pc != nullptr)
        {
            Math::Point pos, dim;

            dim.x = 32.0f/640.0f;
            dim.y = 32.0f/480.0f;
            pos.x = 20.0f/640.0f;
            pos.y = (480.0f-24.0f)/480.0f;

            float zoom = 1.0f+sinf(m_time*6.0f)*0.1f;  // 0.9 .. 1.1
            dim.x *= zoom;
            dim.y *= zoom;
            pos.x -= dim.x/2.0f;
            pos.y -= dim.y/2.0f;

            pc->SetPos(pos);
            pc->SetDim(dim);
        }
    }

    // Moves edition indicator.
    if (m_editLock || m_pause->IsPauseType(PAUSE_ENGINE))  // edition in progress?
    {
        Ui::CControl* pc = m_interface->SearchControl(EVENT_OBJECT_EDITLOCK);
        if (pc != nullptr)
        {
            Math::Point pos, dim;

            if (m_editFull || m_editLock)
            {
                dim.x = 10.0f/640.0f;
                dim.y = 10.0f/480.0f;
                pos.x = -20.0f/640.0f;
                pos.y = -20.0f/480.0f;  // invisible!
            }
            else
            {
                dim.x = 32.0f/640.0f;
                dim.y = 32.0f/480.0f;
                pos.x = (640.0f-24.0f)/640.0f;
                pos.y = (480.0f-24.0f)/480.0f;

                float zoom = 1.0f+sinf(m_time*6.0f)*0.1f;  // 0.9 .. 1.1
                dim.x *= zoom;
                dim.y *= zoom;
                pos.x -= dim.x/2.0f;
                pos.y -= dim.y/2.0f;
            }
            pc->SetPos(pos);
            pc->SetDim(dim);
        }
    }

    Ui::CControl* pc = m_interface->SearchControl(EVENT_OBJECT_SAVING);
    if (pc != nullptr)
    {
        Math::Point pos, dim;

        if (m_shotSaving <= 0)
        {
            dim.x = 10.0f/640.0f;
            dim.y = 10.0f/480.0f;
            pos.x = -20.0f/640.0f;
            pos.y = -20.0f/480.0f;  // invisible!
        }
        else
        {
            dim.x = 32.0f/640.0f;
            dim.y = 32.0f/480.0f;
            pos.x = (640.0f-24.0f)/640.0f;
            pos.y = (480.0f-24.0f)/480.0f;

            float zoom = 1.0f+sinf(m_time*6.0f)*0.1f;  // 0.9 .. 1.1
            dim.x *= zoom;
            dim.y *= zoom;
            pos.x -= dim.x/2.0f;
            pos.y -= dim.y/2.0f;
        }
        pc->SetPos(pos);
        pc->SetDim(dim);
    }

    // Will move the arrow to visit.
    if (m_camera->GetType() == Gfx::CAM_TYPE_VISIT)
        FrameVisit(event.rTime);

    // Moves the boundaries.
    FrameShowLimit(event.rTime);

    if (m_phase == PHASE_SIMUL)
    {
        if (!m_editLock && !m_engine->GetPause())
        {
            CheckEndMission(true);
            UpdateAudio(true);
            if (m_scoreboard)
                m_scoreboard->UpdateObjectCount();
        }

        if (m_winDelay > 0.0f && !m_editLock)
        {
            m_winDelay -= event.rTime;
            if (m_winDelay <= 0.0f)
            {
                if (m_movieLock)
                    m_winDelay = 1.0f;
                else
                    m_eventQueue->AddEvent(Event(EVENT_WIN));
            }
        }

        if (m_lostDelay > 0.0f && !m_editLock)
        {
            m_lostDelay -= event.rTime;
            if (m_lostDelay <= 0.0f)
            {
                if (m_movieLock)
                    m_lostDelay = 1.0f;
                else
                    m_eventQueue->AddEvent(Event(EVENT_LOST));
            }
        }

        if (GetMissionType() == MISSION_CODE_BATTLE)
        {
            if (!m_codeBattleInit)
            {
                // NOTE: It's important to do this AFTER the first update event finished processing
                //       because otherwise all robot parts are misplaced
                m_userPause = m_pause->ActivatePause(PAUSE_ENGINE);
                m_codeBattleInit = true; // Will start on resume
            }

            if (!m_codeBattleStarted && m_userPause == nullptr)
            {
                m_codeBattleStarted = true;
                ApplyCodeBattleInterface();
                CreateCodeBattleInterface();

                SetCodeBattleSpectatorMode(true);

                m_eventQueue->AddEvent(Event(EVENT_UPDINTERFACE));
            }

            UpdateCodeBattleInterface();
        }
    }

    return true;
}

void CRobotMain::ShowSaveIndicator(bool show)
{
    Ui::CControl* pc = m_interface->SearchControl(EVENT_OBJECT_SAVING);
    if (pc != nullptr)
    {
        Math::Point pos, dim;

        if (!show)
        {
            dim.x = 10.0f/640.0f;
            dim.y = 10.0f/480.0f;
            pos.x = -20.0f/640.0f;
            pos.y = -20.0f/480.0f;  // invisible!
        }
        else
        {
            dim.x = 32.0f/640.0f;
            dim.y = 32.0f/480.0f;
            pos.x = (640.0f-24.0f)/640.0f;
            pos.y = (480.0f-24.0f)/480.0f;

            pos.x -= dim.x/2.0f;
            pos.y -= dim.y/2.0f;
        }
        pc->SetPos(pos);
        pc->SetDim(dim);
    }
}

//! Makes the event for all robots
bool CRobotMain::EventObject(const Event &event)
{
    if (m_pause->IsPauseType(PAUSE_OBJECT_UPDATES)) return true;

    m_resetCreate = false;

    for (CObject* obj : m_objMan->GetAllObjects())
    {
        if (obj->Implements(ObjectInterfaceType::Interactive))
        {
            dynamic_cast<CInteractiveObject&>(*obj).EventProcess(event);
        }
    }

    if (m_resetCreate)
        ResetCreate();

    return true;
}



void CRobotMain::ScenePerso()
{
    DeleteAllObjects();  // removes all the current 3D Scene
    m_terrain->FlushRelief();
    m_engine->DeleteAllObjects();
    m_oldModelManager->DeleteAllModelCopies();
    m_terrain->FlushBuildingLevel();
    m_terrain->FlushFlyingLimit();
    m_lightMan->FlushLights();
    m_particle->FlushParticle();

    m_levelFile = "levels/other/perso.txt";
    try
    {
        CreateScene(false, true, false);  // sets scene
    }
    catch (const std::runtime_error& e)
    {
        LevelLoadingError("An error occurred while trying to load appearance scene", e, PHASE_PLAYER_SELECT);
    }

    m_engine->SetDrawWorld(false);  // does not draw anything on the interface
    m_engine->SetDrawFront(true);  // draws on the human interface
    CObject* obj = SearchHuman();
    if (obj != nullptr)
    {
        obj->SetDrawFront(true);  // draws the interface

        assert(obj->Implements(ObjectInterfaceType::Movable));
        CMotionHuman* mh = static_cast<CMotionHuman*>(dynamic_cast<CMovableObject&>(*obj).GetMotion());
        mh->StartDisplayPerso();
    }
}

//! Creates the whole scene
void CRobotMain::CreateScene(bool soluce, bool fixScene, bool resetObject)
{
    m_fixScene = fixScene;

    m_base = nullptr;

    if (!resetObject)
    {
        m_build = 0;
        m_researchDone.clear();  // no research done
        m_researchDone[0] = 0;
        m_researchEnable = 0;

        g_unit = UNIT;

        FlushDisplayInfo();
        m_terrain->FlushMaterials();
        m_audioTrack = "";
        m_audioRepeat = true;
        m_satcomTrack  = "";
        m_satcomRepeat = true;
        m_editorTrack  = "";
        m_editorRepeat = true;
        m_displayText->SetDelay(1.0f);
        m_displayText->SetEnable(true);
        m_immediatSatCom = false;
        m_lockedSatCom = false;
        m_endingWin = "";
        m_endingLost = "";
        m_audioChange.clear();
        m_endTake.clear();
        m_endTakeImmediat = false;
        m_endTakeResearch = 0;
        m_endTakeTimeout = -1.0f;
        m_endTakeTeamImmediateWin = false;
        m_endTakeWinDelay = 2.0f;
        m_endTakeLostDelay = 2.0f;
        m_teamFinished.clear();
        m_scoreboard.reset();
        m_globalMagnifyDamage = 1.0f;
        m_obligatoryTokens.clear();
        m_mapShow = true;
        m_mapImage = false;
        m_mapFilename[0] = 0;

        m_controller = nullptr;

        m_colorNewBot.clear();
        m_colorNewBot[0] = COLOR_REF_BOT;
        m_colorNewAlien = COLOR_REF_ALIEN;
        m_colorNewGreen = COLOR_REF_GREEN;
        m_colorNewWater = COLOR_REF_WATER;

        m_engine->SetAmbientColor(Gfx::Color(0.5f, 0.5f, 0.5f, 0.5f), 0);
        m_engine->SetAmbientColor(Gfx::Color(0.5f, 0.5f, 0.5f, 0.5f), 1);
        m_engine->SetFogColor(Gfx::Color(1.0f, 1.0f, 1.0f, 1.0f), 0);
        m_engine->SetFogColor(Gfx::Color(1.0f, 1.0f, 1.0f, 1.0f), 1);
        m_engine->SetDeepView(1000.0f, 0);
        m_engine->SetDeepView(1000.0f, 1);
        m_engine->SetFogStart(0.75f, 0);
        m_engine->SetFogStart(0.75f, 1);
        m_engine->SetSecondTexture("");
        m_engine->SetForegroundName("");

        GetResource(RES_TEXT, RT_SCRIPT_NEW, m_scriptName);
        m_scriptFile = "";

        m_missionType   = MISSION_NORMAL;
        m_codeBattleInit = false;
        m_codeBattleStarted = false;

        m_teamNames.clear();

        m_missionResult = ERR_MISSION_NOTERM;
        m_missionResultFromScript = false;
    }

    // NOTE: Reset timer always, even when only resetting object positions
    m_missionTimerEnabled = false;
    m_missionTimerStarted = false;
    m_missionTimer = 0.0f;

    std::string backgroundPath = "";
    Gfx::Color backgroundUp = Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f);
    Gfx::Color backgroundDown = Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f);
    Gfx::Color backgroundCloudUp = Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f);
    Gfx::Color backgroundCloudDown = Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f);
    bool backgroundFull = false;

    auto LoadingWarning = [&](const std::string& message)
    {
        GetLogger()->Warn("%s\n", message.c_str());
        m_ui->GetDialog()->StartInformation("Level loading warning", "This level contains problems. It may stop working in future versions of the game.", message);
    };

    try
    {
        m_ui->GetLoadingScreen()->SetProgress(0.05f, RT_LOADING_PROCESSING);
        GetLogger()->Info("Loading level: %s\n", m_levelFile.c_str());
        CLevelParser levelParser(m_levelFile);
        levelParser.SetLevelPaths(m_levelCategory, m_levelChap, m_levelRank);
        levelParser.Load();
        int numObjects = levelParser.CountLines("CreateObject");
        m_ui->GetLoadingScreen()->SetProgress(0.1f, RT_LOADING_LEVEL_SETTINGS);

        int rankObj = 0;
        CObject* sel = nullptr;

        for (auto& line : levelParser.GetLines())
        {
            if (line->GetCommand() == "Title" && !resetObject)
            {
                //strcpy(m_title, line->GetParam("text")->AsString().c_str());
                continue;
            }

            if (line->GetCommand() == "Resume" && !resetObject)
            {
                //strcpy(m_resume, line->GetParam("text")->AsString().c_str());
                continue;
            }

            if (line->GetCommand() == "ScriptName" && !resetObject)
            {
                m_scriptName = line->GetParam("text")->AsString();
                continue;
            }

            if (line->GetCommand() == "ScriptFile" && !resetObject)
            {
                m_scriptFile = line->GetParam("name")->AsString();
                continue;
            }

            if (line->GetCommand() == "Instructions" && !resetObject)
            {
                strcpy(m_infoFilename[SATCOM_HUSTON], line->GetParam("name")->AsPath("help/%lng%").c_str());

                m_immediatSatCom = line->GetParam("immediat")->AsBool(false);
                m_beginSatCom = m_lockedSatCom = line->GetParam("lock")->AsBool(false);
                if (m_app->GetSceneTestMode()) m_immediatSatCom = false;
                continue;
            }

            if (line->GetCommand() == "Satellite" && !resetObject)
            {
                strcpy(m_infoFilename[SATCOM_SAT], line->GetParam("name")->AsPath("help/%lng%").c_str());
                continue;
            }

            if (line->GetCommand() == "Loading" && !resetObject)
            {
                strcpy(m_infoFilename[SATCOM_LOADING], line->GetParam("name")->AsPath("help/%lng%").c_str());
                continue;
            }

            if (line->GetCommand() == "HelpFile" && !resetObject)
            {
                strcpy(m_infoFilename[SATCOM_PROG], line->GetParam("name")->AsPath("help/%lng%").c_str());
                continue;
            }
            if (line->GetCommand() == "SoluceFile" && !resetObject)
            {
                strcpy(m_infoFilename[SATCOM_SOLUCE], line->GetParam("name")->AsPath("help/%lng%").c_str());
                continue;
            }

            if (line->GetCommand() == "EndingFile" && !resetObject)
            {
                auto Process = [&](const std::string& type) -> std::string
                {
                    if (line->GetParam(type)->IsDefined())
                    {
                        try
                        {
                            int rank = boost::lexical_cast<int>(line->GetParam(type)->GetValue());
                            if (rank >= 0)
                            {
                                // TODO: Fix default levels and add a future removal warning
                                GetLogger()->Warn("This level is using deprecated way of defining %1$s scene. Please change the %1$s= parameter in EndingFile from %2$d to \"levels/other/%1$s%2$03d.txt\".\n", type.c_str(), rank);
                                std::stringstream ss;
                                ss << "levels/other/" << type << std::setfill('0') << std::setw(3) << rank << ".txt";
                                return ss.str();
                            }
                            else
                            {
                                // TODO: Fix default levels and add a future removal warning
                                GetLogger()->Warn("This level is using deprecated way of defining %1$s scene. Please remove the %1$s= parameter in EndingFile.\n", type.c_str());
                                return "";
                            }

                        }
                        catch (boost::bad_lexical_cast &e)
                        {
                            return line->GetParam(type)->AsPath("levels");
                        }
                    }
                    return "";
                };
                m_endingWin = Process("win");
                m_endingLost = Process("lost");
                continue;
            }

            if (line->GetCommand() == "MessageDelay" && !resetObject)
            {
                m_displayText->SetDelay(line->GetParam("factor")->AsFloat());
                continue;
            }

            if (line->GetCommand() == "MissionTimer")
            {
                m_missionTimerEnabled = line->GetParam("enabled")->AsBool();
                if (!line->GetParam("program")->AsBool(false))
                {
                    m_missionTimerStarted = true;
                }
                continue;
            }

            if (line->GetCommand() == "TeamName")
            {
                int team = line->GetParam("team")->AsInt();
                std::string name = line->GetParam("name")->AsString();
                m_teamNames[team] = name;
                continue;
            }

            if (line->GetCommand() == "CacheAudio" && !resetObject)
            {
                std::string filename = line->GetParam("filename")->AsPath("music");
                m_ui->GetLoadingScreen()->SetProgress(0.15f, RT_LOADING_MUSIC, filename);
                m_sound->CacheMusic(filename);
                continue;
            }

            if (line->GetCommand() == "AudioChange" && !resetObject)
            {
                auto audioChange = MakeUnique<CAudioChangeCondition>();
                audioChange->Read(line.get());
                m_ui->GetLoadingScreen()->SetProgress(0.15f, RT_LOADING_MUSIC, audioChange->music);
                m_sound->CacheMusic(audioChange->music);
                m_audioChange.push_back(std::move(audioChange));

                if (!line->GetParam("pos")->IsDefined() || !line->GetParam("dist")->IsDefined())
                {
                    LoadingWarning("The defaults for pos= and dist= are going to change, specify them explicitly. See issue #759 (https://git.io/vVBzH)");
                }
                continue;
            }

            if (line->GetCommand() == "Audio" && !resetObject)
            {
                if (line->GetParam("track")->IsDefined())
                {
                    if (line->GetParam("filename")->IsDefined())
                        throw CLevelParserException("You can't use track and filename at the same time");

                    GetLogger()->Warn("Using track= is deprecated. Please replace this with filename=\n");
                    int trackid = line->GetParam("track")->AsInt();
                    if (trackid != 0)
                    {
                        std::stringstream filenameStr;
                        filenameStr << "music/music" << std::setfill('0') << std::setw(3) << trackid << ".ogg";
                        m_audioTrack = filenameStr.str();
                    }
                    else
                    {
                        m_audioTrack = "";
                    }
                }
                else
                {
                    if (line->GetParam("filename")->IsDefined())
                    {
                        m_audioTrack = line->GetParam("filename")->AsPath("music");
                    }
                    else
                    {
                        m_audioTrack = "";
                    }
                }
                if (!m_audioTrack.empty())
                {
                    m_audioRepeat = line->GetParam("repeat")->AsBool(true);
                }

                if (line->GetParam("satcom")->IsDefined())
                {
                    m_satcomTrack = line->GetParam("satcom")->AsPath("music");
                    m_satcomRepeat = line->GetParam("satcomRepeat")->AsBool(true);
                }
                else
                {
                    m_satcomTrack = "";
                }

                if (line->GetParam("editor")->IsDefined())
                {
                    m_editorTrack = line->GetParam("editor")->AsPath("music");
                    m_editorRepeat = line->GetParam("editorRepeat")->AsBool(true);
                }
                else
                {
                    m_editorTrack = "";
                }

                if (!m_audioTrack.empty())
                {
                    m_ui->GetLoadingScreen()->SetProgress(0.15f, RT_LOADING_MUSIC, m_audioTrack);
                    m_sound->CacheMusic(m_audioTrack);
                }
                if (!m_satcomTrack.empty())
                {
                    m_ui->GetLoadingScreen()->SetProgress(0.15f, RT_LOADING_MUSIC, m_satcomTrack);
                    m_sound->CacheMusic(m_satcomTrack);
                }
                if (!m_editorTrack.empty())
                {
                    m_ui->GetLoadingScreen()->SetProgress(0.15f, RT_LOADING_MUSIC, m_editorTrack);
                    m_sound->CacheMusic(m_editorTrack);
                }
                continue;
            }

            if (line->GetCommand() == "AmbientColor" && !resetObject)
            {
                m_engine->SetAmbientColor(line->GetParam("air")->AsColor(Gfx::Color(0.533f, 0.533f, 0.533f, 0.533f)), 0);
                m_engine->SetAmbientColor(line->GetParam("water")->AsColor(Gfx::Color(0.533f, 0.533f, 0.533f, 0.533f)), 1);
                continue;
            }

            if (line->GetCommand() == "FogColor" && !resetObject)
            {
                m_engine->SetFogColor(line->GetParam("air")->AsColor(Gfx::Color(0.533f, 0.533f, 0.533f, 0.533f)), 0);
                m_engine->SetFogColor(line->GetParam("water")->AsColor(Gfx::Color(0.533f, 0.533f, 0.533f, 0.533f)), 1);
                continue;
            }

            if (line->GetCommand() == "VehicleColor" && !resetObject)
            {
                m_colorNewBot[line->GetParam("team")->AsInt(0)] = line->GetParam("color")->AsColor(Gfx::Color(0.533f, 0.533f, 0.533f, 0.533f));
                continue;
            }

            if (line->GetCommand() == "InsectColor" && !resetObject)
            {
                m_colorNewAlien = line->GetParam("color")->AsColor(Gfx::Color(0.533f, 0.533f, 0.533f, 0.533f));
                continue;
            }

            if (line->GetCommand() == "GreeneryColor" && !resetObject)
            {
                m_colorNewGreen = line->GetParam("color")->AsColor(Gfx::Color(0.533f, 0.533f, 0.533f, 0.533f));
                continue;
            }

            if (line->GetCommand() == "DeepView" && !resetObject)
            {
                m_engine->SetDeepView(line->GetParam("air")->AsFloat(500.0f)*g_unit, 0, false);
                m_engine->SetDeepView(line->GetParam("water")->AsFloat(100.0f)*g_unit, 1, false);
                continue;
            }

            if (line->GetCommand() == "FogStart" && !resetObject)
            {
                m_engine->SetFogStart(line->GetParam("air")->AsFloat(0.5f), 0);
                m_engine->SetFogStart(line->GetParam("water")->AsFloat(0.5f), 1);
                continue;
            }

            if (line->GetCommand() == "SecondTexture" && !resetObject)
            {
                if (line->GetParam("rank")->IsDefined())
                {
                    char tex[20] = { 0 };
                    sprintf(tex, "dirty%.2d.png", line->GetParam("rank")->AsInt());
                    m_engine->SetSecondTexture(tex);
                }
                else
                {
                    m_engine->SetSecondTexture("../" + line->GetParam("texture")->AsPath("textures"));
                }
                continue;
            }

            if (line->GetCommand() == "Background" && !resetObject)
            {
                if (line->GetParam("image")->IsDefined())
                    backgroundPath = line->GetParam("image")->AsPath("textures");
                backgroundUp = line->GetParam("up")->AsColor(backgroundUp);
                backgroundDown = line->GetParam("down")->AsColor(backgroundDown);
                backgroundCloudUp = line->GetParam("cloudUp")->AsColor(backgroundCloudUp);
                backgroundCloudDown = line->GetParam("cloudDown")->AsColor(backgroundCloudDown);
                backgroundFull = line->GetParam("full")->AsBool(backgroundFull);
                continue;
            }

            if (line->GetCommand() == "Planet" && !resetObject)
            {
                Math::Vector    ppos, uv1, uv2;

                ppos  = line->GetParam("pos")->AsPoint();
                uv1   = line->GetParam("uv1")->AsPoint();
                uv2   = line->GetParam("uv2")->AsPoint();
                m_planet->Create(line->GetParam("mode")->AsPlanetType(),
                                Math::Point(ppos.x, ppos.z),
                                line->GetParam("dim")->AsFloat(0.2f),
                                line->GetParam("speed")->AsFloat(0.0f),
                                line->GetParam("dir")->AsFloat(0.0f),
                                line->GetParam("image")->AsPath("textures"),
                                Math::Point(uv1.x, uv1.z),
                                Math::Point(uv2.x, uv2.z),
                                line->GetParam("image")->AsPath("textures").find("planet") != std::string::npos // TODO: add transparent op or modify textures
                );
                continue;
            }

            if (line->GetCommand() == "ForegroundName" && !resetObject)
            {
                m_engine->SetForegroundName(line->GetParam("image")->AsPath("textures"));
                continue;
            }

            if (line->GetCommand() == "Level" && !resetObject)
            {
                g_unit = line->GetParam("unitScale")->AsFloat(4.0f);
                m_engine->SetTracePrecision(line->GetParam("traceQuality")->AsFloat(1.0f));
                m_shortCut = line->GetParam("shortcut")->AsBool(true);

                m_missionType = line->GetParam("type")->AsMissionType(MISSION_NORMAL);
                m_globalMagnifyDamage = line->GetParam("magnifyDamage")->AsFloat(1.0f);
                m_globalNuclearCapacity = line->GetParam("nuclearCapacity")->AsFloat(10.0f);
                m_globalCellCapacity = line->GetParam("cellCapacity")->AsFloat(1.0f);

                continue;
            }

            if (line->GetCommand() == "TerrainGenerate" && !resetObject)
            {
                m_ui->GetLoadingScreen()->SetProgress(0.2f, RT_LOADING_TERRAIN);
                m_terrain->Generate(line->GetParam("mosaic")->AsInt(20),
                                    line->GetParam("brick")->AsInt(3),
                                    line->GetParam("size")->AsFloat(20.0f),
                                    line->GetParam("vision")->AsFloat(500.0f)*g_unit,
                                    line->GetParam("depth")->AsInt(2),
                                    line->GetParam("hard")->AsFloat(0.5f));
                continue;
            }

            if (line->GetCommand() == "TerrainWind" && !resetObject)
            {
                m_terrain->SetWind(line->GetParam("speed")->AsPoint());
                continue;
            }

            if (line->GetCommand() == "TerrainRelief" && !resetObject)
            {
                m_ui->GetLoadingScreen()->SetProgress(0.2f+(1.f/5.f)*0.05f, RT_LOADING_TERRAIN, RT_LOADING_TERRAIN_RELIEF);
                m_terrain->LoadRelief(
                    line->GetParam("image")->AsPath("textures"),
                    line->GetParam("factor")->AsFloat(1.0f),
                    line->GetParam("border")->AsBool(true));
                continue;
            }

            if (line->GetCommand() == "TerrainRandomRelief" && !resetObject)
            {
                m_ui->GetLoadingScreen()->SetProgress(0.2f+(1.f/5.f)*0.05f, RT_LOADING_TERRAIN, RT_LOADING_TERRAIN_RELIEF);
                m_terrain->RandomizeRelief();
                continue;
            }

            if (line->GetCommand() == "TerrainResource" && !resetObject)
            {
                m_ui->GetLoadingScreen()->SetProgress(0.2f+(2.f/5.f)*0.05f, RT_LOADING_TERRAIN, RT_LOADING_TERRAIN_RES);
                m_terrain->LoadResources(line->GetParam("image")->AsPath("textures"));
                continue;
            }

            if (line->GetCommand() == "TerrainWater" && !resetObject)
            {
                Math::Vector pos;
                pos.x = line->GetParam("moveX")->AsFloat(0.0f);
                pos.y = line->GetParam("moveY")->AsFloat(0.0f);
                pos.z = pos.x;
                m_water->Create(line->GetParam("air")->AsWaterType(Gfx::WATER_TT),
                                line->GetParam("water")->AsWaterType(Gfx::WATER_TT),
                                line->GetParam("image")->AsPath("textures"),
                                line->GetParam("diffuse")->AsColor(Gfx::Color(1.0f, 1.0f, 1.0f, 1.0f)),
                                line->GetParam("ambient")->AsColor(Gfx::Color(1.0f, 1.0f, 1.0f, 1.0f)),
                                line->GetParam("level")->AsFloat(100.0f)*g_unit,
                                line->GetParam("glint")->AsFloat(1.0f),
                                pos);
                m_colorNewWater = line->GetParam("color")->AsColor(COLOR_REF_WATER);
                m_colorShiftWater = line->GetParam("brightness")->AsFloat(0.0f);
                continue;
            }

            if (line->GetCommand() == "TerrainLava" && !resetObject)
            {
                m_water->SetLava(line->GetParam("mode")->AsBool());
                continue;
            }

            if (line->GetCommand() == "TerrainCloud" && !resetObject)
            {
                std::string path = "";
                if (line->GetParam("image")->IsDefined())
                    path = line->GetParam("image")->AsPath("textures");
                m_cloud->Create(path,
                                line->GetParam("diffuse")->AsColor(Gfx::Color(1.0f, 1.0f, 1.0f, 1.0f)),
                                line->GetParam("ambient")->AsColor(Gfx::Color(1.0f, 1.0f, 1.0f, 1.0f)),
                                line->GetParam("level")->AsFloat(500.0f)*g_unit);
                continue;
            }

            if (line->GetCommand() == "TerrainBlitz" && !resetObject)
            {
                m_lightning->Create(line->GetParam("sleep")->AsFloat(0.0f),
                                    line->GetParam("delay")->AsFloat(3.0f),
                                    line->GetParam("magnetic")->AsFloat(50.0f)*g_unit);
                continue;
            }

            if (line->GetCommand() == "TerrainInitTextures" && !resetObject)
            {
                m_ui->GetLoadingScreen()->SetProgress(0.2f+(3.f/5.f)*0.05f, RT_LOADING_TERRAIN, RT_LOADING_TERRAIN_TEX);
                std::string name = "../" + line->GetParam("image")->AsPath("textures");
                if (name.find(".") == std::string::npos)
                    name += ".png";
                unsigned int dx = line->GetParam("dx")->AsInt(1);
                unsigned int dy = line->GetParam("dy")->AsInt(1);

                int tt[100]; //TODO: I have no idea how TerrainInitTextures works, but maybe we shuld remove the limit to 100?
                if (dx*dy > 100)
                    throw CLevelParserException("In TerrainInitTextures: dx*dy must be <100");
                if (line->GetParam("table")->IsDefined())
                {
                    auto& table = line->GetParam("table")->AsArray();

                    if (table.size() > dx*dy)
                        throw CLevelParserException("In TerrainInitTextures: table size must be dx*dy");

                    for (unsigned int i = 0; i < dx*dy; i++)
                    {
                        if (i >= table.size())
                        {
                            tt[i] = 0;
                        }
                        else
                        {
                            tt[i] = table[i]->AsInt();
                        }
                    }
                }
                else
                {
                    for (unsigned int i = 0; i < dx*dy; i++)
                    {
                        tt[i] = 0;
                    }
                }

                m_terrain->InitTextures(name.c_str(), tt, dx, dy);
                continue;
            }

            if (line->GetCommand() == "TerrainInit" && !resetObject)
            {
                m_terrain->InitMaterials(line->GetParam("id")->AsInt(1));
                continue;
            }

            if (line->GetCommand() == "TerrainMaterial" && !resetObject)
            {
                std::string name = line->GetParam("image")->AsPath("textures");
                if (name.find(".") == std::string::npos)
                    name += ".png";
                name = "../" + name;

                m_terrain->AddMaterial(line->GetParam("id")->AsInt(0),
                                    name.c_str(),
                                    Math::Point(line->GetParam("u")->AsFloat(),
                                                line->GetParam("v")->AsFloat()),
                                    line->GetParam("up")->AsInt(),
                                    line->GetParam("right")->AsInt(),
                                    line->GetParam("down")->AsInt(),
                                    line->GetParam("left")->AsInt(),
                                    line->GetParam("hard")->AsFloat(0.5f));
                continue;
            }

            if (line->GetCommand() == "TerrainLevel" && !resetObject)
            {
                m_ui->GetLoadingScreen()->SetProgress(0.2f+(3.f/5.f)*0.05f, RT_LOADING_TERRAIN, RT_LOADING_TERRAIN_TEX);
                int id[50]; //TODO: I have no idea how TerrainLevel works, but maybe we should remove the limit to 50?
                if (line->GetParam("id")->IsDefined())
                {
                    auto& idArray = line->GetParam("id")->AsArray();

                    if (idArray.size() > 50)
                        throw CLevelParserException("In TerrainLevel: id array size must be < 50");

                    unsigned int i = 0;
                    while (i < 50)
                    {
                        id[i] = idArray[i]->AsInt();
                        i++;
                        if (i >= idArray.size()) break;
                    }
                    id[i] = 0;
                }

                m_terrain->GenerateMaterials(id,
                                            line->GetParam("min")->AsFloat(0.0f)*g_unit,
                                            line->GetParam("max")->AsFloat(100.0f)*g_unit,
                                            line->GetParam("slope")->AsFloat(5.0f),
                                            line->GetParam("freq")->AsFloat(100.0f),
                                            line->GetParam("center")->AsPoint(Math::Vector(0.0f, 0.0f, 0.0f))*g_unit,
                                            line->GetParam("radius")->AsFloat(0.0f)*g_unit);
                continue;
            }

            if (line->GetCommand() == "TerrainCreate" && !resetObject)
            {
                m_ui->GetLoadingScreen()->SetProgress(0.2f+(4.f/5.f)*0.05f, RT_LOADING_TERRAIN, RT_LOADING_TERRAIN_GEN);
                m_terrain->CreateObjects();
                continue;
            }

            if (line->GetCommand() == "BeginObject")
            {
                InitEye();
                SetMovieLock(false);

                if (!resetObject)
                    ChangeColor();  // changes the colors of texture

                if (!m_sceneReadPath.empty())  // loading file ?
                {
                    m_ui->GetLoadingScreen()->SetProgress(0.25f, RT_LOADING_OBJECTS_SAVED);
                    sel = IOReadScene(m_sceneReadPath + "/data.sav", m_sceneReadPath + "/cbot.run");
                }
                else
                {
                    m_ui->GetLoadingScreen()->SetProgress(0.25f, RT_LOADING_OBJECTS);
                }

                continue;
            }

            if (line->GetCommand() == "LevelController" && m_sceneReadPath.empty())
            {
                if (m_controller != nullptr)
                {
                    throw CLevelParserException("There can be only one LevelController in the level");
                }

                m_controller = m_objMan->CreateObject(Math::Vector(0.0f, 0.0f, 0.0f), 0.0f, OBJECT_CONTROLLER);
                assert(m_controller->Implements(ObjectInterfaceType::Programmable));
                assert(m_controller->Implements(ObjectInterfaceType::ProgramStorage));

                assert(m_controller->Implements(ObjectInterfaceType::Old));
                dynamic_cast<COldObject&>(*m_controller).SetCheckToken(false);

                if (line->GetParam("script")->IsDefined())
                {
                    CProgramStorageObject* programStorage = dynamic_cast<CProgramStorageObject*>(m_controller);
                    Program* program = programStorage->AddProgram();
                    programStorage->ReadProgram(program, line->GetParam("script")->AsPath("ai"));
                    program->readOnly = true;
                    dynamic_cast<CProgrammableObject&>(*m_controller).RunProgram(program);
                }
                continue;
            }

            if (line->GetCommand() == "CreateObject" && m_sceneReadPath.empty())
            {
                ObjectCreateParams params = CObject::ReadCreateParams(line.get());

                float objectProgress = static_cast<float>(rankObj) / static_cast<float>(numObjects);
                std::string details = StrUtils::ToString<int>(rankObj+1)+" / "+StrUtils::ToString<int>(numObjects);
                #if DEV_BUILD
                // Object categories may spoil the level a bit, so hide them in release builds
                details += ": "+CLevelParserParam::FromObjectType(params.type);
                #endif
                m_ui->GetLoadingScreen()->SetProgress(0.25f+objectProgress*0.75f, RT_LOADING_OBJECTS, details);

                try
                {
                    CObject* obj = m_objMan->CreateObject(params);
                    obj->Read(line.get());

                    if (m_fixScene && obj->GetType() == OBJECT_HUMAN)
                    {
                        assert(obj->Implements(ObjectInterfaceType::Movable));
                        CMotion* motion = dynamic_cast<CMovableObject&>(*obj).GetMotion();
                        if (m_phase == PHASE_WIN ) motion->SetAction(MHS_WIN,  0.4f);
                        if (m_phase == PHASE_LOST) motion->SetAction(MHS_LOST, 0.5f);
                    }

                    if (obj->Implements(ObjectInterfaceType::Controllable) && line->GetParam("select")->AsBool(false))
                        sel = obj;

                    if (obj->GetType() == OBJECT_BASE)
                        m_base = obj;

                    if (obj->Implements(ObjectInterfaceType::ProgramStorage))
                    {
                        CProgramStorageObject* programStorage = dynamic_cast<CProgramStorageObject*>(obj);

                        if (obj->Implements(ObjectInterfaceType::Controllable) && dynamic_cast<CControllableObject&>(*obj).GetSelectable() && obj->GetType() != OBJECT_HUMAN)
                        {
                            programStorage->SetProgramStorageIndex(rankObj);
                        }

                        char categoryChar = GetLevelCategoryDir(m_levelCategory)[0];
                        programStorage->LoadAllProgramsForLevel(
                            line.get(),
                            m_playerProfile->GetSaveFile(StrUtils::Format("%c%.3d%.3d", categoryChar, m_levelChap, m_levelRank)),
                            soluce
                        );
                    }
                }
                catch (const CObjectCreateException& e)
                {
                    GetLogger()->Error("Error loading level object: %s\n", e.what());
                    throw;
                }

                rankObj ++;
                continue;
            }

            if (line->GetCommand() == "CreateFog" && !resetObject)
            {
                Gfx::ParticleType type = static_cast<Gfx::ParticleType>(Gfx::PARTIFOG0+(line->GetParam("type")->AsInt()));
                Math::Vector pos = line->GetParam("pos")->AsPoint()*g_unit;
                float height = line->GetParam("height")->AsFloat(1.0f)*g_unit;
                float ddim = line->GetParam("dim")->AsFloat(50.0f)*g_unit;
                float delay = line->GetParam("delay")->AsFloat(2.0f);
                m_terrain->AdjustToFloor(pos);
                pos.y += height;
                Math::Point dim;
                dim.x = ddim;
                dim.y = dim.x;
                m_particle->CreateParticle(pos, Math::Vector(0.0f, 0.0f, 0.0f), dim, type, delay, 0.0f, 0.0f);
                continue;
            }

            if (line->GetCommand() == "CreateLight" && !resetObject)
            {
                Gfx::EngineObjectType  type;

                int lightRank = CreateLight(line->GetParam("dir")->AsPoint(),
                                            line->GetParam("color")->AsColor(Gfx::Color(0.5f, 0.5f, 0.5f, 1.0f)));

                type = line->GetParam("type")->AsTerrainType(Gfx::ENG_OBJTYPE_NULL);

                if (type == Gfx::ENG_OBJTYPE_TERRAIN)
                {
                    m_lightMan->SetLightPriority(lightRank, Gfx::LIGHT_PRI_HIGHEST);
                    m_lightMan->SetLightIncludeType(lightRank, Gfx::ENG_OBJTYPE_TERRAIN);
                }

                if (type == Gfx::ENG_OBJTYPE_QUARTZ)
                    m_lightMan->SetLightIncludeType(lightRank, Gfx::ENG_OBJTYPE_QUARTZ);

                if (type == Gfx::ENG_OBJTYPE_METAL)
                    m_lightMan->SetLightIncludeType(lightRank, Gfx::ENG_OBJTYPE_METAL);

                if (type == Gfx::ENG_OBJTYPE_FIX)
                    m_lightMan->SetLightExcludeType(lightRank, Gfx::ENG_OBJTYPE_TERRAIN);

                continue;
            }
            if (line->GetCommand() == "CreateSpot" && !resetObject)
            {
                Gfx::EngineObjectType  type;

                int rankLight = CreateSpot(line->GetParam("pos")->AsPoint()*g_unit,
                                        line->GetParam("color")->AsColor(Gfx::Color(0.5f, 0.5f, 0.5f, 1.0f)));

                type = line->GetParam("type")->AsTerrainType(Gfx::ENG_OBJTYPE_NULL);
                if (type == Gfx::ENG_OBJTYPE_TERRAIN)
                    m_lightMan->SetLightIncludeType(rankLight, Gfx::ENG_OBJTYPE_TERRAIN);

                if (type == Gfx::ENG_OBJTYPE_QUARTZ)
                    m_lightMan->SetLightIncludeType(rankLight, Gfx::ENG_OBJTYPE_QUARTZ);

                if (type == Gfx::ENG_OBJTYPE_METAL)
                    m_lightMan->SetLightIncludeType(rankLight, Gfx::ENG_OBJTYPE_METAL);

                if (type == Gfx::ENG_OBJTYPE_FIX)
                    m_lightMan->SetLightExcludeType(rankLight, Gfx::ENG_OBJTYPE_TERRAIN);

                continue;
            }

            if (line->GetCommand() == "GroundSpot" && !resetObject)
            {
                int rank = m_engine->CreateGroundSpot();
                if (rank != -1)
                {
                    m_engine->SetObjectGroundSpotPos(rank, line->GetParam("pos")->AsPoint(Math::Vector(0.0f, 0.0f, 0.0f))*g_unit);
                    m_engine->SetObjectGroundSpotRadius(rank, line->GetParam("radius")->AsFloat(10.0f)*g_unit);
                    m_engine->SetObjectGroundSpotColor(rank, line->GetParam("color")->AsColor(Gfx::Color(0.533f, 0.533f, 0.533f, 0.533f)));
                    m_engine->SetObjectGroundSpotSmooth(rank, line->GetParam("smooth")->AsFloat(1.0f));
                    m_engine->SetObjectGroundSpotMinMax(rank, line->GetParam("min")->AsFloat(0.0f)*g_unit,
                                                        line->GetParam("max")->AsFloat(0.0f)*g_unit);
                }
                continue;
            }

            if (line->GetCommand() == "WaterColor" && !resetObject)
            {
                m_engine->SetWaterAddColor(line->GetParam("color")->AsColor());
                continue;
            }

            if (line->GetCommand() == "MapColor" && !resetObject)
            {
                m_map->FloorColorMap(line->GetParam("floor")->AsColor(Gfx::Color(0.533f, 0.533f, 0.533f, 0.533f)),
                                    line->GetParam("water")->AsColor(Gfx::Color(0.533f, 0.533f, 0.533f, 0.533f)));
                m_mapShow = line->GetParam("show")->AsBool(true);
                m_map->SetToy(line->GetParam("toyIcon")->AsBool(false));
                m_mapImage = line->GetParam("image")->AsBool(false);
                if (m_mapImage)
                {
                    Math::Vector offset;
                    strcpy(m_mapFilename, line->GetParam("filename")->AsPath("textures").c_str());
                    offset = line->GetParam("offset")->AsPoint(Math::Vector(0.0f, 0.0f, 0.0f));
                    m_map->SetFixParam(line->GetParam("zoom")->AsFloat(1.0f),
                                    offset.x, offset.z,
                                    line->GetParam("angle")->AsFloat(0.0f)*Math::PI/180.0f,
                                    line->GetParam("mode")->AsInt(0),
                                    line->GetParam("debug")->AsBool(false));
                }
                continue;
            }

            if (line->GetCommand() == "MapZoom" && !resetObject)
            {
                m_map->ZoomMap(line->GetParam("factor")->AsFloat(2.0f));
                m_map->MapEnable(line->GetParam("enable")->AsBool(true));
                continue;
            }

            if (line->GetCommand() == "MaxFlyingHeight" && !resetObject)
            {
                m_terrain->SetFlyingMaxHeight(line->GetParam("max")->AsFloat(280.0f)*g_unit);
                continue;
            }

            if (line->GetCommand() == "AddFlyingHeight" && !resetObject)
            {
                m_terrain->AddFlyingLimit(line->GetParam("center")->AsPoint()*g_unit,
                                        line->GetParam("extRadius")->AsFloat(20.0f)*g_unit,
                                        line->GetParam("intRadius")->AsFloat(10.0f)*g_unit,
                                        line->GetParam("maxHeight")->AsFloat(200.0f));
                continue;
            }

            if (line->GetCommand() == "Camera")
            {
                m_camera->Init(line->GetParam("eye")->AsPoint(Math::Vector(0.0f, 0.0f, 0.0f))*g_unit,
                            line->GetParam("lookat")->AsPoint(Math::Vector(0.0f, 0.0f, 0.0f))*g_unit,
                            resetObject ? 0.0f : line->GetParam("delay")->AsFloat(0.0f));

                if (line->GetParam("fadeIn")->AsBool(false))
                    m_camera->StartOver(Gfx::CAM_OVER_EFFECT_FADEIN_WHITE, Math::Vector(0.0f, 0.0f, 0.0f), 1.0f);
                continue;
            }

            //! Note: This feature may be changed in next releases,
            //! Places new viewpoint, which can be selected later in (currently only in Code Battle) UI.
            //! Usage: View eye=x; y; z lookat=x; y; z

            if (line->GetCommand() == "View")
            {
                if(m_viewpoints.size() == 10)
                {
                    GetLogger()->Warn("Reached limit of 10 viewpoints, next ones will be ommited.\n");
                    continue;
                }
                Viewpoint tmp;
                tmp.eye = line->GetParam("eye")->AsPoint()*g_unit;
                tmp.look = line->GetParam("lookat")->AsPoint()*g_unit;
                tmp.button = line->GetParam("button")->AsInt(13); // 13 is the camera button
                m_viewpoints.push_back(tmp);
                continue;
            }

            if (line->GetCommand() == "EndMissionTake" && !resetObject)
            {
                auto endTake = MakeUnique<CSceneEndCondition>();
                endTake->Read(line.get());
                if (endTake->immediat)
                    m_endTakeImmediat = true;
                m_endTake.push_back(std::move(endTake));

                if (!line->GetParam("pos")->IsDefined() || !line->GetParam("dist")->IsDefined())
                {
                    LoadingWarning("The defaults for pos= and dist= are going to change, specify them explicitly. See issue #759 (https://git.io/vVBzH)");
                }
                continue;
            }
            if (line->GetCommand() == "EndMissionTeams" && !resetObject)
            {
                m_endTakeTeamImmediateWin = line->GetParam("immediateWin")->AsBool(false); // false = finishing removes the team that finished, true = finishing for one team ends the whole game
                continue;
            }
            if (line->GetCommand() == "EndMissionDelay" && !resetObject)
            {
                m_endTakeWinDelay  = line->GetParam("win")->AsFloat(2.0f);
                m_endTakeLostDelay = line->GetParam("lost")->AsFloat(2.0f);
                continue;
            }
            if (line->GetCommand() == "EndMissionResearch" && !resetObject) // This is not used in any original Colobot levels, but we'll keep it for userlevel creators
            {
                m_endTakeResearch |= line->GetParam("type")->AsResearchFlag();
                continue;
            }
            if (line->GetCommand() == "EndMissionTimeout" && !resetObject)
            {
                m_endTakeTimeout = line->GetParam("time")->AsFloat();
                continue;
            }

            if (line->GetCommand() == "Scoreboard" && !resetObject)
            {
                if (line->GetParam("enable")->AsBool(false))
                {
                    // Create the scoreboard
                    m_scoreboard = MakeUnique<CScoreboard>();
                    m_scoreboard->SetSortType(line->GetParam("sort")->AsSortType(CScoreboard::SortType::SORT_ID));
                }
                continue;
            }

            if (line->GetCommand() == "ScoreboardKillRule" && !resetObject)
            {
                if (!m_scoreboard)
                    throw CLevelParserException("ScoreboardKillRule encountered but scoreboard is not enabled");
                auto rule = MakeUnique<CScoreboard::CScoreboardKillRule>();
                rule->Read(line.get());
                m_scoreboard->AddKillRule(std::move(rule));
                continue;
            }
            if (line->GetCommand() == "ScoreboardObjectRule" && !resetObject)
            {
                if (!m_scoreboard)
                    throw CLevelParserException("ScoreboardObjectRule encountered but scoreboard is not enabled");
                auto rule = MakeUnique<CScoreboard::CScoreboardObjectRule>();
                rule->Read(line.get());
                m_scoreboard->AddObjectRule(std::move(rule));
                continue;
            }
            if (line->GetCommand() == "ScoreboardEndTakeRule" && !resetObject)
            {
                if (!m_scoreboard)
                    throw CLevelParserException("ScoreboardEndTakeRule encountered but scoreboard is not enabled");
                auto rule = MakeUnique<CScoreboard::CScoreboardEndTakeRule>();
                rule->Read(line.get());
                m_scoreboard->AddEndTakeRule(std::move(rule));
                continue;
            }

            if (line->GetCommand() == "ObligatoryToken" && !resetObject)
            {
                std::string token = line->GetParam("text")->AsString();
                if (!line->GetParam("min")->IsDefined() && !line->GetParam("max")->IsDefined())
                    GetLogger()->Warn("ObligatoryToken without specifying min/max is provided only for backwards compatibility - instead, do this: ObligatoryToken text=\"%s\" min=1\n", token.c_str());
                if (m_obligatoryTokens.count(token))
                    throw CLevelParserException("Incorrect ObligatoryToken specification - you cannot define a token twice");

                m_obligatoryTokens[token].min = line->GetParam("min")->AsInt(line->GetParam("max")->IsDefined() ? -1 : 1); // BACKWARDS COMPATIBILITY: if neither min or max are defined, default to min=1
                m_obligatoryTokens[token].max = line->GetParam("max")->AsInt(-1);
                if (m_obligatoryTokens[token].min >= 0 && m_obligatoryTokens[token].max >= 0 && m_obligatoryTokens[token].min > m_obligatoryTokens[token].max)
                {
                    throw CLevelParserException("Incorrect ObligatoryToken specification - min cannot be greater than max");
                }
                continue;
            }

            if (line->GetCommand() == "ProhibitedToken" && !resetObject) // NOTE: Kept only for backwards compatibility
            {
                std::string token = line->GetParam("text")->AsString();
                GetLogger()->Warn("ProhibitedToken is only provided for backwards compatibility - instead, do this: ObligatoryToken text=\"%s\" max=0\n", token.c_str());
                if (m_obligatoryTokens.count(token))
                    throw CLevelParserException("Incorrect ObligatoryToken specification - you cannot define a token twice");

                m_obligatoryTokens[token].min = -1;
                m_obligatoryTokens[token].max = 0;
                continue;
            }

            if (line->GetCommand() == "EnableBuild" && !resetObject)
            {
                m_build |= line->GetParam("type")->AsBuildFlag();
                continue;
            }

            if (line->GetCommand() == "EnableResearch" && !resetObject)
            {
                m_researchEnable |= line->GetParam("type")->AsResearchFlag();
                continue;
            }

            if (line->GetCommand() == "DoneResearch" && m_sceneReadPath.empty() && !resetObject) // not loading file?
            {
                m_researchDone[0] |= line->GetParam("type")->AsResearchFlag();
                continue;
            }

            if (line->GetCommand() == "NewScript" && !resetObject)
            {
                m_newScriptName.push_back(NewScriptName(line->GetParam("type")->AsObjectType(OBJECT_NULL), line->GetParam("name")->AsString("")));
                continue;
            }

            if (!m_sceneReadPath.empty()) continue; // ignore errors when loading saved game (TODO: don't report ones that are just not loaded when loading saved game)
            if (resetObject) continue; // ignore when reseting just objects (TODO: see above)

            throw CLevelParserException("Unknown command: '" + line->GetCommand() + "' in " + line->GetLevelFilename() + ":" + boost::lexical_cast<std::string>(line->GetLineNumber()));
        }

        // Do this here to prevent the first frame from taking a long time to render
        m_engine->UpdateGroundSpotTextures();

        m_ui->GetLoadingScreen()->SetProgress(1.0f, RT_LOADING_FINISHED);
        if (m_ui->GetLoadingScreen()->IsVisible())
        {
            // Force render of the "Loading finished" screen because it looks weird when the progress bar disappears in the middle
            m_app->Render();
        }

        if (!resetObject)
        {
            m_engine->SetBackground(backgroundPath,
                                    backgroundUp,
                                    backgroundDown,
                                    backgroundCloudUp,
                                    backgroundCloudDown,
                                    backgroundFull);
        }

        if (m_levelCategory == LevelCategory::Missions && !resetObject)  // mission?
        {
            m_playerProfile->SetFreeGameResearchUnlock(m_playerProfile->GetFreeGameResearchUnlock() | m_researchDone[0]);
            m_playerProfile->SetFreeGameBuildUnlock(m_playerProfile->GetFreeGameBuildUnlock() | m_build);
        }

        if (m_levelCategory == LevelCategory::FreeGame && !resetObject)  // free play?
        {
            m_researchDone[0] = m_playerProfile->GetFreeGameResearchUnlock();

            m_build = m_playerProfile->GetFreeGameBuildUnlock();
            m_build &= ~BUILD_RESEARCH;
            m_build &= ~BUILD_LABO;
            m_build |= BUILD_FACTORY;
            m_build |= BUILD_GFLAT;
            m_build |= BUILD_FLAG;
        }

        if (m_levelCategory == LevelCategory::GamePlus && !m_ui->GetPlusResearch() && !resetObject)  // new game plus?
        {
            m_researchDone[0] |= m_playerProfile->GetFreeGameResearchUnlock();
            m_build |= m_playerProfile->GetFreeGameBuildUnlock();
        }

        if (~m_researchDone[0] & RESEARCH_PHAZER && (m_levelCategory == LevelCategory::FreeGame || m_levelCategory == LevelCategory::GamePlus) && m_playerProfile->GetLevelPassed(LevelCategory::Missions, 9, 0))
        {
            m_build |= BUILD_RESEARCH;
            m_researchEnable |= RESEARCH_PHAZER;
        }

        m_researchEnable |= m_researchDone[0];

        if (!resetObject)
        {
            m_short->SetMode(false);  // vehicles?
        }

        m_map->ShowMap(m_mapShow);
        m_map->UpdateMap();
        // TODO: m_engine->TimeInit(); ??
        m_input->ResetKeyStates();
        m_time = 0.0f;
        if (m_sceneReadPath.empty()) m_gameTime = 0.0f;
        m_gameTimeAbsolute = 0.0f;
        m_autosaveLast = 0.0f;
        m_infoUsed = 0;

        m_selectObject = sel;

        if (m_base == nullptr &&  // no main base?
            !m_fixScene)    // interractive scene?
        {
            CObject* obj = sel;
            if (sel == nullptr)
                obj = SearchHuman();

            if (obj != nullptr)
            {
                assert(obj->Implements(ObjectInterfaceType::Controllable));
                SelectObject(obj);
                m_camera->SetControllingObject(obj);
                m_camera->SetType(dynamic_cast<CControllableObject&>(*obj).GetCameraType());
            }
        }

        if (m_fixScene)
            m_camera->SetType(Gfx::CAM_TYPE_SCRIPT);

        if (!m_sceneReadPath.empty() && sel != nullptr)  // loading file?
        {
            Math::Vector pos = sel->GetPosition();
            m_camera->Init(pos, pos, 0.0f);

            SelectObject(sel);
            m_camera->SetControllingObject(sel);

            m_beginSatCom = true;  // message already displayed
        }
    }
    catch (...)
    {
        m_sceneReadPath = "";
        throw;
    }
    m_sceneReadPath = "";

    if (m_app->GetSceneTestMode())
        m_eventQueue->AddEvent(Event(EVENT_QUIT));

    m_ui->ShowLoadingScreen(false);
    if (m_missionType == MISSION_CODE_BATTLE)
    {
        CreateCodeBattleInterface();
    }
    CreateShortcuts();
}

void CRobotMain::LevelLoadingError(const std::string& error, const std::runtime_error& exception, Phase exitPhase)
{
    m_ui->ShowLoadingScreen(false);

    GetLogger()->Error("%s\n", error.c_str());
    GetLogger()->Error("%s\n", exception.what());
    ChangePhase(exitPhase);
    m_ui->GetDialog()->StartInformation("Loading error", error, exception.what(), true, false);
}

//! Creates a directional light
int CRobotMain::CreateLight(Math::Vector direction, Gfx::Color color)
{
    if (direction.x == 0.0f &&
        direction.y == 0.0f &&
        direction.z == 0.0f)
    {
        direction.y = -1.0f;
    }

    Gfx::Light light;
    light.type = Gfx::LIGHT_DIRECTIONAL;
    light.diffuse = color;
    light.ambient = color * 0.1f;
    light.direction  = direction;
    int obj = m_lightMan->CreateLight(Gfx::LIGHT_PRI_HIGH);
    m_lightMan->SetLight(obj, light);

    return obj;
}

//! Creates a light spot
int CRobotMain::CreateSpot(Math::Vector pos, Gfx::Color color)
{
    if (!m_engine->GetLightMode()) return -1;

    pos.y += m_terrain->GetFloorLevel(pos);

    Gfx::Light light;
    light.type          = Gfx::LIGHT_SPOT;
    light.diffuse       = color;
    light.ambient       = color * 0.1f;
    light.position      = pos;
    light.direction     = Math::Vector(0.0f, -1.0f, 0.0f);
    light.spotIntensity = 1.0f;
    light.spotAngle     = 90.0f*Math::PI/180.0f;
    light.attenuation0  = 2.0f;
    light.attenuation1  = 0.0f;
    light.attenuation2  = 0.0f;
    int obj = m_lightMan->CreateLight(Gfx::LIGHT_PRI_HIGH);
    m_lightMan->SetLight(obj, light);

    return obj;
}


//! Change the colors and textures
void CRobotMain::ChangeColor()
{
    if (m_phase != PHASE_SIMUL    &&
        m_phase != PHASE_SETUPds  &&
        m_phase != PHASE_SETUPgs  &&
        m_phase != PHASE_SETUPps  &&
        m_phase != PHASE_SETUPcs  &&
        m_phase != PHASE_SETUPss  &&
        m_phase != PHASE_MOD_LIST &&
        m_phase != PHASE_WIN      &&
        m_phase != PHASE_LOST     &&
        m_phase != PHASE_APPEARANCE ) return;

    // Player texture

    Math::Point ts = Math::Point(0.0f, 0.0f);
    Math::Point ti = Math::Point(1.0f, 1.0f);  // the entire image

    Gfx::Color colorRef1, colorNew1, colorRef2, colorNew2;

    colorRef1.a = 0.0f;
    colorRef2.a = 0.0f;

    colorRef1.r = 206.0f/256.0f;
    colorRef1.g = 206.0f/256.0f;
    colorRef1.b = 204.0f/256.0f;  // ~white
    colorNew1 = m_playerProfile->GetAppearance().colorCombi;
    colorRef2.r = 255.0f/256.0f;
    colorRef2.g = 132.0f/256.0f;
    colorRef2.b =   1.0f/256.0f;  // orange
    colorNew2 = m_playerProfile->GetAppearance().colorBand;

    Math::Point exclu[6];
    exclu[0] = Math::Point(192.0f/256.0f,   0.0f/256.0f);
    exclu[1] = Math::Point(256.0f/256.0f,  64.0f/256.0f);  // crystals + cylinders
    exclu[2] = Math::Point(208.0f/256.0f, 224.0f/256.0f);
    exclu[3] = Math::Point(256.0f/256.0f, 256.0f/256.0f);  // SatCom screen
    exclu[4] = Math::Point(0.0f, 0.0f);
    exclu[5] = Math::Point(0.0f, 0.0f);  // terminator
    m_engine->ChangeTextureColor("textures/objects/human.png", colorRef1, colorNew1, colorRef2, colorNew2, 0.30f, 0.01f, ts, ti, exclu);

    float tolerance;

    int face = GetGamerFace();
    if (face == 0)  // normal?
    {
        colorRef1.r =  90.0f/256.0f;
        colorRef1.g =  95.0f/256.0f;
        colorRef1.b =  85.0f/256.0f;  // black
        tolerance = 0.15f;
    }
    if (face == 1)  // bald?
    {
        colorRef1.r =  74.0f/256.0f;
        colorRef1.g =  58.0f/256.0f;
        colorRef1.b =  46.0f/256.0f;  // brown
        tolerance = 0.20f;
    }
    if (face == 2)  // carlos?
    {
        colorRef1.r =  70.0f/256.0f;
        colorRef1.g =  40.0f/256.0f;
        colorRef1.b =   8.0f/256.0f;  // brown
        tolerance = 0.30f;
    }
    if (face == 3)  // blonde?
    {
        colorRef1.r =  74.0f/256.0f;
        colorRef1.g =  16.0f/256.0f;
        colorRef1.b =   0.0f/256.0f;  // yellow
        tolerance = 0.20f;
    }
    colorNew1 = m_playerProfile->GetAppearance().colorHair;
    colorRef2.r = 0.0f;
    colorRef2.g = 0.0f;
    colorRef2.b = 0.0f;
    colorNew2.r = 0.0f;
    colorNew2.g = 0.0f;
    colorNew2.b = 0.0f;

    char name[100];
    sprintf(name, "textures/objects/face%.2d.png", face+1);
    exclu[0] = Math::Point(105.0f/256.0f, 47.0f/166.0f);
    exclu[1] = Math::Point(153.0f/256.0f, 79.0f/166.0f);  // blue canister
    exclu[2] = Math::Point(0.0f, 0.0f);
    exclu[3] = Math::Point(0.0f, 0.0f);  // terminator
    m_engine->ChangeTextureColor(name, colorRef1, colorNew1, colorRef2, colorNew2, tolerance, 0.00f, ts, ti, exclu);

    colorRef2.r = 0.0f;
    colorRef2.g = 0.0f;
    colorRef2.b = 0.0f;
    colorNew2.r = 0.0f;
    colorNew2.g = 0.0f;
    colorNew2.b = 0.0f;

    // VehicleColor

    for (auto it : m_colorNewBot)
    {
        int team = it.first;
        Gfx::Color newColor = it.second;
        std::string teamStr = StrUtils::ToString<int>(team);
        if(team == 0) teamStr = "";

        m_engine->ChangeTextureColor("textures/objects/base1.png"+teamStr,   "textures/objects/base1.png",   COLOR_REF_BOT, newColor, colorRef2, colorNew2, 0.10f, -1.0f, ts, ti, nullptr, 0, true);
        m_engine->ChangeTextureColor("textures/objects/convert.png"+teamStr, "textures/objects/convert.png", COLOR_REF_BOT, newColor, colorRef2, colorNew2, 0.10f, -1.0f, ts, ti, nullptr, 0, true);
        m_engine->ChangeTextureColor("textures/objects/derrick.png"+teamStr, "textures/objects/derrick.png", COLOR_REF_BOT, newColor, colorRef2, colorNew2, 0.10f, -1.0f, ts, ti, nullptr, 0, true);
        m_engine->ChangeTextureColor("textures/objects/factory.png"+teamStr, "textures/objects/factory.png", COLOR_REF_BOT, newColor, colorRef2, colorNew2, 0.10f, -1.0f, ts, ti, nullptr, 0, true);
        m_engine->ChangeTextureColor("textures/objects/lemt.png"+teamStr,    "textures/objects/lemt.png",    COLOR_REF_BOT, newColor, colorRef2, colorNew2, 0.10f, -1.0f, ts, ti, nullptr, 0, true);
        m_engine->ChangeTextureColor("textures/objects/roller.png"+teamStr,  "textures/objects/roller.png",  COLOR_REF_BOT, newColor, colorRef2, colorNew2, 0.10f, -1.0f, ts, ti, nullptr, 0, true);
        m_engine->ChangeTextureColor("textures/objects/search.png"+teamStr,  "textures/objects/search.png",  COLOR_REF_BOT, newColor, colorRef2, colorNew2, 0.10f, -1.0f, ts, ti, nullptr, 0, true);
        m_engine->ChangeTextureColor("textures/objects/rollert.png"+teamStr, "textures/objects/rollert.png", COLOR_REF_BOT, newColor, colorRef2, colorNew2, 0.10f, -1.0f, ts, ti, nullptr, 0, true);

        exclu[0] = Math::Point(  0.0f/256.0f, 160.0f/256.0f);
        exclu[1] = Math::Point(256.0f/256.0f, 256.0f/256.0f);  // pencils
        exclu[2] = Math::Point(0.0f, 0.0f);
        exclu[3] = Math::Point(0.0f, 0.0f);  // terminator
        m_engine->ChangeTextureColor("textures/objects/drawer.png"+teamStr, "textures/objects/drawer.png",  COLOR_REF_BOT, newColor, colorRef2, colorNew2, 0.10f, -1.0f, ts, ti, exclu, 0, true);

        exclu[0] = Math::Point(237.0f/256.0f, 176.0f/256.0f);
        exclu[1] = Math::Point(256.0f/256.0f, 220.0f/256.0f);  // blue canister
        exclu[2] = Math::Point(106.0f/256.0f, 150.0f/256.0f);
        exclu[3] = Math::Point(130.0f/256.0f, 214.0f/256.0f);  // safe location
        exclu[4] = Math::Point(0.0f, 0.0f);
        exclu[5] = Math::Point(0.0f, 0.0f);  // terminator
        m_engine->ChangeTextureColor("textures/objects/subm.png"+teamStr,   "textures/objects/subm.png",    COLOR_REF_BOT, newColor, colorRef2, colorNew2, 0.10f, -1.0f, ts, ti, exclu, 0, true);
    }

    // AlienColor

    exclu[0] = Math::Point(128.0f/256.0f, 160.0f/256.0f);
    exclu[1] = Math::Point(256.0f/256.0f, 256.0f/256.0f);  // SatCom
    exclu[2] = Math::Point(0.0f, 0.0f);
    exclu[3] = Math::Point(0.0f, 0.0f);  // terminator
    m_engine->ChangeTextureColor("textures/objects/ant.png",     COLOR_REF_ALIEN, m_colorNewAlien, colorRef2, colorNew2, 0.50f, -1.0f, ts, ti, exclu);
    m_engine->ChangeTextureColor("textures/objects/mother.png",  COLOR_REF_ALIEN, m_colorNewAlien, colorRef2, colorNew2, 0.50f, -1.0f, ts, ti);

    // GreeneryColor
    m_engine->ChangeTextureColor("textures/objects/plant.png",   COLOR_REF_GREEN, m_colorNewGreen, colorRef2, colorNew2, 0.50f, -1.0f, ts, ti);

    // water color

    // PARTIPLOUF0 and PARTIDROP :
    ts = Math::Point(0.500f, 0.500f);
    ti = Math::Point(0.875f, 0.750f);
    m_engine->ChangeTextureColor("textures/effect00.png", COLOR_REF_WATER, m_colorNewWater, colorRef2, colorNew2, 0.20f, -1.0f, ts, ti, nullptr, m_colorShiftWater, true);

    // PARTIFLIC :
    ts = Math::Point(0.00f, 0.75f);
    ti = Math::Point(0.25f, 1.00f);
    m_engine->ChangeTextureColor("textures/effect02.png", COLOR_REF_WATER, m_colorNewWater, colorRef2, colorNew2, 0.20f, -1.0f, ts, ti, nullptr, m_colorShiftWater, true);
}

//! Calculates the distance to the nearest object
namespace
{
float SearchNearestObject(CObjectManager* objMan, Math::Vector center, CObject* exclu)
{
    float min = 100000.0f;
    for (CObject* obj : objMan->GetAllObjects())
    {
        if (!obj->GetDetectable()) continue;  // inactive?
        if (IsObjectBeingTransported(obj)) continue;

        if (obj == exclu) continue;

        ObjectType type = obj->GetType();

        if (type == OBJECT_BASE)
        {
            Math::Vector oPos = obj->GetPosition();
            if (oPos.x != center.x ||
                oPos.z != center.z)
            {
                float dist = Math::Distance(center, oPos) - 80.0f;
                if (dist < 0.0f) dist = 0.0f;
                min = Math::Min(min, dist);
                continue;
            }
        }

        if (type == OBJECT_STATION ||
            type == OBJECT_REPAIR ||
            type == OBJECT_DESTROYER)
        {
            Math::Vector oPos = obj->GetPosition();
            float dist = Math::Distance(center, oPos) - 8.0f;
            if (dist < 0.0f) dist = 0.0f;
            min = Math::Min(min, dist);
        }

        for (const auto &crashSphere : obj->GetAllCrashSpheres())
        {
            Math::Vector oPos = crashSphere.sphere.pos;
            float oRadius = crashSphere.sphere.radius;

            float dist = Math::Distance(center, oPos) - oRadius;
            if (dist < 0.0f) dist = 0.0f;
            min = Math::Min(min, dist);
        }
    }
    return min;
}

bool BlockedByObject(CObjectManager* objMan, Math::Vector center, float space, CObject* exclu)
{
    for (CObject* obj : objMan->GetAllObjects())
    {
        if (!obj->GetDetectable()) continue;  // inactive?
        if (IsObjectBeingTransported(obj)) continue;

        if (obj == exclu) continue;

        for (const auto &crashSphere : obj->GetAllCrashSpheres())
        {
            const Math::Vector oPos = crashSphere.sphere.pos;
            const float oRadius = crashSphere.sphere.radius;
            const float minDist = oRadius + space;

            if (Math::DistanceSquared(center, oPos) < minDist * minDist)
                return true;
        }
    }
    return false;
}
}

//! Calculates a free space
bool CRobotMain::FreeSpace(Math::Vector &center, float minRadius, float maxRadius,
                           float space, CObject *exclu)
{
    for (float radius = minRadius; radius <= maxRadius; radius += space)
    {
        float ia = space/radius;
        for (float angle = 0.0f; angle < Math::PI*2.0f; angle += ia)
        {
            Math::Point p;
            p.x = center.x+radius;
            p.y = center.z;
            p = Math::RotatePoint(Math::Point(center.x, center.z), angle, p);
            Math::Vector pos;
            pos.x = p.x;
            pos.z = p.y;
            pos.y = 0.0f;
            pos.y = m_terrain->GetFloorLevel(pos);
            if (!BlockedByObject(m_objMan.get(), pos, space, exclu))
            {
                float flat = m_terrain->GetFlatZoneRadius(pos, space);
                if (flat >= space)
                {
                    center = pos;
                    return true;
                }
            }
        }
    }

    float nan = nanf("");

    center = Math::Vector{ nan, nan, nan };

    return false;
}

//! Calculates a flat free space
bool CRobotMain::FlatFreeSpace(Math::Vector &center, float minFlat, float minRadius, float maxRadius,
                           float space, CObject *exclu)
{
    if (minRadius < maxRadius)  // from internal to external?
    {
        for (float radius = minRadius; radius <= maxRadius; radius += space)
        {
            float ia = space/radius;
            for (float angle = 0.0f; angle < Math::PI*2.0f; angle += ia)
            {
                Math::Point p;
                p.x = center.x+radius;
                p.y = center.z;
                p = Math::RotatePoint(Math::Point(center.x, center.z), angle, p);
                Math::Vector pos;
                pos.x = p.x;
                pos.z = p.y;
                pos.y = 0.0f;
                m_terrain->AdjustToFloor(pos, true);
                float dist = SearchNearestObject(m_objMan.get(), pos, exclu);
                if (dist >= space)
                {
                    float flat = m_terrain->GetFlatZoneRadius(pos, dist/2.0f);
                    if (flat >= dist/2.0f)
                    {
                        flat = m_terrain->GetFlatZoneRadius(pos, minFlat);
                        if(flat >= minFlat)
                        {
                            center = pos;
                            return true;
                        }
                    }
                }
            }
        }
    }
    else    // from external to internal?
    {
        for (float radius=maxRadius; radius >= minRadius; radius -= space)
        {
            float ia = space/radius;
            for (float angle=0.0f ; angle<Math::PI*2.0f ; angle+=ia )
            {
                Math::Point p;
                p.x = center.x+radius;
                p.y = center.z;
                p = Math::RotatePoint(Math::Point(center.x, center.z), angle, p);
                Math::Vector pos;
                pos.x = p.x;
                pos.z = p.y;
                pos.y = 0.0f;
                m_terrain->AdjustToFloor(pos, true);
                float dist = SearchNearestObject(m_objMan.get(), pos, exclu);
                if (dist >= space)
                {
                    float flat = m_terrain->GetFlatZoneRadius(pos, dist/2.0f);
                    if (flat >= dist/2.0f)
                    {
                        flat = m_terrain->GetFlatZoneRadius(pos, minFlat);
                        if(flat >= minFlat)
                        {
                            center = pos;
                            return true;
                        }
                    }
                }
            }
        }
    }
    return false;
}

//! Calculates the maximum radius of a free space
float CRobotMain::GetFlatZoneRadius(Math::Vector center, float maxRadius,
                                    CObject *exclu)
{
    float dist = SearchNearestObject(m_objMan.get(), center, exclu);
    if (dist == 0.0f) return 0.0f;
    if (dist < maxRadius)
        maxRadius = dist;

    return m_terrain->GetFlatZoneRadius(center, maxRadius);
}


//! Hides buildable area when a cube of metal is taken up
void CRobotMain::HideDropZone(CObject* metal)
{
    if (m_showLimit[1].used         &&
        m_showLimit[1].link == metal)
    {
        FlushShowLimit(1);
    }

    if (m_showLimit[2].used         &&
        m_showLimit[2].link == metal)
    {
        FlushShowLimit(2);
    }
}

//! Shows the buildable area when a cube of metal is deposited
void CRobotMain::ShowDropZone(CObject* metal, CObject* transporter)
{
    if (metal == nullptr) return;

    Math::Vector center = metal->GetPosition();

    // Calculates the maximum radius possible depending on other items.
    float oMax = 30.0f;  // radius to build the biggest building
    float tMax;
    for (CObject* obj : m_objMan->GetAllObjects())
    {
        if (!obj->GetDetectable()) continue;  // inactive?
        if (IsObjectBeingTransported(obj)) continue;

        if (obj == metal) continue;
        if (obj == transporter) continue;

        Math::Vector oPos;

        ObjectType type = obj->GetType();
        if (type == OBJECT_BASE)
        {
            oPos = obj->GetPosition();
            float dist = Math::Distance(center, oPos)-80.0f;
            oMax = Math::Min(oMax, dist);
        }
        else
        {
            for (const auto& crashSphere : obj->GetAllCrashSpheres())
            {
                float dist = Math::Distance(center, crashSphere.sphere.pos)-crashSphere.sphere.radius;
                oMax = Math::Min(oMax, dist);
            }
        }

        if ( type == OBJECT_DERRICK  ||
             type == OBJECT_FACTORY  ||
             type == OBJECT_STATION  ||
             type == OBJECT_CONVERT  ||
             type == OBJECT_REPAIR   ||
             type == OBJECT_DESTROYER||
             type == OBJECT_TOWER    ||
             type == OBJECT_RESEARCH ||
             type == OBJECT_RADAR    ||
             type == OBJECT_ENERGY   ||
             type == OBJECT_LABO     ||
             type == OBJECT_NUCLEAR  ||
             type == OBJECT_START    ||
             type == OBJECT_END      ||
             type == OBJECT_INFO     ||
             type == OBJECT_PARA     ||
             type == OBJECT_SAFE     ||
             type == OBJECT_HUSTON   )  // building?
        {
            for (const auto& crashSphere : obj->GetAllCrashSpheres())
            {
                float dist = Math::Distance(center, crashSphere.sphere.pos)-crashSphere.sphere.radius-BUILDMARGIN;
                oMax = Math::Min(oMax, dist);
            }
        }
    }

    // Calculates the maximum possible radius depending on terrain.
    if (oMax >= 2.0f)
        tMax = m_terrain->GetFlatZoneRadius(center, 30.0f);
    else
        tMax = 0.0f;

    float radius = Math::Min(oMax, tMax);
    if (radius >= 2.0f)
        SetShowLimit(1, Gfx::PARTILIMIT2, metal, center, radius, 10.0f);
}

//! Erases the boundaries shown
void CRobotMain::FlushShowLimit(int i)
{
    for (int j = 0; j < m_showLimit[i].total; j++)
    {
        if (m_showLimit[i].parti[j] == 0) continue;

        m_particle->DeleteParticle(m_showLimit[i].parti[j]);
        m_showLimit[i].parti[j] = 0;
    }

    m_showLimit[i].total = 0;
    m_showLimit[i].link = nullptr;
    m_showLimit[i].used = false;
}

//! Specifies the boundaries to show
void CRobotMain::SetShowLimit(int i, Gfx::ParticleType parti, CObject *obj,
                              Math::Vector pos, float radius, float duration)
{
    FlushShowLimit(i);  // erases the current boundaries

    if (radius <= 0.0f) return;

    Math::Point dim;
    float dist;
    if (radius <= 50.0f)
    {
        dim = Math::Point(0.3f, 0.3f);
        dist = 2.5f;
    }
    else
    {
        dim = Math::Point(1.5f, 1.5f);
        dist = 10.0f;
    }

    m_showLimit[i].used = true;
    m_showLimit[i].link = obj;
    m_showLimit[i].pos = pos;
    m_showLimit[i].radius = radius;
    m_showLimit[i].duration = duration;
    m_showLimit[i].total = static_cast<int>((radius*2.0f*Math::PI)/dist);
    if (m_showLimit[i].total > MAXSHOWPARTI) m_showLimit[i].total = MAXSHOWPARTI;
    m_showLimit[i].time = 0.0f;

    for (int j = 0; j < m_showLimit[i].total; j++)
    {
        m_showLimit[i].parti[j] = m_particle->CreateParticle(pos, Math::Vector(0.0f, 0.0f, 0.0f), dim, parti, duration);
    }
}

//! Mount the boundaries of the selected object
void CRobotMain::StartShowLimit()
{
    CObject* obj = GetSelect();
    if (obj == nullptr) return;
    if (!obj->Implements(ObjectInterfaceType::Ranged)) return;
    float range = dynamic_cast<CRangedObject&>(*obj).GetShowLimitRadius();
    if (range == 0.0f) return;
    SetShowLimit(0, Gfx::PARTILIMIT1, obj, obj->GetPosition(), range);
}

//! Advances the boundaries shown
void CRobotMain::FrameShowLimit(float rTime)
{
    if (m_engine->GetPause()) return;

    for (int i = 0; i < MAXSHOWLIMIT; i++)
    {
        if (!m_showLimit[i].used) continue;

        m_showLimit[i].time += rTime;

        if (m_showLimit[i].time >= m_showLimit[i].duration)
        {
            FlushShowLimit(i);
            continue;
        }

        float factor;
        if (m_showLimit[i].time < 1.0f)
            factor = m_showLimit[i].time;
        else if (m_showLimit[i].time > m_showLimit[i].duration-1.0f)
            factor = m_showLimit[i].duration-m_showLimit[i].time;
        else
            factor = 1.0f;

        float speed = 0.4f-m_showLimit[i].radius*0.001f;
        if (speed < 0.1f) speed = 0.1f;
        float angle = m_showLimit[i].time*speed;

        if (m_showLimit[i].link != nullptr)
        {
            m_showLimit[i].pos = m_showLimit[i].link->GetPosition();
        }

        for (int j = 0; j < m_showLimit[i].total; j++)
        {
            if (m_showLimit[i].parti[j] == 0) continue;

            Math::Point center;
            center.x = m_showLimit[i].pos.x;
            center.y = m_showLimit[i].pos.z;
            Math::Point rotate;
            rotate.x = center.x+m_showLimit[i].radius*factor;
            rotate.y = center.y;
            rotate = Math::RotatePoint(center, angle, rotate);

            Math::Vector pos;
            pos.x = rotate.x;
            pos.z = rotate.y;
            pos.y = 0.0f;
            m_terrain->AdjustToFloor(pos, true);
            if (m_showLimit[i].radius <= 50.0f) pos.y += 0.5f;
            else                                pos.y += 2.0f;
            m_particle->SetPosition(m_showLimit[i].parti[j], pos);

            angle += (2.0f*Math::PI)/m_showLimit[i].total;
        }
    }
}

//! Saves all programs of all the robots
void CRobotMain::SaveAllScript()
{
    for (CObject* obj : m_objMan->GetAllObjects())
    {
        SaveOneScript(obj);
    }
}

//! Saves all programs of the robot.
void CRobotMain::SaveOneScript(CObject *obj)
{
    if (! obj->Implements(ObjectInterfaceType::ProgramStorage)) return;

    CProgramStorageObject* programStorage = dynamic_cast<CProgramStorageObject*>(obj);

    char categoryChar = GetLevelCategoryDir(m_levelCategory)[0];
    programStorage->SaveAllUserPrograms(m_playerProfile->GetSaveFile(StrUtils::Format("%c%.3d%.3d", categoryChar, m_levelChap, m_levelRank)));
}

//! Saves the stack of the program in execution of a robot
bool CRobotMain::SaveFileStack(CObject *obj, std::ostream &ostr)
{
    if (! obj->Implements(ObjectInterfaceType::Programmable)) return true;

    CProgrammableObject* programmable = dynamic_cast<CProgrammableObject*>(obj);

    ObjectType type = obj->GetType();
    if (type == OBJECT_HUMAN) return true;

    long status = 1;
    std::stringstream sstr("");

    if (!programmable->WriteStack(sstr))
    {
        GetLogger()->Error("WriteStack failed at object id = %i\n", obj->GetID());
        status = 100; // marked bad
    }

    if (!CBot::WriteLong(ostr, status)) return false;
    if (!CBot::WriteStream(ostr, sstr)) return false;

    return true;
}

//! Resumes the execution stack of the program in a robot
bool CRobotMain::ReadFileStack(CObject *obj, std::istream &istr)
{
    if (! obj->Implements(ObjectInterfaceType::Programmable)) return true;

    CProgrammableObject* programmable = dynamic_cast<CProgrammableObject*>(obj);

    ObjectType type = obj->GetType();
    if (type == OBJECT_HUMAN) return true;

    long status;
    if (!CBot::ReadLong(istr, status)) return false;

    if (status == 100) // was marked bad ?
    {
        if (!CBot::ReadLong(istr, status)) return false;
        if (!istr.seekg(status, istr.cur)) return false;
        return true; // next program
    }

    if (status == 1)
    {
        std::stringstream sstr("");
        if (!CBot::ReadStream(istr, sstr)) return false;

        if (!programmable->ReadStack(sstr))
        {
            GetLogger()->Error("ReadStack failed at object id = %i\n", obj->GetID());
        }
        return true; // next program
    }

    return false; // error: status == ??
}

std::vector<std::string> CRobotMain::GetNewScriptNames(ObjectType type)
{
    std::vector<std::string> names;
    for (const auto& newScript : m_newScriptName)
    {
        if (newScript.type == type        ||
            newScript.type == OBJECT_NULL  )
        {
            names.push_back(newScript.name);
        }
    }

    return names;
}


//! Seeks if an object occupies in a spot, to prevent a backup of the game
bool CRobotMain::IOIsBusy()
{
    if (CScriptFunctions::CheckOpenFiles()) return true;

    for (CObject* obj : m_objMan->GetAllObjects())
    {
        if (! obj->Implements(ObjectInterfaceType::TaskExecutor)) continue;

        if (obj->Implements(ObjectInterfaceType::Programmable) && dynamic_cast<CProgrammableObject&>(*obj).IsProgram()) continue; // TODO: I'm not sure if this is correct but this is how it worked earlier
        if (dynamic_cast<CTaskExecutorObject&>(*obj).IsForegroundTask()) return true;
    }
    return false;
}

//! Writes an object into the backup file
void CRobotMain::IOWriteObject(CLevelParserLine* line, CObject* obj, const std::string& programDir, int objRank)
{
    line->AddParam("type", MakeUnique<CLevelParserParam>(obj->GetType()));
    line->AddParam("id", MakeUnique<CLevelParserParam>(obj->GetID()));
    line->AddParam("pos", MakeUnique<CLevelParserParam>(obj->GetPosition()/g_unit));
    line->AddParam("angle", MakeUnique<CLevelParserParam>(obj->GetRotation() * Math::RAD_TO_DEG));
    line->AddParam("zoom", MakeUnique<CLevelParserParam>(obj->GetScale()));

    if (obj->Implements(ObjectInterfaceType::Old))
    {
        line->AddParam("option", MakeUnique<CLevelParserParam>(obj->GetOption()));
    }

    if (obj->Implements(ObjectInterfaceType::Controllable))
    {
        auto controllableObj = dynamic_cast<CControllableObject*>(obj);
        line->AddParam("trainer", MakeUnique<CLevelParserParam>(controllableObj->GetTrainer()));
        if (controllableObj->GetSelect())
            line->AddParam("select", MakeUnique<CLevelParserParam>(true));
    }

    obj->Write(line);

    if (obj->GetType() == OBJECT_BASE)
        line->AddParam("run", MakeUnique<CLevelParserParam>(3));  // stops and open (PARAM_FIXSCENE)


    if (obj->Implements(ObjectInterfaceType::ProgramStorage))
    {
        CProgramStorageObject* programStorage = dynamic_cast<CProgramStorageObject*>(obj);
        if (programStorage->GetProgramStorageIndex() >= 0)
        {
            programStorage->SaveAllProgramsForSavedScene(line, programDir);
        }
        else
        {
            // Probably an object created after the scene started, not loaded from level file
            // This means it doesn't normally store programs so it doesn't have program storage id assigned
            programStorage->SetProgramStorageIndex(999-objRank); // Set something that won't collide with normal programs
            programStorage->SaveAllProgramsForSavedScene(line, programDir);
            programStorage->SetProgramStorageIndex(-1); // Disable again
        }

        if (obj->Implements(ObjectInterfaceType::Programmable))
        {
            int run = dynamic_cast<CProgramStorageObject&>(*obj).GetProgramIndex(dynamic_cast<CProgrammableObject&>(*obj).GetCurrentProgram());
            if (run != -1)
            {
                line->AddParam("run", MakeUnique<CLevelParserParam>(run+1));
            }
        }
    }
}

//! Saves the current game
bool CRobotMain::IOWriteScene(std::string filename, std::string filecbot, std::string filescreenshot, const std::string& info, bool emergencySave)
{
    if (!emergencySave)
    {
        // Render the indicator to show that we are working
        ShowSaveIndicator(true);
        m_app->Render(); // update
    }

    std::string dirname = filename.substr(0, filename.find_last_of("/"));

    CLevelParser levelParser(filename);
    CLevelParserLineUPtr line;

    line = MakeUnique<CLevelParserLine>("Title");
    line->AddParam("text", MakeUnique<CLevelParserParam>(std::string(info)));
    levelParser.AddLine(std::move(line));


    //TODO: Do we need that? It's not used anyway
    line = MakeUnique<CLevelParserLine>("Version");
    line->AddParam("maj", MakeUnique<CLevelParserParam>(0));
    line->AddParam("min", MakeUnique<CLevelParserParam>(1));
    levelParser.AddLine(std::move(line));


    line = MakeUnique<CLevelParserLine>("Created");
    line->AddParam("date", MakeUnique<CLevelParserParam>(static_cast<int>(time(nullptr))));
    levelParser.AddLine(std::move(line));

    line = MakeUnique<CLevelParserLine>("Mission");
    line->AddParam("base", MakeUnique<CLevelParserParam>(GetLevelCategoryDir(m_levelCategory)));
    if (m_levelCategory == LevelCategory::CustomLevels)
        line->AddParam("dir", MakeUnique<CLevelParserParam>(GetCustomLevelDir()));
    else
        line->AddParam("chap", MakeUnique<CLevelParserParam>(m_levelChap));
    line->AddParam("rank", MakeUnique<CLevelParserParam>(m_levelRank));
    line->AddParam("gametime", MakeUnique<CLevelParserParam>(GetGameTime()));
    levelParser.AddLine(std::move(line));

    line = MakeUnique<CLevelParserLine>("Map");
    line->AddParam("zoom", MakeUnique<CLevelParserParam>(m_map->GetZoomMap()));
    levelParser.AddLine(std::move(line));

    line = MakeUnique<CLevelParserLine>("DoneResearch");
    line->AddParam("bits", MakeUnique<CLevelParserParam>(static_cast<int>(m_researchDone[0])));
    levelParser.AddLine(std::move(line));

    float sleep, delay, magnetic, progress;
    if (m_lightning->GetStatus(sleep, delay, magnetic, progress))
    {
        line = MakeUnique<CLevelParserLine>("BlitzMode");
        line->AddParam("sleep", MakeUnique<CLevelParserParam>(sleep));
        line->AddParam("delay", MakeUnique<CLevelParserParam>(delay));
        line->AddParam("magnetic", MakeUnique<CLevelParserParam>(magnetic/g_unit));
        line->AddParam("progress", MakeUnique<CLevelParserParam>(progress));
        levelParser.AddLine(std::move(line));
    }


    int objRank = 0;
    for (CObject* obj : m_objMan->GetAllObjects())
    {
        if (obj->GetType() == OBJECT_TOTO) continue;
        if (IsObjectBeingTransported(obj)) continue;
        if (obj->Implements(ObjectInterfaceType::Destroyable) && dynamic_cast<CDestroyableObject&>(*obj).IsDying()) continue;

        if (obj->Implements(ObjectInterfaceType::Slotted))
        {
            CSlottedObject* slotted = dynamic_cast<CSlottedObject*>(obj);
            for (int slot = slotted->GetNumSlots() - 1; slot >= 0; slot--)
            {
                if (CObject *sub = slotted->GetSlotContainedObject(slot))
                {
                    if (slot == slotted->MapPseudoSlot(CSlottedObject::Pseudoslot::POWER))
                        line = MakeUnique<CLevelParserLine>("CreatePower");
                    else if (slot == slotted->MapPseudoSlot(CSlottedObject::Pseudoslot::CARRYING))
                        line = MakeUnique<CLevelParserLine>("CreateFret");
                    else
                        line = MakeUnique<CLevelParserLine>("CreateSlotObject");
                    line->AddParam("slotNum", MakeUnique<CLevelParserParam>(slot));
                    IOWriteObject(line.get(), sub, dirname, objRank++);
                    levelParser.AddLine(std::move(line));
                }
            }
        }

        line = MakeUnique<CLevelParserLine>("CreateObject");
        IOWriteObject(line.get(), obj, dirname, objRank++);
        levelParser.AddLine(std::move(line));
    }
    try
    {
        levelParser.Save();
    }
    catch (CLevelParserException& e)
    {
        GetLogger()->Error("Failed to save level state - %s\n", e.what()); // TODO add visual error to notify user that save failed
        return false;
    }

    // Writes the file of stacks of execution.
    COutputStream ostr(filecbot);
    if (!ostr.is_open()) return false;

    bool bError = false;
    long version = 1;
    CBot::WriteLong(ostr, version);                 // version of COLOBOT
    version = CBot::CBotProgram::GetVersion();
    CBot::WriteLong(ostr, version);                 // version of CBOT
    CBot::WriteWord(ostr, 0); // TODO

    for (CObject* obj : m_objMan->GetAllObjects())
    {
        if (obj->GetType() == OBJECT_TOTO) continue;
        if (IsObjectBeingTransported(obj)) continue;
        if (obj->Implements(ObjectInterfaceType::Destroyable) && dynamic_cast<CDestroyableObject&>(*obj).IsDying()) continue;

        if (!SaveFileStack(obj, ostr))
        {
            GetLogger()->Error("SaveFileStack failed at object id = %i\n", obj->GetID());
            bError = true;
            break;
        }
    }

    if (!bError && !CBot::CBotClass::SaveStaticState(ostr))
    {
        GetLogger()->Error("CBotClass save static state failed\n");
    }

    ostr.close();

    if (!emergencySave)
    {
        ShowSaveIndicator(false); // force hide for screenshot
        MouseMode oldMouseMode = m_app->GetMouseMode();
        m_app->SetMouseMode(MOUSE_NONE); // disable the mouse
        m_displayText->HideText(true); // hide
        m_engine->SetScreenshotMode(true);

        m_engine->Render(); // update (but don't show, we're not swapping buffers here!)
        m_engine->WriteScreenShot(filescreenshot);
        m_shotSaving++;

        m_engine->SetScreenshotMode(false);
        m_displayText->HideText(false);
        m_app->SetMouseMode(oldMouseMode);

        m_app->ResetTimeAfterLoading();
    }
    return true;
}

//! Notifies the user that scene write is finished
void CRobotMain::IOWriteSceneFinished()
{
    m_displayText->DisplayError(INFO_WRITEOK, Math::Vector(0.0f,0.0f,0.0f));
    m_shotSaving--;
}

//! Resumes the game
CObject* CRobotMain::IOReadObject(CLevelParserLine *line, const std::string& programDir, const std::string& objCounterText, float objectProgress, int objRank)
{
    ObjectCreateParams params = CObject::ReadCreateParams(line);
    params.power = -1.0f;
    params.id = line->GetParam("id")->AsInt();

    std::string details = objCounterText;
    #if DEV_BUILD
    // Object categories may spoil the level a bit, so hide them in release builds
    details += ": "+CLevelParserParam::FromObjectType(params.type);
    #endif
    m_ui->GetLoadingScreen()->SetProgress(0.25f+objectProgress*0.7f, RT_LOADING_OBJECTS_SAVED, details);

    CObject* obj = m_objMan->CreateObject(params);

    if (obj->Implements(ObjectInterfaceType::Old))
    {
        COldObject* oldObj = dynamic_cast<COldObject*>(obj);
        oldObj->SetPosition(line->GetParam("pos")->AsPoint() * g_unit);
        oldObj->SetRotation(line->GetParam("angle")->AsPoint() * Math::DEG_TO_RAD);
    }

    if (obj->GetType() == OBJECT_BASE) m_base = obj;

    obj->Read(line);

    int run = line->GetParam("run")->AsInt(-1);
    if (run != -1)
    {
        CAuto* automat = obj->GetAuto();
        if (automat != nullptr)
            automat->Start(run);  // starts the film
    }

    if (obj->Implements(ObjectInterfaceType::ProgramStorage))
    {
        CProgramStorageObject* programStorage = dynamic_cast<CProgramStorageObject*>(obj);
        if (!line->GetParam("programStorageIndex")->IsDefined()) // Backwards compatibility
            programStorage->SetProgramStorageIndex(objRank);
        programStorage->LoadAllProgramsForSavedScene(line, programDir);
    }

    return obj;
}

//! Resumes some part of the game
CObject* CRobotMain::IOReadScene(std::string filename, std::string filecbot)
{
    std::string dirname = filename.substr(0, filename.find_last_of("/"));

    CLevelParser levelParser(filename);
    levelParser.SetLevelPaths(m_levelCategory, m_levelChap, m_levelRank);
    levelParser.Load();
    int numObjects = levelParser.CountLines("CreateObject") + levelParser.CountLines("CreatePower") + levelParser.CountLines("CreateFret") + levelParser.CountLines("CreateSlotObject");

    m_base = nullptr;

    CObject* cargo   = nullptr;
    CObject* power  = nullptr;
    CObject* sel    = nullptr;
    int objRank = 0;
    int objCounter = 0;
    std::map<int, CObject*> slots;
    for (auto& line : levelParser.GetLines())
    {
        if (line->GetCommand() == "Mission")
            m_gameTime = line->GetParam("gametime")->AsFloat(0.0f);

        if (line->GetCommand() == "Map")
            m_map->ZoomMap(line->GetParam("zoom")->AsFloat());

        if (line->GetCommand() == "DoneResearch")
            m_researchDone[0] = line->GetParam("bits")->AsInt();

        if (line->GetCommand() == "BlitzMode")
        {
            float sleep = line->GetParam("sleep")->AsFloat();
            float delay = line->GetParam("delay")->AsFloat();
            float magnetic = line->GetParam("magnetic")->AsFloat()*g_unit;
            float progress = line->GetParam("progress")->AsFloat();
            m_lightning->SetStatus(sleep, delay, magnetic, progress);
        }

        if (line->GetCommand() == "CreateFret")
        {
            cargo = IOReadObject(line.get(), dirname, StrUtils::ToString<int>(objCounter+1)+" / "+StrUtils::ToString<int>(numObjects), static_cast<float>(objCounter) / static_cast<float>(numObjects));
            objCounter++;
        }

        if (line->GetCommand() == "CreatePower")
        {
            power = IOReadObject(line.get(), dirname, StrUtils::ToString<int>(objCounter+1)+" / "+StrUtils::ToString<int>(numObjects), static_cast<float>(objCounter) / static_cast<float>(numObjects));
            objCounter++;
        }

        if (line->GetCommand() == "CreateSlotObject")
        {
            int slotNum = line->GetParam("slotNum")->AsInt();
            CObject *slotObject = IOReadObject(line.get(), dirname, StrUtils::ToString<int>(objCounter+1)+" / "+StrUtils::ToString<int>(numObjects), static_cast<float>(objCounter) / static_cast<float>(numObjects));
            objCounter++;

            assert(slots.find(slotNum) == slots.end());
            slots.emplace(slotNum, slotObject);
        }

        if (line->GetCommand() == "CreateObject")
        {
            CObject* obj = IOReadObject(line.get(), dirname, StrUtils::ToString<int>(objCounter+1)+" / "+StrUtils::ToString<int>(numObjects), static_cast<float>(objCounter) / static_cast<float>(numObjects), objRank++);

            if (line->GetParam("select")->AsBool(false))
                sel = obj;

            if (obj->Implements(ObjectInterfaceType::Slotted))
            {
                CSlottedObject* asSlotted = dynamic_cast<CSlottedObject*>(obj);
                if (cargo != nullptr)
                {
                    int slotNum = asSlotted->MapPseudoSlot(CSlottedObject::Pseudoslot::CARRYING);
                    assert(slotNum >= 0);
                    assert(slots.find(slotNum) == slots.end());
                    asSlotted->SetSlotContainedObject(slotNum, cargo);

                    // TODO: eww!
                    assert(obj->Implements(ObjectInterfaceType::Old));
                    auto task = MakeUnique<CTaskManip>(dynamic_cast<COldObject*>(obj));
                    task->Start(TMO_AUTO, TMA_GRAB);  // holds the object!
                }

                if (power != nullptr)
                {
                    int slotNum = asSlotted->MapPseudoSlot(CSlottedObject::Pseudoslot::POWER);
                    assert(slotNum >= 0);
                    assert(slots.find(slotNum) == slots.end());
                    asSlotted->SetSlotContainedObject(slotNum, power);
                    dynamic_cast<CTransportableObject&>(*power).SetTransporter(obj);
                }

                for (std::pair<const int, CObject*>& slot : slots)
                {
                    asSlotted->SetSlotContainedObject(slot.first, slot.second);
                }

                cargo = nullptr;
                power = nullptr;
                slots.clear();
            }
            else
            {
                // TODO: exception?
                assert(slots.empty());
                assert(power == nullptr);
                assert(cargo == nullptr);
            }

            objCounter++;
        }
    }

    // all slot objects assigned to parent objects
    assert(slots.empty());
    assert(power == nullptr);
    assert(cargo == nullptr);

    m_ui->GetLoadingScreen()->SetProgress(0.95f, RT_LOADING_CBOT_SAVE);

    // Reads the file of stacks of execution.
    CInputStream istr(filecbot);

    if (istr.is_open())
    {
        bool bError = false;
        long version = 0;
        CBot::ReadLong(istr, version);             // version of COLOBOT
        if (version == 1)
        {
            CBot::ReadLong(istr, version);         // version of CBOT
            if (version == CBot::CBotProgram::GetVersion())
            {
                unsigned short flag;
                CBot::ReadWord(istr, flag); // TODO
                bError = (flag != 0);

                if (!bError) for (CObject* obj : m_objMan->GetAllObjects())
                {
                    if (obj->GetType() == OBJECT_TOTO) continue;
                    if (IsObjectBeingTransported(obj)) continue;
                    if (obj->Implements(ObjectInterfaceType::Destroyable) && dynamic_cast<CDestroyableObject&>(*obj).IsDying()) continue;

                    if (!ReadFileStack(obj, istr))
                    {
                        GetLogger()->Error("ReadFileStack failed at object id = %i\n", obj->GetID());
                        bError = true;
                        break;
                    }
                }

                if (!bError && !CBot::CBotClass::RestoreStaticState(istr))
                {
                    GetLogger()->Error("CBotClass restore static state failed\n");
                    bError = true;
                }
            }
            else
                GetLogger()->Error("cbot.run file is wrong version: %i\n", version);
        }

        if (bError) GetLogger()->Error("Restoring CBOT state failed at stream position: %li\n", istr.tellg());
        istr.close();
    }

    m_ui->GetLoadingScreen()->SetProgress(1.0f, RT_LOADING_FINISHED);

    return sel;
}


//! Changes current player
void CRobotMain::SelectPlayer(std::string playerName)
{
    assert(!playerName.empty());

    m_playerProfile = MakeUnique<CPlayerProfile>(playerName);
    SetGlobalGamerName(playerName);
}

CPlayerProfile* CRobotMain::GetPlayerProfile()
{
    return m_playerProfile.get();
}


//! Resets all objects to their original position
void CRobotMain::ResetObject()
{
    // schedule reset during next frame
    m_resetCreate = true;
}

//! Resets all objects to their original position
void CRobotMain::ResetCreate()
{
    SaveAllScript();

    // Removes all bullets in progress.
    m_particle->DeleteParticle(Gfx::PARTIGUN1);
    m_particle->DeleteParticle(Gfx::PARTIGUN2);
    m_particle->DeleteParticle(Gfx::PARTIGUN3);
    m_particle->DeleteParticle(Gfx::PARTIGUN4);

    DeselectAll();  // removes the control buttons
    DeleteAllObjects();  // removes all the current 3D Scene

    m_particle->FlushParticle();
    m_terrain->FlushBuildingLevel();

    m_camera->SetType(Gfx::CAM_TYPE_NULL);

    try
    {
        CreateScene(m_ui->GetSceneSoluce(), false, true);

        for (CObject* obj : m_objMan->GetAllObjects())
        {
            if (obj->GetAnimateOnReset())
            {
                m_engine->GetPyroManager()->Create(Gfx::PT_RESET, obj);
            }
        }
    }
    catch (const std::runtime_error& e)
    {
        LevelLoadingError("An error occurred while trying to reset scene", e);
    }
}

//! Updates the audiotracks
void CRobotMain::UpdateAudio(bool frame)
{
    for(std::unique_ptr<CAudioChangeCondition>& audioChange : m_audioChange)
    {
        if (audioChange->changed) continue;

        if (audioChange->Check())
        {
            GetLogger()->Info("Changing music to \"%s\"\n", audioChange->music.c_str());
            m_sound->PlayMusic(audioChange->music, audioChange->repeat);
            audioChange->changed = true;
        }
    }
}

//! Set mission result from LevelController script
void CRobotMain::SetMissionResultFromScript(Error result, float delay)
{
    m_endTakeWinDelay = delay;
    m_endTakeLostDelay = delay;
    m_missionResult = result;
    m_missionResultFromScript = true;
}

Error CRobotMain::ProcessEndMissionTakeForGroup(std::vector<CSceneEndCondition*>& endTakes)
{
    Error finalResult = ERR_OK;
    bool hasWinningConditions = false;
    for (CSceneEndCondition* endTake : endTakes)
    {
        Error result = endTake->GetMissionResult();
        if (endTake->lost < 0)
            hasWinningConditions = true;

        if (result == ERR_OK && endTake->immediat)
        {
            hasWinningConditions = true;
            finalResult = result;
            break;
        }

        if (result != ERR_OK)
        {
            finalResult = result;
            break;
        }
    }
    if (finalResult == ERR_OK && !hasWinningConditions) finalResult = ERR_MISSION_NOTERM; // Never end mission without ending conditions
    return finalResult;
}

//! Process EndMissionTake commands, result is stored in m_missionResult
//! If return value is different than ERR_MISSION_NOTERM, assume the mission is finished and pass on the result
Error CRobotMain::ProcessEndMissionTake()
{
    bool timeout = false;
    if (m_missionResult != INFO_LOST && m_missionResult != INFO_LOSTq)
    {
        if (m_endTakeTimeout >= 0.0f)
        {
            // Use the mission timer if available, or global mission time otherwise
            // Useful for exercises where the time starts when you start the program, not the mission itself
            float currentTime = m_missionTimerEnabled ? m_missionTimer : m_gameTime;
            if (currentTime > m_endTakeTimeout)
            {
                m_missionResult = INFO_LOST;
                timeout = true;
            }
        }
    }

    // Sort end conditions by teams
    std::map<int, std::vector<CSceneEndCondition*>> teamsEndTake;
    for (std::unique_ptr<CSceneEndCondition>& endTake : m_endTake)
        teamsEndTake[endTake->winTeam].push_back(endTake.get());

    // This is just a smart way to check if we have any map values other than 0 defined
    bool usesTeamConditions = teamsEndTake.size() > teamsEndTake.count(0);

    if (!usesTeamConditions)
    {
        if (!timeout)
            m_missionResult = ProcessEndMissionTakeForGroup(teamsEndTake[0]);

        if (m_missionResult != INFO_LOST && m_missionResult != INFO_LOSTq)
        {
            if (m_endTakeResearch != 0)
            {
                if (m_endTakeResearch != (m_endTakeResearch&m_researchDone[0]))
                {
                    m_missionResult = ERR_MISSION_NOTERM;
                }
            }
        }
    }
    else
    {
        assert(m_endTakeResearch == 0); // TODO: Add support for per-team EndTakeResearch

        // Special handling for teams
        m_missionResult = ERR_MISSION_NOTERM;

        if (GetAllActiveTeams().empty() || timeout)
        {
            GetLogger()->Info("All teams died, mission ended\n");
            if (m_scoreboard)
            {
                std::string title, text, details_line;
                GetResource(RES_TEXT, RT_SCOREBOARD_RESULTS, title);
                if (m_missionTimerEnabled && m_missionTimerStarted)
                {
                    GetResource(RES_TEXT, RT_SCOREBOARD_RESULTS_TIME, text);
                    text = StrUtils::Format(text.c_str(), TimeFormat(m_missionTimer).c_str());
                }
                else
                {
                    GetResource(RES_TEXT, RT_SCOREBOARD_RESULTS_TEXT, text);
                }
                GetResource(RES_TEXT, RT_SCOREBOARD_RESULTS_LINE, details_line);
                std::string details = "";
                for (std::pair<int, CScoreboard::Score> team : m_scoreboard->GetSortedScores())
                {
                    if (!details.empty())
                        details += ", ";
                    details += StrUtils::Format(details_line.c_str(), GetTeamName(team.first).c_str(), team.second.points);
                }
                m_ui->GetDialog()->StartInformation(
                    title,
                    text,
                    details,
                    false, true,
                    [&]()
                    {
                        ChangePhase(PHASE_WIN);
                    }
                );
                m_endTakeWinDelay = 0.0f;
                m_missionResult = ERR_OK;
            }
            else
            {
                m_missionResult = INFO_LOST;
            }
        }
        else
        {
            for (auto it : teamsEndTake)
            {
                int team = it.first;
                if (team == 0) continue;
                if (m_teamFinished[team]) continue;

                Error result = ProcessEndMissionTakeForGroup(it.second);
                if (result == INFO_LOST || result == INFO_LOSTq)
                {
                    GetLogger()->Info("Team %d lost\n", team);
                    std::string text;
                    GetResource(RES_ERR, INFO_TEAM_DEAD, text);
                    text = StrUtils::Format(text.c_str(), GetTeamName(team).c_str());
                    m_displayText->DisplayText(text.c_str(), Math::Vector(0.0f,0.0f,0.0f), 15.0f, 60.0f, 10.0f, Ui::TT_ERROR);

                    m_displayText->SetEnable(false); // To prevent "bot destroyed" messages
                    m_objMan->DestroyTeam(team);
                    m_displayText->SetEnable(true);

                    m_teamFinished[team] = true;
                }
                else if (result == ERR_OK)
                {
                    /*if (m_winDelay == 0.0f)
                    {
                        GetLogger()->Info("Team %d won\n", team);

                        m_displayText->DisplayText(("<<< Team "+boost::lexical_cast<std::string>(team)+" won the game >>>").c_str(), Math::Vector(0.0f,0.0f,0.0f));
                        if (m_missionTimerEnabled && m_missionTimerStarted)
                        {
                            GetLogger()->Info("Mission time: %s\n", TimeFormat(m_missionTimer).c_str());
                            m_displayText->DisplayText(("Time: " + TimeFormat(m_missionTimer)).c_str(), Math::Vector(0.0f,0.0f,0.0f));
                        }
                        m_missionTimerEnabled = m_missionTimerStarted = false;
                        m_winDelay  = m_endTakeWinDelay;  // wins in two seconds
                        m_lostDelay = 0.0f;
                        m_displayText->SetEnable(false);
                    }
                    m_missionResult = ERR_OK;
                    return ERR_OK;*/
                    GetLogger()->Info("Team %d finished\n", team);
                    std::string text;
                    GetResource(RES_ERR, INFO_TEAM_FINISH, text);
                    text = StrUtils::Format(text.c_str(), GetTeamName(team).c_str());
                    m_displayText->DisplayText(text.c_str(), Math::Vector(0.0f,0.0f,0.0f));
                    if (m_scoreboard)
                        m_scoreboard->ProcessEndTake(team);
                    m_objMan->DestroyTeam(team, DestructionType::Win);
                    m_teamFinished[team] = true;
                    if (m_endTakeTeamImmediateWin)
                    {
                        // All other teams fail
                        for(int other_team : GetAllActiveTeams())
                        {
                            m_displayText->SetEnable(false); // To prevent "bot destroyed" messages
                            m_objMan->DestroyTeam(other_team);
                            m_displayText->SetEnable(true);

                            m_teamFinished[other_team] = true;
                        }
                    }
                }
            }
        }
    }

    return ERR_MISSION_NOTERM;
}

//! Checks if the mission is over
Error CRobotMain::CheckEndMission(bool frame)
{
    // Process EndMissionTake, unless we are using LevelController script for processing ending conditions
    if (!m_missionResultFromScript)
    {
        Error result = ProcessEndMissionTake();
        if (result != ERR_MISSION_NOTERM) return result;
    }
    // Take action depending on m_missionResult

    if (m_missionResult == INFO_LOSTq)
    {
        if (m_lostDelay == 0.0f)
        {
            m_lostDelay = 0.1f;  // lost immediately
            m_winDelay  = 0.0f;
        }
        m_missionTimerEnabled = m_missionTimerStarted = false;
        m_displayText->SetEnable(false);
        return INFO_LOSTq;
    }

    if (m_missionResult == INFO_LOST)
    {
        if (m_lostDelay == 0.0f)
        {
            m_displayText->DisplayError(INFO_LOST, Math::Vector(0.0f,0.0f,0.0f));
            m_lostDelay = m_endTakeLostDelay;  // lost in 6 seconds
            m_winDelay  = 0.0f;
        }
        m_missionTimerEnabled = m_missionTimerStarted = false;
        m_displayText->SetEnable(false);
        return INFO_LOST;
    }

    if (m_missionResult == ERR_OK)
    {
        if (m_endTakeWinDelay == -1.0f && m_winDelay == 0.0f)
        {
            m_winDelay  = 1.0f;  // wins in one second
            m_lostDelay = 0.0f;
            m_missionTimerEnabled = m_missionTimerStarted = false;
            m_displayText->SetEnable(false);
            return ERR_OK;  // mission ended
        }

        if (frame)
        {
            if (m_base != nullptr && !m_endTakeImmediat)
            {
                assert(m_base->Implements(ObjectInterfaceType::Controllable));
                if(dynamic_cast<CControllableObject&>(*m_base).GetSelectable())
                    return ERR_MISSION_NOTERM;
            }
        }

        if (m_winDelay == 0.0f)
        {
            m_displayText->DisplayError(INFO_WIN, Math::Vector(0.0f,0.0f,0.0f));
            if (m_missionTimerEnabled && m_missionTimerStarted)
            {
                GetLogger()->Info("Mission time: %s\n", TimeFormat(m_missionTimer).c_str());
                m_displayText->DisplayText(("Time: " + TimeFormat(m_missionTimer)).c_str(), Math::Vector(0.0f,0.0f,0.0f));
            }
            m_missionTimerEnabled = m_missionTimerStarted = false;
            m_winDelay  = m_endTakeWinDelay;  // wins in two seconds
            m_lostDelay = 0.0f;
        }
        m_displayText->SetEnable(false);
        return ERR_OK;  // mission ended
    }
    else
    {
        m_displayText->SetEnable(true);
        return ERR_MISSION_NOTERM;
    }
}


//! Returns the list instructions required in CBot program in level
const std::map<std::string, MinMax>& CRobotMain::GetObligatoryTokenList()
{
    return m_obligatoryTokens;
}

//! Indicates whether it is possible to control a driving robot
bool CRobotMain::GetTrainerPilot()
{
    return m_cheatTrainerPilot;
}

bool CRobotMain::GetPlusTrainer()
{
    return m_ui->GetPlusTrainer();
}

bool CRobotMain::GetPlusExplorer()
{
    return m_ui->GetPlusExplorer();
}

//! Indicates whether the scene is fixed, without interaction
bool CRobotMain::GetFixScene()
{
    return m_fixScene;
}


const std::string& CRobotMain::GetScriptName()
{
    return m_scriptName;
}

const std::string& CRobotMain::GetScriptFile()
{
    return m_scriptFile;
}


bool CRobotMain::GetShowSoluce()
{
    return m_cheatShowSoluce;
}

bool CRobotMain::GetSceneSoluce()
{
    if (m_infoFilename[SATCOM_SOLUCE][0] == 0) return false;
    return m_ui->GetSceneSoluce();
}

bool CRobotMain::GetShowAll()
{
    return m_cheatAllMission;
}

bool CRobotMain::GetRadar()
{
    if (m_cheatRadar)
        return true;

    for (CObject* obj : m_objMan->GetAllObjects())
    {
        ObjectType type = obj->GetType();
        if (type == OBJECT_RADAR && !obj->GetLock() && !obj->GetProxyActivate())
            return true;
    }
    return false;
}

MissionType CRobotMain::GetMissionType()
{
    return m_missionType;
}


//! Returns the representation to use for the player
int CRobotMain::GetGamerFace()
{
    return m_playerProfile->GetAppearance().face;
}

//! Returns the representation to use for the player
int CRobotMain::GetGamerGlasses()
{
    return m_playerProfile->GetAppearance().glasses;
}

//! Returns the mode with just the head
bool CRobotMain::GetGamerOnlyHead()
{
    return m_ui->GetGamerOnlyHead();
}

//! Returns the angle of presentation
float CRobotMain::GetPersoAngle()
{
    return m_ui->GetPersoAngle();
}

void CRobotMain::SetLevel(LevelCategory cat, int chap, int rank)
{
    GetLogger()->Debug("Change level to %s %d %d\n", GetLevelCategoryDir(cat).c_str(), chap, rank);
    m_levelCategory = cat;
    m_levelChap = chap;
    m_levelRank = rank;
    m_levelFile = CLevelParser::BuildScenePath(m_levelCategory, m_levelChap, m_levelRank);
}

LevelCategory CRobotMain::GetLevelCategory()
{
    return m_levelCategory;
}

int CRobotMain::GetLevelChap()
{
    return m_levelChap;
}

int CRobotMain::GetLevelRank()
{
    return m_levelRank;
}

//! Returns folder name of the scene that user selected to play.
std::string CRobotMain::GetCustomLevelDir()
{
    assert(m_levelCategory == LevelCategory::CustomLevels);
    return m_ui->GetCustomLevelName(m_levelChap);
}

void CRobotMain::SetReadScene(std::string path)
{
    m_sceneReadPath = path;
}

void CRobotMain::UpdateChapterPassed()
{
    return m_ui->UpdateChapterPassed();
}

//! Changes game speed
void CRobotMain::SetSpeed(float speed)
{
    speed = Math::Clamp(speed, MIN_SPEED, MAX_SPEED);

    m_app->SetSimulationSpeed(speed);
    UpdateSpeedLabel();
}

float CRobotMain::GetSpeed()
{
    return m_app->GetSimulationSpeed();
}

void CRobotMain::UpdateSpeedLabel()
{
    Ui::CButton* pb = static_cast<Ui::CButton*>(m_interface->SearchControl(EVENT_SPEED));
    float speed = m_app->GetSimulationSpeed();

    if (pb != nullptr)
    {
        if (speed == 1.0f)
        {
            pb->ClearState(Ui::STATE_VISIBLE);
        }
        else
        {
            char text[10];
            sprintf(text, "x%.1f", speed);
            pb->SetName(text);
            pb->SetState(Ui::STATE_VISIBLE);
        }
    }

}


bool CRobotMain::CreateShortcuts()
{
    if (m_phase != PHASE_SIMUL) return false;
    if (m_ui->GetLoadingScreen()->IsVisible()) return false;
    if (!m_shortCut) return false;
    return m_short->CreateShortcuts();
}

//! Updates the map
void CRobotMain::UpdateMap()
{
    m_map->UpdateMap();
}

//! Indicates whether the mini-map is visible
bool CRobotMain::GetShowMap()
{
    return m_mapShow;
}


//! Management of the lock mode for movies
void CRobotMain::SetMovieLock(bool lock)
{
    m_movieLock = lock;

    CreateShortcuts();
    m_map->ShowMap(!m_movieLock && m_mapShow && !m_ui->GetLoadingScreen()->IsVisible());
    if (m_movieLock) HiliteClear();
}

bool CRobotMain::GetMovieLock()
{
    return m_movieLock;
}

bool CRobotMain::GetInfoLock()
{
    return m_displayInfo != nullptr;  // info in progress?
}

//! Management of the blocking of the call of SatCom
void CRobotMain::SetSatComLock(bool lock)
{
    m_satComLock = lock;
}

bool CRobotMain::GetSatComLock()
{
    return m_satComLock;
}

//! Management of the lock mode for the edition
void CRobotMain::SetEditLock(bool lock, bool edit)
{
    m_editLock = lock;

    CreateShortcuts();

    // Do not remove the card if it contains a still image.
    if (!lock || !m_map->GetFixImage())
        m_map->ShowMap(!m_editLock && m_mapShow);

    m_displayText->HideText(lock);
    m_input->ResetKeyStates();

    if (m_editLock)
        HiliteClear();
    else
        m_editFull = false;
}

bool CRobotMain::GetEditLock()
{
    return m_editLock;
}

//! Management of the fullscreen mode during editing
void CRobotMain::SetEditFull(bool full)
{
    m_editFull = full;
}

bool CRobotMain::GetEditFull()
{
    return m_editFull;
}


//! Indicates whether mouse is on an friend object, on which we should not shoot
void CRobotMain::SetFriendAim(bool friendAim)
{
    m_friendAim = friendAim;
}

bool CRobotMain::GetFriendAim()
{
    return m_friendAim;
}


//! Starts music with a mission
void CRobotMain::StartMusic()
{
    GetLogger()->Debug("Starting music...\n");
    if (m_audioTrack != "")
    {
        m_sound->PlayMusic(m_audioTrack, m_audioRepeat, 0.0f);
    }
}

void CRobotMain::UpdatePause(PauseType pause)
{
    m_engine->SetPause(pause & PAUSE_ENGINE);
    m_camera->SetFreeze(pause & PAUSE_CAMERA);
    m_sound->MuteAll(pause & PAUSE_MUTE_SOUND);
    CreateShortcuts();
    if (pause != PAUSE_NONE) HiliteClear();
}

void CRobotMain::UpdatePauseMusic(PauseMusic music)
{
    switch (music)
    {
        case PAUSE_MUSIC_NONE:
            m_sound->StopPauseMusic();
            break;

        case PAUSE_MUSIC_EDITOR:
            if (m_editorTrack != "")
                m_sound->PlayPauseMusic(m_editorTrack, m_editorRepeat);
            break;

        case PAUSE_MUSIC_SATCOM:
            if (m_satcomTrack != "")
                m_sound->PlayPauseMusic(m_satcomTrack, m_satcomRepeat);
            break;
    }
}

//! Removes hilite and tooltip
void CRobotMain::ClearInterface()
{
    HiliteClear();  // removes setting evidence
    m_tooltipName.clear();  // really removes the tooltip
}

void CRobotMain::DisplayError(Error err, CObject* pObj, float time)
{
    m_displayText->DisplayError(err, pObj, time);
}

void CRobotMain::DisplayError(Error err, Math::Vector goal, float height, float dist, float time)
{
    m_displayText->DisplayError(err, goal, height, dist, time);
}

void CRobotMain::UpdateCustomLevelList()
{
    m_ui->UpdateCustomLevelList();
}

std::string CRobotMain::GetCustomLevelName(int id)
{
    return m_ui->GetCustomLevelName(id);
}

const std::vector<std::string>& CRobotMain::GetCustomLevelList()
{
    return m_ui->GetCustomLevelList();
}

bool CRobotMain::IsLoading()
{
    return m_ui->GetLoadingScreen()->IsVisible();
}

void CRobotMain::StartMissionTimer()
{
    if (m_missionTimerEnabled && !m_missionTimerStarted)
    {
        GetLogger()->Info("Starting mission timer...\n");
        m_missionTimerStarted = true;
    }
}

void CRobotMain::SetAutosave(bool enable)
{
    if (m_autosave == enable) return;

    m_autosave = enable;
    m_autosaveLast = m_gameTimeAbsolute;
}

bool CRobotMain::GetAutosave()
{
    return m_autosave;
}

void CRobotMain::SetAutosaveInterval(int interval)
{
    if (m_autosaveInterval == interval) return;

    m_autosaveInterval = interval;
    m_autosaveLast = m_gameTimeAbsolute;
}

int CRobotMain::GetAutosaveInterval()
{
    return m_autosaveInterval;
}

void CRobotMain::SetAutosaveSlots(int slots)
{
    if (m_autosaveSlots == slots) return;

    m_autosaveSlots = slots;
}

int CRobotMain::GetAutosaveSlots()
{
    return m_autosaveSlots;
}

// Remove oldest saves with autosave prefix
void CRobotMain::AutosaveRotate()
{
    if (m_playerProfile == nullptr)
        return;

    GetLogger()->Debug("Rotate autosaves...\n");
    auto saveDirs = CResourceManager::ListDirectories(m_playerProfile->GetSaveDir());
    const std::string autosavePrefix = "autosave";
    std::vector<std::string> autosaves;
    std::copy_if(saveDirs.begin(), saveDirs.end(), std::back_inserter(autosaves), [&](const std::string &save)
    {
        return save.substr(0, autosavePrefix.length()) == autosavePrefix;
    });

    std::sort(autosaves.begin(), autosaves.end(), std::less<std::string>());
    for (int i = 0; i < static_cast<int>(autosaves.size()) - m_autosaveSlots + 1; i++)
    {
        CResourceManager::RemoveExistingDirectory(m_playerProfile->GetSaveDir() + "/" + autosaves[i]);
    }
}

void CRobotMain::Autosave()
{
    AutosaveRotate();
    GetLogger()->Info("Autosave!\n");

    char timestr[100];
    char infostr[100];
    time_t now = time(nullptr);
    strftime(timestr, 99, "%y%m%d%H%M%S", localtime(&now));
    strftime(infostr, 99, "%y.%m.%d %H:%M", localtime(&now));
    std::string info = std::string("[AUTOSAVE] ") + infostr;
    std::string dir = m_playerProfile->GetSaveFile(std::string("autosave") + timestr);

    m_playerProfile->SaveScene(dir, info);
}

void CRobotMain::QuickSave()
{
    GetLogger()->Info("Quicksave!\n");

    char infostr[100];
    time_t now = time(nullptr);
    strftime(infostr, 99, "%y.%m.%d %H:%M", localtime(&now));
    std::string info = std::string("[QUICKSAVE]") + infostr;
    std::string dir = m_playerProfile->GetSaveFile(std::string("quicksave"));

    m_playerProfile->SaveScene(dir, info);
}

void CRobotMain::QuickLoad()
{
    std::string dir = m_playerProfile->GetSaveFile(std::string("quicksave"));
    if(!CResourceManager::Exists(dir))
    {
        m_displayText->DisplayError(ERR_NO_QUICK_SLOT, Math::Vector(0.0f,0.0f,0.0f), 15.0f, 60.0f, 1000.0f);
        GetLogger()->Debug("Quicksave slot not found\n");
        return;
    }
    m_playerProfile->LoadScene(dir);
}

void CRobotMain::LoadSaveFromDirName(const std::string& gameDir)
{
    std::string dir = m_playerProfile->GetSaveFile(gameDir);
    if(!CResourceManager::Exists(dir))
    {
        GetLogger()->Error("Save slot not found\n");
        return;
    }
    m_playerProfile->LoadScene(dir);
}

void CRobotMain::SetExitAfterMission(bool exit)
{
    m_exitAfterMission = exit;
}

bool CRobotMain::CanPlayerInteract()
{
    if(GetMissionType() == MISSION_CODE_BATTLE)
    {
        return !m_codeBattleStarted;
    }
    return true;
}

const std::string NO_TEAM_NAME = "Team";
const std::string& CRobotMain::GetTeamName(int id)
{
    if(m_teamNames.count(id) == 0) return NO_TEAM_NAME;
    return m_teamNames[id];
}

bool CRobotMain::IsTeamColorDefined(int id)
{
    if(id == 0) return false; // Always use default for team 0

    return m_colorNewBot.find(id) != m_colorNewBot.end();
}


int CRobotMain::GetEnableBuild()
{
    return m_build;
}

void CRobotMain::SetEnableBuild(int enableBuild)
{
    m_build = enableBuild;
}

int CRobotMain::GetEnableResearch()
{
    return m_researchEnable;
}

void CRobotMain::SetEnableResearch(int enableResearch)
{
    m_researchEnable = enableResearch;
}

int CRobotMain::GetDoneResearch(int team)
{
    return m_researchDone[team];
}

void CRobotMain::SetDoneResearch(int doneResearch, int team)
{
    m_researchDone[team] = doneResearch;
}


bool CRobotMain::IsBuildingEnabled(BuildType type)
{
    return (m_build & type) != 0;
}

bool CRobotMain::IsBuildingEnabled(ObjectType type)
{
    if(type == OBJECT_DERRICK) return IsBuildingEnabled(BUILD_DERRICK);
    if(type == OBJECT_FACTORY) return IsBuildingEnabled(BUILD_FACTORY);
    if(type == OBJECT_STATION) return IsBuildingEnabled(BUILD_STATION);
    if(type == OBJECT_CONVERT) return IsBuildingEnabled(BUILD_CONVERT);
    if(type == OBJECT_REPAIR) return IsBuildingEnabled(BUILD_REPAIR);
    if(type == OBJECT_TOWER) return IsBuildingEnabled(BUILD_TOWER);
    if(type == OBJECT_RESEARCH) return IsBuildingEnabled(BUILD_RESEARCH);
    if(type == OBJECT_RADAR) return IsBuildingEnabled(BUILD_RADAR);
    if(type == OBJECT_ENERGY) return IsBuildingEnabled(BUILD_ENERGY);
    if(type == OBJECT_LABO) return IsBuildingEnabled(BUILD_LABO);
    if(type == OBJECT_NUCLEAR) return IsBuildingEnabled(BUILD_NUCLEAR);
    if(type == OBJECT_INFO) return IsBuildingEnabled(BUILD_INFO);
    if(type == OBJECT_PARA) return IsBuildingEnabled(BUILD_PARA);
    if(type == OBJECT_SAFE) return IsBuildingEnabled(BUILD_SAFE);
    if(type == OBJECT_DESTROYER) return IsBuildingEnabled(BUILD_DESTROYER);

    return false;
}

bool CRobotMain::IsResearchEnabled(ResearchType type)
{
    return (m_researchEnable & type) != 0;
}

bool CRobotMain::IsResearchDone(ResearchType type, int team)
{
    if(team != 0 && m_researchDone.count(team) == 0)
    {
        // Initialize with defaults
        m_researchDone[team] = m_researchDone[0];
    }

    return (m_researchDone[team] & type) != 0;
}

void CRobotMain::MarkResearchDone(ResearchType type, int team)
{
    if(team != 0 && m_researchDone.count(team) == 0)
    {
        // Initialize with defaults
        m_researchDone[team] = m_researchDone[0];
    }

    m_researchDone[team] |= type;

    if(team == 0)
    {
        m_playerProfile->SetFreeGameResearchUnlock(m_playerProfile->GetFreeGameResearchUnlock() | m_researchDone[0]);
    }
}

Error CRobotMain::CanBuildError(ObjectType type, int team)
{
    if(!IsBuildingEnabled(type)) return ERR_BUILD_DISABLED;

    if(type == OBJECT_TOWER   && !IsResearchDone(RESEARCH_TOWER,  team)) return ERR_BUILD_RESEARCH;
    if(type == OBJECT_NUCLEAR && !IsResearchDone(RESEARCH_ATOMIC, team)) return ERR_BUILD_RESEARCH;

    return ERR_OK;
}

Error CRobotMain::CanFactoryError(ObjectType type, int team)
{
    ToolType tool = GetToolFromObject(type);
    DriveType drive = GetDriveFromObject(type);

    if (tool == ToolType::Sniffer        && !IsResearchDone(RESEARCH_SNIFFER,  team)) return ERR_BUILD_RESEARCH;
    if (tool == ToolType::Shooter        && !IsResearchDone(RESEARCH_CANON,    team)) return ERR_BUILD_RESEARCH;
    if (tool == ToolType::OrganicShooter && !IsResearchDone(RESEARCH_iGUN,     team)) return ERR_BUILD_RESEARCH;
    if (tool == ToolType::Builder        && !IsResearchDone(RESEARCH_BUILDER,  team)) return ERR_BUILD_RESEARCH;

    if (drive == DriveType::Tracked      && !IsResearchDone(RESEARCH_TANK,     team)) return ERR_BUILD_RESEARCH;
    if (drive == DriveType::Winged       && !IsResearchDone(RESEARCH_FLY,      team)) return ERR_BUILD_RESEARCH;
    if (drive == DriveType::Legged       && !IsResearchDone(RESEARCH_iPAW,     team)) return ERR_BUILD_RESEARCH;
    if (drive == DriveType::Heavy        && !IsResearchDone(RESEARCH_TANK,     team)) return ERR_BUILD_RESEARCH;

    if (type == OBJECT_MOBILErt          && !IsResearchDone(RESEARCH_THUMP,    team)) return ERR_BUILD_RESEARCH;
    if (type == OBJECT_MOBILErc          && !IsResearchDone(RESEARCH_PHAZER,   team)) return ERR_BUILD_RESEARCH;
    if (type == OBJECT_MOBILErr          && !IsResearchDone(RESEARCH_RECYCLER, team)) return ERR_BUILD_RESEARCH;
    if (type == OBJECT_MOBILErs          && !IsResearchDone(RESEARCH_SHIELD,   team)) return ERR_BUILD_RESEARCH;
    if (type == OBJECT_MOBILEsa          && !IsResearchDone(RESEARCH_SUBM,     team)) return ERR_BUILD_DISABLED; // Can be only researched manually in Scene file
    if (type == OBJECT_MOBILEst          && !IsResearchDone(RESEARCH_SUBM,     team)) return ERR_BUILD_DISABLED;
    if (type == OBJECT_MOBILEtg          && !IsResearchDone(RESEARCH_TARGET,   team)) return ERR_BUILD_RESEARCH;

    if (tool == ToolType::Other && drive == DriveType::Other && type != OBJECT_MOBILEtg)   return ERR_WRONG_OBJ;

    return ERR_OK;
}

void CRobotMain::PushToSelectionHistory(CObject* obj)
{
    if (!m_selectionHistory.empty() && m_selectionHistory.back() == obj)
        return; // already in history

    m_selectionHistory.push_back(obj);

    if (m_selectionHistory.size() > 50) // to avoid infinite growth
        m_selectionHistory.pop_front();
}

CObject* CRobotMain::PopFromSelectionHistory()
{
    if (m_selectionHistory.empty())
        return nullptr;

    CObject* obj = m_selectionHistory.back();
    m_selectionHistory.pop_back();
    return obj;
}

void CRobotMain::RemoveFromSelectionHistory(CObject* object)
{
    auto it = std::remove_if(m_selectionHistory.begin(), m_selectionHistory.end(),
                             [object](const CObject* obj) { return obj == object; });
    m_selectionHistory.erase(it, m_selectionHistory.end());
}

float CRobotMain::GetGlobalMagnifyDamage()
{
    return m_globalMagnifyDamage;
}

float CRobotMain::GetGlobalNuclearCapacity()
{
    return m_globalNuclearCapacity;
}

float CRobotMain::GetGlobalCellCapacity()
{
    return m_globalCellCapacity;
}

// Beginning of the effect when the instruction "detect" is used.

void CRobotMain::StartDetectEffect(COldObject* object, CObject* target)
{
    Math::Matrix*   mat;
    Math::Vector    pos, goal;
    Math::Point     dim;

    mat = object->GetWorldMatrix(0);
    pos = Math::Transform(*mat, Math::Vector(2.0f, 3.0f, 0.0f));

    if ( target == nullptr )
    {
        goal = Math::Transform(*mat, Math::Vector(50.0f, 3.0f, 0.0f));
    }
    else
    {
        goal = target->GetPosition();
        goal.y += 3.0f;
        goal = Math::SegmentPoint(pos, goal, Math::Distance(pos, goal)-3.0f);
    }

    dim.x = 3.0f;
    dim.y = dim.x;
    m_particle->CreateRay(pos, goal, Gfx::PARTIRAY2, dim, 0.2f);

    if ( target != nullptr )
    {
        goal = target->GetPosition();
        goal.y += 3.0f;
        goal = Math::SegmentPoint(pos, goal, Math::Distance(pos, goal)-1.0f);
        dim.x = 6.0f;
        dim.y = dim.x;
        m_particle->CreateParticle(goal, Math::Vector(0.0f, 0.0f, 0.0f), dim,
                                     target != nullptr ? Gfx::PARTIGLINT : Gfx::PARTIGLINTr, 0.5f);
    }

    m_sound->Play(target != nullptr ? SOUND_BUILD : SOUND_RECOVER);
}

void CRobotMain::CreateCodeBattleInterface()
{
    if (m_phase == PHASE_SIMUL)
    {
        Math::Point pos, ddim;
        float offset = (ceil(m_viewpoints.size() / 2.0f) * 50);

        int numTeams = m_scoreboard ? GetAllTeams().size() : 0;
        assert(numTeams < EVENT_SCOREBOARD_MAX-EVENT_SCOREBOARD+1);
        float textHeight = m_engine->GetText()->GetHeight(Gfx::FONT_COMMON, Gfx::FONT_SIZE_SMALL);

        //window
        ddim.x = 100.0f/640.0f;
        ddim.y = (100.0f+offset)/480.0f + numTeams * textHeight;
        pos.x = 540.0f/640.0f;
        pos.y = 100.0f/480.0f;
        Ui::CWindow* pw = m_interface->CreateWindows(pos, ddim, 3, EVENT_WINDOW6);

        //label text
        ddim.x = 100.0f/640.0f;
        ddim.y = 16.0f/480.0f;
        pos.x = 540.0f/640.0f;
        pos.y = (178.0f+offset)/480.0f + numTeams * textHeight;
        std::string text;
        GetResource(RES_EVENT, EVENT_LABEL_CODE_BATTLE, text);
        pw->CreateLabel(pos, ddim, 0, EVENT_LABEL_CODE_BATTLE, text);

        //viewpoint selection section
        ddim.x = 40.0f/640.0f;
        ddim.y = 50.0f/640.0f;
        for(unsigned int i = 0; i < m_viewpoints.size(); i++)
        {
            //create button
            pos.x = (550.0f+40.0f*(i%2))/640.0f;
            pos.y = (130.0f+offset)/480.0f + numTeams * textHeight - 45.0f*(i/2)/480.0f;
            pw->CreateButton(pos, ddim, m_viewpoints[i].button, EventType(EVENT_VIEWPOINT0 + i));
        }

        //start/camera button
        float titleBarSize = (11.0f/64.0f); // this is from the texture
        ddim.x = 80.0f/640.0f;
        ddim.y = ((1-titleBarSize)*100.0f-20.0f)/480.0f;
        pos.x = 550.0f/640.0f;
        pos.y = 110.0f/480.0f;
        if (!m_codeBattleStarted)
        {
            pw->CreateButton(pos, ddim, 21, EVENT_CODE_BATTLE_START);
        }
        else
        {
            pw->CreateButton(pos, ddim, 13, EVENT_CODE_BATTLE_SPECTATOR);
        }

        if (!m_scoreboard) return;
        pos.y += ddim.y;
        ddim.y = textHeight;
        int i = 0;
        auto teams = GetAllTeams();
        for (auto it = teams.rbegin(); it != teams.rend(); ++it)
        {
            int team = *it;
            Ui::CControl* pl;
            ddim.x = 55.0f/640.0f;
            pl = m_codeBattleStarted
                 ? static_cast<Ui::CControl*>(pw->CreateLabel(pos, ddim, 0, static_cast<EventType>(EVENT_SCOREBOARD+2*(numTeams-i-1)+0), "XXXXX"))
                 : static_cast<Ui::CControl*>(pw->CreateEdit( pos, ddim, 0, static_cast<EventType>(EVENT_SCOREBOARD+2*(numTeams-i-1)+0)));
            pl->SetTextAlign(Gfx::TEXT_ALIGN_LEFT);
            pl->SetFontSize(m_codeBattleStarted ? Gfx::FONT_SIZE_SMALL : Gfx::FONT_SIZE_SMALL*0.75f);
            m_codeBattleStarted ? pl->SetName(GetTeamName(team)) : static_cast<Ui::CEdit*>(pl)->SetText(GetTeamName(team));
            pos.x += 57.5f/640.0f;
            ddim.x = 22.5f/640.0f;
            pl = m_codeBattleStarted
                 ? static_cast<Ui::CControl*>(pw->CreateLabel(pos, ddim, 0, static_cast<EventType>(EVENT_SCOREBOARD+2*(numTeams-i-1)+1), "???"))
                 : static_cast<Ui::CControl*>(pw->CreateEdit( pos, ddim, 0, static_cast<EventType>(EVENT_SCOREBOARD+2*(numTeams-i-1)+1)));
            pl->SetTextAlign(Gfx::TEXT_ALIGN_RIGHT);
            pl->SetFontSize(m_codeBattleStarted ? Gfx::FONT_SIZE_SMALL : Gfx::FONT_SIZE_SMALL*0.75f);
            m_codeBattleStarted ? pl->SetName(StrUtils::ToString<int>(m_scoreboard->GetScore(team).points)) : static_cast<Ui::CEdit*>(pl)->SetText(StrUtils::ToString<int>(m_scoreboard->GetScore(team).points));
            pos.x -= 57.5f/640.0f;
            pos.y += ddim.y;
            i++;
        }
    }
}

void CRobotMain::ApplyCodeBattleInterface()
{
    assert(GetMissionType() == MISSION_CODE_BATTLE);
    if (!m_scoreboard) return;

    Ui::CWindow* pw = static_cast<Ui::CWindow*>(m_interface->SearchControl(EVENT_WINDOW6));
    assert(pw != nullptr);

    int i = 0;
    for (int team : GetAllTeams())
    {
        Ui::CEdit* pl;

        pl = static_cast<Ui::CEdit*>(pw->SearchControl(static_cast<EventType>(EVENT_SCOREBOARD+2*i+0)));
        assert(pl != nullptr);
        m_teamNames[team] = pl->GetText(pl->GetTextLength());

        pl = static_cast<Ui::CEdit*>(pw->SearchControl(static_cast<EventType>(EVENT_SCOREBOARD+2*i+1)));
        assert(pl != nullptr);
        m_scoreboard->SetScore(team, StrUtils::FromString<int>(pl->GetText(pl->GetTextLength())));

        i++;
    }
}

void CRobotMain::UpdateCodeBattleInterface()
{
    assert(GetMissionType() == MISSION_CODE_BATTLE);
    if (!m_scoreboard) return;

    Ui::CWindow* pw = static_cast<Ui::CWindow*>(m_interface->SearchControl(EVENT_WINDOW6));
    assert(pw != nullptr);

    int i = 0;
    for (std::pair<int, CScoreboard::Score> team : m_scoreboard->GetSortedScores())
    {
        Ui::CControl* pl;

        pl = pw->SearchControl(static_cast<EventType>(EVENT_SCOREBOARD+2*i+0));
        assert(pl != nullptr);
        pl->SetName(GetTeamName(team.first));

        pl = pw->SearchControl(static_cast<EventType>(EVENT_SCOREBOARD+2*i+1));
        assert(pl != nullptr);
        pl->SetName(StrUtils::ToString<int>(team.second.points));

        i++;
    }
}

void CRobotMain::DestroyCodeBattleInterface()
{
    m_viewpoints.clear();
    m_interface->DeleteControl(EVENT_WINDOW6);
}

void CRobotMain::SetCodeBattleSpectatorMode(bool mode)
{
    // Deselect object, but keep camera attached to it
    CObject* obj = DeselectAll();
    if (m_codeBattleSpectator)
        obj = m_camera->GetControllingObject();

    m_codeBattleSpectator = mode;
    SelectObject(obj, false); // this uses code battle selection mode already
}

void CRobotMain::UpdateDebugCrashSpheres()
{
    if (m_debugCrashSpheres)
    {
        for (CObject* obj : m_objMan->GetAllObjects())
        {
            for (const auto& crashSphere : obj->GetAllCrashSpheres())
            {
                m_engine->RenderDebugSphere(crashSphere.sphere, Math::Matrix{}, Gfx::Color{0.0f, 0.0f, 1.0f, 1.0f});
            }
        }
    }
}

void CRobotMain::SetDebugCrashSpheres(bool draw)
{
    m_debugCrashSpheres = draw;
}

bool CRobotMain::GetDebugCrashSpheres()
{
    return m_debugCrashSpheres;
}

void CRobotMain::PushToCommandHistory(std::string cmd)
{
    if (!m_commandHistory.empty() && m_commandHistory.front() == cmd) // already in history
        return;

    m_commandHistory.push_front(cmd);

    if (m_commandHistory.size() > 50) // to avoid infinite growth
        m_commandHistory.pop_back();
}

std::string CRobotMain::GetNextFromCommandHistory()
{
    if (m_commandHistory.empty() || static_cast<int>(m_commandHistory.size()) <= m_commandHistoryIndex + 1) // no next element
        return "";
    return m_commandHistory[++m_commandHistoryIndex];
}

std::string CRobotMain::GetPreviousFromCommandHistory()
{
    if (m_commandHistory.empty() || m_commandHistoryIndex < 1) // first or none element selected
        return "";
    return m_commandHistory[--m_commandHistoryIndex];
}

CScoreboard* CRobotMain::GetScoreboard()
{
    return m_scoreboard.get();
}

std::set<int> CRobotMain::GetAllTeams()
{
    std::set<int> teams = GetAllActiveTeams();
    for(auto& it : m_teamFinished)
    {
        teams.insert(it.first);
    }
    return teams;
}

std::set<int> CRobotMain::GetAllActiveTeams()
{
    std::set<int> teams;
    for (CObject* obj : m_objMan->GetAllObjects())
    {
        int team = obj->GetTeam();
        if (team == 0) continue;
        teams.insert(team);
    }
    return teams;
}
