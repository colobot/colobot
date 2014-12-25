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

#include "net/services/client.h"


#include "common/logger.h"

#include "net/connections/connclient.h"

#include "object/robotmain.h"

#include <boost/lexical_cast.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/vector.hpp>


CClient::CClient(std::string server) : CNetworkService(), m_resolver(m_service), m_connection(nullptr)
{
    Connect(server);
    StartIOThread();
    m_updateTimer = UPDATE_INTERVAL;
    m_lastPower = nullptr;
    m_lastLoad = nullptr;
}

void CClient::Stop()
{
    m_connection->Send(PACKET_C_DISCONNECT);
    m_connection->GetSocket().cancel();
    StopIOThread();
}

CClientConnection* CClient::GetConnection()
{
    return m_connection;
}

void CClient::Connect(std::string server)
{
    tcp::resolver::query query(server, boost::lexical_cast<std::string>(PORT));
    m_resolver.async_resolve(query, boost::bind(&CClient::HandleResolve, this, boost::asio::placeholders::error, boost::asio::placeholders::iterator));
}

void CClient::HandleResolve(const boost::system::error_code& err, tcp::resolver::iterator endpoint_iterator)
{
    if (!err)
    {
        m_connection = new CClientConnection(m_resolver.get_io_service(), this);
        // Try to connect to first endpoint in the list
        tcp::endpoint endpoint = *endpoint_iterator;
        m_connection->GetSocket().async_connect(endpoint, boost::bind(&CClient::HandleConnect, this, boost::asio::placeholders::error, ++endpoint_iterator));
    } else {
        CLogger::GetInstancePointer()->Error("Failed to resolve hostname: %s\n", err.message().c_str());
    }
}

void CClient::HandleConnect(const boost::system::error_code& err, tcp::resolver::iterator endpoint_iterator)
{
    if (!err)
    {
        // The connection was successful
        m_connection->Init();
    }
    else if (endpoint_iterator != tcp::resolver::iterator())
    {
        // The connection failed. Try the next endpoint in the list
        m_connection->GetSocket().close();
        tcp::endpoint endpoint = *endpoint_iterator;
        m_connection->GetSocket().async_connect(endpoint, boost::bind(&CClient::HandleConnect, this, boost::asio::placeholders::error, ++endpoint_iterator));
    } else {
        // The connection failed on all endpoints, report an error
        CLogger::GetInstancePointer()->Error("Failed to connect: %s\n", err.message().c_str());
    }
}

void CClient::ProcessEvent(Event& event)
{
    if(event.type == EVENT_FRAME)
    {
        m_updateTimer -= event.rTime;
        if(m_updateTimer <= 0.0f)
        {
            m_updateTimer += UPDATE_INTERVAL;
            SendObjectUpdate();
        }
        
        if(m_connection != nullptr)
        {
            m_connection->EventUpdate();
        }
    }
}

void CClient::SendObjectUpdate()
{
    if(m_connection == nullptr) return;
    std::ostringstream so;
    boost::archive::text_oarchive ar_out(so);
    std::vector<ObjectParams> objects;
    
    CObject* o = CRobotMain::GetInstancePointer()->GetSelect();
    if(o == nullptr) return;
    ObjectParams op = ObjectParams::FromObject(o);
    objects.push_back(op);
    
    CObject* power = o->GetPower();
    if(power != nullptr)
    {
        ObjectParams op_power = ObjectParams::FromObject(power);
        objects.push_back(op_power);
    }
    if(power != m_lastPower)
    {
        if(m_lastPower != nullptr)
        {
            ObjectParams op_power = ObjectParams::FromObject(m_lastPower);
            objects.push_back(op_power);
            if(m_lastPower->GetTruck() != nullptr && m_lastPower->GetTruck() != o) {
                ObjectParams op_truck = ObjectParams::FromObject(m_lastPower->GetTruck());
                objects.push_back(op_truck);
            }
        }
        m_lastPower = power;
    }
    
    CObject* load = o->GetFret();
    if(load != nullptr)
    {
        ObjectParams op_load = ObjectParams::FromObject(load);
        objects.push_back(op_load);
    }
    if(load != m_lastLoad)
    {
        if(m_lastLoad != nullptr)
        {
            ObjectParams op_load = ObjectParams::FromObject(m_lastLoad);
            objects.push_back(op_load);
            if(m_lastLoad->GetTruck() != nullptr && m_lastLoad->GetTruck() != o) {
                ObjectParams op_truck = ObjectParams::FromObject(m_lastPower->GetTruck());
                objects.push_back(op_truck);
            }
        }
        m_lastLoad = load;
    }
    
    ar_out & objects;
    
    m_connection->Send(PACKET_C_OBJECT_UPDATE, so.str());
}