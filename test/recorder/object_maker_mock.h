/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2016, Daniel Roux, EPSITEC SA & TerranovaTeam
 * http://epsitec.ch; http://colobot.info; http://github.com/colobot
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


#include "common/object_maker.h"

#include "test/recorder/recorder.h"


class CObjectMakerRecord : public CObjectMaker
{
public:
    CObjectMakerRecord(CRecord* record);
    
    std::unique_ptr<CEventQueue> 	MakeEventQueue() override;
    std::unique_ptr<CController> 	MakeController() override;
    
protected:
    CRecord* m_record;
};


class CObjectMakerReplay : public CObjectMaker
{
public:
    CObjectMakerReplay(CRecord* record);
    
    std::unique_ptr<CEventQueue> 	MakeEventQueue() override;
    std::unique_ptr<CController> 	MakeController() override;
    std::unique_ptr<CInput> 		MakeInput() override;
    
protected:
    CRecord* m_record;
};
    
 
 
