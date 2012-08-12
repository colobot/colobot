// * This file is part of the COLOBOT source code
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

// buffer.h

#pragma once

#include <string>

#include <AL/alut.h>

#include <sound/sound.h>
#include <common/logger.h>

#include "check.h"

class Buffer
{
    public:
        Buffer();
        ~Buffer();

        bool LoadFromFile(std::string, Sound);
        bool IsLoaded();

        Sound GetSoundType();
        ALuint GetBuffer();
        float GetDuration();

    private:
        ALuint mBuffer;
        Sound mSound;
        bool mLoaded;
        float mDuration;
};
