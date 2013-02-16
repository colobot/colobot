#include "graphics/engine/engine.h"
#include "graphics/engine/text.h"

#include "mocks/text_mock.h"

template<> Gfx::CEngine* CSingleton<Gfx::CEngine>::m_instance = nullptr;

namespace Gfx {

CEngine::CEngine(CApplication* app) :
    m_app(app)
{
    m_text = new CTextMock(this);
    m_text->Create();
}

CEngine::~CEngine()
{
    delete m_text;
    m_text = nullptr;
}

CParticle* CEngine::GetParticle()
{
    return nullptr;
}

Math::Point CEngine::WindowToInterfaceSize(Math::IntPoint size)
{
    return Math::Point(size.x, size.y);
}

void CEngine::SetState(int state, const Color& color)
{
    if (state == m_lastState && color == m_lastColor)
        return;

    m_lastState = state;
    m_lastColor = color;
}

Math::IntPoint CEngine::GetWindowSize()
{
    return m_size;
}

void CEngine::AddStatisticTriangle(int count)
{
        m_statisticTriangle += count;
}

void CEngine::SetMouseType(EngineMouseType type)
{
    m_mouseType = type;
}

bool CEngine::SetTexture(const std::string& /* name */, int /* stage */)
{
    return true;
}

CText* CEngine::GetText()
{
    return m_text;
}

CDevice* CEngine::GetDevice()
{
    return m_device;
}

int CEngine::GetEditIndentValue()
{
    return m_editIndentValue;
}

void CEngine::DeleteTexture(const std::string& /* texName */)
{
}
Texture CEngine::LoadTexture(const std::string& /* name */)
{
    Texture texture;
    return texture;
}

} /* Gfx */

