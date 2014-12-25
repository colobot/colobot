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

#include "net/packets/packettype.h"

#include <boost/asio.hpp>

#include <map>

class CServerConnection;
class CController;

class CServer : public CNetworkService {
friend class CServerConnection;
friend class CController;
const float UPDATE_INTERVAL = 1.0f/30.0f;
public:
    CServer();
    void Stop();
    
    void ProcessEvent(Event& event);
    
    void Broadcast(PacketType packet, std::string data = "");

private:
    void StartAccept();
    void HandleAccept(CServerConnection* new_connection, const boost::system::error_code& err);
    
    void SendObjectsUpdate();
    
private:
    tcp::acceptor m_acceptor;
    std::vector<CServerConnection*> m_connections;
    float m_updateTimer;
};
