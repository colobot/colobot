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

#include <stdint.h>

//! Packet magic identifier - try to guess what this value means ;)
static const uint32_t PACKET_MAGIC = 0x444c4f47;

enum PacketType {
    PACKET_S_HANDSHAKE,         // [S] Hello, this is server '%s' and we're running map '%s' on version '%s'. Do you want to connect? (disconnect now if you only wanted info to display in server list)
    PACKET_C_CONNECT,           // [C] Hi, I'm '%s' and I'd like to connect. This (is|isn't) a local game.
    PACKET_S_LEVEL_DATA,        // [S] (a set of CreateObject instructions)
    PACKET_C_MAP_LOADED,        // (*) [C] OK, level is loaded
    PACKET_S_JOINED,            // (*) [S] Welcome on the server! (broadcast to all players)
    
    PACKET_C_OBJECT_UPDATE,     // [C] I've updated an object
    PACKET_S_OBJECT_UPDATE,     // [S] Object %d has been updated. It's now at position ..., angle ..., and it's velocity is ...
    
    PACKET_C_TASK_START,        // (*) [C] Start task ...
    PACKET_S_TASK_START,        // (*) [S] Task ... has been started on object ...
    PACKET_S_TASK_UPDATE,       // (*) [S] Task has been updated
    PACKET_C_TASK_ABORT,        // (*) [C] Abort task ...
    PACKET_S_TASK_ABORT,        // (*) [S] Task ... has been aborted,
    
    PACKET_C_CBOT_SCRIPT,       // (*) [C] I've edited the script
    PACKET_S_CBOT_SCRIPT,       // (*) [S] The script has been edited
    PACKET_C_CBOT_STARTSTOP,    // (*) [C] Start/stop the script (see params)
    PACKET_S_CBOT_STARTSTOP,    // (*) [S] The script has been started/stopped (updates blinking lights etc., DOESN'T start the script clientside)
    
    PACKET_C_DISCONNECT,        // [C] I'm disconnecting, bye
    PACKET_S_DISCONNECT,        // [S] You are a cheater, I'm disconnecting you! (or the server is just shutting down... :P)
    PACKET_S_DISCONNECT_NOTIFY, // [S] %s has disconnected
};

struct PacketHeader {
    //! Packet magic number
    uint32_t magic;
    //! Packet type
    PacketType type;
    //! Size of additional packet data (after the header)
    uint32_t len;
} __attribute__((__packed__));