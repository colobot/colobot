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

#include "src/net/packets/packettype.h"

#include <queue>
#include <boost/asio.hpp>
using boost::asio::ip::tcp;

class CPacketData;

struct ReceivedPacket
{
    PacketType packet;
    std::string data;
    
    ReceivedPacket(PacketType p, std::string d = "") : packet(p), data(d) {};
};

class CConnection
{
public:
    CConnection(boost::asio::io_service& service);
    tcp::socket& GetSocket();
    
    void Init();

    virtual void Start() = 0;
    virtual void Receive(PacketType packet, std::string data = "") = 0;
    void Send(PacketType packet, std::string data = "");
    
    void EventUpdate();

private:
    void HandleWriteFinished(char* buffer);
    
    void StartReceiveHeader();
    void HandleReceiveHeader(char* buffer, const boost::system::error_code& err);
    void StartReceiveContent(unsigned int len);
    void HandleReceiveContent(char* buffer, unsigned int len, const boost::system::error_code& err);

    tcp::socket m_socket;
    PacketHeader m_packetHeader;
    std::queue<ReceivedPacket> m_recvQueue;
};
