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

#pragma once

#include "net/services/netservice.h"

#include <boost/asio.hpp>

class CClientConnection;
class CController;
class CObject;

class CClient : public CNetworkService {
friend class CClientConnection;
friend class CController;
const float UPDATE_INTERVAL = 1.0f/30.0f;
public:
    CClient(std::string server);
    void Stop();
    
    CClientConnection* GetConnection();
    
    void ProcessEvent(Event& event);
    
private:
    void Connect(std::string server);
    void HandleResolve(const boost::system::error_code& err, tcp::resolver::iterator endpoint_iterator);
    void HandleConnect(const boost::system::error_code& err, tcp::resolver::iterator endpoint_iterator);
    
    void SendObjectUpdate();
    
private:
    tcp::resolver m_resolver;
    CClientConnection* m_connection;
    float m_updateTimer;
    
    CObject* m_lastPower;
    CObject* m_lastLoad;
};
