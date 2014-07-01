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

/**
 * \file common/stringutils.h
 * \brief Some useful string operations
 */

#pragma once


#include <string>
#include <sstream>

namespace StrUtils {

//! Converts a value to string
/** If given, \a ok is set to true/false on success/failure.
    Warning: To avoid unnecessary problems, *always* give full template qualifier e.g. ToString\<int\> */
template<class T>
std::string ToString(T value, bool *ok = nullptr)
{
  std::ostringstream s;
  s << value;
  if (ok != nullptr)
    *ok = !s.fail();
  return s.str();
}

//! Converts a value to string
/** If given, \a ok is set to true/false on success/failure.
    Warning: To avoid unnecessary problems, *always* give full template qualifier e.g. FromString\<int\> */
template<class T>
T FromString(const std::string &str, bool *ok = nullptr)
{
  std::istringstream s;
  s.str(str);
  T value;
  s >> value;
  if (ok != nullptr)
    *ok = !s.fail();
  return value;
}

//! Replacement for sprintf()
std::string Format(const char *fmt, ...);

//! Returns a string with every occurence of \a oldStr in \a str replaced to \a newStr
std::string Replace(const std::string &str, const std::string &oldStr, const std::string &newStr);


//! Converts a wide Unicode char to a single UTF-8 encoded char
std::string UnicodeCharToUtf8(unsigned int ch);

//! Converts a wide Unicode string to a UTF-8 encoded string
std::string UnicodeStringToUtf8(const std::wstring &str);

//! Converts a UTF-8 encoded single character to wide Unicode char
unsigned int Utf8CharToUnicode(const std::string &ch);

//! Converts a UTF-8 encoded string to wide Unicode string
std::wstring Utf8StringToUnicode(const std::string &str);

//! Returns the size in bytes of UTF-8 character at given \a pos in a UTF-8 \a str
int Utf8CharSizeAt(const std::string &str, unsigned int pos);

//! Returns the length in characters of UTF-8 string \a str
size_t Utf8StringLength(const std::string &str);

}; // namespace StrUtil

