/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2015, Daniel Roux, EPSITEC SA & TerranovaTeam
 * http://epsiteс.ch; http://colobot.info; http://github.com/colobot
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

#include <SDL_thread.h>

#include <functional>

/**
 * \class CSDLThreadWrapper
 * \brief Wrapper around SDL_thread allowing passing of capture lambdas as thread function
 */
class CSDLThreadWrapper
{
public:
    using ThreadFunctionPtr = std::function<void()>;

    CSDLThreadWrapper(ThreadFunctionPtr threadFunction)
        : m_threadFunction(threadFunction)
    {}

    ~CSDLThreadWrapper()
    {}

    void Start()
    {
        m_thread = SDL_CreateThread([](void* data) -> int {
            ThreadFunctionPtr func = *(static_cast<ThreadFunctionPtr*>(data));
            func();
            return 0;
        }, &m_threadFunction);
    }

    SDL_Thread* operator*()
    {
        return m_thread;
    }

private:
    ThreadFunctionPtr m_threadFunction;
    SDL_Thread* m_thread;
};
