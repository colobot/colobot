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

#include "net/connections/conn.h"

#include "common/event.h"
#include "common/logger.h"

#include "net/packets/packettype.h"

#include <boost/bind.hpp>

CConnection::CConnection(boost::asio::io_service& service) : m_socket(service)
{
}

tcp::socket& CConnection::GetSocket()
{
    return m_socket;
}

void CConnection::Init()
{
    StartReceiveHeader();
    Start();
}

void CConnection::Send(PacketType packet, std::string data)
{
    char* buf = new char[sizeof(PacketHeader)+data.length()];
    PacketHeader* p = reinterpret_cast<PacketHeader*>(buf);
    p->magic = PACKET_MAGIC;
    p->type = packet;
    p->len = data.length();
    memcpy(buf+sizeof(PacketHeader), data.c_str(), data.length());
    
    boost::asio::async_write(m_socket, boost::asio::buffer(buf, sizeof(PacketHeader)+data.length()), boost::bind(&CConnection::HandleWriteFinished, this, buf));
}


void CConnection::HandleWriteFinished(char* buffer)
{
    delete[] buffer;
}

void CConnection::StartReceiveHeader()
{
    char* buf = new char[sizeof(PacketHeader)];
    boost::asio::async_read(m_socket, boost::asio::buffer(buf, sizeof(PacketHeader)), boost::bind(&CConnection::HandleReceiveHeader, this, buf, boost::asio::placeholders::error));
}

void CConnection::HandleReceiveHeader(char* buffer, const boost::system::error_code& err)
{
    if(!err)
    {
        PacketHeader* p = reinterpret_cast<PacketHeader*>(buffer);
        if(p->magic != PACKET_MAGIC)
        {
            CLogger::GetInstancePointer()->Error("Received bad packet (invalid magic), disconnecting...\n");
            // TODO: Drop the connection
            // For now just keep going:
            StartReceiveHeader();
        } else {
            memcpy(&m_packetHeader, p, sizeof(PacketHeader));
            if(m_packetHeader.len > 0)
            {
                StartReceiveContent(m_packetHeader.len);
            } else {
                // This makes sure Receive() is called on the main thread
                m_recvQueue.push(ReceivedPacket(m_packetHeader.type));
                StartReceiveHeader();
            }
        }
    } else {
        CLogger::GetInstancePointer()->Error("Socket read error: %s\n", err.message().c_str());
    }
    delete[] buffer;
}

void CConnection::StartReceiveContent(unsigned int len)
{
    char* buf = new char[len];
    m_socket.async_receive(boost::asio::buffer(buf, len), boost::bind(&CConnection::HandleReceiveContent, this, buf, len, boost::asio::placeholders::error));
}

void CConnection::HandleReceiveContent(char* buffer, unsigned int len, const boost::system::error_code& err)
{
    if(!err)
    {
        // This makes sure Receive() is called on the main thread
        m_recvQueue.push(ReceivedPacket(m_packetHeader.type, std::string(buffer, len)));
        StartReceiveHeader();
    } else {
        CLogger::GetInstancePointer()->Error("Socket read error: %s\n", err.message().c_str());
    }
    delete[] buffer;
}

void CConnection::EventUpdate()
{
    while(!m_recvQueue.empty())
    {
        ReceivedPacket p = m_recvQueue.front();
        Receive(p.packet, p.data);
        m_recvQueue.pop();
    }
}