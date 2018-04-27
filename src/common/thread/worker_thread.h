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

#include "common/thread/sdl_cond_wrapper.h"
#include "common/thread/sdl_mutex_wrapper.h"
#include "common/thread/thread.h"

#include <functional>
#include <string>
#include <queue>

/**
 * \class CWorkerThread
 * \brief Thread that runs functions, one at a time
 */
class CWorkerThread
{
public:
    using ThreadFunctionPtr = std::function<void()>;

public:
    CWorkerThread(std::string name = "")
        : m_thread(std::bind(&CWorkerThread::Run, this), name)
    {
        m_thread.Start();
    }

    ~CWorkerThread()
    {
        m_mutex.Lock();
        m_running = false;
        m_cond.Signal();
        m_mutex.Unlock();
        m_thread.Join();
    }

    void Start(ThreadFunctionPtr func)
    {
        m_mutex.Lock();
        m_queue.push(func);
        m_cond.Signal();
        m_mutex.Unlock();
    }

    CWorkerThread(const CWorkerThread&) = delete;
    CWorkerThread& operator=(const CWorkerThread&) = delete;

private:
    void Run()
    {
        m_mutex.Lock();
        while (true)
        {
            while (m_queue.empty() && m_running)
            {
                m_cond.Wait(*m_mutex);
            }
            if (!m_running) break;

            ThreadFunctionPtr func = m_queue.front();
            m_queue.pop();
            func();
        }
        m_mutex.Unlock();
    }

    CThread m_thread;
    CSDLMutexWrapper m_mutex;
    CSDLCondWrapper m_cond;
    bool m_running = true;
    std::queue<ThreadFunctionPtr> m_queue;
};
