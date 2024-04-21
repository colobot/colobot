/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2023, Daniel Roux, EPSITEC SA & TerranovaTeam
 * http://epsitec.ch; http://colobot.info; http://github.com/colobot
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

#include "common/stringutils.h"
#include "common/version.h"

#include "common/resources/resourcemanager.h"

#include "common/system/system.h"

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
    std::string signalStr = StrUtils::ToString(sig);
    switch(sig)
    {
        case SIGSEGV: signalStr = "SIGSEGV, segmentation fault"; break;
        case SIGABRT: signalStr = "SIGABRT, abort"; break;
        case SIGFPE:  signalStr = "SIGFPE, arithmetic exception"; break;
        case SIGILL:  signalStr = "SIGILL, illegal instruction"; break;
    }
    ReportError(signalStr);
}

#if HAVE_DEMANGLE
// For gcc and clang
#include <cstdlib>
#include <memory>
#include <cxxabi.h>
static std::string demangle(const char* name)
{
    int status;
    std::unique_ptr<char[], void(*)(void*)> result {
        abi::__cxa_demangle(name, nullptr, nullptr, &status),
        std::free
    };

    return (result != nullptr && status == 0) ? result.get() : name;
}
#else
// For MSVC and others
// In MSVC typeinfo(e).name() should be already demangled
static std::string demangle(const char* name)
{
    return name;
}
#endif

void CSignalHandlers::UnhandledExceptionHandler()
{
    std::exception_ptr exptr = std::current_exception();
    if (!exptr)
    {
        ReportError("std::terminate called without an exception");
        return;
    }

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
    static bool triedSaving = false;

    if (SDL_WasInit(SDL_INIT_VIDEO))
    {
        // Close the SDL window on crash, because otherwise the error doesn't show on in fullscreen mode and the game appears to freeze
        SDL_Quit();
    }

    std::stringstream msg;
    msg << "Unhandled exception occurred!" << std::endl;
    msg << "==============================" << std::endl;
    msg << errorMessage << std::endl;
    msg << "==============================" << std::endl;
    msg << std::endl;
    msg << "This is usually caused by a bug. Please report this on http://github.com/colobot/colobot/issues" << std::endl;
    msg << "including information on what you were doing before this happened and all the information below." << std::endl;
    msg << "==============================" << std::endl;

    if constexpr (Version::BUILD_NUMBER == 0)
    {
        if constexpr (Version::OFFICIAL_BUILD)
        {
            msg << "You are running official " << Version::VERSION_DISPLAY << " build." << std::endl;
        }
        else
        {
            msg << "You seem to be running a custom compilation of version " << Version::VERSION_DISPLAY << ", but please verify that." << std::endl;
        }
    }
    else
    {
        msg << "You are running version " << Version::VERSION_DISPLAY << " from CI build #" << Version::BUILD_NUMBER << std::endl;
    }

    msg << std::endl;
    bool canSave = false;
    CRobotMain* robotMain = nullptr;
    if (!CRobotMain::IsCreated())
    {
        msg << "CRobotMain instance does not seem to exist" << std::endl;
    }
    else
    {
        robotMain = CRobotMain::GetInstancePointer();
        msg << "The game was in phase " << PhaseToString(robotMain->GetPhase()) << " (ID=" << robotMain->GetPhase() << ")" << std::endl;
        msg << "Last started level was: category=" << GetLevelCategoryDir(robotMain->GetLevelCategory()) << " chap=" << robotMain->GetLevelChap() << " rank=" << robotMain->GetLevelRank() << std::endl;
        canSave = (robotMain->GetPhase() == PHASE_SIMUL || IsInSimulationConfigPhase(robotMain->GetPhase())) && !robotMain->IsLoading();
    }
    msg << "==============================" << std::endl;
    msg << std::endl;
    msg << "Sorry for inconvenience!";

    std::cerr << std::endl << msg.str() << std::endl;

    m_systemUtils->SystemDialog(SystemDialogType::ERROR_MSG, "Unhandled exception occurred!", msg.str());

    if (canSave && !triedSaving)
    {
        msg.str("");
        msg << "You can try saving the game at the moment of a crash. Keep in mind, the game engine is in" << std::endl;
        msg << "an unstable state so the saved game may be corrupted or even cause another crash." << std::endl;
        msg << std::endl;
        msg << "Do you want to try saving now?";

        SystemDialogResult result = m_systemUtils->SystemDialog(SystemDialogType::YES_NO, "Try to save?", msg.str());
        if (result == SystemDialogResult::YES)
        {
            triedSaving = true;
            CResourceManager::CreateNewDirectory("crashsave");
            robotMain->IOWriteScene("crashsave/data.sav", "crashsave/cbot.run", "crashsave/screen.png", "Backup at the moment of a crash", true);
            m_systemUtils->SystemDialog(SystemDialogType::INFO, "Try to save?", "Saving finished.\nPlease restart the game now");
        }
    }

    exit(1);
}
