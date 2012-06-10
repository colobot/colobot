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

// language.h

#pragma once


#define _FULL			true		// CoLoBoT
#define _SCHOOL			false		// CeeBot-A or Teen
  #define _TEEN			false		// false for CeeBot-A, true for CeeBot-Teen
  #define _EDU			false
  #define _PERSO		false
  #define _CEEBOTDEMO	false
#define _NET			false
#define _DEMO			false		// DEMO only CoLoBoT (with _Full = false)!

#define _FRENCH			true
#define _ENGLISH		false
#define _GERMAN			false
#define _WG				false
#define _POLISH			false

#define _NEWLOOK		false		// false for CoLoBoT, true for all CeeBot
#define _SOUNDTRACKS	false		// always false since InitAudioTrackVolume crop in Vista


// Verifications

#if !_FULL & !_SCHOOL & !_NET & !_DEMO
-> no version chosen!
#endif

#if _SCHOOL
#if !_EDU & !_PERSO & !_CEEBOTDEMO
-> EDU or PERSO or CEEBOTDEMO?
#endif
#if _EDU & _PERSO & _CEEBOTDEMO
-> EDU and PERSO and CEEBOTDEMO not at the same time!!!
#endif
#endif
