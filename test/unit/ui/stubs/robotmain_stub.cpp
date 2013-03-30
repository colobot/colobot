#include "object/robotmain.h"


template<> CRobotMain* CSingleton<CRobotMain>::m_instance = nullptr;

CRobotMain::CRobotMain(CApplication* app)
{
}

CRobotMain::~CRobotMain()
{
}

bool CRobotMain::GetGlint()
{
    return false;
}

const InputBinding& CRobotMain::GetInputBinding(InputSlot slot)
{
    unsigned int index = static_cast<unsigned int>(slot);
    assert(index >= 0 && index < INPUT_SLOT_MAX);
    return m_inputBindings[index];
}

