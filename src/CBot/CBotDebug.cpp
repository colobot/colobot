/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2021, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include "CBot/CBotDebug.h"

#include "CBot/CBotProgram.h"
#include "CBot/CBotInstr/CBotFunction.h"
#include "CBot/CBotInstr/CBotInstrCall.h"

#include "core/stringutils.h"

#include <functional>
#include <sstream>
#include <iostream>
#include <iomanip>

namespace CBot
{

void CBotDebug::DumpCompiledProgram(CBotProgram* program)
{
    std::stringstream ss;
    ss << "digraph {" << std::endl;

    std::map<long, CBotFunction*> funcIdMap;
    for (CBotFunction* func : program->GetFunctions())
    {
        funcIdMap[func->m_nFuncIdent] = func;
    }

    std::set<CBotInstr*> finished;
    std::map<void*, int> instructions;
    int instructionsNextId = 0;
    auto GetPointerAsString = [&instructions, &instructionsNextId](void* ptr) -> std::string
    {
        if(instructions.count(ptr) == 0)
        {
            instructions[ptr] = instructionsNextId++;
        }

        char buffer[20];
        sprintf(buffer, "instr%d", instructions[ptr]);
        return std::string(buffer);
    };
    std::function<void(CBotInstr*)> DumpInstr = [&](CBotInstr* instr)
    {
        if (finished.find(instr) != finished.end()) return;
        finished.insert(instr);

        std::string label = "<b>"+instr->GetDebugName()+"</b>\n";
        std::string data = instr->GetDebugData();
        data = StrUtils::Replace(data, "&", "&amp;");
        data = StrUtils::Replace(data, "<", "&lt;");
        data = StrUtils::Replace(data, ">", "&gt;");
        label += data;
        label = StrUtils::Replace(label, "\n", "<br/>");

        std::string additional = "";
        if (instr->GetDebugName() == "CBotFunction")
        {
            label = instr->GetDebugData(); // hide the title
            CBotFunction* function = static_cast<CBotFunction*>(instr);
            if (function == program->m_entryPoint) additional += " shape=box3d";
            else additional += " shape=box";
            if (function->IsExtern()) additional += " color=cyan";
            else additional += " color=blue";
            additional += " group=func";
        }

        ss << GetPointerAsString(instr) << " [label=<" << label << ">" << additional << "]" << std::endl;

        if (instr->GetDebugName() == "CBotInstrCall")
        {
            CBotInstrCall* call = static_cast<CBotInstrCall*>(instr);
            if (funcIdMap.count(call->m_nFuncIdent) > 0)
            {
                ss << GetPointerAsString(instr) << " -> " << GetPointerAsString(funcIdMap[call->m_nFuncIdent]) << " [style=dotted color=gray weight=15]" << std::endl;
            }
        }

        for (const auto& it : instr->GetDebugLinks())
        {
            if (it.second == nullptr) continue;
            if (it.second->GetDebugName() == "CBotFunction") continue;
            DumpInstr(it.second);
            ss << GetPointerAsString(instr) << " -> " << GetPointerAsString(it.second) << " [label=\"" << it.first << "\"" << (it.first == "m_next" ? " weight=1" : " weight=5") << "]" << std::endl;
            if (it.first == "m_next" || (instr->GetDebugName() == "CBotFunction" && it.first == "m_block") || (instr->GetDebugName() == "CBotListInstr" && it.first == "m_instr"))
            {
                ss << "{ rank=same; " << GetPointerAsString(instr) << "; " << GetPointerAsString(it.second) << "; }" << std::endl;
            }
        }
    };

    if (program->m_entryPoint != nullptr)
    {
        DumpInstr(program->m_entryPoint);
    }
    std::string prev = GetPointerAsString(program->m_entryPoint);
    for (CBotFunction* func : program->GetFunctions())
    {
        if (func != program->m_entryPoint)
        {
            DumpInstr(func);

            //ss << prev << " -> " << GetPointerAsString(func) << " [style=invis]" << std::endl;
            prev = GetPointerAsString(func);
        }
    }

    ss << "}" << std::endl;
    std::cout << ss.str() << std::endl;

    /* // Terrible platform-dependent code :P
    std::stringstream filename;
    filename << "compiled" << (program->m_entryPoint != nullptr ? "_"+program->m_entryPoint->GetName() : "") << ".png";

    int pipeOut[2];
    pipe(pipeOut);
    if (fork())
    {
        close(pipeOut[0]);
        write(pipeOut[1], ss.str().c_str(), ss.str().size());
        close(pipeOut[1]);
        int rv;
        wait(&rv);
        if(rv != 0) exit(rv);
    }
    else
    {
        dup2(pipeOut[0], 0);
        close(pipeOut[1]);
        execl("/usr/bin/dot", "dot", "-Tpng", "-o", filename.str().c_str(), nullptr);
        exit(1); // failed to start
    } */
}

} // namespace CBot
