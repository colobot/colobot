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

#include <iostream>
#include <memory>

#include "common/restext.h"

#include "CBot/CBot.h"

using namespace CBot;

namespace
{

CBotTypResult cMessage(CBotVar* &var, void* user)
{
    if ( var == nullptr )  return CBotTypResult(CBotErrLowParam);
    if ( var->GetType() != CBotTypString &&
         var->GetType() >  CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();

    if ( var == nullptr )  return CBotTypResult(CBotTypFloat);
    return CBotTypResult(CBotErrOverParam);
}

bool rMessage(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    std::string message = var->GetValString();

    std::cout << message << std::endl;

    return true;
}

} // namespace

int main(int argc, char* argv[])
{
    // Read program code from stdin
    std::string code = "";
    std::string line;
    while (std::getline(std::cin, line))
    {
        code += line;
        code += "\n";
    }

    // Initialize the CBot engine, add standard library functions
    CBotProgram::Init();
    CBotProgram::AddFunction("message", rMessage, cMessage);

    // Error message strings are stored on Colobot side (meh!) so let's initialize that
    InitializeRestext();

    CBotProgramGroup progGroup;

    // Compile the program
    std::vector<std::string> externFunctions;
    std::unique_ptr<CBotProgram> program = progGroup.AddProgram(nullptr);
    if (!program->Compile(code.c_str(), externFunctions, nullptr))
    {
        CBotError error;
        int cursor1, cursor2;
        program->GetError(error, cursor1, cursor2);
        std::string errorStr;
        GetResource(RES_CBOT, error, errorStr);
        std::cerr << "COMPILE ERROR: " << errorStr << " (code: " << error << ") @ " << cursor1 << " - " << cursor2 << std::endl;
        return 1;
    }

    // Execute all compiled functions marked as "extern"
    if (externFunctions.empty())
    {
        std::cerr << "NO EXTERN FUNCTIONS FOUND";
        return 2;
    }
    bool runErrors = false;
    for (const std::string& func : externFunctions)
    {
        if (!program->Start(func))
        {
            std::cerr << "FAILED TO START: " << func << std::endl;
            continue;
        }

        std::cerr << "Running program: " << func << std::endl;

        while (!program->Run(nullptr)); // Run the program

        CBotError error;
        int cursor1, cursor2;
        program->GetError(error, cursor1, cursor2);
        if (error != 0)
        {
            std::string errorStr;
            GetResource(RES_CBOT, error, errorStr);
            std::cerr << "RUNTIME ERROR: " << errorStr << " (code: " << error << ") @ " << cursor1 << " - " << cursor2 << std::endl;
            runErrors = true;
        }
        else
        {
            std::cerr << "Program finished." << std::endl;
        }
    }

    return runErrors ? 3 : 0;
}
