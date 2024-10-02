/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2023, Daniel Roux, EPSITEC SA & TerranovaTeam
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
#include <filesystem>

class CScript;
class CLevelParserLine;

struct Program
{
    std::unique_ptr<CScript> script;
    bool        readOnly = false;
    bool        runnable = true;
    std::string filename;
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

    /**
     * Infects a random program of a robot with virus
     * \return true if program was successfully infected, false on failure (no programs or no suitable keywords to infect found)
     */
    virtual bool IntroduceVirus() = 0;
    /**
     * Sets the virus as active, this controls the "Program infected" message and is cleared after you edit the program.
     * Note that this status is independent from the virus particles (see COldObject::SetVirusMode for that)
     */
    virtual void SetActiveVirus(bool bActive) = 0;
    //! Checks virus active status
    virtual bool GetActiveVirus() = 0;

    //! Read program from a given file
    virtual bool ReadProgram(Program* program, const std::filesystem::path& filename) = 0;
    //! Write program to a given file
    virtual bool WriteProgram(Program* program, const std::filesystem::path& filename) = 0;
    //! Check if program was compiled successfully
    virtual bool GetCompile(Program* program) = 0;

    //! Add a new program
    virtual Program* AddProgram() = 0;
    //! Add given program to the program list stored in this robot
    virtual void AddProgram(std::unique_ptr<Program> program) = 0;
    //! Remove program
    virtual void RemoveProgram(Program* program) = 0;
    //! Clone a program
    virtual Program* CloneProgram(Program* program) = 0;

    //! Return a std::vector of all available programs
    virtual std::vector<std::unique_ptr<Program>>& GetPrograms() = 0;
    //! Return count of all programs
    virtual int GetProgramCount() = 0;
    //! Return program with the given index
    virtual Program* GetProgram(int index) = 0;
    //! Return program at the given index (creating it and all previous ones as needed)
    virtual Program* GetOrAddProgram(int index) = 0;
    //! Return index in the list of given Program instance
    virtual int GetProgramIndex(Program* program) = 0;

    //! Set index for use in filename for saved programs (-1 to disable)
    virtual void SetProgramStorageIndex(int programStorageIndex) = 0;
    //! Return index that is used while saving programs
    virtual int GetProgramStorageIndex() = 0;

    //! Save all user programs
    virtual void SaveAllUserPrograms(const std::string& userSource) = 0;
    //! Load all programs when loading the level including previously saved user programs
    virtual void LoadAllProgramsForLevel(CLevelParserLine* levelSource, const std::string& userSource, bool loadSoluce) = 0;

    //! Save all programs when saving the saved scene
    virtual void SaveAllProgramsForSavedScene(CLevelParserLine* levelSourceLine, const std::filesystem::path& levelSource) = 0;
    //! Load all programs when loading the saved scene
    virtual void LoadAllProgramsForSavedScene(CLevelParserLine* levelSourceLine, const std::string& levelSource) = 0;
};
