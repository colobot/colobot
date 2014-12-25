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

#include "net/connections/connclient.h"

#include "app/controller.h"

#include "common/config.h"
#include "common/logger.h"

#include "net/packets/packettype.h"
#include "net/services/client.h"

#include "object/objman.h"
#include "object/robotmain.h"

#include "ui/maindialog.h"

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/vector.hpp>

CClientConnection::CClientConnection(boost::asio::io_service& service, CClient* client) : CConnection(service), m_client(client)
{
}

void CClientConnection::Start()
{
}

void CClientConnection::Receive(PacketType packet, std::string data)
{
    switch(packet) {
        case PACKET_S_HANDSHAKE:
            PacketHandshake(data);
            break;
            
        case PACKET_S_LEVEL_DATA:
            PacketLevelData(data);
            break;
        
        case PACKET_S_OBJECT_UPDATE:
            PacketObjectUpdate(data);
            break;
            
        case PACKET_S_DISCONNECT:
            PacketDisconnect(data);
            break;
            
        default:
            CLogger::GetInstancePointer()->Error("Received bad packet (unknown type %d), disconnecting...\n", packet);
            // TODO: Drop the connection
            break;
    }
}

void CClientConnection::PacketHandshake(std::string data)
{
    CLogger::GetInstancePointer()->Debug("Received server info packet\n");
    
    std::istringstream si(data);
    boost::archive::text_iarchive ar(si);
    
    std::string serverName; ar & serverName;
    std::string serverVersion; ar & serverVersion;
    if(serverVersion != COLOBOT_FULLNAME)
        CLogger::GetInstancePointer()->Warn("Version mismatch! %s != %s\n", COLOBOT_FULLNAME, serverVersion.c_str());
    ar & m_mapName;
    ar & m_mapRank;
    CLogger::GetInstancePointer()->Info("Connecting to server '%s', running map '%s%d'\n", serverName.c_str(), m_mapName.c_str(), m_mapRank);
    
    
    std::ostringstream so;
    boost::archive::text_oarchive ar_out(so);
    
    std::string playerName = CRobotMain::GetInstancePointer()->GetGamerName(); ar_out & playerName;
    
    Send(PACKET_C_CONNECT, so.str());
}

void CClientConnection::PacketLevelData(std::string data)
{
    CLogger::GetInstancePointer()->Debug("Received level data packet\n");
    
    std::istringstream si(data);
    boost::archive::text_iarchive ar(si);
    
    ar & m_levelData;
    
    // Load the map now
    CController::GetInstancePointer()->GetMainDialog()->SetSceneName(m_mapName.c_str());
    CController::GetInstancePointer()->GetMainDialog()->SetSceneRank(m_mapRank);
    CController::GetInstancePointer()->GetRobotMain()->ChangePhase(PHASE_LOADING);
}

const std::vector<ObjectParams>& CClientConnection::GetLevelData()
{
    return m_levelData;
}

void CClientConnection::PacketObjectUpdate(std::string data)
{
    //CLogger::GetInstancePointer()->Debug("Received object update packet\n");
    
    std::istringstream si(data);
    boost::archive::text_iarchive ar(si);
    
    std::vector<ObjectParams> objects;
    ar & objects;
    
    for(ObjectParams op : objects)
    {
        CObject* obj = CObjectManager::GetInstancePointer()->GetObjectById(op.id);
        if(obj == nullptr)
        {
            CLogger::GetInstancePointer()->Error("Update about unknown object %d!\n", op.id);
            continue;
        }
        CObject* sel = CRobotMain::GetInstancePointer()->GetSelect();
        if(sel != nullptr && (obj == sel || obj == sel->GetPower() || obj == sel->GetFret())) continue;
        if(obj == m_client->m_lastLoad || obj == m_client->m_lastPower) continue;
        op.Apply();
    }
}

void CClientConnection::PacketDisconnect(std::string data)
{
    CLogger::GetInstancePointer()->Info("The server kicked us out :(\n");
}