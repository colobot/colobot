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

#include "net/connections/connserver.h"

#include "app/controller.h"

#include "common/config.h"
#include "common/logger.h"

#include "net/packets/objectparams.h"
#include "net/packets/packettype.h"
#include "net/services/server.h"

#include "object/object.h"
#include "object/objman.h"

#include "ui/maindialog.h"

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>

CServerConnection::CServerConnection(boost::asio::io_service& service, CServer* server) : CConnection(service), m_server(server)
{
}

void CServerConnection::Start()
{
    CLogger::GetInstancePointer()->Debug("New connection...\n");
    
    std::ostringstream s;
    boost::archive::text_oarchive ar(s);
    std::string serverName = "Test server"; ar & serverName;
    std::string serverVersion = COLOBOT_FULLNAME; ar & serverVersion;
    std::string mapName = CController::GetInstancePointer()->GetMainDialog()->GetSceneName(); ar & mapName;
    int mapRank = CController::GetInstancePointer()->GetMainDialog()->GetSceneRank(); ar & mapRank;
    Send(PACKET_S_HANDSHAKE, s.str());
}

void CServerConnection::Receive(PacketType packet, std::string data)
{
    switch(packet) {
        case PACKET_C_CONNECT:
            PacketConnect(data);
            break;
            
        case PACKET_C_OBJECT_UPDATE:
            PacketObjectUpdate(data);
            break;
            
        case PACKET_C_DISCONNECT:
            PacketDisconnect(data);
            break;
            
        default:
            CLogger::GetInstancePointer()->Error("Received bad packet (unknown type %d), disconnecting...\n", packet);
            // TODO: Drop the connection
            break;
    }
}

void CServerConnection::PacketConnect(std::string data)
{
    std::istringstream si(data);
    boost::archive::text_iarchive ar(si);
    
    ar & m_playerName;
    
    CLogger::GetInstancePointer()->Info("Player '%s' is connecting...\n", m_playerName.c_str());
    
    
    std::ostringstream so;
    boost::archive::text_oarchive ar_out(so);
    
    std::vector<ObjectParams> objects;
    for(auto it : CObjectManager::GetInstancePointer()->GetAllObjects())
    {
        CObject* o = it.second;
        ObjectParams op = ObjectParams::FromObject(o);
        objects.push_back(op);
    }
    ar_out & objects;
    Send(PACKET_S_LEVEL_DATA, so.str());
}

void CServerConnection::PacketObjectUpdate(std::string data)
{
    std::istringstream si(data);
    boost::archive::text_iarchive ar(si);
    std::vector<ObjectParams> objects;
    ar & objects;
    for(ObjectParams op : objects)
    {
        op.Apply();
    }
}

void CServerConnection::PacketDisconnect(std::string data)
{
    CLogger::GetInstancePointer()->Info("'%s' disconnected\n", m_playerName.c_str());
    m_server->Broadcast(PACKET_S_DISCONNECT_NOTIFY);
    //TODO: remove from CServer's list
}