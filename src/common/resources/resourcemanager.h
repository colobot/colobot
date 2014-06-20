// * This file is part of the COLOBOT source code
// * Copyright (C) 2014 Polish Portal of Colobot (PPC)
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

#pragma once

#include <string>
#include <SDL.h>

class CResourceManager
{
public:
    CResourceManager(const char *argv0);
    ~CResourceManager();

    static bool AddLocation(const std::string &location, bool prepend = true);
    static bool RemoveLocation(const std::string &location);
    static bool SetSaveLocation(const std::string &location);
    static std::string GetLanguageLocation();
    static SDL_RWops* GetSDLFileHandler(const std::string &filename);

private:
    static int SDLSeek(SDL_RWops *context, int offset, int whence);
    static int SDLRead(SDL_RWops *context, void *ptr, int size, int maxnum);
    static int SDLWrite(SDL_RWops *context, const void *ptr, int size, int num);
    static int SDLClose(SDL_RWops *context);
    static bool CheckSDLContext(SDL_RWops *context);
};
