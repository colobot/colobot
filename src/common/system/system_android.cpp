/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2020, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include "common/system/system_android.h"

#include <SDL2/SDL.h>

void CSystemUtilsAndroid::Init()
{
}

SystemDialogResult CSystemUtilsAndroid::SystemDialog(SystemDialogType type, const std::string& title, const std::string& message)
{
    switch(type) {
        case SDT_INFO:
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, title.c_str(), message.c_str(), NULL);
            return SDR_OK;
        case SDT_WARNING:
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, title.c_str(), message.c_str(), NULL);
            return SDR_OK;
        case SDT_ERROR:
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title.c_str(), message.c_str(), NULL);
            return SDR_OK;
        case SDT_YES_NO: {
            const SDL_MessageBoxButtonData buttons[] = {
                    {SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 0, "no"},
                    {SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 1, "yes"},
            };
            const SDL_MessageBoxData messageboxdata = {
                    SDL_MESSAGEBOX_INFORMATION,
                    NULL,
                    title.c_str(),
                    message.c_str(),
                    SDL_arraysize(buttons),
                    buttons,
                    NULL
            };
            int buttonid = -1;
            SDL_ShowMessageBox(&messageboxdata, &buttonid);
            return buttonid == 1 ? SDR_YES : SDR_NO;
        }
        case SDT_OK_CANCEL: {
            const SDL_MessageBoxButtonData buttons[] = {
                    {SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 0, "cancel"},
                    {SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 1, "ok"},
            };
            const SDL_MessageBoxData messageboxdata = {
                    SDL_MESSAGEBOX_INFORMATION,
                    NULL,
                    title.c_str(),
                    message.c_str(),
                    SDL_arraysize(buttons),
                    buttons,
                    NULL
            };
            int buttonid = -1;
            SDL_ShowMessageBox(&messageboxdata, &buttonid);
            return buttonid == 1 ? SDR_OK : SDR_CANCEL;
        }
        default:
            assert(false);
            return SDR_OK;
    }
}

std::string CSystemUtilsAndroid::GetBasePath()
{
    if (m_basePath.empty())
    {
        const char* path = SDL_AndroidGetExternalStoragePath();
        m_basePath = path;
        m_basePath += "/";
    }
    return m_basePath;
}

std::string CSystemUtilsAndroid::GetDataPath()
{
    return GetBasePath() + "data";
}

std::string CSystemUtilsAndroid::GetLangPath()
{
    return GetBasePath() + "lang";
}

std::string CSystemUtilsAndroid::GetSaveDir()
{
    return GetBasePath() + "saves";
}