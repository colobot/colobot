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

#pragma once

#include "common/language.h"
#include "common/singleton.h"

#include "math/point.h"

namespace Gfx
{
struct DeviceConfig;
}

class CSettings : public CSingleton<CSettings>
{
public:
    CSettings();
    void SaveResolutionSettings(const Gfx::DeviceConfig& config);
    void SaveSettings();
    void LoadSettings();


    void SetTooltips(bool tooltips);
    bool GetTooltips();

    void SetInterfaceGlint(bool interfaceGlint);
    bool GetInterfaceGlint();

    void SetInterfaceRain(bool interfaceRain);
    bool GetInterfaceRain();

    void SetSoluce4(bool soluce4);
    bool GetSoluce4();

    void SetMovies(bool movies);
    bool GetMovies();

    void SetFocusLostPause(bool focusLostPause);
    bool GetFocusLostPause();


    //! Managing the size of the default fonts
    //@{
    void        SetFontSize(float size);
    float       GetFontSize();
    //@}

    //! Managing the size of the default window
    //@{
    void        SetWindowPos(Math::Point pos);
    Math::Point GetWindowPos();

    void        SetWindowDim(Math::Point dim);
    Math::Point GetWindowDim();

    void        SetWindowMax(bool max);
    bool        GetWindowMax();
    //@}

    //! Managing windows open/save
    //@{
    void        SetIOPublic(bool mode);
    bool        GetIOPublic();

    void        SetIOPos(Math::Point pos);
    Math::Point GetIOPos();

    void        SetIODim(Math::Point dim);
    Math::Point GetIODim();
    //@}

    void SetLanguage(Language language);
    Language GetLanguage();

protected:
    bool m_tooltips;
    bool m_interfaceGlint;
    bool m_interfaceRain;
    bool m_soluce4;
    bool m_movies;
    bool m_focusLostPause;

    float           m_fontSize;
    Math::Point     m_windowPos;
    Math::Point     m_windowDim;
    bool            m_windowMax;

    bool            m_IOPublic;
    Math::Point     m_IOPos;
    Math::Point     m_IODim;

    Language m_language;
};
