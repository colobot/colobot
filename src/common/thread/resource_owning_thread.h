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

#include "common/thread/sdl_cond_wrapper.h"
#include "common/thread/sdl_mutex_wrapper.h"

#include <SDL_thread.h>

#include <memory>
#include <string>

/**
 * \class CResourceOwningThread
 * \brief Wrapper around SDL thread allowing passing of resources in safe manner
 *
 * This class is a workaround for passing ownership of resources in a safe
 * manner to newly created threads. It takes a pointer to a function to call
 * in new thread and a unique_ptr to resource which is to be passed to the new thread.
 *
 * This is how it works:
 *  - in main thread: create a new thread passing to it a special temporary context,
 *  - in main thread: wait for synchronization signal that the ownership was passed,
 *  - in new thread: acquire the resource from the context
 *  - in new thread: signal back to main thread that the resource was acquired,
 *  - in main thread: clean up temporary context and exit
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

    CResourceOwningThread(ThreadFunctionPtr threadFunction, ResourceUPtr resource, std::string name = "")
        : m_threadFunction(threadFunction),
          m_resource(std::move(resource)),
          m_name(name)
    {}

    ~CResourceOwningThread()
    {
        SDL_DetachThread(m_thread);
    }

    void Start()
    {
        CSDLMutexWrapper mutex;
        CSDLCondWrapper cond;
        bool condition = false;

        ThreadData data;
        data.resource = std::move(m_resource);
        data.threadFunction = m_threadFunction;
        data.mutex = &mutex;
        data.cond = &cond;
        data.condition = &condition;

        SDL_LockMutex(*mutex);

        m_thread = SDL_CreateThread(Run, !m_name.empty() ? m_name.c_str() : nullptr, reinterpret_cast<void*>(&data));

        while (!condition)
        {
            SDL_CondWait(*cond, *mutex);
        }

        SDL_UnlockMutex(*mutex);
    }

    void Join()
    {
        if (m_thread == nullptr) return;
        SDL_WaitThread(m_thread, nullptr);
        m_thread = nullptr;
    }

private:
    static int Run(void* data)
    {
        ThreadFunctionPtr threadFunction = nullptr;
        ResourceUPtr resource;

        ThreadData* threadData = reinterpret_cast<ThreadData*>(data);
        SDL_LockMutex(**threadData->mutex);

        threadFunction = threadData->threadFunction;
        resource = std::move(threadData->resource);

        *threadData->condition = true;
        SDL_CondSignal(**threadData->cond);
        SDL_UnlockMutex(**threadData->mutex);

        threadFunction(std::move(resource));
        return 0;
    }

private:
    struct ThreadData
    {
        ResourceUPtr resource;
        CSDLMutexWrapper* mutex = nullptr;
        CSDLCondWrapper* cond = nullptr;
        bool* condition = nullptr;
        ThreadFunctionPtr threadFunction = nullptr;
    };

    ThreadFunctionPtr m_threadFunction;
    ResourceUPtr m_resource;
    std::string m_name;
    SDL_Thread* m_thread = nullptr;
};
