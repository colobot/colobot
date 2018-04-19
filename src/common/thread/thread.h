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

#include "common/make_unique.h"

#include "common/thread/resource_owning_thread.h"

#include <functional>
#include <string>
#include <memory>

/**
 * \class CThread
 * \brief Wrapper for using SDL_thread with std::function
 */
class CThread
{
public:
    using ThreadFunctionPtr = std::function<void()>;

private:
    struct ThreadData
    {
        ThreadFunctionPtr func;
    };

public:
    CThread(ThreadFunctionPtr func, std::string name = "")
        : m_func(std::move(func))
        , m_name(name)
    {}

    void Start()
    {
        std::unique_ptr<ThreadData> data = MakeUnique<ThreadData>();
        data->func = m_func;
        m_thread = MakeUnique<CResourceOwningThread<ThreadData>>(Run, std::move(data), m_name);
        m_thread->Start();
    }

    void Join()
    {
        if (!m_thread) return;
        m_thread->Join();
    }

    CThread(const CThread&) = delete;
    CThread& operator=(const CThread&) = delete;

private:
    static void Run(std::unique_ptr<ThreadData> data)
    {
        data->func();
    }

    std::unique_ptr<CResourceOwningThread<ThreadData>> m_thread;
    ThreadFunctionPtr m_func;
    std::string m_name;
};
