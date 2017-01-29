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

#include "app/app.h"
#include "app/controller.h"

#include <boost/function.hpp>


class CApplicationMock;
class CRecord;


typedef boost::function<void(CApplicationMock*)> AssertionFunc;


class CControllerMock  : public CController
{
public:
	void                    Initialize(CApplication* app) { m_app = app; }

protected:
	CApplication*           m_app;
};


class CControllerRecord : public CControllerMock
{
public:
    CControllerRecord(CRecord* record);

    void                    ProcessEvent(Event &event) override;

protected:
    CRecord*                m_record;
};


class CControllerReplay : public CControllerMock
{
public:
    void                    ProcessEvent(Event &event) override;
    //! Set function that tests correctness of code, invoked while every EVENT_FRAME
    void                    SetAssertion(AssertionFunc assertion) { m_assertion = assertion; }

protected:
    AssertionFunc 			m_assertion;
};
