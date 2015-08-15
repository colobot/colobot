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

#include "object/implementation/programmable_impl.h"

#include "common/global.h"

#include "level/robotmain.h"

#include "math/all.h"

#include "object/object.h"
#include "object/old_object.h"

#include "object/interface/controllable_object.h"
#include "object/interface/task_executor_object.h"

#include "object/motion/motion.h"
#include "object/motion/motionvehicle.h"

#include "physics/physics.h"

#include "script/script.h"

#include "ui/controls/edit.h"

#include <algorithm>
#include <iomanip>

CProgramStorageObjectImpl::CProgramStorageObjectImpl(ObjectInterfaceTypes& types, CObject* object)
    : CProgramStorageObject(types),
      m_object(object),
      m_program(),
      m_activeVirus(false),
      m_soluceName("")
{
}

CProgramStorageObjectImpl::~CProgramStorageObjectImpl()
{}

// Introduces a virus into a program.
// Returns true if it was inserted.

bool CProgramStorageObjectImpl::IntroduceVirus()
{
    if(m_program.size() == 0) return false;

    for ( int i=0 ; i<50 ; i++ )
    {
        int programIndex = rand() % m_program.size();
        if ( m_program[programIndex]->script->IntroduceVirus() )  // tries to introduce
        {
            m_activeVirus = true;  // active virus
            return true;
        }
    }
    return false;
}

// Active Virus indicates that the object is contaminated. Unlike ch'tites (??? - Programerus)
// letters which automatically disappear after a while,
// ActiveVirus does not disappear after you edit the program
// (Even if the virus is not fixed).

void CProgramStorageObjectImpl::SetActiveVirus(bool bActive)
{
    m_activeVirus = bActive;

    if ( !m_activeVirus )  // virus disabled?
    {
        m_object->SetVirusMode(false);  // chtites (??? - Programerus) letters also
    }
}

bool CProgramStorageObjectImpl::GetActiveVirus()
{
    return m_activeVirus;
}


Program* CProgramStorageObjectImpl::AddProgram()
{
    assert(m_object->Implements(ObjectInterfaceType::Old)); //TODO
    auto program = MakeUnique<Program>();
    program->script = MakeUnique<CScript>(dynamic_cast<COldObject*>(this));
    program->readOnly = false;
    program->runnable = true;

    Program* prog = program.get();
    AddProgram(std::move(program));
    return prog;
}

void CProgramStorageObjectImpl::AddProgram(std::unique_ptr<Program> program)
{
    m_program.push_back(std::move(program));
    m_object->UpdateInterface();
}

void CProgramStorageObjectImpl::RemoveProgram(Program* program)
{
    m_program.erase(
        std::remove_if(m_program.begin(), m_program.end(),
            [program](std::unique_ptr<Program>& prog) { return prog.get() == program; }),
        m_program.end());

    m_object->UpdateInterface();
}

Program* CProgramStorageObjectImpl::CloneProgram(Program* program)
{
    Program* newprog = AddProgram();

    // TODO: Is there any reason CScript doesn't have a function to get the program code directly?
    auto edit = MakeUnique<Ui::CEdit>();
    edit->SetMaxChar(Ui::EDITSTUDIOMAX);
    program->script->PutScript(edit.get(), "");
    newprog->script->GetScript(edit.get());

    return newprog;
}

std::vector<std::unique_ptr<Program>>& CProgramStorageObjectImpl::GetPrograms()
{
    return m_program;
}

int CProgramStorageObjectImpl::GetProgramCount()
{
    return static_cast<int>(m_program.size());
}

int CProgramStorageObjectImpl::GetProgramIndex(Program* program)
{
    for(unsigned int i = 0; i < m_program.size(); i++)
    {
        if(m_program[i].get() == program)
        {
            return i;
        }
    }
    return -1;
}

Program* CProgramStorageObjectImpl::GetProgram(int index)
{
    if(index < 0 || index >= static_cast<int>(m_program.size()))
        return nullptr;

    return m_program[index].get();
}

Program* CProgramStorageObjectImpl::GetOrAddProgram(int index)
{
    if(index < 0)
        return nullptr;

    if(index < static_cast<int>(m_program.size()))
        return m_program[index].get();

    for(int i = m_program.size(); i < index; i++)
    {
        AddProgram();
    }
    return AddProgram();
}


void CProgramStorageObjectImpl::SetSoluceName(const std::string& name)
{
    m_soluceName = name;
}

const std::string& CProgramStorageObjectImpl::GetSoluceName()
{
    return m_soluceName;
}


// Load a script solution, in the first free script.
// If there is already an identical script, nothing is loaded.

bool CProgramStorageObjectImpl::ReadSoluce(const std::string& filename)
{
    Program* prog = AddProgram();

    if ( !ReadProgram(prog, filename) )  return false;  // load solution
    prog->readOnly = true;

    for(unsigned int i = 0; i < m_program.size(); i++)
    {
        if(m_program[i].get() == prog) continue;

        //TODO: This is bad. It's very sensitive to things like \n vs \r\n etc.
        if ( m_program[i]->script->Compare(prog->script.get()) )  // the same already?
        {
            m_program[i]->readOnly = true; // Mark is as read-only
            RemoveProgram(prog);
            return false;
        }
    }

    return true;
}

// Load a script with a text file.

bool CProgramStorageObjectImpl::ReadProgram(Program* program, const std::string& filename)
{
    if ( program->script->ReadScript(filename.c_str()) )  return true;

    return false;
}

// Indicates whether a program is compiled correctly.

bool CProgramStorageObjectImpl::GetCompile(Program* program)
{
    return program->script->GetCompile();
}

// Saves a script in a text file.

bool CProgramStorageObjectImpl::WriteProgram(Program* program, const std::string& filename)
{
    if ( program->script->WriteScript(filename.c_str()) )  return true;

    return false;
}
