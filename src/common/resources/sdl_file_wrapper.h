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

#include <string>

#include <SDL.h>

class CSDLFileWrapper
{
public:
    CSDLFileWrapper(const std::string& filename);
    ~CSDLFileWrapper();

    CSDLFileWrapper(const CSDLFileWrapper&) = delete;
    CSDLFileWrapper& operator=(const CSDLFileWrapper&) = delete;

    bool IsOpen() const;
    SDL_RWops* GetHandler();

private:
    static Sint64 SDLSeek(SDL_RWops *context, Sint64 offset, int whence);
    static Sint64 SDLSize(SDL_RWops *context);
    static std::size_t SDLRead(SDL_RWops *context, void *ptr, std::size_t size, std::size_t maxnum);
    static std::size_t SDLWrite(SDL_RWops *context, const void *ptr, std::size_t size, std::size_t num);
    static int SDLClose(SDL_RWops *context, bool freeRW);
    static int SDLCloseWithoutFreeRW(SDL_RWops *context);
    static int SDLCloseWithFreeRW(SDL_RWops *context);
    static bool CheckSDLContext(SDL_RWops *context);

private:
    SDL_RWops* m_rwops;
};
