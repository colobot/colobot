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

#include "CBot/CBot.h"
#include "CBot/CBotDebug.h"

/**
 * \file test/cbot/compile_graph.cpp
 * \brief A tool for generating graphs of compiled CBot instructions
 *
 * To generate the graph, pass the output to GraphViz:
 * \code{.sh}
 * ./CBot-CompileGraph < input_file.txt | dot -Tx11
 * \endcode
 *
 * or for output to file:
 *
 * \code{.sh}
 * ./CBot-CompileGraph < input_file.txt | dot -Tpng -o output.png
 * \endcode
 */

using namespace CBot;

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

    // Initialize the CBot engine
    CBotProgram::Init();

    CBotNamespace namespace_;

    // Compile the program
    std::vector<std::string> externFunctions;
    std::unique_ptr<CBotProgram> program = namespace_.AddProgram(nullptr);
    if (!program->Compile(code.c_str(), externFunctions, nullptr))
    {
        CBotError error;
        int cursor1, cursor2;
        program->GetError(error, cursor1, cursor2);
        std::cerr << "COMPILE ERROR: " << error << " @ " << cursor1 << " - " << cursor2 << std::endl;
        return 1;
    }

    // Generate graphs for all compiled functions marked as "extern"
    if (externFunctions.empty())
    {
        std::cerr << "NO EXTERN FUNCTIONS FOUND";
        return 2;
    }
    for (const std::string& func : externFunctions)
    {
        program->Start(func);
        std::cerr << "# Graph for " << func << std::endl;
        CBotDebug::DumpCompiledProgram(program.get());
        program->Stop();
    }

    // Free the engine
    CBotProgram::Free();

    return 0;
}
