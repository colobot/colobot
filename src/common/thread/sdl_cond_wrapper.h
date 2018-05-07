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

#include "common/thread/sdl_mutex_wrapper.h"

#include <SDL_thread.h>

/**
 * \class CSDLCondWrapper
 * \brief Wrapper for safe creation/deletion of SDL_cond
 */
class CSDLCondWrapper
{
public:
    CSDLCondWrapper()
        : m_cond(SDL_CreateCond())
    {}

    ~CSDLCondWrapper()
    {
        SDL_DestroyCond(m_cond);
    }

    CSDLCondWrapper(const CSDLCondWrapper&) = delete;
    CSDLCondWrapper& operator=(const CSDLCondWrapper&) = delete;

    SDL_cond* operator*()
    {
        return m_cond;
    }

    void Signal()
    {
        SDL_CondSignal(m_cond);
    }

    void Wait(SDL_mutex* mutex)
    {
        SDL_CondWait(m_cond, mutex);
    }

private:
    SDL_cond* m_cond;
};
