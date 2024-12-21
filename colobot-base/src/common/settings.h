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

#include "common/language.h"
#include "common/singleton.h"

#include <deque>
#include <glm/glm.hpp>

namespace Gfx
{
    struct DeviceConfig;
}

class CSettings : public CSingleton<CSettings>
{
public:
    void SaveResolutionSettings(const Gfx::DeviceConfig& config);
    void SaveSettings();
    void LoadSettings();

    void SaveAudioSettings();
    void SaveCommandHistory(const std::deque<std::string>& commands);

    void SetTooltips(bool tooltips);
    bool GetTooltips() const;

    void SetInterfaceGlint(bool interfaceGlint);
    bool GetInterfaceGlint() const;

    void SetMouseParticlesEnabled(bool mouseParticlesEnabled);
    bool GetMouseParticlesEnabled() const;

    void SetSoluce4(bool soluce4);
    bool GetSoluce4() const;

    void SetMovies(bool movies);
    bool GetMovies() const;

    void SetFocusLostPause(bool focusLostPause);
    bool GetFocusLostPause() const;

    void SetFocusLostMute(bool focusLostMute);
    bool GetFocusLostMute() const;

    //! Managing the size of the default fonts
    //@{
    void        SetFontSize(float size);
    float       GetFontSize() const;
    //@}

    //! Managing the size of the default window
    //@{
    void        SetWindowPos(const glm::vec2& pos);
    glm::vec2   GetWindowPos() const;

    void        SetWindowDim(const glm::vec2& dim);
    glm::vec2   GetWindowDim() const;

    void        SetWindowMax(bool max);
    bool        GetWindowMax() const;
    //@}

    //! Managing windows open/save
    //@{
    void        SetIOPublic(bool mode);
    bool        GetIOPublic() const;

    void        SetIOPos(const glm::vec2& pos);
    glm::vec2   GetIOPos() const;

    void        SetIODim(const glm::vec2& dim);
    glm::vec2   GetIODim() const;
    //@}

    void SetLanguage(Language language);
    Language GetLanguage() const;

protected:
    bool m_tooltips = true;
    bool m_interfaceGlint = true;
    bool m_mouseParticlesEnabled = true;
    bool m_soluce4 = true;
    bool m_movies = true;
    bool m_focusLostPause = true;
    bool m_focusLostMute = true;

    float m_fontSize = 19.0f;
    glm::vec2 m_windowPos = { 0.15f, 0.17f };
    glm::vec2 m_windowDim = { 0.70f, 0.66f };
    bool m_windowMax = false;

    bool m_IOPublic = false;
    glm::vec2 m_IOPos = { (1.0f - m_IODim.x) / 2.0f, (1.0f - m_IODim.y) / 2.0f };
    glm::vec2 m_IODim = { 320.0f / 640.0f, (121.0f + 18.0f * 8) / 480.0f };

    Language m_language = LANGUAGE_ENV;
};
