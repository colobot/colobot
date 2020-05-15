/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2018, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include "graphics/engine/camera.h"
#include "graphics/engine/engine.h"

#include "level/robotmain.h"

#include "sound/sound.h"

CSettings::CSettings()
{
    m_tooltips       = true;
    m_interfaceGlint = true;
    m_interfaceRain  = true;
    m_soluce4        = true;
    m_movies         = true;
    m_focusLostPause = true;

    m_fontSize  = 19.0f;
    m_windowPos = Math::Point(0.15f, 0.17f);
    m_windowDim = Math::Point(0.70f, 0.66f);
    m_windowMax = false;

    m_IOPublic = false;
    m_IODim = Math::Point(320.0f/640.0f, (121.0f+18.0f*8)/480.0f);
    m_IOPos.x = (1.0f-m_IODim.x)/2.0f;  // in the middle
    m_IOPos.y = (1.0f-m_IODim.y)/2.0f;

    m_language = LANGUAGE_ENV;
}

void CSettings::SaveResolutionSettings(const Gfx::DeviceConfig& config)
{
    // NOTE: These settings are loaded in CApplication

    std::ostringstream ss;
    ss << config.size.x << "x" << config.size.y;
    GetConfigFile().SetStringProperty("Setup", "Resolution", ss.str());
    GetConfigFile().SetBoolProperty("Setup", "Fullscreen", config.fullScreen);
    GetConfigFile().Save();
}

void CSettings::SaveSettings()
{
    CApplication* app = CApplication::GetInstancePointer();
    CRobotMain* main = CRobotMain::GetInstancePointer();
    Gfx::CEngine* engine = Gfx::CEngine::GetInstancePointer();
    Gfx::CCamera* camera = main->GetCamera();
    CSoundInterface* sound = app->GetSound();

    GetConfigFile().SetBoolProperty("Setup", "Tooltips", m_tooltips);
    GetConfigFile().SetBoolProperty("Setup", "InterfaceGlint", m_interfaceGlint);
    GetConfigFile().SetBoolProperty("Setup", "InterfaceRain", m_interfaceRain);
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
    GetConfigFile().SetIntProperty("Setup", "AudioVolume", sound->GetAudioVolume());
    GetConfigFile().SetIntProperty("Setup", "MusicVolume", sound->GetMusicVolume());
    GetConfigFile().SetBoolProperty("Setup", "EditIndentMode", engine->GetEditIndentMode());
    GetConfigFile().SetIntProperty("Setup", "EditIndentValue", engine->GetEditIndentValue());
    GetConfigFile().SetBoolProperty("Setup", "PauseBlur", engine->GetPauseBlurEnabled());

    GetConfigFile().SetIntProperty("Setup", "MipmapLevel", engine->GetTextureMipmapLevel());
    GetConfigFile().SetIntProperty("Setup", "Anisotropy", engine->GetTextureAnisotropyLevel());
    GetConfigFile().SetFloatProperty("Setup", "ShadowColor", engine->GetShadowColor());
    GetConfigFile().SetFloatProperty("Setup", "ShadowRange", engine->GetShadowRange());
    GetConfigFile().SetIntProperty("Setup", "MSAA", engine->GetMultiSample());
    GetConfigFile().SetIntProperty("Setup", "FilterMode", engine->GetTextureFilterMode());
    GetConfigFile().SetBoolProperty("Setup", "ShadowMapping", engine->GetShadowMapping());
    GetConfigFile().SetBoolProperty("Setup", "ShadowMappingQuality", engine->GetShadowMappingQuality());
    GetConfigFile().SetIntProperty("Setup", "ShadowMappingResolution",
                                   engine->GetShadowMappingOffscreen() ? engine->GetShadowMappingOffscreenResolution() : 0);

    // Experimental settings
    GetConfigFile().SetBoolProperty("Experimental", "TerrainShadows", engine->GetTerrainShadows());
    GetConfigFile().SetIntProperty("Setup", "VSync", engine->GetVSync());

    CInput::GetInstancePointer()->SaveKeyBindings();



    GetConfigFile().SetFloatProperty("Edit", "FontSize", m_fontSize);
    GetConfigFile().SetFloatProperty("Edit", "WindowPosX", m_windowPos.x);
    GetConfigFile().SetFloatProperty("Edit", "WindowPosY", m_windowPos.y);
    GetConfigFile().SetFloatProperty("Edit", "WindowDimX", m_windowDim.x);
    GetConfigFile().SetFloatProperty("Edit", "WindowDimY", m_windowDim.y);
    GetConfigFile().SetBoolProperty ("Edit", "WindowMaximized", m_windowMax);

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
    GetConfigFile().GetBoolProperty("Setup", "InterfaceRain", m_interfaceRain);
    GetConfigFile().GetBoolProperty("Setup", "Soluce4", m_soluce4);
    GetConfigFile().GetBoolProperty("Setup", "Movies", m_movies);
    GetConfigFile().GetBoolProperty("Setup", "FocusLostPause", m_focusLostPause);

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
            for(const auto& joystick : joysticks)
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
        engine->SetTextureFilterMode(static_cast<Gfx::TexFilter>(iValue));

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



    GetConfigFile().GetFloatProperty("Edit", "FontSize",    m_fontSize);
    GetConfigFile().GetFloatProperty("Edit", "WindowPosX",  m_windowPos.x);
    GetConfigFile().GetFloatProperty("Edit", "WindowPosY",  m_windowPos.y);
    GetConfigFile().GetFloatProperty("Edit", "WindowDimX",  m_windowDim.x);
    GetConfigFile().GetFloatProperty("Edit", "WindowDimY",  m_windowDim.y);
    GetConfigFile().GetBoolProperty ("Edit", "WindowMaximized", m_windowMax);

    GetConfigFile().GetBoolProperty ("Edit", "IOPublic", m_IOPublic);
    GetConfigFile().GetFloatProperty("Edit", "IOPosX",   m_IOPos.x);
    GetConfigFile().GetFloatProperty("Edit", "IOPosY",   m_IOPos.y);
    GetConfigFile().GetFloatProperty("Edit", "IODimX",   m_IODim.x);
    GetConfigFile().GetFloatProperty("Edit", "IODimY",   m_IODim.y);

    m_language = LANGUAGE_ENV;
    if (GetConfigFile().GetStringProperty("Language", "Lang", sValue))
    {
        if (!sValue.empty() && !ParseLanguage(sValue, m_language))
        {
            GetLogger()->Error("Failed to parse language '%s' from config file. Default language will be used.\n",
                               sValue.c_str());
        }
    }
    app->SetLanguage(m_language);
}

void CSettings::SetTooltips(bool tooltips)
{
    m_tooltips = tooltips;
}
bool CSettings::GetTooltips()
{
    return m_tooltips;
}

void CSettings::SetInterfaceGlint(bool interfaceGlint)
{
    m_interfaceGlint = interfaceGlint;
}
bool CSettings::GetInterfaceGlint()
{
    return m_interfaceGlint;
}

void CSettings::SetInterfaceRain(bool interfaceRain)
{
    m_interfaceRain = interfaceRain;
}
bool CSettings::GetInterfaceRain()
{
    return m_interfaceRain;
}

void CSettings::SetSoluce4(bool soluce4)
{
    m_soluce4 = soluce4;
}
bool CSettings::GetSoluce4()
{
    return m_soluce4;
}

void CSettings::SetMovies(bool movies)
{
    m_movies = movies;
}
bool CSettings::GetMovies()
{
    return m_movies;
}

void CSettings::SetFocusLostPause(bool focusLostPause)
{
    m_focusLostPause = focusLostPause;
}
bool CSettings::GetFocusLostPause()
{
    return m_focusLostPause;
}


void CSettings::SetFontSize(float size)
{
    m_fontSize = size;
    GetConfigFile().SetFloatProperty("Edit", "FontSize", m_fontSize);
    GetConfigFile().Save();
}

float CSettings::GetFontSize()
{
    return m_fontSize;
}

void CSettings::SetWindowPos(Math::Point pos)
{
    m_windowPos = pos;
    GetConfigFile().SetFloatProperty("Edit", "WindowPosX", m_windowPos.x);
    GetConfigFile().SetFloatProperty("Edit", "WindowPosY", m_windowPos.y);
    GetConfigFile().Save();
}

Math::Point CSettings::GetWindowPos()
{
    return m_windowPos;
}

void CSettings::SetWindowDim(Math::Point dim)
{
    m_windowDim = dim;
    GetConfigFile().SetFloatProperty("Edit", "WindowDimX", m_windowDim.x);
    GetConfigFile().SetFloatProperty("Edit", "WindowDimY", m_windowDim.y);
    GetConfigFile().Save();
}

Math::Point CSettings::GetWindowDim()
{
    return m_windowDim;
}

void CSettings::SetWindowMax(bool max)
{
    m_windowMax = max;
    GetConfigFile().SetBoolProperty("Edit", "WindowMaximized", m_windowMax);
    GetConfigFile().Save();
}

bool CSettings::GetWindowMax()
{
    return m_windowMax;
}

void CSettings::SetIOPublic(bool mode)
{
    m_IOPublic = mode;
    GetConfigFile().SetBoolProperty("Edit", "IOPublic", m_IOPublic);
    GetConfigFile().Save();
}

bool CSettings::GetIOPublic()
{
    return m_IOPublic;
}

void CSettings::SetIOPos(Math::Point pos)
{
    m_IOPos = pos;
    GetConfigFile().SetFloatProperty("Edit", "IOPosX", m_IOPos.x);
    GetConfigFile().SetFloatProperty("Edit", "IOPosY", m_IOPos.y);
    GetConfigFile().Save();
}

Math::Point CSettings::GetIOPos()
{
    return m_IOPos;
}

void CSettings::SetIODim(Math::Point dim)
{
    m_IODim = dim;
    GetConfigFile().SetFloatProperty("Edit", "IODimX", m_IODim.x);
    GetConfigFile().SetFloatProperty("Edit", "IODimY", m_IODim.y);
    GetConfigFile().Save();
}

Math::Point CSettings::GetIODim()
{
    return m_IODim;
}

void CSettings::SetLanguage(Language language)
{
    m_language = language;
    CApplication::GetInstancePointer()->SetLanguage(m_language);
}

Language CSettings::GetLanguage()
{
    return m_language;
}
