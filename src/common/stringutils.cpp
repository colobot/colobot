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


#include "common/stringutils.h"

#include "common/logger.h"
#include <cstdarg>
#include <cstdio>
#include <cstring>          // memset used for trace
#include <vector>

unsigned int StrUtils::HexStringToInt(const std::string& str)
{
    std::stringstream ss;
    ss << std::hex << str;
    unsigned int x;
    ss >> x;
    return x;
}

namespace
{

std::string VFormat(const char *fmt, va_list ap)
{
    std::size_t size = 1024;
    char stackbuf[1024];
    std::vector<char> dynamicbuf;
    char *buf = &stackbuf[0];

    while (true)
    {
        int needed = vsnprintf (buf, size, fmt, ap);

        if (needed <= static_cast<int>(size) && needed >= 0)
        {
            return std::string(buf, static_cast<std::size_t>(needed));
        }

        size = (needed > 0) ? (needed+1) : (size*2);
        dynamicbuf.resize(size);
        buf = &dynamicbuf[0];
    }
}

} // anonymous namespace

std::string StrUtils::Format(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    std::string buf = VFormat(fmt, ap);
    va_end(ap);
    return buf;
}

std::string StrUtils::Replace(const std::string &str, const std::string &oldStr, const std::string &newStr)
{
    std::string result = str;
    std::size_t pos = 0;
    while ((pos = str.find(oldStr, pos)) != std::string::npos)
    {
        result.replace(pos, oldStr.length(), newStr);
        pos += newStr.length();
    }
    return result;
}

std::string StrUtils::UnicodeCharToUtf8(unsigned int ch)
{
    // nota : similare with CBot:: CodePointToUTF8 (without traces)
    std::string result("");
    if (ch < 0x0080)
    {
        result += static_cast<char>(ch);
        //GetLogger()->Trace("StrUtils::UnicodeCharToUtf8 1: %d <%s>\n",ch,result.c_str());
    }
    else if (ch < 0x0800)
    {
        char ch1 = 0xC0 | ((ch & 0x07C0) >> 6);
        char ch2 = 0x80 | (ch & 0x3F);
        result += ch1;
        result += ch2;
        GetLogger()->Trace("StrUtils::UnicodeCharToUtf8 2: %d <%s>\n",ch,result.c_str());
    }
    else if(0xd800<=ch && ch<=0xDFFF)   //invalid block of utf8
        GetLogger()->Trace("StrUtils::UnicodeCharToUtf8 INVALID UNICODE CHAR: %d %c\n",ch,ch);
    else if(ch<=0xFFFF)
    {
        char ch1 = 0xE0 | ((ch & 0xF000) >> 12);
        char ch2 = 0x80 | ((ch & 0x07C0) >> 6);
        char ch3 = 0x80 | (ch & 0x3F);
        result += ch1;
        result += ch2;
        result += ch3;
        GetLogger()->Trace("StrUtils::UnicodeCharToUtf8 3: %d <%s>\n",ch,result.c_str());
    }
    else if(ch<=0x10FFFF)
    {
        char ch1 = 0xF0 | (ch >> 18);
        char ch2 = 0x80 | ((ch >> 12) & 0x3F);
        char ch3 = 0x80 | ((ch >> 6) & 0x3F);
        char ch4 = 0x80 | (ch & 0x3F);
        result += ch1;
        result += ch2;
        result += ch3;
        result += ch4;
        GetLogger()->Trace("StrUtils::UnicodeCharToUtf8 4: %d <%s>\n",ch,result.c_str());
    }
    else
        GetLogger()->Warn("StrUtils::UnicodeCharToUtf8 FAIL (size>=5? UTF16?): %d\n",ch);
    return result;
}

std::string StrUtils::UnicodeStringToUtf8(const std::wstring &str)
{
    std::string result;
    for (unsigned int i = 0; i < str.size(); ++i)
        result += StrUtils::UnicodeCharToUtf8(static_cast<unsigned int>(str[i]));

    return result;
}

unsigned int StrUtils::Utf8CharToUnicode(const std::string &ch)
{
    if (ch.empty())
        return 0;
    //  memo UTF8:
    // 1 byte  : 00000000 -- 0000007F:  0xxxxxxx
    // 2 bytes : 00000080 -- 000007FF:  110xxxxx 10xxxxxx
    // 3 bytes : 00000800 -- 0000FFFF:  1110xxxx 10xxxxxx 10xxxxxx
    // 4 bytes : 00010000 -- 001FFFFF:  11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
    unsigned int result = 0;
    if ((ch[0] & 0x80) == 0)
    {
        if (ch.size() == 1)
        {
            result = static_cast<unsigned int>(ch[0]);
            //GetLogger()->Trace("StrUtils::Utf8CharToUnicode 1: <%s> %d\n",ch.c_str(),result);
        }
        else
            GetLogger()->Warn("StrUtils::Utf8CharToUnicode Bad UTF8 1 char input ? : <%s>\n",ch.c_str());
    }
    else if ((ch[0] & 0xE0) == 0xC0)
    {
        if (ch.size() == 2)
        {
            unsigned int ch1 = (ch[0] & 0x1F) << 6;
            unsigned int ch2 = (ch[1] & 0x3F);
            result = ch1 | ch2;
            GetLogger()->Trace("StrUtils::Utf8CharToUnicode 2: <%s> %d\n",ch.c_str(),result);
        }
        else
            GetLogger()->Warn("StrUtils::Utf8CharToUnicode Bad UTF8 2 char input ? : <%s>\n",ch.c_str());
    }
    else if (0xED==static_cast<unsigned char>(ch[0]) && (ch[1] & 0xA0) == 0xA0)
        GetLogger()->Warn("StrUtils::Utf8CharToUnicode Bad UTF8 2 char input ? : EDA0? <%s>\n",ch.c_str());
    else if ((ch[0] & 0xF0) == 0xE0)
    {
        if (ch.size() == 3)
        {
            unsigned int ch1 = (ch[0] & 0x0F) << 12;
            unsigned int ch2 = (ch[1] & 0x3F) << 6;
            unsigned int ch3 = (ch[2] & 0x3F);
            result = ch1 | ch2 | ch3;
            GetLogger()->Trace("StrUtils::Utf8CharToUnicode 3: <%s> %d\n",ch.c_str(),result);
        }
        else
            GetLogger()->Warn("StrUtils::Utf8CharToUnicode Bad UTF8 3 char input ? : <%s>\n",ch.c_str());
    }
    else if ((ch[0] & 0xF8) == 0xF0)
    {
        if (ch.size() == 4)
        {
            unsigned int ch1 = (ch[0] & 0x07) << 18;
            unsigned int ch2 = (ch[1] & 0x3F) << 12;
            unsigned int ch3 = (ch[2] & 0x3F) << 6;
            unsigned int ch4 = (ch[3] & 0x3F) ;
            result = ch1 | ch2 | ch3 | ch4;
            GetLogger()->Trace("StrUtils::Utf8CharToUnicode 4: <%s> %d\n",ch.c_str(),result);
        }
        else
            GetLogger()->Warn("StrUtils::Utf8CharToUnicode Bad UTF8 4 char input ? : <%s>\n",ch.c_str());
    }
    else
        GetLogger()->Warn("StrUtils::Utf8CharToUnicode Bad UTF8 (more than 4 char inputs ?) : <%s>\n",ch.c_str());

    return result;
}

std::wstring StrUtils::Utf8StringToUnicode(const std::string &str)
{
    std::wstring result;
    std::size_t pos = 0;
    unsigned short len;
    while (pos < str.size())
    {
        len = StrUtils::Utf8CharSizeAt(str, pos);
        if (len == 0)
            break;

        std::string ch = str.substr(pos, len);
        result += static_cast<wchar_t>(StrUtils::Utf8CharToUnicode(ch));
        pos += len;
    }
    return result;
}

/**
 * @brief      get the nb of char to encode current utf8 element
 *
 * @param[in]  str   input string
 * @param[in]  pos   position into string
 * @param[in]  size  preknown size of str (avoid new strlen)
 *
 * @return     nb of char equivalent for current displayed utf8 character
 */
unsigned short StrUtils::Utf8CharSizeAt(const char*const str, const std::size_t pos, const std::size_t size)
{
    //  memo UTF8:
    // 1 byte  : 00000000 -- 0000007F:  0xxxxxxx
    // 2 bytes : 00000080 -- 000007FF:  110xxxxx 10xxxxxx
    // 3 bytes : 00000800 -- 0000FFFF:  1110xxxx 10xxxxxx 10xxxxxx
    // 4 bytes : 00010000 -- 001FFFFF:  11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
    if (size <= pos)  // secu (avoid slide after the end reached)
        return 0;
    if ((str[pos] & 0x80) == 0)
        return 1;
    //nota:  quick header check without checking inside.
    if ((str[pos] & 0xE0) == 0xC0)
        return 2;
    if ((str[pos] & 0xF0) == 0xE0)
        return 3;
    if ((str[pos] & 0xF8) == 0xF0)
        return 4;
    /* //theorical chars, non respecting current UTF8 std - let for possible extensions
    if ((str[pos] & 0xFC) == 0xF8)
        // GetLogger()->Warn("UTF8 input with 5 bytes - UTF8 ?\n");
        return 5;
    if ((str[pos] & 0xFE) == 0xFC)
        // GetLogger()->Warn("UTF8 input with 6 bytes - UTF8 ?\n");
        return 6; */
    // rescue in case of bad input placement (pos "inside" a char), return a way to get next one
    unsigned short ret=1;
    while (pos+ret<size && 0x80==(str[pos+ret] & 0xC0) )
        ++ret;
    for(unsigned short i=0;i<ret && pos+i<size;++i)
        GetLogger()->Warn("Bad UTF8 input => try rescue: size : %d : at %d: <%c>(%#02X)\n", ret, pos+i, str[pos+i], static_cast<unsigned char>(str[pos+i]));
    return ret; //nota : occurrences !! - ret:1 on helpers screen (sample earth level 3 fr)
}

std::size_t StrUtils::Utf8StringLength(const std::string &str)
{
    std::size_t result = 0;
    std::size_t i = 0;
    unsigned short tmp;
    while (i < str.size())
    {
        tmp = Utf8CharSizeAt(str, i);
        if(!tmp)
        {
            GetLogger()->Warn("Bad UTF8 input : <%s> a 0 size elt at : %d, nb %d\n", str.c_str(),i,result);
            break;
        }
        i+=tmp;
        ++result;
    }
    return result;
}

/**
 * @brief      warning log for utf8 char - trace function to display eventual
 *             problematic chars
 * @note       don't forget to check if the wanted utf8 char is permit by the
 *             used font ( check: http://mathew-kurian.github.io/CharacterMap/ )
 *
 * @param[in]  invit    The invit message
 * @param[in]  str      input string
 * @param[in]  pos      The position into input string
 * @param[in]  size     preknown size of str (avoid new strlen)
 * @param[in]  bLogStd  do log standard one chars (default false)
 */
void StrUtils::Utf8Log(const char*const invit, const char*const str, const std::size_t pos,
                       const std::size_t size, const bool bLogStd)
{
    assert(pos<size);
    const unsigned char len = Utf8CharSizeAt(str, pos);
    if(1==len)
    {
        if(bLogStd)
            GetLogger()->Warn("%s len %d <%c>\n", invit, len, str[pos]);
    }
    else
    {
        if(7<=len)   //improbable secu
            GetLogger()->Warn("%s Bad UTF8 input at 1st char there : <%s> - => try rescue : %d\n",
                invit, str+pos, len);
        else
        {
            char plomp[7];
            std::memset(plomp,0,9);
            for(unsigned short i=0 ; i<len && pos+i<size ; ++i)
                plomp[i]=str[pos+i];
            GetLogger()->Warn("%s len %d <%s>\n", invit, len, plomp);
        }
    }
}

/**
 * @brief      Check a utf8 & warn trace if ko
 *
 * @param[in]  invit          invit in case of KO
 * @param[in]  str            input string
 * @param[in]  pos            The position into str
 * @param[in]  doInsideCheck  do detailed inside utf8 check
 * @param[in]  nbBytes        optionnal - The number of bytes of this char
 * @param[in]  size     preknown size of str (avoid new strlen)
 * @return     the designed utf8 char is correct
 * @precond    pos is inside the string, not at end
 */
bool StrUtils::Utf8CheckWarnup(const char*const invit ,const char*const str, const std::size_t pos,
                               const bool doInsideCheck, short nbBytes, const std::size_t size)
{
    // assert(strlen(str)>=pos);
    if ('\0' == str[pos])
        //warn?
        return -1 == nbBytes || 0 ==nbBytes;
    if ( -1 == nbBytes)
        nbBytes = Utf8CharSizeAt(str, pos, size);
    if ( 0 == nbBytes || 4 < nbBytes )
    {
        //potential character error !! (non UTF8)
        StrUtils::Utf8Log(invit, str, pos, size);
        return false;
    }
    if (doInsideCheck)
        for (short i=1;i<nbBytes;++i)
            if(0x80 != (str[pos+i] & 0xC0))
            {
                GetLogger()->Warn("%s - inside check KO : <%s> at %d: <%c>(%#02X)\n", invit, str+pos, pos, str[pos+i], static_cast<unsigned char>(str[pos+i]));
                return false;   //error : bad UTF8 !! => dismiss it
            }
    return true;
}

bool StrUtils::Utf8CheckWarnup(const char*const invit ,
                               const char*const str, const std::size_t pos,
                               const bool doInsideCheck)
{
    return Utf8CheckWarnup(invit, str, pos, doInsideCheck, -1, strlen(str) );
}
