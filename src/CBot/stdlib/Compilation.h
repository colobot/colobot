#pragma once

#include "CBot/CBotTypResult.h"

class CBotVar;

// Commonly used functions for parameter compilation
// type "number" is anything > CBotTypDouble

// float foo()
CBotTypResult cNull(CBotVar* &var, void* user);
// float foo(number)
CBotTypResult cOneFloat(CBotVar* &var, void* user);
// float foo(number, number)
CBotTypResult cTwoFloat(CBotVar* &var, void* user);
// float foo(string)
CBotTypResult cString(CBotVar* &var, void* user);
// string foo(string)
CBotTypResult cStringString(CBotVar* &var, void* user);
// int foo(int)
CBotTypResult cOneInt(CBotVar* &var, void* user);
// bool foo(int)
CBotTypResult cOneIntReturnBool(CBotVar* &var, void* user);


// string foo(string)
CBotTypResult cStrStr(CBotVar*& var, void* user);
// int foo(string, string)
CBotTypResult cIntStrStr(CBotVar*& var, void* user);
// float foo(string)
CBotTypResult cFloatStr(CBotVar*& var, void* user);
// string foo(string, number[, number])
CBotTypResult cStrStrIntInt(CBotVar*& var, void* user);
// string foo(string, number)
CBotTypResult cStrStrInt(CBotVar*& var, void* user);
// int foo(string)
CBotTypResult cIntStr(CBotVar*& var, void* user);