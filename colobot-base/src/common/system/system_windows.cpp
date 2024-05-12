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

#include "common/system/system_windows.h"

#include "common/logger.h"
#include "common/stringutils.h"
#include "common/version.h"

#include <array>
#include <filesystem>
#include <iostream>
#include <sstream>

#include <Windows.h>
#include <DbgHelp.h>
#include <process.h>

LONG StackTraceExceptionFilter(LPEXCEPTION_POINTERS exception);

std::unique_ptr<CSystemUtils> CSystemUtils::Create()
{
    return std::make_unique<CSystemUtilsWindows>();
}

CSystemUtilsWindows::CSystemUtilsWindows()
{
    SymInitialize(GetCurrentProcess(), nullptr, true);
}

CSystemUtilsWindows::~CSystemUtilsWindows()
{
    SymCleanup(GetCurrentProcess());
}

void CSystemUtilsWindows::Init([[maybe_unused]] const std::vector<std::string>& args)
{
    m_arguments.clear();

    int wargc = 0;
    wchar_t** wargv = CommandLineToArgvW(GetCommandLineW(), &wargc);
    if (wargv == nullptr)
    {
        GetLogger()->Error("CommandLineToArgvW failed\n");
        return;
    }

    for (int i = 0; i < wargc; i++)
    {
        std::wstring warg = wargv[i];
        m_arguments.push_back(CSystemUtilsWindows::UTF8_Encode(warg));
    }

    LocalFree(wargv);
}

void CSystemUtilsWindows::InitErrorHandling()
{
    SetUnhandledExceptionFilter(StackTraceExceptionFilter);
}

SystemDialogResult CSystemUtilsWindows::SystemDialog(SystemDialogType type, const std::string& title, const std::string& message)
{
    unsigned int windowsType = 0;
    std::wstring windowsMessage = UTF8_Decode(message);
    std::wstring windowsTitle = UTF8_Decode(title);

    switch (type)
    {
        case SystemDialogType::INFO:
        default:
            windowsType = MB_ICONINFORMATION|MB_OK;
            break;
        case SystemDialogType::WARNING:
            windowsType = MB_ICONWARNING|MB_OK;
            break;
        case SystemDialogType::ERROR_MSG:
            windowsType = MB_ICONERROR|MB_OK;
            break;
        case SystemDialogType::YES_NO:
            windowsType = MB_ICONQUESTION|MB_YESNO;
            break;
        case SystemDialogType::OK_CANCEL:
            windowsType = MB_ICONWARNING|MB_OKCANCEL;
            break;
    }

    switch (MessageBoxW(nullptr, windowsMessage.c_str(), windowsTitle.c_str(), windowsType))
    {
        case IDOK:
            return SystemDialogResult::OK;
        case IDCANCEL:
            return SystemDialogResult::CANCEL;
        case IDYES:
            return SystemDialogResult::YES;
        case IDNO:
            return SystemDialogResult::NO;
        default:
            break;
    }

    return SystemDialogResult::OK;
}

//! Converts a wide Unicode string to an UTF8 string
std::string CSystemUtilsWindows::UTF8_Encode(const std::wstring& wstr)
{
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], static_cast<int>(wstr.size()), nullptr, 0, nullptr, nullptr);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], static_cast<int>(wstr.size()), &strTo[0], size_needed, nullptr, nullptr);
    return strTo;
}

//! Converts an UTF8 string to a wide Unicode String
std::wstring CSystemUtilsWindows::UTF8_Decode(const std::string& str)
{
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], static_cast<int>(str.size()), nullptr, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], static_cast<int>(str.size()), &wstrTo[0], size_needed);
    return wstrTo;

}

std::filesystem::path CSystemUtilsWindows::GetSaveDir() const
{
    if constexpr (Version::PORTABLE_SAVES || Version::DEVELOPMENT_BUILD)
    {
        return CSystemUtils::GetSaveDir();
    }
    else
    {
        std::filesystem::path savegameDir;

        auto envUSERPROFILE = GetEnvVar("USERPROFILE");
        if (envUSERPROFILE.empty())
        {
            GetLogger()->Warn("Unable to find directory for saves - using default directory");
            savegameDir = CSystemUtils::GetSaveDir();
        }
        else
        {
            savegameDir = StrUtils::ToPath(envUSERPROFILE) / "colobot";
        }
        GetLogger()->Trace("Saved game files are going to %%", savegameDir);

        return savegameDir;
    }
}

std::string CSystemUtilsWindows::GetEnvVar(const std::string& name) const
{
    std::wstring wname(name.begin(), name.end());
    wchar_t* envVar = _wgetenv(wname.c_str());
    if (envVar == nullptr)
    {
        return "";
    }
    else
    {
        std::string var = UTF8_Encode(std::wstring(envVar));
        GetLogger()->Trace("Detected environment variable %% = %%", name, var);
        return var;
    }
}

bool CSystemUtilsWindows::OpenPath(const std::filesystem::path& path)
{
    int result = system(("start explorer \"" + std::filesystem::path{path}.make_preferred().string() + "\"").c_str());
    if (result != 0)
    {
        GetLogger()->Error("Failed to open path: %%, error code: %%", path, result);
        return false;
    }
    return true;
}

bool CSystemUtilsWindows::OpenWebsite(const std::string& url)
{
    int result = system(("rundll32 url.dll,FileProtocolHandler \"" + url + "\"").c_str());
    if (result != 0)
    {
        GetLogger()->Error("Failed to open website: %%, error code: %%", url, result);
        return false;
    }
    return true;
}

bool CSystemUtilsWindows::IsDebuggerPresent() const
{
    return ::IsDebuggerPresent();
}

namespace
{

#if defined(_M_AMD64)
constexpr DWORD MACHINE = IMAGE_FILE_MACHINE_AMD64;
#else
constexpr DWORD MACHINE = IMAGE_FILE_MACHINE_I386;
#endif

}

LONG StackTraceExceptionFilter(LPEXCEPTION_POINTERS exception)
{
    auto ctx = exception->ContextRecord;

    // On x64, StackWalk64 modifies the context record, that could
    // cause crashes, so we create a copy to prevent it
    CONTEXT ctxCopy = *ctx;

    STACKFRAME64 stack;
    memset(&stack, 0, sizeof(STACKFRAME64));

#if !defined(_M_AMD64)
    stack.AddrPC.Offset = (*ctx).Eip;
    stack.AddrPC.Mode = AddrModeFlat;
    stack.AddrStack.Offset = (*ctx).Esp;
    stack.AddrStack.Mode = AddrModeFlat;
    stack.AddrFrame.Offset = (*ctx).Ebp;
    stack.AddrFrame.Mode = AddrModeFlat;
#endif

    std::stringstream stream;

    switch (exception->ExceptionRecord->ExceptionCode)
    {
    case EXCEPTION_ACCESS_VIOLATION:
    {
        const auto code = exception->ExceptionRecord->ExceptionInformation[0];
        const auto address = reinterpret_cast<void*>(exception->ExceptionRecord->ExceptionInformation[1]);

        if (code == 0)
            stream << "Access violation reading location 0x" << address;
        else if (code == 1)
            stream << "Access violation writing location 0x" << address;
        else if (code == 8)
            stream << "Data execution prevention violation at location 0x" << address;
        else
            stream << "Access violation accessing location 0x" << address;

        break;
    }
    case EXCEPTION_FLT_DENORMAL_OPERAND:
        stream << "Floating-point denormal operand";
        break;
    case EXCEPTION_FLT_DIVIDE_BY_ZERO:
        stream << "Floating-point division by zero";
        break;
    case EXCEPTION_FLT_INEXACT_RESULT:
        stream << "Inexact floating-point result";
        break;
    case EXCEPTION_FLT_INVALID_OPERATION:
        stream << "Invalid floating-point operation";
        break;
    case EXCEPTION_FLT_OVERFLOW:
        stream << "Floating-point overflow";
        break;
    case EXCEPTION_FLT_STACK_CHECK:
        stream << "Floating-point stack check";
        break;
    case EXCEPTION_FLT_UNDERFLOW:
        stream << "Floating-point underflow";
        break;
    case EXCEPTION_ILLEGAL_INSTRUCTION:
        stream << "Illegal instruction";
        break;
    case EXCEPTION_IN_PAGE_ERROR:
        stream << "Page error";
        break;
    case EXCEPTION_INT_DIVIDE_BY_ZERO:
        stream << "Integer division by zero";
        break;
    case EXCEPTION_INT_OVERFLOW:
        stream << "Integer overflow";
        break;
    case EXCEPTION_STACK_OVERFLOW:
        stream << "Stack overflow";
        break;
    default:
        stream << "Error 0x" << std::hex << exception->ExceptionRecord->ExceptionCode << std::dec;
        break;
    }

    stream << '\n';

    std::string lastModule = "";

    const HANDLE process = GetCurrentProcess();
    const HANDLE thread = GetCurrentThread();

    while (true)
    {
        // Get next entry from stack
        BOOL result = StackWalk64(MACHINE, process, thread, &stack, &ctxCopy,
                nullptr, SymFunctionTableAccess64, SymGetModuleBase64, nullptr);

        if (!result)
            break;

        // Get symbol name
        std::array<char, sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(char)> buffer;
        PSYMBOL_INFO pSymbol = reinterpret_cast<PSYMBOL_INFO>(buffer.data());

        pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
        pSymbol->MaxNameLen = MAX_SYM_NAME;

        DWORD64 displacement = 0;

        std::string name;

        if (SymFromAddr(process, (ULONG64)stack.AddrPC.Offset, &displacement, pSymbol))
        {
            name = pSymbol->Name;
        }

        // Get information about line in the code
        IMAGEHLP_LINE64 line;
        line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);

        DWORD disp = {};

        int lineNumber = {};
        std::string file = {};
        void* address = {};

        if (SymGetLineFromAddr64(process, stack.AddrPC.Offset, &disp, &line))
        {
            lineNumber = line.LineNumber;
            file = std::filesystem::path(line.FileName).filename().string();
            address = reinterpret_cast<void*>(line.Address);
        }
        else
        {
            address = reinterpret_cast<void*>(stack.AddrPC.Offset);
        }

        // Get module name
        HMODULE hModule = nullptr;

        GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                          reinterpret_cast<LPCTSTR>(stack.AddrPC.Offset), &hModule);

        std::string module = "<unknown>";

        if (hModule != NULL)
        {
            std::array<char, 1024> name;

            GetModuleFileNameA(hModule, name.data(), static_cast<DWORD>(name.size()));

            module = std::filesystem::path(name.data()).filename().string();
        }

        stream << '\n';

        if (lastModule != module)
        {
            stream << module << '\n';

            lastModule = module;
        }

        stream << "    " << name << " at ";

        if (file.empty())
            stream << std::hex << address << std::dec;
        else
            stream << file << ":" << lineNumber;
    }

    CSystemUtils::GetInstance().CriticalError(stream.str());

    return EXCEPTION_EXECUTE_HANDLER;
}
