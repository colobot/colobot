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

    std::string GetValString() const override
    {
        if (m_binit == CBotVar::InitType::UNDEF)
            return UndefinedTokenString();

        std::ostringstream s;
        s << std::boolalpha << m_val;
        return s.str();
    }

protected:
    virtual void SetValue(T val)
    {
        this->m_val = val;
        this->m_binit = CBotVar::InitType::DEF;
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
    CBotVarNumberBase(const CBotToken &name) : CBotVarValue<T, type>(name)
    {
        this->m_val = static_cast<T>(0);
    }

    void SetValByte(signed char val) override
    {
        this->SetValue(static_cast<T>(val));
    }

    void SetValShort(short val) override
    {
        this->SetValue(static_cast<T>(val));
    }

    void SetValChar(uint32_t val) override
    {
        this->SetValue(static_cast<T>(val));
    }

    void SetValInt(int val, const std::string &s = "") override
    {
        this->SetValue(static_cast<T>(val));
    }

    void SetValLong(long val) override
    {
        this->SetValue(static_cast<T>(val));
    }

    void SetValFloat(float val) override
    {
        this->SetValue(static_cast<T>(val));
    }

    void SetValDouble(double val) override
    {
        this->SetValue(static_cast<T>(val));
    }

    signed char GetValByte() const override
    {
        return static_cast<signed char>(this->m_val);
    }

    short GetValShort() const override
    {
        return static_cast<short>(this->m_val);
    }

    uint32_t GetValChar() const override
    {
        return static_cast<uint32_t>(this->m_val);
    }

    int GetValInt() const override
    {
        return static_cast<int>(this->m_val);
    }

    long GetValLong() const override
    {
        return static_cast<long>(this->m_val);
    }

    float GetValFloat() const override
    {
        return static_cast<float>(this->m_val);
    }

    double GetValDouble() const override
    {
        return static_cast<double>(this->m_val);
    }

    bool Eq(CBotVar* left, CBotVar* right) override
    {
        return static_cast<T>(*left) == static_cast<T>(*right);
    }
    bool Ne(CBotVar* left, CBotVar* right) override
    {
        return static_cast<T>(*left) != static_cast<T>(*right);
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
        this->SetValue(static_cast<T>(*left) * static_cast<T>(*right));
    }
    void Power(CBotVar* left, CBotVar* right) override
    {
        this->SetValue(pow(static_cast<T>(*left), static_cast<T>(*right)));
    }
    CBotError Div(CBotVar* left, CBotVar* right) override
    {
        T r = static_cast<T>(*right);
        if ( r == static_cast<T>(0) ) return CBotErrZeroDiv;
        this->SetValue(static_cast<T>(*left) / r);
        return CBotNoErr;
    }
    CBotError Modulo(CBotVar* left, CBotVar* right) override
    {
        T r = static_cast<T>(*right);
        if ( r == static_cast<T>(0) ) return CBotErrZeroDiv;
        this->SetValue(fmod(static_cast<T>(*left), r));
        return CBotNoErr;
    }
    void Add(CBotVar* left, CBotVar* right) override
    {
        this->SetValue(static_cast<T>(*left) + static_cast<T>(*right));
    }
    void Sub(CBotVar* left, CBotVar* right) override
    {
        this->SetValue(static_cast<T>(*left) - static_cast<T>(*right));
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
        return static_cast<T>(*left) < static_cast<T>(*right);
    }
    bool Hi(CBotVar* left, CBotVar* right) override
    {
        return static_cast<T>(*left) > static_cast<T>(*right);
    }
    bool Ls(CBotVar* left, CBotVar* right) override
    {
        return static_cast<T>(*left) <= static_cast<T>(*right);
    }
    bool Hs(CBotVar* left, CBotVar* right) override
    {
        return static_cast<T>(*left) >= static_cast<T>(*right);
    }
};

/**
 * \brief An integer variable (byte, short, char, int, long)
 */
template <typename T, CBotType type>
class CBotVarInteger : public CBotVarNumber<T, type>
{
public:
    CBotVarInteger(const CBotToken &name) : CBotVarNumber<T, type>(name) {}

    CBotError Modulo(CBotVar* left, CBotVar* right) override
    {
        T r = static_cast<T>(*right);
        if ( r == static_cast<T>(0) ) return CBotErrZeroDiv;
        this->SetValue(static_cast<T>(*left) % r);
        return CBotNoErr;
    }

    void XOr(CBotVar* left, CBotVar* right) override
    {
        this->SetValue(static_cast<T>(*left) ^ static_cast<T>(*right));
    }
    void And(CBotVar* left, CBotVar* right) override
    {
        this->SetValue(static_cast<T>(*left) & static_cast<T>(*right));
    }
    void Or(CBotVar* left, CBotVar* right) override
    {
        this->SetValue(static_cast<T>(*left) | static_cast<T>(*right));
    }

    void SL(CBotVar* left, CBotVar* right) override
    {
        this->SetValue(static_cast<T>(*left) << right->GetValInt());
    }
    void ASR(CBotVar* left, CBotVar* right) override
    {
        this->SetValue(static_cast<T>(*left) >> right->GetValInt());
    }

    void Not() override
    {
        this->m_val = ~(this->m_val);
    }

    std::string GetValString() const override
    {
        std::ostringstream s;
        s << +this->m_val;
        return s.str();
    }
};

} // namespace CBot
