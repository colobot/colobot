#include "app/app.h"
#include "graphics/opengl/gldevice.h"

template<> CApplication* CSingleton<CApplication>::mInstance = nullptr;

namespace Gfx {

GLDeviceConfig::GLDeviceConfig()
{
}

} /* Gfx */ 
CApplication::CApplication()
{
}

CApplication::~CApplication()
{
}

std::string CApplication::GetDataFilePath(DataDir /* dataDir */, const std::string& subpath)
{
    return subpath;
}


