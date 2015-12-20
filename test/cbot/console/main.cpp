#include <iostream>
#include <memory>

#include "common/restext.h"

#include "CBot/CBot.h"

CBotTypResult cMessage(CBotVar* &var, void* user)
{
    if ( var == nullptr )  return CBotTypResult(CBotErrLowParam);
    if ( var->GetType() != CBotTypString &&
         var->GetType() >  CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();

    if ( var == nullptr )  return CBotTypResult(CBotTypFloat);
    return CBotTypResult(CBotErrOverParam);
}

bool rMessage(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CBotString cbs = var->GetValString();
    const char* message = cbs; // Don't ask me why, but it doesn't work if you write it on a single line

    std::cout << message << std::endl;

    return true;
}

int main(int argc, char* argv[])
{
    // Read program code from stdin
    std::string code = "";
    std::string line;
    while (std::getline(std::cin, line))
    {
        code += line;
        code += "\n";
    }

    // Error message strings are stored on Colobot side (meh!) so let's initialize that
    InitializeRestext();

    // Initialize the CBot engine, add standard library functions
    CBotProgram::AddFunction("message", rMessage, cMessage);

    // Compile the program
    std::vector<CBotString> externFunctions;
    std::unique_ptr<CBotProgram> program{new CBotProgram(nullptr)};
    if (!program->Compile(code.c_str(), externFunctions, nullptr))
    {
        int error, cursor1, cursor2;
        program->GetError(error, cursor1, cursor2);
        std::string errorStr;
        GetResource(RES_CBOT, error, errorStr);
        std::cerr << "COMPILE ERROR: " << errorStr << " (code: " << error << ") @ " << cursor1 << " - " << cursor2 << std::endl;
        return 1;
    }

    // Execute all compiled functions marked as "extern"
    if (externFunctions.empty())
    {
        std::cerr << "NO EXTERN FUNCTIONS FOUND";
        return 2;
    }
    bool runErrors = false;
    for (const char* func : externFunctions)
    {
        if (!program->Start(func))
        {
            std::cerr << "FAILED TO START: " << func << std::endl;
            continue;
        }

        std::cerr << "Running program: " << func << std::endl;

        while (!program->Run(nullptr)); // Run the program

        int error, cursor1, cursor2;
        program->GetError(error, cursor1, cursor2);
        if (error != 0)
        {
            std::string errorStr;
            GetResource(RES_CBOT, error, errorStr);
            std::cerr << "RUNTIME ERROR: " << errorStr << " (code: " << error << ") @ " << cursor1 << " - " << cursor2 << std::endl;
            runErrors = true;
        }
        else
        {
            std::cerr << "Program finished." << std::endl;
        }
    }

    return runErrors ? 3 : 0;
}
