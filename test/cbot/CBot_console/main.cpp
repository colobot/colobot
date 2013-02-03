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
