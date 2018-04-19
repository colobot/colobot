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

#include "common/logger.h"

#include "graphics/model/model_input.h"
#include "graphics/model/model_io_exception.h"
#include "graphics/model/model_output.h"

#include <iostream>
#include <fstream>
#include <map>

using namespace Gfx;


bool EndsWith(std::string const &fullString, std::string const &ending)
{
    if (fullString.length() >= ending.length())
        return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
    else
        return false;
}


struct Args
{
    bool usage;
    bool dumpInfo;
    std::string inputFile;
    std::string outputFile;
    std::string inputFormat;
    std::string outputFormat;

    Args()
    {
        usage = false;
        dumpInfo = false;
    }
};

Args ARGS;

void PrintUsage(const std::string& program)
{
    std::cerr << "Colobot model converter" << std::endl;
    std::cerr << std::endl;
    std::cerr << "Usage:" << std::endl;
    std::cerr << std::endl;
    std::cerr << " Convert files:" << std::endl;
    std::cerr << "   " << program << " -i input_file -if input_format -o output_file -of output_format" << std::endl;
    std::cerr << std::endl;
    std::cerr << " Dump info:" << std::endl;
    std::cerr << "   " << program << " -d -i input_file -if input_format" << std::endl;
    std::cerr << std::endl;
    std::cerr << " Help:" << std::endl;
    std::cerr << "   " << program << " -h" << std::endl;
    std::cerr << std::endl;

    std::cerr << "Model formats:" << std::endl;
    std::cerr << " old       => old binary format" << std::endl;
    std::cerr << " new_bin   => new binary format" << std::endl;
    std::cerr << " new_txt   => new text format" << std::endl;
}

bool ParseArgs(int argc, char *argv[])
{
    bool waitI = false, waitO = false;
    bool waitIf = false, waitOf = false;
    for (int i = 1; i < argc; ++i)
    {
        std::string arg = std::string(argv[i]);

        if (arg == "-i")
        {
            waitI = true;
            continue;
        }
        if (arg == "-o")
        {
            waitO = true;
            continue;
        }
        if (arg == "-if")
        {
            waitIf = true;
            continue;
        }
        if (arg == "-of")
        {
            waitOf = true;
            continue;
        }

        if (waitI)
        {
            waitI = false;
            ARGS.inputFile = arg;
        }
        else if (waitO)
        {
            waitO = false;
            ARGS.outputFile = arg;
        }
        else if (waitIf)
        {
            waitIf = false;
            ARGS.inputFormat = arg;
        }
        else if (waitOf)
        {
            waitOf = false;
            ARGS.outputFormat = arg;
        }
        else if (arg == "-h")
        {
            PrintUsage(argv[0]);
            ARGS.usage = true;
        }
        else if (arg == "-d")
        {
            ARGS.dumpInfo = true;
        }
        else
        {
            return false;
        }
    }

    if (waitI || waitO || waitIf || waitOf)
        return false;

    if (ARGS.usage)
        return true;

    if (ARGS.inputFile.empty() || (!ARGS.dumpInfo && ARGS.outputFile.empty() ))
        return false;

    if (ARGS.inputFormat.empty() || (!ARGS.dumpInfo && ARGS.outputFormat.empty() ))
        return false;

    return true;
}

template<typename T>
void PrintStats(const std::map<T, int>& stats, int total)
{
    for (auto it = stats.begin(); it != stats.end(); ++it)
    {
        std::cerr << "   " << (*it).first << " : " << (*it).second << " / " << total << std::endl;
    }
}

void DumpInfo(const CModel& model)
{
    const CModelMesh* mesh = model.GetMesh("main");
    if (mesh == nullptr)
    {
        std::cerr << "Main mesh not found!" << std::endl;
        return;
    }

    Math::Vector bboxMin( Math::HUGE_NUM,  Math::HUGE_NUM,  Math::HUGE_NUM);
    Math::Vector bboxMax(-Math::HUGE_NUM, -Math::HUGE_NUM, -Math::HUGE_NUM);

    std::map<std::string, int> texs1, texs2;
    std::map<int, int> states;
    int variableTexs2 = 0;

    for (const ModelTriangle& t : mesh->GetTriangles())
    {
        bboxMin.x = Math::Min(t.p1.coord.x, t.p2.coord.x, t.p3.coord.x, bboxMin.x);
        bboxMin.y = Math::Min(t.p1.coord.y, t.p2.coord.y, t.p3.coord.y, bboxMin.y);
        bboxMin.z = Math::Min(t.p1.coord.z, t.p2.coord.z, t.p3.coord.z, bboxMin.z);

        bboxMax.x = Math::Max(t.p1.coord.x, t.p2.coord.x, t.p3.coord.x, bboxMax.x);
        bboxMax.y = Math::Max(t.p1.coord.y, t.p2.coord.y, t.p3.coord.y, bboxMax.y);
        bboxMax.z = Math::Max(t.p1.coord.z, t.p2.coord.z, t.p3.coord.z, bboxMax.z);

        texs1[t.tex1Name] += 1;
        if (! t.tex2Name.empty())
            texs2[t.tex2Name] += 1;
        if (t.variableTex2)
            variableTexs2 += 1;
    }

    int total = mesh->GetTriangleCount();

    std::cerr << "---- Info ----" << std::endl;
    std::cerr << "Total triangles: " << total;
    std::cerr << std::endl;
    std::cerr << "Bounding box:" << std::endl;
    std::cerr << " bboxMin: [" << bboxMin.x << ", " << bboxMin.y << ", " << bboxMin.z << "]" << std::endl;
    std::cerr << " bboxMax: [" << bboxMax.x << ", " << bboxMax.y << ", " << bboxMax.z << "]" << std::endl;
    std::cerr << std::endl;
    std::cerr << "Textures:" << std::endl;
    std::cerr << " tex1:" << std::endl;
    PrintStats(texs1, total);
    std::cerr << " tex2:" << std::endl;
    PrintStats(texs2, total);
    std::cerr << " variable tex2: " << variableTexs2 << " / " << total << std::endl;
    std::cerr << std::endl;
}

int main(int argc, char *argv[])
{
    CLogger logger;
    logger.SetLogLevel(LOG_ERROR);

    if (!ParseArgs(argc, argv))
    {
        std::cerr << "Invalid arguments! Run with -h for usage info." << std::endl;
        return 1;
    }

    if (ARGS.usage)
        return 0;

    ModelFormat inputFormat = ModelFormat::Old;

    if (ARGS.inputFormat == "old")
        inputFormat = ModelFormat::Old;
    else if (ARGS.inputFormat == "new_bin")
        inputFormat = ModelFormat::Binary;
    else if (ARGS.inputFormat == "new_txt")
        inputFormat = ModelFormat::Text;
    else
    {
        std::cerr << "Invalid input format: " << ARGS.inputFormat << std::endl;
        return 1;
    }

    CModel model;

    try
    {
        std::ifstream stream;
        stream.open(ARGS.inputFile, std::ios_base::in | std::ios_base::binary);
        if (!stream.good())
            throw CModelIOException(std::string("Could not open file: ") + ARGS.inputFile);

        model = ModelInput::Read(stream, inputFormat);
    }
    catch (const CModelIOException& e)
    {
        std::cerr << "Reading input model failed with error:" << std::endl;
        std::cerr << e.what() << std::endl;
        return 1;
    }

    if (ARGS.dumpInfo)
    {
        DumpInfo(model);

        return 0;
    }

    ModelFormat outputFormat = ModelFormat::Old;

    if (ARGS.outputFormat == "old")
        outputFormat = ModelFormat::Old;
    else if (ARGS.outputFormat == "new_bin")
        outputFormat = ModelFormat::Binary;
    else if (ARGS.outputFormat == "new_txt")
        outputFormat = ModelFormat::Text;
    else
    {
        std::cerr << "Invalid output format: " << ARGS.outputFormat << std::endl;
        return 1;
    }

    try
    {
        std::ofstream stream;
        stream.open(ARGS.outputFile, std::ios_base::out | std::ios_base::binary);
        if (!stream.good())
            throw CModelIOException(std::string("Could not open file: ") + ARGS.inputFile);
        ModelOutput::Write(model, stream, outputFormat);
    }
    catch (const CModelIOException& e)
    {
        std::cerr << "Writing output model failed with error:" << std::endl;
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}

