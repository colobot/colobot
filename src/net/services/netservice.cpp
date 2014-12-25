/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2014, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include "net/services/netservice.h"

CNetworkService::CNetworkService()
{
    m_thread = nullptr;
}

CNetworkService::~CNetworkService()
{
    if(m_thread != nullptr) {
        StopIOThread();
    }
}

void CNetworkService::StartIOThread()
{
    assert(m_thread == nullptr);
    m_thread = new boost::thread(boost::bind(&boost::asio::io_service::run, &m_service));
}

void CNetworkService::StopIOThread()
{
    m_service.stop();
    m_thread->join(); //TODO: try to remove this
    m_thread = nullptr;
}