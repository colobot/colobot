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

#pragma once

#include "CBot/CBotVar/CBotVar.h"

#include "CBot/CBotEnums.h"
#include "CBot/CBotToken.h"

#include <sstream>
#include <cmath>


namespace CBot
{

/**
 * \brief A variable holding a simple value (bool, int, float, string)
 */
template <typename T, CBotType type>
class CBotVarValue : public CBotVar
{
public:
    /**
     * \brief Constructor. Do not call directly, use CBotVar::Create()
     */
    CBotVarValue(const CBotToken& name) : CBotVar(name)
    {
        m_type = type;
    }

    void Copy(CBotVar* pSrc, bool bName = true) override
    {
        CBotVar::Copy(pSrc, bName);

        CBotVarValue* p = static_cast<CBotVarValue*>(pSrc);
        m_val = p->m_val;
    }


    void SetValString(const std::string& val) override
    {
        std::istringstream s(val);
        s >> m_val;
        m_binit = CBotVar::InitType::DEF;
    }

    std::string GetValString() override
    {
        if (m_binit == CBotVar::InitType::UNDEF)
            return LoadString(TX_UNDEF);
        if (m_binit == CBotVar::InitType::IS_NAN)
            return LoadString(TX_NAN);

        std::ostringstream s;
        s << m_val;
        return s.str();
    }

protected:
    //! The value
    T m_val;
};

/**
 * \brief A number based variable (bool, int, float)
 */
template <typename T, CBotType type>
class CBotVarNumberBase : public CBotVarValue<T, type>
{
public:
    CBotVarNumberBase(const CBotToken &name) : CBotVarValue<T, type>(name) {}

    void SetValInt(int val, const std::string &s = "") override
    {
        this->m_val = static_cast<T>(val);
        this->m_binit = CBotVar::InitType::DEF;
    }

    void SetValFloat(float val) override
    {
        this->m_val = static_cast<T>(val);
        this->m_binit = CBotVar::InitType::DEF;
    }

    int GetValInt() override
    {
        return static_cast<int>(this->m_val);
    }

    float GetValFloat() override
    {
        return static_cast<float>(this->m_val);
    }


    bool Eq(CBotVar* left, CBotVar* right) override
    {
        return left->GetValFloat() == right->GetValFloat();
    }
    bool Ne(CBotVar* left, CBotVar* right) override
    {
        return left->GetValFloat() != right->GetValFloat();
    }
};

/**
 * \brief A number variable (int, float)
 */
template <typename T, CBotType type>
class CBotVarNumber : public CBotVarNumberBase<T, type>
{
public:
    CBotVarNumber(const CBotToken &name) : CBotVarNumberBase<T, type>(name) {}

    void Mul(CBotVar* left, CBotVar* right) override
    {
        this->SetValFloat(left->GetValFloat() * right->GetValFloat());
    }
    void Power(CBotVar* left, CBotVar* right) override
    {
        this->SetValFloat(pow(left->GetValFloat(), right->GetValFloat()));
    }
    CBotError Div(CBotVar* left, CBotVar* right) override
    {
        float r = right->GetValFloat();
        if (r == 0) return CBotErrZeroDiv;
        this->SetValFloat(left->GetValFloat() / r);
        return CBotNoErr;
    }
    CBotError Modulo(CBotVar* left, CBotVar* right) override
    {
        float r = right->GetValFloat();
        if (r == 0) return CBotErrZeroDiv;
        this->SetValFloat(fmod(left->GetValFloat(), r));
        return CBotNoErr;
    }
    void Add(CBotVar* left, CBotVar* right) override
    {
        this->SetValFloat(left->GetValFloat() + right->GetValFloat());
    }
    void Sub(CBotVar* left, CBotVar* right) override
    {
        this->SetValFloat(left->GetValFloat() - right->GetValFloat());
    }

    void Neg() override
    {
        this->m_val = - this->m_val;
    }
    void Inc() override
    {
        this->m_val++;
    }
    void Dec() override
    {
        this->m_val--;
    }

    bool Lo(CBotVar* left, CBotVar* right) override
    {
        return left->GetValFloat() < right->GetValFloat();
    }
    bool Hi(CBotVar* left, CBotVar* right) override
    {
        return left->GetValFloat() > right->GetValFloat();
    }
    bool Ls(CBotVar* left, CBotVar* right) override
    {
        return left->GetValFloat() <= right->GetValFloat();
    }
    bool Hs(CBotVar* left, CBotVar* right) override
    {
        return left->GetValFloat() >= right->GetValFloat();
    }
};

}

