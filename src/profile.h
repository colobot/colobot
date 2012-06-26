// * This file is part of the COLOBOT source code
// * Copyright (C) 2001-2008, Daniel ROUX & EPSITEC SA, www.epsitec.ch
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

// profile.h

#ifndef _PROFILE_H_
#define _PROFILE_H_


#define STRICT
#define D3D_OVERLOADS


extern BOOL InitCurrentDirectory();
extern BOOL SetProfileString(char* section, char* key, char* string);
extern BOOL GetProfileString(char* section, char* key, char* buffer, int max);
extern BOOL SetProfileInt(char* section, char* key, int value);
extern BOOL GetProfileInt(char* section, char* key, int &value);
extern BOOL SetProfileFloat(char* section, char* key, float value);
extern BOOL GetProfileFloat(char* section, char* key, float &value);


#endif //_PROFILE_H_
