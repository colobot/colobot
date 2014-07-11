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

CSoundInterface* CApplication::GetSound()
{
    return nullptr;
}

CEventQueue* CApplication::GetEventQueue()
{
    return nullptr;
}

Event CApplication::CreateUpdateEvent()
{
    return Event(EVENT_NULL);
}

char CApplication::GetLanguageChar() const
{
    return 'E';
}
