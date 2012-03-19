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
// * along with this program. If not, see  http://www.gnu.org/licenses/.// language.h

#define _FULL			TRUE		// CoLoBoT
#define _SCHOOL			FALSE		// CeeBot-A ou Teen
  #define _TEEN			FALSE		// FALSE si CeeBot-A, TRUE si CeeBot-Teen
  #define _EDU			FALSE
  #define _PERSO		FALSE
  #define _CEEBOTDEMO	FALSE
#define _NET			FALSE
#define _DEMO			FALSE		// DEMO uniquement de CoLoBoT (avec _FULL = FALSE) !

#define _FRENCH			TRUE
#define _ENGLISH		FALSE
#define _GERMAN			FALSE
#define _WG				FALSE
#define _POLISH			FALSE

#define _NEWLOOK		FALSE		// FALSE pour CoLoBoT, TRUE pour tous les CeeBot
#define _SOUNDTRACKS	FALSE		// toujours FALSE depuis que InitAudioTrackVolume plante sous Vista


// Vérifications

#if !_FULL & !_SCHOOL & !_NET & !_DEMO
-> aucune version choisie !!!
#endif

#if _SCHOOL
#if !_EDU & !_PERSO & !_CEEBOTDEMO
-> EDU ou PERSO ou CEEBOTDEMO ?
#endif
#if _EDU & _PERSO & _CEEBOTDEMO
-> pas EDU et PERSO et CEEBOTDEMO en même temps !!!
#endif
#endif
