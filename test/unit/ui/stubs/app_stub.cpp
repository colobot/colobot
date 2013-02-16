#include "app/app.h"

#include "graphics/opengl/gldevice.h"

template<> CApplication* CSingleton<CApplication>::m_instance = nullptr;

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

CSoundInterface* CApplication::GetSound()
{
    return nullptr;
}

CEventQueue* CApplication::GetEventQueue()
{
    return nullptr;
}

