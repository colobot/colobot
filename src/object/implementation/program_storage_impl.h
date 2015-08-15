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

#include "object/interface/program_storage_object.h"

#include "math/vector.h"

#include <sstream>

class CObject;

class CProgramStorageObjectImpl : public CProgramStorageObject
{
public:
    explicit CProgramStorageObjectImpl(ObjectInterfaceTypes& types, CObject* object);
    virtual ~CProgramStorageObjectImpl();

    bool IntroduceVirus() override;
    void SetActiveVirus(bool bActive) override;
    bool GetActiveVirus() override;

    void SetSoluceName(const std::string& name) override;
    const std::string& GetSoluceName() override;

    bool ReadSoluce(const std::string& filename) override;
    bool ReadProgram(Program* program, const std::string& filename) override;
    bool GetCompile(Program* program) override;
    bool WriteProgram(Program* program, const std::string& filename) override;

    Program* AddProgram() override;
    void AddProgram(std::unique_ptr<Program> program) override;
    void RemoveProgram(Program* program) override;
    Program* CloneProgram(Program* program) override;

    std::vector<std::unique_ptr<Program>>& GetPrograms() override;
    int GetProgramCount() override;
    Program* GetProgram(int index) override;
    Program* GetOrAddProgram(int index) override;
    int GetProgramIndex(Program* program) override;
private:
    CObject* m_object;

private:
    std::vector<std::unique_ptr<Program>> m_program;

    bool                m_activeVirus;

    std::string         m_soluceName;
};
