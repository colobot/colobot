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
    m_systemUtils->CriticalError(signalStr);
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
        m_systemUtils->CriticalError("std::terminate called without an exception");
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
        m_systemUtils->CriticalError(ss.str());
    }
    catch (...)
    {
        m_systemUtils->CriticalError("Unknown unhandled exception (not inherited from std::exception)");
    }
}
