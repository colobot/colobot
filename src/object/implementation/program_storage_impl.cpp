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

#include "object/implementation/program_storage_impl.h"

#include "common/global.h"
#include "common/logger.h"
#include "common/stringutils.h"

#include "common/resources/resourcemanager.h"

#include "level/robotmain.h"

#include "level/parser/parserline.h"

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
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>

CProgramStorageObjectImpl::CProgramStorageObjectImpl(ObjectInterfaceTypes& types, CObject* object)
    : CProgramStorageObject(types),
      m_object(object),
      m_program(),
      m_activeVirus(false),
      m_programStorageIndex(-1),
      m_allowProgramSave(true)
{
}

CProgramStorageObjectImpl::~CProgramStorageObjectImpl()
{}


bool CProgramStorageObjectImpl::IntroduceVirus()
{
    if(m_program.size() == 0) return false;

    for ( int i=0 ; i<50 ; i++ )
    {
        int programIndex = rand() % m_program.size();
        if ( m_program[programIndex]->script->IntroduceVirus() )  // tries to introduce
        {
            m_program[programIndex]->filename = ""; // The program is changed, so force it to save instead of just the filename
            m_activeVirus = true;  // active virus
            return true;
        }
    }
    return false;
}

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


bool CProgramStorageObjectImpl::ReadProgram(Program* program, const std::string& filename)
{
    if ( program->script->ReadScript(filename.c_str()) )  return true;

    return false;
}

bool CProgramStorageObjectImpl::WriteProgram(Program* program, const std::string& filename)
{
    if ( program->script->WriteScript(filename.c_str()) )  return true;

    return false;
}

bool CProgramStorageObjectImpl::GetCompile(Program* program)
{
    return program->script->GetCompile();
}



void CProgramStorageObjectImpl::SetProgramStorageIndex(int programStorageIndex)
{
    m_programStorageIndex = programStorageIndex;
}

int CProgramStorageObjectImpl::GetProgramStorageIndex()
{
    return m_programStorageIndex;
}

void CProgramStorageObjectImpl::SaveAllUserPrograms(const std::string& userSource)
{
    if (!m_allowProgramSave) return;
    if (m_programStorageIndex < 0) return;
    GetLogger()->Debug("Saving user programs to '%s%.3d___.txt'\n", userSource.c_str(), m_programStorageIndex);

    for (unsigned int i = 0; i < m_program.size(); i++)
    {
        std::string filename = userSource + StrUtils::Format("%.3d%.3d.txt", m_programStorageIndex, i);

        if (m_program[i]->filename.empty())
        {
            GetLogger()->Trace("Saving program '%s' into user directory\n", filename.c_str());
            WriteProgram(m_program[i].get(), filename);
        }
    }

    std::string dir = userSource.substr(0, userSource.find_last_of("/"));
    std::string file = userSource.substr(userSource.find_last_of("/")+1) + StrUtils::Format("%.3d([0-9]{3})\\.txt", m_programStorageIndex);
    boost::regex regex(file);
    for (const std::string& filename : CResourceManager::ListFiles(dir))
    {
        boost::smatch matches;
        if (boost::regex_match(filename, matches, regex))
        {
            unsigned int id = boost::lexical_cast<unsigned int>(matches[1]);
            if (id >= m_program.size() || !m_program[id]->filename.empty())
            {
                GetLogger()->Trace("Removing old program '%s/%s'\n", dir.c_str(), filename.c_str());
                CResourceManager::Remove(dir+"/"+filename);
            }
        }
    }
}

void CProgramStorageObjectImpl::LoadAllProgramsForLevel(CLevelParserLine* levelSource, const std::string& userSource, bool loadSoluce)
{
    int run = levelSource->GetParam("run")->AsInt(0)-1;
    bool allFilled = true;
    for (int i = 0; i < 10 || allFilled; i++)
    {
        std::string op = "script" + StrUtils::ToString<int>(i+1); // script1..script10
        std::string opReadOnly = "scriptReadOnly" + StrUtils::ToString<int>(i+1); // scriptReadOnly1..scriptReadOnly10
        std::string opRunnable = "scriptRunnable" + StrUtils::ToString<int>(i+1); // scriptRunnable1..scriptRunnable10
        if (levelSource->GetParam(op)->IsDefined())
        {
            std::string filename = levelSource->GetParam(op)->AsPath("ai");
            GetLogger()->Trace("Loading program '%s' from level file\n", filename.c_str());
            Program* program = AddProgram();
            ReadProgram(program, filename);
            program->readOnly = levelSource->GetParam(opReadOnly)->AsBool(true);
            program->runnable = levelSource->GetParam(opRunnable)->AsBool(true);
            program->filename = levelSource->GetParam(op)->AsString();

            if (m_object->Implements(ObjectInterfaceType::Programmable) && i == run)
            {
                dynamic_cast<CProgrammableObject*>(m_object)->RunProgram(program);
            }
        }
        else
        {
            allFilled = false;
        }
    }

    if (loadSoluce && levelSource->GetParam("soluce")->IsDefined())
    {
        std::string filename = levelSource->GetParam("soluce")->AsPath("ai");
        GetLogger()->Trace("Loading program '%s' as soluce file\n", filename.c_str());
        Program* program = AddProgram();
        ReadProgram(program, filename);
        program->readOnly = true;
        program->runnable = false;
        program->filename = levelSource->GetParam("soluce")->AsString();
    }

    if (m_programStorageIndex >= 0)
    {
        GetLogger()->Debug("Loading user programs from '%s%.3d___.txt'\n", userSource.c_str(), m_programStorageIndex);

        std::string dir = userSource.substr(0, userSource.find_last_of("/"));
        std::string file = userSource.substr(userSource.find_last_of("/")+1) + StrUtils::Format("%.3d([0-9]{3})\\.txt", m_programStorageIndex);
        boost::regex regex(file);
        for (const std::string& filename : CResourceManager::ListFiles(dir))
        {
            boost::smatch matches;
            if (boost::regex_match(filename, matches, regex))
            {
                unsigned int i = boost::lexical_cast<unsigned int>(matches[1]);
                Program* program = GetOrAddProgram(i);
                if(GetCompile(program)) program = AddProgram(); // If original slot is already used, get a new one
                GetLogger()->Trace("Loading program '%s/%s' from user directory\n", dir.c_str(), filename.c_str());
                ReadProgram(program, dir+"/"+filename);
            }
        }
    }
}

void CProgramStorageObjectImpl::SaveAllProgramsForSavedScene(CLevelParserLine* levelSourceLine, const std::string& levelSource)
{
    levelSourceLine->AddParam("programStorageIndex", MakeUnique<CLevelParserParam>(m_programStorageIndex));

    for (unsigned int i = 0; i < m_program.size(); i++)
    {
        if (!m_program[i]->filename.empty() && m_program[i]->readOnly)
        {
            levelSourceLine->AddParam("script" + StrUtils::ToString<int>(i+1), MakeUnique<CLevelParserParam>(m_program[i]->filename));
            levelSourceLine->AddParam("scriptReadOnly" + StrUtils::ToString<int>(i+1), MakeUnique<CLevelParserParam>(m_program[i]->readOnly));
            levelSourceLine->AddParam("scriptRunnable" + StrUtils::ToString<int>(i+1), MakeUnique<CLevelParserParam>(m_program[i]->runnable));
        }
    }

    if (m_programStorageIndex < 0) return;
    if (!m_object->Implements(ObjectInterfaceType::Controllable) || !dynamic_cast<CControllableObject*>(m_object)->GetSelectable() || m_object->GetType() == OBJECT_HUMAN) return;

    GetLogger()->Debug("Saving saved scene programs to '%s/prog%.3d___.txt'\n", levelSource.c_str(), m_programStorageIndex);
    for (unsigned int i = 0; i < m_program.size(); i++)
    {
        std::string filename = levelSource + StrUtils::Format("/prog%.3d%.3d.txt", m_programStorageIndex, i);
        if (!m_program[i]->filename.empty() && m_program[i]->readOnly) continue;

        GetLogger()->Trace("Saving program '%s' to saved scene\n", filename.c_str());
        WriteProgram(m_program[i].get(), filename);
        levelSourceLine->AddParam("scriptReadOnly" + StrUtils::ToString<int>(i+1), MakeUnique<CLevelParserParam>(m_program[i]->readOnly));
        levelSourceLine->AddParam("scriptRunnable" + StrUtils::ToString<int>(i+1), MakeUnique<CLevelParserParam>(m_program[i]->runnable));
    }

    boost::regex regex(StrUtils::Format("prog%.3d([0-9]{3})\\.txt", m_programStorageIndex));
    for (const std::string& filename : CResourceManager::ListFiles(levelSource))
    {
        boost::smatch matches;
        if (boost::regex_match(filename, matches, regex))
        {
            unsigned int id = boost::lexical_cast<unsigned int>(matches[1]);
            if (id >= m_program.size() || !m_program[id]->filename.empty())
            {
                GetLogger()->Trace("Removing old program '%s/%s' from saved scene\n", levelSource.c_str(), filename.c_str());
                CResourceManager::Remove(levelSource+"/"+filename);
            }
        }
    }
}

void CProgramStorageObjectImpl::LoadAllProgramsForSavedScene(CLevelParserLine* levelSourceLine, const std::string& levelSource)
{
    int run = levelSourceLine->GetParam("run")->AsInt(0)-1;
    m_programStorageIndex = levelSourceLine->GetParam("programStorageIndex")->AsInt(-1);

    for (int i = 0; i <= 999; i++)
    {
        std::string op = "script" + StrUtils::ToString<int>(i+1); // script1..script10
        std::string opReadOnly = "scriptReadOnly" + StrUtils::ToString<int>(i+1); // scriptReadOnly1..scriptReadOnly10
        std::string opRunnable = "scriptRunnable" + StrUtils::ToString<int>(i+1); // scriptRunnable1..scriptRunnable10
        if (levelSourceLine->GetParam(op)->IsDefined())
        {
            std::string filename = levelSourceLine->GetParam(op)->AsPath("ai");
            GetLogger()->Trace("Loading program '%s' from saved scene\n", filename.c_str());
            Program* program = GetOrAddProgram(i);
            ReadProgram(program, filename);
            program->readOnly = levelSourceLine->GetParam(opReadOnly)->AsBool(true);
            program->runnable = levelSourceLine->GetParam(opRunnable)->AsBool(true);
            program->filename = levelSourceLine->GetParam(op)->AsString();

            if (m_object->Implements(ObjectInterfaceType::Programmable) && i == run)
            {
                dynamic_cast<CProgrammableObject*>(m_object)->RunProgram(program);
            }
        }
    }

    if(m_programStorageIndex < 0) return;

    GetLogger()->Debug("Loading saved scene programs from '%s/prog%.3d___.txt'\n", levelSource.c_str(), m_programStorageIndex);
    for (int i = 0; i <= 999; i++)
    {
        std::string opReadOnly = "scriptReadOnly" + StrUtils::ToString<int>(i+1); // scriptReadOnly1..scriptReadOnly10
        std::string opRunnable = "scriptRunnable" + StrUtils::ToString<int>(i+1); // scriptRunnable1..scriptRunnable10

        std::string filename = levelSource + StrUtils::Format("/prog%.3d%.3d.txt", m_programStorageIndex, i);
        if (CResourceManager::Exists(filename))
        {
            GetLogger()->Trace("Loading program '%s' from saved scene\n", filename.c_str());
            Program* program = GetOrAddProgram(i);
            ReadProgram(program, filename);
            program->readOnly = levelSourceLine->GetParam(opReadOnly)->AsBool(true);
            program->runnable = levelSourceLine->GetParam(opRunnable)->AsBool(true);

            if (m_object->Implements(ObjectInterfaceType::Programmable) && i == run)
            {
                dynamic_cast<CProgrammableObject*>(m_object)->RunProgram(program);
            }
        }
    }

    // Disable automatic user program storage now!!
    // This is to prevent overwriting auto-saved user programs with older versions from saved scenes
    m_allowProgramSave = false;
}
