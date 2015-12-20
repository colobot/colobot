#pragma once

#include "CBot/stdlib/Compilation.h"

#include <memory>

class CBotFile
{
public:
    virtual ~CBotFile() {}

    virtual bool Opened() = 0;
    virtual bool Errored() = 0;
    virtual bool IsEOF() = 0;

    virtual std::string ReadLine() = 0;
    virtual void Write(const std::string& s) = 0;

    //TODO
};

class CBotFileAccessHandler
{
public:
    virtual ~CBotFileAccessHandler() {}

    enum class OpenMode : char { Read = 'r', Write = 'w' };
    virtual std::unique_ptr<CBotFile> OpenFile(const std::string& filename, OpenMode mode) = 0;
    virtual bool DeleteFile(const std::string& filename) = 0;
};

void SetFileAccessHandler(std::unique_ptr<CBotFileAccessHandler> fileHandler);

// TODO: provide default implementation of CBotFileAccessHandler