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

#include "net/services/server.h"


#include "common/logger.h"

#include "net/connections/connserver.h"
#include "net/packets/objectparams.h"
#include "net/packets/packettype.h"

#include "object/objman.h"

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>


CServer::CServer() : CNetworkService(), m_acceptor(m_service, tcp::endpoint(tcp::v4(), PORT))
{
    StartAccept();
    StartIOThread();
    m_updateTimer = UPDATE_INTERVAL;
}

void CServer::Stop()
{
    Broadcast(PACKET_S_DISCONNECT);
    for(auto connection : m_connections)
    {
        connection->GetSocket().cancel();
    }
    StopIOThread();
}

void CServer::Broadcast(PacketType packet, std::string data)
{
    for(auto connection : m_connections)
    {
        connection->Send(packet, data);
    }
}

void CServer::StartAccept()
{
    CServerConnection* new_connection = new CServerConnection(m_acceptor.get_io_service(), this);
    
    m_acceptor.async_accept(new_connection->GetSocket(), boost::bind(&CServer::HandleAccept, this, new_connection, boost::asio::placeholders::error));
}

void CServer::HandleAccept(CServerConnection* new_connection, const boost::system::error_code& err)
{
    if(!err)
    {
        m_connections.push_back(new_connection);
        new_connection->Init();
    } else {
        CLogger::GetInstancePointer()->Warn("Failed to accept connection: %s\n", err.message().c_str());
    }
    StartAccept();
}

void CServer::ProcessEvent(Event& event)
{
    if(event.type == EVENT_FRAME)
    {
        m_updateTimer -= event.rTime;
        if(m_updateTimer <= 0.0f)
        {
            m_updateTimer += UPDATE_INTERVAL;
            SendObjectsUpdate();
        }
        
        for(auto connection : m_connections)
        {
            connection->EventUpdate();
        }
    }
}

void CServer::SendObjectsUpdate()
{
    std::ostringstream so;
    boost::archive::text_oarchive ar_out(so);
    
    std::vector<ObjectParams> objects;
    
    for(auto it : CObjectManager::GetInstancePointer()->GetAllObjects())
    {
        CObject* o = it.second;
        ObjectType type = o->GetType();
        //TODO: We need a function to tell us if this is a stationary object
        if(type == OBJECT_PLANT0 ||
            type == OBJECT_PLANT1 ||
            type == OBJECT_PLANT2 ||
            type == OBJECT_PLANT3 ||
            type == OBJECT_PLANT4 ||
            type == OBJECT_PLANT5 ||
            type == OBJECT_PLANT6 ||
            type == OBJECT_PLANT7 ||
            type == OBJECT_PLANT8 ||
            type == OBJECT_PLANT9 ||
            type == OBJECT_PLANT10 ||
            type == OBJECT_PLANT11 ||
            type == OBJECT_PLANT12 ||
            type == OBJECT_PLANT13 ||
            type == OBJECT_PLANT14 ||
            type == OBJECT_PLANT15 ||
            type == OBJECT_PLANT16 ||
            type == OBJECT_PLANT17 ||
            type == OBJECT_PLANT18 ||
            type == OBJECT_PLANT19 ||
            type == OBJECT_TREE0 ||
            type == OBJECT_TREE1 ||
            type == OBJECT_TREE2 ||
            type == OBJECT_TREE3 ||
            type == OBJECT_TREE4 ||
            type == OBJECT_TREE5 ||
            type == OBJECT_BARRIER0 ||
            type == OBJECT_BARRIER1 ||
            type == OBJECT_BARRIER2 ||
            type == OBJECT_BARRIER3 ||
            type == OBJECT_RUINmobilew1 ||
            type == OBJECT_RUINmobilew2 ||
            type == OBJECT_RUINmobilet1 ||
            type == OBJECT_RUINmobilet2 ||
            type == OBJECT_RUINmobiler1 ||
            type == OBJECT_RUINmobiler2 ||
            type == OBJECT_RUINfactory ||
            type == OBJECT_RUINdoor ||
            type == OBJECT_RUINsupport ||
            type == OBJECT_RUINradar ||
            type == OBJECT_RUINconvert ||
            type == OBJECT_RUINbase ||
            type == OBJECT_RUINhead ||
            type == OBJECT_TEEN0 ||
            type == OBJECT_TEEN1 ||
            type == OBJECT_TEEN2 ||
            type == OBJECT_TEEN3 ||
            type == OBJECT_TEEN4 ||
            type == OBJECT_TEEN5 ||
            type == OBJECT_TEEN6 ||
            type == OBJECT_TEEN7 ||
            type == OBJECT_TEEN8 ||
            type == OBJECT_TEEN9 ||
            type == OBJECT_TEEN10 ||
            type == OBJECT_TEEN11 ||
            type == OBJECT_TEEN12 ||
            type == OBJECT_TEEN13 ||
            type == OBJECT_TEEN14 ||
            type == OBJECT_TEEN15 ||
            type == OBJECT_TEEN16 ||
            type == OBJECT_TEEN17 ||
            type == OBJECT_TEEN18 ||
            type == OBJECT_TEEN19 ||
            type == OBJECT_TEEN20 ||
            type == OBJECT_TEEN21 ||
            type == OBJECT_TEEN22 ||
            type == OBJECT_TEEN23 ||
            type == OBJECT_TEEN24 ||
            type == OBJECT_TEEN25 ||
            type == OBJECT_TEEN26 ||
            type == OBJECT_TEEN27 ||
            type == OBJECT_TEEN28 ||
            type == OBJECT_TEEN29 ||
            type == OBJECT_TEEN30 ||
            type == OBJECT_TEEN31 ||
            type == OBJECT_TEEN32 ||
            type == OBJECT_TEEN33 ||
            type == OBJECT_TEEN34 ||
            type == OBJECT_TEEN35 ||
            type == OBJECT_TEEN36 ||
            type == OBJECT_TEEN37 ||
            type == OBJECT_TEEN38 ||
            type == OBJECT_TEEN39 ||
            type == OBJECT_TEEN40 ||
            type == OBJECT_TEEN41 ||
            type == OBJECT_TEEN42 ||
            type == OBJECT_TEEN43 ||
            type == OBJECT_TEEN44 ||
            type == OBJECT_QUARTZ0 ||
            type == OBJECT_QUARTZ1 ||
            type == OBJECT_QUARTZ2 ||
            type == OBJECT_QUARTZ3 ||
            type == OBJECT_ROOT0 ||
            type == OBJECT_ROOT1 ||
            type == OBJECT_ROOT2 ||
            type == OBJECT_ROOT3 ||
            type == OBJECT_ROOT4 ||
            type == OBJECT_ROOT5 ||
            type == OBJECT_MUSHROOM1 ||
            type == OBJECT_MUSHROOM2 ||
            type == OBJECT_APOLLO1 ||
            type == OBJECT_APOLLO2 ||
            type == OBJECT_APOLLO3 ||
            type == OBJECT_APOLLO4 ||
            type == OBJECT_APOLLO5 ||
            type == OBJECT_HOME1) continue;
        ObjectParams op = ObjectParams::FromObject(o);
        objects.push_back(op);
    }
    ar_out & objects;
    Broadcast(PACKET_S_OBJECT_UPDATE, so.str());
}
