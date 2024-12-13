/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2023, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

/**
 * \class CWorkerThread
 * \brief Thread that runs functions, one at a time
 */
class CWorkerThread
{
public:
    using ThreadFunctionPtr = std::function<void()>;

public:
    CWorkerThread() : m_thread{&CWorkerThread::Run, this} {}

    ~CWorkerThread()
    {
        {
            std::lock_guard<std::mutex> lock{m_mutex};
            m_running = false;
            m_cond.notify_one();
        }
        m_thread.join();
    }

    void Start(ThreadFunctionPtr&& func)
    {
        std::lock_guard<std::mutex> lock{m_mutex};
        m_queue.push(func);
        m_cond.notify_one();
    }

    CWorkerThread(const CWorkerThread&) = delete;
    CWorkerThread& operator=(const CWorkerThread&) = delete;

private:
    void Run()
    {
        auto lock = std::unique_lock<std::mutex>(m_mutex);
        while (true)
        {
            m_cond.wait(lock, [&]() { return !m_running || !m_queue.empty(); });
            if (!m_running) break;

            ThreadFunctionPtr func = std::move(m_queue.front());
            m_queue.pop();
            func();
        }
    }

    std::mutex m_mutex;
    std::condition_variable m_cond;
    bool m_running = true;
    std::queue<ThreadFunctionPtr> m_queue;
    std::thread m_thread;
};
