/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2015, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include "object/object_interface_type.h"

#include <memory>
#include <string>
#include <vector>

class CScript;

struct Program
{
    std::unique_ptr<CScript> script;
    std::string filename;
    bool        readOnly = false;
    bool        runnable = true;
};

/**
 * \class CProgramStorageObject
 * \brief Interface for objects that store CBOT programs
 */
class CProgramStorageObject
{
public:
    explicit CProgramStorageObject(ObjectInterfaceTypes& types)
    {
        types[static_cast<int>(ObjectInterfaceType::ProgramStorage)] = true;
    }
    virtual ~CProgramStorageObject()
    {}

    virtual bool IntroduceVirus() = 0;
    virtual void SetActiveVirus(bool bActive) = 0;
    virtual bool GetActiveVirus() = 0;

    virtual void SetSoluceName(const std::string& name) = 0;
    virtual const std::string& GetSoluceName() = 0;

    virtual bool ReadSoluce(const std::string& filename) = 0;
    virtual bool ReadProgram(Program* program, const std::string& filename) = 0;
    virtual bool GetCompile(Program* program) = 0;
    virtual bool WriteProgram(Program* program, const std::string& filename) = 0;

    virtual Program* AddProgram() = 0;
    virtual void AddProgram(std::unique_ptr<Program> program) = 0;
    virtual void RemoveProgram(Program* program) = 0;
    virtual Program* CloneProgram(Program* program) = 0;

    virtual std::vector<std::unique_ptr<Program>>& GetPrograms() = 0;
    virtual int GetProgramCount() = 0;
    virtual Program* GetProgram(int index) = 0;
    virtual Program* GetOrAddProgram(int index) = 0;
    virtual int GetProgramIndex(Program* program) = 0;
};
