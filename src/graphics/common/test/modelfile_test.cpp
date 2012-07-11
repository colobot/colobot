#include "graphics/common/modelfile.h"
#include "common/iman.h"

#include <iostream>


int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        std::cerr << "Usage: " << argv[0] << " {mod|dxf} in_file out_file" << std::endl;
        return 1;
    }

    CInstanceManager iMan;
    Gfx::CModelFile modfile(&iMan);

    std::string mode(argv[1]);
    if (mode == "mod")
    {
        if (! modfile.ReadModel(argv[2], false, false) )
        {
            std::cerr << "Read error: " << modfile.GetError() << std::endl;
            return 2;
        }
    }
    else if (mode == "dxf")
    {
        if (! modfile.ReadDXF(argv[2], false, false) )
        {
            std::cerr << "Read error: " << modfile.GetError() << std::endl;
            return 2;
        }
    }
    else
    {
        std::cerr << "Usage: " << argv[0] << " {mod|dxf} in_file out_file" << std::endl;
        return 1;
    }

    if (! modfile.WriteModel(argv[3]) )
    {
        std::cerr << "Write error: " << modfile.GetError() << std::endl;
        return 3;
    }

    return 0;
}
