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

#include "CBot/CBotVar/CBotVarValue.h"

namespace CBot
{

/**
 * \brief CBotVar subclass for managing string values (::CBotTypString)
 */
class CBotVarString : public CBotVarValue<std::string, CBotTypString>
{
public:
    CBotVarString(const CBotToken &name) : CBotVarValue(name) {}

    void SetValString(const std::string& val) override
    {
        m_val = val;
        m_binit = CBotVar::InitType::DEF;
    }

    void SetValInt(int val, const std::string& s = "") override
    {
        SetValString(ToString(val));
    }

    void SetValFloat(float val) override
    {
        SetValString(ToString(val));
    }

    int GetValInt() override
    {
        return FromString<int>(GetValString());
    }

    float GetValFloat() override
    {
        return FromString<float>(GetValString());
    }

    void Add(CBotVar* left, CBotVar* right) override;

    bool Eq(CBotVar* left, CBotVar* right) override;
    bool Ne(CBotVar* left, CBotVar* right) override;

    bool Save1State(FILE* pf) override;

private:
    template<typename T>
    static std::string ToString(T val)
    {
        std::ostringstream ss;
        ss << val;
        return ss.str();
    }

    template<typename T>
    static T FromString(std::string val)
    {
        std::istringstream ss(val);
        T v;
        ss >> v;
        return v;
    }
};

} // namespace CBot
