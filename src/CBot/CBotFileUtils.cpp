/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2021, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include "CBot/CBotFileUtils.h"

#include "CBot/CBotClass.h"
#include "CBot/CBotEnums.h"

namespace CBot
{

template<typename T>
static bool WriteBinary(std::ostream &ostr, T value, unsigned padTo = 0)
{
    unsigned char chr;
    unsigned count = 1;
    while (value > 127) // unsigned LEB128
    {
        ++count;
        chr = (value & 0x7F) | 0x80;
        if (!ostr.write(reinterpret_cast<char*>(&chr), 1)) return false;
        value >>= 7;
    }
    chr = value & 0x7F;
    if (count < padTo) chr |= 0x80;
    if (!ostr.write(reinterpret_cast<char*>(&chr), 1)) return false;

    if (count < padTo)
    {
        while (++count < padTo)
            if (!(ostr << '\x80')) return false;
        if (!(ostr << '\x00')) return false;
    }
    return true;
}

template<typename T>
static bool ReadBinary(std::istream &istr, T &value)
{
    value = 0;
    unsigned char chr;
    unsigned shift = 0;
    while (true)        // unsigned LEB128
    {
        if (!istr.read(reinterpret_cast<char*>(&chr), 1)) return false;
        if (shift < sizeof(T) * 8)
            value |= static_cast<T>(chr & 0x7F) << shift;
        shift += 7;
        if ((chr & 0x80) == 0) break;
    }
    return true;
}

template<typename T>
static bool WriteSignedBinary(std::ostream &ostr, T value, unsigned padTo = 0)
{
    signed char sign = value >> (8 * sizeof(T) - 1);
    unsigned count = 0;
    while (true)        // signed LEB128
    {
        ++count;
        unsigned char chr = value & 0x7F;
        value >>= 7;
        if (!(value != sign || ((chr ^ sign) & 0x40) != 0))
        {
            if (count < padTo) chr |= 0x80;
            if (!ostr.write(reinterpret_cast<char*>(&chr), 1)) return false;
            break;
        }
        chr |= 0x80;
        if (!ostr.put(chr)) return false;
    }

    if (count < padTo)
    {
        char chr = (sign < 0) ? 0x7F : 0x00;
        while (++count < padTo)
            if (!ostr.put(chr | 0x80)) return false;
        if (!ostr.put(chr)) return false;
    }
    return true;
}

template<typename T>
static bool ReadSignedBinary(std::istream &istr, T &value)
{
    value = 0;
    unsigned char chr;
    unsigned shift = 0;
    while (true)        // signed LEB128
    {
        if (!istr.read(reinterpret_cast<char*>(&chr), 1)) return false;
        if (shift < sizeof(T) * 8 - 1)
            value |= (static_cast<T>(chr & 0x7F) << shift);
        shift += 7;
        if ((chr & 0x80) == 0) break;
    }

    if (shift >= 8 * sizeof(T) - 1) shift = 8 * sizeof(T) - 1;
    if ((chr & 0x40) != 0)
        value |= static_cast<T>(-1) << shift;

    return true;
}

bool WriteWord(std::ostream &ostr, unsigned short w)
{
    return WriteBinary<unsigned short>(ostr, w);
}

bool ReadWord(std::istream &istr, unsigned short &w)
{
    return ReadBinary<unsigned short>(istr, w);
}

bool WriteByte(std::ostream &ostr, char c)
{
    if (!ostr.put(c)) return false;
    return true;
}

bool ReadByte(std::istream &istr, char& c)
{
    if (!istr.get(c)) return false;
    return true;
}

bool WriteShort(std::ostream &ostr, short s)
{
    return WriteSignedBinary<short>(ostr, s);
}

bool ReadShort(std::istream &istr, short &s)
{
    return ReadSignedBinary<short>(istr, s);
}

bool WriteUInt32(std::ostream &ostr, uint32_t i)
{
    return WriteBinary<uint32_t>(ostr, i);
}

bool ReadUInt32(std::istream &istr, uint32_t &i)
{
    return ReadBinary<uint32_t>(istr, i);
}

bool WriteInt(std::ostream &ostr, int i)
{
    return WriteSignedBinary<int>(ostr, i);
}

bool ReadInt(std::istream &istr, int &i)
{
    return ReadSignedBinary<int>(istr, i);
}

bool WriteLong(std::ostream &ostr, long l, unsigned padTo)
{
    return WriteSignedBinary<long>(ostr, l, padTo);
}

bool ReadLong(std::istream &istr, long &l)
{
    return ReadSignedBinary<long>(istr, l);
}

bool WriteFloat(std::ostream &ostr, float f)
{
    union TypeConverter
    {
        float fValue;
        uint32_t iValue;
    };

    TypeConverter u;
    u.fValue = 0.0f;
    u.iValue = 0;

    u.fValue = f;
    return WriteBinary<uint32_t>(ostr, u.iValue);
}

bool ReadFloat(std::istream &istr, float &f)
{
    union TypeConverter
    {
        float fValue;
        uint32_t iValue;
    };

    TypeConverter u;
    u.fValue = 0.0f;
    u.iValue = 0;

    if (!ReadBinary<uint32_t>(istr, u.iValue)) return false;
    f = u.fValue;
    return true;
}

bool WriteDouble(std::ostream &ostr, double d)
{
    union TypeConverter
    {
        double dValue;
        uint64_t iValue;
    };

    TypeConverter u;
    u.dValue = 0.0;
    u.iValue = 0;

    u.dValue = d;
    return WriteBinary<uint64_t>(ostr, u.iValue);
}

bool ReadDouble(std::istream &istr, double &d)
{
    union TypeConverter
    {
        double dValue;
        uint64_t iValue;
    };

    TypeConverter u;
    u.dValue = 0.0;
    u.iValue = 0;

    if (!ReadBinary<uint64_t>(istr, u.iValue)) return false;
    d = u.dValue;
    return true;
}

bool WriteString(std::ostream &ostr, const std::string &s)
{
    if (!WriteBinary<size_t>(ostr, s.size())) return false;
    if (!ostr.write(&(s[0]), s.size())) return false;

    return true;
}

bool ReadString(std::istream &istr, std::string &s)
{
    size_t length = 0;
    if (!ReadBinary<size_t>(istr, length)) return false;

    s.resize(length);
    if (length != 0)
    {
        if (!istr.read(&(s[0]), length)) return false;
    }
    return true;
}

bool WriteType(std::ostream &ostr, const CBotTypResult &type)
{
    int typ = type.GetType();
    if ( typ == CBotTypIntrinsic ) typ = CBotTypClass;
    if ( !WriteWord(ostr, typ) ) return false;
    if ( typ == CBotTypClass )
    {
        CBotClass* p = type.GetClass();
        if (!WriteString(ostr, p->GetName())) return false;
    }
    if ( type.Eq( CBotTypArrayBody ) ||
         type.Eq( CBotTypArrayPointer ) )
    {
        if (!WriteWord(ostr, type.GetLimite())) return false;
        if (!WriteType(ostr, type.GetTypElem())) return false;
    }

    if ( type.Eq(CBotTypPointer) )
    {
        if (type.GetClass() != nullptr)
        {
            if (!WriteString(ostr, type.GetClass()->GetName())) return false;
        }
        else if (!WriteString(ostr, "")) return false;
    }
    return true;
}

bool ReadType(std::istream &istr, CBotTypResult &type)
{
    unsigned short  w, ww;
    if (!ReadWord(istr, w)) return false;
    type.SetType(w);

    if ( type.Eq( CBotTypIntrinsic ) )
    {
        type = CBotTypResult( w, "point" );
    }

    if ( type.Eq( CBotTypClass ) )
    {
        std::string  s;
        if (!ReadString(istr, s)) return false;
        type = CBotTypResult( w, s );
    }

    if ( type.Eq( CBotTypArrayPointer ) ||
         type.Eq( CBotTypArrayBody ) )
    {
        CBotTypResult   r;
        if (!ReadWord(istr, ww)) return false;
        if (!ReadType(istr, r)) return false;
        type = CBotTypResult( w, r );
        type.SetLimite(static_cast<short>(ww));
    }

    if ( type.Eq(CBotTypPointer) )
    {
        std::string className;
        if (!ReadString(istr, className)) return false;
        type = CBotTypResult(w, className);
    }
    return true;
}

bool WriteStream(std::ostream &ostr, std::istream& istr)
{
    if (!istr.seekg(0, istr.end)) return false;
    auto size = istr.tellg();

    if (size == 0) return WriteLong(ostr, 0);
    if (!WriteLong(ostr, size)) return false;

    if (!istr.seekg(0, istr.beg)) return false;
    while (size > 0)
    {
        size -= 1;
        if (!ostr.put(istr.get())) return false;
    }

    return true;
}

bool ReadStream(std::istream& istr, std::ostream &ostr)
{
    long length;
    if (!ReadLong(istr, length)) return false;
    if (length == 0) return true;

    while (length-- > 0)
    {
        if (!ostr.put(istr.get())) return false;
    }
    return true;
}

} // namespace CBot
