/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2015, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include "app/signal_handlers.h"

#include "common/config.h"

#include "app/system.h"

#include "common/stringutils.h"

#include "level/robotmain.h"

#include <csignal>
#include <sstream>
#include <iostream>

CSystemUtils* CSignalHandlers::m_systemUtils = nullptr;

void CSignalHandlers::Init(CSystemUtils* systemUtils)
{
    m_systemUtils = systemUtils;
    signal(SIGSEGV, SignalHandler);
    signal(SIGABRT, SignalHandler);
    signal(SIGFPE,  SignalHandler);
    signal(SIGILL,  SignalHandler);
    std::set_terminate(UnhandledExceptionHandler);
}

void CSignalHandlers::SignalHandler(int sig)
{
    std::string signalStr = StrUtils::ToString(signal);
    switch(sig)
    {
        case SIGSEGV: signalStr = "SIGSEGV, segmentation fault"; break;
        case SIGABRT: signalStr = "SIGABRT, abort"; break;
        case SIGFPE:  signalStr = "SIGFPE, arithmetic exception"; break;
        case SIGILL:  signalStr = "SIGILL, illegal instruction"; break;
    }
    ReportError(signalStr);
}

// TODO: How portable across compilers is this?
#include <cstdlib>
#include <memory>
#include <cxxabi.h>
std::string demangle(const char* name) {
    int status;
    std::unique_ptr<char[], void(*)(void*)> result {
        abi::__cxa_demangle(name, nullptr, nullptr, &status),
        std::free
    };

    return result != nullptr ? result.get() : name;
}
// END OF TODO

void CSignalHandlers::UnhandledExceptionHandler()
{
    std::exception_ptr exptr = std::current_exception();
    try
    {
        std::rethrow_exception(exptr);
    }
    catch (const std::exception& e)
    {
        std::stringstream ss;
        ss << "Type: " << demangle(typeid(e).name()) << std::endl;
        ss << "Message: " << e.what();
        ReportError(ss.str());
    }
    catch (...)
    {
        ReportError("Unknown unhandled exception (not inherited from std::exception)");
    }
}

void CSignalHandlers::ReportError(const std::string& errorMessage)
{
    std::stringstream msg;
    msg << "Unhandled exception occured!" << std::endl;
    msg << "==============================" << std::endl;
    msg << errorMessage << std::endl;
    msg << "==============================" << std::endl;
    msg << std::endl;
    msg << "This is usually caused by a bug. Please report this on http://github.com/colobot/colobot/issues" << std::endl;
    msg << "including information on what you were doing before this happened and all the information below." << std::endl;
    msg << "==============================" << std::endl;
    #if BUILD_NUMBER == 0
        // COLOBOT_VERSION_DISPLAY doesn't update if you don't run CMake after "git pull"
        msg << "You seem to be running a custom compilation of version " << COLOBOT_VERSION_DISPLAY << ", but please verify that." << std::endl;
    #else
        msg << "You are running version " << COLOBOT_VERSION_DISPLAY << " from CI build #" << BUILD_NUMBER << std::endl;
    #endif
    msg << std::endl;
    if (!CRobotMain::IsCreated())
    {
        msg << "CRobotMain instance does not seem to exist" << std::endl;
    }
    else
    {
        CRobotMain* robotMain = CRobotMain::GetInstancePointer();
        msg << "The game was in phase " << PhaseToString(robotMain->GetPhase()) << " (ID=" << robotMain->GetPhase() << ")" << std::endl;
        msg << "Last started level was: category=" << GetLevelCategoryDir(robotMain->GetLevelCategory()) << " chap=" << robotMain->GetLevelChap() << " rank=" << robotMain->GetLevelRank() << std::endl;
    }
    msg << "==============================" << std::endl;
    msg << std::endl;
    msg << "Sorry for inconvenience!";

    std::cerr << std::endl << msg.str() << std::endl;
    m_systemUtils->SystemDialog(SDT_ERROR, "Unhandled exception occured!", msg.str());
    exit(1);
}
