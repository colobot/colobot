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

#include "common/settings.h"

#include "app/app.h"
#include "app/input.h"

#include "common/config_file.h"
#include "common/logger.h"

#include "graphics/core/device.h"
#include "graphics/engine/camera.h"
#include "graphics/engine/engine.h"

#include "level/robotmain.h"

#include "sound/sound.h"

void CSettings::SaveResolutionSettings(const Gfx::DeviceConfig& config)
{
    std::vector<int> values = { config.size.x, config.size.y };
    GetConfigFile().SetArrayProperty("Setup", "Resolution", values);
    GetConfigFile().SetBoolProperty("Setup", "Fullscreen", config.fullScreen);
    GetConfigFile().Save();
}

void CSettings::SaveSettings()
{
    CApplication* app = CApplication::GetInstancePointer();
    CRobotMain* main = CRobotMain::GetInstancePointer();
    Gfx::CEngine* engine = Gfx::CEngine::GetInstancePointer();
    Gfx::CCamera* camera = main->GetCamera();

    GetConfigFile().SetBoolProperty("Setup", "Tooltips", m_tooltips);
    GetConfigFile().SetBoolProperty("Setup", "InterfaceGlint", m_interfaceGlint);
    GetConfigFile().SetBoolProperty("Setup", "MouseParticlesEnabled", m_mouseParticlesEnabled);
    GetConfigFile().SetBoolProperty("Setup", "Soluce4", m_soluce4);
    GetConfigFile().SetBoolProperty("Setup", "Movies", m_movies);
    GetConfigFile().SetBoolProperty("Setup", "FocusLostPause", m_focusLostPause);
    GetConfigFile().SetBoolProperty("Setup", "OldCameraScroll", camera->GetOldCameraScroll());
    GetConfigFile().SetBoolProperty("Setup", "CameraInvertX", camera->GetCameraInvertX());
    GetConfigFile().SetBoolProperty("Setup", "CameraInvertY", camera->GetCameraInvertY());
    GetConfigFile().SetBoolProperty("Setup", "InterfaceEffect", camera->GetEffect());
    GetConfigFile().SetBoolProperty("Setup", "Blood", camera->GetBlood());
    GetConfigFile().SetBoolProperty("Setup", "Autosave", main->GetAutosave());
    GetConfigFile().SetIntProperty("Setup", "AutosaveInterval", main->GetAutosaveInterval());
    GetConfigFile().SetIntProperty("Setup", "AutosaveSlots", main->GetAutosaveSlots());
    GetConfigFile().SetBoolProperty("Setup", "ObjectDirty", engine->GetDirty());
    GetConfigFile().SetBoolProperty("Setup", "FogMode", engine->GetFog());
    GetConfigFile().SetBoolProperty("Setup", "LightMode", engine->GetLightMode());
    GetConfigFile().SetIntProperty("Setup", "JoystickIndex", app->GetJoystickEnabled() ? app->GetJoystick().index : -1);
    GetConfigFile().SetFloatProperty("Setup", "ParticleDensity", engine->GetParticleDensity());
    GetConfigFile().SetFloatProperty("Setup", "ClippingDistance", engine->GetClippingDistance());
    GetConfigFile().SetBoolProperty("Setup", "EditIndentMode", engine->GetEditIndentMode());
    GetConfigFile().SetIntProperty("Setup", "EditIndentValue", engine->GetEditIndentValue());
    GetConfigFile().SetBoolProperty("Setup", "PauseBlur", engine->GetPauseBlurEnabled());

    GetConfigFile().SetIntProperty("Setup", "MipmapLevel", engine->GetTextureMipmapLevel());
    GetConfigFile().SetIntProperty("Setup", "Anisotropy", engine->GetTextureAnisotropyLevel());
    GetConfigFile().SetFloatProperty("Setup", "ShadowColor", engine->GetShadowColor());
    GetConfigFile().SetFloatProperty("Setup", "ShadowRange", engine->GetShadowRange());
    GetConfigFile().SetIntProperty("Setup", "MSAA", engine->GetMultiSample());
    GetConfigFile().SetIntProperty("Setup", "FilterMode", static_cast<int>(engine->GetTextureFilterMode()));
    GetConfigFile().SetBoolProperty("Setup", "ShadowMapping", engine->GetShadowMapping());
    GetConfigFile().SetBoolProperty("Setup", "ShadowMappingQuality", engine->GetShadowMappingQuality());
    GetConfigFile().SetIntProperty("Setup", "ShadowMappingResolution",
        engine->GetShadowMappingOffscreen() ? engine->GetShadowMappingOffscreenResolution() : 0);

    // Save Audio settings
    SaveAudioSettings();

    // Experimental settings
    GetConfigFile().SetBoolProperty("Experimental", "TerrainShadows", engine->GetTerrainShadows());
    GetConfigFile().SetIntProperty("Setup", "VSync", engine->GetVSync());

    CInput::GetInstancePointer()->SaveKeyBindings();



    GetConfigFile().SetFloatProperty("Edit", "FontSize", m_fontSize);
    GetConfigFile().SetFloatProperty("Edit", "WindowPosX", m_windowPos.x);
    GetConfigFile().SetFloatProperty("Edit", "WindowPosY", m_windowPos.y);
    GetConfigFile().SetFloatProperty("Edit", "WindowDimX", m_windowDim.x);
    GetConfigFile().SetFloatProperty("Edit", "WindowDimY", m_windowDim.y);
    GetConfigFile().SetBoolProperty("Edit", "WindowMaximized", m_windowMax);

    GetConfigFile().SetBoolProperty("Edit", "IOPublic", m_IOPublic);
    GetConfigFile().SetFloatProperty("Edit", "IOPosX", m_IOPos.x);
    GetConfigFile().SetFloatProperty("Edit", "IOPosY", m_IOPos.y);
    GetConfigFile().SetFloatProperty("Edit", "IODimX", m_IODim.x);
    GetConfigFile().SetFloatProperty("Edit", "IODimY", m_IODim.y);

    std::string lang = "";
    LanguageToString(m_language, lang);
    GetConfigFile().SetStringProperty("Language", "Lang", lang);

    GetConfigFile().Save();
}

void CSettings::SaveAudioSettings()
{
    CApplication* app = CApplication::GetInstancePointer();
    CSoundInterface* sound = app->GetSound();

    GetConfigFile().SetIntProperty("Setup", "AudioVolume", sound->GetAudioVolume());
    GetConfigFile().SetIntProperty("Setup", "MusicVolume", sound->GetMusicVolume());
    GetConfigFile().SetBoolProperty("Setup", "FocusLostMute", m_focusLostMute);
}

void CSettings::SaveCommandHistory(const std::deque<std::string>& commands)
{
    GetConfigFile().SetDequeProperty("Setup", "CommandHistory", commands);
    GetConfigFile().Save();
}

void CSettings::LoadSettings()
{
    CApplication* app = CApplication::GetInstancePointer();
    CRobotMain* main = CRobotMain::GetInstancePointer();
    Gfx::CEngine* engine = Gfx::CEngine::GetInstancePointer();
    Gfx::CCamera* camera = main->GetCamera();
    CSoundInterface* sound = app->GetSound();

    int iValue = 0;
    float fValue = 0.0f;
    bool bValue = false;
    std::string sValue = "";

    GetConfigFile().GetBoolProperty("Setup", "Tooltips", m_tooltips);
    GetConfigFile().GetBoolProperty("Setup", "InterfaceGlint", m_interfaceGlint);
    GetConfigFile().GetBoolProperty("Setup", "MouseParticlesEnabled", m_mouseParticlesEnabled);
    GetConfigFile().GetBoolProperty("Setup", "Soluce4", m_soluce4);
    GetConfigFile().GetBoolProperty("Setup", "Movies", m_movies);
    GetConfigFile().GetBoolProperty("Setup", "FocusLostPause", m_focusLostPause);
    GetConfigFile().GetBoolProperty("Setup", "FocusLostMute", m_focusLostMute);

    if (GetConfigFile().GetBoolProperty("Setup", "OldCameraScroll", bValue))
        camera->SetOldCameraScroll(bValue);

    if (GetConfigFile().GetBoolProperty("Setup", "CameraInvertX", bValue))
        camera->SetCameraInvertX(bValue);

    if (GetConfigFile().GetBoolProperty("Setup", "CameraInvertY", bValue))
        camera->SetCameraInvertY(bValue);

    if (GetConfigFile().GetBoolProperty("Setup", "InterfaceEffect", bValue))
        camera->SetEffect(bValue);

    if (GetConfigFile().GetBoolProperty("Setup", "Blood", bValue))
        camera->SetBlood(bValue);

    if (GetConfigFile().GetBoolProperty("Setup", "Autosave", bValue))
        main->SetAutosave(bValue);

    if (GetConfigFile().GetIntProperty("Setup", "AutosaveInterval", iValue))
        main->SetAutosaveInterval(iValue);

    if (GetConfigFile().GetIntProperty("Setup", "AutosaveSlots", iValue))
        main->SetAutosaveSlots(iValue);

    if (GetConfigFile().GetBoolProperty("Setup", "ObjectDirty", bValue))
        engine->SetDirty(bValue);

    if (GetConfigFile().GetBoolProperty("Setup", "FogMode", bValue))
    {
        engine->SetFog(bValue);
        camera->SetOverBaseColor(Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f)); // TODO: color ok?
    }

    if (GetConfigFile().GetBoolProperty("Setup", "LightMode", bValue))
        engine->SetLightMode(bValue);

    if (GetConfigFile().GetIntProperty("Setup", "JoystickIndex", iValue))
    {
        if (iValue >= 0)
        {
            auto joysticks = app->GetJoystickList();
            for (const auto& joystick : joysticks)
            {
                if (joystick.index == iValue)
                {
                    app->ChangeJoystick(joystick);
                    app->SetJoystickEnabled(true);
                }
            }
        }
        else
        {
            app->SetJoystickEnabled(false);
        }
    }

    if (GetConfigFile().GetFloatProperty("Setup", "ParticleDensity", fValue))
        engine->SetParticleDensity(fValue);

    if (GetConfigFile().GetFloatProperty("Setup", "ClippingDistance", fValue))
        engine->SetClippingDistance(fValue);

    if (GetConfigFile().GetIntProperty("Setup", "AudioVolume", iValue))
        sound->SetAudioVolume(iValue);

    if (GetConfigFile().GetIntProperty("Setup", "MusicVolume", iValue))
        sound->SetMusicVolume(iValue);

    if (GetConfigFile().GetBoolProperty("Setup", "EditIndentMode", bValue))
        engine->SetEditIndentMode(bValue);

    if (GetConfigFile().GetIntProperty("Setup", "EditIndentValue", iValue))
        engine->SetEditIndentValue(iValue);

    if (GetConfigFile().GetBoolProperty("Setup", "PauseBlur", bValue))
        engine->SetPauseBlurEnabled(bValue);


    if (GetConfigFile().GetIntProperty("Setup", "MipmapLevel", iValue))
        engine->SetTextureMipmapLevel(iValue);

    if (GetConfigFile().GetIntProperty("Setup", "Anisotropy", iValue))
        engine->SetTextureAnisotropyLevel(iValue);

    if (GetConfigFile().GetFloatProperty("Setup", "ShadowColor", fValue))
        engine->SetShadowColor(fValue);

    if (GetConfigFile().GetFloatProperty("Setup", "ShadowRange", fValue))
        engine->SetShadowRange(fValue);

    if (GetConfigFile().GetIntProperty("Setup", "MSAA", iValue))
        engine->SetMultiSample(iValue);

    if (GetConfigFile().GetIntProperty("Setup", "FilterMode", iValue))
        engine->SetTextureFilterMode(static_cast<Gfx::TextureFilter>(iValue));

    if (GetConfigFile().GetBoolProperty("Setup", "ShadowMapping", bValue))
        engine->SetShadowMapping(bValue);

    if (GetConfigFile().GetBoolProperty("Setup", "ShadowMappingQuality", bValue))
        engine->SetShadowMappingQuality(bValue);

    if (GetConfigFile().GetIntProperty("Setup", "ShadowMappingResolution", iValue))
    {
        if (iValue == 0)
        {
            engine->SetShadowMappingOffscreen(false);
        }
        else
        {
            engine->SetShadowMappingOffscreen(true);
            engine->SetShadowMappingOffscreenResolution(iValue);
        }
    }

    if (GetConfigFile().GetBoolProperty("Experimental", "TerrainShadows", bValue))
        engine->SetTerrainShadows(bValue);
    if (GetConfigFile().GetIntProperty("Setup", "VSync", iValue))
    {
        engine->SetVSync(iValue);
    }

    CInput::GetInstancePointer()->LoadKeyBindings();

    std::deque<std::string> commands;
    GetConfigFile().GetDequeProperty("Setup", "CommandHistory", commands);
    main->SetCommandHistory(std::move(commands));

    GetConfigFile().GetFloatProperty("Edit", "FontSize", m_fontSize);
    GetConfigFile().GetFloatProperty("Edit", "WindowPosX", m_windowPos.x);
    GetConfigFile().GetFloatProperty("Edit", "WindowPosY", m_windowPos.y);
    GetConfigFile().GetFloatProperty("Edit", "WindowDimX", m_windowDim.x);
    GetConfigFile().GetFloatProperty("Edit", "WindowDimY", m_windowDim.y);
    GetConfigFile().GetBoolProperty("Edit", "WindowMaximized", m_windowMax);

    GetConfigFile().GetBoolProperty("Edit", "IOPublic", m_IOPublic);
    GetConfigFile().GetFloatProperty("Edit", "IOPosX", m_IOPos.x);
    GetConfigFile().GetFloatProperty("Edit", "IOPosY", m_IOPos.y);
    GetConfigFile().GetFloatProperty("Edit", "IODimX", m_IODim.x);
    GetConfigFile().GetFloatProperty("Edit", "IODimY", m_IODim.y);

    m_language = LANGUAGE_ENV;
    if (GetConfigFile().GetStringProperty("Language", "Lang", sValue))
    {
        if (!sValue.empty() && !ParseLanguage(sValue, m_language))
        {
            GetLogger()->Error("Failed to parse language '%%' from config file. Default language will be used.", sValue);
        }
    }
    app->SetLanguage(m_language);
}

void CSettings::SetTooltips(bool tooltips)
{
    m_tooltips = tooltips;
}
bool CSettings::GetTooltips() const
{
    return m_tooltips;
}

void CSettings::SetInterfaceGlint(bool interfaceGlint)
{
    m_interfaceGlint = interfaceGlint;
}
bool CSettings::GetInterfaceGlint() const
{
    return m_interfaceGlint;
}

void CSettings::SetMouseParticlesEnabled(bool mouseParticlesEnabled)
{
    m_mouseParticlesEnabled = mouseParticlesEnabled;
}
bool CSettings::GetMouseParticlesEnabled() const
{
    return m_mouseParticlesEnabled;
}

void CSettings::SetSoluce4(bool soluce4)
{
    m_soluce4 = soluce4;
}
bool CSettings::GetSoluce4() const
{
    return m_soluce4;
}

void CSettings::SetMovies(bool movies)
{
    m_movies = movies;
}
bool CSettings::GetMovies() const
{
    return m_movies;
}

void CSettings::SetFocusLostPause(bool focusLostPause)
{
    m_focusLostPause = focusLostPause;
}
bool CSettings::GetFocusLostPause() const
{
    return m_focusLostPause;
}

void CSettings::SetFocusLostMute(bool focusLostMute)
{
    m_focusLostMute = focusLostMute;
}
bool CSettings::GetFocusLostMute() const
{
    return m_focusLostMute;
}

void CSettings::SetFontSize(float size)
{
    m_fontSize = size;
    GetConfigFile().SetFloatProperty("Edit", "FontSize", m_fontSize);
    GetConfigFile().Save();
}

float CSettings::GetFontSize() const
{
    return m_fontSize;
}

void CSettings::SetWindowPos(const glm::vec2& pos)
{
    m_windowPos = pos;
    GetConfigFile().SetFloatProperty("Edit", "WindowPosX", m_windowPos.x);
    GetConfigFile().SetFloatProperty("Edit", "WindowPosY", m_windowPos.y);
    GetConfigFile().Save();
}

glm::vec2 CSettings::GetWindowPos() const
{
    return m_windowPos;
}

void CSettings::SetWindowDim(const glm::vec2& dim)
{
    m_windowDim = dim;
    GetConfigFile().SetFloatProperty("Edit", "WindowDimX", m_windowDim.x);
    GetConfigFile().SetFloatProperty("Edit", "WindowDimY", m_windowDim.y);
    GetConfigFile().Save();
}

glm::vec2 CSettings::GetWindowDim() const
{
    return m_windowDim;
}

void CSettings::SetWindowMax(bool max)
{
    m_windowMax = max;
    GetConfigFile().SetBoolProperty("Edit", "WindowMaximized", m_windowMax);
    GetConfigFile().Save();
}

bool CSettings::GetWindowMax() const
{
    return m_windowMax;
}

void CSettings::SetIOPublic(bool mode)
{
    m_IOPublic = mode;
    GetConfigFile().SetBoolProperty("Edit", "IOPublic", m_IOPublic);
    GetConfigFile().Save();
}

bool CSettings::GetIOPublic() const
{
    return m_IOPublic;
}

void CSettings::SetIOPos(const glm::vec2& pos)
{
    m_IOPos = pos;
    GetConfigFile().SetFloatProperty("Edit", "IOPosX", m_IOPos.x);
    GetConfigFile().SetFloatProperty("Edit", "IOPosY", m_IOPos.y);
    GetConfigFile().Save();
}

glm::vec2 CSettings::GetIOPos() const
{
    return m_IOPos;
}

void CSettings::SetIODim(const glm::vec2& dim)
{
    m_IODim = dim;
    GetConfigFile().SetFloatProperty("Edit", "IODimX", m_IODim.x);
    GetConfigFile().SetFloatProperty("Edit", "IODimY", m_IODim.y);
    GetConfigFile().Save();
}

glm::vec2 CSettings::GetIODim() const
{
    return m_IODim;
}

void CSettings::SetLanguage(Language language)
{
    m_language = language;
    CApplication::GetInstancePointer()->SetLanguage(m_language);
}

Language CSettings::GetLanguage() const
{
    return m_language;
}
