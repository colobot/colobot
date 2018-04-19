/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2018, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include "CBot/CBot.h"

#include <cmath>
#include <cstdlib>

namespace CBot
{
const float PI = 3.14159265358979323846f;

// Instruction "sin(degrees)".

bool rSin(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    float   value;

    value = var->GetValFloat();
    result->SetValFloat(sinf(value*PI/180.0f));
    return true;
}

// Instruction "cos(degrees)".

bool rCos(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    float   value;

    value = var->GetValFloat();
    result->SetValFloat(cosf(value*PI/180.0f));
    return true;
}

// Instruction "tan(degrees)".

bool rTan(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    float   value;

    value = var->GetValFloat();
    result->SetValFloat(tanf(value*PI/180.0f));
    return true;
}

// Instruction "asin(degrees)".

bool raSin(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    float   value;

    value = var->GetValFloat();
    result->SetValFloat(asinf(value)*180.0f/PI);
    return true;
}

// Instruction "acos(degrees)".

bool raCos(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    float   value;

    value = var->GetValFloat();
    result->SetValFloat(acosf(value)*180.0f/PI);
    return true;
}

// Instruction "atan(degrees)".

bool raTan(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    float   value;

    value = var->GetValFloat();
    result->SetValFloat(atanf(value)*180.0f/PI);
    return true;
}

// Instruction "atan2(y,x)".

bool raTan2(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    float y = var->GetValFloat();
    var = var->GetNext();
    float x = var->GetValFloat();

    result->SetValFloat(atan2(y, x) * 180.0f / PI);
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

} // namespace CBot
