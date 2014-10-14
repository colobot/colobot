/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2014, Daniel Roux, EPSITEC SA & TerranovaTeam
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
#include "CClass.h"
#include "CBotDoc.h"
#include <iostream>


#include <fstream>

std::string str;

// routine to update the instance of the class Bot common

int main(int argc, char* argv[])
{
    CClass newclass;
    CBotDoc *botdoc;
    if (argc != 2)
    {
    	std::cout << "Usage: "<<argv[0] << " <filename>" << std::endl;
    	return 0;
    }

    std::ifstream in(argv[1]);
    std::cout << argv[1] << std::endl;
    if (!in.good())
    {
    	std::cout << "Oh no, error!" << std::endl;
    	return 1;
    }

    std::string contents((std::istreambuf_iterator<char>(in)),
        std::istreambuf_iterator<char>());
    str = contents;

    if(!newclass.InitInstance())
    {
    	std::cerr << "Initialization not complete!" << std::endl;
    	return 1;
    }

    botdoc = new CBotDoc(str);
//    std::cout << "Hello CBot!" << std::endl << s;
    botdoc->OnRun();
    delete botdoc;
    newclass.ExitInstance();
}
