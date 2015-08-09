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

#include "common/thread/sdl_cond_wrapper.h"
#include "common/thread/sdl_mutex_wrapper.h"
#include "common/thread/sdl_thread_wrapper.h"

#include <SDL_thread.h>

#include <memory>

/**
 * \class CResourceOwningThread
 * \brief Wrapper around SDL thread allowing passing of resources in safe manner
 *
 * This class is a workaround for passing ownership of resources in a safe
 * manner to newly created threads. It takes a pointer to a function to call
 * in new thread and a unique_ptr to resource which is to be passed to the new thread.
 *
 * This is how it works:
 *  - in main thread: create a new thread passing the resource to it,
 *  - in main thread: wait for synchronization signal that the ownership was passed,
 *  - in new thread: acquire the resource
 *  - in new thread: signal back to main thread that the resource was acquired,
 *  - in main thread: clean up and exit
 *  - in new thread: run the specified function with the acquired resource.
 *
 * It's a bit complicated, but that's the safe (thread-safe and exception-safe)
 * way of doing this.
 */
template<typename Resource>
class CResourceOwningThread
{
public:
    using ResourceUPtr = std::unique_ptr<Resource>;
    using ThreadFunctionPtr = void(*)(ResourceUPtr);

    CResourceOwningThread(ThreadFunctionPtr threadFunction, ResourceUPtr resource)
        : m_threadFunction(threadFunction),
          m_resource(std::move(resource))
    {}

    void Start()
    {
        CSDLMutexWrapper mutex;
        CSDLCondWrapper cond;
        bool condition = false;

        SDL_LockMutex(*mutex);

        CSDLThreadWrapper thread([&]() {
            SDL_LockMutex(*mutex);

            ResourceUPtr resource = std::move(m_resource);

            condition = true;
            SDL_CondSignal(*cond);
            SDL_UnlockMutex(*mutex);

            m_threadFunction(std::move(resource));
        });
        thread.Start();

        while (!condition)
        {
            SDL_CondWait(*cond, *mutex);
        }

        SDL_UnlockMutex(*mutex);
    }

private:
    ThreadFunctionPtr m_threadFunction;
    ResourceUPtr m_resource;
};
