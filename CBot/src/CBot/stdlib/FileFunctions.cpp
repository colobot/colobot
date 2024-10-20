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

#include "CBot/stdlib/stdlib.h"

#include "common/stringutils.h"

#include "CBot/CBot.h"

#include <memory>
#include <unordered_map>
#include <cassert>

namespace CBot
{

namespace
{
std::unique_ptr<CBotFileAccessHandler> g_fileHandler;
std::unordered_map<int, std::unique_ptr<CBotFile>> g_files;
int g_nextFileId = 1;

bool FileClassOpenFile(CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception)
{
    CBotFileAccessHandler::OpenMode openMode = CBotFileAccessHandler::OpenMode::Read;

    // must be a character string
    if ( pVar->GetType() != CBotTypString ) { Exception = CBotErrBadString; return false; }

    std::filesystem::path  filename;
    try
    {
        filename = StrUtils::ToPath(pVar->GetValString());
    }
    catch(...)
    {
        Exception = CBotErrFileOpen;
        return false;
    }

    // there may be a second parameter
    pVar = pVar->GetNext();
    if ( pVar != nullptr )
    {
        // recover mode
        std::string mode = pVar->GetValString();
        if ( mode == "r" ) openMode = CBotFileAccessHandler::OpenMode::Read;
        else if ( mode == "w" ) openMode = CBotFileAccessHandler::OpenMode::Write;
        else if ( mode == "a" ) openMode = CBotFileAccessHandler::OpenMode::Append;
        else { Exception = CBotErrBadParam; return false; }

        // no third parameter
        if ( pVar->GetNext() != nullptr ) { Exception = CBotErrOverParam; return false; }
    }

    // saves the file name
    pVar = pThis->GetItem("filename");
    pVar->SetValString(StrUtils::ToString(filename));

    // retrieve the item "handle"
    pVar = pThis->GetItem("handle");
    // which must not be initialized
    if ( pVar->IsDefined()) { Exception = CBotErrFileOpen; return false; }

    // opens the requested file
    assert(g_fileHandler != nullptr);

    std::unique_ptr<CBotFile> file = g_fileHandler->OpenFile(filename, openMode);

    if (!file->Opened()) { Exception = CBotErrFileOpen; return false; }

    int fileHandle = g_nextFileId++;
    g_files[fileHandle] = std::move(file);

    // save the file handle
    pVar = pThis->GetItem("handle");
    pVar->SetValInt(fileHandle);

    return true;
}

// constructor of the class
// get the filename as a parameter

// execution
bool rfconstruct (CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception, void* user)
{
    // accepts no parameters
    if ( pVar == nullptr ) return true;

    return FileClassOpenFile(pThis, pVar, pResult, Exception);
}

// compilation
CBotTypResult cfconstruct (CBotVar* pThis, CBotVar* &pVar)
{
    // accepts no parameters
    if ( pVar == nullptr ) return CBotTypResult( 0 );

    // must be a character string
    if ( pVar->GetType() != CBotTypString )
        return CBotTypResult( CBotErrBadString );

    // there may be a second parameter
    pVar = pVar->GetNext();
    if ( pVar != nullptr )
    {
        // which must be a string
        if ( pVar->GetType() != CBotTypString )
            return CBotTypResult( CBotErrBadString );
        // no third parameter
        if ( pVar->GetNext() != nullptr ) return CBotTypResult( CBotErrOverParam );
    }

    // the result is void (constructor)
    return CBotTypResult( 0 );
}


// destructor of the class

// execution
bool rfdestruct (CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception, void* user)
{
    // retrieve the item "handle"
    pVar = pThis->GetItem("handle");

    if (!pVar->IsDefined()) return true; // file not opened
    g_files.erase(pVar->GetValInt());

    pVar->SetInit(CBotVar::InitType::UNDEF);
    return true;
}


// process FILE :: open
// get the r/w mode as a parameter

// execution
bool rfopen (CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception, void* user)
{
    // there must be a parameter
    if ( pVar == nullptr ) { Exception = CBotErrLowParam; return false; }

    bool result = FileClassOpenFile(pThis, pVar, pResult, Exception);
    pResult->SetValInt(result);
    return result;
}

// compilation
CBotTypResult cfopen (CBotVar* pThis, CBotVar* &pVar)
{
    // there must be a parameter
    if ( pVar == nullptr ) return CBotTypResult( CBotErrLowParam );

    // which must be a string
    if ( pVar->GetType() != CBotTypString )
        return CBotTypResult( CBotErrBadString );

    // there may be a second parameter
    pVar = pVar->GetNext();
    if ( pVar != nullptr )
    {
        // which must be a string
        if ( pVar->GetType() != CBotTypString )
            return CBotTypResult( CBotErrBadString );

        // no third parameter
        if ( pVar->GetNext() != nullptr ) return CBotTypResult( CBotErrOverParam );
    }

    // the result is bool
    return CBotTypResult(CBotTypBoolean);
}


// process FILE :: close

// execution
bool rfclose (CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception, void* user)
{
    // it shouldn't be any parameters
    if (pVar != nullptr) { Exception = CBotErrOverParam; return false; }

    // retrieve the item "handle"
    pVar = pThis->GetItem("handle");

    if (!pVar->IsDefined()) { Exception = CBotErrNotOpen; return false; }

    int fileHandle = pVar->GetValInt();

    const auto handleIter = g_files.find(fileHandle);
    if (handleIter == g_files.end())
    {
        Exception = CBotErrNotOpen;
        return false;
    }

    g_files.erase(handleIter);

    pVar->SetInit(CBotVar::InitType::UNDEF);
    return true;
}

// compilation
CBotTypResult cfclose (CBotVar* pThis, CBotVar* &pVar)
{
    // it shouldn't be any parameters
    if ( pVar != nullptr ) return CBotTypResult( CBotErrOverParam );

    // function returns a result "void"
    return CBotTypResult( 0 );
}

// process FILE :: writeln

// execution
bool rfwrite (CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception, void* user)
{
    // there must be a parameter
    if ( pVar == nullptr ) { Exception = CBotErrLowParam; return false; }

    // which must be a character string
    if ( pVar->GetType() != CBotTypString ) { Exception = CBotErrBadString; return false; }

    std::string param = pVar->GetValString();

    // retrieve the item "handle"
    pVar = pThis->GetItem("handle");

    if ( !pVar->IsDefined()) { Exception = CBotErrNotOpen; return false; }

    int fileHandle = pVar->GetValInt();

    const auto handleIter = g_files.find(fileHandle);
    if (handleIter == g_files.end())
    {
        Exception = CBotErrNotOpen;
        return false;
    }

    handleIter->second->Write(param + "\n");

    // if an error occurs generate an exception
    if ( handleIter->second->Errored() ) { Exception = CBotErrWrite; return false; }

    return true;
}

// compilation
CBotTypResult cfwrite (CBotVar* pThis, CBotVar* &pVar)
{
    // there must be a parameter
    if ( pVar == nullptr ) return CBotTypResult( CBotErrLowParam );

    // which must be a character string
    if ( pVar->GetType() != CBotTypString ) return CBotTypResult( CBotErrBadString );

    // no other parameter
    if ( pVar->GetNext() != nullptr ) return CBotTypResult( CBotErrOverParam );

    // the function returns a void result
    return CBotTypResult( 0 );
}

// process FILE :: readln

// execution
bool rfread(CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception, void* user)
{
    // it shouldn't be any parameters
    if (pVar != nullptr) { Exception = CBotErrOverParam; return false; }

    // retrieve the item "handle"
    pVar = pThis->GetItem("handle");

    if (!pVar->IsDefined()) { Exception = CBotErrNotOpen; return false; }

    int fileHandle = pVar->GetValInt();

    const auto handleIter = g_files.find(fileHandle);
    if (handleIter == g_files.end())
    {
        Exception = CBotErrNotOpen;
        return false;
    }

    std::string line = handleIter->second->ReadLine();

    // if an error occurs generate an exception
    if ( handleIter->second->Errored() ) { Exception = CBotErrRead; return false; }

    pResult->SetValString( line.c_str() );

    return true;
}

// compilation
CBotTypResult cfread (CBotVar* pThis, CBotVar* &pVar)
{
    // it should not be any parameter
    if ( pVar != nullptr ) return CBotTypResult( CBotErrOverParam );

    // function returns a result "string"
    return CBotTypResult( CBotTypString );
}
// process FILE :: readln


// execution
bool rfeof (CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception, void* user)
{
    // it should not be any parameter
    if ( pVar != nullptr ) { Exception = CBotErrOverParam; return false; }

    // retrieve the item "handle"
    pVar = pThis->GetItem("handle");

    if ( !pVar->IsDefined()) { Exception = CBotErrNotOpen; return false; }

    int fileHandle = pVar->GetValInt();

    const auto handleIter = g_files.find(fileHandle);
    if (handleIter == g_files.end())
    {
        Exception = CBotErrNotOpen;
        return false;
    }

    pResult->SetValInt( handleIter->second->IsEOF() );

    return true;
}

// compilation
CBotTypResult cfeof (CBotVar* pThis, CBotVar* &pVar)
{
    // it shouldn't be any parameter
    if ( pVar != nullptr ) return CBotTypResult( CBotErrOverParam );

    // the function returns a boolean result
    return CBotTypResult( CBotTypBoolean );
}

// Instruction "deletefile(filename)".

bool rDeleteFile(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    std::filesystem::path filename;
    try
    {
        filename = StrUtils::ToPath(var->GetValString());
    }
    catch(...)
    {
        exception = CBotErrFileOpen;
        return false;
    }
    assert(g_fileHandler != nullptr);
    return g_fileHandler->DeleteFile(filename);
}

} // namespace

void InitFileFunctions()
{
    // create a class for file management
    // the use is as follows:
    // file canal( "NomFichier.txt" )
    // canal.open( "r" );   // open for read
    // s = canal.readln( ); // reads a line
    // canal.close();   // close the file

    // create the class FILE
    CBotClass* bc = CBotClass::Create("file", nullptr);
    // adds the component ".filename"
    bc->AddItem("filename", CBotTypString);
    // adds the component ".handle"
    bc->AddItem("handle", CBotTypInt, CBotVar::ProtectionLevel::Private);

    // define a constructor and a destructor
    bc->AddFunction("file", rfconstruct, cfconstruct);
    bc->AddFunction("~file", rfdestruct, nullptr);

    // end of the methods associated
    bc->AddFunction("open", rfopen, cfopen);
    bc->AddFunction("close", rfclose, cfclose);
    bc->AddFunction("writeln", rfwrite, cfwrite);
    bc->AddFunction("readln", rfread, cfread);
    bc->AddFunction("eof", rfeof, cfeof );

    CBotProgram::AddFunction("deletefile", rDeleteFile, cString);

    //m_pFuncFile = new CBotProgram( );
    //std::stringArray ListFonctions;
    //m_pFuncFile->Compile( "public file openfile(string name, string mode) {return new file(name, mode);}", ListFonctions);
    //m_pFuncFile->SetIdent(-2);  // restoreState in special identifier for this function
}

void SetFileAccessHandler(std::unique_ptr<CBotFileAccessHandler> fileHandler)
{
    g_fileHandler = std::move(fileHandler);
}

} // namespace CBot
