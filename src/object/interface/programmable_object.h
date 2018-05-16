/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2018, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include "object/object_interface_type.h"

#include <memory>
#include <string>
#include <vector>

struct Program;

/**
 * \class CProgrammableObject
 * \brief Interface for programmable objects
 *
 * Programmable objects can be programmed in CBOT
 */
class CProgrammableObject
{
public:
    explicit CProgrammableObject(ObjectInterfaceTypes& types)
    {
        types[static_cast<int>(ObjectInterfaceType::Programmable)] = true;
    }
    virtual ~CProgrammableObject()
    {}

    //! Start a program
    virtual void RunProgram(Program* program) = 0;
    //! Stop currently running program
    virtual void StopProgram() = 0;
    //! Returns the currently running program, or nullptr if no program is running
    virtual Program* GetCurrentProgram() = 0;
    //! Check if a program is running
    virtual bool IsProgram() = 0;

    //! Save current execution status to file
    virtual bool WriteStack(FILE *file) = 0;
    //! Read current execution status from file
    virtual bool ReadStack(FILE *file) = 0;

    //! Start recording trace
    virtual void TraceRecordStart() = 0;
    //! Stop recording trace and generate CBot program
    virtual void TraceRecordStop() = 0;
    //! Returns true if trace recording is in progress
    virtual bool IsTraceRecord() = 0;

    //! Management of object "activity" (temporairly stops program execution, right now used only by Aliens in eggs)
    //@{
    virtual void SetActivity(bool bMode) = 0;
    virtual bool GetActivity() = 0;
    //@}

    //! Returns program cmdline values for an object
    virtual float GetCmdLine(unsigned int rank) = 0;
};
