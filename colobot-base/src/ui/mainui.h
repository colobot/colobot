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

#pragma once

#include "level/robotmain.h"

#include "ui/particles_generator.h"

#include <glm/glm.hpp>

#include <array>
#include <string>
#include <vector>
#include <filesystem>


class CApplication;
class CSettings;
class CSoundInterface;

namespace Gfx
{
class CEngine;
class CParticle;
} // namespace Gfx

namespace Ui
{
class CInterface;
class CMainDialog;

class CScreen;
class CScreenAppearance;
class CScreenIORead;
class CScreenIOWrite;
class CScreenLevelList;
class CScreenLoading;
class CScreenMainMenu;
class CScreenModList;
class CScreenPlayerSelect;
class CScreenQuit;
class CScreenSetup;
class CScreenSetupControls;
class CScreenSetupDisplay;
class CScreenSetupGame;
class CScreenSetupGraphics;
class CScreenSetupSound;
class CScreenWelcome;


class CMainUserInterface
{
public:
    CMainUserInterface();
    ~CMainUserInterface();

    CMainDialog* GetDialog();

    void    ShowLoadingScreen(bool show);
    CScreenLoading* GetLoadingScreen();

    bool    EventProcess(const Event &event);
    void    ChangePhase(Phase phase);

    bool    GetSceneSoluce();
    bool    GetPlusTrainer();
    bool    GetPlusResearch();
    bool    GetPlusExplorer();

    void    UpdateChapterPassed();
    void    NextMission();

    bool    GetGamerOnlyHead();
    float   GetPersoAngle();

    void    AllMissionUpdate();
    void    ShowSoluceUpdate();

    void    UpdateCustomLevelList();
    std::filesystem::path GetCustomLevelName(int id);
    const std::vector<std::filesystem::path>& GetCustomLevelList();

protected:
    void    GlintMove();
    void    FrameParticle(float rTime);
    void    CreateMouseParticles(const glm::vec2& mousePosition, bool buttonPressed);
    CScreenSetup* GetSetupScreen(Phase phase);

protected:
    CApplication*     m_app;
    CRobotMain*       m_main;
    Gfx::CEngine*     m_engine;
    Gfx::CParticle*   m_particleManager;
    CInterface*       m_interface;
    CSoundInterface*  m_sound;
    CSettings*        m_settings;

    std::unique_ptr<CMainDialog> m_dialog;

    CScreen* m_currentScreen;
    std::unique_ptr<CScreenAppearance> m_screenAppearance;
    std::unique_ptr<CScreenIORead> m_screenIORead;
    std::unique_ptr<CScreenIOWrite> m_screenIOWrite;
    std::unique_ptr<CScreenLevelList> m_screenLevelList;
    std::unique_ptr<CScreenLoading> m_screenLoading;
    std::unique_ptr<CScreenMainMenu> m_screenMainMenu;
    std::unique_ptr<CScreenModList> m_screenModList;
    std::unique_ptr<CScreenPlayerSelect> m_screenPlayerSelect;
    std::unique_ptr<CScreenQuit> m_screenQuit;
    std::unique_ptr<CScreenSetupControls> m_screenSetupControls;
    std::unique_ptr<CScreenSetupDisplay> m_screenSetupDisplay;
    std::unique_ptr<CScreenSetupGame> m_screenSetupGame;
    std::unique_ptr<CScreenSetupGraphics> m_screenSetupGraphics;
    std::unique_ptr<CScreenSetupSound> m_screenSetupSound;
    std::unique_ptr<CScreenWelcome> m_screenWelcome;

    Phase                m_phase;            // copy of CRobotMain

    int                  m_shotDelay;        // number of frames before copy
    std::string          m_shotName;        // generate a file name

    glm::vec2            m_glintMouse;
    float                m_glintTime;

    struct Particle
    {
        int phase = 0;
        float time = 0.0f;
        glm::vec2 pos = { 0, 0 };
    };
    std::array<Particle, 10> m_particles;

private:
    std::unique_ptr<UI::CParticlesGenerator> m_mouseParticlesGenerator;
    bool isAllowedToCreateMouseParticles();
};

} // namespace Ui
