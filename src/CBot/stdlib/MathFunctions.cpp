#include "CBot/stdlib/stdlib.h"

#include "CBot/CBot.h"

#include <cmath>

// Instruction "sin(degrees)".

bool rSin(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    float   value;

    value = var->GetValFloat();
    result->SetValFloat(sinf(value*M_PI/180.0f));
    return true;
}

// Instruction "cos(degrees)".

bool rCos(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    float   value;

    value = var->GetValFloat();
    result->SetValFloat(cosf(value*M_PI/180.0f));
    return true;
}

// Instruction "tan(degrees)".

bool rTan(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    float   value;

    value = var->GetValFloat();
    result->SetValFloat(tanf(value*M_PI/180.0f));
    return true;
}

// Instruction "asin(degrees)".

bool raSin(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    float   value;

    value = var->GetValFloat();
    result->SetValFloat(asinf(value)*180.0f/M_PI);
    return true;
}

// Instruction "acos(degrees)".

bool raCos(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    float   value;

    value = var->GetValFloat();
    result->SetValFloat(acosf(value)*180.0f/M_PI);
    return true;
}

// Instruction "atan(degrees)".

bool raTan(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    float   value;

    value = var->GetValFloat();
    result->SetValFloat(atanf(value)*180.0f/M_PI);
    return true;
}

// Instruction "atan2(y,x)".

bool raTan2(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    float y = var->GetValFloat();
    var = var->GetNext();
    float x = var->GetValFloat();

    result->SetValFloat(atan2(y, x) * 180.0f / M_PI);
    return true;
}

// Instruction "sqrt(value)".

bool rSqrt(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    float   value;

    value = var->GetValFloat();
    result->SetValFloat(sqrtf(value));
    return true;
}

// Instruction "pow(x, y)".

bool rPow(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    float   x, y;

    x = var->GetValFloat();
    var = var->GetNext();
    y = var->GetValFloat();
    result->SetValFloat(powf(x, y));
    return true;
}

// Instruction "rand()".

bool rRand(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    result->SetValFloat(static_cast<float>(rand()) / static_cast<float>(RAND_MAX));
    return true;
}

// Instruction "abs()".

bool rAbs(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    float   value;

    value = var->GetValFloat();
    result->SetValFloat(fabs(value));
    return true;
}

// Instruction "floor()"

bool rFloor(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    float   value;

    value = var->GetValFloat();
    result->SetValFloat(floor(value));
    return true;
}

// Instruction "ceil()"

bool rCeil(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    float   value;

    value = var->GetValFloat();
    result->SetValFloat(ceil(value));
    return true;
}

// Instruction "round()"

bool rRound(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    float   value;

    value = var->GetValFloat();
    result->SetValFloat(round(value));
    return true;
}

// Instruction "trunc()"

bool rTrunc(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    float   value;

    value = var->GetValFloat();
    result->SetValFloat(trunc(value));
    return true;
}

void InitMathFunctions()
{
    CBotProgram::AddFunction("sin",   rSin,   cOneFloat);
    CBotProgram::AddFunction("cos",   rCos,   cOneFloat);
    CBotProgram::AddFunction("tan",   rTan,   cOneFloat);
    CBotProgram::AddFunction("asin",  raSin,  cOneFloat);
    CBotProgram::AddFunction("acos",  raCos,  cOneFloat);
    CBotProgram::AddFunction("atan",  raTan,  cOneFloat);
    CBotProgram::AddFunction("atan2", raTan2, cTwoFloat);
    CBotProgram::AddFunction("sqrt",  rSqrt,  cOneFloat);
    CBotProgram::AddFunction("pow",   rPow,   cTwoFloat);
    CBotProgram::AddFunction("rand",  rRand,  cNull);
    CBotProgram::AddFunction("abs",   rAbs,   cOneFloat);
    CBotProgram::AddFunction("floor", rFloor, cOneFloat);
    CBotProgram::AddFunction("ceil",  rCeil,  cOneFloat);
    CBotProgram::AddFunction("round", rRound, cOneFloat);
    CBotProgram::AddFunction("trunc", rTrunc, cOneFloat);
}