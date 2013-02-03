#pragma once

#include <CBot/CBot.h>
#include <string>

extern std::string s;

class CClass
{
public:
    CClass();
    
    CBotClass*		m_pClassPoint;
	CBotClass*		m_pClassPointIntr;

	bool InitInstance();
	void ExitInstance();
};
