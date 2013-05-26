// * This file is part of the COLOBOT source code
// * Copyright (C) 2012, Polish Portal of Colobot (PPC)
// *
// * This program is free software: you can redistribute it and/or modify
// * it under the terms of the GNU General Public License as published by
// * the Free Software Foundation, either version 3 of the License, or
// * (at your option) any later version.
// *
// * This program is distributed in the hope that it will be useful,
// * but WITHOUT ANY WARRANTY; without even the implied warranty of
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// * GNU General Public License for more details.
// *
// * You should have received a copy of the GNU General Public License
// * along with this program. If not, see  http://www.gnu.org/licenses/.


#include "common/stringutils.h"


std::string StrUtils::Replace(const std::string &str, const std::string &oldStr, const std::string &newStr)
{
  std::string result = str;
  size_t pos = 0;
  while ((pos = str.find(oldStr, pos)) != std::string::npos)
  {
    result.replace(pos, oldStr.length(), newStr);
    pos += newStr.length();
  }
  return result;
}

std::string StrUtils::UnicodeCharToUtf8(unsigned int ch)
{
  std::string result;
  if (ch < 0x0080)
  {
    result += static_cast<char>(ch);
  }
  else if (ch < 0x0800)
  {
    char ch1 = 0xC0 | ((ch & 0x07C0) >> 6);
    char ch2 = 0x80 | (ch & 0x3F);
    result += ch1;
    result += ch2;
  }
  else
  {
    char ch1 = 0xE0 | ((ch & 0xF000) >> 12);
    char ch2 = 0x80 | ((ch & 0x07C0) >> 6);
    char ch3 = 0x80 | (ch & 0x3F);
    result += ch1;
    result += ch2;
    result += ch3;
  }
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

  unsigned int result = 0;
  if ((ch[0] & 0x80) == 0)
  {
    if (ch.size() == 1)
      result = static_cast<unsigned int>(ch[0]);
  }
  else if ((ch[0] & 0xC0) == 0xC0)
  {
    if (ch.size() == 2)
    {
      unsigned int ch1 = (ch[0] & 0x1F) << 6;
      unsigned int ch2 = (ch[1] & 0x3F);
      result = ch1 | ch2;
    }
  }
  else
  {
    if (ch.size() == 3)
    {
      unsigned int ch1 = (ch[0] & 0xF0) << 12;
      unsigned int ch2 = (ch[1] & 0xC0) << 6;
      unsigned int ch3 = (ch[2] & 0xC0);
      result = ch1 | ch2 | ch3;
    }
  }

  return result;
}

std::wstring StrUtils::Utf8StringToUnicode(const std::string &str)
{
  std::wstring result;
  unsigned int pos = 0;
  while (pos < str.size())
  {
    int len = StrUtils::Utf8CharSizeAt(str, pos);
    if (len == 0)
      break;

    std::string ch = str.substr(pos, len);
    result += static_cast<wchar_t>(StrUtils::Utf8CharToUnicode(ch));
    pos += len;
  }
  return result;
}

int StrUtils::Utf8CharSizeAt(const std::string &str, unsigned int pos)
{
  if (pos >= str.size())
    return 0;

  if ((str[pos] & 0x80) == 0)
    return 1;
  else if ((str[pos] & 0xC0) == 0xC0)
    return 2;
  else
    return 3;

  return 0;
}

size_t StrUtils::Utf8StringLength(const std::string &str)
{
  size_t result = 0;
  unsigned int i = 0;
  while (i < str.size())
  {
      i += Utf8CharSizeAt(str, i);
      ++result;
  }
  return result;
}

