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


// interpreter of the lanuage CBot for game COLOBOT
// writing a program is first transformed into a list of tokens
// before tackling the compiler itself
// for example
// int var = 3 * ( pos.y + x )
// is decomposed into (each line is a token)
//      int
//      var
//      =
//      3
//      *
//      (
//      pos.y
//      +
//      x
//      )

#pragma once

extern bool IsOfType(CBotToken* &p, int type1, int type2 = -1);
extern bool IsOfTypeList(CBotToken* &p, int type1, ...);

