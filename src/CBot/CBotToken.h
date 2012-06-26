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
// * along with this program. If not, see  http://www.gnu.org/licenses/.////////////////////////////////////////////////////////////////////
// interpréteur pour le language CBot du jeu COLOBOT


// un programme écrit est tout d'abord transformé en une liste de tokens
// avant d'aborder le compilateur proprement dit
// par exemple
// int var = 3 * ( pos.y + x )
// est décomposé en (chaque ligne est un token)
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


extern BOOL IsOfType(CBotToken* &p, int type1, int type2 = -1);
extern BOOL IsOfTypeList(CBotToken* &p, int type1, ...);
